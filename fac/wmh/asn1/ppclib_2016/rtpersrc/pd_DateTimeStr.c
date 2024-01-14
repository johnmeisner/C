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
#include "rtxsrc/rtxCharStr.h"

EXTPERMETHOD int pd_DateTimeStr
(OSCTXT* pctxt, const char** string, OSUINT32 flags)
{
   char* str;
   size_t strSz = pu_GetDateTimeStrSize (flags);
   int stat;

   /* create datetime string */
   str = (char*) rtxMemAlloc (pctxt, strSz + 1);
   if (0 == str) return LOG_RTERR (pctxt, RTERR_NOMEM);

   stat = pd_DateToStrn (pctxt, str, strSz, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   str[stat] = 'T';

   stat = pd_TimeToStrn (pctxt, str + stat + 1, strSz - stat - 1, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   *string = str;

   return 0;
}
