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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.h"

EXTXMLMETHOD int rtSaxGetAttributeID (const OSUTF8CHAR* attrName,
                         size_t nAttr,
                         const OSUTF8CHAR* attrNames[],
                         OSUINT32 attrPresent[])
{
   size_t i, api = 0;
   OSUINT32 mask = 0x1;

   for (i = 0; i < nAttr; i++) {

      if (!(attrPresent[api] & mask) &&
          rtXmlCmpQName (attrName, attrNames[i], 0))
      {
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


