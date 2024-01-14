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

#ifdef HAS_STL
#include <stdlib.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCppXmlSTLString.h"

using namespace std;

OSXMLSTLStringClass::OSXMLSTLStringClass () : cdata(FALSE)
{
}

OSXMLSTLStringClass::OSXMLSTLStringClass
(const OSUTF8CHAR* strval, OSBOOL cdata_) :
   value((const char*)strval), cdata(cdata_)
{
}

OSXMLSTLStringClass::OSXMLSTLStringClass
(const OSUTF8CHAR* strval, size_t nbytes, OSBOOL cdata_) :
   value((const char*)strval, nbytes), cdata(cdata_)
{
}

OSXMLSTLStringClass::OSXMLSTLStringClass (const char* strval, OSBOOL cdata_) :
   value(strval), cdata(cdata_)
{
}

OSXMLSTLStringClass::OSXMLSTLStringClass (const OSXMLSTRING& str) :
   value((const char*)str.value), cdata(str.cdata)
{
}

OSXMLSTLStringClass::OSXMLSTLStringClass (const OSXMLSTLStringClass& str) :
   value(str.value), cdata(str.cdata)
{
}

OSXMLSTLStringClass& OSXMLSTLStringClass::operator=
(const OSXMLSTLStringClass& original)
{
   if (this != &original) {
      cdata = original.cdata;
      value = original.value;
   }
   return *this;
}

OSXMLSTLStringClass& OSXMLSTLStringClass::operator= (const char* original)
{
   cdata = FALSE;
   value = original;
   return *this;
}

OSXMLSTLStringClass& OSXMLSTLStringClass::operator= (const OSUTF8CHAR* original)
{
   cdata = FALSE;
   value = (const char*) original;
   return *this;
}

void OSXMLSTLStringClass::copyValue (const OSUTF8CHAR* pString, size_t nbytes)
{
   if (0 == nbytes) {
      value = (const char*) pString;
   }
   else {
      value = "";
      value.append ((const char*) pString, nbytes);
   }
}

void OSXMLSTLStringClass::appendValue
(const OSUTF8CHAR* pString, size_t nbytes)
{
   if (0 != pString) {
      if (0 == nbytes) {
         value.append ((const char*)pString);
      }
      else {
         value.append ((const char*)pString, nbytes);
      }
   }
}
#if defined(_MSC_VER)
// this disables ' unreferenced formal parameter' warning
#pragma warning(disable: 4100)
#endif
int OSXMLSTLStringClass::decodeXML (OSCTXT* pctxt)
{
   return 0;
}

int OSXMLSTLStringClass::encodeXML
(OSRTMessageBufferIF& msgbuf, const OSUTF8CHAR* elemName, OSXMLNamespace* pNS)
{
   return 0;
}
#endif

