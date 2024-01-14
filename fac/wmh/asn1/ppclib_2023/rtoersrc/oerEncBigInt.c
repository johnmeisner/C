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

EXTOERMETHOD int oerEncBigIntValue (OSCTXT* pctxt, struct OSBigInt* pvalue)
{
   OSSIZE bufsize;
   OSOCTET* buffer = 0;
   int ret;

   //get signed representation of big int
   bufsize = rtxBigIntGetDataLen(pvalue);
   buffer = rtxMemAlloc(pctxt, bufsize);
   if ( buffer != 0 ) {
      rtxBigIntGetData (pctxt, pvalue, buffer, bufsize);
   }
   else {
      return RTERR_NOMEM;
   }

   //encode length of signed representation
   ret = oerEncLen (pctxt, bufsize);
   if ( ret < 0 ) LOG_RTERR(pctxt, ret);

   //encode signed representation
   if (0 == ret) {
      ret = rtxWriteBytes(pctxt, buffer, bufsize);
      if ( ret < 0 ) LOG_RTERR(pctxt, ret);
   }

   if ( buffer != 0 ) rtxMemFreePtr(pctxt, buffer);

   return ret;
}

EXTOERMETHOD int oerEncBigInt (OSCTXT* pctxt, const char* pvalue, int radix)
{
   int ret = 0;
   OSBigInt bigInt;

   rtxBigIntInit(&bigInt);

   //convert string to big num
   ret = rtxBigIntSetStr(pctxt, &bigInt, pvalue, radix);

   if ( ret >= 0 ) {
      ret = oerEncBigIntValue (pctxt, &bigInt);
   }

   rtxBigIntFree(pctxt, &bigInt);

   return ret;
}
