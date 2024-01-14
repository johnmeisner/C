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

#include "rtxsrc/rtxBase64.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"

/* Uncomment to enable diagnostics or add -DB64DEBUG to gcc command line.
#define B64DEBUG
*/
#ifdef B64DEBUG
#define RTLDIAG3(pctxt,msg,a,b)     rtxDiagStream(pctxt,msg,a,b)
#define RTLDIAG4(pctxt,msg,a,b,c)   rtxDiagStream(pctxt,msg,a,b,c)
#define RTLHEXDUMP(pctxt,buffer,numocts) \
rtxDiagStreamHexDump(pctxt,buffer,numocts)
#else
#define RTLDIAG3(pctxt,msg,a,b)
#define RTLDIAG4(pctxt,msg,a,b,c)
#define RTLHEXDUMP(pctxt,buffer,numocts)
#endif

/**
 * Return base64/base64url character for given index.
 * @param url if true, return the base64url character; otherwise, return the
 *          base64 character.
 */
char rtxBase64IdxToChar (int idx, OSBOOL url)
{
   if (idx >= 0 && idx <= 25) return (char)('A' + (char)idx);
   else if (idx > 25 && idx <= 51) return (char)('a' + (char)idx - 26);
   else if (idx > 51 && idx <= 61) return (char)('0' + (char)idx - 52);
   else if (idx == 62) return url ? '-' : '+';
   else if (idx == 63) return url ? '_' : '/';
   else return -1;
}

int rtxBase64CharToIdx (char c, OSBOOL url)
{
   if (c >= 48 && c <= 57) return (c + 4);
   else if (c >= 65 && c <= 90) return (c - 65);
   else if (c >= 97 && c <= 122) return (c - 71);
   else if (c == '-' && url) return 62;
   else if (c == '+' && !url) return 62;
   else if (c == '_' && url) return 63;
   else if (c == '/' && !url) return 63;
   else return -1;
}

/**
 * Convert binary data to base64/base64url.
 * @param urlParam: If true, convert to base4url; otherwise to base64.
 */
static long base64EncodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize,
 OSOCTET** ppDstData, OSBOOL url)
{
   size_t i;
   OSRTMEMBUF membuf;

   OSOCTET* pDstData;
   size_t numFullGroups = srcDataSize/3;
   size_t numBytesInPartialGroup = srcDataSize - 3*numFullGroups;
   size_t resultLen = 4*((srcDataSize + 2)/3);

   rtxMemBufInit (pctxt, &membuf, 64);

   pDstData = *ppDstData = rtxMemAllocArray (pctxt, resultLen + 1, OSOCTET);
   if (pDstData == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   /* Translate all full groups from byte array elements to Base64 */
   for (i = 0; i < numFullGroups; i++) {
       int byte0 = *pSrcData++ & 0xff;
       int byte1 = *pSrcData++ & 0xff;
       int byte2 = *pSrcData++ & 0xff;
       *pDstData++ = rtxBase64IdxToChar (byte0 >> 2, url);
       *pDstData++ = rtxBase64IdxToChar
          (((byte0 << 4) & 0x3f) | (byte1 >> 4), url);
       *pDstData++ = rtxBase64IdxToChar
          (((byte1 << 2) & 0x3f) | (byte2 >> 6), url);
       *pDstData++ = rtxBase64IdxToChar (byte2 & 0x3f, url);
   }

   /* Translate partial group if present */
   if (numBytesInPartialGroup != 0) {
       int byte0 = *pSrcData++ & 0xff;
       *pDstData++ = rtxBase64IdxToChar (byte0 >> 2, url);
       if (numBytesInPartialGroup == 1) {
          *pDstData++ = rtxBase64IdxToChar ((byte0 << 4) & 0x3f, url);
          *pDstData++ = '=';
          *pDstData++ = '=';
       } else {
          /* assert numBytesInPartialGroup == 2; */
          int byte1 = *pSrcData++ & 0xff;
          *pDstData++ = rtxBase64IdxToChar
             (((byte0 << 4) & 0x3f) | (byte1 >> 4), url);
          *pDstData++ = rtxBase64IdxToChar ((byte1 << 2) & 0x3f, url);
          *pDstData++ = '=';
       }
   }
   *pDstData = '\0';

   return (long)resultLen;
}

EXTRTMETHOD long rtxBase64EncodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData)
{
   return base64EncodeData (pctxt, pSrcData, srcDataSize, ppDstData, FALSE);
}

