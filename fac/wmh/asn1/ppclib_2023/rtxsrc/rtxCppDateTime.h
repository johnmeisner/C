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
 * @file rtxCppDateTime.h
 * C++ XML schema date/time definition.
 */
#ifndef _RTXCPPDATETIME_H_
#define _RTXCPPDATETIME_H_

#include "rtxsrc/rtxCommon.h"
#include "rtxsrc/OSRTBaseType.h"

class EXTRTCLASS OSXSDDateTimeClass
   : public OSXSDDateTime, public OSRTBaseType {
 public:
   /**
    * OSXSDDateTimeClass():
    * This is a default constructor, sets the date and time fields to zero.
    */
   OSXSDDateTimeClass ();

   /**
    * OSXSDDateTimeClass(string):
    * This is a parameterized constructor, parses string and sets
    * the date and time fields.
    */
   OSXSDDateTimeClass (const OSUTF8CHAR* dtString);

   /**
    * OSXSDDateTimeClass(param1,param2,..):
    * This is a parameterized constructor, sets the date and time elements
    * with the supplied parameter values .
    * @param    year_    OSINT32 sets year field
    * @param    mon_     OSUINT8 sets month field
    * @param    day_     OSUINT8 sets day field
    * @param    hour_    OSUINT8 sets hour field
    * @param    min_     OSUINT8 sets minute filed
    * @param    sec_     OSREAL  sets second field
    * @param    tz_flag_ OSBOOL  sets timezone flag
    * @param    tzo_     OSINT32 sets timezone value
    */
   OSXSDDateTimeClass (OSINT32 year_, OSUINT8 mon_,
                        OSUINT8 day_, OSUINT8 hour_, OSUINT8 min_,
                        OSREAL sec_, OSBOOL tz_flag_, OSINT32 tzo_);
   /**
    * OSXSDDateTimeClass(const OSXSDDateTimeClass& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDDateTimeClass (const OSXSDDateTimeClass& dt);

   /**
    * OSXSDDateTimeClass(const OSXSDDateTime& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDDateTimeClass (const OSXSDDateTime& dt);

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSXSDDateTimeClass (*this); }

   /**
    * This method converts the datetime value to a calendar time encoded
    * as a value of type time_t.
    *
    * @param timeMs    A pointer to time_t value to be set.
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error.
    */
   int getTime (time_t& timeMs);

   /**
    * parseString:
    * This method parses the datetime string and sets the date and
    * time value
    * @param dtString    const OSUTF8CHAR* - Date and time string
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error
    */
   virtual int parseString (const OSUTF8CHAR* dtString);

   /**
    * This method prints the datetime value to standard output.
    *
    * @param name - Name of generated string variable.
    */
   void print (const char* name);

   /**
    * setCurrent:
    * This method sets the date and time fields to the values of
    * current date and time.
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error
    */
   int setCurrent ();

   /**
    * setCurrentTz:
    * This method sets the date, time and timezone fields to the values of
    * current date and time.
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error
    */
   int setCurrentTz ();

   /**
    * This method converts a structure of type tm to the datetime value.
    *
    * @param time        A pointer to tm structure to be converted.
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error.
    */
   int setDateTime (struct tm* time);

   /**
    * This method sets local date and time to the datetime value.
    *
    * @param timeMs      A calendar time encoded as a value of type time_t.
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error.
    */
   int setLocalTime (time_t timeMs);

   /**
    * This method sets UTC date and time to the datetime value.
    *
    * @param timeMs      A calendar time encoded as a value of type time_t.
    *                      The time is represented as seconds elapsed since
    *                      midnight (00:00:00), January 1, 1970, coordinated
    *                      universal time (UTC).
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error.
    */
   int setUtcTime (time_t timeMs);

   /**
    * This method sets the string value to the given datetime instance.
    *
    * @param utf8str - C null-terminated string.
    */
   void setValue (const OSUTF8CHAR* utf8str);

   /**
    * This method sets the datetime value to the given datetime instance.
    *
    * @param dt - OSXSDDateTimeClass type object.
    */
   void setValue (const OSXSDDateTime& dt);

   /**
    * toString:
    * This method sets the date and time fields to the values of
    * current date and time.
    * @return            const OSUTF8CHAR*  pointer
    *                      returns the datetime string
    */
   const OSUTF8CHAR* toString ();

   /**
    * This method sets the date and time fields to the values of
    * current date and time.
    * @param buffer      OSUTF8CHAR* - pointer to Date and time string
    * @param bufsize     size_t specifies buffer size
    * @return            const OSUTF8CHAR*
    *                      returns the datetime string
    */
   virtual const OSUTF8CHAR* toString (OSUTF8CHAR* buffer, size_t bufsize);

   /**
    * Assignment operator.
    */
   inline OSXSDDateTimeClass& operator= (const OSXSDDateTimeClass& orig) {
      setValue (orig);
      return *this;
   }
} ;

