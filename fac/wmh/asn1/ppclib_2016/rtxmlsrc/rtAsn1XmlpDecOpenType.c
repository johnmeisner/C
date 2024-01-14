/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "rtsrc/asn1type.h"
#include "rtxsrc/osMacros.h"
#include "rtxmlsrc/asn1xml.h"

EXTXMLMETHOD int rtAsn1XmlpDecOpenType (OSCTXT *pctxt, ASN1OpenType* pOpenType)
{
   OSUTF8CHAR* pUTF8Str;

   int ret = rtXmlpDecAny2 (pctxt, &pUTF8Str);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   /* Check returned string to see if contains all valid hex string
      characters.  If this is the case, assume binary data */

   if (0 != pUTF8Str) {
      size_t i, len = rtxUTF8LenBytes (pUTF8Str);
      for (i = 0; i < len; i++) {
         if (!OS_ISHEXCHAR (pUTF8Str[i])) break;
      }
      if (i < len) {
         /* Not hex string, store character data in open type structure */
         pOpenType->numocts = (OSUINT32) len;
         pOpenType->data = pUTF8Str;
      }
      else {
         /* Convert hex string to binary */
         ret = rtxHexCharsToBinCount ((const char*)pUTF8Str, len);

         if (ret > 0) {
            pOpenType->numocts = (OSUINT32) ret;
            pOpenType->data = rtxMemAlloc (pctxt, pOpenType->numocts);
            ret = (0 != pOpenType->data) ?
               rtxHexCharsToBin ((const char*)pUTF8Str, len,
                                 (OSOCTET*)pOpenType->data,
                                 pOpenType->numocts) :
               RTERR_NOMEM;
         }
         else if (0 == ret) {
            pOpenType->numocts = 0;
            pOpenType->data = 0;
         }

         rtxMemFreePtr (pctxt, pUTF8Str);
      }
   }

   return (ret >= 0) ? 0 : LOG_RTERR (pctxt, ret);
}
