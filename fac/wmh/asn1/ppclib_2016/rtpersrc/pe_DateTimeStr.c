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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxDateTime.hh"

EXTPERMETHOD int pe_DateTimeStr
(OSCTXT* pctxt, const char* string, OSUINT32 flags)
{
   int stat;

   stat = pe_DateFromStr (pctxt, string, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (string[stat] != 'T')
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   stat = pe_TimeFromStr (pctxt, string + stat + 1, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