class EXTRTCLASS OSXSDDateClass : public OSXSDDateTimeClass {
 public:
   /**
    * OSXSDDateClass():
    * This is a default constructor, sets the date and time fields to zero.
    */
   OSXSDDateClass () {}

   /**
    * OSXSDDateClass(string):
    * This is a parameterized constructor, parses string and sets
    * the date and time fields.
    */
   OSXSDDateClass (const OSUTF8CHAR* dtString);

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
   OSXSDDateClass (OSINT32 year_, OSUINT8 mon_,
                   OSUINT8 day_, OSUINT8 hour_, OSUINT8 min_,
                   OSREAL sec_, OSBOOL tz_flag_,OSINT32 tzo_):
      OSXSDDateTimeClass (year_, mon_, day_, hour_, min_, sec_, tz_flag_, tzo_) {}

   /**
    * OSXSDDateTimeClass(const OSXSDDateTimeClass& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDDateClass (const OSXSDDateTimeClass& dt) :
      OSXSDDateTimeClass (dt) {}

   /**
    * OSXSDDateTimeClass(const OSXSDDateTime& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDDateClass (const OSXSDDateTime& dt) :
      OSXSDDateTimeClass (dt) {}

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSXSDDateClass (*this); }

   /**
    * parseString:
    * This method parses the date string and sets the date and
    * time value
    * @param dtString    const OSUTF8CHAR* - Date and time string
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error
    */
   virtual int parseString (const OSUTF8CHAR* dtString);

   /**
    * This method prints the datetime value to standard output.
    *
    * @param name - Name of generated string variable.
    */
   void print (const char* name);

   /**
    * This method sets the date and time fields to the values of
    * current date and time.
    * @param buffer      OSUTF8CHAR* - pointer to Date and time string
    * @param bufsize     size_t specifies buffer size
    * @return            const OSUTF8CHAR*
    *                      returns the datetime string
    */
   virtual const OSUTF8CHAR* toString (OSUTF8CHAR* buffer, size_t bufsize);
} ;

class EXTRTCLASS OSXSDTimeClass : public OSXSDDateTimeClass {
 public:
   /**
    * OSXSDTimeClass():
    * This is a default constructor, sets the date and time fields to zero.
    */
   OSXSDTimeClass () {}

   /**
    * OSXSDTimeClass(string):
    * This is a parameterized constructor, parses string and sets
    * the date and time fields.
    */
   OSXSDTimeClass (const OSUTF8CHAR* dtString);

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
   OSXSDTimeClass (OSINT32 year_, OSUINT8 mon_,
                   OSUINT8 day_, OSUINT8 hour_, OSUINT8 min_,
                   OSREAL sec_, OSBOOL tz_flag_, OSINT32 tzo_):
      OSXSDDateTimeClass (year_, mon_, day_, hour_, min_, sec_, tz_flag_, tzo_) {}

   /**
    * OSXSDDateTimeClass(const OSXSDDateTimeClass& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDTimeClass (const OSXSDDateTimeClass& dt) :
      OSXSDDateTimeClass (dt) {}

   /**
    * OSXSDDateTimeClass(const OSXSDDateTime& dt):
    * This is a copy constructor, sets the date and time fields to
    * that of equal to supplied OSXSDDateTimeClass type object.
    * @param    dt      OSXSDDateTimeClass type object.
    */
   OSXSDTimeClass (const OSXSDDateTime& dt) :
      OSXSDDateTimeClass (dt) {}

   /**
    * Clone method. Creates a copied instance and returns pointer to
    * OSRTBaseType.
    */
   OSRTBaseType* clone () const { return new OSXSDTimeClass (*this); }

   /**
    * parseString:
    * This method parses the time string and sets the date and
    * time value
    * @param dtString    const OSUTF8CHAR* - Date and time string
    * @return            Completion status of operation:
    *                  - 0(RT_OK) = success,
    *                  - negative return value is error
    */
   virtual int parseString (const OSUTF8CHAR* dtString);

   /**
    * This method prints the datetime value to standard output.
    *
    * @param name - Name of generated string variable.
    */
   void print (const char* name);

   /**
    * This method sets the date and time fields to the values of
    * current date and time.
    * @param buffer      OSUTF8CHAR* - pointer to Date and time string
    * @param bufsize     size_t specifies buffer size
    * @return            const OSUTF8CHAR*
    *                      returns the datetime string
    */
   virtual const OSUTF8CHAR* toString (OSUTF8CHAR* buffer, size_t bufsize);
} ;

#endif