EXTRTMETHOD long rtxBase64UrlEncodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData)
{
   return base64EncodeData (pctxt, pSrcData, srcDataSize, ppDstData, TRUE);
}

EXTRTMETHOD long rtxBase64EncodeURLParam
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData)
{
   OSOCTET* pBase64Str;
   int plusCount = 0;
   long ret = base64EncodeData (pctxt, pSrcData, srcDataSize, ppDstData, FALSE);
   if (ret <= 0) return ret;

   /* Scan returned string for '+' characters.  These need to be escaped
      by converting to %2B sequences. */
   pBase64Str = *ppDstData;
   if (pBase64Str != 0) {
      while (*pBase64Str != '\0') {
         if (*pBase64Str++ == '+') plusCount++;
      }
   }

   if (plusCount > 0) {
      size_t newSize = (plusCount * 2) + ret;
      OSOCTET* pSrcData2;
      OSOCTET* pDstData = rtxMemAllocArray (pctxt, newSize + 1, OSOCTET);
      if (pDstData == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

      pBase64Str = pSrcData2 = *ppDstData;
      *ppDstData = pDstData;

      while (*pBase64Str != '\0') {
         if (*pBase64Str == '+') {
            *pDstData++ = '%';
            *pDstData++ = '2';
            *pDstData++ = 'B';
         }
         else {
            *pDstData++ = *pBase64Str;
         }
         pBase64Str++;
      }

      *pDstData = '\0';
      rtxMemFreePtr (pctxt, pSrcData2);
      ret = (long) newSize;
   }

   return ret;
}

/* Decode */

/*
 * Convert base64 or base64url string to binary.
 * This will treat URL escape sequence for '+' (%2B) as a '+' character if
 * url is FALSE (base64url does not use the '+' character).
 * @param url If true, process input as base64url otherwise as base64.
 */
static long base64ToBin
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET* pvalue,
   OSBOOL url)
{
   size_t i, j = 0;
   int ch[4]; /* group buffer */
   OSOCTET* bufp = pvalue;

   for (i = 0; i < srcDataSize; ) {
      if (OS_ISSPACE (pSrcData[i])) {
         i++;
      }
      else if (pSrcData[i] != '=') {
         /* Check for URL escaped + character */
         if (pSrcData[i] == '%' && pSrcData[i+1] == '2' &&
             pSrcData[i+2] == 'B' && !url) {
            ch[j] = rtxBase64CharToIdx ('+', FALSE);
            i += 2;
         }
         else {
            ch[j] = rtxBase64CharToIdx (pSrcData[i], url);
         }

         if (ch[j] < 0) {
            RTLDIAG4 (pctxt, "bad char: offset = %d, ch = %c (%d)\n",
                      i, pSrcData[i], pSrcData[i]);

            return (0 != pctxt) ?
               LOG_RTERRNEW (pctxt, RTERR_INVBASE64) : RTERR_INVBASE64;
         }
         else if (++j == 4) {
            /* Group is complete */
            *pvalue++ = (OSOCTET) ((ch[0] << 2) | (ch[1] >> 4));
            *pvalue++ = (OSOCTET) ((ch[1] << 4) | (ch[2] >> 2));
            *pvalue++ = (OSOCTET) ((ch[2] << 6) |  ch[3]);
            j = 0;
         }
         i++;
      }
      else { /* partial group */
         if (j < 2 || j > 3) {
            RTLDIAG3 (pctxt, "invalid number of characters %d detected in "
                      "partial block at offset %d\n", j, i);

            return (0 != pctxt) ?
               LOG_RTERRNEW (pctxt, RTERR_INVBASE64) : RTERR_INVBASE64;
         }
         *pvalue++ = (OSOCTET) ((ch[0] << 2) | (ch[1] >> 4));
         if (j == 3) {
            *pvalue++ = (OSOCTET) ((ch[1] << 4) | (ch[2] >> 2));
         }
         while (pSrcData[i] == '=') i++;
         j = 0;
      }
   }

   return (long)(pvalue - bufp);
}

