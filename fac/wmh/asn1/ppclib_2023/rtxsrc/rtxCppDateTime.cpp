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
/**
 * @file rtxCppDateTime.cpp
 * C++ implementation of OSXSDDateTimeClass class defined in rtxCppDateTime.h
 */

#include <time.h>
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCppDateTime.h"
#include "rtxsrc/rtxDateTime.hh"
#include "rtxsrc/rtxPrint.h"

#define DATETIME_STRING_LENGTH1 40
#define DATETIME_STRING_LENGTH2 46

///////////////////////////////////////////////////////////////////////
//
// DateTime
//

/**
 * OSXSDDateTimeClass():
 * This is a default constructor, sets the date and time fields to zero.
 */
OSXSDDateTimeClass::OSXSDDateTimeClass()
{
   year = 0;
   mon = 0;
   day = 0;
   hour = 0;
   min = 0;
   sec = 0;
   tz_flag = FALSE;
   tzo = 0;
}

/**
 * OSXSDDateTimeClass(string):
 * This is a parameterized constructor, parses string and sets
 * the date and time fields.
 */
OSXSDDateTimeClass::OSXSDDateTimeClass (const OSUTF8CHAR* dtString)
{
   parseString (dtString);
}

/**
 * OSXSDDateTimeClass(param1,param2,..):
 * This is a parameterized constructor, sets the date and time elements
 * with the supplied parameter values .
 * @param    year    OSINT32 sets year field
 * @param    mon     OSUINT8 sets month field
 * @param    day     OSUINT8 sets day field
 * @param    hour    OSUINT8 sets hour field
 * @param    min     OSUINT8 sets minute filed
 * @param    sec     OSREAL  sets second field
 * @param    tz_flag OSBOOL  sets timezone flag
 * @param    tzo     OSINT32 sets timezone value
 */
OSXSDDateTimeClass::OSXSDDateTimeClass(OSINT32 year_, OSUINT8 mon_,
                        OSUINT8 day_, OSUINT8 hour_, OSUINT8 min_,
                        OSREAL sec_, OSBOOL tz_flag_,OSINT32 tzo_)
{
   this->year = year_;
   this->mon = mon_;
   this->day = day_;
   this->hour = hour_;
   this->min = min_;
   this->sec = sec_;
   this->tz_flag = tz_flag_;
   this->tzo = tzo_;
}

/**
 * OSXSDDateTimeClass(const OSXSDDateTimeClass& dt):
 * This is a copy constructor, sets the date and time fields to
 * that of equal to supplied OSXSDDateTimeClass type object.
 * @param    dt      OSXSDDateTimeClass type object.
 */
OSXSDDateTimeClass::OSXSDDateTimeClass(const OSXSDDateTimeClass& dt):
   OSRTBaseType (dt)
{
   setValue (dt);
}

/**
 * OSXSDDateTimeClass(const OSXSDDateTime& dt):
 * This is a copy constructor, sets the date and time fields to
 * that of equal to supplied OSXSDDateTimeClass type object.
 * @param    dt      OSXSDDateTimeClass type object.
 */
OSXSDDateTimeClass::OSXSDDateTimeClass(const OSXSDDateTime& dt)
{
   setValue (dt);
}

int OSXSDDateTimeClass::getTime (time_t& timeMs)
{
   return rtxGetDateTime (this, &timeMs);
}

/**
 * setCurrent:
 * This method sets the date and time fields to the values of
 * current date and time.
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 */
int OSXSDDateTimeClass::setCurrent()
{
   time_t tim = time((time_t *)NULL);
   struct tm ltime;
   rtxGetLocalTime (&ltime, tim);
   year = 1900 + ltime.tm_year;
   mon = (OSUINT8)(1 + ltime.tm_mon);
   day = (OSUINT8)ltime.tm_mday;
   hour = (OSUINT8)ltime.tm_hour;
   min = (OSUINT8)ltime.tm_min;
   sec = ltime.tm_sec;
   tz_flag = FALSE;
   tzo = 0;
   return RT_OK;
}


