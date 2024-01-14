/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/

/***********************************************************************
 *
 *  Routine name: rtCheckBuffer
 *
 *  Description:  This routine checks to see if the encode buffer
 *                contains enough free space to hold the given
 *                number of bytes.  If it does not and the buffer
 *                is dynamic, it is expanded.  If not dynamic, an
 *                'RTERR_BUFOVFLW' error is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt       struct* Pointer to context structure
 *  nbytes      uint    Number of bytes of additional space required
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation
 *
 **********************************************************************/

#include <stdlib.h>
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStream.h"

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test flushing */
#endif

int rtxCheckBuffer (OSCTXT* pctxt, size_t nbytes)
{
   /* Accommodate the bit offset */
   size_t idx = (pctxt->buffer.bitOffset != 8) ?
      pctxt->buffer.byteIndex + 1 : pctxt->buffer.byteIndex;

   if ( ( idx + nbytes ) > pctxt->buffer.size ) {
#ifndef _NO_STREAM
      if (OSRTISSTREAM (pctxt)) {
         int stat;

         stat = rtxStreamFlush (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         if ( ( pctxt->buffer.byteIndex + nbytes ) <= pctxt->buffer.size )
            return 0;
      }
#endif /* _NO_STREAM */

      if (pctxt->buffer.dynamic) {

         /* If dynamic encoding enabled, expand the current buffer 	*/
         /* to allow encoding to continue.                              */
         /* change to double buffer size                                */
         /* pctxt->buffer.size += OSRTMAX (OSRTENCBUFSIZ, nbytes);      */

         size_t extent = OSRTMAX (pctxt->buffer.size, nbytes);
         size_t newSize = pctxt->buffer.size + extent;

         pctxt->buffer.size += newSize;

         pctxt->buffer.data = rtxMemReallocArray
            (pctxt, pctxt->buffer.data, pctxt->buffer.size, OSOCTET);

         if (!pctxt->buffer.data)
            return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
      }
      else {
         return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
      }
   }

   return 0;
}

EXTRTMETHOD int rtxCheckOutputBuffer (OSCTXT* pctxt, size_t nbytes)
{
   /* Account for the bit offset.
      rtxExpandOutputBuffer counts partial bytes as available, so if
      we want nbytes byte fully available and there is a partial byte,
      we need to pass nbytes + 1.
   */ 
   if ( pctxt->buffer.bitOffset != 8 ) nbytes++;
   
   if ( ( pctxt->buffer.byteIndex + nbytes ) > pctxt->buffer.size ) {
      int stat = rtxExpandOutputBuffer (pctxt, nbytes);
      if (stat != 0) return LOG_RTERR(pctxt, stat);
   }

   return 0;
}

EXTRTMETHOD int rtxExpandOutputBuffer (OSCTXT* pctxt, size_t nbytes)
{
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      int stat;

      if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_MEMORY) {
         if (0 == (pctxt->pStream->flags & OSRTSTRMF_UNBUFFERED)) {
            pctxt->pStream->flags |= OSRTSTRMF_BUFFERED;

            if (pctxt->buffer.data == 0) {
               /* allocate new buffer */
               stat = rtxInitContextBuffer (pctxt, 0, OSRTSTRM_K_BUFSIZE);
               if (stat != 0)
                  return LOG_RTERR (pctxt, stat);
            }
         }

         if (!(pctxt->pStream->flags & OSRTSTRMF_FIXINMEM) &&
              pctxt->buffer.byteIndex > 0) {
            /* flush buffer to stream and empty it */
            stat = rtxStreamWrite (pctxt, pctxt->buffer.data,
                                   pctxt->buffer.byteIndex);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            if (pctxt->buffer.byteIndex != pctxt->buffer.size)
               pctxt->buffer.data[0] =
                  pctxt->buffer.data[pctxt->buffer.byteIndex];

            pctxt->pStream->bytesProcessed += pctxt->buffer.byteIndex;
            pctxt->buffer.byteIndex = 0;
         }
      }

      if ( ( pctxt->buffer.byteIndex + nbytes ) <= pctxt->buffer.size )
         return 0;
   }
