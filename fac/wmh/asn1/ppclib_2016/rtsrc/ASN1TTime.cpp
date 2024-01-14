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

//////////////////////////////////////////////////////////////////////
//
// ASN1TTime
//
// ISO 8601 time and date for ITU-T X.680.
//
// Author Artem Bolgar.
// version 3.00   23 Aug, 2004 (converted to ASN1TTime)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rtsrc/ASN1TTime.h"
#include "rtxsrc/rtxCtype.h"
#include "rtsrc/asn1intl.h"

#if defined(_WIN32_WCE)
#include "rtxsrc/wceAddon.h"
#endif

//////////////////////////////////////////////////////////////////////
//
// ASN1CTime
EXTRTMETHOD ASN1TTime::ASN1TTime ():
   mbDerRules (FALSE)
{
   privateInit ();
}

EXTRTMETHOD ASN1TTime::ASN1TTime (OSBOOL useDerRules):
   mbDerRules (useDerRules)
{
   privateInit ();
}

EXTRTMETHOD ASN1TTime::ASN1TTime (const ASN1TTime& original) :
   mYear (original.mYear), mMonth (original.mMonth), mDay (original.mDay),
   mHour (original.mHour), mMinute (original.mMinute), mSecond (original.mSecond),
   mDiffHour (original.mDiffHour), mDiffMin (original.mDiffMin),
   mSecFraction (original.mSecFraction), mSecFracLen (original.mSecFracLen),
   mbUtcFlag (original.mbUtcFlag), mbDerRules (original.mbDerRules)
{
}

EXTRTMETHOD ASN1TTime::~ASN1TTime()
{
}

EXTRTMETHOD void ASN1TTime::privateInit ()
{
   mYear = mMonth = mDay = mHour = RTERR_IDNOTFOU;
   mMinute = mSecond = 0;
   mSecFracLen = mSecFraction = 0;
   mDiffHour = mDiffMin = 0;
   mbUtcFlag = (mbDerRules) ? TRUE : FALSE;
   mStatus = 0;
}

/* The following two functions are provided to help avoid using writeable static
 * memory. */
EXTRTMETHOD short ASN1TTime::daysInMonth(int i) {
   switch(i) {
      case 0:
         return 0;
      case 1: case 3: case 5: case 7: case 8: case 10: case 12:
         return 31;
      case 2:
         return 28;
      case 4: case 6: case 9: case 11:
         return 30;
      default:
         return 0;
   }
}

EXTRTMETHOD int ASN1TTime::daysAfterMonth(int i) {
   switch(i) {
      case 0: return 0;
      case 1: return 31;
      case 2: return 59;
      case 3: return 90;
      case 4: return 120;
      case 5: return 151;
      case 6: return 181;
      case 7: return 212;
      case 8: return 243;
      case 9: return 273;
      case 10:return 304;
      case 11:return 334;
      case 12:return 365;
      default:return 0;
   }
}


/*
* Gets mYear (full 4 digits).
* Returns: mYear - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getYear() const {
   if (mStatus != 0) return mStatus;
   return mYear;
}

/*
* Gets mMonth. 1 - January, 12 - December.
* Returns: mMonth - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getMonth() const {
   if (mStatus != 0) return mStatus;
   return mMonth;
}

/*
* Gets mDay of mMonth. 1 - 31 (30, 29, 28).
* Returns: mDay - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getDay() const {
   if (mStatus != 0) return mStatus;
   return mDay;
}

/*
* Gets hours, 0..23
* Returns: hours - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getHour() const {
   if (mStatus != 0) return mStatus;
   return mHour;
}

/*
* Gets minutes, 0..59.
* Returns: minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getMinute() const {
   if (mStatus != 0) return mStatus;
   return mMinute;
}

/*
* Gets seconds, 0..59.
* Returns: seconds - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getSecond() const {
   if (mStatus != 0) return mStatus;
   return mSecond;
}

/*
* Gets fraction of second, 0..9.
* Returns: fraction - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getFraction() const {
   if (mStatus != 0) return mStatus;
   return mSecFraction;
}

/*
* Gets fraction of second, 0..9.
* Returns: fraction - if success;
*          ASN_E_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD double ASN1TTime::getFractionAsDouble() const {
   if (mStatus != 0) return mStatus;
   double v = mSecFraction;
   v /= pow (double(10), double(mSecFracLen));
   return v;
}

EXTRTMETHOD int ASN1TTime::getFractionStr (char* const pBuf, size_t bufSize) const {
   if (mStatus != 0) return mStatus;
   if ((size_t)mSecFracLen >= bufSize) return RTERR_STROVFLW;

   int i, f = mSecFraction;
   for (i = mSecFracLen - 1; i >= 0; i--) {
      pBuf[i] = (f % 10) + '0';
      f /= 10;
   }
   pBuf [mSecFracLen] = '\0';
   return mSecFracLen;
}

/*
* Gets fraction length of second.
* Returns: fraction - if success;
*          ASN_E_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getFractionLen() const {
   if (mStatus != 0) return mStatus;
   return mSecFracLen;
}

/*
* Gets hours of difference between local time and UTC.
* Returns: hours - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getDiffHour() const {
   if (mStatus != 0) return mStatus;
   return mDiffHour;
}

/*
* Gets minutes of difference between local time and UTC.
* Returns: minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getDiffMinute() const {
   if (mStatus != 0) return mStatus;
   return mDiffMin;
}

/*
* Gets difference (in minutes) between local time and UTC.
* Returns: difference in minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1TTime::getDiff() const {
   if (mStatus != 0) return mStatus;
   return mDiffHour*60 + mDiffMin;
}

/*
* Gets UTC flag (Z).
* Returns: TRUE, if UTC flag ('Z') is set.
*/
EXTRTMETHOD OSBOOL ASN1TTime::getUTC() const {
   return mbUtcFlag;
}

