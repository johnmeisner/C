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

static int rtXmlpDecStrList_ (OSCTXT* pctxt, OSRTDList* plist, OSBOOL xmlStr)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   int stat = 0;

   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInit (pctxt, &memBuf, 0);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* inpdata = dataCtxt.mData.value;
         size_t nbytes = dataCtxt.mData.length;
         size_t tailLen;
         const OSUTF8CHAR* ptm;

         if (plist) while (nbytes) {
#ifdef _PULL_FROM_BUFFER
            ptm = (const OSUTF8CHAR*) memchr (inpdata, ' ', nbytes);
            tailLen = (ptm) ? (ptm - inpdata) : nbytes;
#else
            OSUTF8CHAR c;
            const OSUTF8CHAR* end = inpdata + nbytes;

            ptm = inpdata;
            while (ptm != end && (c = *ptm) != ' ' && c != '\n' &&
                   c != '\r' && c != '\t')
               ptm++;

            tailLen = ptm - inpdata;
#endif
            if (tailLen) {
               stat = rtxMemBufPreAllocate (&memBuf, tailLen + 1);
               if (stat < 0) {
                  LOG_RTERR (pctxt, stat);
                  break;
               }
               rtxMemBufAppend (&memBuf, (const OSOCTET*) inpdata, tailLen);
            }

#ifdef _PULL_FROM_BUFFER
            if (ptm) {
#else
            if (ptm != end) {
#endif
               if (OSMEMBUFUSEDSIZE(&memBuf)) {
                  *OSMEMBUFENDPTR(&memBuf) = 0;

                  if (xmlStr) {
                     OSXMLSTRING* pdata = OSRTALLOCTYPEZ (pctxt, OSXMLSTRING);
                     if (pdata == 0) {
                        stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
                        break;
                     }

                     pdata->value = (const OSUTF8CHAR*) OSMEMBUFPTR(&memBuf);
                     pdata->cdata = FALSE;

                     if (!rtxDListAppend (pctxt, plist, pdata))
                     {
                        stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
                        break;
                     }
                  }
                  else {
                     if (!rtxDListAppend (pctxt, plist, OSMEMBUFPTR(&memBuf)))
                     {
                        stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
                        break;
                     }
                  }

                  rtxMemBufInit (pctxt, &memBuf, 0);
               }

            tailLen++;
            }

            inpdata += tailLen;
            nbytes -= tailLen;
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

   if (plist && stat >= 0 && OSMEMBUFUSEDSIZE(&memBuf) > 0) {
      *OSMEMBUFENDPTR(&memBuf) = 0;

      if (xmlStr) {
         OSXMLSTRING* pdata = OSRTALLOCTYPEZ (pctxt, OSXMLSTRING);
         if (pdata == 0)
            stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
         else {
            pdata->value = (const OSUTF8CHAR*) OSMEMBUFPTR(&memBuf);
            pdata->cdata = dataCtxt.mbCDATA;
            dataCtxt.mbCDATA = FALSE;

            if (!rtxDListAppend (pctxt, plist, pdata))
               stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
         }
      }
      else {
         if (!rtxDListAppend (pctxt, plist, OSMEMBUFPTR(&memBuf)))
            stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
      }

      rtxMemBufInit (pctxt, &memBuf, 0);
   }

   if (stat > 0)
      stat = 0;

   rtxMemBufFree (&memBuf);

   if (plist && stat < 0)
      rtxDListFreeAll (pctxt, plist);

   return stat;
}

EXTXMLMETHOD int rtXmlpDecStrList (OSCTXT* pctxt, OSRTDList* plist)
{
   return rtXmlpDecStrList_ (pctxt, plist, FALSE);
}

EXTXMLMETHOD int rtXmlpDecXmlStrList (OSCTXT* pctxt, OSRTDList* plist)
{
   return rtXmlpDecStrList_ (pctxt, plist, TRUE);
}

