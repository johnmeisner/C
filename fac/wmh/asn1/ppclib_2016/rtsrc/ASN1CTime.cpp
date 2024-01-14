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
// ASN1CTime
//
// ISO 8601 time and date for ITU-T X.680.
//
// Author Artem Bolgar.
// version 3.00   03 Sep, 2004 (refactored to use ASN1TTime)
//
// History:
// version 2.17   03 Jun, 2004 (fix for addMilliseconds and negative times)
// version 2.16   11 Nov, 2003 (secFraction now may contain more than one
//                              digit)
// version 2.15    4 Sep, 2003 (rtxsrc/wceAddon.h include added)
// version 2.14   31 Jul, 2003 (leap year bug fixed in addDays)
// version 2.13   11 Jul, 2003 (2038 year problem fixed + some other fixes)
// version 2.12   21 May, 2003 (WinCE support)
// version 1.11   03 Aug, 2002 (copy ctor, some fixes)
// version 1.09   05 Jun, 2002 (some warnings on Solaris were eliminated)
// version 1.08   23 Mar, 2002
//

#include <stdio.h>
#include <stdlib.h>
#include "rtsrc/ASN1CTime.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtsrc/asn1intl.h"

#if defined(_WIN32_WCE)
#include "rtxsrc/wceAddon.h"
#endif

//////////////////////////////////////////////////////////////////////
//
// ASN1CTime
EXTRTMETHOD ASN1CTime::ASN1CTime(OSRTMessageBufferIF& msgBuf, char*& buf, int bufSize,
   OSBOOL useDerRules):
   ASN1CType(msgBuf), parsed(FALSE), derRules(useDerRules),
   timeStr(buf), strSize(bufSize)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::ASN1CTime(OSRTMessageBufferIF& msgBuf, ASN1VisibleString& buf,
   OSBOOL useDerRules):
   ASN1CType(msgBuf), parsed(FALSE), derRules(useDerRules),
   timeStr(*(char**)&buf), strSize(0)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::ASN1CTime(OSRTContext& ctxt, char*& buf, int bufSize,
   OSBOOL useDerRules):
   ASN1CType(ctxt), parsed(FALSE), derRules(useDerRules),
   timeStr(buf), strSize(bufSize)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::ASN1CTime(OSRTContext& ctxt, ASN1VisibleString& buf,
   OSBOOL useDerRules):
   ASN1CType(ctxt), parsed(FALSE), derRules(useDerRules),
   timeStr(*(char**)&buf), strSize(0)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::ASN1CTime (const ASN1CTime& original) :
   ASN1CType (original),
   parsed (original.parsed), derRules (original.derRules),
   timeStr (pTimeStr), strSize(0),
   pTimeStr ((char*)timeStrBuf)
{
   size_t len = OSCRTLSTRLEN(original.timeStr);
   rtxStrcpy (timeStr, len, original.timeStr);
}

EXTRTMETHOD ASN1CTime::ASN1CTime(char*& buf, int bufSize, OSBOOL useDerRules) :
   ASN1CType(), parsed(FALSE), derRules(useDerRules),
   timeStr(buf), strSize(bufSize)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::ASN1CTime(ASN1VisibleString& buf, OSBOOL useDerRules) :
   ASN1CType(), parsed(FALSE), derRules(useDerRules),
   timeStr(*(char**)&buf), strSize(0)
{
   privateInit ();
}

EXTRTMETHOD ASN1CTime::~ASN1CTime()
{
}

EXTRTMETHOD void ASN1CTime::privateInit ()
{
}

/*
* Gets year (full 4 digits).
* Returns: year - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getYear() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getYear ();
}

/*
* Gets month. 1 - January, 12 - December.
* Returns: month - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getMonth() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getMonth ();
}

/*
* Gets day of month. 1 - 31 (30, 29, 28).
* Returns: day - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getDay() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getDay ();
}

/*
* Gets hours, 0..23
* Returns: hours - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getHour() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getHour ();
}

/*
* Gets minutes, 0..59.
* Returns: minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getMinute() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getMinute ();
}

/*
* Gets seconds, 0..59.
* Returns: seconds - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getSecond() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getSecond ();
}

/*
* Gets fraction of second, 0..9.
* Returns: fraction - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getFraction() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getFraction ();
}

EXTRTMETHOD int ASN1CTime::getFractionLen() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getFractionLen ();
}

EXTRTMETHOD double ASN1CTime::getFractionAsDouble() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getFractionAsDouble ();
}

EXTRTMETHOD int ASN1CTime::getFractionStr(char* const pBuf, size_t bufSize) {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getFractionStr (pBuf, bufSize);
}
/*
* Gets hours of difference between local time and UTC.
* Returns: hours - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getDiffHour() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getDiffHour ();
}

/*
* Gets minutes of difference between local time and UTC.
* Returns: minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getDiffMinute() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getDiffMinute ();
}

/*
* Gets difference (in minutes) between local time and UTC.
* Returns: difference in minutes - if success;
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CTime::getDiff() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getDiff ();
}

/*
* Gets UTC flag (Z).
* Returns: TRUE, if UTC flag ('Z') is set.
*/
EXTRTMETHOD OSBOOL ASN1CTime::getUTC() {
   if(!derRules && !parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0) {
         LOG_TMERR(getCtxtPtr (), stat);
         return FALSE;
      }
   }
   return getTimeObj().getUTC ();
}

