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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStream.h"

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test flushing */
#endif

EXTRTMETHOD int rtxReadBytesSafe
(OSCTXT* pctxt, OSOCTET* buffer, size_t bufsize, size_t nocts)
{
   return (nocts <= bufsize) ?
      rtxReadBytes (pctxt, buffer, nocts) : RTERR_BUFOVFLW;
}

EXTRTMETHOD int rtxReadBytes (OSCTXT* pctxt, OSOCTET* pdata, size_t nocts)
{
#ifndef _NO_STREAM
   OSRTSTREAM* pStream = OSRTISSTREAM(pctxt) ? pctxt->pStream : 0;
#endif
   int stat = 0;

   if (nocts == 0) return 0;

   /* Check if buffer contains number of bytes requested */
   if (pctxt->buffer.byteIndex + nocts > pctxt->buffer.size) {
#ifndef _NO_STREAM
      if (0 != pStream) {
         /* Make sure context buffering is set up */
         if (pctxt->buffer.data == 0) {
            stat = rtxStreamInitCtxtBuf (pctxt);
            if (0 != stat) return LOG_RTERR (pctxt, stat);
         }
         /* Read data from stream */
         stat = (int) rtxStreamRead (pctxt, pdata, nocts);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
      else
#endif
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }
#ifndef _NO_STREAM
   else if (0 != pStream && 0 == (pStream->flags & OSRTSTRMF_BUFFERED)) {
      /* Read data directly from stream */
      stat = (int) rtxStreamRead (pctxt, pdata, nocts);
      return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
   }
#endif
   else {
      if (0 != pdata) {
         OSCRTLSAFEMEMCPY (pdata, nocts, OSRTBUFPTR(pctxt), nocts);
      }
      pctxt->buffer.byteIndex += nocts;
   }

   /* Set last character in context */
   if (pctxt->buffer.byteIndex > 0) {
      pctxt->lastChar = pctxt->buffer.data[pctxt->buffer.byteIndex-1];
   }

   return 0;
}

EXTRTMETHOD int rtxReadBytesDynamic
(OSCTXT* pctxt, OSOCTET** ppdata, size_t nocts, OSBOOL* pMemAlloc)
{
   /* Check if buffer contains number of bytes requested */
   if (pctxt->buffer.byteIndex + nocts > pctxt->buffer.size) {
#ifndef _NO_STREAM
      if (OSRTISSTREAM (pctxt)) {
         int stat;

         if (nocts > OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF) {
            *ppdata = (OSOCTET*) rtxMemAlloc (pctxt, nocts);
            if (0 == *ppdata)
               return LOG_RTERR (pctxt, RTERR_NOMEM);

            *pMemAlloc = TRUE;

            stat = rtxReadBytes (pctxt, *ppdata, nocts);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);
         }
         else {
            /* refill buffer */
            stat = rtxCheckInputBuffer (pctxt, nocts);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            *ppdata = OSRTBUFPTR (pctxt);
            pctxt->buffer.byteIndex += nocts;
            *pMemAlloc = FALSE;
         }
      }
      else
#endif /* _NO_STREAM */
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }
   else {
      *ppdata = OSRTBUFPTR (pctxt);
      pctxt->buffer.byteIndex += nocts;
      *pMemAlloc = FALSE;
   }

   return 0;
}

