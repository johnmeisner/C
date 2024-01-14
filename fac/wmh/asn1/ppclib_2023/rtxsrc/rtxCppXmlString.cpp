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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCppXmlString.h"

#define MAX(a,b) (a) < (b) ? (b) : (a)

OSRTXMLString::OSRTXMLString () : mCapacityIncrement(DEFAULT_CAPACITY)
{
   value = NULL;
   cdata = FALSE;
   mCurrentBufferSize = getRequiredCapacity(1);
   mCurrentStringSize = 0;
   OSUTF8CHAR* buf = new OSUTF8CHAR [mCurrentBufferSize];
   buf[0] = '\0';
   value = buf;
}

OSRTXMLString::OSRTXMLString (const OSUTF8CHAR* strval, OSBOOL cdata_) :
   mCapacityIncrement(DEFAULT_CAPACITY)
{
   cdata = cdata_;
   newString (strval);
}

OSRTXMLString::OSRTXMLString
(const OSUTF8CHAR* strval, size_t nbytes, OSBOOL cdata_) :
   mCapacityIncrement(DEFAULT_CAPACITY)
{
   cdata = cdata_;
   newString (strval, nbytes);
}

OSRTXMLString::OSRTXMLString (const char* strval, OSBOOL cdata_) :
   mCapacityIncrement(DEFAULT_CAPACITY)
{
   cdata = cdata_;
   newString (OSUTF8(strval));
}

OSRTXMLString::OSRTXMLString (const OSXMLSTRING& str) :
   mCapacityIncrement(DEFAULT_CAPACITY)
{
   cdata = str.cdata;
   newString(str.value);
}

OSRTXMLString::OSRTXMLString (const OSRTXMLString& str) :
   OSRTBaseType (str)
{
   cdata = str.cdata;
   mCapacityIncrement = str.mCapacityIncrement;
   mCurrentBufferSize = str.mCurrentBufferSize;
   mCurrentStringSize = str.mCurrentStringSize;
   OSUTF8CHAR* buf = new OSUTF8CHAR [mCurrentBufferSize];
   OSCRTLMEMCPY (buf, str.value, mCurrentStringSize);
   buf[mCurrentStringSize] = '\0';
   value = buf;
}

OSRTXMLString& OSRTXMLString::operator=
(const OSRTXMLString& original)
{
   if (this != &original) {
      cdata = original.cdata;
      mCapacityIncrement = original.mCapacityIncrement;
      OSUTF8CHAR* buf = (OSUTF8CHAR*)value;

      if (mCurrentBufferSize < original.mCurrentBufferSize) {
         delete [] value;
         buf = new OSUTF8CHAR [original.mCurrentBufferSize];
      }

      mCurrentBufferSize = original.mCurrentBufferSize;
      mCurrentStringSize = original.mCurrentStringSize;
      OSCRTLMEMCPY (buf, original.value, mCurrentStringSize);
      buf[mCurrentStringSize] = '\0';
      value = buf;
   }
   return *this;
}

OSRTXMLString& OSRTXMLString::operator=
(const char* original)
{
   cdata = FALSE;
   copyValue (original);
   return *this;
}

OSRTXMLString& OSRTXMLString::operator=
(const OSUTF8CHAR* original)
{
   cdata = FALSE;
   copyValue (original);
   return *this;
}

OSRTXMLString::~OSRTXMLString ()
{
   delete [] value;
}

void OSRTXMLString::setValue (const OSUTF8CHAR* utf8str, size_t nbytes)
{
   copyValue (utf8str, nbytes);
}

size_t OSRTXMLString::getRequiredCapacity (const char* pString) const
{
   size_t numBytesRequired = (0 != pString) ? OSCRTLSTRLEN(pString) + 1 : 1;
   return getRequiredCapacity (numBytesRequired);
}

size_t OSRTXMLString::getRequiredCapacity (const OSUTF8CHAR* pString) const
{
   size_t numBytesRequired = (0 != pString) ? getLenInBytes(pString) + 1 : 1;
   return getRequiredCapacity (numBytesRequired);
}

