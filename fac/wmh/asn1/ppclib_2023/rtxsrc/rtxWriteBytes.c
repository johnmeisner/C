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

#include <string.h>
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxStream.h"

EXTRTMETHOD int rtxWriteBytes
(OSCTXT* pctxt, const OSOCTET* pdata, size_t nocts)
{
   int stat = 0;
   if (nocts == 0) return 0;
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt) &&
       (0 == (pctxt->pStream->flags & OSRTSTRMF_BUFFERED)))
   {
      /* For memory stream, no flush is needed because the context buffer
         is the stream target */
      if (pctxt->pStream->id != OSRTSTRMID_MEMORY) {
         /* Flush any data that may be in context buffer to stream.  Normally,
            data is written directly to stream; however, some operations (for
            example, bit encode) may write data to the context buffer. */
         if (0 != pctxt->buffer.data) {
            if (pctxt->buffer.bitOffset != 8) {
               /* partial byte */
               pctxt->buffer.byteIndex++;
            }
            if (pctxt->buffer.byteIndex > 0) {
               stat = rtxStreamWrite
                  (pctxt, pctxt->buffer.data, pctxt->buffer.byteIndex);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               pctxt->buffer.byteIndex = 0;
               pctxt->buffer.bitOffset = 8;
            }
         }

         /* Write directly to stream */
         stat = rtxStreamWrite (pctxt, pdata, nocts);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         pctxt->lastChar = pdata[nocts-1];
      }
      else if (nocts > 0) {
         stat = rtxCheckOutputBuffer (pctxt, nocts);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* copy to buffer */
         OSCRTLSAFEMEMCPY (&pctxt->buffer.data[pctxt->buffer.byteIndex],
            pctxt->buffer.size - pctxt->buffer.byteIndex, pdata, nocts);

         pctxt->buffer.byteIndex += nocts;
         pctxt->pStream->ioBytes += nocts;
         pctxt->lastChar = pctxt->buffer.data[pctxt->buffer.byteIndex-1];
      }

   }
   else {
#endif
      if (pctxt->buffer.byteIndex + nocts > pctxt->buffer.size) {
         /* expand buffer */
         if (!OSRTISSTREAM (pctxt)) {
            stat = rtxCheckOutputBuffer (pctxt, nocts);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
#ifndef _NO_STREAM
         else {
            /* copy data to buffer end */
            size_t tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

            if (tail > 0) {
               /* copy to buffer */
               OSCRTLSAFEMEMCPY (&pctxt->buffer.data[pctxt->buffer.byteIndex],
                     tail, pdata, tail);

               pctxt->buffer.byteIndex += tail;
               pdata += tail;
               nocts -= tail;
            }

            /* flush buffer */
            stat = rtxCheckOutputBuffer
               (pctxt, OSRTMAX (pctxt->buffer.size, 1));
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            if (nocts >= pctxt->buffer.size) {
               /* write to stream */
               stat = rtxStreamWrite (pctxt, pdata, nocts);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               pctxt->lastChar = pdata[nocts-1];
               nocts = 0;
            }
         }
#endif
      }

      if (nocts > 0) {
         /* copy to buffer */
         OSCRTLSAFEMEMCPY (&pctxt->buffer.data[pctxt->buffer.byteIndex],
               pctxt->buffer.size - pctxt->buffer.byteIndex, pdata, nocts);

         pctxt->buffer.byteIndex += nocts;
         pctxt->lastChar = pctxt->buffer.data[pctxt->buffer.byteIndex-1];
      }
#ifndef _NO_STREAM
   }
#endif

#ifdef XBDEMO
#define XB_DEMO_MAX_MSG_LENGTH ((OSINT16)10240L)
   if (rtxCtxtGetIOByteCount (pctxt) > XB_DEMO_MAX_MSG_LENGTH) {
      printf ("encode ERROR: Max XML message length for demo exceeded.\n");
      return (RTERR_NOTSUPP);
   }
#endif
   /* LCHECKX (pctxt); */

   return 0;
}

