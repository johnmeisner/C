/*
 * Copyright (c) 2017-2018 Objective Systems, Inc.
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

#include "osrtjson.h"

#include "rtxsrc/rtxBitString.h"

static int decBitStrChars (OSCTXT* pctxt, const char* data, OSSIZE nchars, 
                           OSOCTET* outbuf, OSSIZE outbufsize)
{
   int stat;
   OSSIZE i, obufSizeBits = outbufsize * 8;

   if (nchars > obufSizeBits) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);

   for (i = 0; i < nchars; i++) {
      char ch = data[i];
      if (ch != '0' && ch != '1') {
         return LOG_RTERR (pctxt, RTERR_BADVALUE);
      }
      else if (ch == '1') {
         stat = rtxSetBit (outbuf, obufSizeBits, i);
         if (stat < 0) return LOG_RTERR (pctxt, stat);
      }
   }

   return 0;
}

int rtJsonDecDynBitStr (OSCTXT* pctxt, OSUINT32 *nbits, OSOCTET** data)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecDynBitStr64(pctxt, &nbits64, data);
   if (stat == 0) {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX)) {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else if (0 != nbits) {
         *nbits = (OSUINT32)nbits64;
      }
   }

   return stat;
}

int rtJsonDecDynBitStr64 (OSCTXT* pctxt, OSSIZE *nbits, OSOCTET** data)
{
   OSUTF8CHAR* bstring;
   int  stat;
   OSSIZE nb;

   /* decode the bit string */
   stat = rtJsonDecStringValue (pctxt, &bstring);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   nb = OSCRTLSTRLEN ((const char *)bstring);
   if (0 != nbits) {
      *nbits = nb;
   }

   if (0 != data) {
      /* allocate space for the data */
      OSSIZE bufsize = nb / 8;
      if (nb % 8 != 0) bufsize++;

      *data = (OSOCTET *)rtxMemAllocZ (pctxt, bufsize);
      if (0 == *data) {
         rtxMemFreePtr (pctxt, bstring);
         return RTERR_NOMEM;
      }

      stat = decBitStrChars (pctxt, bstring, nb, *data, bufsize);
   }

   rtxMemFreePtr (pctxt, bstring);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int rtJsonDecBitStrValue
(OSCTXT *pctxt, OSUINT32 *nbits, OSOCTET *data, OSSIZE bufsize)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecBitStrValue64(pctxt, &nbits64, data, bufsize);
   if (stat == 0)
   {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else
      {
         if (nbits)
         {
            *nbits = (OSUINT32)nbits64;
         }
      }
   }

   return 0;
}

int rtJsonDecBitStrValue64
(OSCTXT *pctxt, OSSIZE *nbits, OSOCTET *data, OSSIZE bufsize)
{
   OSUTF8CHAR* bstring;
   int  stat;
   OSSIZE nb;

   /* decode the bit string */
   stat = rtJsonDecStringValue (pctxt, &bstring);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   nb = OSCRTLSTRLEN ((const char *)bstring);

   if (0 != nbits) {
      *nbits = nb;
   }

   if (0 != data) {
      OSCRTLMEMSET (data, 0, bufsize);
      stat = decBitStrChars (pctxt, (const char*)bstring, nb, data, bufsize);
   }

   rtxMemFreePtr (pctxt, bstring);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int rtJsonDecBitStrValueExt(OSCTXT* pctxt,
                            OSUINT32 *nbits, OSOCTET *data,
                            OSSIZE bufsize, OSOCTET **extdata)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecBitStrValueExt64(pctxt, &nbits64, data, bufsize, extdata);
   if (stat == 0)
   {
      if ((sizeof(nbits64) > 4) && (nbits64 > OSUINT32_MAX))
      {
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }
      else if (0 != nbits) {
         *nbits = (OSUINT32)nbits64;
      }
   }

   return 0;
}

int rtJsonDecBitStrValueExt64(OSCTXT* pctxt,
                              OSSIZE *nbits, OSOCTET *data,
                              OSSIZE bufsize, OSOCTET **extdata)
{
   OSUTF8CHAR* bstring;
   OSSIZE datasizeBits, nb;
   OSOCTET *pExtData = NULL;
   int stat;

   /* decode the bit string */
   stat = rtJsonDecStringValue (pctxt, &bstring);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   nb = OSCRTLSTRLEN ((const char*)bstring);

   if (0 != nbits) {
      *nbits = nb;
   }

   if (0 == data) {
      rtxMemFreePtr (pctxt, bstring);
      return 0;
   }

   datasizeBits = bufsize * 8;

   /* zero out the array */
   OSCRTLMEMSET (data, 0, bufsize);

   if (nb <= datasizeBits) {
      stat = decBitStrChars (pctxt, (const char*)bstring, nb, data, bufsize);
   }
   else {
      stat = decBitStrChars 
         (pctxt, (const char*)bstring, datasizeBits, data, bufsize);

      if (0 == stat && 0 != extdata) {
         /* Allocate and zero out the extdata array... */
         OSSIZE sz = ((nb - datasizeBits) / 8) + 1;
         pExtData = (OSOCTET *)rtxMemAlloc(pctxt, sz);
         if (0 == pExtData) return RTERR_NOMEM;
         OSCRTLMEMSET (pExtData, 0, sz);

         stat = decBitStrChars 
            (pctxt, (const char*)&bstring[datasizeBits], nb - datasizeBits, 
             pExtData, sz);

         *extdata = pExtData;
      }
   }

   rtxMemFreePtr (pctxt, bstring);

   return 0;
}
