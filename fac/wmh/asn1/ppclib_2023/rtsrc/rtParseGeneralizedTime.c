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

#define LOG_CANON(msg) { \
   rtxErrAddStrParm(pctxt, msg); \
   LOG_RTERRNEW(pctxt, ASN_E_NOTCANON); \
   rtxErrSetNonFatal(pctxt); }


EXTRTMETHOD int rtParseGeneralizedTime2 (OSCTXT *pctxt,
                            const char* value,
                            OSNumDateTime* dateTime,
                            OSBOOL logCanon)
{
   int year, month, day, hour, minute, second, diffhour, diffmin;
   OSREAL fracsec = 0.0, mult = 0.1;
   char tzd;
   const char* p = value;
   OSBOOL canon = (pctxt->flags & (ASN1CANON | ASN1DER) ) != 0;
   OSBOOL notcanon = FALSE;   /* true if value is non-canonical */
   OSBOOL fixMidnight = FALSE;

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

      if ( canon && n < 3 ) {
         if ( logCanon ) LOG_CANON("seconds is required");
         notcanon = TRUE;
      }

      if (*p == '.' || *p == ',') { /* fraction */
         int j;      /* count of fractional digits */

         if ( canon && *p == ',') {
            if ( logCanon ) LOG_CANON("must use '.' for fraction");
            notcanon = TRUE;
         }

         p++;

         /* parse next fraction second */
         for (j = 0; OS_ISDIGIT(*p); j++, p++) {
            fracsec += ((*p - '0') * mult);
            mult /= 10.0;
         }
         if (j == 0)
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);

         if (canon && *(p-1) == '0') {
            if ( logCanon ) LOG_CANON("trailing zeros in fraction disallowed");
            notcanon = TRUE;
         }

         /* convert frational hours/mins to frac seconds */
         if (n == 1) {
            fracsec *= 60;    /* was hours; now is minutes */
            if (fracsec >= 1) {
               minute = (int) fracsec;
               fracsec -= minute;
            }
         }

         if ( n < 3 ) {
            fracsec *= 60;    /* was minutes; now is seconds */
            if (fracsec >= 1) {
               second = (int)fracsec;
               fracsec -= second;
            }
         }
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
         if ( canon ) {
            if (logCanon) LOG_CANON("must use Z");
            notcanon = TRUE;
         }

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
      else if ( canon ) {
         if (logCanon) LOG_CANON("must use Z");
         notcanon = TRUE;
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

   if (dateTime->hour == 24 && dateTime->min == 0 && dateTime->sec == 0)
   {
      /* 240000 is valid for midnight.  Change it to 00, makes sure fields
         are otherwise valid, then add a day. */
      fixMidnight = TRUE;
      dateTime->hour = 0;
      if ( canon ) {
         if (logCanon) LOG_CANON("Must use 00 for midnight");
         notcanon = TRUE;
      }
   }

   if (!rtxDateTimeIsValid (dateTime))
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   if (fixMidnight)
   {
      if (dateTime->day == MAX_DAY_INMONTH(dateTime->year, dateTime->mon))
      {
         dateTime->day = 1;
         if (dateTime->mon == 12) {
            dateTime->mon = 1;
            dateTime->year += 1;
         }
         else dateTime->mon += 1;
      }
      else dateTime->day += 1;
   }

   if (notcanon) return ASN_E_NOTCANON;
   else return 0;
}


EXTRTMETHOD int rtParseGeneralizedTime(OSCTXT *pctxt,
   const char* value,
   OSNumDateTime* dateTime)
{
   int status = rtParseGeneralizedTime2(pctxt, value, dateTime, FALSE);
   if ( status == ASN_E_NOTCANON ) return 0;
   else return status;
}