/*
* Converts time string to time_t value (local time).
* Returns: time in seconds from midnight, January 1, 1970 - if success;
*          it will be local time.
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD time_t ASN1TTime::getTime() const
{
   if (mStatus != 0) return 0;
   struct tm tm;
   memset(&tm, 0, sizeof(tm));
   tm.tm_year = mYear - 1900;
   tm.tm_mon = mMonth - 1;
   tm.tm_mday = mDay;
   tm.tm_hour = mHour;
   tm.tm_min = mMinute;
   tm.tm_sec = mSecond;
#ifdef OMNITRACS_RT
   time_t t1 = rtWCEMkTime(&tm);
#else
   time_t t1 = mktime(&tm);
#endif
   time_t diff = 0;
   if (abs(mDiffHour) > 0 || abs(mDiffMin) > 0)
      diff = mDiffHour*3600 + mDiffMin*60;
   return t1 + diff;
}

/*
* Sets UTC flag (Z).
* Returns 0, if succeed.
*/
EXTRTMETHOD int ASN1TTime::setUTC(OSBOOL utc)
{
   if (mbDerRules)
      return 0;
   mbUtcFlag = utc;
   return 0;
}

EXTRTMETHOD int ASN1TTime::checkDate (int day, int month, int year)
{
   if (day <= 0 || month <= 0) return 1; // don't check, if day or month is not set
   if (year >= 0 && month > 0) { // check as usual
      int dim = daysInMonth(month);
      // check for February of leap-year
      if (month == February && year%4 == 0 && (year%100 != 0 || year%400 == 0))
         dim++;
      if (day >= 1 && day <= dim)
         return 1;
   }
   else if (month > 0) {
      if (day <= daysInMonth(month))
         return 1;
      if (month == February && day <= daysInMonth(month) + 1)
         return 1;
   }
   return 0;
}

