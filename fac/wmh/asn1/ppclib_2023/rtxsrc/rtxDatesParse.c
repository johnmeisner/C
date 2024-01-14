/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxDateTime.hh"

/**
 * rtxParseYearString: This function parses a date value from a supplied
 * string and sets the pointed OSNumDateTime to the decoded date value.
 */
EXTRTMETHOD int rtxParseGYearString
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   int stat = 0;
   OSUINT32 i = 0;
   OSUINT32 tmpi = 0;
   const OSUTF8CHAR *cur = inpdata;
   int sign = 1;

   if (inpdata == NULL) return RTERR_NOTINIT;

   if (inpdatalen < 4) return RTERR_INVFORMAT;

   /* parse year taking generally first 4 character
      AB: but it could be more characters, and even with sign '-'! */
   if (*cur == '-') {
      sign = -1;
      cur++;
   }
   for (i = 0; OS_ISDIGIT(*cur); i++) {
      tmpi = tmpi * 10 + (*cur - '0');
      cur++;
   }
   pvalue->year = tmpi * sign;

   /* check the presence of character after CCYY */
   if ((size_t)(cur - inpdata) < inpdatalen) {
      /* check the presence of '-' or '+' after CCYY*/
      stat = rtxParseTimeZone (cur, inpdatalen - (size_t)(cur - inpdata), pvalue);
      if (stat < 0) return stat;
      cur += stat;
   }
   else pvalue->tz_flag = FALSE;

   return (int)(cur-inpdata);
}

EXTRTMETHOD int rtxParseGYearMonthString
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   int stat = 0;
   OSUINT32 i = 0;
   OSUINT32 tmpi = 0;
   const OSUTF8CHAR *cur = inpdata;
   int sign = 1;

   if (inpdata == NULL) return RTERR_NOTINIT;

   if (inpdatalen < 7) return RTERR_INVFORMAT;

   /* parse year taking generally first 4 character
      AB: but it could be more characters, and even with sign '-'! */
   if (*cur == '-') {
      sign = -1;
      cur++;
   }
   for (i = 0; OS_ISDIGIT(*cur); i++) {
      tmpi = tmpi * 10 + (*cur - '0');
      cur++;
   }
   pvalue->year = tmpi * sign;

   /* check the presence of '-' after CCYY*/
   if (*cur != '-') return RTERR_INVFORMAT;

   /* parse month taking next 2 character after '-' */
   cur++;
   PARSE_2_DIGITS(tmpi, cur, stat);

   /* check the parse status of month presence of '-' after CCYY-MM*/
   if (stat != 0) return RTERR_INVFORMAT;
   if(tmpi < 1 || tmpi > 12) return RTERR_BADVALUE;
   pvalue->mon = (OSINT8)tmpi;

   /* check the presence of character after CCYY-MM */
   if ((size_t)(cur - inpdata) < inpdatalen) {
      /* check the presence of '-' or '+' after CCYY-MM */
      stat = rtxParseTimeZone (cur, inpdatalen - (size_t)(cur - inpdata), pvalue);
      if (stat < 0) return stat;
      cur += stat;
   }
   else pvalue->tz_flag = FALSE;

   return (int)(cur-inpdata);
}

/**
 * rtxParseMonthString:
 * This function decodes a time value from a supplied string and set the
 * pointed OSNumDateTime structure to the decoded time value.
 */
