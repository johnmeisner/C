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

#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"
#include "rtxsrc/rtxMemBuf.h"

#ifndef XMLP_DECODE_SEGSIZE
   #define XMLP_DECODE_SEGSIZE 1
#endif

EXTXMLMETHOD int rtXmlpDecBigInt (OSCTXT* pctxt, const OSUTF8CHAR** pvalue)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   int stat = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pvalue)
      *pvalue = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;
         size_t i;
         OSOCTET* dst;

         stat = rtxMemBufPreAllocate (&memBuf, dataCtxt.mData.length + 1);

         if (stat < 0) {
            LOG_RTERR (pctxt, stat);
            break;
         }

         dst = OSMEMBUFENDPTR(&memBuf);
         i = 0;

         if (((OSMEMBUFUSEDSIZE(&memBuf) == 0) && (inpdata[0] == '-')) ||
             ((OSMEMBUFUSEDSIZE(&memBuf) == 0) && (inpdata[0] == '+'))) {
            *dst++ = *inpdata++;
            i++;
         }

         for (; i < nbytes; i++) {
            OSUTF8CHAR c = *inpdata++;
            if (!OS_ISDIGIT (c)) {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
               break;
            }
            *dst++ = c;
         }

         memBuf.usedcnt = (OSUINT32) (dst - OSMEMBUFPTR(&memBuf));

         if (stat >= 0) {
            stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
            if (stat < 0)
               LOG_RTERR (pctxt, stat);
         }
      } while (stat > 0);

      /* Null terminate the string */
      rtxMemBufSet (&memBuf, 0, 1);
   }
   else if (stat < 0)
      LOG_RTERR (pctxt, stat);
   else { /* empty content */
      rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
      stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }

   if (stat >= 0) {
      stat = 0;
      if (pvalue)
         *pvalue = OSMEMBUFPTR(&memBuf);
      else
         rtxMemBufFree (&memBuf);
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}

