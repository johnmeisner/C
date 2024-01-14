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

static const char trueVal[4] = {'t', 'r', 'u', 'e'};
static const char falseVal[5] = {'f', 'a', 'l', 's', 'e'};

EXTXMLMETHOD int rtXmlpDecBool (OSCTXT* pctxt, OSBOOL* pvalue)
{
   size_t pos = 0;
   struct OSXMLReader* pXmlReader;
   OSXMLDataCtxt dataCtxt;
   int stat = 0;
   const char* curVal = 0;
   size_t curValLen = 1;
   OSBOOL value = FALSE;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pvalue)
      *pvalue = FALSE;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;
         size_t i = 0;

         if (pos == 0) { /* start match */
            pos = 1;
            i = 1;

            if (inpdata[0] == '0' && nbytes == 1)
               value = FALSE;
            else if (inpdata[0] == '1' && nbytes == 1)
               value = TRUE;
            else if ((inpdata[0] == 'f') && (nbytes <= sizeof(falseVal))) {
               value = FALSE;
               curVal = falseVal;
               curValLen = sizeof(falseVal);
            }
            else if ((inpdata[0] == 't') && (nbytes <= sizeof(trueVal))) {
               value = TRUE;
               curVal = trueVal;
               curValLen = sizeof(trueVal);
            }
            else {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
            }
         }

         if (stat >= 0 && pos > 0) { /* continue match */
            if (nbytes - i + pos <= curValLen) {
               while (i < nbytes) {
                  if (curVal[pos++] != inpdata[i++]) {
                     rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt,
                                              nbytes - i + 1);
                     stat = LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
                     break;
                  }
               }
            }
            else {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
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
      if (pos != curValLen) {
         rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 0);
         stat = LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
      }
      else if (pvalue)
         *pvalue = value;
   }

   return stat;
}

