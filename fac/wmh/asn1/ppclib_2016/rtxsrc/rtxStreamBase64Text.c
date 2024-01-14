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

#ifndef _NO_STREAM

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxStreamBase64Text.h"

static const signed char decodeTableB64[] = {
   /*
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1, */
   -1, -1, -1, 62, -1, -1, -1, 63,
   52, 53, 54, 55, 56, 57, 58, 59,
   60, 61, -1, -1, -1, -1, -1, -1,
   -1,  0,  1,  2,  3,  4,  5,  6,
   7,   8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22,
   23, 24, 25, -1, -1, -1, -1, -1,
   -1, 26, 27, 28, 29, 30, 31, 32,
   33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48,
   49, 50, 51, -1, -1, -1, -1, -1
};

#define BASE64TOINT(c) ((((OSOCTET)c) < 128 && ((OSOCTET)c) >= 40) ? decodeTableB64 [(c) - 40] : -1)

static long base64TextReadData
(OSRTSTREAM* pStream, OSOCTET* pdata, size_t size)
{
   Base64TextStrmDesc* pdesc = (Base64TextStrmDesc*) pStream->extra;
   OSRTSTREAM* pUnderStream = pdesc->pUnderStream;
   long buflen = 0, stat;
   size_t j = 0;
   int ch[4]; /* group buffer */
   char c;

   char lCertHeader[28];
   int header = 0;
   int footer = 0;
   char lCertFooter[26];

   char *headerTmp = "-----BEGIN CERTIFICATE-----";
   char *footerTmp = "-----END CERTIFICATE-----";

   OSBOOL headerOK = FALSE;
   OSBOOL footerOK = FALSE;

   pdesc->mbCertificate = FALSE;

   if (!(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (size == 0)
      return 0;

   /* Whitespace between chars is OK */
   for (;;) {
      /* Read a character from the stream */
      stat = pUnderStream->read (pUnderStream, (OSOCTET*)&c, 1);
      if (stat == 1) {

         /* Certificate recognition */
         if (c == '-') {
            /*probably header*/
            if (buflen == 0 && header < 27)
            {
                lCertHeader[header++] = c;
                while(header < 27)
                {
                   stat = pUnderStream->read (pUnderStream, (OSOCTET*)&c, 1);
                   if (stat ==1)
                      lCertHeader[header++] = c;
                   else if (stat == 0)
                      return RTERR_INVBASE64;
                   else  /* Error */
                      return stat;
                }
                lCertHeader[header] = '\0';
                if (!strcmp(lCertHeader, headerTmp))
                   headerOK = TRUE;
                else
                   return RTERR_INVBASE64;
                pStream->bytesProcessed +=header;
            }
            else if (buflen != 0 && footer < 25)
            {
               lCertFooter[footer++] = c;
               while(footer < 25)
               {
                  stat = pUnderStream->read (pUnderStream, (OSOCTET*)&c, 1);
                  if (stat ==1)
                  {
                     lCertFooter[footer++] = c;
                  }
                  else if (stat == 0)
                     return RTERR_INVBASE64;
                  else /* Error */
                     return stat;
                }
                lCertFooter[footer] = '\0';
                if (!strcmp(lCertFooter, footerTmp))
                   footerOK = TRUE;
                else
                   return RTERR_INVBASE64;
                pStream->bytesProcessed +=footer;
            }
            else
               return RTERR_INVBASE64;
            continue;
         }

         /* Character successfully read */
         if (!OS_ISSPACE (c)) {
            if ((size_t)buflen >= size) {
               return RTERR_STROVFLW;
            }

            if (c != '=') {
               ch[j] = BASE64TOINT (c);
               if (ch[j] < 0) {
                  return RTERR_INVBASE64;
               }
               else if (++j == 4) {
                  /* Group is complete */
                  pdata[buflen++] = (OSOCTET) ((ch[0] << 2) | (ch[1] >> 4));
                  pdata[buflen++] = (OSOCTET) ((ch[1] << 4) | (ch[2] >> 2));
                  pdata[buflen++] = (OSOCTET) ((ch[2] << 6) |  ch[3]);
                  j = 0;
               }
            }
            else { /* partial group */
               if (j < 2 || j > 3) {
                  return RTERR_INVBASE64;
               }
               pdata[buflen++] = (OSOCTET) ((ch[0] << 2) | (ch[1] >> 4));
               if (j == 3) {
                  pdata[buflen++] = (OSOCTET) ((ch[1] << 4) | (ch[2] >> 2));
               }

            }
         }
      }
      else if (stat == 0) {
         break;
      }
      else { /* Error */
         return stat;
      }

      pStream->bytesProcessed++;
   }

   if (headerOK && footerOK)
      pdesc->mbCertificate = TRUE;

   return buflen;
}


static char getBase64Char (int idx)
{
   if (idx >= 0 && idx <= 25) return (char)('A' + (char)idx);
   else if (idx > 25 && idx <= 51) return (char)('a' + (char)idx - 26);
   else if (idx > 51 && idx <= 61) return (char)('0' + (char)idx - 52);
   else if (idx == 62) return '+';
   else if (idx == 63) return '/';
   else return -1;
}

static long base64TextWrite
(OSRTSTREAM* pStream, const OSOCTET* pdata, size_t numocts)
{
   Base64TextStrmDesc* pdesc = (Base64TextStrmDesc*) pStream->extra;
   OSCTXT* pctxt = pdesc->pctxt;
   OSRTSTREAM* pUnderStream = pdesc->pUnderStream;
   size_t i;
   char pDstData[4];
   size_t numFullGroups = numocts/3;
   size_t numBytesInPartialGroup = numocts - 3*numFullGroups;

   long stat = 0;

   /* Translate all full groups from byte array elements to Base64 */
   for (i = 0; i < numFullGroups; i++) {
       int byte0 = *pdata++ & 0xff;
       int byte1 = *pdata++ & 0xff;
       int byte2 = *pdata++ & 0xff;
       pDstData[0] = getBase64Char (byte0 >> 2);
       pDstData[1] = getBase64Char (((byte0 << 4) & 0x3f) | (byte1 >> 4));
       pDstData[2] = getBase64Char (((byte1 << 2) & 0x3f) | (byte2 >> 6));
       pDstData[3] = getBase64Char (byte2 & 0x3f);
      stat = pUnderStream->write (pUnderStream, (const OSOCTET*)pDstData, 4);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }

   /* Translate partial group if present */
   if (numBytesInPartialGroup != 0) {
       int byte0 = *pdata++ & 0xff;
       pDstData[0] = getBase64Char (byte0 >> 2);
       if (numBytesInPartialGroup == 1) {
          pDstData[1] = getBase64Char ((byte0 << 4) & 0x3f);
          pDstData[2] = '=';
          pDstData[3] = '=';
       } else {
          /* assert numBytesInPartialGroup == 2; */
          int byte1 = *pdata++ & 0xff;
          pDstData[1] = getBase64Char (((byte0 << 4) & 0x3f) | (byte1 >> 4));
          pDstData[2] = getBase64Char ((byte1 << 2) & 0x3f);
          pDstData[3] = '=';
       }
      stat = pUnderStream->write (pUnderStream, (const OSOCTET*)pDstData, 4);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }
   return 0;
}

static long base64TextRead
(OSRTSTREAM* pStream, OSOCTET* pdata, size_t size)
{
   return base64TextReadData (pStream, pdata, size);
}

static long base64TextBlockingRead
(OSRTSTREAM* pStream, OSOCTET* pdata, size_t size)
{
   return base64TextReadData (pStream, pdata, size);
}

static int base64TextClose (OSRTSTREAM* pStream)
{
   Base64TextStrmDesc* pdesc = (Base64TextStrmDesc*) pStream->extra;
   OSCTXT* pctxt = pdesc->pctxt;
   OSRTSTREAM* pUnderStream = pdesc->pUnderStream;
   int stat = 0;

   /* Close underlying stream */
   if (0 != pUnderStream) {
      stat = pUnderStream->close (pUnderStream);
      rtxMemSysFreePtr (pctxt, pUnderStream);
   }
   rtxMemSysFreePtr (pctxt, pStream->extra);

   return (0 == stat) ? 0 : LOG_RTERR (pctxt, stat);
}

static int base64TextFlush (OSRTSTREAM* pStream)
{
   Base64TextStrmDesc* pdesc = (Base64TextStrmDesc*) pStream->extra;
   OSCTXT* pctxt = pdesc->pctxt;
   OSRTSTREAM* pUnderStream = pdesc->pUnderStream;
   int stat = 0;

   /* Verify this is an output stream */
   if (!(pStream->flags & OSRTSTRMF_OUTPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   /* Flush underlying stream */
   if (0 != pUnderStream && pUnderStream->flush != 0) {
      stat = pUnderStream->flush (pUnderStream);
   }

   return stat;
}

static int base64TextSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   Base64TextStrmDesc* pdesc = (Base64TextStrmDesc*) pStream->extra;
   OSRTSTREAM* pUnderStream = pdesc->pUnderStream;

   return pUnderStream->skip (pStream, skipBytes);
}

int rtxStreamBase64TextAttach (OSCTXT* pctxt, OSUINT16 flags)
{
   OSRTSTREAM *pStream;
   OSRTSTREAM *pUnderStream = pctxt->pStream;
   Base64TextStrmDesc* pdesc;
   int stat;

   if (0 == pUnderStream) {
      return LOG_RTERR (pctxt, RTERR_NOTINIT);
   }
   pctxt->pStream = 0;

   /* re-init stream */
   stat = rtxStreamInit (pctxt);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   pStream = pctxt->pStream;
   pStream->flags = (OSUINT16) (pUnderStream->flags | OSRTSTRMF_UNBUFFERED);

   if (pStream->flags & OSRTSTRMF_OUTPUT) {
      pStream->write = base64TextWrite;
      pStream->flush = base64TextFlush;
   }
   else if (pStream->flags & OSRTSTRMF_INPUT) {
      pStream->read  = base64TextRead;
      pStream->blockingRead  = base64TextBlockingRead;
      pStream->skip  = base64TextSkip;
      pStream->mark = 0;
      pStream->reset = 0;
   }
   pStream->close = base64TextClose;

   pdesc = rtxMemSysAllocType (pctxt, Base64TextStrmDesc);
   if (pdesc == 0) {
      rtxMemSysFreePtr (pctxt, pStream);
      pctxt->pStream = pUnderStream;
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }

   pdesc->pctxt = pctxt;
   pdesc->pUnderStream = pUnderStream;

   pStream->extra = pdesc;

   return 0;
}

#endif