static long getBinDataLen (const char* pSrcData, size_t srcDataSize)
{
   size_t i, j = 0;
   long nbytes = 0;
   for (i = 0; i < srcDataSize; ) {
      if (OS_ISSPACE (pSrcData[i])) {
         i++;
      }
      else if (pSrcData[i] != '=') {
         if (++j == 4) {
            nbytes += 3;
            j = 0;
         }
         /* check for escaped + character */
         if (pSrcData[i] == '%' && pSrcData[i+1] == '2' && pSrcData[i+2] == 'B')
            i += 3;
         else
            i++;
      }
      else { /* partial group */
         if (j == 2) nbytes += 1;
         else if (j == 3) nbytes += 2;
         else return RTERR_INVBASE64;
         while (pSrcData[i] == '=') i++;
         j = 0;
      }
   }
   return nbytes;
}

/**
 * Decode base64/base64url data to binary.
 * @param url If true, decode base64url; otherwise, decode base64.
 */
static long base64DecodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData,
   OSBOOL url)
{
   size_t bufsiz;
   OSOCTET* pvalue;
   long stat = getBinDataLen (pSrcData, srcDataSize);

   if (stat < 0) return LOG_RTERRNEW (pctxt, (int)stat);
   else bufsiz = (size_t) stat;

   pvalue = *ppDstData = rtxMemAllocArray (pctxt, bufsiz, OSOCTET);
   if (pvalue == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   stat = base64ToBin (pctxt, pSrcData, srcDataSize, pvalue, url);

   if (stat > 0) {
      RTLHEXDUMP (pctxt, pvalue, (size_t)stat);
      if (bufsiz != (size_t)stat) {
         RTLDIAG3 (pctxt, "Size mismatch: bufsiz = %lu, stat = %ld\n",
                   bufsiz, stat);
      }
   }

   return stat;
}

EXTRTMETHOD long rtxBase64DecodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData)
{
   return base64DecodeData(pctxt, pSrcData, srcDataSize, ppDstData, FALSE);
}

EXTRTMETHOD long rtxBase64UrlDecodeData
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize, OSOCTET** ppDstData)
{
   return base64DecodeData(pctxt, pSrcData, srcDataSize, ppDstData, TRUE);
}

/**
 * Decode base64/base64url data to binary.
 * @param url If true, decode base64url; otherwise, decode base64.
 */
static long base64DecodeDataToFSB
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize,
 OSOCTET* buf, size_t bufsiz, OSBOOL url)
{
   size_t nbytes;
   long stat = getBinDataLen (pSrcData, srcDataSize);

   if (stat < 0)
      return (0 != pctxt) ? LOG_RTERRNEW (pctxt, (int)stat) : (int)stat;
   else
      nbytes = (size_t) stat;

   if (nbytes > bufsiz)
      return (0 != pctxt) ?
         LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW) : RTERR_BUFOVFLW;

   return base64ToBin (pctxt, pSrcData, srcDataSize, buf, url);
}

EXTRTMETHOD long rtxBase64DecodeDataToFSB
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize,
 OSOCTET* buf, size_t bufsiz)
{
   return base64DecodeDataToFSB(pctxt, pSrcData, srcDataSize, buf, bufsiz,
                                 FALSE);
}

EXTRTMETHOD long rtxBase64UrlDecodeDataToFSB
(OSCTXT* pctxt, const char* pSrcData, size_t srcDataSize,
 OSOCTET* buf, size_t bufsiz)
{
   return base64DecodeDataToFSB(pctxt, pSrcData, srcDataSize, buf, bufsiz,
                                 TRUE);
}

EXTRTMETHOD long rtxBase64GetBinDataLen
(const char* pSrcData, size_t srcDataSize)
{
   return getBinDataLen (pSrcData, srcDataSize);
}
