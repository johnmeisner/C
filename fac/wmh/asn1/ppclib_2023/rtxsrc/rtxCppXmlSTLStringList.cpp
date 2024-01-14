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

#include "rtxCppXmlSTLStringList.h"

OSXMLSTLStringListClass::OSXMLSTLStringListClass(const OSXMLSTLStringListClass& orig)
{
   mElemList = orig.mElemList;
}

OSXMLSTLStringListClass& OSXMLSTLStringListClass::operator= (const OSXMLSTLStringListClass& orig)
{
   mElemList.release();
   mElemList = orig.mElemList;
   return *this;
}

void OSXMLSTLStringListClass::append(OSXMLSTLStringClass* pdata)
{
   mElemList.append(pdata);
}

void OSXMLSTLStringListClass::appendCopy(OSXMLSTLStringClass* pdata)
{
   mElemList.appendCopy(pdata);
}

OSRTBaseType* OSXMLSTLStringListClass::clone() const
{
   return new OSXMLSTLStringListClass(*this);
}

OSXMLSTLStringClass* OSXMLSTLStringListClass::getItem(int idx)
{
   return (OSXMLSTLStringClass*)mElemList.getItem(idx);
}

#endif