/*
* Sets mYear (full 4 digits).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setYear(short year_) {
   if (year_ < 0 || !checkDate (mDay, mMonth, year_))
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mYear = year_;
   return 0;
}

/*
* Sets mMonth. 1 - January, 12 - December.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setMonth(short month_) {
   if (month_ < 1 || month_ > 12 || !checkDate (mDay, month_, mYear))
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mMonth = month_;
   return 0;
}

/*
* Sets mDay of mMonth. 1 - 31 (30, 29, 28).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setDay(short day_) {
   if (day_ < 1 || day_ > 31 || !checkDate (day_, mMonth, mYear))
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mDay = day_;
   return 0;
}

/*
* Sets hours, 0..23.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setHour(short hour_) {
   if (hour_ < 0 || hour_ > 23)
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mHour = hour_;
   return 0;
}

/*
* Sets minutes, 0..59.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setMinute(short minute_) {
   if (minute_ < 0 || minute_ > 59)
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mMinute = minute_;
   return 0;
}

/*
* Sets seconds, 0..59
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setSecond(short second_) {
   if (second_ < 0 || second_ > 59)
      return LOG_TTMERR (RTERR_INVFORMAT);
   this->mSecond = second_;
   return 0;
}

/*
* Sets fraction of second, 0..9.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1TTime::setFraction(int fraction, int fracLen) {
   this->mSecFraction = fraction;
   if (fracLen < 0) {
      this->mSecFracLen = int(log10 (double(fraction))) + 1;
   }
   else {
      this->mSecFracLen = fracLen;
   }
   return 0;
}

EXTRTMETHOD int ASN1TTime::setFraction (double frac, int fracLen) {
   mSecFracLen = fracLen;
   mSecFraction = 0;
   for (int i = 0; i < fracLen; ++i) {
      mSecFraction *= 10;
      frac *= 10;
      int intpart = int (frac + 0.5);
      mSecFraction += intpart;
      frac -= intpart;
   }
   return 0;
}

EXTRTMETHOD int ASN1TTime::setFraction (char const* frac) {
   size_t i = 0, l = strlen (frac);
   mSecFraction = 0;
   for (; i < l; ++i) {
      mSecFraction *= 10;
      mSecFraction += int (frac[i] - '0');
   }
   mSecFracLen = (int)l;
   return 0;
}

/*
* Sets mHour's part of difference between local time and UTC.
* It may be either positive or negative in range -12..12.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1TTime::setDiffHour(short dhour) {
   if (dhour < -12 || dhour > 13)
      return LOG_TTMERR (RTERR_INVFORMAT);
   mDiffHour = dhour;
   return 0;
}

/*
* Sets difference between local time and UTC.
* Hours may be either positive or negative in range -12..12.
* Sign of minutes is not important, abs(minutes) must be in range 0..59.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1TTime::setDiff(short dhour, short dminute) {
   if (dhour < -12 || dhour > 12 || abs(dminute) > 59)
      return LOG_TTMERR (RTERR_INVFORMAT);
   mDiffHour = dhour;
   if (mDiffHour < 0)
      mDiffMin = (short)-abs(dminute);
   else
      mDiffMin = (short)abs(dminute);
   return 0;
}

/*
* Sets difference between local time and UTC in minutes.
* It may be either positive or negative in range -12*60..12*60.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1TTime::setDiff(short inMinutes) {
   if (abs(inMinutes) > 12*60)
      return LOG_TTMERR (RTERR_INVFORMAT);
   mDiffHour = inMinutes/60;
   mDiffMin = inMinutes%60;
   return 0;
}

/* Assignment operator. */
       const ASN1TTime& ASN1TTime::operator= (const ASN1TTime& op) {
   mYear = op.mYear;
   mMonth = op.mMonth;
   mDay = op.mDay;
   mHour = op.mHour;
   mMinute = op.mMinute;
   mSecond = op.mSecond;
   mSecFraction = op.mSecFraction;
   mSecFracLen = op.mSecFracLen;
   mbUtcFlag = op.mbUtcFlag;
   mDiffHour = op.mDiffHour;
   mDiffMin = op.mDiffMin;
   mbDerRules = op.mbDerRules;
   return *this;
}

/*
* Clears out time string.
*/
EXTRTMETHOD void ASN1TTime::clear() {
   mYear = mMonth = mDay = mHour = RTERR_IDNOTFOU;
   mMinute = mSecond = mDiffHour = mDiffMin = 0;
   mSecFraction = mSecFracLen = 0;
   mbUtcFlag = (mbDerRules) ? TRUE : FALSE;
}

/* Copies the compiled time string into specified buffer */
EXTRTMETHOD const char* ASN1TTime::toString (char* pbuf, size_t bufsize) const
{
   if (compileString(pbuf, bufsize) == 0)
      return pbuf;
   return 0;
}

EXTRTMETHOD const char* ASN1TTime::toString (OSCTXT* pctxt) const
{
   char tstr [MAX_TIMESTR_SIZE];
   if (compileString (tstr, MAX_TIMESTR_SIZE) == 0) {
      size_t len = strlen (tstr);
      char* pStr = (char*)rtxMemAlloc (pctxt, len + 1);
      strcpy (pStr, tstr);
      return pStr;
   }
   return 0;
}

/* Copies the compiled time string into specified buffer */
EXTRTMETHOD const char* ASN1TTime::toString () const
{
   char tstr [MAX_TIMESTR_SIZE];
   if (compileString (tstr, MAX_TIMESTR_SIZE) == 0) {
      size_t len = strlen (tstr);
      char* pStr = new char[len + 1];
      strcpy (pStr, tstr);
      return pStr;
   }
   return 0;
}

EXTRTMETHOD int ASN1TTime::equals (const ASN1TTime& op) const
{
   int days = getDaysNum ();
   int opdays = op.getDaysNum ();

   if (days != opdays)
      return days - opdays;

   long ms = getMillisNum ();
   long opms = op.getMillisNum ();

   return (int)(ms - opms);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator == (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) == 0);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator != (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) != 0);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator > (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) > 0);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator < (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) < 0);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator >= (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) >= 0);
}

