/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include "asn1mder.h"
#include "rtxsrc/rtxStream.h"

int mderEncInt32 (OSCTXT *pctxt, OSINT32 value)
{
   int stat;
   OSOCTET b[4];

   b[0] = (value & 0xFF000000) >> 24;
   b[1] = (value & 0x00FF0000) >> 16;
   b[2] = (value & 0x0000FF00) >> 8;
   b[3] = (value & 0x000000FF);

   stat = ME_SAFEPUT4 (pctxt, b);
   if (stat != 0) return LOG_RTERR(pctxt, stat);

   return 0;
}
