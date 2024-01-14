/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "rtpersrc/pd_common.hh"
#include "rtxsrc/rtxStream.h"

EXTPERMETHOD OSBOOL pd_isFragmented (OSCTXT* pctxt)
{
   unsigned bitOff = pctxt->buffer.bitOffset;
   size_t idx = pctxt->buffer.byteIndex;
   const OSOCTET* p = pctxt->buffer.data + idx;

   OSUINT32 nbytes = (17 - pctxt->buffer.bitOffset) / 8;

   if (idx + nbytes > pctxt->buffer.size) {
      int stat = rtxCheckInputBuffer (pctxt, 1);
      if (stat < 0)
         return FALSE; /* pd_Length will return error */
   }

   if (((*p >> --bitOff) & 1) == 0)
      return FALSE;

   if (bitOff == 0) {
      bitOff = 8;
      p++;
   }

   if (((*p >> --bitOff) & 1) == 0)
      return FALSE;

   return TRUE;
}

EXTPERMETHOD void pd_OpenTypeStart
(OSCTXT* pctxt, OSSIZE* pSavedSize, OSINT16* pSavedBitOff)
{
   size_t tm = *(pSavedSize) + pctxt->buffer.byteIndex;
   if (pctxt->buffer.bitOffset != 8) tm++;

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      *pSavedSize = pctxt->pStream->segsize;
      pctxt->pStream->segsize = tm + pctxt->pStream->bytesProcessed;
   }
   else
#endif /* _NO_STREAM */
      *pSavedSize = pctxt->buffer.size;

   if (tm < pctxt->buffer.size)
      pctxt->buffer.size = tm;

   *pSavedBitOff = pctxt->buffer.bitOffset;
}

EXTPERMETHOD int pd_OpenTypeEnd
(OSCTXT* pctxt, OSSIZE savedSize, OSINT16 savedBitOff)
{
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      if (pctxt->buffer.byteIndex == pctxt->buffer.size &&
          pctxt->buffer.bitOffset < savedBitOff)
      {
         /* position out of open type border */
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
      }

      pctxt->buffer.byteIndex = pctxt->buffer.size - (savedBitOff != 8);
      pctxt->pStream->segsize = savedSize;

      pctxt->buffer.size =
         pctxt->pStream->ioBytes - pctxt->pStream->bytesProcessed;

      if (pctxt->buffer.size + pctxt->pStream->bytesProcessed > savedSize) {
         /* set segment border */
         pctxt->buffer.size =
            pctxt->pStream->segsize - pctxt->pStream->bytesProcessed;
      }

      pctxt->buffer.bitOffset = savedBitOff;
      return 0;
   }
   else
#endif /* _NO_STREAM */
   {
      pctxt->buffer.byteIndex = pctxt->buffer.size - (savedBitOff != 8);
      pctxt->buffer.size = savedSize;
      pctxt->buffer.bitOffset = savedBitOff;

      return (pctxt->buffer.byteIndex + (savedBitOff != 8) <= savedSize) ?
         0 : RTERR_ENDOFBUF;
   }
}

