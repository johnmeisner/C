/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxBigInt.h"
#include <math.h>

int oerDecBigIntImpl (OSCTXT* pctxt, const char** ppvalue, int radix,
                        OSBOOL isSigned)
{
   int stat = 0;
   OSBigInt bigInt;
   OSOCTET* tmpBuf;
   OSBOOL tmbBufAllocated = FALSE;
   OSSIZE len;
   OSBOOL bNonCanonical = FALSE;
   char* pvalue;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON) {
      bNonCanonical = TRUE;
      stat = 0;
   }
   else if (stat < 0) LOG_RTERR (pctxt, stat);

   stat = rtxReadBytesDynamic (pctxt, &tmpBuf, len, &tmbBufAllocated);
   if (stat < 0) {
      if (tmbBufAllocated) rtxMemFreePtr(pctxt, tmpBuf);
      return LOG_RTERR (pctxt, stat);
   }

   rtxBigIntInit(&bigInt);
   if (isSigned) {
      /* If first byte is a sign byte that is not needed, the encoding is
         not canonical */
      bNonCanonical = bNonCanonical ||
               ( len > 1 && ( (tmpBuf[0] == 0xFF && tmpBuf[1] >= 0x80) ||
                        (tmpBuf[0] == 0x00 && tmpBuf[1] < 0x80) ) );
      stat = rtxBigIntSetBytesSigned(pctxt, &bigInt, tmpBuf, len);
   }
   else {
      /* If first byte is a zero byte that is not needed, the encoding is
         not canonical */
      bNonCanonical = bNonCanonical || ((len > 1) && (tmpBuf[0] == 0x00));

      stat = rtxBigIntSetBytesUnsigned(pctxt, &bigInt, tmpBuf, len);
   }

   if (tmbBufAllocated) rtxMemFreePtr(pctxt, tmpBuf);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   // Alloc string buffer, use binary as the longest string
   len = rtxBigIntDigitsNum(&bigInt, radix) + 1;
   *ppvalue = pvalue = (char*) rtxMemAlloc (pctxt, len);
   if (0 == *ppvalue) return LOG_RTERR (pctxt, RTERR_NOMEM);

   stat = rtxBigIntToString(pctxt, &bigInt, radix, pvalue, len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   //assert: stat >= 0, so we aren't hiding an error if returning ASN_E_NOTCANON
   return bNonCanonical ? ASN_E_NOTCANON : stat;
}

EXTOERMETHOD int oerDecBigUInt (OSCTXT* pctxt, const char** ppvalue, int radix)
{
   return oerDecBigIntImpl(pctxt, ppvalue, radix, FALSE);
}

EXTOERMETHOD int oerDecBigInt (OSCTXT* pctxt, const char** ppvalue, int radix)
{
   return oerDecBigIntImpl(pctxt, ppvalue, radix, TRUE);
}
