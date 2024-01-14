/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxsrc/OSRTString.h"
#include "rtxsrc/rtxCtype.h"

EXTRTMETHOD OSRTString::OSRTString ()
{
   mpValue = new char [1];
   *mpValue = '\0';
}

EXTRTMETHOD OSRTString::OSRTString (const char* strval)
{
   if (0 != strval) {
      size_t bufsiz = OSCRTLSTRLEN(strval)+1;
      mpValue = new char [bufsiz];
      if (0 != mpValue)
         rtxStrcpy (mpValue, bufsiz, strval);
      else
      {
         mpValue = new char [1];
         *mpValue = '\0';
      }
   }
   else {
      mpValue = new char [1];
      *mpValue = '\0';
   }
}

EXTRTMETHOD OSRTString::OSRTString (const OSRTString& str)
{
   size_t bufsiz = OSCRTLSTRLEN((const char*)str.mpValue)+1;
   mpValue = new char [bufsiz];
   if (0 != mpValue)
      rtxStrcpy (mpValue, bufsiz, str.mpValue);
}

EXTRTMETHOD OSRTString::~OSRTString ()
{
   delete [] mpValue;
}

EXTRTMETHOD int OSRTString::indexOf (char ch) const
{
   if (0 != mpValue) {
      for (int i = 0; i < (int)length(); i++) {
         if (ch == mpValue[i]) return i;
      }
   }
   return -1;
}

EXTRTMETHOD void OSRTString::setValue (const char* strval)
{
   if (0 != strval) {
      delete [] mpValue;
      size_t bufsiz = OSCRTLSTRLEN(strval)+1;
      mpValue = new char [bufsiz];
      if (0 != mpValue)
         rtxStrcpy (mpValue, bufsiz, strval);
   }
}

EXTRTMETHOD void OSRTString::setValue (const OSUTF8CHAR* strval)
{
   if (0 != strval) {
      delete [] mpValue;
      size_t bufsiz = OSCRTLSTRLEN((const char*)strval)+1;
      mpValue = new char [bufsiz];
      if (0 != mpValue)
         rtxStrcpy (mpValue, bufsiz, (const char*)strval);
   }
}

bool OSRTString::toInt (OSINT32& value) const
{
   static const OSINT32 maxInt32 = 0x7FFFFFFF;
   static const OSINT32 maxInt32_10 = 0xCCCCCCC; /* 0x80000000/10 */

   size_t i = 0;
   const char* cstr = mpValue;
   bool  minus = false;
   OSINT32 maxval = maxInt32;

   value = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' or '-' first character */

   if (cstr[i] == '+') {
      i++;
   }
   else if (cstr[i] == '-') {
      minus = true;
      i++;
      maxval++; /* support for 0x80000000 */
   }

   /* Convert characters to an integer value */

   for (;;) {
      if (OS_ISDIGIT (cstr[i])) {
         OSINT32 tm = cstr[i] - '0';

         if (value > maxInt32_10) return false;

         value *= 10;

         if ((OSUINT32)value > (OSUINT32)(maxval - tm))
            return false;

         value += tm; i++;
      }
      else if (cstr[i] == '\0') {
         break;
      }
      else { // non-numeric character
         return false;
      }
   }

   /* If negative, negate number */

   if (minus) value = 0 - value;

   return (true);
}

bool OSRTString::toUInt (OSUINT32& value) const
{
   static const OSUINT32 maxUInt32 = 0xFFFFFFFFu;
   static const OSUINT32 maxUInt32_10 = 0x19999999u; /* 0x100000000/10 */

   size_t i = 0;
   const char* cstr = mpValue;

   value = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' first character */

   if (cstr[i] == '+') {
      i++;
   }

   /* Convert characters to an integer value */

   for (;;) {
      if (OS_ISDIGIT (cstr[i])) {
         OSUINT32 tm = cstr[i] - '0';

         if (value > maxUInt32_10) return false;

         value *= 10;

         if (value > maxUInt32 - tm)
            return false;

         value += tm; i++;
      }
      else if (cstr[i] == '\0') {
         break;
      }
      else { // non-numeric character
         return false;
      }
   }

   return (true);
}

#if !defined(_NO_INT64_SUPPORT)
bool OSRTString::toUInt64 (OSUINT64& value) const
{
   static const OSUINT64 maxUInt64 = OSUI64CONST(0xFFFFFFFFFFFFFFFF);
   static const OSUINT64 maxUInt64_10 = OSUI64CONST(0x1999999999999999);

   size_t i = 0;
   const char* cstr = mpValue;

   value = 0;

   /* Consume leading white space */

   while (OS_ISSPACE (cstr[i])) i++;

   /* Check for '+' first character */

   if (cstr[i] == '+') {
      i++;
   }

   /* Convert characters to an integer value */

   for (;;) {
      if (OS_ISDIGIT (cstr[i])) {
         OSUINT32 tm = cstr[i] - '0';

         if (value > maxUInt64_10) return false;

         value *= 10;

         if (value > maxUInt64 - tm)
            return false;

         value += tm; i++;
      }
      else if (cstr[i] == '\0') {
         break;
      }
      else { // non-numeric character
         return false;
      }
   }

   return (true);
}
#endif

bool OSRTString::toSize (OSSIZE& value) const
{
#if !defined(_NO_INT64_SUPPORT)
   if (sizeof(value) == 8) { // 64-bit
      return toUInt64 ((OSUINT64&)value);
   } else
#endif
   if (sizeof(value) == 4) { // 32-bit
      return toUInt ((OSUINT32&)value);
   }
   else { // unsupported size value type
      return false;
   }
}

OSRTString& OSRTString::operator= (const OSRTString& original)
{
   setValue (original.mpValue);
   return *this;
}

