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

int pe_DateFromStr (OSCTXT* pctxt, const char* string, OSUINT32 flags)
{
   const char* p = string;
   OSINT32 year = 0;
   OSUINT32 mw = 0; /* month or week */
   OSUINT32 day = 0;
   OSUINT32 maxDay = 0;
   OSUINT32 maxMW = 0;
   int yearDigits = 0;
   int stat = 0;

   /* Parse date string. */
   if (flags & OSANY_MASK) {
      OSBOOL yearNegative = FALSE;

      if (*p == '-')
         yearNegative = TRUE;
      else if (*p != '+')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      p++;

      while (OS_ISDIGIT (*p)) {
         year = year * 10 + (OSINT32) (*p++ - '0');
         yearDigits++;
      }

      if (yearNegative)
         year = -year;
   }
   else if (flags & (OSCENTURY | OSYEAR)) {
      int nmDigits = yearDigits = (flags & OSYEAR) ? 4 : 2;

      while (nmDigits--) {
         if (OS_ISDIGIT (*p))
            year = year * 10 + (OSINT32) (*p++ - '0');
         else
            return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      }
   }

   if (flags & (OSCENTURY | OSYEAR)) {
      OSINT32 yearTm = year;

      int ln = (int) (flags >> 28);
      if (ln > 0 && (ln < 5 || ln > 9))
         return LOG_RTERR (pctxt, RTERR_INVPARAM);

      /* set default basic and proleptic year */
      if ((flags & OSYEAR_MASK) == 0)
         flags |= (OSYEAR_BASIC | OSYEAR_PROLEPTIC);

      /* check range */
      if (flags & OSCENTURY) {
         yearDigits += 2;
         yearTm = yearTm * 100 + 99; /* convert cc -> cc99 */
      }

      if ((flags & OSYEAR_BASIC) && yearDigits == 4 &&
          yearTm >= 1582 && yearTm <= 9999)
         ; /* OK */
      else if ((flags & OSYEAR_PROLEPTIC) && yearDigits == 4 &&
               yearTm >= 0 && yearTm <= 1581)
         ; /* OK */
      else if ((flags & OSYEAR_NEGATIVE) && yearDigits == 4 &&
               yearTm >= -9999 && yearTm <= -1)
         ; /* OK */
      else if (ln > 0 && yearDigits == ln)
         ; /* OK */
      else
         return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   if ((flags & OSCENTURY) && *p++ != 'C')
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   if (flags & (OSMONTH | OSWEEK)) {
      maxMW = (flags & OSMONTH) ? 12 : 53;

      if (*p++ != '-')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      if ((flags & OSWEEK) && *p++ != 'W')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      PARSE_2_DIGITS (mw, p, stat);
      if (stat != 0) return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      else if (mw < 1 || mw > maxMW) return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   if (flags & OSDAY) {
      int nmDigits = 3;
      maxDay = 366;

      if (flags & OSMONTH) {
         maxDay = 31;
         nmDigits = 2;
      }
      else if (flags & OSWEEK) {
         maxDay = 7;
         nmDigits = 1;
      }

      if (*p++ != '-')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      while (nmDigits--) {
         if (OS_ISDIGIT (*p))
            day = day * 10 + (OSUINT32) (*p++ - '0');
         else
            return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      }
   }

   /* Encode component values */
   PU_PUSHNAME (pctxt, (flags & OSCENTURY) ? "century" : "year");

   if (flags & OSANY_MASK)
      stat = pe_UnconsInteger (pctxt, year);
   else if (flags & OSCENTURY)
      stat = pe_ConsInteger (pctxt, year, 0, 99);
   else if (flags & OSYEAR)
      stat = pe_YearInt (pctxt, year);

   PU_POPNAME (pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (flags & (OSMONTH | OSWEEK)) {
      PU_PUSHNAME (pctxt, (flags & OSWEEK) ? "week" : "month");
      stat = pe_ConsUnsigned (pctxt, mw, OSUINTCONST(1), maxMW);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSDAY) {
      PU_PUSHNAME (pctxt, "day");
      stat = pe_ConsUnsigned (pctxt, day, OSUINTCONST(1), maxDay);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   return (int) (p - string);
}

EXTPERMETHOD int pe_DateStr
(OSCTXT* pctxt, const char* string, OSUINT32 flags)
{
   int stat = pe_DateFromStr (pctxt, string, flags);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
