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

#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"
#include "rtxsrc/rtxMemBuf.h"

#ifndef XMLP_DECODE_SEGSIZE
   #define XMLP_DECODE_SEGSIZE 1
#endif

EXTXMLMETHOD int rtXmlpDecDynBitString (OSCTXT* pctxt, OSDynOctStr* pvalue)
{
   OSXMLDataCtxt dataCtxt;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   size_t oldsize = 0;
   OSOCTET* data = 0;
   OSUINT32 bitIndex = 0;
   int stat = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
   }

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if (rtXmlRdFirstData (pXmlReader, &dataCtxt) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         size_t nbytes = dataCtxt.mData.length;
         size_t newsize = oldsize + nbytes;
         size_t i;

         stat = rtxMemBufPreAllocate (&memBuf, (newsize + 7) / 8);
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
               rtxSetBit (data, (OSUINT32)newsize, bitIndex);
            else if (c != '0') {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
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

   if (stat >= 0) {
      stat = 0;
      if (pvalue) {
         pvalue->numocts = (OSUINT32)((bitIndex + 7) / 8);
         pvalue->data = data;
      }
      else
         rtxMemBufFree (&memBuf);
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}


