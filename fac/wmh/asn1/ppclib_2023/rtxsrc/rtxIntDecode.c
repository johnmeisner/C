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

#include "rtxsrc/rtxBitDecode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxIntDecode.h"

/* Decode an 8-bit signed integer */

EXTRTMETHOD int rtxDecInt8(OSCTXT* pctxt, OSINT8* pvalue)
{
   if (0 == pvalue) {
      return rtxSkipBytes(pctxt, 1);
   }
   else if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      return rtxReadBytes (pctxt, (OSOCTET*)pvalue, 1);
   }
   else {
      return rtxDecBitsToByteArray (pctxt, (OSOCTET*)pvalue, 1, 8);
   }
}

/* Decode a 16-bit signed integer */

#define RTXDECINTFN rtxDecInt16
#define RTXDECINTTYPE OSINT16
#include "rtxIntDecTmpl.c"

/* Decode a 32-bit signed integer */

#define RTXDECINTFN rtxDecInt32
#define RTXDECINTTYPE OSINT32
#include "rtxIntDecTmpl.c"

/* Decode a 64-bit signed integer */

#define RTXDECINTFN rtxDecInt64
#define RTXDECINTTYPE OSINT64
#include "rtxIntDecTmpl.c"

/* Decode an 8-bit unsigned integer */

EXTRTMETHOD int rtxDecUInt8(OSCTXT* pctxt, OSUINT8* pvalue)
{
   OSUINT8 tmpbuf;
   if (0 == pvalue) pvalue = &tmpbuf;
   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      return rtxReadBytes (pctxt, pvalue, 1);
   }
   else {
      return rtxDecBitsToByteArray (pctxt, pvalue, 1, 8);
   }
}

/* Decode a 16-bit unsigned integer */

#define RTXDECUINTFN rtxDecUInt16
#define RTXDECUINTTYPE OSUINT16
#include "rtxUIntDecTmpl.c"

/* Decode a 32-bit unsigned integer */

#define RTXDECUINTFN rtxDecUInt32
#define RTXDECUINTTYPE OSUINT32
#include "rtxUIntDecTmpl.c"

/* Decode a 64-bit signed integer */

#define RTXDECUINTFN rtxDecUInt64
#define RTXDECUINTTYPE OSUINT64
#include "rtxUIntDecTmpl.c"
