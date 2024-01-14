/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2018 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include <string.h>
#include "ASN1XERString.h"
#include "rtsrc/asn1type.h"

const XMLCHAR* ASN1XERString::zeroLenString () {
   static const XMLCHAR *sZeroLenString = { 0 };
   return sZeroLenString;
}

unsigned int ASN1XERString::stringLen (const XMLCHAR* const str)
{
   if (str == 0) return 0;
   unsigned int len = 0;
   while (str [len] != 0)
      len++;
   return len;
}

unsigned int ASN1XERString::stringLen (const XMLCHAR16* const str)
{
   if (str == 0) return 0;
   unsigned int len = 0;
   while (str [len] != 0)
      len++;
   return len;
}

XMLCHAR* ASN1XERString::replicate (const XMLCHAR* const str)
{
   if (str == 0) return 0;
   int len = ASN1XERString::stringLen (str);
   XMLCHAR* nstr = new XMLCHAR [len + 1];
   memcpy (nstr, str, sizeof (XMLCHAR) * (len + 1));
   return nstr;
}

XMLCHAR* ASN1XERString::transcode (const char* const str)
{
   if (str == 0) return 0;
   size_t len = strlen (str), i = 0;
   XMLCHAR* nstr = new XMLCHAR [len + 1];
   for (; i <= len; i++)
      nstr[i] = str[i];
   return nstr;
}

char* ASN1XERString::transcode (const XMLCHAR16* const str)
{
   if (str == 0) return 0;
   int len = ASN1XERString::stringLen (str);
   char* nstr = new char [len + 1];
   for (int i = 0; i <= len; i++)
      nstr[i] = (char)str[i];
   return nstr;
}

int ASN1XERString::compareString(const XMLCHAR* const str1,
                               const XMLCHAR* const str2)
{
   const XMLCHAR* psz1 = str1;
   const XMLCHAR* psz2 = str2;

   if (psz1 == 0 || psz2 == 0) {
      if (psz1 == 0) {
         return 0 - ASN1XERString::stringLen(psz2);
      }
  	else if (psz2 == 0) {
         return ASN1XERString::stringLen(psz1);
      }
   }

   while (XMLTRUE)
   {
      // If an inequality, then return the difference
      if (*psz1 != *psz2)
          return int(*psz1) - int(*psz2);

      // If either has ended, then they both ended, so equal
      if (!*psz1 || !*psz2)
          break;

      // Move upwards for the next round
      psz1++;
      psz2++;
   }
   return 0;
}

XMLCHAR* ASN1XERString::catString (XMLCHAR* destStr, int destStrSize,
                                 const XMLCHAR* const str)
{
   int len = stringLen (str);
   int destLen = stringLen (destStr);
   if (destLen + len >= destStrSize)
      len = destStrSize - destLen - 1;

   if (len > 0) {
      memcpy (destStr + destLen, str, len * sizeof (XMLCHAR));
      destStr[destLen + len] = 0;
      return destStr;
   }
   return 0;
}

#ifdef XML_UNICODE
XMLCHAR* ASN1XERString::catString (XMLCHAR* destStr, int destStrSize,
                                 const char* const str)
#else
XMLCHAR* ASN1XERString::catString (XMLCHAR* destStr, int destStrSize,
                                 const XMLCHAR16* const str)
#endif
{
   int len = stringLen (str);
   int destLen = stringLen (destStr);
   if (destLen + len >= destStrSize)
      len = destStrSize - destLen - 1;

   if (len > 0) {
      for (int i = 0; i < len; i++) {
         destStr[destLen + i] = (XMLCHAR)str[i];
      }
      destStr[destLen + len] = 0;
      return destStr;
   }
   return 0;
}

/* UTF-8 validation and conversion functions */

static unsigned int encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};

static unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};


int ASN1XERString::stringUTF8Len (const unsigned char* utf8Str, int strLen)
{
   unsigned inlen = (strLen == 0) ?
      (unsigned)strlen ((const char*)utf8Str) : strLen;
   unsigned inbufx = 0;
   size_t startx;
   int nbytes, nchars = 0;
   unsigned char byte;

   while (inbufx < inlen) {
      startx = inbufx;

      /* Process the first byte of the UTF-8 sequence */

      byte = utf8Str[inbufx++];
      if (byte < 0x80) {
         nbytes = 0;
      }
      else if ((byte & 0xe0) == 0xc0) {
         nbytes = 1;
      }
      else if ((byte & 0xf0) == 0xe0) {
         nbytes = 2;
      }
      else if ((byte & 0xf8) == 0xf0) {
         nbytes = 3;
      }
      else if ((byte & 0xfc) == 0xf8) {
         nbytes = 4;
      }
      else if ((byte & 0xfe) == 0xfc) {
         nbytes = 5;
      }
      else {
         return -1;  /* invalid encoding error */
      }

      inbufx += nbytes;
      nchars ++;
   }

   return (nchars);
}

