/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "rtxText.h"

/* These defs allow this function to be used as a template for other
   text functions writing integer values (int64, unsigned int, etc.) */
#ifndef RTTXTWRITEINTFUNC
#define RTTXTWRITEINTFUNC rtxTxtWriteInt
#define RTXINTTOCHARSTR rtxIntToCharStr
#define OSINTTYPE OSINT32
#endif

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxError.h"


EXTRTMETHOD int RTTXTWRITEINTFUNC (OSCTXT* pctxt, OSINTTYPE value)
{
   int stat;
   char lbuf[40];

   stat = RTXINTTOCHARSTR (value, lbuf, sizeof(lbuf), 0);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   stat = rtxCopyAsciiText (pctxt, lbuf);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
