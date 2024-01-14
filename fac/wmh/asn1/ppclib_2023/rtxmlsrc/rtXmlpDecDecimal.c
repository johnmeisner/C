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

#include <math.h>
#include <limits.h>
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"

EXTXMLMETHOD int rtXmlpDecDecimal (OSCTXT* pctxt, OSREAL* pvalue,
                     int totalDigits, int fractionDigits)
{
   size_t i;
   OSBOOL minus = FALSE;
   OSREAL value = 0.;
   OSREAL fracPos = 0.1;

   OSBOOL significantDigit = FALSE;
   int digits = 0;
   int fracDigits = 0;
   int stat = 0;

   struct OSXMLReader* pXmlReader;
   OSXMLDataCtxt dataCtxt;

   int mode = 0;
   /* 0 - sign, 1 - int, 2 - frac */

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if ( totalDigits == 0 ||
       (totalDigits > 0 && fractionDigits > totalDigits))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (totalDigits < 0)
      totalDigits = INT_MAX;

   if (fractionDigits < 0)
      fractionDigits = INT_MAX;

   if (pvalue)
      *pvalue = 0.;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;

         i = 0;
         while (i < nbytes && stat >= 0) {
            switch (mode) {
            case 0: /* sign */
               if (inpdata[i] == '+') {
                  i++;
                  mode = 1;
               }
               else if (inpdata[i] == '-') {
                  minus = TRUE;
                  i++;
                  mode = 1;
               }
               else if (OS_ISDIGIT (inpdata[i])) {
                  mode = 1;
               }
               else if (inpdata[i] == '.') {
                  i++;
                  mode = 2;
               }
               else {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               }
               break;
            case 1: /* integer */
               if (OS_ISDIGIT (inpdata[i])) {
                  OSINT32 tm = inpdata[i] - '0';
                  i++;

                  if (tm > 0)
                     significantDigit = TRUE;

                  if (significantDigit && (++digits > totalDigits)) {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVREAL);
                  }

                  value = value * 10. + tm;
               }
               else if (inpdata[i] == '.') {
                  i++;
                  mode = 2;
               }
               else {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               }
               break;
            case 2: /* fractional */
               if (OS_ISDIGIT (inpdata[i])) {
                  OSINT32 tm = inpdata[i] - '0';
                  i++;

                  digits++;
                  fracDigits++;

                  if ((tm > 0) && ((digits > totalDigits) ||
                                   (fracDigits > fractionDigits))) {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i);
                     stat = LOG_RTERRNEW (pctxt, RTERR_INVREAL);
                  }

                  value += tm * fracPos;
                  fracPos *= 0.1;
               }
               else {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               }
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
   else {/* empty content */
      rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
      stat = LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }

   if (stat >= 0) {
      stat = 0;
      /* If negative, negate number */
      if (minus)
         value = -value;

      if (pvalue)
         *pvalue = value;
   }

   return stat;
}
