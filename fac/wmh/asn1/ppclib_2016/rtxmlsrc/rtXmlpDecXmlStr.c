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
#include "rtxmlsrc/rtXmlPull.hh"
#include "rtxsrc/rtxMemBuf.h"

#ifndef XMLP_DECODE_SEGSIZE
   #define XMLP_DECODE_SEGSIZE 1
#endif

EXTXMLMETHOD int rtXmlpDecUTF8Str (OSCTXT* pctxt, OSUTF8CHAR* out, size_t max_len)
{
   OSXMLDataCtxt dataCtxt;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   int stat = 0;
   size_t copied;
   OSUTF8CHAR* msg = out;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, pXmlReader->mStringWhiteSpaceMode);

   /* leave space for null terminator */
   --max_len;

   if (!out) {
      /* skip content */
      if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
         do {
            if (stat >= 0) {
               if (max_len < dataCtxt.mData.length) {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
                  stat = LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
               }
               else {
                  max_len -= dataCtxt.mData.length;
                  stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
                  if (stat < 0)
                     LOG_RTERR (pctxt, stat);
               }
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         } while (stat > 0 && max_len != 0);
      }
      else if (stat < 0)
         LOG_RTERR (pctxt, stat);
   }
   else if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         if (stat >= 0) {
            if (max_len < dataCtxt.mData.length) {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
               stat = LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
               copied = max_len;
               OSCRTLSAFEMEMCPY (out, max_len, dataCtxt.mData.value, copied);
               out += copied;
            }
            else {
               copied = dataCtxt.mData.length;
               OSCRTLSAFEMEMCPY (out, max_len, dataCtxt.mData.value, copied);
               out += copied;
               max_len -= copied;
               stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
               if (stat < 0)
                  LOG_RTERR (pctxt, stat);
            }
         }
         else {
            LOG_RTERR (pctxt, stat);
            break;
         }
      } while (stat > 0);

      /* Null terminate the string */
      *out = '\0';
      if (stat == 0) {
#ifndef _PULL_FROM_BUFFER
         if (pXmlReader->mStringWhiteSpaceMode != OSXMLWSM_PRESERVE) {
            char* p = (char*) out;
            while (*p) {
               char c = *p;
               if (c == '\n' || c == '\r' || c == '\t')
                  *p = ' ';

               p++;
            }
         }
#endif
         if (xmlCtxt->encodingStr &&
            OSCRTLSTRCMP ((const char*)xmlCtxt->encodingStr, OSXMLHDRUTF8) == 0)
            stat = rtxValidateUTF8 (pctxt, msg);
         if (stat < 0)
            LOG_RTERR (pctxt, stat);
      }
   }
   else if (stat < 0)
      LOG_RTERR (pctxt, stat);
   else /* empty string */
      *out = '\0';

   pXmlReader->mStringWhiteSpaceMode = OSXMLWSM_PRESERVE;

   if (stat >= 0) {
      stat = 0;
   }
   return stat;
}

EXTXMLMETHOD int rtXmlpDecXmlStr (OSCTXT* pctxt, OSXMLSTRING* outdata)
{
   OSXMLDataCtxt dataCtxt;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   int stat = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, pXmlReader->mStringWhiteSpaceMode);

   if (!outdata) {
      /* skip content */
      if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
         do {
            stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
            if (stat < 0)
               LOG_RTERR (pctxt, stat);
         } while (stat > 0);
      }

      if (stat > 0)
         stat = 0;

      return stat;
   }

   outdata->value = 0;
   outdata->cdata = FALSE;

   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         stat = rtxMemBufPreAllocate (&memBuf, dataCtxt.mData.length + 1);

         if (stat == 0) {
            rtxMemBufAppend (&memBuf, dataCtxt.mData.value,
               dataCtxt.mData.length);

            stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
            if (stat < 0)
               LOG_RTERR (pctxt, stat);
         }
         else {
            LOG_RTERR (pctxt, stat);
            break;
         }
      } while (stat > 0);

      /* Null terminate the string */
      rtxMemBufSet (&memBuf, 0, 1);

      if (stat == 0) {
#ifndef _PULL_FROM_BUFFER
         if (pXmlReader->mStringWhiteSpaceMode != OSXMLWSM_PRESERVE) {
            char* p = (char*) OSMEMBUFPTR(&memBuf);
            while (*p) {
               char c = *p;
               if (c == '\n' || c == '\r' || c == '\t')
                  *p = ' ';

               p++;
            }
         }
#endif
         if (xmlCtxt->encodingStr != 0 &&
            OSCRTLSTRCMP ((const char*)xmlCtxt->encodingStr, OSXMLHDRUTF8) == 0)
            stat = rtxValidateUTF8 (pctxt, OSMEMBUFPTR(&memBuf));
         if (stat < 0)
            LOG_RTERR (pctxt, stat);
      }
   }
   else if (stat < 0)
      LOG_RTERR (pctxt, stat);
   else /* empty string */
      rtxMemBufSet (&memBuf, 0, 1);

   pXmlReader->mStringWhiteSpaceMode = OSXMLWSM_PRESERVE;

   if (stat >= 0) {
      stat = 0;
      outdata->value = rtxUTF8Strdup (pctxt, OSMEMBUFPTR(&memBuf));
      outdata->cdata = dataCtxt.mbCDATA;
   }
   rtxMemBufFree (&memBuf);

   return stat;
}

EXTXMLMETHOD int rtXmlpDecDynUTF8Str (OSCTXT* pctxt, const OSUTF8CHAR** outdata)
{
   OSXMLSTRING xmlStr;
   int stat;

   if (outdata) {
      stat = rtXmlpDecXmlStr (pctxt, &xmlStr);
      *outdata = xmlStr.value;
   }
   else
      stat = rtXmlpDecXmlStr (pctxt, 0);

   return stat;
}
