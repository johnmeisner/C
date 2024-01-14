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

#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlDecBase64StrValue (OSCTXT* pctxt,
                            OSOCTET* pvalue, OSUINT32* pnocts,
                            size_t bufSize, size_t srcDataLen)
{
   if ( pnocts )
   {
      OSSIZE nocts;
      int ret;

      ret = rtXmlDecBase64StrValue64(pctxt, pvalue, &nocts, bufSize,
                                       srcDataLen);
      if ( ret != 0 ) return LOG_RTERR(pctxt, ret);

      if ( nocts > OSUINT32_MAX )
         return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

      *pnocts = (OSUINT32) nocts;
      return 0;
   }
   else {
      return rtXmlDecBase64StrValue64(pctxt, pvalue, 0, bufSize, srcDataLen);
   }

}

EXTXMLMETHOD int rtXmlDecBase64StrValue64 (OSCTXT* pctxt,
                            OSOCTET* pvalue, OSSIZE* pnocts,
                            size_t bufSize, size_t srcDataLen)
{
   const OSUTF8CHAR* inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   OSUINT32 i;
   OSBOOL pad = FALSE;
   int prevVal = 0, itmp = 0;
   OSOCTET* end = pvalue + bufSize;
   OSOCTET* start = pvalue;

   /* Translate all full groups from base64 to byte array elements */
   for (i = 0; i < srcDataLen; i++) {
      int val;

      while (OS_ISSPACE(*inpdata)) inpdata++;

      if (*inpdata == '=') {
         if (i < srcDataLen - 2)
            return LOG_RTERRNEW (pctxt, RTERR_INVBASE64);

         inpdata++;
         pad = TRUE;
         continue;
      }

      if (pad)
         return LOG_RTERRNEW (pctxt, RTERR_INVBASE64);

      val = BASE64TOINT(*inpdata);
      inpdata++;

      if (val < 0)
         return LOG_RTERRNEW (pctxt, RTERR_INVBASE64);

      if (pvalue >= end)
         return LOG_RTERRNEW (pctxt, RTERR_STROVFLW);

      switch (i & 3) {
      case 0:
         itmp = (val << 2);
         break;
      case 1:
         itmp |= (val >> 4);
         break;
      case 2:
         itmp = ((prevVal & 0x0f) << 4) | (val >> 2);
         break;
      case 3:
         itmp = ((prevVal & 0x03) << 6) | (val);
         break;
      default:
         itmp = 0; /* to keep VC++ -W4 happy */
      }

      if (itmp >= 0 && itmp <= 255) {
         *pvalue = (OSOCTET)itmp;
         if ((i & 3) != 0) pvalue++;
      }
      else
         return LOG_RTERRNEW (pctxt, RTERR_INVBASE64);

      prevVal = val;
   }

   if (pnocts != 0) *pnocts = (OSUINT32)(pvalue - start);

   return 0;
}


