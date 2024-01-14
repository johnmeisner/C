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

#include "rtsrc/asn1type.h"
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtsrc/asn1type.h"
#include "rtxmlsrc/asn1xml.h"

#define BUFF_SZ 32

static int AppendBitStrToList (OSCTXT* pctxt, OSRTDList* plist,
   OSOCTET* str, size_t sz)
{
   int stat = 0;
   ASN1DynBitStr* pdata;
   OSRTDListNode* pnode = (OSRTDListNode*)
      rtxMemAlloc (pctxt, sizeof(ASN1DynBitStr) + sizeof(OSRTDListNode));

   if (pnode == 0) {
      stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }

   pdata = (ASN1DynBitStr*) (pnode + 1);
   stat = rtUTF8StrnToASN1DynBitStr (pctxt, (OSUTF8CHAR*)str, sz, pdata);

   if (stat == 0) {
      pnode->data = pdata;
      rtxDListAppendNode (plist, pnode);
   }
   else
      rtxMemFreePtr (pctxt, pnode);

   return stat;
}

int rtXmlpDecListOfASN1DynBitStr (OSCTXT* pctxt, OSRTDList* plist)
{
   OSXMLDataCtxt dataCtxt;
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pXmlReader;
   int stat = 0;

   OSOCTET buffer[BUFF_SZ];
   OSRTMEMBUF memBuf;
   size_t buffPos = 0;
   size_t buffSz = BUFF_SZ;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pXmlReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pXmlReader);

   if (!plist)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInitBuffer (pctxt, &memBuf, buffer, BUFF_SZ, 1);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         size_t nbytes = dataCtxt.mData.length;
         size_t i;

         for (i = 0; i < nbytes; i++) {
            OSOCTET c = inpdata[i];

            if (c == ' ') {
               stat = AppendBitStrToList (pctxt, plist, OSMEMBUFPTR(&memBuf),
                  buffPos);

               if (stat < 0)
                  LOG_RTERR (pctxt, stat);

               rtxMemBufReset (&memBuf);
               buffPos = 0;
            }
            else {
               if (buffPos == buffSz) { /* enlarge buffer */
                  buffSz *= 2;
                  stat = rtxMemBufPreAllocate (&memBuf, buffSz);
                  /* memBuf.usedcnt always 0; memRealloc save buffer content*/

                  if (stat < 0) {
                     LOG_RTERR (pctxt, stat);
                     break;
                  }
               }

            OSMEMBUFPTR(&memBuf)[buffPos++] = c;
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

   if (stat >= 0 && buffPos > 0) {
      AppendBitStrToList (pctxt, plist, OSMEMBUFPTR(&memBuf), buffPos);

      if (stat < 0)
         LOG_RTERR (pctxt, stat);
      }

   if (stat == XML_E_UNEXPEOF) ; /* can't continue decoding */
   else if (rtXmlRdGetDataMode (pXmlReader) == OSXMLDM_CONTENT) {
      int statTm = stat;
      stat = rtXmlpMatchEndTag (pctxt, dataCtxt.mDataLevel);
      if (stat == 0)
         stat = statTm;
      else
         LOG_RTERR (pctxt, stat);
   }
   else if (stat > 0)
      stat = 0;

   rtxMemBufFree (&memBuf);

   if (stat < 0)
      rtxDListFreeAll (pctxt, plist);

   return stat;
}