XMLCHAR16* ASN1XERString::transUTF8 (const unsigned char* utf8Str,
   int *strSize, XMLCHAR16* destBuf, int destBufSize)
{
   unsigned inlen = (strSize == 0 || *strSize == 0) ?
      (unsigned)strlen ((const char*)utf8Str) + 1 : *strSize;
   unsigned inbufx = 0;
   XMLCHAR16 value;
   size_t startx;
   int nbytes, result = 1;
   unsigned char byte;
   XMLCHAR16* outbuf;
   int outbufsiz = stringUTF8Len (utf8Str, inlen), outbufx = 0;

   if (destBuf == 0 || destBufSize < outbufsiz)
      outbuf = new XMLCHAR16[outbufsiz + 1];
   else
      outbuf = destBuf;

   while (inbufx < inlen) {
      startx = inbufx;

      /* Make sure there is enough space in the output buffer   */
      /* available to hold the parsed character..               */

      if (0 != outbuf) {
         if (outbufx >= outbufsiz) {
            result = 0;
            break;
         }
      }

      /* Process the first byte of the UTF-8 sequence */

      byte = utf8Str[inbufx++];
      if (byte < 0x80) {
         nbytes = 0;
         value = byte;
      }
      else if ((byte & 0xe0) == 0xc0) {
         nbytes = 1;
         value = (XMLCHAR16) (byte & 0x1f);
      }
      else if ((byte & 0xf0) == 0xe0) {
         nbytes = 2;
         value = (XMLCHAR16) (byte & 0x0f);
      }
      else if ((byte & 0xf8) == 0xf0) {
         nbytes = 3;
         value = (XMLCHAR16) (byte & 0x07);
      }
      else if ((byte & 0xfc) == 0xf8) {
         nbytes = 4;
         value = (XMLCHAR16) (byte & 0x03);
      }
      else if ((byte & 0xfe) == 0xfc) {
         nbytes = 5;
         value = (XMLCHAR16) (byte & 0x01);
      }
      else {
         /* invalid encoding error */
         result = 0;
         break;
      }

      /* Read the possible remaining bytes.  */

      while (inbufx < inlen && nbytes > 0) {
         byte = utf8Str[inbufx++];
         --nbytes;
         if ((byte & 0xc0) != 0x80) {
            /* This is an illegal encoding.  */
            result = 0;
            break;
         }
         value <<= 6;
         value |= byte & 0x3f;
      }

      if (!result) break;

      if (0 != outbuf) {
         outbuf[outbufx++] = value;
      }
   }
   if (!result) {
      delete [] outbuf;
      return 0;
   }
   if (strSize != 0)
      *strSize = outbufsiz;
   return (outbuf);
}

XMLCHAR* ASN1XERString::safeTranscode (const XMLCHAR16* str, int* strSize,
                                       XMLCHAR* staticBuf, int bufSize)
{
   if (str == 0) return 0;

   int sz = (strSize == 0) ? stringLen (str) : *strSize;
#ifdef XML_UNICODE
   XMLCHAR* nstr = (sz + 1 > bufSize) ?
                    new XMLCHAR [*strSize + 1] : staticBuf;
   int i;
   for (i = 0; i < sz; i++)
      nstr[i] = (char)str[i];
   nstr[i] = 0;
   return nstr;
#else
   int len = 0, i, j;
   XMLCHAR* res;

   for (i = 0; i < sz; i++) {
      len += rtxUTF8CharSize (str[i]);
   }

   if (len + 1 > bufSize)
      res = new XMLCHAR [len + 1];
   else
      res = staticBuf;

   int remBytes = len;
   for (i = j = 0; j < sz; j++) {
      len = rtxUTF8EncodeChar (*str, (OSOCTET*)&res[i], remBytes);
      if (len > 0) {
         i += len;
         remBytes -= len;
         str++;
      }
      else
         break;
   }
   res[i] = 0;
   if (strSize) *strSize = i;
   return res;
#endif
}

#ifndef XML_UNICODE
XMLCHAR16* ASN1XERString::safeTranscode (const XMLCHAR* str, int* strSize,
                                         XMLCHAR16* staticBuf, int bufSize)
{
   if (str == 0) return 0;

   XMLCHAR16* res = transUTF8 ((const unsigned char*)str, strSize,
                               staticBuf, bufSize);
   return res;
}
#endif

