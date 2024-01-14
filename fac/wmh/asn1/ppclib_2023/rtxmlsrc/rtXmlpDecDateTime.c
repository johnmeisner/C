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

static const int multiplier[4] = {1, 10, 100, 1000};
static const int tzh_multiplier[4] = {60, 600};
static const int tzm_multiplier[4] = {1, 10};

enum DecDateFrom {
   Years, Months, Days, TimeStart, TimeEnd = 8
};

static int rtXmlDecDateTimeInner (OSCTXT* pctxt, OSXSDDateTime* pvalue,
   int decDateFrom, int decDateTo)
{
   size_t i;
   OSBOOL minus = FALSE;
   OSBOOL tz_minus = FALSE;
   OSREAL frac = 0.1;
   int minusCnt = 0;
   int stat = 0;
   OSXSDDateTime value;

   struct OSXMLReader* pXmlReader;
   OSXMLDataCtxt dataCtxt;

   int mode = decDateFrom;
   /* 0 - yyyy, 1 - mm, 2 - dd, 3 - hh, 4 - mm, 5 - ss, 6 - .ss,
   7 - tzh, 8 - tzm, 9 - end */

   int idx = (decDateFrom == Years) ? 3 : 1;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (!pvalue)
      pvalue = &value;

   OSCRTLMEMSET (pvalue, 0, sizeof(OSXSDDateTime));

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;

         i = 0;
         if (dataCtxt.mnChunk == 0) {
            /* Check for '-' first character */

            if ((mode == 0) && (inpdata[i] == '-')) {
               minus = TRUE;
               i++;
            }
         }

         for ( ; i < nbytes && stat >= 0; i++) {
            OSUTF8CHAR c = inpdata[i];

            if (mode == 9) {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
               break;
            }

            if (OS_ISDIGIT (c)) {
               OSINT32 tm = c - '0';

               if ((mode == 0) && (idx < 0)) { /* year > 4 digits */
                  pvalue->year *= 10;
                  idx = 0;
               }

               if ((idx < 0) && (mode != 6)) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  break;
               }

               switch (mode) {
               case 0:
                  pvalue->year += tm * multiplier[idx--];
                  break;
               case 1:
                  if (decDateFrom == 1 && minusCnt != 2) {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                     break;
                  }

                  pvalue->mon =
                     (OSUINT8)(pvalue->mon + tm * multiplier[idx--]);
                  break;
               case 2:
                  if (decDateFrom == 2 && minusCnt != 3) {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                     break;
                  }

                  pvalue->day =
                     (OSUINT8)(pvalue->day + tm * multiplier[idx--]);
                  break;
               case 3:
                  pvalue->hour =
                     (OSUINT8)(pvalue->hour + tm * multiplier[idx--]);
                  break;
               case 4:
                  pvalue->min =
                     (OSUINT8)(pvalue->min + tm * multiplier[idx--]);
                  break;
               case 5:
                  pvalue->sec += tm * multiplier[idx--];
                  break;
               case 6:
                  pvalue->sec += tm * frac;
                  frac *= 0.1;
                  break;
               case 7:
                  pvalue->tzo += tm * tzh_multiplier[idx--];
                  break;
               case 8:
                  pvalue->tzo += tm * tzm_multiplier[idx--];
                  break;
               }
            }
            else if ((c == '-') && (idx == 1) &&
                  (decDateFrom == 1 || decDateFrom == 2) &&
                  (mode == decDateFrom)) {

               if(minusCnt <= decDateFrom)
                  minusCnt++;
               else {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
               }
            }
            else if (idx < 0) {
               if (c == '-') {
                  if (mode < 2) {
                     mode++;
                     idx = 1;
                  }
                  else if (mode == decDateTo || mode == 5 || mode == 6) {
                     mode = 7;
                     idx = 1;
                     pvalue->tz_flag = TRUE;
                     tz_minus = TRUE;
                  }
                  else {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  }
               }
               else if (c == 'T') {
                  if ((decDateFrom == 0) && (mode == 2)) {
                     mode++;
                     idx = 1;
                  }
                  else {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  }
               }
               else if (c == ':') {
                  if((mode > 2) && ((mode < 5) || (mode == 7))) {
                     mode++;
                     idx = 1;
                  }
                  else {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  }
               }
               else if (c == '.') {
                  if(mode == 5)
                     mode++;
                  else {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  }
               }
               else if (c == '+') {
                  if(mode == decDateTo || mode == 5 || mode == 6) {
                     mode = 7;
                     idx = 1;
                     pvalue->tz_flag = TRUE;
                  }
                  else {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
                  }
               }
               else if (c == 'Z') {
                  pvalue->tz_flag = TRUE;
                  mode = 9;
               }
               else {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                           nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
               }
            }
            else {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
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
      if ((idx >= 0) || ((decDateTo < TimeEnd) && (mode < decDateTo))) {
         rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
         stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
      }
   }

   if (minus) pvalue->year = -pvalue->year;
   if (tz_minus) pvalue->tzo = -pvalue->tzo;

   return stat;
}

EXTXMLMETHOD int rtXmlpDecDate (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Years, Days);
}

EXTXMLMETHOD int rtXmlpDecTime (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, TimeStart, TimeEnd);
}

EXTXMLMETHOD int rtXmlpDecDateTime (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Years, TimeEnd);
}

EXTXMLMETHOD int rtXmlpDecGYear (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Years, Years);
}

EXTXMLMETHOD int rtXmlpDecGYearMonth (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Years, Months);
}

EXTXMLMETHOD int rtXmlpDecGMonth (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Months, Months);
}

EXTXMLMETHOD int rtXmlpDecGMonthDay (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Months, Days);
}

EXTXMLMETHOD int rtXmlpDecGDay (OSCTXT* pctxt, OSXSDDateTime* pvalue)
{
   return rtXmlDecDateTimeInner (pctxt, pvalue, Days, Days);
}

