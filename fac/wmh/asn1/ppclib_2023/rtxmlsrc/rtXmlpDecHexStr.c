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

EXTXMLMETHOD int rtXmlpDecHexStr64
(OSCTXT* pctxt, OSOCTET* pvalue, OSSIZE* pnocts, OSSIZE bufsize)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   OSSIZE oldsize = 0;
   int stat = 0;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pnocts)
      *pnocts = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSSIZE nbytes = dataCtxt.mData.length;
         OSSIZE newsize = oldsize + nbytes;
         OSSIZE i;

         if ((newsize + 1) / 2 > bufsize) {
            rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
            stat = LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
         }
         else {
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

               if (pvalue) {
                  if ((oldsize & 1) == 0)
                     pvalue[oldsize >> 1] = (OSOCTET) (c << 4);
                  else
                     pvalue[oldsize >> 1] |= c;
               }
            }
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
            OSUTF8CHAR* p = pvalue + oldsize / 2;

            for (;p != pvalue; p--) {
               *p = (OSOCTET) ((p[-1] << 4) | (*p >> 4));
            }

            *p >>= 4;
         }
      }

      if (0 != pnocts)
         *pnocts = (oldsize + 1) / 2;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpDecHexStr
(OSCTXT* pctxt, OSOCTET* pvalue, OSUINT32* pnocts, OSSIZE bufsize)
{
   int ret = 0;
   if (0 != pnocts) {
      OSSIZE nocts64;
      ret = rtXmlpDecHexStr64 (pctxt, pvalue, &nocts64, bufsize);
      if (ret >= 0) {
         if (sizeof(nocts64) > 4 && nocts64 > OSUINT32_MAX)
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         else
            *pnocts = (OSUINT32)nocts64;
      }
   }
   else {
      ret = rtXmlpDecHexStr64 (pctxt, pvalue, 0, bufsize);
   }

   return (0 != ret) ? LOG_RTERR (pctxt, ret) : 0;
}

