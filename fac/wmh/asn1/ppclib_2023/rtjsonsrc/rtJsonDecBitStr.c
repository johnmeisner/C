/*
 * Copyright (c) 2017-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxUTF8.h"

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

int rtJsonDecDynBitStrV72 (OSCTXT* pctxt, OSUINT32 *nbits, OSOCTET** data)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecDynBitStr64V72(pctxt, &nbits64, data);
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

int rtJsonDecDynBitStr64V72 (OSCTXT* pctxt, OSSIZE *nbits, OSOCTET** data)
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

      stat = decBitStrChars (pctxt, (const char*)bstring, nb, *data, bufsize);
   }

   rtxMemFreePtr (pctxt, bstring);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int rtJsonDecBitStrValueV72
(OSCTXT *pctxt, OSUINT32 *nbits, OSOCTET *data, OSSIZE bufsize)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecBitStrValue64V72(pctxt, &nbits64, data, bufsize);
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

int rtJsonDecBitStrValue64V72
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

int rtJsonDecBitStrValueExtV72(OSCTXT* pctxt,
                            OSUINT32 *nbits, OSOCTET *data,
                            OSSIZE bufsize, OSOCTET **extdata)
{
   OSSIZE nbits64;
   int stat;

   stat = rtJsonDecBitStrValueExt64V72(pctxt, &nbits64, data, bufsize, extdata);
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

int rtJsonDecBitStrValueExt64V72(OSCTXT* pctxt,
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

/**
 * Decode a JSON string for a BIT STRING encoding.
 *
 * @param parray preallocated array to decode value into, or null if all data
 *                should be decoded into pext.
 * @param arrayaize  Size of parray array.
 * @param pext Pointer to object to decode data that doesn't fit in parray to.
 *             This may be NULL if all data must fit into (not-null) parray.
 * @param plen *plen is set to the actual number of decoded octets.
 */
