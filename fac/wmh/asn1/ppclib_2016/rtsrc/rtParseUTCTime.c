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

#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDateTime.hh"

static int digitCharsToInt (const char* digits, OSSIZE ndigits, int* pvalue)
{
   OSSIZE i;
   *pvalue = 0;
   for (i = 0; i < ndigits; i++) {
      if (OS_ISDIGIT(digits[i])) *pvalue = *pvalue * 10 + (digits[i] - '0');
      else return RTERR_BADVALUE;
   }
   return 0; 
}

static int parseYYMMDD (const char* str, int* pyear, int* pmonth, int* pday)
{
   int ret;
   *pyear = *pmonth = *pday = 0;
   ret = digitCharsToInt (str, 2, pyear);
   if (0 == ret) {
      str += 2;
      ret = digitCharsToInt (str, 2, pmonth);
   }
   if (0 == ret) {
      str += 2;
      ret = digitCharsToInt (str, 2, pday);
   }
   return ret;
}

EXTRTMETHOD int rtParseUTCTime
(OSCTXT *pctxt, const char* value, OSNumDateTime* dateTime)
{
   int year, month, day, hour, minute, second, diffhour, diffmin;
   char tzd;
   const char* p = value;

   year = month = day = hour = minute = RTERR_IDNOTFOU;
   second = diffhour = diffmin = 0;

   if (parseYYMMDD (p, &year, &month, &day) == 0) {
      int n = 0;
      p += 6;

      if (year < 0)
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);

      if (year < 100) {
         if (year >= 50)
            year += 1900;
         else
            year += 2000;
      }

      if (OS_ISDIGIT(*p) && digitCharsToInt(p, 2, &hour) == 0) {
         p += 2;
         n++;
      }
      else
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);

      if (OS_ISDIGIT(*p) && digitCharsToInt(p, 2, &minute) == 0) {
         p += 2;
         n++;
         if (OS_ISDIGIT(*p) && digitCharsToInt(p, 2, &second) == 0) {
            p += 2;
            n++;
         }
      }
      else
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);

      dateTime->tz_flag = FALSE;
      tzd = *p;
      if (tzd == 'Z') { /* utc | GMT */
         dateTime->tz_flag = TRUE;
         if (*++p != 0)
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else {
         /* DER/CER/PER time should be terminated with 'Z'!
             else return LOG_RTERR(pctxt, RTERR_INVFORMAT); */
         if (tzd == '-' || tzd == '+') {
            dateTime->tz_flag = TRUE;
            p++;
            if(!OS_ISDIGIT(*p) || digitCharsToInt(p, 2, &diffhour) != 0)
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
            p += 2;
            if(!OS_ISDIGIT(*p) || digitCharsToInt(p, 2, &diffmin) != 0)
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
            if(diffhour < 0 || diffhour > 13)
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
            if(diffmin < 0 || diffmin > 59)
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
         else {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
   }
   else
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

   dateTime->year = (OSINT32)year;
   dateTime->mon = (OSUINT8)month;
   dateTime->day = (OSUINT8)day;
   dateTime->hour = (OSUINT8)hour;
   dateTime->min = (OSUINT8)minute;
   dateTime->sec = (OSREAL)second;
   dateTime->tzo = (diffhour * 60) + diffmin;
   if (tzd == '-') dateTime->tzo *= -1;

   if (!rtxDateTimeIsValid (dateTime))
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   return 0;
}
