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

int pe_TimeFromStr (OSCTXT* pctxt, const char* string, OSUINT32 flags)
{
   const char* p = string;
   OSUINT32 hours = 0;
   OSUINT32 minutes = 0;
   OSUINT32 seconds = 0;
   OSUINT32 fraction = 0;
   int stat = 0;

   /* Parse time string. */

   PARSE_2_DIGITS (hours, p, stat);
   if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);
   else if (hours > 24) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   if (flags & OSMINUTES) {
      if (*p++ != ':')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      PARSE_2_DIGITS (minutes, p, stat);
      if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      else if (minutes > 59) return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   if (flags & OSSECONDS) {
      if (*p++ != ':')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      PARSE_2_DIGITS (seconds, p, stat);
      if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      else if (minutes > 60) return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   if (flags & OSFRACTION) {
      unsigned fracDigits = (flags & OSFRACTION);

      if (*p != '.' && *p != ',')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      p++;

      while (OS_ISDIGIT (*p) && fracDigits) {
         fraction = fraction * 10 + (OSUINT32) (*p++ - '0');
         fracDigits--;
      }

      if (fracDigits)
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);
   }

   if (flags & OSUTC) {
      if (*p++ != 'Z')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);
   }

   /* Encode component values */
   PU_PUSHNAME (pctxt, "hours");
   stat = pe_ConsUnsigned (pctxt, hours, OSUINTCONST(0), OSUINTCONST(24));
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   if (flags & OSMINUTES) {
      PU_PUSHNAME (pctxt, "minutes");
      stat = pe_ConsUnsigned (pctxt, minutes, OSUINTCONST(0), OSUINTCONST(59));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSSECONDS) {
      PU_PUSHNAME (pctxt, "seconds");
      stat = pe_ConsUnsigned (pctxt, seconds, OSUINTCONST(0), OSUINTCONST(60));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSFRACTION) {
      OSBOOL extbit = (OSBOOL) (fraction > 999);

      PU_PUSHNAME (pctxt, "fraction");
      PU_NEWFIELD (pctxt, "extension marker");
      stat = pe_bit (pctxt, extbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);

      if (extbit)
         stat = pe_UnconsUnsigned (pctxt, fraction);
      else
         stat = pe_ConsUnsigned (pctxt, fraction, OSUINTCONST(0),
                                 OSUINTCONST(999));

      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSDIFF) {
      stat = pe_TimeDiff (pctxt, p);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      p += stat;
   }

   return (int) (p - string);
}

EXTPERMETHOD int pe_TimeStr
(OSCTXT* pctxt, const char* string, OSUINT32 flags)
{
   int stat = pe_TimeFromStr (pctxt, string, flags);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
