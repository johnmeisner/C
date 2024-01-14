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

static const OSUINT32 maxUInt32 = 0xFFFFFFFFu;
static const OSUINT32 maxUInt32_10 = 0x19999999u; /* 0x100000000/10 */
static const OSUINT32 maxUInt16 = 0xFFFFu;
static const OSUINT32 maxUInt16_10 = 0x1999u; /* 0x10000/10 */
static const OSUINT32 maxUInt8 = 0xFFu;
static const OSUINT32 maxUInt8_10 = 0x19u; /* 0x100/10 */

static int rtXmlpDecUIntLim (OSCTXT* pctxt, OSUINT32* pvalue,
   OSUINT32 maxval, OSUINT32 maxval10)
{
   size_t i;
   OSBOOL minus = FALSE;
   register OSUINT32 value = 0;
   struct OSXMLReader* pXmlReader;
   OSXMLDataCtxt dataCtxt;
   int stat = 0;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pvalue)
      *pvalue = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;

         i = 0;
         if (dataCtxt.mnChunk == 0) {
            /* Skip '+' */
            if (inpdata[i] == '+') {
               i++;
            }
            /* Skip '-' for '-0' */
            else if (inpdata[0] == '-') {
               i++;
               minus = TRUE;
            }
         }

         /* Convert Unicode characters to an integer value */

         for ( ; i < nbytes; i++) {
            if (!minus && OS_ISDIGIT (inpdata[i])) {
               OSUINT32 tm = inpdata[i] - '0';

               if (value > maxval10) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
                  break;
               }

               value *= 10;

               if (value > maxval - tm) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
                  break;
               }

               value += tm;
            }
            else if (minus && inpdata[i] == '0') ; /* -0000... */
            else {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               break;
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
   else { /* empty content */
      rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
      stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }

   if (stat >= 0) {
      stat = 0;
      if (pvalue)
         *pvalue = value;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpDecUInt (OSCTXT* pctxt, OSUINT32* pvalue)
{
   return rtXmlpDecUIntLim (pctxt, pvalue, maxUInt32, maxUInt32_10);
}

EXTXMLMETHOD int rtXmlpDecUInt8 (OSCTXT* pctxt, OSOCTET* pvalue)
{
   OSUINT32 tm;
   int stat = rtXmlpDecUIntLim (pctxt, &tm, maxUInt8, maxUInt8_10);
   if (pvalue)
      *pvalue = (OSOCTET) tm;
   return stat;
}

EXTXMLMETHOD int rtXmlpDecUInt16 (OSCTXT* pctxt, OSUINT16* pvalue)
{
   OSUINT32 tm;
   int stat = rtXmlpDecUIntLim (pctxt, &tm, maxUInt16, maxUInt16_10);
   if (pvalue)
      *pvalue = (OSUINT16) tm;
   return stat;
}