EXTRTMETHOD int rtxParseGMonthString
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   int stat = 0;
   OSUINT32 tmpi = 0;
   const OSUTF8CHAR *cur = inpdata;

   if (inpdata == NULL) return RTERR_NOTINIT;

   if (inpdatalen < 4)
      return RTERR_INVFORMAT;

   if (*cur != '-' || *(cur + 1) != '-') /* skip first '--' */
      return RTERR_INVFORMAT;

   /* parse month taking next 2 character after '-' */
   cur += 2;
   PARSE_2_DIGITS(tmpi, cur, stat);

   /* check the parse status of month presence of '-' after CCYY-MM*/
   if (stat != 0) return RTERR_INVFORMAT;
   if (tmpi < 1 || tmpi > 12) return RTERR_BADVALUE;
   pvalue->mon = (OSINT8)tmpi;

   /* check the presence of character after --MM */
   if (((size_t)(cur - inpdata) < inpdatalen) &&
      !(inpdatalen - (size_t)(cur - inpdata) == 2 &&
      *cur == '-' && *(cur + 1) == '-')) {

      /* check the presence of '-' or '+' after --MM */
      stat = rtxParseTimeZone (cur, inpdatalen - (size_t)(cur - inpdata), pvalue);
      if (stat < 0) return stat;
      cur += stat;
   }
   else pvalue->tz_flag = FALSE;

   return (int)(cur-inpdata);
}

EXTRTMETHOD int rtxParseGMonthDayString
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   int stat = 0;
   OSUINT32 tmpi = 0;
   OSUINT32 daysInMonth;
   const OSUTF8CHAR *cur = inpdata;

   if (inpdata == NULL) return RTERR_NOTINIT;

   if (inpdatalen < 7)
      return RTERR_INVFORMAT;

   if (*cur != '-' || *(cur + 1) != '-') /* skip first '--' */
      return RTERR_INVFORMAT;

   /* parse month taking next 2 character after '-' */
   cur += 2;
   PARSE_2_DIGITS(tmpi, cur, stat);

   /* check the parse status of month presence of '-' after CCYY-MM*/
   if (stat != 0 || *cur != '-') return RTERR_INVFORMAT;
   if (tmpi < 1 || tmpi > 12) return RTERR_BADVALUE;
   pvalue->mon = (OSINT8)tmpi;

   /* parse day taking next 2 character after '-' */
   cur++;
   PARSE_2_DIGITS(tmpi, cur, stat);

   /* check the parse status of day and presence of '-' after CCYY-MM-DD*/
   if (stat != 0) return RTERR_INVFORMAT;

   daysInMonth = gDaysInMonth[pvalue->mon - 1];
   if (pvalue->mon == 2) {
      daysInMonth++;
   }
   if (tmpi < 1 || tmpi > daysInMonth) return RTERR_BADVALUE;
   pvalue->day = (OSINT8)tmpi;

   /* check the presence of character after --MM-DD */
   if ((size_t)(cur - inpdata) < inpdatalen) {
      /* check the presence of '-' or '+' after --MM-DD */
      stat = rtxParseTimeZone (cur, inpdatalen - (size_t)(cur - inpdata), pvalue);
      if (stat < 0) return stat;
      cur += stat;
   }
   else pvalue->tz_flag = FALSE;

   return (int)(cur-inpdata);
}

EXTRTMETHOD int rtxParseGDayString
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   int stat = 0;
   OSUINT32 tmpi = 0;
   const OSUTF8CHAR *cur = inpdata;

   if (inpdata == NULL) return RTERR_NOTINIT;

   if (inpdatalen < 5)
      return RTERR_INVFORMAT;

   /* skip first '---' */
   if (*cur != '-' || *(cur + 1) != '-' || *(cur + 2) != '-')
      return RTERR_INVFORMAT;

   /* parse day taking next 2 character after '-' */
   cur += 3;
   PARSE_2_DIGITS(tmpi, cur, stat);

   /* check the parse status of day and presence of '-' after CCYY-MM-DD*/
   if (stat != 0) return RTERR_INVFORMAT;

   if (tmpi < 1 || tmpi > 31) return RTERR_BADVALUE;
   pvalue->day = (OSINT8)tmpi;

   /* check the presence of character after ---DD */
   if ((size_t)(cur - inpdata) < inpdatalen) {
      /* check the presence of '-' or '+' after ---DD */
      stat = rtxParseTimeZone (cur, inpdatalen - (size_t)(cur - inpdata), pvalue);
      if (stat < 0) return stat;
      cur += stat;
   }
   else pvalue->tz_flag = FALSE;

   return (int)(cur-inpdata);
}

