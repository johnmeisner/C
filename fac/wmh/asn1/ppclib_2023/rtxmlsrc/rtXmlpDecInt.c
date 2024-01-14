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

static const OSINT32 maxInt32 = 0x7FFFFFFF;
static const OSINT32 maxInt32_10 = 0xCCCCCCC; /* 0x80000000/10 */
static const OSINT32 maxInt16 = 0x7FFF;
static const OSINT32 maxInt16_10 = 0xCCC; /* 0x8000/10 */
static const OSINT32 maxInt8 = 0xFF;
static const OSINT32 maxInt8_10 = 0xC; /* 0x80/10 */

static int rtXmlpDecIntLim (OSCTXT* pctxt, OSINT32* pvalue,
   OSINT32 maxval, OSINT32 maxval10)
{
   size_t i;
   OSBOOL minus = FALSE;
   register OSINT32 value = 0;
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
            /* Check for '+' or '-' first character */

            if (inpdata[i] == '+') {
               i++;
            }
            else if (inpdata[i] == '-') {
               minus = TRUE;
               maxval++; /* support for 0x80000000 */
               i++;
            }
         }

         /* Convert Unicode characters to an integer value */

         for ( ; i < nbytes; i++) {
            if (OS_ISDIGIT (inpdata[i])) {
               OSINT32 tm = inpdata[i] - '0';

               if (value > maxval10) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
                  break;
               }

               value *= 10;

               if ((OSUINT32)value > (OSUINT32)(maxval - tm)) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
                  break;
               }

               value += tm;
            }
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
      /* If negative, negate number */
      if (pvalue) {
         if (minus) *pvalue = -value;
         else *pvalue = value;
      }
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpDecInt (OSCTXT* pctxt, OSINT32* pvalue)
{
   return rtXmlpDecIntLim (pctxt, pvalue, maxInt32, maxInt32_10);
}

EXTXMLMETHOD int rtXmlpDecInt8 (OSCTXT* pctxt, OSINT8* pvalue)
{
   OSINT32 tm;
   int stat = rtXmlpDecIntLim (pctxt, &tm, maxInt8, maxInt8_10);
   if (pvalue)
      *pvalue = (OSINT8) tm;
   return stat;
}

EXTXMLMETHOD int rtXmlpDecInt16 (OSCTXT* pctxt, OSINT16* pvalue)
{
   OSINT32 tm;
   int stat = rtXmlpDecIntLim (pctxt, &tm, maxInt16, maxInt16_10);
   if (pvalue)
      *pvalue = (OSINT16) tm;
   return stat;
}