size_t OSRTXMLString::getRequiredCapacity (size_t numBytesRequired) const
{
   size_t numSegments = ((numBytesRequired - 1) / mCapacityIncrement) + 1;
   return (numSegments * mCapacityIncrement);
}

size_t OSRTXMLString::getLenInBytes (const OSUTF8CHAR* xstr) const
{
   size_t len = 0;

   if (0 != xstr) {
      while (0 != xstr[len]) len++;
   }

   return len;
}

void OSRTXMLString::newString (const OSUTF8CHAR* pString, size_t nbytes)
{
   size_t srclen = (0 == nbytes) ? getLenInBytes (pString) : nbytes;
   mCurrentStringSize = srclen;
   mCurrentBufferSize = getRequiredCapacity (srclen + 1);
   mCapacityIncrement = MAX(mCapacityIncrement, mCurrentBufferSize / 2);
   OSUTF8CHAR* buf = new OSUTF8CHAR [mCurrentBufferSize];
   if (0 != buf) {
      buf[0] = '\0';
      xstrcat (buf, pString);
   }
   value = buf;
}

void OSRTXMLString::copyValue (const OSUTF8CHAR* pString, size_t nbytes)
{
   OSUTF8CHAR* buf = (OSUTF8CHAR*)value;
   if (0 != pString) {
      if (0 == nbytes) nbytes = getLenInBytes (pString);
      size_t requiredCapacity = getRequiredCapacity (nbytes + 1);

      if (requiredCapacity > mCurrentBufferSize) {
         delete [] value;
         mCurrentBufferSize = requiredCapacity;
         mCapacityIncrement = MAX(mCapacityIncrement, mCurrentBufferSize / 2);
         buf = new OSUTF8CHAR [mCurrentBufferSize];
      }
      mCurrentStringSize = nbytes;
      rtxStrncpy ((char*)buf, mCurrentBufferSize,
                  (const char*)pString, mCurrentStringSize);
      buf[mCurrentStringSize] = '\0';
   }
   else {
      buf[0] = '\0';
      mCurrentStringSize = 0;
   }

   value = buf;
}

void OSRTXMLString::xstrncat
(OSUTF8CHAR* dststr, const OSUTF8CHAR* srcstr, size_t nbytes)
{
   size_t offset = getLenInBytes (dststr);
   size_t ui;

   for (ui = 0; ui < nbytes; ui++) {
      dststr[offset + ui] = srcstr[ui];
   }

   dststr[offset + ui] = '\0';
}

void OSRTXMLString::appendValue
(const OSUTF8CHAR* pString, size_t nbytes)
{
   OSUTF8CHAR* buf = (OSUTF8CHAR*)value;
   if (0 != pString) {
      if (0 == nbytes) nbytes = getLenInBytes (pString);
      size_t requiredCapacity =
         getRequiredCapacity (nbytes + 1 + mCurrentStringSize);

      if (requiredCapacity > mCurrentBufferSize) {
         mCurrentBufferSize = requiredCapacity;
         mCapacityIncrement = MAX(mCapacityIncrement, mCurrentBufferSize / 2);
         buf = new OSUTF8CHAR [mCurrentBufferSize];
         OSCRTLSAFEMEMCPY(buf, mCurrentBufferSize, value,
            mCurrentStringSize + 1);   /* +1 to copy \0 */
         delete [] value;
      }

      OSCRTLSAFEMEMCPY(buf + mCurrentStringSize,
         mCurrentBufferSize - mCurrentStringSize, pString,
         nbytes);

      mCurrentStringSize += nbytes;
      buf[mCurrentStringSize] = '\0';
   }

   value = buf;
}

#if defined(_MSC_VER)
// this disables 'unreferenced formal parameter' warning
#pragma warning(disable: 4100)
#endif
int OSRTXMLString::decodeXML (OSCTXT* /* pctxt */)
{
   return 0;
}

int OSRTXMLString::encodeXML
(OSRTMessageBufferIF& /* msgbuf */, const OSUTF8CHAR* /* elemName */,
 OSXMLNamespace* /* pNS */)
{
   return 0;
}

