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

static int parseYYYYMMDD (const char* str, int* pyear, int* pmonth, int* pday)
{
   int ret;
   *pyear = *pmonth = *pday = 0;
   ret = digitCharsToInt (str, 4, pyear);
   if (0 == ret) {
      str += 4;
      ret = digitCharsToInt (str, 2, pmonth);
   }
   if (0 == ret) {
      str += 2;
      ret = digitCharsToInt (str, 2, pday);
   }
   return ret;
}

EXTRTMETHOD int rtParseGeneralizedTime (OSCTXT *pctxt,
                            const char* value,
                            OSNumDateTime* dateTime)
{
   int year, month, day, hour, minute, second, diffhour, diffmin;
   OSREAL fracsec = 0.0, mult = 0.1;
   char tzd;
   const char* p = value;

   year = month = day = hour = RTERR_IDNOTFOU;
   minute = second = diffhour = diffmin = 0;

   if (parseYYYYMMDD (p, &year, &month, &day) == 0) {
      int n = 0;
      p += 8;

      if (OS_ISDIGIT(*p) && 0 == digitCharsToInt (p, 2, &hour)) {
         p += 2;
         n++;
      }
      else
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);

      if (OS_ISDIGIT(*p) && 0 == digitCharsToInt (p, 2, &minute)) {
         p += 2;
         n++;
         if(OS_ISDIGIT(*p) && 0 == digitCharsToInt (p, 2, &second)) {
            p += 2;
            n++;
         }
      }

      if (*p == '.' || *p == ',') { /* fraction */
         int j;
         p++;

         if(n < 3) /* fraction second can be present if second is present */
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);

         /* parse next fraction second */
         for (j = 0; OS_ISDIGIT(*p); j++, p++) {
            fracsec += ((*p - '0') * mult);
            mult /= 10.0;
         }
         if (j == 0)
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      dateTime->tz_flag = FALSE;
      tzd = *p;
      if (tzd == 'Z') { /* utc */
         dateTime->tz_flag = TRUE;
         p++;
         if(*p != 0)
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else if (tzd == '-' || tzd == '+') {
         dateTime->tz_flag = TRUE;
         p++;
         if(!OS_ISDIGIT(*p) || 0 != digitCharsToInt (p, 2, &diffhour))
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         p += 2;
         if(diffhour < 0 || diffhour > 13)
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         if(OS_ISDIGIT(*p)) {
            if(digitCharsToInt (p, 2, &diffmin) != 0)
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
            if(diffmin < 0 || diffmin > 59)
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
   dateTime->sec = (OSREAL)second + fracsec;
   dateTime->tzo = (diffhour * 60) + diffmin;
   if (tzd == '-') dateTime->tzo *= -1;

   if (!rtxDateTimeIsValid (dateTime))
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   return 0;
}