static int decString(OSCTXT* pctxt, OSOCTET* parray, OSSIZE arraysize,
                           OSDynOctStr64* pext, OSSIZE* plen)
{
   OSSIZE numocts = 0;   /* the actual number of octets decoded into parray */
   int stat;

   *plen = 0;

   stat = rtJsonDecMatchChar(pctxt, '"');
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if ( parray ) {
      stat = rtJsonDecHexData64(pctxt, parray, &numocts, arraysize);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if ( !pext ) {
         /* overflow if all data did not fit into parray */
         if ( numocts == arraysize )
         {
            char c = rtJsonDecPeekChar2(pctxt);
            if ( c != '"' ) {
               return LOG_RTERRNEW(pctxt, RTERR_STROVFLW);
            }
         }
      }
   }

   /* Decode remaining hex chars.  In the case where pext is null,
      we've already ensured all data fit into parray, assuming parray
      was not null as required.
   */
   if ( pext ) {
      stat = rtJsonDecDynHexData64(pctxt, pext);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = rtJsonDecMatchChar(pctxt, '"');
   if (stat != 0) LOG_RTERR (pctxt, stat);

   if ( stat == 0 ) {
      *plen = numocts;
      if ( pext ) *plen += pext->numocts;
   }

   if ( stat != 0 ) {
      /* Error.  Don't leak memory for the value; free it. */
      if ( pext && pext->data != 0 ) {
         rtxMemFreePtr(pctxt, pext->data);
         pext->data = 0;
         pext->numocts = 0;
      }
   }
   return stat;
}


/**
 * Decode a JSON object with "value" and "length" keys for a BIT STRING
 * encoding.
 *
 * This verifies the encoded value and length are consistent with each other
 * and that unused bits were encoded as zero.
 *
 * @param parray preallocated array to decode value into, or null if all data
 *                should be decoded into pext.
 * @param arrayaize  Size of parray array.
 * @param pext Pointer to object to decode data that doesn't fit in parray to.
 *             This may be NULL if all data must fit into (not-null) parray.
 * @param plen *plen is set to the decoded length from the "length" key.
 */
static int decValueAndLength(OSCTXT* pctxt, OSOCTET* parray, OSSIZE arraysize,
                           OSDynOctStr64* pext, OSSIZE* plen)
{
   OSUTF8CHAR* pkey;
   OSUTF8CHAR c;
   OSBOOL bFoundValue = FALSE;
   OSBOOL bFoundLength = FALSE;
   OSSIZE numocts = 0;   /* the actual number of octets decoded from value */
   int stat;

   stat = rtJsonDecMatchChar(pctxt, '{');
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   for(;;) {
      /* decode next key */
      stat = rtJsonDecStringValue(pctxt, &pkey);
      if (stat != 0) { LOG_RTERR (pctxt, stat); break; }

      if ( rtxUTF8StrEqual(OSUTF8("value"), pkey) && !bFoundValue )
      {
         rtxMemFreePtr(pctxt, pkey);
         bFoundValue = TRUE;

         stat = rtJsonDecMatchChar(pctxt, ':');
         if (stat != 0) { LOG_RTERR (pctxt, stat); break; }

         stat = decString(pctxt, parray, arraysize, pext, &numocts);
         if (stat != 0) { LOG_RTERR (pctxt, stat); break; }
      }
      else if ( rtxUTF8StrEqual(OSUTF8("length"), pkey) && !bFoundLength )
      {
         OSUINT64 len;

         rtxMemFreePtr(pctxt, pkey);
         bFoundLength = TRUE;

         stat = rtJsonDecMatchChar(pctxt, ':');
         if (stat != 0) { LOG_RTERR (pctxt, stat); break; }

         stat = rtJsonDecUInt64Value(pctxt, &len);
         if (stat != 0) { LOG_RTERR (pctxt, stat); break; }

         if ( len > OSSIZE_MAX ) {
            stat = LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
            break;
         }

         *plen = (OSSIZE) len;
      }
      else {
         /* unexpected key */
         rtxErrAddStrParm(pctxt, (const char*)pkey);
         rtxMemFreePtr(pctxt, (void*)pkey);

         stat = LOG_RTERRNEW(pctxt, RTERR_UNEXPELEM);
         break;
      }

      stat = rtJsonDecPeekChar(pctxt, &c);
      if (stat != 0) { LOG_RTERR (pctxt, stat); break; }
      if (c == '}') break;    /* end of object */

      stat = rtJsonDecMatchChar(pctxt, ',');
      if (stat != 0) { LOG_RTERR (pctxt, stat); break; }
   }

   /* end of object reached; match the closing brace */
   if ( stat == 0 ) {
      stat = rtJsonDecMatchChar(pctxt, '}');
      if (stat != 0) LOG_RTERR (pctxt, stat);
   }

   /* It is an error if we did not find both "value" and "length" */
   if ( stat == 0 && !bFoundValue )
   {
      rtxErrAddStrParm(pctxt, "value");
      stat = LOG_RTERRNEW(pctxt, RTERR_SETMISRQ);
   }

   if ( stat == 0 && !bFoundLength )
   {
      rtxErrAddStrParm(pctxt, "length");
      stat = LOG_RTERRNEW(pctxt, RTERR_SETMISRQ);
   }

   if ( stat == 0 )
   {
      /* verify value * length consistency and trailing bits are zero. */
      OSSIZE expOctets;       /* octets expected based on length */
      OSSIZE finalBits;       /* final bits used in final octet, if partial */
      OSUTF8CHAR* pFinal;     /* pointer to the final octet */

      finalBits = *plen % 8;
      expOctets = *plen / 8;
      if ( finalBits > 0 ) ++expOctets;

      if ( parray && numocts <= arraysize )
      {
         /* last octet is in parray */
         pFinal = parray + (numocts - 1);
      }
      else {
         /* last octet is in pext, if length & value are consistent */
         pFinal = pext->data + (pext->numocts - 1);
      }

      if ( numocts == expOctets )
      {
         if ( finalBits > 0 )
         {
            //lowest 8 - finalBits bits of final octet should be zero
            if ( (*pFinal & (0xFF >> finalBits)) != 0 )
            {
               stat = LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
            }
         }
      }
      else
      {
         stat = LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }
   }

   if ( stat != 0 ) {
      /* Error.  Don't leak memory for the value; free it. */
      if ( pext && pext->data != 0 ) {
         rtxMemFreePtr(pctxt, pext->data);
         pext->data = 0;
         pext->numocts = 0;
      }
      *plen = 0;
   }
   return stat;
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
   int  stat;
   OSSIZE len;
   OSDynOctStr64 dynData;

   if ( nbits ) *nbits = 0;
   if ( data ) *data = 0;

   stat = decValueAndLength(pctxt, 0, 0, &dynData, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if ( stat == 0 )
   {
      if ( nbits ) *nbits = len;
      if ( data ) *data = dynData.data;
   }

   return stat;
}

int rtJsonDecFixedDynBitStr (OSCTXT* pctxt, OSSIZE nbits, OSOCTET** data)
{
   int  stat;
   OSSIZE decbits;      /* # of bits decoded */
   OSSIZE finalBits;    /* # of bits in last octet if partial; else 0 */
   OSDynOctStr64 dynData;

   stat = rtJsonDecDynHexStr64(pctxt, &dynData);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* check that decoded # of octets is correct and unused bits are zero */
   decbits = dynData.numocts * 8;
   if ( decbits < nbits || decbits - nbits >= 8 )
   {
      rtxErrAddSizeParm(pctxt, decbits);
      stat = LOG_RTERR(pctxt, RTERR_CONSVIO);
   }

   finalBits = nbits % 8;
   if ( stat == 0 && finalBits > 0 )
   {
      if ( (dynData.data[dynData.numocts - 1] & (0xFF >> finalBits)) != 0 )
      {
         stat = LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   if ( data ) {
      if (stat == 0) *data = dynData.data;
      else {
         rtxMemFreePtr(pctxt, dynData.data);
         *data = 0;
      }
   }

   return stat;
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
   int  stat;
   OSSIZE len;

   if ( nbits ) *nbits = 0;

   /* JSON object. */
   stat = decValueAndLength(pctxt, data, bufsize, 0, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if ( nbits ) *nbits = len;

   return stat;
}


int rtJsonDecFixedBitStrValue
(OSCTXT *pctxt, OSSIZE nbits, OSOCTET *data, OSSIZE bufsize)
{
   int  stat;
   OSSIZE numocts;
   OSSIZE decbits;      /* # of bits decoded */
   OSSIZE finalBits;    /* # of bits in last octet if partial; else 0 */

   if ( data == 0 )
   {
      /* Use the dynamic version.  It will still ensure the unused bits are
         zeros.
      */
      return rtJsonDecFixedDynBitStr(pctxt, nbits, 0);
   }

   stat = rtJsonDecHexStr64(pctxt, data, &numocts, bufsize);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* check that decoded # of octets is correct */
   decbits = numocts * 8;
   if ( decbits < nbits || decbits - nbits >= 8 )
   {
      rtxErrAddSizeParm(pctxt, decbits);
      stat = LOG_RTERR(pctxt, RTERR_CONSVIO);
   }

   /* check that unused bits are zero */
   finalBits = nbits % 8;
   if ( stat == 0 && finalBits > 0 )
   {
      if ( (data[numocts - 1] & (0xFF >> finalBits)) != 0 )
      {
         stat = LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   return stat;
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
   int  stat;
   OSSIZE len;
   OSDynOctStr64 dynData;

   if ( nbits ) *nbits = 0;
   if ( extdata ) *extdata = 0;

   /* JSON object. */
   stat = decValueAndLength(pctxt, data, bufsize, &dynData, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if ( nbits ) *nbits = len;
   if ( extdata ) *extdata = dynData.data;

   return stat;
}
