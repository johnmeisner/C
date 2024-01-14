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
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxMemBuf.h"

#define BUFF_SZ 32

static OSUINT32 FindBitMapItem (const OSBitMapItem* pBitMap,
   const OSOCTET* str, size_t strSz)
{
   OSUINT32 i = 0;
   for ( ; pBitMap[i].name != 0; i++) {
      if (pBitMap[i].namelen == strSz &&
            memcmp (pBitMap[i].name, str, strSz) == 0)
         return pBitMap[i].bitno;
   }

   return ~0u;
}

EXTXMLMETHOD int rtXmlpDecNamedBits (OSCTXT* pctxt, const OSBitMapItem* pBitMap,
                        OSOCTET* pvalue, OSUINT32* pnbits, OSUINT32 bufsize)
{
   OSSIZE nbits;
   int ret = rtXmlpDecNamedBits64(pctxt, pBitMap, pvalue, &nbits, bufsize);

   if ( pnbits )
   {
      if ( nbits > OSUINT32_MAX ) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

      *pnbits = (OSUINT32) nbits;
   }

   return ret;
}


EXTXMLMETHOD int rtXmlpDecNamedBits64 (OSCTXT* pctxt,
                                       const OSBitMapItem* pBitMap,
                                       OSOCTET* pvalue, OSSIZE* pnbits,
                                       OSSIZE bufsize)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   OSSIZE numbits = bufsize * 8;
   int stat = 0;
   OSUINT32 maxbit = 0;

   OSOCTET buffer[BUFF_SZ];
   OSRTMEMBUF memBuf;
   size_t buffPos = 0;
   size_t buffSz = BUFF_SZ;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (!pBitMap)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);
   rtxMemBufInitBuffer (pctxt, &memBuf, buffer, BUFF_SZ, 1);

   if (pnbits)
      *pnbits = 0;

   if (pvalue && bufsize)
      OSCRTLMEMSET (pvalue, 0, bufsize);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         size_t nbytes = dataCtxt.mData.length;
         size_t i;

         for (i = 0; i < nbytes; i++) {
            OSOCTET c = inpdata[i];

#ifdef _PULL_FROM_BUFFER
            if (c == ' ') {
#else
            if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
#endif
               OSUINT32 bitno = FindBitMapItem (pBitMap,
                  OSMEMBUFPTR(&memBuf), buffPos);

               if (bitno == ~0u)
                  RTDIAG3 (pctxt,
                     "rtXmlpDecNamedBits: BitMapItem = '%*s' not found\n",
                     OSMEMBUFPTR(&memBuf), buffPos);
               else {
                  if (pvalue)
                     rtxSetBit (pvalue, numbits, bitno);
                  if (bitno > maxbit)
                     maxbit = bitno;
               }

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
      OSUINT32 bitno = FindBitMapItem (pBitMap, OSMEMBUFPTR(&memBuf), buffPos);

      if (bitno == ~0u)
         RTDIAG3 (pctxt, "rtXmlpDecNamedBits: BitMapItem = '%*s' not found\n",
            OSMEMBUFPTR(&memBuf), buffPos);
      else {
         if (pvalue)
            rtxSetBit (pvalue, numbits, bitno);
         if (bitno > maxbit)
            maxbit = bitno;
      }
   }


   if (stat >= 0) {
      stat = 0;
      if (pnbits)
         *pnbits = maxbit + 1;
   }

   rtxMemBufFree (&memBuf);

   return stat;
}