/**
 * setCurrentTz:
 * This method sets the date, time and timezone fields to the values of
 * current date and time.
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 */
int OSXSDDateTimeClass::setCurrentTz()
{
   time_t tim = time((time_t *)NULL);
   struct tm ltime;
   rtxGetLocalTime (&ltime, tim);
   year = 1900 + ltime.tm_year;
   mon = (OSUINT8)(1 + ltime.tm_mon);
   day = (OSUINT8)ltime.tm_mday;
   hour = (OSUINT8)ltime.tm_hour;
   min = (OSUINT8)ltime.tm_min;
   sec = ltime.tm_sec;
   tz_flag = TRUE;
#if defined(_MSC_VER)
   long tzsecs;
   _get_timezone(&tzsecs);
   tzo = -1 * tzsecs / 60;
#elif defined(__BORLANDC__) || defined(__WATCOMC__) || defined(__CYGWIN__) || \
   defined (_OS_RTOSARMHF)
   tzo = -1 * _timezone / 60;
#elif !defined(__vxworks) && !defined(__APPLE__) && !defined(_OS_NOTIMEZONE)
   tzo = -1 * timezone / 60;
#elif defined(__APPLE__)
   tzo = ltime.tm_gmtoff / 60;
#else
#warning "Timezone setting not supported on this platform!"
   tzo = 0;
#endif
   return RT_OK;
}


/**
 * parseString:
 * This method parses the datetime string and sets the date and
 * time value
 * @param dtString    const OSUTF8CHAR* - Date and time string
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 */
int OSXSDDateTimeClass::parseString (const OSUTF8CHAR* dtString)
{
   OSXSDDateTime dtval;
   size_t inpdatalen = rtxUTF8LenBytes (dtString);

   int stat = rtxParseDateTimeString (dtString, inpdatalen, &dtval);
   if (stat >= 0){
      year = dtval.year;
      mon = dtval.mon;
      day = dtval.day;
      hour = dtval.hour;
      min = dtval.min;
      sec = dtval.sec;
      tz_flag = dtval.tz_flag;
      tzo = dtval.tzo;
   }

   return stat;
}


/**
 * This method prints the datetime value to standard output.
 *
 * @param name - Name of generated string variable.
 */
void OSXSDDateTimeClass::print (const char* name)
{
   rtxPrintDateTime (name, this);
}

/**
 * This method sets the string value to the given datetime instance.
 *
 * @param utf8str - C null-terminated string.
 */
void OSXSDDateTimeClass::setValue (const OSUTF8CHAR* utf8str)
{
   parseString (utf8str);
}

/**
 * This method sets the datetime value to the given datetime instance.
 *
 * @param utf8str - C null-terminated string.
 */
void OSXSDDateTimeClass::setValue (const OSXSDDateTime& dt)
{
   year = dt.year;
   mon = dt.mon;
   day = dt.day;
   hour = dt.hour;
   min = dt.min;
   sec = dt.sec;
   tz_flag = dt.tz_flag;
   tzo = dt.tzo;
}

/**
 * toString:
 * This method sets the date and time fields to the values of
 * current date and time.
 * @return            const OSUTF8CHAR*  pointer
 *                      returns the datetime string
 */
const OSUTF8CHAR* OSXSDDateTimeClass::toString ()
{

   // Allocate a buffer
   OSUINT32 bufferlen;
   OSUTF8CHAR* buffer;
   if (tz_flag == (OSBOOL)TRUE) {
      bufferlen = DATETIME_STRING_LENGTH2 + 1;
      buffer = new OSUTF8CHAR[bufferlen];
      return toString(buffer, bufferlen);
   } else {
      bufferlen = DATETIME_STRING_LENGTH1 + 1;
      buffer = new OSUTF8CHAR[bufferlen];
      return toString(buffer, bufferlen);
   }
}

