/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

/* Uncomment to enable diagnostics or add -DRTXDEBUG to gcc command line.
#define RTXDEBUG
*/
#ifdef RTXDEBUG
#define RTLDIAG1(pctxt,msg)     rtxDiagStream(pctxt,msg)
#define RTLDIAG2(pctxt,msg,a)   rtxDiagStream(pctxt,msg,a,b)
#define RTLDIAG3(pctxt,msg,a,b) rtxDiagStream(pctxt,msg,a,b)
#define RTLHEXDUMP(pctxt,buffer,numocts) \
rtxDiagStreamHexDump(pctxt,buffer,numocts)
#else
#define RTLDIAG1(pctxt,msg)
#define RTLDIAG2(pctxt,msg,a)
#define RTLDIAG3(pctxt,msg,a,b)
#define RTLHEXDUMP(pctxt,buffer,numocts)
#endif

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


EXTRTMETHOD int rtxCheckInputBuffer (OSCTXT* pctxt, size_t nbytes)
{
   int badretval = RTERR_ENDOFBUF;
#ifndef _NO_STREAM
   if (OSRTISBUFSTREAM (pctxt)) {
      int stat = rtxStreamLoadInputBuffer(pctxt, nbytes);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      /* The pre-existing behavior in the case of not finding the required
         data in the stream was to log & return RTERR_ENDOFFILE. User code
         may specifically look for that return.
         */
      badretval = RTERR_ENDOFFILE;
   }
   /* else: Not streaming into context buffer, so the data in the context buffer
      is all the data we have. */
#endif
   return (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) ?
      LOG_RTERR (pctxt, badretval) : 0;
}
