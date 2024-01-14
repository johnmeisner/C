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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

EXTXMLMETHOD int rtXmlpGetAttributeID (const OSXMLStrFragment* attrName,
                          OSINT16 nsidx,
                          size_t nAttr,
                          const OSXMLAttrDescr attrNames[],
                          OSUINT32 attrPresent[])
{
   size_t i, api = 0;
   OSUINT32 mask = 0x1;

   for (i = 0; i < nAttr; i++) {

      if (!(attrPresent[api] & mask)
          &&
          (nsidx == -2 ||
             attrNames[i].nsidx == -1 ||
             nsidx == attrNames[i].nsidx)
          &&
          (attrNames[i].localName.length == 0 ||
             attrName->length == attrNames[i].localName.length)
          &&
          rtxUTF8Strncmp (attrName->value, attrNames[i].localName.value,
                          attrName->length) == 0)
      {
         if (attrPresent[api] & mask)
            return XML_E_DUPLATTR;
         attrPresent[api] |= mask;
         return (int) i;
      }
      if (mask == 0x80000000) {
         mask = 0x1;
         api++;
      }
      else
         mask <<= 1;
   }
   return RTERR_INVATTR;
}