#endif /* _NO_STREAM */

   if (pctxt->buffer.dynamic) {
      OSOCTET* newbuf;
      size_t extent = OSRTMAX (pctxt->buffer.size, nbytes);

      /* If dynamic encoding is enabled, expand the current buffer to   */
      /* allow encoding to continue.                                    */
      pctxt->buffer.size += extent;

      if (0 != (pctxt->flags & OSBUFSYSALLOC)) {
         newbuf = (OSOCTET *)rtxMemSysRealloc
            (pctxt, pctxt->buffer.data, pctxt->buffer.size);
      }
      else {
         newbuf = (OSOCTET *)rtxMemRealloc
            (pctxt, pctxt->buffer.data, pctxt->buffer.size);
      }
      if (newbuf == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

      pctxt->buffer.data = newbuf;
   }
   else {
      return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
   }

   return 0;
}


EXTRTMETHOD OSBOOL rtxIsOutputBufferFlushable(OSCTXT* pctxt)
{
/* The logic here was based on logic for flushing in rtxCheckOutputBuffer.
*/
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_MEMORY) {
         return !(pctxt->pStream->flags & OSRTSTRMF_FIXINMEM);
      }
   }
#endif /* _NO_STREAM */
   return FALSE;
}


EXTRTMETHOD int rtxFlushOutputBuffer(OSCTXT* pctxt)
{
/* The logic here was based on logic for flushing in rtxCheckOutputBuffer.
   It is assumed that rtxIsOutputBufferFlushable would return true, so we
   do not make checks that are made there, to avoid duplication.
*/

#ifndef _NO_STREAM
   int stat;

   if (pctxt->buffer.byteIndex > 0) 
   {
      /* flush buffer to stream and empty it */
      stat = rtxStreamWrite (pctxt, pctxt->buffer.data,
                             pctxt->buffer.byteIndex);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (pctxt->buffer.bitOffset != 8) {
         /* move partial byte to the start of buffer */
         pctxt->buffer.data[0] =
            pctxt->buffer.data[pctxt->buffer.byteIndex];
      }

      pctxt->pStream->bytesProcessed += pctxt->buffer.byteIndex;
      pctxt->buffer.byteIndex = 0;
      
      return stat;
   }
   /* else: nothing to do */

#endif /* _NO_STREAM */

   return 0;
}


