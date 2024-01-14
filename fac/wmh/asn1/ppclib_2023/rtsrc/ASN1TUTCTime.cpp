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

//////////////////////////////////////////////////////////////////////
//
// ASN1TUTCTime
//
// ISO 8601 time and date for ITU-T X.680.
//
#include <stdio.h>
#include <stdlib.h>
#include "rtsrc/ASN1TTime.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDateTime.h"

#if defined(_WIN32_WCE)
#include "rtxsrc/wceAddon.h"
#endif

EXTRTMETHOD ASN1TUTCTime::ASN1TUTCTime ()
{
   mHour = mMinute = RTERR_IDNOTFOU;
   mbUtcFlag = TRUE;
}

EXTRTMETHOD ASN1TUTCTime::ASN1TUTCTime
   (const char* timeStr, OSBOOL useDerRules) :
   ASN1TTime (useDerRules)
{
   mHour = mMinute = RTERR_IDNOTFOU;
   mbUtcFlag = TRUE;
   ASN1TUTCTime::parseString (timeStr);
}

EXTRTMETHOD ASN1TUTCTime::ASN1TUTCTime (OSBOOL useDerRules) :
   ASN1TTime (useDerRules)
{
   mHour = mMinute = RTERR_IDNOTFOU;
   mbUtcFlag = TRUE;
}