/*
* Converts time string to time_t value (local time).
* Returns: time in seconds from midnight, January 1, 1970 - if success;
*          it will be local time.
*          RTERR_INVFORMAT - if parsed value is invalid.
*/
EXTRTMETHOD time_t ASN1CTime::getTime() {
   if(!parsed) {
      int stat = getTimeObj().parseString(timeStr);
      if(stat != 0)
         return LOG_TMERR(getCtxtPtr (), stat);
   }
   return getTimeObj().getTime ();
}

/*
* Sets UTC flag (Z).
* Returns 0, if succeed.
*/
EXTRTMETHOD int ASN1CTime::setUTC(OSBOOL utc) {
   if(derRules)
      return 0;
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setUTC (utc);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets year (full 4 digits).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setYear(short year_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setYear (year_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets month. 1 - January, 12 - December.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setMonth(short month_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setMonth (month_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets day of month. 1 - 31 (30, 29, 28).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setDay(short day_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setDay (day_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets hours, 0..23.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setHour(short hour_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setHour (hour_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets minutes, 0..59.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setMinute(short minute_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setMinute (minute_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets seconds, 0..59
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setSecond(short second_) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setSecond (second_);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets fraction of second, 0..9.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CTime::setFraction(int fraction, int fracLen) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setFraction (fraction, fracLen);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

EXTRTMETHOD int ASN1CTime::setFraction(double frac, int fracLen) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setFraction (frac, fracLen);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

EXTRTMETHOD int ASN1CTime::setFraction(char const* frac) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setFraction (frac);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets hour's part of difference between local time and UTC.
* It may be either positive or negative in range -12..12.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1CTime::setDiffHour(short dhour) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setDiffHour (dhour);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets difference between local time and UTC.
* Hours may be either positive or negative in range -12..12.
* Sign of minutes is not important, abs(minutes) must be in range 0..59.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1CTime::setDiff(short dhour, short dminute) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setDiff (dhour, dminute);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/*
* Sets difference between local time and UTC in minutes.
* It may be either positive or negative in range -12*60..12*60.
* Returns: 0 - if success;
*          RTERR_INVFORMAT - if value is invalid.
*/
EXTRTMETHOD int ASN1CTime::setDiff(short inMinutes) {
   if(!parsed)
      getTimeObj().parseString(timeStr);
   int rv = getTimeObj().setDiff (inMinutes);
   if (rv != 0) return rv;
   compileString();
   return 0;
}

/* Assignment operator. */
       const ASN1CTime& ASN1CTime::operator= (const ASN1CTime& op) {
   getTimeObj() = op.getTimeObj();
   derRules = op.derRules;
   parsed = op.parsed;
   checkCapacity ();
   strcpy (timeStr, op.timeStr);
   return *this;
}

/*
* Clears out time string.
*/
EXTRTMETHOD void ASN1CTime::clear() {
   getTimeObj ().clear ();
   checkCapacity();
   *timeStr = 0;
   parsed = TRUE;
}

/*
* Returns time string. NULL, if error occured.
*/
EXTRTMETHOD char*& ASN1CTime::getTimeStringPtr ()
{
   compileString();
   return timeStr;
}

/* Returns the length of compiled time string */
EXTRTMETHOD size_t ASN1CTime::getTimeStringLen ()
{
   compileString();
   return strlen (timeStr);
}

/* Copies the compiled time string into specified buffer */
EXTRTMETHOD const char* ASN1CTime::getTimeString (char* pbuf, size_t bufsize)
{
   size_t len = getTimeStringLen ();
   if (len >= bufsize)
      return 0;
   strcpy (pbuf, timeStr);
   return pbuf;
}

/* checks capacity of timeStr. If it is not enough - assign internal buffer. */
EXTRTMETHOD void ASN1CTime::checkCapacity() {
   if(strSize < MAX_TIMESTR_SIZE) {
      timeStr = timeStrBuf;
      strSize = MAX_TIMESTR_SIZE;
   }
}

EXTRTMETHOD int ASN1CTime::equals (ASN1CTime& op)
{
   if(!parsed)
      parseString(timeStr);

   if(!op.parsed)
      op.parseString(op.timeStr);

   return equals (op);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator == (ASN1CTime& op) {
   return (OSBOOL)(equals (op) == 0);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator != (ASN1CTime& op) {
   return (OSBOOL)(equals (op) != 0);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator > (ASN1CTime& op) {
   return (OSBOOL)(equals (op) > 0);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator < (ASN1CTime& op) {
   return (OSBOOL)(equals (op) < 0);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator >= (ASN1CTime& op) {
   return (OSBOOL)(equals (op) >= 0);
}

/* comparison operator. */
OSBOOL ASN1CTime::operator <= (ASN1CTime& op) {
   return (OSBOOL)(equals (op) <= 0);
}

EXTRTMETHOD int ASN1CTime::parseString(const char* str) {
   return getTimeObj().parseString (str);
}