#ifndef _NO_STREAM
static int readInputStream (OSCTXT* pctxt, size_t nbytes)
{
   int stat;
   OSSIZE tail = 0;

   if (0 == pctxt) {
      return RTERR_INVPARAM;
   }

   if (0 == pctxt->pStream) {
      return LOG_RTERR (pctxt, RTERR_NOTINIT);
   }

   if (pctxt->buffer.data == 0) {
      stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   if (pctxt->buffer.size + pctxt->pStream->bytesProcessed ==
       pctxt->pStream->segsize)
   {
      /* unexpected end of segment */
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }

   if (pctxt->buffer.byteIndex + nbytes > pctxt->pStream->bufsize) {
      if (pctxt->pStream->flags & OSRTSTRMF_FIXINMEM) {
         if (pctxt->pStream->markedBytesProcessed > 0) {
            /* skip unmarked space */
            tail = pctxt->buffer.size - pctxt->pStream->markedBytesProcessed;
            if (tail > 0)
               OSCRTLSAFEMEMMOVE (pctxt->buffer.data, pctxt->buffer.size,
                     pctxt->buffer.data + pctxt->pStream->markedBytesProcessed,
                     tail);

            pctxt->buffer.byteIndex -= pctxt->pStream->markedBytesProcessed;
            pctxt->pStream->markedBytesProcessed = 0;
            pctxt->buffer.size = tail;
         }

         if (pctxt->buffer.byteIndex + nbytes <= pctxt->pStream->bufsize)
            ; /* buffer has enough space now */
         else if (pctxt->buffer.dynamic) {
            OSOCTET* newbuf;
            size_t extent = OSRTMAX (pctxt->pStream->bufsize, nbytes);

            pctxt->pStream->bufsize += extent;

#ifndef _NO_STREAM
            if ((pctxt->pStream->flags & OSRTSTRMF_BUFFERED))
               newbuf = (OSOCTET *)rtxMemSysRealloc (pctxt, pctxt->buffer.data,
                  pctxt->pStream->bufsize);
            else
#endif /* _NO_STREAM */
               newbuf = (OSOCTET *)rtxMemRealloc (pctxt, pctxt->buffer.data,
                  pctxt->pStream->bufsize);

            if (newbuf == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

            pctxt->buffer.data = newbuf;
            tail = pctxt->buffer.size;
         }
         else {
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
         }
      }
      else {
         /* move data to start of buffer */

         tail = pctxt->buffer.size - pctxt->buffer.byteIndex +
                MIN_STREAM_BACKOFF;

         if (tail > 0) {
            RTDIAG3 (pctxt, "move " OSSIZEFMT " bytes from index "
                     OSSIZEFMT " to start of buffer\n", tail,
                     pctxt->buffer.byteIndex);

            OSCRTLSAFEMEMMOVE (pctxt->buffer.data, pctxt->buffer.size,
              pctxt->buffer.data + pctxt->buffer.byteIndex - MIN_STREAM_BACKOFF,
              tail);
         }
         pctxt->pStream->bytesProcessed +=
            pctxt->buffer.byteIndex - MIN_STREAM_BACKOFF;

         pctxt->buffer.size = tail;
         pctxt->buffer.byteIndex = MIN_STREAM_BACKOFF;
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;

         RTDIAG2 (pctxt, "pctxt->buffer.size = " OSSIZEFMT "\n", tail);
         RTHEXDUMP (pctxt, pctxt->buffer.data, pctxt->buffer.size);
      }
   }
   else
      tail = pctxt->buffer.size;

   /* read data */
   stat = rtxStreamReadDirect (pctxt, pctxt->buffer.data + tail,
                               pctxt->pStream->bufsize - tail);
   if (stat == 0)
      return LOG_RTERR (pctxt, RTERR_ENDOFFILE);
   else if (stat < 0)
      return LOG_RTERR (pctxt, stat);

   pctxt->buffer.size += stat;

   if (pctxt->buffer.size + pctxt->pStream->bytesProcessed >
          pctxt->pStream->segsize)
   {
      /* set segment border */
      pctxt->buffer.size =
         pctxt->pStream->segsize - pctxt->pStream->bytesProcessed;
   }

   return 0;
}
#endif /* _NO_STREAM */

EXTRTMETHOD int rtxCheckInputBuffer (OSCTXT* pctxt, size_t nbytes)
{
#ifndef _NO_STREAM
   if (OSRTISBUFSTREAM (pctxt)) {
      if (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) {
         int stat;
         RTDIAG3 (pctxt, OSSIZEFMT " > " OSSIZEFMT "; "
                  "stream buffer refill needed\n",
                  pctxt->buffer.byteIndex + nbytes, pctxt->buffer.size);

         stat = readInputStream (pctxt, nbytes);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         RTDIAG1 (pctxt, "buffer contents after read:\n");
         RTDIAG2 (pctxt, "pctxt->buffer.size = " OSSIZEFMT "\n",
                  pctxt->buffer.size);
         RTDIAG2 (pctxt, "pctxt->buffer.byteIndex = " OSSIZEFMT "\n",
                  pctxt->buffer.byteIndex);
         RTHEXDUMP (pctxt, pctxt->buffer.data, pctxt->buffer.size);
      }
   }
#endif
   return (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) ?
      LOG_RTERR (pctxt, RTERR_ENDOFBUF) : 0;
}
