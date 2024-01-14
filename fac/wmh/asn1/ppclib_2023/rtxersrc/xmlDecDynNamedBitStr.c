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

#include "xed_common.hh"
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxToken.h"

int xmlDecDynNamedBitStr
  (OSCTXT* pctxt, ASN1DynBitStr* pvalue,
   const XmlNamedBitsDict* pBitDict)
{
   OSTOKENCTXT tokCtxt;
   const OSUTF8CHAR* pToken;
   OSOCTET* data;
   OSUINT32 nbits = 0, bufsize;
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   OSBOOL namedstring = 0;    /* named-bits or binary digits value */

   /* check values is binary digits (1 & 0) */
   while (*inpdata != 0) {
      char c = (char)*inpdata;
      if (!OS_ISSPACE (c)) {
         if (c != '0' && c != '1') { namedstring= 1; break; }
      }
      inpdata ++;
   }

   if(!namedstring) {
      /* Decode binary digit value */
      return xmlDecDynBitStr(pctxt, pvalue);
   }

   /* Decode Named Bits value */
   /* first, determine number of bits in the buffer */

   pToken = rtxTokGetFirst (&tokCtxt, (OSUTF8CHAR*)ASN1BUFPTR (pctxt),
                            pctxt->buffer.size + 1, " \r\n\t\0", 5);

   while (pToken != 0) {
      int i;
      for (i = 0; pBitDict[i].name != 0; i++) {
         if (xerCmpText (pToken, pBitDict[i].name)) {
            if (nbits <= pBitDict[i].bitnum)
               nbits = pBitDict[i].bitnum + 1;
            break;
         }
      }
      if (pBitDict[i].name == 0) {
         return LOG_RTERR (pctxt, RTERR_IDNOTFOU);
      }

      pToken = rtxTokGetNext (&tokCtxt);
   }

   /* allocate memory for bit string */

   bufsize = (nbits + 7) / 8;
   data = rtxMemAllocArray (pctxt, bufsize, OSOCTET);
   if (data != 0) {
      pvalue->data = data;
      pvalue->numbits = nbits;

      /* Convert characters to a bit string value */

      pToken = rtxTokGetFirst (&tokCtxt, (OSUTF8CHAR*)ASN1BUFPTR (pctxt),
                              pctxt->buffer.size + 1, " \r\n\t\0", 5);

      while (pToken != 0) {
         int i;
         for (i = 0; pBitDict[i].name != 0; i++) {
            if (xerCmpText (pToken, pBitDict[i].name)) {
               rtxSetBit (data, bufsize * 8, pBitDict[i].bitnum);
               break;
            }
         }
         pToken = rtxTokGetNext (&tokCtxt);
      }
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   return 0;
}

