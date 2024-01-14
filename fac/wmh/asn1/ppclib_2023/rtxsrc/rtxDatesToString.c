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

#include <stdio.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDateTime.hh"

EXTRTMETHOD int rtxGYearToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat, len = 0;
   char* const str = (char*)buffer;

   if (bufsize < 5) return RTERR_OUTOFBND;
   *str = 0;

   len += rtxIntToCharStr (pvalue->year, str, 5, 0);

   if (pvalue->tz_flag == (OSBOOL)TRUE) {
      stat = rtxTimeZoneToString
         (pvalue, buffer + len, bufsize - len);
      if (stat < 0) return stat;
      len += stat;
   }

   return len;
}

EXTRTMETHOD int rtxGYearMonthToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat, len = 0;
   char* str = (char*)buffer;

   if (bufsize < 8) return RTERR_OUTOFBND;
   *str = 0;

   len += rtxIntToCharStr (pvalue->year, str, bufsize, 0);
   rtxStrcat (str, bufsize, "-"); len++;

   if (pvalue->mon < 1 || pvalue->mon > 12) {
      /*add "\?\?" to tmpStr if mon is is out-of-range*/
      rtxStrcat (str, bufsize, "\?\?");
      return RTERR_INVFORMAT;
   }
   else {
      len += rtxIntToCharStr (pvalue->mon, str + len, 3, '0');
   }

   if (pvalue->tz_flag == (OSBOOL)TRUE) {
      stat = rtxTimeZoneToString
         (pvalue, buffer + len, bufsize - len);
      if (stat < 0) return stat;
      len += stat;
   }

   return len;
}

EXTRTMETHOD int rtxGMonthToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat, len = 0;
   char* str = (char*)buffer;

   if (bufsize < 5) return RTERR_OUTOFBND;
   *str = 0;

   if (pvalue->mon < 1 || pvalue->mon > 12) {
      /*add "\?\?" to tmpStr if mon is is out-of-range*/
      rtxStrcpy (str, 5, "--\?\?");
      return RTERR_INVFORMAT;
   }
   else {
      rtxStrcpy (str, 5, "--");
      len = 2 + rtxIntToCharStr (pvalue->mon, str + 2, 3, '0');
   }

   if (pvalue->tz_flag == (OSBOOL)TRUE) {
      stat = rtxTimeZoneToString
         (pvalue, buffer + len, bufsize - len);
      if (stat < 0) return stat;
      len += stat;
   }

   return len;
}

EXTRTMETHOD int rtxGMonthDayToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat, len = 0;
   char* const str = (char*)buffer;
   int daysInMonth;

   if (bufsize < 8) return RTERR_OUTOFBND;
   *str = 0;

   if (pvalue->mon < 1 || pvalue->mon > 12) {
      /*add "\?\?" to tmpStr if mon is is out-of-range*/
      rtxStrcpy (str, bufsize, "--\?\?-\?\?");
      return RTERR_INVFORMAT;
   }
   else {
      rtxStrcpy (str, 5, "--");
      len = 2 + rtxIntToCharStr (pvalue->mon, str + 2, 3, '0');
   }

   daysInMonth = gDaysInMonth[pvalue->mon - 1];
   if (pvalue->mon == 2) {
      daysInMonth++;
   }
   if (pvalue->day < 1 || pvalue->day > daysInMonth) {
      /* add "\?\?" to tmpStr if day is out-of-range */
      rtxStrcat (str, bufsize, "-\?\?");
      return RTERR_INVFORMAT;
   }
   else {
      rtxStrcat (str, bufsize, "-"); len++;
      len += rtxIntToCharStr (pvalue->day, str + len, 3, '0');
   }

   if (pvalue->tz_flag == (OSBOOL)TRUE) {
      stat = rtxTimeZoneToString
         (pvalue, buffer + len, bufsize - len);
      if (stat < 0) return stat;
      len += stat;
   }

   return len;
}

EXTRTMETHOD int rtxGDayToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat, len = 0;
   char* str = (char*)buffer;

   if (bufsize < 6) return RTERR_OUTOFBND;
   *str = 0;

   if (pvalue->day < 1 || pvalue->day > 31) {
      /* add "\?\?" to tmpStr if day is out-of-range */
      rtxStrcpy (str, 6, "---\?\?");
      return RTERR_INVFORMAT;
   }
   else {
      rtxStrcpy (str, 6, "---");
      len = 3 + rtxIntToCharStr (pvalue->day, str + 3, 3, '0');
   }

   if (pvalue->tz_flag == (OSBOOL)TRUE) {
      stat = rtxTimeZoneToString
         (pvalue, buffer + len, bufsize - len);
      if (stat < 0) return stat;
      len += stat;
   }

   return len;
}
