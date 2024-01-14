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

#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"

EXTXMLMETHOD int rtSaxTestAttributesPresent (OSCTXT* pctxt,
   const OSUINT32* attrPresent, const OSUINT32* reqAttrMask,
   const OSUTF8CHAR* const* attrNames, size_t numOfAttrs,
   const char* parentTypeName)
{
   size_t i;
   const size_t ndwords = (numOfAttrs + 31)/32;
   int stat = 0;

   for (i = 0; i < ndwords; i++) {
      const OSUINT32 reqAbsentMask =
         (reqAttrMask[i] & attrPresent[i]) ^ reqAttrMask[i];
      if (reqAbsentMask != 0) {
         unsigned j, mask = 0x1;

         /* required and present masks not match, find mismatches and
            generate errors to pctxt */

         for (j = 0; j < 32; j++, mask <<= 1) {
            if ((reqAbsentMask & mask) != 0) {
               char buf[512];
               rtxStrcpy (buf, sizeof(buf), parentTypeName);
               rtxStrcat (buf, sizeof(buf), (const char*)attrNames[j + i*32]);

               stat = LOG_RTERRNEW (pctxt, RTERR_ATTRMISRQ);
               rtxErrAddStrParm (pctxt, buf);
               rtSaxIncErrors (pctxt);
            }
         }
      }
   }

   return stat;
}


