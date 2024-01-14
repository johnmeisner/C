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
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"

#ifdef _WIN32_WCE
#include "rtxsrc/wceAddon.h"
#endif

const signed char gDaysInMonth[12] =
   { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

EXTRTMETHOD OSBOOL rtxDateIsValid (const OSNumDateTime* dateTime)
{
   int dim;
   if (dateTime->year < 0 || dateTime->year > 9999 ||
       dateTime->mon < 1 || dateTime->mon > 12 ||
       dateTime->day < 1 || dateTime->day > 31 ||
       (dateTime->tz_flag && (dateTime->tzo < -840 || dateTime->tzo > 840)) )
      return FALSE;

   dim = (int) gDaysInMonth [dateTime->mon - 1];

   /* check for February of leap-year */
   if (dateTime->mon == 2 && dateTime->year % 4 == 0 &&
       (dateTime->year % 100 != 0 || dateTime->year % 400 == 0))
      dim++;

   if (dateTime->day < 1 || dateTime->day > dim)
      return FALSE;

   return TRUE;
}

EXTRTMETHOD OSBOOL rtxTimeIsValid (const OSNumDateTime* dateTime)
{
   if (dateTime->hour > 23 ||
       dateTime->min > 59 ||
       dateTime->sec < 0 || dateTime->sec >= 60 ||
       (dateTime->tz_flag && (dateTime->tzo < -840 || dateTime->tzo > 840)))
      return FALSE;

   return TRUE;
}


EXTRTMETHOD OSBOOL rtxDateTimeIsValid (const OSNumDateTime* dateTime)
{
   return (OSBOOL)(rtxDateIsValid (dateTime) && rtxTimeIsValid (dateTime));
}

/**
  * rtxGetCurrDateTime:
  * This function reads the system date and time and return the same in
  * OSNumDateTime structure
  */
EXTRTMETHOD int rtxGetCurrDateTime(OSNumDateTime* pvalue)
{
#ifdef OMNITRACS_RT
   time_t tim = rtWCETime((time_t *)NULL);
#else
   time_t tim = time((time_t *)NULL);
#endif
   return rtxSetLocalDateTime (pvalue, tim);
}

EXTRTMETHOD int rtxGetCurrDateTimeString
(char* buffer, OSSIZE bufsize, OSBOOL local)
{
   struct tm tmval;
   time_t currTime;
   int ret = 0;

   time (&currTime);

   if (local)
      ret = rtxGetLocalTime(&tmval, currTime);
   else
      ret = rtxGetGMTime(&tmval, currTime);

   if (0 == ret)
      ret = rtxAscTime(buffer, bufsize, &tmval);

   return ret;
}

EXTRTMETHOD int rtxSetDateTime (OSNumDateTime* pvalue, struct tm* timeStruct)
{
   if (0 == timeStruct) {
      return RTERR_INVPARAM;
   }

   pvalue->year = 1900 + timeStruct->tm_year;
   pvalue->mon = (OSINT8)(1 + timeStruct->tm_mon);
   pvalue->day = (OSINT8)(timeStruct->tm_mday);
   pvalue->hour = (OSINT8)(timeStruct->tm_hour);
   pvalue->min = (OSINT8)(timeStruct->tm_min);
   pvalue->sec = timeStruct->tm_sec;
   pvalue->tz_flag = FALSE;
   pvalue->tzo = 0;
   return RT_OK;
}

EXTRTMETHOD int rtxGetGMTime (struct tm* pvalue, time_t timeMs)
{
   if (0 == pvalue) return RTERR_NULLPTR;
#if defined(OMNITRACS_RT)
   *pvalue = *rtWCEGmtTime(&timeMs);
#elif defined(_MSC_VER) && _MSC_VER >= 1400
   if (gmtime_s(pvalue, &timeMs) != 0) {
      return RTERR_FAILED;
   }
/* assume gmtime_r only defined for Linux and Mac OSX */
#elif defined(__linux__) || defined(__APPLE__)
   gmtime_r(&timeMs, pvalue);
#else
   *pvalue = *gmtime(&timeMs);
#endif
   return 0;
}

EXTRTMETHOD int rtxGetLocalTime (struct tm* pvalue, time_t timeMs)
{
   if (0 == pvalue) return RTERR_NULLPTR;
#if defined(OMNITRACS_RT)
   *pvalue = *rtWCELocalTime(&timeMs);
#elif defined(_MSC_VER) && _MSC_VER >= 1400
   if (localtime_s(pvalue, &timeMs) != 0) {
      return RTERR_FAILED;
   }
/* assume localtime_r only defined for Linux and Mac OSX */
#elif defined(__linux__) || defined(__APPLE__)
   localtime_r(&timeMs, pvalue);
#else
   *pvalue = *localtime(&timeMs);
#endif
   return 0;
}

EXTRTMETHOD int rtxSetLocalDateTime (OSNumDateTime* pvalue, time_t timeMs)
{
   struct tm* ltime;
#ifdef OMNITRACS_RT
   ltime = rtWCELocalTime(&timeMs);
#else

#if defined(_MSC_VER) && _MSC_VER >= 1400
   struct tm tmvar;
   ltime = &tmvar;
   if (0 != localtime_s (ltime, &timeMs)) {
      return RTERR_INVPARAM;
   }
#else
   ltime = localtime (&timeMs);
#endif

#endif

   return (0 != ltime) ? rtxSetDateTime (pvalue, ltime) : RTERR_INVPARAM;
}

EXTRTMETHOD int rtxSetUtcDateTime (OSNumDateTime* pvalue, time_t timeMs)
{
   struct tm* gtime;
#ifdef OMNITRACS_RT
   gtime = rtWCEGmtTime(&timeMs);
#else

#if defined(_MSC_VER) && _MSC_VER >= 1400
   struct tm tmvar;
   gtime = &tmvar;
   if (0 != gmtime_s (gtime, &timeMs)) {
      return RTERR_INVPARAM;
   }
#else
   gtime = gmtime (&timeMs);
#endif

#endif
   return (0 != gtime) ? rtxSetDateTime (pvalue, gtime) : RTERR_INVPARAM;
}

EXTRTMETHOD int rtxGetDateTime (const OSNumDateTime* pvalue, time_t* timeMs)
{
   struct tm timeStruct;

   OSCRTLMEMSET ((void*)&timeStruct, 0, sizeof(timeStruct));

   if (timeMs == 0 || pvalue == 0) return RTERR_INVPARAM;
   timeStruct.tm_year = pvalue->year - 1900;
   timeStruct.tm_mon = pvalue->mon - 1;
   timeStruct.tm_mday = pvalue->day;
   timeStruct.tm_hour = pvalue->hour;
   timeStruct.tm_min = pvalue->min;
   timeStruct.tm_sec = (int)pvalue->sec;
   timeStruct.tm_isdst = -1;
#ifdef OMNITRACS_RT
   * timeMs = rtWCEMkTime(&timeStruct);
#else
   *timeMs = mktime(&timeStruct);
#endif

   if (pvalue->tz_flag) {
      struct tm* nowTimeStruct = 0;
      time_t utc_now = time (0);
      time_t local_tzo = 0;
#ifdef OMNITRACS_RT
      nowTimeStruct = rtWCEGmtTime(&utc_now);
#else

#if defined(_MSC_VER) && _MSC_VER >= 1400
      struct tm tmvar;
      nowTimeStruct = &tmvar;
      if (0 != gmtime_s (nowTimeStruct, &utc_now)) {
         return RTERR_INVPARAM;
      }
#else
      if (0 == (nowTimeStruct = gmtime (&utc_now))) {
         return RTERR_INVPARAM;
      }
#endif

#endif

      nowTimeStruct->tm_isdst = -1;
#ifdef OMNITRACS_RT
      local_tzo = (rtWCEMkTime(nowTimeStruct) - utc_now) / 60;
#else
      local_tzo = (mktime(nowTimeStruct) - utc_now) / 60;
#endif
      *timeMs += ((pvalue->tzo * -1) - local_tzo) * 60;
   }

   return RT_OK;
}

EXTRTMETHOD int rtxAscTime(char* buffer, OSSIZE bufsize, struct tm* timeptr)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
   return asctime_s(buffer, bufsize, timeptr);
#else
   const char* timestr = asctime(timeptr);
   rtxStrcpy(buffer, bufsize, timestr);
   return 0;
#endif
}
