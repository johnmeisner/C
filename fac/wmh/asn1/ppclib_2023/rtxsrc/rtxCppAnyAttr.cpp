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

#include "rtxsrc/rtxCppAnyAttr.h"

OSAnyAttrClass::OSAnyAttrClass ()
{
   name  = (const OSUTF8CHAR*)0;
   value = (const OSUTF8CHAR*)0;
}


OSAnyAttrClass::OSAnyAttrClass
(const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue)
{
   name = NULL;
   value = NULL;
   copyValue(pname, pvalue);
}

OSAnyAttrClass::OSAnyAttrClass
(const char* pname, const char* pvalue)
{
   name = NULL;
   value = NULL;
   copyValue((const OSUTF8CHAR*)pname, (const OSUTF8CHAR*)pvalue);
}


OSAnyAttrClass::OSAnyAttrClass (OSAnyAttr& os)
{
   copyValue(os.name, os.value);
}

OSAnyAttrClass::OSAnyAttrClass (const OSAnyAttrClass& os):
   OSRTBaseType (os)
{
   name = NULL;
   value = NULL;
   copyValue (os.name, os.value);
}

OSAnyAttrClass::~OSAnyAttrClass ()
{
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, name);
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, value);
}

void OSAnyAttrClass::copyValue
(const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue)
{
   if (0 != this->name)
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, this->name);
   if (0 != this->value)
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, this->value);
   if (pname != 0) {
      size_t len = rtxUTF8LenBytes(pname);
      OSUTF8CHAR* pbuf = new OSUTF8CHAR [len+1];
      memcpy ((void*)pbuf, pname, len);
      pbuf[len] = 0;
      this->name = pbuf;
   }
   else
      this->name = 0;
   if (pvalue != 0) {
      size_t len = rtxUTF8LenBytes(pvalue);
      OSUTF8CHAR* pbuf = new OSUTF8CHAR [len+1];
      memcpy ((void*)pbuf, pvalue, len);
      pbuf[len] = 0;
      this->value = pbuf;
   }
   else
      this->value = 0;
}

void OSAnyAttrClass::setValue
(const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue)
{
   copyValue(pname, pvalue);
}

OSAnyAttrClass& OSAnyAttrClass::operator= (const OSAnyAttrClass& original)
{
   // test for assignment to self
   if (this != &original)
      copyValue (original.name, original.value);
   return *this;
}

void OSAnyAttrClass::setValue
(const OSUTF8CHAR* pname, const OSUTF8CHAR* pvalue,
 size_t namebytes, size_t valuebytes)
{
   if (0 != this->name)
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, this->name);
   if (0 != this->value)
      delete [] OSRTSAFECONSTCAST (OSUTF8CHAR*, this->value);
   if (pname != 0) {
      size_t len = (namebytes == 0) ? rtxUTF8LenBytes(pname) : namebytes;
      OSUTF8CHAR* pbuf = new OSUTF8CHAR [len+1];
      memcpy ((void*)pbuf, pname, len);
      pbuf[len] = 0;
      this->name = pbuf;
   }
   else
      this->name = 0;
   if (pvalue != 0) {
      size_t len = (valuebytes == 0) ? rtxUTF8LenBytes(pvalue) : valuebytes;
      OSUTF8CHAR* pbuf = new OSUTF8CHAR [len+1];
      memcpy ((void*)pbuf, pvalue, len);
      pbuf[len] = 0;
      this->value = pbuf;
   }
   else
      this->value = 0;
}

