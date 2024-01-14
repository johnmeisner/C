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

#include "xee_common.hh"
#include "rtxsrc/rtxBitString.h"

int xmlEncBitStr (OSCTXT* pctxt,
                  XmlNamedBitsDict *namedbits,
                  OSSIZE noofnamedbits,
                  OSSIZE nbits,
                  const OSOCTET* data,
                  const char* elemName,
                  ASN1StrType outputType)
{
   const char* attributes = 0;
   int stat;

   if (0 == elemName) elemName = "BIT_STRING";

   if (ASN1HEX == outputType) {
      attributes = "form=\"hex\"";
   }

   stat = xerEncStartElement (pctxt, elemName, attributes);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (nbits > 0) {
      OSUINT32 i = 0;
      OSBOOL namedbitspresent = FALSE;

      for(i = 0; i < noofnamedbits; i ++) {
         if(namedbits[i].bitnum < nbits &&
            rtxTestBit(data, nbits, namedbits[i].bitnum)) {

            if(namedbitspresent) { xerCopyText (pctxt, " "); }
            else { namedbitspresent = TRUE; }

            stat = xerCopyText (pctxt, namedbits[i].name);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
      }

      if(namedbitspresent == FALSE) {
         switch (outputType) {
         case ASN1BIN:
            stat = xerEncBinStrValue (pctxt, nbits, data);
            break;
         case ASN1HEX:
            stat = xerEncHexStrValue (pctxt, nbits, data);
            break;
         default:
            stat = RTERR_INVPARAM;
         }
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