/*
* Sets year (4 or 2 last digits).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TUTCTime::setYear(short year_) {
   if (year_ < 0)
      return LOG_TTMERR (RTERR_INVFORMAT);
   if (year_ < 100) {
      if (year_ >= 50)
         ASN1TTime::setYear(year_ + 1900);
      else
         ASN1TTime::setYear(year_ + 2000);
   }
   return ASN1TTime::setYear(year_);
}

/*
* Parses string.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1TUTCTime::parseString(const char* str)
{
   int year_, month_, day_, hour_, minute_, second_;
   int diffHour_, diffMin_, stat;
   const char* p = str;

   if (str == 0)
   {
      return LOG_TTMERR (0);
   }

   year_ = month_ = day_ = hour_ = minute_ = RTERR_IDNOTFOU;
   second_ = diffHour_ = diffMin_ = 0;
   mSecFraction = mSecFracLen = 0;

   // Parse year
   stat = ncharsToInt (p, 2, year_);
   if (0 != stat)
   {
      return LOG_TTMERR (stat);
   }
   if (year_ < 0)
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }

   if (year_ < 100)
   {
      if (year_ >= 50)
      {
         year_ += 1900;
      }
      else
      {
         year_ += 2000;
      }
   }

   // Parse month
   p += 2;
   stat = ncharsToInt (p, 2, month_);
   if (0 != stat)
   {
      return LOG_TTMERR (stat);
   }
   if (month_ < 1 || month_ > 12)
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }

   int dim = daysInMonth(month_);
   // check for February of leap-year_
   if (month_ == 2 && year_%4 == 0 && (year_%100 != 0 || year_%400 == 0))
   {
      dim++;
   }

   // Parse day
   p += 2;
   stat = ncharsToInt (p, 2, day_);
   if (0 != stat)
   {
      return LOG_TTMERR (stat);
   }
   if (day_ < 1 || day_ > dim)
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }

   // Parse hour
   p += 2;
   int n = 0;
   if (OS_ISDIGIT(*p))
   {
      stat = ncharsToInt (p, 2, hour_);
      if (0 != stat)
      {
         return LOG_TTMERR (stat);
      }
      p += 2;
      n++;
   }

   // Parse minute
   if (OS_ISDIGIT(*p))
   {
      stat = ncharsToInt (p, 2, minute_);
      if (0 != stat)
      {
         return LOG_TTMERR (stat);
      }
      p += 2;
      n++;
   }

   // Parse second
   if (OS_ISDIGIT(*p))
   {
      stat = ncharsToInt (p, 2, second_);
      if (0 != stat)
      {
         return LOG_TTMERR (stat);
      }
      p += 2;
      n++;
   }

   if (n >= 1 && (hour_ < 0 || hour_ > 23))
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }
   if (n >= 2 && (minute_ < 0 || minute_ > 59))
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }
   if (n == 3 && (second_ < 0 || second_ > 59))
   {
      return LOG_TTMERR (RTERR_INVFORMAT);
   }

   if (*p == 'Z') // utc
   {
      p++;
      mbUtcFlag = TRUE;
      if (*p != 0)
      {
         return LOG_TTMERR (RTERR_INVFORMAT);
      }
   }
   else
   {
      if (mbDerRules) // DER/CER/PER time should be terminated with 'Z'!
      {
         return LOG_TTMERR (RTERR_INVFORMAT);
      }
      mbUtcFlag = FALSE;

      char sign = *p;
      if (sign == '-' || sign == '+')
      {
         // Parse diff hour
         p++;
         if (!OS_ISDIGIT(*p))
         {
            return LOG_TTMERR (RTERR_INVFORMAT);
         }

         stat = ncharsToInt (p, 2, diffHour_);
         if (0 != stat)
         {
            return LOG_TTMERR (stat);
         }
         if (diffHour_ < 0 || diffHour_ > 14)
         {
            return LOG_TTMERR (RTERR_INVFORMAT);
         }

         // Parse diff minute
         p += 2;
         if (!OS_ISDIGIT(*p))
         {
            return LOG_TTMERR (RTERR_INVFORMAT);
         }

         stat = ncharsToInt (p, 2, diffMin_);
         if (0 != stat)
         {
            return LOG_TTMERR (stat);
         }
         if (diffMin_ < 0 || diffMin_ > 59)
         {
            return LOG_TTMERR (RTERR_INVFORMAT);
         }

         if (sign == '-')
         {
            if (diffHour_ > 12)
            {
               return LOG_TTMERR (RTERR_INVFORMAT);
            }
            diffHour_ = -diffHour_;
            diffMin_ = -diffMin_;
         }
      }
      else
      {
         return LOG_TTMERR (RTERR_INVFORMAT);
      }
   }

   mYear = (short) year_;
   mMonth = (short) month_;
   mDay = (short) day_;
   mHour = (short) hour_;
   mMinute = (short) minute_;
   mSecond = (short) second_;
   mDiffHour = (short) diffHour_;
   mDiffMin = (short) diffMin_;

   return LOG_TTMERR (0);
}

/*
* Compiles new time string accoring X.680 (clause 42) and ISO 8601.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TUTCTime::compileString(char* pbuf, size_t bufsize) const {
   if (mYear < 0 || mDay <= 0 || mMonth <= 0 || mHour < 0 || mMinute < 0)
      return RTERR_INVFORMAT;

   char* p = pbuf;

   size_t cursize = 13; // incl trailing zero
   if (bufsize < cursize)
      return RTERR_BUFOVFLW;

   const char* ymdhm = "%.2d%.2d%.2d%.2d%.2d";
   if ((mbDerRules || mbUtcFlag) && (mDiffHour || mDiffMin)) {
      // we need to recalculate time with zone offset
      short year_ = mYear;
      short month_ = mMonth;
      short day_ = mDay;
      short hour_ = mHour;
      short minute_ = mMinute;
      short second_ = mSecond;
      int   secFr_ = mSecFraction;
      addMilliseconds (-(mDiffHour * 60 + mDiffMin) * 60 * 1000L,
                       year_, month_, day_, hour_, minute_, second_, secFr_,
                       mSecFracLen);
      os_snprintf(p, bufsize, ymdhm, year_%100, month_, day_, hour_, minute_);
   }
   else {
      os_snprintf(p, bufsize, ymdhm, mYear%100, mMonth, mDay, mHour, mMinute);
   }

   p += 10;

   os_snprintf(p, 3, "%.2hu", mSecond);
   p += 2;

   if (mbDerRules || mbUtcFlag) {
      cursize++;
      if (bufsize < cursize)
         return RTERR_BUFOVFLW;
      *p++ = 'Z'; *p = 0;
   }
   else if (mDiffHour || mDiffMin) {
      cursize += 5;
      if (bufsize < cursize)
         return RTERR_BUFOVFLW;

      *p++ = (mDiffHour > 0) ? '+' : '-';
      os_snprintf(p, 5, "%.2hu%.2hu", (short)abs(mDiffHour),
                                      (short)abs(mDiffMin));
   }
   else
      return RTERR_INVFORMAT;
   return 0;
}

/*
* Converts time_t to time string.
* Parameter 'diffTime' == TRUE means the difference between local time
*   and UTC will be calculated; in other case only UTC will be stored.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TUTCTime::setTime(time_t time, OSBOOL diffTime) {
   struct tm loctm;
   int ret = rtxGetLocalTime (&loctm, time);
   if (0 != ret) return ret;

   if (diffTime) {
      struct tm gmtm;
      long diff;

      ret = rtxGetGMTime (&gmtm, time);
      if (0 != ret) return ret;

      gmtm.tm_isdst = loctm.tm_isdst;
#ifdef OMNITRACS_RT // FINDME_OMNITRACS
      diff = (long)(rtWCEMkTime(&loctm) - rtWCEMkTime(&gmtm));
#else
      diff = (long)(mktime(&loctm) - mktime(&gmtm));
#endif
      // diff is cast to long because on some platforms the time_t is
      // unsigned (e.g. WinCE, ARM)
      mDiffHour = (short) (diff/3600);
      mDiffMin = (short) (diff%60);
      mbUtcFlag = (mbDerRules) ? TRUE : FALSE;
   }
   else {
      mDiffHour = mDiffMin = 0;
      mbUtcFlag = TRUE;
   }
   mYear = (short)(loctm.tm_year + 1900);
   mMonth = (short)(loctm.tm_mon + 1);
   mDay = (short)loctm.tm_mday;
   mHour = (short)loctm.tm_hour;
   mMinute = (short)loctm.tm_min;
   mSecond = (short)loctm.tm_sec;
   mSecFraction = mSecFracLen = 0;
   return LOG_TTMERR (0);
}

/*
* Clears out time string.
*/
EXTRTMETHOD void ASN1TUTCTime::clear() {
   ASN1TTime::clear();
   mHour = mMinute = RTERR_IDNOTFOU;
   mbUtcFlag = TRUE;
}

/*
* Gets fraction of second, 0..9.
* Returns: Zero.
*/
EXTRTMETHOD int ASN1TUTCTime::getFraction() const {
   return 0;
}

/*
* This method doesn't work for ASN1TUTCTime.
* Returns RTERR_NOTSUPP.
*/
EXTRTMETHOD int ASN1TUTCTime::setFraction(int /*fraction*/, int /*fracLen*/) {
   mSecFraction = mSecFracLen = 0;
   return LOG_TTMERR (RTERR_NOTSUPP);
}

/*
* Sets UTC flag (Z).
* Returns 0, if succeed.
*/
EXTRTMETHOD int ASN1TUTCTime::setUTC(OSBOOL utc) {
   if (utc != FALSE || mDiffHour + mDiffMin != 0)
      return ASN1TTime::setUTC (utc);
   return 0;
}

