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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxDateTime.hh"

EXTPERMETHOD int pe_TimeDiff (OSCTXT* pctxt, const char* string)
{
   const char* p = string;
   OSINT32 hours = 0;
   OSUINT32 minutes = 0;
   OSBOOL minutesPresents = FALSE;
   OSBOOL hoursNegative = FALSE;
   int stat = 0;

   /* Parse date string. The string is expected to be in sHH:MM format. */

   if (*p == '-')
      hoursNegative = TRUE;
   else if (*p != '+')
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   p++;

   PARSE_2_DIGITS (hours, p, stat);
   if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   if (hours > 15) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   if (*p++ == ':') {
      PARSE_2_DIGITS (minutes, p, stat);
      if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      if (minutes > 0)
         minutesPresents = TRUE;

      if (minutes > 59)
         return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   PU_PUSHNAME (pctxt, "timeDiff");

   PU_NEWFIELD (pctxt, "minutesPresent");
   stat = pe_bit (pctxt, minutesPresents);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_SETBITCOUNT (pctxt);

   PU_PUSHNAME (pctxt, "sign");
   stat = pe_ConsWholeNumber (pctxt, hoursNegative, 2);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   PU_PUSHNAME (pctxt, "hours");
   stat = pe_ConsInteger (pctxt, hours, 0, 15);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   if (minutesPresents) {
      PU_PUSHNAME (pctxt, "minutes");
      stat = pe_ConsUnsigned (pctxt, minutes, OSUINTCONST(1), OSUINTCONST(59));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   PU_POPNAME (pctxt);

   return (int) (p - string);
}
