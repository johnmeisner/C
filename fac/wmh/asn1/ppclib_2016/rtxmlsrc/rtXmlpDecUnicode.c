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

EXTXMLMETHOD int rtXmlpDecDynUnicodeStr
(OSCTXT* pctxt, const OSUNICHAR** ppdata, OSSIZE* pnchars)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   OSSIZE datasize = 0;
   OSUNICHAR* data = 0;
   int stat = 0;
   OSUNICHAR curUniChar = 0;
   int curCharOctet = 0;
   int curCharLen = 0;
   OSSIZE dstPos = 0;
   OSSIZE srcPos = 0;
   OSRTMEMBUF memBuf;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (ppdata)
      *ppdata = 0;
   if (pnchars)
      *pnchars = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, pXmlReader->mStringWhiteSpaceMode);
   rtxMemBufInit (pctxt, &memBuf, XMLP_DECODE_SEGSIZE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;
         OSSIZE i;

         if (datasize - dstPos < nbytes) {
            stat = rtxMemBufPreAllocate (&memBuf,
               (nbytes + dstPos - datasize) * sizeof (OSUNICHAR));

            if (stat < 0) {
               LOG_RTERR (pctxt, stat);
               break;
            }

            data = (OSUNICHAR*) OSMEMBUFPTR(&memBuf);
            datasize = dstPos + nbytes;
         }

         for (i = 0; i < nbytes; i++, srcPos++) {
            OSUTF8CHAR c = inpdata[i];

            if (curCharOctet == 0) { /* first octet */
#ifndef _PULL_FROM_BUFFER
               if (pXmlReader->mStringWhiteSpaceMode != OSXMLWSM_PRESERVE) {
                  if (c == '\n' || c == '\r' || c == '\t')
                     c = ' ';
               }
#endif
               /* get char len */

               if (c < 0x80) {
                  curCharLen = 1;
                  curUniChar = (OSUNICHAR) (c & 0x7F);
               }
               else if (c < 0xE0) {
                  curCharLen = 2;
                  curUniChar = (OSUNICHAR) (c & 0x1F);
               }
               else if (c < 0xF0) {
                  curCharLen = 3;
                  curUniChar = (OSUNICHAR) (c & 0xF);
               }
               else if (c < 0xF8) {
                  curCharLen = 4;
                  curUniChar = (OSUNICHAR) (c & 0x7);
               }
               else if (c < 0xFC) {
                  curCharLen = 5;
                  curUniChar = (OSUNICHAR) (c & 0x3);
               }
               else if (c < 0xFE) {
                  curCharLen = 6;
                  curUniChar = (OSUNICHAR) (c & 0x1);
               }
               else { /* invalid utf8 character */
                  rtxErrNewNode (pctxt);
                  rtxErrAddUIntParm (pctxt, (OSUINT32)srcPos);
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
                  break;
               }

               if ((curCharLen > 3) && (sizeof (OSUNICHAR) < 4)) {
                  /* character not fit to USC2 */
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
                  break;
               }
            }
            else if ((c >= 0x80) && (c < 0xC0)) { /* next octet */
               curUniChar = (OSUNICHAR) ((curUniChar << 6) | (c & 0x3F));
            }
            else { /* invalid utf8 character */
               rtxErrNewNode (pctxt);
               rtxErrAddUIntParm (pctxt, (OSUINT32)srcPos);
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
               break;
            }

            curCharOctet++;

            if (curCharOctet == curCharLen) {
               data[dstPos++] = curUniChar;
               curCharOctet = 0;
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

   pXmlReader->mStringWhiteSpaceMode = OSXMLWSM_PRESERVE;

   if (stat >= 0 && curCharOctet != 0) {
      rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
      stat = LOG_RTERRNEW (pctxt, RTERR_ENDOFBUF);
   }

   if (stat >= 0) {
      stat = 0;

      if (datasize != dstPos && XMLP_DECODE_SEGSIZE == 1) /* shrink block */
         data = (OSUNICHAR*) rtxMemRealloc (pctxt, data,
            dstPos * sizeof (OSUNICHAR));

      if (pnchars)
         *pnchars = dstPos;

      if (ppdata)
         *ppdata = data;
      else
         rtxMemBufFree (&memBuf);
   }
   else
      rtxMemBufFree (&memBuf);

   return stat;
}