/* comparison operator. */
OSBOOL ASN1TTime::operator <= (const ASN1TTime& op) const
{
   return (OSBOOL)(equals (op) <= 0);
}

EXTRTMETHOD int ASN1TTime::getDaysNum () const
{
   int d = mDay - 1 + daysAfterMonth(mMonth - 1) + (mYear - 1) * 365;
   // correct leap mYear's days
   int leapYears = (mYear - 1) / 4 - ((mYear - 1) / 100) + ((mYear - 1) / 400);
   d += leapYears;
   if (mMonth > February && mYear%4 == 0 && (mYear%100 != 0 || mYear%400 == 0))
      d++;
   return d;
}

EXTRTMETHOD long ASN1TTime::getMillisNum () const
{
   long ms = int(mSecFraction * 1000. / pow (double(10), mSecFracLen)) +
      (mSecond + mMinute * 60 + mHour * 3600) * 1000L;
   if (mDiffHour + mDiffMin != 0)
      ms -= (mDiffHour * 60 + mDiffMin) * 60 * 1000L;
   return ms;
}

EXTRTMETHOD void ASN1TTime::addMilliseconds (int deltaMs,
                                 short& year, short& month, short& day,
                                 short& hour, short& minute,
                                 short& second, int& secFraction,
                                 int secFracLen)
{
   long ms = int(secFraction * 1000. / pow (double(10), secFracLen))
      + (second + minute * 60 + hour * 3600) * 1000L;
   ms += deltaMs;

   long daysDelta = ms / (3600 * 24 * 1000L);
   if (ms < 0) { // added to handle negative times.
      ms += (1 - daysDelta) * 3600 * 24 * 1000L;
      daysDelta += - 1 + ms / (3600 * 24 * 1000L);
   }
   if (daysDelta != 0) {
      ms %= (3600 * 24 * 1000);
      addDays (daysDelta, year, month, day);
   }

   hour = (short)(ms / (1000 * 3600L));
   minute = (short)((ms % (1000 * 3600L)) / (60 * 1000L));
   second = (short)((ms % (60 * 1000L)) / 1000L);
   secFraction = int(double(ms % 1000L) / pow (double (10), 3 - secFracLen));
}

EXTRTMETHOD void ASN1TTime::addDays
(int deltaDays, short& year, short& month, short& day)
{
   int d = day - 1 + daysAfterMonth(month - 1) + (year - 1) * 365;

   // correct leap year's days

   int leapYears = (year - 1) / 4 - ((year - 1) / 100) + ((year - 1) / 400);
   d += leapYears;

   if (month > February && year%4 == 0 && (year%100 != 0 || year%400 == 0))
      d++;

   d += deltaDays;

   int n400, n100, n4, n1, dayOfYear;

   n400 = d / (400 * 365 + (100/4 - 1) * 4 + 1); // 146097
   dayOfYear = d % (400 * 365 + (100/4 - 1) * 4 + 1); // 146097
   n100 = dayOfYear / (365 * 100 + 100/4 - 1); //36524;
   dayOfYear %= (365 * 100 + 100/4 - 1); //36524;
   n4 = dayOfYear / (365*4 + 1); //1461;
   dayOfYear %= (365*4 + 1); //1461;
   n1 = dayOfYear / 365;
   dayOfYear %= 365; // zero-based day of year

   year = (short)(400 * n400 + 100 * n100 + 4 * n4 + n1);
   if (n100 == 4 || n1 == 4)
      dayOfYear = 365; // Dec 31 at end of 4- or 400-yr cycle
   else
      year++;

   int isLeap = (year%4 == 0) && (year%100 != 0 || year%400 == 0);

   int correction = 0;
   int march1 = isLeap ? 60 : 59; // zero-based DOY for March 1
   if (dayOfYear >= march1) {
      correction = isLeap ? 1 : 2;
   }
   month = (short)((12 * (dayOfYear + correction) + 6) / 367 + 1);
   day = (short)(dayOfYear -  daysAfterMonth(month - 1) + 1);
   if (month > Feb && isLeap)
      day--; // correct day if leap year, because 'daysAfterMonth(month - 1)'
             // contains values for non-leap year.
}

int ASN1TTime::ncharsToInt (const char* str, OSSIZE nchars, int& value)
{
   value = 0;

   for (OSSIZE i = 0; i < nchars; i++) {
      if (!OS_ISDIGIT(str[i])) return RTERR_INVFORMAT;
      else value = (value * 10) + (str[i] - '0');
   }

   return 0;
}
