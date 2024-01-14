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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxXmlQName.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

EXTRTMETHOD OSXMLFullQName* rtxNewFullQName
(OSCTXT* pctxt, const OSUTF8CHAR* localName, const OSUTF8CHAR* prefix,
 const OSUTF8CHAR* nsuri)
{
   OSXMLFullQName* pqname = rtxMemAllocType (pctxt, OSXMLFullQName);
   if (0 != pqname) {
      pqname->nsPrefix = rtxUTF8StrRefOrDup (pctxt, prefix);
      pqname->nsURI = rtxUTF8StrRefOrDup (pctxt, nsuri);
      pqname->localName = rtxUTF8StrRefOrDup (pctxt, localName);
   }
   return pqname;
}

EXTRTMETHOD OSXMLFullQName* rtxNewFullQNameDeepCopy
(OSCTXT* pctxt, const OSXMLFullQName* pqname)
{
   if (0 != pqname) {
      OSXMLFullQName* pNewQName = rtxMemAllocType (pctxt, OSXMLFullQName);
      if (0 != pNewQName) {
         pNewQName->nsPrefix = rtxUTF8Strdup (pctxt, pqname->nsPrefix);
         pNewQName->nsURI = rtxUTF8Strdup (pctxt, pqname->nsURI);
         pNewQName->localName = rtxUTF8Strdup (pctxt, pqname->localName);
      }
      return pNewQName;
   }
   else return 0;
}

EXTRTMETHOD void rtxQNameDeepCopy
(OSCTXT* pctxt, OSXMLFullQName* pdest, const OSXMLFullQName* psrc)
{
   if (0 != psrc && 0 != pdest) {
      pdest->nsPrefix = rtxUTF8Strdup (pctxt, psrc->nsPrefix);
      pdest->nsURI = rtxUTF8Strdup (pctxt, psrc->nsURI);
      pdest->localName = rtxUTF8Strdup (pctxt, psrc->localName);
   }
}

EXTRTMETHOD void rtxQNameFreeMem
(OSCTXT* pctxt, OSXMLFullQName* pqname, OSBOOL dynamic)
{
   if (0 != pqname) {
      int refcnt = (dynamic) ? rtxMemAutoPtrUnref (pctxt, pqname) : 0;
      if (refcnt == 0) {
         rtxMemFreePtr (pctxt, pqname->nsPrefix);
         rtxMemFreePtr (pctxt, pqname->nsURI);
         rtxMemFreePtr (pctxt, pqname->localName);
      }
   }
}

EXTRTMETHOD OSUINT32 rtxQNameHash (const OSXMLFullQName* pqname)
{
   OSUINT32 hashCode = rtxUTF8StrHash (pqname->localName);
   if (!OS_ISEMPTY (pqname->nsURI)) {
      hashCode += rtxUTF8StrHash (pqname->nsURI);
   }
   return hashCode;
}

EXTRTMETHOD OSBOOL rtxQNamesEqual
(const OSXMLFullQName* pqname1, const OSXMLFullQName* pqname2)
{
   if (pqname1 == pqname2) return TRUE;
   else return (OSBOOL)
      (rtxUTF8StrEqual (pqname1->nsURI, pqname2->nsURI) &&
       rtxUTF8StrEqual (pqname1->localName, pqname2->localName));
}

EXTRTMETHOD const OSUTF8CHAR* rtxQNameToString
(const OSXMLFullQName* pqname, OSUTF8CHAR* buffer, OSUINT32 bufsiz)
{
   OSUINT32 i = 0, j;
   if (0 == bufsiz || 0 == buffer || 0 == pqname)
      return 0;

   if (!OS_ISEMPTY(pqname->nsURI)) {
      if (i < bufsiz) {
         buffer[i++] = '{';
      }
      for (j = 0; j < rtxUTF8LenBytes (pqname->nsURI); j++) {
         if (i < bufsiz) {
            buffer[i++] = pqname->nsURI[j];
         }
         else break;
      }
      if (i < bufsiz) {
         buffer[i++] = '}';
      }
   }
   if (!OS_ISEMPTY(pqname->localName)) {
      for (j = 0; j < rtxUTF8LenBytes (pqname->localName); j++) {
         if (i < bufsiz) {
            buffer[i++] = pqname->localName[j];
         }
         else break;
      }
   }

   if (i >= bufsiz) i = bufsiz - 1;
   buffer[i] = '\0';

   return buffer;
}

