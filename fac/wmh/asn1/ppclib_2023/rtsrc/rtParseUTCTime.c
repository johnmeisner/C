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

#define LOG_CANON(msg) { \
   rtxErrAddStrParm(pctxt, msg); \
   LOG_RTERRNEW(pctxt, ASN_E_NOTCANON); \
   rtxErrSetNonFatal(pctxt); }

EXTRTMETHOD int rtParseUTCTime2
(OSCTXT *pctxt, const char* value, OSNumDateTime* dateTime, OSBOOL logCanon)
{
   int year, month, day, hour, minute, second, diffhour, diffmin;
   char tzd;
   const char* p = value;
   OSBOOL canon = (pctxt->flags & (ASN1CANON | ASN1DER)) != 0;
   OSBOOL notcanon = FALSE;   /* true if value is non-canonical */
   OSBOOL fixMidnight = FALSE;

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
         else if ( canon ) {
            if ( logCanon ) LOG_CANON("seconds required");
            notcanon = TRUE;
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
            if ( canon ) {
               if (logCanon) LOG_CANON ("must use Z");
               notcanon = TRUE;
            }

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

   if (dateTime->hour == 24 && dateTime->min == 0 && dateTime->sec == 0)
   {
      /* 240000 is valid for midnight.  Change it to 00, makes sure fields
      are otherwise valid, then add a day. */
      fixMidnight = TRUE;
      dateTime->hour = 0;
      if (canon) {
         if (logCanon) LOG_CANON("Must use 00 for midnight");
         notcanon = TRUE;
      }
   }

   if (!rtxDateTimeIsValid(dateTime))
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

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

EXTRTMETHOD int rtParseUTCTime
(OSCTXT *pctxt, const char* value, OSNumDateTime* dateTime)
{
   int status = rtParseUTCTime2(pctxt, value, dateTime, FALSE);
   if ( status == ASN_E_NOTCANON ) return 0;
   else return status;
}
