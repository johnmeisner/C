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
#include "rtxsrc/rtxBitString.h"

EXTXMLMETHOD int rtXmlpDecBitString
(OSCTXT* pctxt, OSOCTET* pvalue, OSUINT32* pnbits, OSUINT32 bufsize)
{
   OSSIZE nbits64;
   int stat;

   stat = rtXmlpDecBitString64(pctxt, pvalue, &nbits64, bufsize);
   if (stat == 0)
   {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnbits)
         {
            *pnbits = (OSUINT32)nbits64;
         }
      }
   }

   return 0;
}

EXTXMLMETHOD int rtXmlpDecBitString64
(OSCTXT *pctxt, OSOCTET* pvalue, OSSIZE* pnbits, OSSIZE bufsize)
{
   OSXMLDataCtxt dataCtxt;
   struct OSXMLReader* pXmlReader;
   OSSIZE bitIndex = 0;
   OSSIZE numbits = bufsize * 8;
   int stat = 0;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (pnbits)
      *pnbits = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0) {
      do {
         const OSUTF8CHAR* const inpdata = dataCtxt.mData.value;
         OSUINT32 nbytes = (OSUINT32) dataCtxt.mData.length;
         size_t i;

         if (bitIndex + (OSUINT32)nbytes > numbits) {
            rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, 1);
            stat = LOG_RTERRNEW (pctxt, RTERR_STROVFLW);
         }

         for (i = 0; i < nbytes && stat >= 0; i++) {
            OSUTF8CHAR c = inpdata[i];

            if (pvalue) {
               if (bitIndex % 8 == 0)
                  pvalue[bitIndex / 8] = 0;

               if (c == '1')
                  rtxSetBit (pvalue, numbits, bitIndex);
               else if (c != '0') {
                  rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
                  stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
               }
            }
            else if (c != '0' && c != '1') {
               rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, nbytes - i);
               stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
            }

            bitIndex++;
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

   if (stat >= 0) {
      stat = 0;
      if (pnbits)
         *pnbits = bitIndex;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpDecBitStringExt(OSCTXT* pctxt, OSOCTET* pvalue,
                                       OSUINT32* pnbits, OSOCTET** ppextdata,
                                       OSUINT32 bufsize)
{
   OSSIZE nbits64;
   int stat;

   stat = rtXmlpDecBitStringExt64(pctxt, pvalue, &nbits64, ppextdata, bufsize);
   if (stat == 0)
   {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (pnbits)
         {
            *pnbits = (OSUINT32)nbits64;
         }
      }
   }

   return 0;
}

EXTXMLMETHOD int rtXmlpDecBitStringExt64(OSCTXT* pctxt, OSOCTET* pvalue,
                                         OSSIZE* pnbits, OSOCTET** ppextdata,
                                         OSSIZE bufsize)
{
   OSSIZE bitIndex = 0, numbits = bufsize * 8, totalBytes = 0;
   OSUTF8CHAR c;
   OSXMLDataCtxt dataCtxt;
   size_t i;
   OSOCTET *pExtData = NULL;
   struct OSXMLReader* pXmlReader;
   char *pXmlString = NULL;
   int stat = 0;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (!pvalue)
   {
      LOG_RTERR(pctxt, RTERR_INVPARAM);
      return RTERR_INVPARAM;
   }

   if (pnbits)
   {
      *pnbits = 0;
   }

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   /* Run through the data once to get total number of bytes to allocate for
       extdata member and XML string... */
   rtXmlRdMarkPos(pXmlReader);
   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0)
   {
      do
      {
         totalBytes += (OSUINT32)dataCtxt.mData.length;

         stat = rtXmlRdNextData(pXmlReader, &dataCtxt);
         if (stat < 0)
         {
            LOG_RTERR(pctxt, stat);
         }
      } while (stat > 0);
   }

   pExtData = (OSOCTET *)rtxMemAlloc(pctxt, totalBytes - numbits);
   pXmlString = (char *)rtxMemAlloc(pctxt, totalBytes + 1);

   /* Now rewind to beginning and gather string for processing... */
   rtXmlRdRewindToMarkedPos(pXmlReader);
   if ((stat = rtXmlRdFirstData (pXmlReader, &dataCtxt)) > 0)
   {
      rtxStrncpy(pXmlString, totalBytes + 1,
                 (const char *)dataCtxt.mData.value, dataCtxt.mData.length);
      do
      {
         stat = rtXmlRdNextData(pXmlReader, &dataCtxt);
         if (stat > 0)
         {
            rtxStrncat(pXmlString, totalBytes + 1,
               (const char *)dataCtxt.mData.value, dataCtxt.mData.length);
         }
      } while (stat > 0);
   }
   pXmlString[totalBytes] = '\0';

   if (stat < 0)
   {
      LOG_RTERR(pctxt, stat);
      return stat;
   }

   for (i = 0; i < totalBytes; i++)
   {
      c = (OSUTF8CHAR)pXmlString[i];

      if (c != '0' && c != '1')
      {
         rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, totalBytes - i);
         stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
      }
      else
      {
         if (bitIndex % 8 == 0)
         {
            if (i >= numbits)
            {
               pExtData[bitIndex / 8] = 0;
            }
            else
            {
               pvalue[bitIndex / 8] = 0;
            }
         }

         if (c == '1')
         {
            if (i >= numbits)
            {
               rtxSetBit (pExtData, numbits, bitIndex);
            }
            else
            {
               rtxSetBit (pvalue, numbits, bitIndex);
            }
         }
         else if (c != '0')
         {
            rtXmlRdErrAddDataSrcPos (pXmlReader, &dataCtxt, totalBytes - i);
            stat = LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
         }
      }

      if (i == (numbits - 1))
      {
         bitIndex = 0;
      }
      else
      {
         bitIndex++;
      }
   }

   if (stat >= 0)
   {
      stat = 0;
      *ppextdata = pExtData;
      if (pnbits)
      {
         *pnbits = totalBytes;
      }
   }

   rtxMemFreePtr(pctxt, pXmlString);
   return stat;
}

