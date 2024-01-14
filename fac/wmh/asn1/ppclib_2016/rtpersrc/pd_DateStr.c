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

#include <stdio.h>

static const OSINT32 yearLimit[] = {
   OSINTCONST(99999), OSINTCONST(999999), OSINTCONST(99999999),
   OSINTCONST(999999999), OSINTCONST(999999999)
};

int pd_DateToStrn (OSCTXT* pctxt, char* string, size_t strSz, OSUINT32 flags)
{
   OSINT32 year = 0;
   OSUINT8 mw = 0; /* month or week */
   OSUINT32 day = 0;
   int yearDigits = 0;
   int stat;

   char* p = string;

   /* decode date */
   if (flags & OSYEAR) {
      PU_PUSHNAME (pctxt, "year");
      if (flags & OSANY_MASK)
         stat = pd_UnconsInteger (pctxt, &year);
      else
         stat = pd_YearInt (pctxt, &year);

      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }
   else if (flags & OSCENTURY) {
      PU_PUSHNAME (pctxt, "century");
      if (flags & OSANY_MASK)
         stat = pd_UnconsInteger (pctxt, &year);
      else
         stat = pd_ConsInteger (pctxt, &year, 0, 99);

      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & (OSMONTH | OSWEEK)) {
      OSUINT32 maxMW = (flags & OSMONTH) ? 12 : 53;

      PU_PUSHNAME (pctxt, (flags & OSWEEK) ? "week" : "month");
      stat = pd_ConsUInt8 (pctxt, &mw, OSUINTCONST(1), maxMW);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSDAY) {
      OSUINT32 maxDay = 366;

      if (flags & OSMONTH)
         maxDay = 31;
      else if (flags & OSWEEK)
         maxDay = 7;

      PU_PUSHNAME (pctxt, "day");
      stat = pd_ConsUnsigned (pctxt, &day, OSUINTCONST(1), maxDay);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & (OSCENTURY | OSYEAR)) {
      OSINT32 yearTm = year;
      int ln = (int) (flags >> 28);
      OSINT32 yearLim = 0;

      if (ln > 0) {
         if (ln < 5 || ln > 9)
            return LOG_RTERR (pctxt, RTERR_INVPARAM);

         yearLim = yearLimit[ln - 5];
      }

      /* set default basic and proleptic year */
      if ((flags & OSYEAR_MASK) == 0)
         flags |= (OSYEAR_BASIC | OSYEAR_PROLEPTIC);

      /* check range */
      if (flags & OSCENTURY)
         yearTm = yearTm * 100 + 99; /* convert cc -> cc99 */

      yearDigits = 4;

      if ((flags & OSYEAR_BASIC) && yearTm >= 1582 && yearTm <= 9999)
         ; /* OK */
      else if ((flags & OSYEAR_PROLEPTIC) && yearTm >= 0 && yearTm <= 1581)
         ; /* OK */
      else if ((flags & OSYEAR_NEGATIVE) && yearTm >= -9999 && yearTm <= -1)
         ; /* OK */
      else if (ln > 0 && yearTm >= -yearLim && yearTm <= yearLim)
         yearDigits = ln; /* OK */
      else
         return LOG_RTERR (pctxt, RTERR_BADVALUE);

      if (flags & OSCENTURY)
         yearDigits -= 2;
   }

   /* fill date string */
   if (flags & OSANY_MASK) {
      if (year < 0) {
         *p++ = '-';
         year = -year;
      }
      else
         *p++ = '+';

      stat = rtxIntToCharStr (year, p, (size_t) yearDigits + 1, '0');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      p += yearDigits;

      if (flags & OSCENTURY)
         *p++ = 'C';
   }
   else if (flags & OSYEAR) {
      stat = rtxIntToCharStr (year, p, 5, '0');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      p += 4;
   }
   else if (flags & OSCENTURY) {
      *p++ = (char) (year / 10 + '0');
      *p++ = (char) (year % 10 + '0');
      *p++ = 'C';
   }
   else {
      *p++ = '-';
   }

   if (flags & (OSMONTH | OSWEEK)) {
      *p++ = '-';

      if (flags & OSWEEK)
         *p++ = 'W';

      *p++ = (char) (mw / 10 + '0');
      *p++ = (char) (mw % 10 + '0');
   }

   if (flags & OSDAY) {
      *p++ = '-';

      if (flags & OSMONTH) {
         *p++ = (char) (day / 10 + '0');
         *p++ = (char) (day % 10 + '0');
      }
      else if (flags & OSWEEK) {
         *p++ = (char) (day + '0');
      }
      else {
         *p++ = (char) (day / 100 + '0');
         *p++ = (char) ((day % 100) / 10 + '0');
         *p++ = (char) (day % 10 + '0');
      }
   }

   *p = 0;

   return (int) (p - string);
}

EXTPERMETHOD int pd_DateStr
(OSCTXT* pctxt, const char** string, OSUINT32 flags)
{
   char* datestr;
   size_t strSz = pu_GetDateTimeStrSize (flags) + 1;
   int stat;

   /* create date string */
   datestr = (char*) rtxMemAlloc (pctxt, strSz);
   if (0 == datestr) return RTERR_NOMEM;

   stat = pd_DateToStrn (pctxt, datestr, strSz, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   *string = datestr;

   return 0;
}
