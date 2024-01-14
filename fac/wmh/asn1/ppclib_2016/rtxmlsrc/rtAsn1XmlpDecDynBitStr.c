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

#include "rtsrc/asn1type.h"
#include "rtxmlsrc/asn1xml.h"
#include "rtxmlsrc/rtXmlPull.h"
#include "rtxsrc/rtxMemBuf.h"

#ifndef XMLP_DECODE_SEGSIZE
   #define XMLP_DECODE_SEGSIZE 1
#endif

int rtAsn1XmlpDecDynBitStr64 (OSCTXT* pctxt, ASN1DynBitStr64* pvalue)
{
   OSXMLDataCtxt dataCtxt;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   size_t oldsize = 0;
   OSOCTET* data = 0;
   OSSIZE bitIndex = 0;
   int stat = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pXmlReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pXmlReader);

   if (!pvalue)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pvalue->numbits = 0;
   pvalue->data = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSSIZE nbytes = dataCtxt.mData.length;
         OSSIZE newsize = oldsize + nbytes;
         OSSIZE i, allocSize = newsize/8;

         if (0 != newsize % 8) allocSize++;

         stat = rtxMemBufPreAllocate (&memBuf, allocSize);
         /* memBuf.usedcnt always 0; memRealloc save buffer content*/

         if (stat < 0) {
            LOG_RTERR (pctxt, stat);
            break;
         }

         data = OSMEMBUFPTR(&memBuf);

         for (i = 0; i < nbytes; i++, oldsize++) {
            OSUTF8CHAR c = inpdata[i];

            if (bitIndex % 8 == 0)
               data[bitIndex / 8] = 0;

            if (c == '1')
               rtxSetBit (data, newsize, bitIndex);
            else if (c != '0') {
               stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               break;
            }

            bitIndex++;
         }

         if (stat >= 0) {
            stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
            if (stat < 0)
               LOG_RTERR (pctxt, stat);
         }
      } while (stat > 0);
   }
   else if (stat < 0)
      LOG_RTERR (pctxt, stat);

   if (stat > 0)
      stat = 0;

   if (stat == 0) {
      pvalue->numbits = bitIndex;
      pvalue->data = data;
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}

int rtAsn1XmlpDecDynBitStr (OSCTXT* pctxt, ASN1DynBitStr* pvalue)
{
   ASN1DynBitStr64 bitstr64;
   int ret = rtAsn1XmlpDecDynBitStr64 (pctxt, &bitstr64);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   if (sizeof(bitstr64.numbits) > 4 && bitstr64.numbits > OSUINT32_MAX)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   pvalue->numbits = (OSUINT32) bitstr64.numbits;
   pvalue->data = bitstr64.data;

   return 0;
}