/**
 * toString:
 * This method sets the date and time fields to the values of
 * current date and time.
 * @param toString    const OSUTF8CHAR* - pointer to Date and time string
 * @param buffer      OSUTF8CHAR* - pointer to Date and time string
 * @param bufsize     size_t specifies buffer size
 * @return            const OSUTF8CHAR*
 *                      returns the datetime string
 */
const OSUTF8CHAR* OSXSDDateTimeClass::toString
   (OSUTF8CHAR* buffer, size_t bufsize)
{
   rtxDateTimeToString (this, buffer, bufsize);
   return buffer;
}

int OSXSDDateTimeClass::setDateTime (struct tm* time)
{
   return rtxSetDateTime (this, time);
}

int OSXSDDateTimeClass::setLocalTime (time_t timeMs)
{
   return rtxSetLocalDateTime (this, timeMs);
}

int OSXSDDateTimeClass::setUtcTime (time_t timeMs)
{
   return rtxSetUtcDateTime (this, timeMs);
}

///////////////////////////////////////////////////////////////////////
//
// Date
//

/**
 * OSXSDDateClass(string):
 * This is a parameterized constructor, parses string and sets
 * the date and time fields.
 */
OSXSDDateClass::OSXSDDateClass (const OSUTF8CHAR* dtString)
{
   parseString (dtString);
}

/**
 * parseString:
 * This method parses the date string and sets the date and
 * time value
 * @param dtString    const OSUTF8CHAR* - Date and time string
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 */
int OSXSDDateClass::parseString (const OSUTF8CHAR* dtString)
{
   OSXSDDateTime dtval;
   size_t inpdatalen = rtxUTF8LenBytes (dtString);

   int stat = rtxParseDateString (dtString, inpdatalen, &dtval);
   if (stat >= 0){
      year = dtval.year;
      mon = dtval.mon;
      day = dtval.day;
      hour = 0;
      min = 0;
      sec = 0;
      tz_flag = FALSE;
      tzo = 0;
   }

   return stat;
}

/**
 * This method prints the datetime value to standard output.
 *
 * @param name - Name of generated string variable.
 */
void OSXSDDateClass::print (const char* name)
{
   rtxPrintDate (name, this);
}

const OSUTF8CHAR* OSXSDDateClass::toString
   (OSUTF8CHAR* buffer, size_t bufsize)
{
   rtxDateToString (this, buffer, bufsize);
   return buffer;
}

///////////////////////////////////////////////////////////////////////
//
// Time
//

/**
 * OSXSDTimeClass(string):
 * This is a parameterized constructor, parses string and sets
 * the date and time fields.
 */
OSXSDTimeClass::OSXSDTimeClass (const OSUTF8CHAR* dtString)
{
   parseString (dtString);
}

/**
 * parseString:
 * This method parses the time string and sets the date and
 * time value
 * @param dtString    const OSUTF8CHAR* - Date and time string
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 */
int OSXSDTimeClass::parseString (const OSUTF8CHAR* dtString)
{
   OSXSDDateTime dtval;
   size_t inpdatalen = rtxUTF8LenBytes (dtString);

   int stat = rtxParseTimeString (dtString, inpdatalen, &dtval);
   if (stat >= 0){
      year = 0;
      mon = 0;
      day = 0;
      hour = dtval.hour;
      min = dtval.min;
      sec = dtval.sec;
      tz_flag = dtval.tz_flag;
      tzo = dtval.tzo;
   }

   return stat;
}

/**
 * This method prints the datetime value to standard output.
 *
 * @param name - Name of generated string variable.
 */
void OSXSDTimeClass::print (const char* name)
{
   rtxPrintTime (name, this);
}

const OSUTF8CHAR* OSXSDTimeClass::toString
   (OSUTF8CHAR* buffer, size_t bufsize)
{
   rtxTimeToString (this, buffer, bufsize);
   return buffer;
}

