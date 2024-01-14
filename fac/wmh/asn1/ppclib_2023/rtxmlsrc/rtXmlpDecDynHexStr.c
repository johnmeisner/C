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

EXTXMLMETHOD int rtXmlpDecDynHexStr64 (OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   size_t oldsize = 0;
   OSOCTET* data = 0;
   int stat = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
   }

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;
         size_t newsize = oldsize + nbytes;
         size_t i;

         stat = rtxMemBufPreAllocate (&memBuf, (newsize + 1) / 2);
         /* memBuf.usedcnt always 0; memRealloc save buffer content*/

         if (stat < 0) {
            LOG_RTERR (pctxt, stat);
            break;
         }

         data = OSMEMBUFPTR(&memBuf);

         for (i = 0; i < nbytes; i++, oldsize++) {
            OSUTF8CHAR c = inpdata[i];
            if (c >= '0' && c <= '9')
               c -= '0';
            else if (c >= 'a' && c <= 'f')
               c -= 'a' - 10;
            else if (c >= 'A' && c <= 'F')
               c -= 'A' - 10;
            else {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVHEXS);
               break;
            }

            if ((oldsize & 1) == 0)
               data[oldsize >> 1] = (OSOCTET) (c << 4);
            else
               data[oldsize >> 1] |= c;
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
         if ((oldsize & 1) != 0) {/* shift hex str (211 -> 0211) */
            OSUTF8CHAR* p = data + oldsize / 2;

            for (;p != data; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }

         pvalue->numocts = (oldsize + 1) / 2;
         pvalue->data = data;
      }
      else
         rtxMemBufFree (&memBuf);
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}

EXTXMLMETHOD int rtXmlpDecDynHexStr (OSCTXT* pctxt, OSDynOctStr* pvalue)
{
   OSDynOctStr64 octstr64;
   int ret;

   if (pvalue) {
      pvalue->numocts = 0;
      pvalue->data = 0;
   }

   ret = rtXmlpDecDynHexStr64 (pctxt, &octstr64);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   if (sizeof(octstr64.numocts) > 4 && octstr64.numocts > OSUINT32_MAX)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   if ( pvalue )
   {
      pvalue->numocts = (OSUINT32) octstr64.numocts;
      pvalue->data = octstr64.data;
   }

   return 0;
}
