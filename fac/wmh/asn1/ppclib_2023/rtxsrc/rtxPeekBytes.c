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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStream.h"


EXTRTMETHOD int rtxPeekByte(OSCTXT* pctxt, OSOCTET* pbyte)
{
#ifndef _NO_STREAM
   OSRTSTREAM* pStream;
   int stat = 0;

   pStream = OSRTISSTREAM(pctxt) ? pctxt->pStream : 0;

   /* If streaming, load more data from stream if we don't already have enough
      in the buffer. */
   if (0 != pStream && pctxt->buffer.byteIndex >= pctxt->buffer.size)
   {
      stat = rtxStreamLoadInputBuffer(pctxt, 1);
      if (stat < 0) return LOG_RTERR(pctxt, stat);
   }
#endif

   if (pctxt->buffer.byteIndex >= pctxt->buffer.size ) return 0;
   else {
      *pbyte = pctxt->buffer.data[pctxt->buffer.byteIndex];
      return 1;
   }
}


EXTRTMETHOD int rtxPeekBytes(OSCTXT* pctxt, OSOCTET* pdata, OSSIZE bufsize,
   OSSIZE nocts, OSSIZE* pactual)
{
#ifndef _NO_STREAM
   OSRTSTREAM* pStream;
#endif
   if (nocts > bufsize) nocts = bufsize;

   if (nocts == 0) {
      *pactual = 0;
      return 0;
   }

#ifndef _NO_STREAM
   pStream = OSRTISSTREAM(pctxt) ? pctxt->pStream : 0;

   /* If streaming, load more data from stream if we don't already have enough
      in the buffer. */
   if (0 != pStream && pctxt->buffer.byteIndex + nocts > pctxt->buffer.size)
   {
      int stat = rtxStreamLoadInputBuffer(pctxt, nocts);
      if (stat < 0) return LOG_RTERR(pctxt, stat);
   }
#endif

   nocts = OSRTMIN(nocts, pctxt->buffer.size - pctxt->buffer.byteIndex);

   /* If nocts <= 3, do assignments rather than mem copy.*/
   if (nocts <= 3)
   {
      pdata[0] = pctxt->buffer.data[pctxt->buffer.byteIndex];
      if (nocts >= 2) {
         pdata[1] = pctxt->buffer.data[pctxt->buffer.byteIndex + 1];
         if (nocts == 3) pdata[2] =
                           pctxt->buffer.data[pctxt->buffer.byteIndex + 2];
      }
   }
   else {
      OSCRTLSAFEMEMCPY(pdata, bufsize, OSRTBUFPTR(pctxt), nocts);
   }

   *pactual = nocts;

   return 0;
}

