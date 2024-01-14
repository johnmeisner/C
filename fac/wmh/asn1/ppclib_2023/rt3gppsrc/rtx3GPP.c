/*
 * Copyright (c) 2013-2018 Objective Systems, Inc.
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

#include "rtx3GPP.h"
#include "rtsrc/rtContext.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxBitDecode.h"
#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxTBCD.h"

/* define rotate bits macro. MS VC before 2005 will use _rotl intrinsic. */
#if defined(_MSC_VER) && _MSC_VER < 1400 && defined(_M_IX86)
#define ROTL(value, shift) _rotl (value, shift)
#else
#define ROTL(value, shift) (value >> (32 - shift)) | (value << shift)
#endif

static OSUINT32 mod(OSINT32 a, OSUINT32 b);

static OSUINT32 greatestPow2LtEqTo (OSUINT32 idx)
{
    OSUINT32 j = 1;
    do {
        j *= 2;
    } while (j <= idx);
    j >>= 1;
    return j;
}

EXTRTMETHOD int rtx3GPPFromRangeFormat
(OSUINT32 rfarray[], OSUINT32 rfcount, OSUINT32 range, OSUINT32* presult)
{
   OSINT32 idx, n, j;
   if (!(range == 1024 || range == 512 || range == 256 || range == 128)) {
      return RTERR_BADVALUE;
   }
   idx = (OSINT32) rfcount;
   range -= 1;
   range = range/greatestPow2LtEqTo(rfcount);
   n = rfarray[idx] - 1;

   while (idx > 1) {
      j = greatestPow2LtEqTo (idx);
      range = 2 * range + 1;
      if ((2*idx) < 3*j){ /* left child */
         idx -= j/2;
         n = (n+rfarray[idx]-1+((range-1)/2)+1)%range;
      }
      else { /* right child */
         idx -= j;
         n = (n+rfarray[idx]-1+1)%range;
      }
   }

   if (0 != presult) *presult = (n+1)%1024;

   return 0;
}

static const OSOCTET gsmCharSet[] = {

   '@',  0xa3, '$',  0xa5, 0xe8, 0xe9, 0xf9, 0xec,
   0xf2, 0xc7, '\n', 0xd8, 0xf8, '\r', 0xc5, 0xe5,
   '?',  '_',  '?',  '?',  '?',  '?',  '?',  '?',
   '?',  '?',  '?',  '?',  0xc6, 0xe6, 0xdf, 0xc9,
   ' ',  '!',  '\"', '#',  0xa4,  '%',  '&',  '\'',
   '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
   '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
   0xa1, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
   'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
   'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
   'X',  'Y',  'Z',  0xc4, 0xd6, 0xd1, 0xdc, 0xa7,
   0xbf, 'a',  'b',  'c',  'd',  'e',  'f',  'g',
   'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
   'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
   'x',  'y',  'z',  0xe4, 0xf6, 0xf1, 0xfc, 0xe0
};

static const OSOCTET gsmRevCharSet[] = {
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x0a, 0x3f, 0x3f, 0x0d, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x20, 0x21, 0x22, 0x23, 0x02, 0x25, 0x26, 0x27,
   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
   0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
   0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
   0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
   0x58, 0x59, 0x5a, 0x3f, 0x3f, 0x3f, 0x3f, 0x11,
   0x3f, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
   0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
   0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
   0x78, 0x79, 0x7a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x40, 0x3f, 0x01, 0x24, 0x03, 0x3f, 0x5f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x60,
   0x3f, 0x3f, 0x3f, 0x3f, 0x5b, 0x0e, 0x1c, 0x09,
   0x3f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
   0x3f, 0x5d, 0x3f, 0x3f, 0x3f, 0x3f, 0x5c, 0x3f,
   0x0b, 0x3f, 0x3f, 0x3f, 0x5e, 0x3f, 0x3f, 0x1e,
   0x7f, 0x3f, 0x3f, 0x3f, 0x7b, 0x0f, 0x1d, 0x3f,
   0x04, 0x05, 0x3f, 0x3f, 0x07, 0x3f, 0x3f, 0x3f,
   0x3f, 0x7d, 0x08, 0x3f, 0x3f, 0x3f, 0x7c, 0x3f,
   0x0c, 0x06, 0x3f, 0x3f, 0x7e, 0x3f, 0x3f, 0x3f
};

static OSBOOL isGSMExt (OSOCTET ch)
{
   return (OSBOOL)(ch == 0x0c || ch == '^' || ch == '{' || ch == '}' ||
                   ch == '\\' || ch == '[' || ch == '~' || ch == ']' ||
                   ch == '|');
}

static char gsmExtCharToAscii (OSOCTET value)
{
   switch (value) {
   case 0x0a: return 0x0c; break; /* form feed */
   case 0x14: return '^';  break;
   case 0x28: return '{';  break;
   case 0x29: return '}';  break;
   case 0x2f: return '\\'; break;
   case 0x3c: return '[';  break;
   case 0x3d: return '~';  break;
   case 0x3e: return ']';  break;
   case 0x40: return '|';  break;
   case 0x65: return 0xa4; break; /* euro */
   default: return '?';    break; /* invalid character */
   }
}

static OSOCTET asciiToGsmExtChar (char value)
{
   switch ((unsigned)value) {
   case 0x0c: return 0x0a; /* from feed */
   case '^':  return 0x14;
   case '{':  return 0x28;
   case '}':  return 0x29;
   case '\\': return 0x2f;
   case '[':  return 0x3c;
   case '~':  return 0x3d;
   case ']':  return 0x3e;
   case '|':  return 0x40;
   case 0xa4: return 0x65; /* euro */
   default: return 0x00; /* invalid character */
   }
}

static OSOCTET asciiToGsmChar (char value)
{
   return gsmRevCharSet[(int)value];
}

static char gsmCharToAscii (OSOCTET value)
{
   return (value < (OSOCTET)sizeof(gsmCharSet)) ?
      gsmCharSet[value] : '?';
}

static int mungeChars (OSCTXT* pctxt, OSOCTET* pub1, OSOCTET ub2, OSSIZE j)
{
   int ret;
   OSUINT8 shift = j % 8;
   OSUINT8 mask = (1 << shift) - 1;
   OSUINT8 bits = *pub1;
   bits |= (ub2 & mask) << (8 - shift);

   ret = rtxWriteBytes (pctxt, &bits, 1);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   *pub1 = ub2 >> shift;

   return 0;
}

EXTRTMETHOD int rtx3GPPEncGSMCharStr (OSCTXT* pctxt, const char* value)
{
   OSSIZE j, len;
   OSOCTET ub1, ub2;
   int ret = 0;

   if (0 == value) return 0;

   len = OSCRTLSTRLEN (value);

   for (j = 0; j < len; j++) {
      if (isGSMExt (value[j])) {
         if (j % 8 > 0) {
            ub2 = 0x1b; /* escape char */
            ret = mungeChars (pctxt, &ub1, ub2, j);
         }
         else {
            ub1 = 0x1b;
         }
         ub2 = asciiToGsmExtChar (value[j]);
      }
      else if (j % 8 == 0) {
         ub1 = asciiToGsmChar (value[j]);
      }
      else {
         ub2 = asciiToGsmChar (value[j]);
         ret = mungeChars (pctxt, &ub1, ub2, j);
      }
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   if (len % 8 > 0) {
      /* write last byte */
      ret = rtxWriteBytes (pctxt, &ub1, 1);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   LCHECKNAS (pctxt);

   return 0;
}

static int cvtGsmCharToAscii
(OSOCTET gsmChar, OSBOOL* pEsc, char* outbuf,
 OSSIZE outbufsiz, OSSIZE* poutbufx)
{
   OSSIZE j = *poutbufx;
   /* convert decoded GSM char to ASCII */
   if (gsmChar == 0x1b) { /* escape */
      *pEsc = TRUE;
   }
   else if (*pEsc) {
      *pEsc = FALSE;
      if (j < outbufsiz) {
         outbuf[j++] = gsmExtCharToAscii (gsmChar);
      }
      else return RTERR_BUFOVFLW;
   }
   else if (j < outbufsiz) {
      outbuf[j++] = gsmCharToAscii (gsmChar);
   }
   else return RTERR_BUFOVFLW;

   *poutbufx = j;
   return 0;
}

EXTRTMETHOD int rtx3GPPDecGSMCharStr
(OSCTXT* pctxt, char* outbuf, OSSIZE outbufsiz, OSSIZE nchars)
{
   OSSIZE i, j = 0;
   OSOCTET mask, ub = 0, rest = 0;
   OSBOOL escape = FALSE;
   int bits = 7, ret = 0;

   for (i = 0; i < nchars; i++) {
      mask = ((1 << bits) - 1);
      ub = ((OSRTBUFCUR(pctxt) & mask) << (7 - bits)) | rest;
      rest = OSRTBUFCUR(pctxt) >> bits;
      pctxt->buffer.byteIndex++;

      /* convert decoded GSM char to ASCII */
      ret = cvtGsmCharToAscii (ub, &escape, outbuf, outbufsiz, &j);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if (bits == 1) {
         ret = cvtGsmCharToAscii (rest, &escape, outbuf, outbufsiz, &j);
         if (0 != ret) return LOG_RTERR (pctxt, ret);

         bits = 7; rest = 0; i++;
      }
      else {
         bits--;
      }
   }

   LCHECKNAS (pctxt);

   return 0;
}

/* MCC/MNC pair */

static int encBCDDigit (OSCTXT* pctxt, char digit)
{
   OSUINT8 ub;

   switch (digit) {
   case '*': ub = 0x0A; break;
   case '#': ub = 0x0B; break;
   case 'a': ub = 0x0C; break;
   case 'b': ub = 0x0D; break;
   case 'c': ub = 0x0E; break;
   case '\0': ub = 0x0F; break;
   default:
      if (digit >= '0' && digit <= '9') {
         ub = digit - '0';
      }
      else return RTERR_INVCHAR;
   }

   return rtxEncBits (pctxt, ub, 4);
}

int rtx3GPPEnc_MCC_MNC (OSCTXT* pctxt, const char* mcc, const char* mnc)
{
   int ret;

   /* Encode MCC and MNC digits */
   RTDIAG_NEWBITFIELD (pctxt, "mcc/mnc");

   ret = encBCDDigit (pctxt, mcc[1]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = encBCDDigit (pctxt, mcc[0]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (mnc[2] == 0x0F || mnc[2] == 0) {
      ret = rtxEncBits (pctxt, 0x0F, 4);
   }
   else {
      ret = encBCDDigit (pctxt, mnc[2]);
   }
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = encBCDDigit (pctxt, mcc[2]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = encBCDDigit (pctxt, mnc[1]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = encBCDDigit (pctxt, mnc[0]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   LCHECKNAS (pctxt);

   return 0;
}

static int decTBCDigit (OSCTXT* pctxt, char* pdigit)
{
   int ret;
   OSUINT8 bcdDigit;

   ret = rtxDecBitsToByte (pctxt, &bcdDigit, 4);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   switch (bcdDigit) {
   case 0x0A: *pdigit = '*'; break;
   case 0x0B: *pdigit = '#'; break;
   case 0x0C: *pdigit = 'a'; break;
   case 0x0D: *pdigit = 'b'; break;
   case 0x0E: *pdigit = 'c'; break;
   case 0x0F: *pdigit = '\0'; break;
   default: *pdigit = bcdDigit + '0';
   }

   return 0;
}

int rtx3GPPDec_MCC_MNC (OSCTXT* pctxt, char mcc[4], char mnc[4])
{
   int ret;

   /* Decode MCC and MNC digits */
   RTDIAG_NEWBITFIELD (pctxt, "mcc/mnc");

   ret = decTBCDigit (pctxt, &mcc[1]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = decTBCDigit (pctxt, &mcc[0]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = decTBCDigit (pctxt, &mnc[2]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = decTBCDigit (pctxt, &mcc[2]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = decTBCDigit (pctxt, &mnc[1]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = decTBCDigit (pctxt, &mnc[0]);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   mcc[3] = mnc[3] = '\0';

   LCHECKNAS (pctxt);

   return 0;
}

/*
Decode the non-imperative part of an L3 message (TS 24.008
section 9.3) having an empty non-imperative part, excepting the SHIFT IE
which is defined as known.

This will report an error if a comprehension required IE is found.
*/
int rtx3GPPDecL3NonImperative (OSCTXT* pctxt, OSBOOL callCtrl)
{
   /* All known IEIs are assumed to be in codeset 0, and IEI's outside
      codeset 0 are unknown.
   */
   OSUINT8 lockedCodeset = 0;   /* locked-in IEI codeset */

   int ret;

   while (rtxCtxtContainerHasRemBits(pctxt)) {
      OSUINT8 iei;              /* the current iei */
      OSUINT8 skipBytes = 0;    /* number of bytes to skip to get past
                                   unprocessed IE */

      OSUINT8 activeCodeset = lockedCodeset;

      /* Decode IEI */
      RTDIAG_NEWBITFIELD (pctxt, "IEI");

      ret = rtxDecBitsToByte (pctxt, &iei, 8);
      if (ret == RTERR_ENDOFBUF) break;
      else if (0 != ret) return LOG_RTERR (pctxt, ret);

      RTDIAG_SETBITFLDCOUNT (pctxt);

      if (callCtrl) {
         OSUINT8 nibble = iei >> 4;      /* nibble == high nibble */

         /* Handle any series of shift IEs */
         while ( nibble == SHIFT_IEI ) {
            OSUINT8 codeset = iei & 0x07;  /* lowest 3 bits */
            OSBOOL locking = (iei & 0x08 ) == 0;   /* bit 4 clear => locking */
            if ( locking && codeset > lockedCodeset ) {
              lockedCodeset = codeset;
              activeCodeset = codeset;
            }
            else if ( !locking ) activeCodeset = codeset;

            /* move to next IEI */
            RTDIAG_NEWBITFIELD (pctxt, "IEI");

            if ( rtxCtxtGetContainerRemBits(pctxt) < 8 ) return 0;
            ret = rtxDecBitsToByte (pctxt, &iei, 8);
            if (0 != ret) return LOG_RTERR (pctxt, ret);

            RTDIAG_SETBITFLDCOUNT (pctxt);
            nibble = iei >> 4;   /* nibble == high nibble */
         }
      }

      /* Note: comprehension is not required for TV type 1 or T type 2 */
      if ( !(iei & 0x80 ) ) {
         /* bit 8 is set 0; assuming TLV type 4 */
         ret = rtxDecBitsToByte (pctxt, &skipBytes, 8);
         if (0 != ret) return LOG_RTERR (pctxt, ret);

         /* if comprehension scheme applies then set comprehension required
            active codeset of 0 && high nibble all zeros <==> comprehension
            required */
         if (activeCodeset == 0 && (( iei & 0x0F0 ) == 0)) {
            rtxErrAddStrParm (pctxt, "comprehension required on unknown IEI");
            return LOG_RTERR (pctxt, RTERR_NOTSUPP);
         }

         if ( skipBytes != 0 ) {
            ret = rtxSkipBits (pctxt, skipBytes * 8);
            if (0 != ret) return LOG_RTERR (pctxt, ret);
         }
      }
      /* else bit 8 set to 1; leave skipBytes == 0 for 1-octet IE */
   }

   LCHECKNAS (pctxt);

   return 0;
}

/* Common functions for encoding/decoding several alternatives of
   MobileIdentity */

int rtx3GPPEncMobileIdentityType1
(OSCTXT* pctxt, OSUINT8 type, const char* pstr)
{
   OSSIZE  len;
   OSBOOL  odd;
   OSUINT8 ub;
   int     ret;

   if (0 == pstr) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   len = OSCRTLSTRLEN (pstr);
   odd = ((len % 2) == 1);

   /* Encode length */
   RTDIAG_NEWBITFIELD (pctxt, "length");

   ret = rtxEncBits (pctxt, (OSUINT32)(len/2 + 1), 8);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   /* Encode first digit */
   RTDIAG_NEWBITFIELD (pctxt, "digit1");

   ret = encBCDDigit (pctxt, *pstr++);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   /* Encode odd/even indicator */
   RTDIAG_NEWBITFIELD (pctxt, "odd");

   ret = rtxEncBit (pctxt, odd);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   /* Encode type of identity */
   RTDIAG_NEWBITFIELD (pctxt, "type");

   ret = rtxEncBits (pctxt, type, 3);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   RTDIAG_SETBITFLDCOUNT (pctxt);

   /* Encode remaining digits */
   RTDIAG_NEWBITFIELD (pctxt, "");

   while (*pstr != '\0') {
      ret = rtxTBCDCharToBin (*pstr++, &ub);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if (*pstr != '\0') {
         OSOCTET ub2;

         ret = rtxTBCDCharToBin (*pstr++, &ub2);
         if (0 != ret) return LOG_RTERR (pctxt, ret);

         ub |= (OSOCTET)(ub2 << 4);
      }
      else { ub |= 0x0F0; }

      ret = rtxEncBits (pctxt, ub, 8);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   RTDIAG_SETBITFLDCOUNT (pctxt);

   LCHECKNAS (pctxt);

   return 0;
}

int rtx3GPPDecMobileIdentityType1
(OSCTXT* pctxt, OSUINT8 len, OSBOOL odd, OSUINT8 digit1,
 char* strbuf, OSSIZE bufsize)
{
   OSUINT8 i, ndigits;
   OSUINT8 digit2;
   char*   pstr = strbuf;
   int     ret;

   len--;
   ndigits = len * 2;
   if (odd) ndigits++;

   if (ndigits >= bufsize) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   if (digit1 <= 9) {
      *pstr++ = digit1 + '0';
   }
   else return LOG_RTERR (pctxt, RTERR_INVCHAR);

   RTDIAG_NEWBITFIELD (pctxt, "");

   for (i = 0; i < len - 1; i++) {
      /* Decode 2nd digit (p + 1) */
      ret = rtxDecBitsToByte (pctxt, &digit2, 4);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      /* Decode first digit (p) */
      ret = rtxDecBitsToByte (pctxt, &digit1, 4);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      /* Convert digits to text */
      ret = rtxTBCDBinToChar (digit1, pstr++);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      ret = rtxTBCDBinToChar (digit2, pstr++);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   /* Decode 2nd digit (p + 1) */
   ret = rtxDecBitsToByte (pctxt, &digit2, 4);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   /* Decode first digit (p) */
   ret = rtxDecBitsToByte (pctxt, &digit1, 4);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   /* Convert digits to text */
   ret = rtxTBCDBinToChar (digit1, pstr++);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (odd) {
      ret = rtxTBCDBinToChar (digit2, pstr++);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   RTDIAG_SETBITFLDCOUNT (pctxt);

   *pstr = '\0';

   LCHECKNAS (pctxt);

   return 0;
}


EXTRTMETHOD int rtx3GPPDecIntAsBits(OSCTXT* pctxt, OSUINT8* pvalue, OSUINT8 bit)
{
   int ret;
   OSBOOL decbit;
   *pvalue = 0;

   for(;;) {
      ret = rtxDecBit(pctxt, &decbit);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if ( decbit == bit ) {
         if ( *pvalue == 255 ) {
            return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
         }
         (*pvalue)++;
      }
      else break;
   }
   return 0;
}

EXTRTMETHOD int rtx3GPPDecIntAsBitsPattern(OSCTXT* pctxt, OSUINT8* pvalue,
                                             OSUINT8 pattern)
{
   int ret;
   OSBOOL decbit;
   *pvalue = 0;

   for(;;) {
      ret = rtxDecBit(pctxt, &decbit);
      if (0 != ret) return LOG_RTERR (pctxt, ret);

      if ( decbit == rtx3GPPGetPatternBitPrev(pctxt, pattern) ) {
         if ( *pvalue == 255 ) {
            return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
         }
         (*pvalue)++;
      }
      else break;
   }
   return 0;
}

EXTRTMETHOD int rtx3GPPEncIntAsBits(OSCTXT* pctxt, OSUINT8 value, OSUINT8 bit)
{
   int ret;
   int i;

   for(i = 0; i < value; i++) {
      ret = rtxEncBit(pctxt, bit);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   ret = rtxEncBit(pctxt, ~bit);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   return 0;
}

EXTRTMETHOD int rtx3GPPEncIntAsBitsPattern(OSCTXT* pctxt, OSUINT8 value,
                                          OSUINT8 pattern)
{
   int ret;
   int i;

   for(i = 0; i < value; i++) {
      ret = rtxEncBit(pctxt, rtx3GPPGetPatternBit(pctxt, pattern));
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   ret = rtxEncBit(pctxt, rtx3GPPGetPatternBit(pctxt, ~pattern));
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   return 0;
}

EXTRTMETHOD int rtx3GPPDecAltDetmnt(OSCTXT* pctxt, OSUINT32* pDetmnt,
   size_t detmntBits, OSUINT32 lhmask)
{
   int stat;
   OSUINT32 encode_value;
   OSUINT32 pattern = 0x2B2B2B2B;
   OSUINT32 shift;

   /*We would shift left by 8 - bitOffset to align the pattern with the
      MSB of the 32-bit value, but then we would have to right shift it
      32-detmntBits to align it with the highest bit actually being decoded.
      This is equivalent to shifting left 8 - offset + detmntBits, but
      shifting any multiple of 8 is pointless, due to the repetition, so we
      take it modulo 8. */
   shift = (8 - pctxt->buffer.bitOffset + detmntBits) % 8;

   /* Rotate pattern according to the context's buffer offset so that we can
      XOR it with the decoded bit to get the determinant */
   if ( shift != 0 )
      pattern = ROTL(pattern, shift);
   /* else: no rotation is required */

   /* For any bit that is 0 in lhmask (indicating that bit is 0/1 and not L/H)
      set that bit to 0 in the pattern.
      We have to align the lhmask with the bits in detmnt. */
   pattern = (lhmask >> (32-detmntBits)) & pattern;

   stat = rtxDecBits(pctxt, &encode_value, detmntBits);
   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

   /* Now XOR detmnt with pattern.  Every 0/1 bit will be XOR'd with 0, giving
      0/1, while every bit encoding an L/H value will be XOR'd with the bit
      used to encode L, given 0 for L bits and 1 for H bits. */
   *pDetmnt = encode_value ^ pattern;

   return 0;
}

EXTRTMETHOD int rtx3GPPDecVarAltDetmnt(OSCTXT* pctxt, OSUINT32* detmnt,
   OS3GPPAltDecodeTabEntry* table, size_t tableSize)
{
   int result = 0;
   size_t entry = 0;
   for(;;) {
      OSBOOL bit;
      OS3GPPAltDecodeAction* pAction;
      result = rtxDecBit(pctxt, &bit);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);

      if (table[entry].lhflag) {
         /* set bit to 0 if decoded L; 1 if decoded H */
         bit = ( bit == rtx3GPPGetPatternBitPrev(pctxt, 0x2B) );
      }
      /* else: no L/H translation required */

      if ( bit ) pAction = &table[entry].oneHAction;
      else pAction = &table[entry].zeroLAction;

      if ( pAction->gotoEntry > 0 ) {
         if ( entry >= tableSize ) {
            /* This should never happen unless we create a poorly defined table
               in the generated code.
            */
            return LOG_RTERRNEW(pctxt, RTERR_OUTOFBND);
         }
         entry = pAction->gotoEntry;
      }
      else if ( pAction->gotoEntry == 0 ) {
         *detmnt = pAction->choiceId;
         return 0;
      }
      else return LOG_RTERRNEW(pctxt, RTERR_INVOPT);
   }
   return result;
}


EXTRTMETHOD int rtx3GPPEncAltDetmnt(OSCTXT* pctxt, OSUINT32 detmnt,
   size_t detmntBits, OSUINT32 lhmask)
{
   /* Note: if lhmask == 0, we can just encode the highest bits of detmnt.  The
      following is only needed if lhmask != 0.  Similarly if lhpattern were 0.
   */

   OSUINT32 pattern = 0x2B2B2B2B;
   OSUINT32 encode_value;
   OSUINT32 shift;

   /* We would shift left by 8 - bitOffset to align the pattern with the
      MSB of the 32-bit value, but then we would have to right shift it
      32-detmntBits to align it with the highest bit actually being encoded.
      This is equivalent to shifting left 8 - offset + detmntBits, but
      shifting any multiple of 8 is pointless, due to the repetition, so we
      take it modulo 8. */
   shift = (8 - pctxt->buffer.bitOffset + detmntBits) % 8;

   /* Rotate pattern according to the context's buffer offset so that we can
      XOR it with detmnt to get the bits that should be encoded. */
   if ( shift != 0 )
      pattern = ROTL(pattern, shift);
   /* else: no rotation is required */

   /* For any bit that is 0 in lhmask (indicating that bit is 0/1 and not L/H)
      set that bit to 0 in the pattern.
      We have to align the lhmask with the bits in detmnt. */
   pattern = (lhmask >> (32-detmntBits)) & pattern;

   /* Now XOR detmnt with pattern.  Every 0/1 bit will be XOR'd with 0, giving
      0/1, while every L/H bit (represented as 0/1) will be XOR'd with the bit
      used for L, giving L for L and H for H.
   */
   encode_value = pattern ^ detmnt;

   return rtxEncBits(pctxt, encode_value, detmntBits);
}


static int ptable[] = {0, 10, 19, 28, 36, 44, 52, 60, 67, 74, 81, 88, 95,
   102, 109, 116, 122};

EXTRTMETHOD int TS44018Msg_p(OSUINT8 n) {
   if ( n >= 17 ) return 0;
   else return ptable[n];
}


static int qtable[] = {0, 9, 17, 25, 32, 39, 46, 53, 59, 65, 71, 77, 83, 89, 95,
   101, 106, 111, 116, 121, 126};

EXTRTMETHOD int TS44018Msg_q(OSUINT8 n) {
   if ( n >= 21 ) return 0;
   else return qtable[n];
}


EXTRTMETHOD int rtx3GPPGetARFCNRange(OSUINT16* arfcns,
   OSUINT16 arfcnCount, OSUINT16* pF0, OSUINT16* pRange)
{
   OSUINT16* pLast = arfcns + arfcnCount - 1;   /* last entry in array */
   OSUINT16* pArfcn; /* pointer to a candidate for F0 */

   /* if no ARFCNs, range is 0 */
   if ( arfcnCount == 0 ) {
      *pRange = 0;
      *pF0 = 0;
      return 0;
   }

   *pRange = 1024;

   /* for each arfcn in arfcns, check largest distance with other arfcns
      and whether that is a smaller distance than we have found thus far */
   for(pArfcn = arfcns; pArfcn <= pLast; pArfcn++) {
      /*maxDiff is greatest distance bt. pArfcn and another ARFCN. maxDiff + 1
         gives what the range would be using pArfcn as FO */
      OSUINT16 maxDiff = 0;
      OSUINT16* pArfcn2;

      for(pArfcn2 = arfcns; pArfcn2 <= pLast; pArfcn2++) {
         OSUINT16 diff;    /* arfcn2 - arfcn (modulo 1024) */
         diff = mod(*pArfcn2 - *pArfcn, 1024);

         if ( diff > maxDiff ) maxDiff = diff;
      }

      if ( maxDiff + 1 < *pRange ) {
         /* pArfcn gives us a smaller range than we already have */
         *pRange = maxDiff + 1;
         *pF0 = *pArfcn;

         /* can't find maxDiff to be less than count - 1 */
         if ( *pRange == arfcnCount ) return 0;
      }
   }

   return 0;
}


/*
total_bits_* arrays are used by FreqListFormat, which see.
*/
static const OSUINT16 total_bits_1024[] =
   {0, 10, 28, 60, 116, 212, 372, 628, 1012, 1524, 2036};

static const OSUINT16 total_bits_512[] =
   {0, 9, 25, 53, 101, 181, 309, 501, 757, 1013};

static const OSUINT16 total_bits_256[] =
   {0, 8, 22, 46, 86, 150, 246, 374, 502};

static const OSUINT16 total_bits_128[] =
   {0, 7, 19, 39, 71, 119, 183, 247};


typedef struct FreqListFormat
{
   /*
   The total_bits array is used to determine how many bits certain groups of W
   values will take up in the various formats.

   Consider each group of k-bit values encoded in the format as a bucket.
   Then total_bits[n] is the total number of bits used to encode the first n
   buckets when those buckets are completely filled.  (total_bits[0] == 0)

   For example, in format 1024, the first bucket has 1 10-bit value and the
   second bucket has 2 9-bit values.  Thus, total_bits[2] = 28 = 10 + 2 * 9.

   It determining how many buckets are filled, it is useful to note that when
   the first n buckets are completely filled, they hold 2^n - 1 values.
   */
   const OSUINT16* total_bits;

   /* range: The reange encode_subtree is first invoked with for this format. */
   const OSUINT16 range;

   /* bitsPerValue: bits per value in first group; also # of groups */
   const OSUINT8 bitsPerValue;

   /* leadingBits: # of bits in the encoding that precede the W values */
   const OSUINT8 leadingBits;

   /* startBit: The bit in the IE where the encoding of the format begins.
   E.g. for format 1024, it is 3 as the F0 field is encoded in bit 3. */
   const OSUINT8 startBit;
} FreqListFormat;

static const FreqListFormat FORMAT_1024 =
   {  total_bits_1024, 1023, 10, 1, 3 };

static const FreqListFormat FORMAT_512 =
   {  total_bits_512, 511, 9, 10, 1 };

static const FreqListFormat FORMAT_256 =
   {  total_bits_256, 255, 8, 10, 1 };

static const FreqListFormat FORMAT_128 =
   {  total_bits_128, 127, 7, 10, 1 };


/**
 * Locate a value, pivot, in the given values which divides the set such that
 * exactly (numValues -1)/2 values are between pivot + 1 (modulo range) and
 * pivot + half the range (modulo range).
 *
 * More precisely, pivot is chosen so that there are exactly (numValues - 1)/2
 * values v such that (v - pivot) % range <= (range -1) / 2 and v != pivot.
 *
 * @param values Array of values.  All values lie between 0 and range - 1,
 *    inclusive.  Each value is distinct.
 * @param numValues The number of values in the array.
 * @param range  The range.  range >= numValues.
 * @return Returns a pointer to the chosen pivot value, which is a value in
 *       values.  If numValues == 0, returns 0.  Otherwise, a return of 0
 *       indicates that values did not meet the precondition given above.
 */
static OSUINT16* locate_pivot_value(OSUINT16* values, OSSIZE numValues,
      OSUINT16 range )
{
   OSUINT16* pI;  /* loop var & candidate pivot */
   OSUINT16* pLast = values + numValues - 1;
   OSUINT16 halfRange = ( range -1 ) / 2;
   OSSIZE halfValues = (numValues - 1) / 2;

   /* test each value to see if it can be pivot */
   for(pI = values; pI <= pLast; pI++) {
      /* count how many values fall in the specified range */
      OSSIZE valueCount = 0;
      OSUINT16* pJ;
      for (pJ = values; pJ <= pLast; pJ++) {
         OSINT32 remainder = mod(*pJ - *pI, range);
         if ( remainder <= halfRange ) {
            valueCount++;
         }
      }

      /* use valueCount - 1 because pI itself will have been counted. */
      if ( valueCount-1 == halfValues ) {
         return pI;
      }
   }

   return 0;
}


/**
 * Set a number of bits in a bit string to the given value.
 *
 * An error will be returned if the given value cannot fit in the given number
 * of bits, or if the given number of bits do not fit in the bit string starting
 * at the given offset.
 *
 * @param pctxt   The context structure; used for logging errors.
 * @param value   The value to set into the given number of bits.
 * @param bitstr  The bitstring in which the given bits should be set.
 * @param octets  The number of octets in the bitstring
 * @param offset  The bit offset in bitstr where value should begin.
 *                The first bit has offset 0.
 * @param numbits The number of bits that should be set according to the value.
 */

static int setValue( OSCTXT* pctxt, OSUINT16 value,
   OSOCTET* bitstr, OSSIZE octets, OSSIZE offset, OSUINT8 numbits)
{
   OSOCTET* pbyte;
   OSOCTET octet;
   OSUINT8 usedbits;

   /* Check that value can be encoded in the given number of bits */
   if ( value >= (1 << numbits) ) return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);

   /* Check that we won't go outside the bitstr array */
   if ( (offset + numbits + 7) / 8 > octets )
      return LOG_RTERRNEW(pctxt, RTERR_OUTOFBND);


   /* Locate the nearest prior byte boundary from offset.  This is the first
      byte we'll touch.  We'll touch as many bytes as necessary to set the
      given number of bits. */
   pbyte = bitstr + offset / 8;

   usedbits = offset % 8;   /* # of bits already used in pbyte */

   for(;;)
   {
      /* set octet so that we can OR it with pbyte.  Conceptually,
            - left shift (16 - numbits) to move first bit into msb
            - right shift 8 to align that with first bit of lower byte
            - right shift usedbits to adjust for bits I can't touch
         On balance, this could be a left or right shift.
      */
      int shift = (16 - numbits) - 8 - usedbits;
      if ( shift >= 0 ) octet = value << shift;
      else octet = value >> -shift;

      *pbyte |= octet;

      /* we will have just set 8 - usedbits bits
         shift > 0 <-> 8 - usedbits >= numbits
       */
      if ( shift > 0 ) break;
      else numbits = -shift; /* == numbits - (8 - usedbits) */

      /* in the next byte, no bits are alredy used.  Move to next byte. */
      usedbits = 0;
      pbyte++;
   }

   return 0;
}


/**
 * Assign W[idx] = value, where the W values are held in the given bitstring.
 *
 * The W values are arranged in a tree (see 44.018 Annex J).  Each level in
 * the tree is a generation.
 *
 * @param pctxt   The context structure; used for logging errors.
 * @param idx     The index into W to assign (1, 2, ...)
 * @param value   The value to assign to W[idx]
 * @param bitstr  The bitstring holding the encoding.
 * @param octets  The number of octets in the bitstring
 * @param offset  The bit offset in bitstr where the encoding for the current
 *                generation begins.  The first bit has offset 0.
 * @param nodes   The (maximum) number of nodes that may appear in the current
 *                generation.
 * @param bitsPerValue  The number of bits used to encode each value in the
 *                current generation.
 */

static int assignWindex( OSCTXT* pctxt, OSUINT16 idx, OSUINT16 value,
   OSOCTET* bitstr, OSSIZE octets, OSSIZE offset, OSSIZE nodes,
   OSUINT8 bitsPerValue)
{
   /* The index for the first node in a generation is the number of nodes
      in that generation.
      k: idx is the kth node of the generation (k>=0)
   */
   OSSIZE k = idx - nodes;

   LCHECKNAS (pctxt);

   offset = offset + k * bitsPerValue; /* bit offset for index */

   return setValue(pctxt, value, bitstr, octets, offset, bitsPerValue);
}

/**
 * Given a positive b, return a % b as a nonnegative remainder, r,
 * 0 <= r < b.
 *
 * Motiviation: if a is negative, C and C++11 return a negative remainder in
 * this case.  C++03 has implementation-defined behavior.
 */
static OSUINT32 mod(OSINT32 a, OSUINT32 b)
{
   if ( a >= 0 ) return a % b;
   else {
      OSUINT32 rem = -a % b;
      if ( rem == 0 ) return 0;
      else return b - rem;
   }
}

/**
 * Implementation of encoding algorithm for frequency list from 44.018 Annex J.
 *
 * @param pctxt   The context structure; used for logging errors.
 * @param idx     Index to the W value to be set.
 * @param values List of values.  What is passed depends on the format being
 *    encoded.
 * @param numValues The number of values in values.
 * @param range The range of the values.  All values in values are between
 *       (inclusive) 0 and range-1, and are distinct.
 * @param bitstr The bit string to encode into
 * @param octets The number of octets allocated to bitstr.
 * @param offset The bit offset (first bit is 0) to begin encoding at.
 * @param nodes   The (maximum) number of nodes that may appear in the current
 *                generation.
 * @param bitsPerValue  The number of bits used to encode each value in the
 *                current generation.
 */
static int encode_subtree(OSCTXT* pctxt, OSUINT16 idx, OSUINT16* values,
      OSSIZE numValues, OSUINT16 range, OSOCTET* bitstr, OSSIZE octets,
      OSSIZE offset, OSSIZE nodes, OSUINT8 bitsPerValue )

{
   int result = 0;
   const OSUINT16 HALF_RANGE = (range - 1) / 2;
   const OSUINT16 RANGE_DIV2 = range / 2;
   const OSSIZE HALF_VALUES = (numValues - 1) / 2;
   const OSSIZE VALUES_DIV2 = numValues / 2;

   OSUINT16* pPivot;    /* pointer to pivot value; replaces INDEX_IN_SET */
   OSUINT16 tmp;

   OSUINT16 leftOrigin; /* origin for left half (value opposite pivot) */
   OSUINT16 rightOrigin; /* origin for right half (pivot + 1) */

   /*pLastLeft: points to the location that will hold the last value in the
      left half of the values when we split */
   OSUINT16* pLastLeft;
   OSUINT16* pLeft, *pRight; /*pointer into left/right side during shuffling*/


   if ( numValues == 0 ) {
      result = assignWindex(pctxt, idx, 0, bitstr, octets, offset, nodes,
                     bitsPerValue);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);
      return 0;
   }
   else if ( numValues == 1 ) {
      result = assignWindex(pctxt, idx, 1 + *values, bitstr, octets, offset,
                              nodes, bitsPerValue);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);
      return 0;
   }

   pPivot = locate_pivot_value(values, numValues, range);
   if ( pPivot == 0 ) {
      rtxErrAddStrParm(pctxt, "\nUnable to locate pivot value.");
      rtxErrAddStrParm(pctxt, "\nCheck for uniqueness of ARFCN values.");
      return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
   }

   result = assignWindex(pctxt, idx, *pPivot + 1, bitstr, octets, offset,
                           nodes, bitsPerValue);
   if ( result < 0 ) return LOG_RTERR(pctxt, result);

   /* Take the two halves of the range split by pPivot, and encode each half
      as a subtree.  The algorithm in 44.018 Annex J creates two new sets to
      do this.  We're going to use our existing array and shuffle elements into
      two halves of the array.  We can do this because the order doesn't matter.

      The left hand side will have numValues/2 values and the right hand side
      will have (numValues - 1) / 2 (possibly 0) values.
   */

   /*first, move pivot value to the end of the array so it is out of the way -
      it doesn't belong to left or right side.
   */
   tmp = values[numValues-1];
   values[numValues-1] = *pPivot;
   *pPivot = tmp;
   pPivot = values + numValues - 1; /* repoint at pivot */

   leftOrigin = (*pPivot + HALF_RANGE + 1) % range;
   rightOrigin = (*pPivot + 1) % range;

   pRight = values + VALUES_DIV2;
   pLastLeft = pRight - 1;
   for(pLeft = values; pLeft <= pLastLeft; pLeft++) {
      /* Examine each value in the left hand side of the array and swap any
         that don't belong with one on the right. */
      OSUINT32 leftRemainder = mod(*pLeft - leftOrigin, range);
      OSUINT32 rightRemainder;

      if ( leftRemainder >= RANGE_DIV2 ) {
         /* This value belongs in right half. Find next value in right half that
            belongs in left half and swap. */
         for(; pRight < pPivot; pRight++) {
            rightRemainder = mod(*pRight - rightOrigin, range);
            if ( rightRemainder >= RANGE_DIV2 ) {
               /* pRight belongs on left.  Swap values. */
               tmp = *pLeft;
               *pLeft = *pRight;
               *pRight = tmp;
               break;
            }
            else {
               /* this value belongs where it is.  Apply value adjustment. */
               *pRight = rightRemainder;
            }
         }

         /* We have swapped right & left.  Now we need to apply the value
         adjustment. The leftRemainder was calculated relative to the
         leftOrigin, but on the right value, and similarly for the right value.
         It happens, however, that, for a given value, the left_remainder and
         right_remainder (computed to the left and right origin) have a
         relationship:
            (1) left_remainder = ( right_remainder - (range-1)/2) % range
            (2) right_remainder = (left_remainder + (range-1)/2) % range
         Because we know range > rightRemainder >= RANGE_DIV2 >= HALF_RANGE,
         (1) can be simplified to:
            (3) left_remainder = right_remainder - (range-1)/2
         */
         *pLeft = rightRemainder - HALF_RANGE;
         *pRight = (leftRemainder + HALF_RANGE ) % range;
         pRight++;
      }
      else {
         /* this value is fine where it is.  Apply value adjustment. */
         *pLeft = leftRemainder;
      }
   }

   /* The values are now shuffled to the correct sides since all values on the
      left belong there and there are exactly that many left values, as a
      result of locate_pivot_value's behavior.  It is left to adjust the value
      for the remaining RHS values. */
   for(; pRight < pPivot; pRight++) {
      *pRight = mod(*pRight - rightOrigin, range);
   }

   /* When we recurse, the offset is increated by the # of nodes for this
      generation * the number of bits; the number of nodes doubles;
      the number of bits decreases by 1.
   */

   /* recurse for left hand and right hand sides */
   result = encode_subtree( pctxt,
      idx + greatestPow2LtEqTo(idx) /*idx */,
      values /*values*/,
      VALUES_DIV2 /*numValues*/,
      RANGE_DIV2 /*range*/, bitstr, octets,
      offset + nodes * bitsPerValue /*offset */,
      2 * nodes /* nodes */,
      bitsPerValue -1 /* bitsPerValue*/);

   if ( result < 0 ) return LOG_RTERR(pctxt, result);

   /* As long as numValues > 2, there will be 1 or more values on the RHS. */
   if ( numValues > 2 ) {
      result = encode_subtree( pctxt,
                  idx + 2*greatestPow2LtEqTo(idx) /*idx*/,
                  values + VALUES_DIV2 /*values*/,
                  HALF_VALUES /*numValues*/,
                  HALF_RANGE /*range*/,
                  bitstr, octets,
                  offset + nodes * bitsPerValue /*offset*/,
                  2 * nodes /*nodes*/,
                  bitsPerValue -1 /*bitsPerValue*/);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);
   }
   return 0;
}

/**
 * Return largest m such that 2^m <= n.
 * n must be > 0.
 */
static OSUINT32 int_log2(OSUINT32 n) {
   OSUINT32 result = 0;

   OSRTASSERT(n != 0);

   for(;;) {
      n = n >> 1;
      if ( n == 0 ) return result;
      else result++;
   }
}

/**
 * Return FreqListFormat structure for given format.
 * Return NULL if the format is not one of the range formats.
 */
static const FreqListFormat* formatInfo(OS3GPP_FREQ_LIST_FMT format)
{
   switch (format) {
      case OS3GPP_FREQ_LIST_FMT_RANGE_1024: return &FORMAT_1024;
      case OS3GPP_FREQ_LIST_FMT_RANGE_512: return &FORMAT_512;
      case OS3GPP_FREQ_LIST_FMT_RANGE_256: return &FORMAT_256;
      case OS3GPP_FREQ_LIST_FMT_RANGE_128: return &FORMAT_128;
      default: return NULL;
   }
}


EXTRTMETHOD int rtx3GPPEncARFCN(OSCTXT* pctxt, OSUINT16* arfcns,
   OSUINT16 arfcnCount, OS3GPP_FREQ_LIST_FMT format,  OSUINT16 range,
   OSUINT16 f0, OSOCTET** pBuffer, OSSIZE bufferSize, OSSIZE* pNumbits)
{
   int result = 0;
   OSOCTET* buffer = *pBuffer;  /* == *pBuffer */

   if ( format == OS3GPP_FREQ_LIST_FMT_BITMAP0 ) {
      OSUINT16 idx = 0;   /*loop var*/

      /* buffer size must be 16 for this format.  We ensure that all bits
         are initialized to 0 so that we only need to set bits corresponding
         to ARFCN in the array. */
      if ( buffer == 0 ) {
         *pBuffer = buffer = rtxMemAllocArrayZ(pctxt, 16, OSOCTET);
         bufferSize = 16;
      }
      else if ( bufferSize < 16 ) {
         return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
      }
      else {
         memset(buffer, 0, 16);
      }

      for(idx = 0; idx < arfcnCount; idx++) {
         OSUINT16 arfcn = arfcns[idx];

         if ( arfcn < 1 || arfcn > 124 ) {
            rtxErrAddStrParm(pctxt, "ARFCN not between 1 and 124");
            return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
         }

         /* ARFCN-n is at bit 124-n (0 being first bit) */
         setValue(pctxt, 1, buffer, bufferSize, 124 - arfcn /*offset*/,
                     1 /*numbits*/);
      }
   }
   else if ( format == OS3GPP_FREQ_LIST_FMT_VAR_BITMAP ) {
      /*
      Without padding, orig_arfcn and following will use 9 + range bits.
      The IE is laid out so that the first bit is the last bit of an octet,
      so 9 + range - 1 bits will be encoded with padding to be a multiple of 8.
      Therefore, to hold the entire encoding, with the padding, we'll need:
         reqdBytes = 1 + (9 + range - 1 + 7 ) / 8
                     = 2 + (range + 7) / 8
      */
      OSSIZE minBuffer = 2 + (range + 7) / 8;
      OSUINT16 idx;   /* loop var */

      if ( buffer == 0 ) {
         *pBuffer = buffer = rtxMemAllocArrayZ(pctxt, minBuffer, OSOCTET);
         bufferSize = minBuffer;
      }
      else if ( bufferSize < minBuffer ) {
         return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
      }
      else {
         memset(buffer, 0, bufferSize);
      }

      /* take F0 as orig_arfcn */
      result = setValue(pctxt, f0, buffer, bufferSize, 0, 10);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);

      for(idx = 0; idx < arfcnCount; idx++) {
         OSUINT16 arfcn = arfcns[idx];
         OSUINT16 offset;

         if ( arfcn == f0) continue;

         /* if arfcn == orig_arfcn + N (mod 1024) the offset is 9 + N */
         offset = 9 + mod(arfcn - f0, 1024);

         result = setValue(pctxt, 1, buffer, bufferSize, offset, 1 /*numbits*/);
         if ( result < 0 ) return LOG_RTERR(pctxt, result);
      }

      *pNumbits = (minBuffer - 1) * 8 + 1;
   }
   else /* range 1024, 512, 256, 128 */
   {
      OSUINT16* values;     /* copy of ARFCN that we can modify */
      OSUINT16 nvalues = arfcnCount;    /* # values in values */
      OSSIZE idx;          /* loop var */
      OSUINT32 n, k, bits;    /* computation of required buffer size */
      OSSIZE reqdBytes;     /* the required buffer size */
      OSUINT16 decrementBy;   /* value to subtract from each ARFCN */
      OSUINT16* pZero = NULL; /* pointer to ARFCN = 0, if present */
      OSUINT16* pF0;          /* pointer to ARFCN == f0 */

      const FreqListFormat* pFormat;

      pFormat = formatInfo(format);

      if ( range > pFormat->range + 1 ) {
         rtxErrAddStrParm(pctxt, "\nRange too large for chosen format.");
         return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      /* create copy of arfcns that we'll modify and pass on to encode_subtree
       */
      values = rtxMemAlloc(pctxt, sizeof(OSUINT16) * arfcnCount );
      if ( values == 0 ) return LOG_RTERR(pctxt, RTERR_NOMEM);

      memcpy(values, arfcns, sizeof(OSUINT16) * arfcnCount);

      /* Run over the ARFCN values in values and subtract the decrementBy
         from each value (subtraction is modulo 1024).  Also, locate 0, if it
         is present and f0.
      */
      if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) decrementBy = 1;
      else decrementBy = f0 + 1;

      for(idx = 0; idx < nvalues; idx++)
      {
         if ( values[idx] == 0 ) pZero = values + idx;
         if ( values[idx] == f0 ) pF0 = values + idx;

         if ( values[idx] > decrementBy ) values[idx] -= decrementBy;
         else {
            values[idx] = mod( values[idx] - decrementBy, 1024);
         }
      }

      /* for 1024, remove 0 from list (if present); for all others,
         remove F0 */
      if ( pZero || format != OS3GPP_FREQ_LIST_FMT_RANGE_1024 )
      {
         /* setting pF0 just makes it easier to remove in all cases */
         if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) pF0 = pZero;

         if ( pF0 == values + nvalues - 1 ) {
            /* removal of last value */
            nvalues--;
         }
         else {
            /* replace pF0 with last value and truncate list */
            *pF0 = values[nvalues - 1];
            nvalues--;
         }
      }

      /* Determine # of octets required in buffer and ensure we have them.
         Think of each group of k-bit values as a bucket.  The first n buckets
         combined hold (2^n - 1) values total.
         Let n be the number of buckets that will be completely filled.
         Let k be the number of values in the last bucket (whose values are
            bitsPerValue-n bits).
         Let bits be the total number of bits used to encode F0/orig_arfcn on.
       */
      n = int_log2( nvalues + 1);
      k = nvalues - (1 << n) + 1;

      bits = pFormat->leadingBits + pFormat->total_bits[n] +
                                                k * (pFormat->bitsPerValue-n);

      /* The first startBit bits will go into a part of an octet, the rest will
      be padded to a mulitple of 8.  To hold the entire encoding with padding
      we'll need 1 byte for the startBits (assuming != 8, which it never is),
      then however many full bytes it takes to hold bits - startBit bits.
      */
      reqdBytes = 1 + (bits - pFormat->startBit + 7) / 8;
      *pNumbits = reqdBytes * 8 - (8- pFormat->startBit);

      if ( *pBuffer == 0 ) {
         /* allocate buffer */
         *pBuffer = rtxMemAllocZ(pctxt, reqdBytes);
         if ( *pBuffer == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
      }
      else {
         if ( bufferSize < reqdBytes )
            return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
         else {
            /* make sure the last byte we'll touch is zeroed. */
            (*pBuffer)[reqdBytes -1] = 0;
         }
      }

      /* encode the leading field before the W values (i.e. F0/orig_arfcn). */
      if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) {
         /* Set f0 field in first bit: 1 if 0 was present and 0 otherwise. */
         *pBuffer[0] = pZero ? 0x80 : 0x00;
      }
      else {
         /* Set orig_arfcn field in first 10 bits to F0 */
         *pBuffer[0] = f0 >> 2;
         *pBuffer[1] = f0 << 6;
      }

      result = encode_subtree(pctxt, 1 /*idx*/, values, nvalues,
         pFormat->range /*range*/,
         *pBuffer, reqdBytes, pFormat->leadingBits /*offset*/, 1 /*nodes*/,
         pFormat->bitsPerValue /*bitsPerValue*/);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);

   }

   LCHECKNAS (pctxt);

   return result;
}



EXTRTMETHOD int rtx3GPPDecARFCN(OSCTXT* pctxt, OS3GPP_FREQ_LIST_FMT format,
               const OSOCTET* data, OSUINT16 numBits, OSUINT16** pArfcns,
               OSUINT16* pArfcnCount)
{
   OSUINT16* arfcns;       /* array of ARFCN values; *pArfcns == arfcns */

   if ( format == OS3GPP_FREQ_LIST_FMT_BITMAP0 ) {
      OSUINT8 mask = 0x80; /* mask to pick out next bit */

      if ( numBits != 124 ) {
         rtxErrAddStrParm(pctxt, "bitmap0 has other than 124 bits");
         rtxErrAddUIntParm(pctxt, numBits);
         return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      /* allocate array to hold maximum # of ARFCNs */
      *pArfcns = arfcns = rtxMemAllocArrayZ(pctxt, 124, OSUINT16);
      *pArfcnCount = 0;
      while (numBits > 0 ) {
         /* check the next bit using mask */
         if ( *data & mask ) {
            /* bit is set */
            *arfcns = numBits;   /*bits encode ARFCN 124, 123, ..., 1 */
            arfcns++;
            (*pArfcnCount)++;
         }

         numBits--;

         if ( mask == 1 ) {
            /* time to move to next byte */
            data++;
            mask = 0x80;
         }
         else mask = mask >> 1;
      }
      /* we could reallocate ARFCNS array to shrink it, but is it worth it? */
   }
   else if ( format == OS3GPP_FREQ_LIST_FMT_VAR_BITMAP ) {
      OSUINT8 mask;        /* mask to pick out next bit */
      OSUINT16 n;          /* next ARFCN will be orig_arfcn + n (mod 1024) */
      OSUINT16* pNextArfcn;   /* pointer to next ARFCN to set */

      if ( numBits < 10 ) {
         rtxErrAddStrParm(pctxt, "variable bitmap has less than 10 bits");
         rtxErrAddUIntParm(pctxt, numBits);
         return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
      }

      /* allocate array to hold maximum # of ARFCNs, which is
         1 + numbits - 10 */
      *pArfcns = arfcns = rtxMemAllocArrayZ(pctxt, numBits - 9, OSUINT16);
      *pArfcnCount = 1;

      /* first 10 bits is orig_arfcn */
      arfcns[0] = data[0] << 2 | data[1] >> 6;
      data++;

      pNextArfcn = arfcns + 1;
      mask = 0x20;   /* bits 8,7 already used for orig_arfcn */

      for(n = 1; n <= numBits - 10; n++) {
         /* check the next bit using mask */
         if ( *data & mask ) {
            /* bit is set */
            *pNextArfcn = (arfcns[0] + n ) % 1024;
            pNextArfcn++;
            (*pArfcnCount)++;
         }

         if ( mask == 1 ) {
            /* time to move to next byte */
            data++;
            mask = 0x80;
         }
         else mask = mask >> 1;
      }
      /* we could reallocate ARFCNS array to shrink it, but is it worth it? */
   }
   else {
      const FreqListFormat* pFormat = formatInfo(format);
      OSUINT16 wbits = numBits - pFormat->leadingBits;  /* bits for W values */
      OSUINT8 fullBuckets;
      OSBOOL bZero = FALSE;   /* true if 1024 format and zero was present */
      OSUINT16 origArfcn;
      OSUINT8 remInByte;      /* bits remaining in current byte */
      OSUINT8 bitsPerValue;   /* bits per W value in current bucket */
      OSUINT16* wvalue;       /* target W value in pArfcns */
      OSUINT8 bucketSize;     /* # of W values current bucket can hold */
      OSUINT8 valuesInBucket; /* # values already put into current bucket */
      OSUINT16 wValueCount;   /* # of non-zero W values */
      OSUINT16 k;             /* loop var for W to ARFCN translation */

      /* determine how many buckets wbits would fill */
      for(fullBuckets = pFormat->bitsPerValue; fullBuckets > 0; fullBuckets--)
      {
         if ( pFormat->total_bits[fullBuckets] <= wbits ) break;
      }

      *pArfcnCount = 0;

      if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) {
         if ( numBits < 1 ) {
            rtxErrAddStrParm(pctxt, "format 1024 has less than 1 bit");
            rtxErrAddUIntParm(pctxt, numBits);
            return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
         }

         /* check if F0 is set */
         if ( data[0] & 0x80 ) {
            bZero = TRUE;
            (*pArfcnCount)++;
         }
         remInByte = 7;
         numBits--;
      }
      else {
         if ( numBits < 10 ) {
            rtxErrAddStrParm(pctxt, "format 512/256/128 has less than 10 bits");
            rtxErrAddUIntParm(pctxt, numBits);
            return LOG_RTERRNEW(pctxt, RTERR_BADVALUE);
         }

         /* orig_arfcn is 8 bits of first byte and high 2 bits of second byte */
         origArfcn = (data[0] << 2) | data[1] >> 6;
         (*pArfcnCount)++;
         remInByte = 6;
         data++;
         numBits -= 10;
      }

      /* We can now determine how many values we have. In addition to ARFCN 0
         (if present) or orig_arfcn, there are:
            2^fullBuckets -1 values in the full buckets and
               (left over bits) / (bits per value for last bucket)
            values in the last bucket.
      */
      *pArfcnCount += (1 << fullBuckets) - 1;
      if ( pFormat->bitsPerValue > fullBuckets ) {
         /* not all buckets are filled */
         *pArfcnCount += (wbits - pFormat->total_bits[fullBuckets]) /
            (pFormat->bitsPerValue - fullBuckets);
      }

      /* create array for ARFCN values.*/
      *pArfcns = arfcns = rtxMemAllocArrayZ(pctxt, *pArfcnCount, OSUINT16);
      if ( arfcns == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);

      /* decode the W values into the ARFCN array.  We'll then convert them to
      ARFCN values in place. */
      bitsPerValue = pFormat->bitsPerValue;
      wvalue = arfcns;
      bucketSize = 1;
      valuesInBucket = 0;

      while ( numBits >= bitsPerValue ) {
         /* read bitsPerValue bits from data as the next W value */
         /* by initialization, wvalue will be 0 to start */
         OSUINT8 needBits = bitsPerValue;

         /* this assertion will only fail if we have a bug in our code and have
            determined # of ARFCN values incorrectly. */
         OSRTASSERT( wvalue <  arfcns + *pArfcnCount );

         for(;;) {
            OSUINT8 useBits =
               needBits > remInByte ? remInByte : needBits;
            /*
            We're going to set the lowest useBits in wvalue from data.
            The first 8 - remInByte bits in data are going to be used.
            The remaining remInByte bits need to be shifted into the lowest
            bits.  We therefore right shift remInByte - useBits.
            Finally, we mask out the high 8 - useBits bits in the shifted
            result as they are not being used.
            */
            OSUINT8 mask = 0xFF >> (8 - useBits);  /*mask out high unused bits*/
            OSUINT8 shift = remInByte - useBits;

            *wvalue = (*wvalue << useBits) | ( mask & (*data >> shift ) );

            if ( useBits == remInByte ) {
               data++;
               remInByte = 8;
            }
            else remInByte = shift; /* shift = remInByte - useBits */
            needBits -= useBits;
            if ( needBits == 0 ) break;
         }

         if ( *wvalue == 0 ) break; /* any remaining W must be 0 also */

         wvalue++;
         valuesInBucket++;
         numBits -= bitsPerValue;

         if ( valuesInBucket >= bucketSize ) {
            /* move to next bucket */
            bucketSize *= 2;
            valuesInBucket = 0;
            bitsPerValue--;
         }
      }

      /* wvalues either points at a final W value that was 0 or else points
         just past the last W value.
         There can't be more than about 1024 ARFCN or W values, so the
         cast to OSUINT16 is safe.
       */
      wValueCount = (OSUINT16) (wvalue - arfcns);

      /* convert the W values into ARFCN values. We can do this in place
         by running over the W values in reverse order.  Note that where
         the algorithm uses W[idx] or W[k], we use W[idx-1] or W[k-1]
         because W1 is at arfcns[0].
      */
      for (k = wValueCount; k >= 1; k--)
      {
         OSUINT16 idx = k;
         OSUINT16 greatestPow2 = greatestPow2LtEqTo(idx);
         OSUINT16 range = pFormat->range / greatestPow2;
         OSUINT16 n = arfcns[idx-1] - 1;

         while (idx > 1) {
            range = 2* range + 1;
            if ( 2*idx < 3* greatestPow2 ) {
               /* this is a left child */
               idx = idx - greatestPow2 / 2;
               n = (n + arfcns[idx-1] + (range-1)/2 ) % range;
            }
            else {
               /* this is a right child */
               idx = idx - greatestPow2;
               n = (n + arfcns[idx-1] ) % range;
            }
            greatestPow2 /= 2;
         }

         if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) {
            arfcns[k-1] = ( n + 1 ) % 1024;
         }
         else {
            arfcns[k-1] = ( n + origArfcn + 1 ) % 1024;
         }
      }

      if ( format == OS3GPP_FREQ_LIST_FMT_RANGE_1024 ) {
         /* Add zero into list, if it was indicated as present.
            Adjust ARFCN count based on number of non-zero W values and
            prsence of ARFCN 0. */
         if (bZero) {
            *pArfcnCount = wValueCount + 1;
            arfcns[*pArfcnCount-1] = 0;
         }
         else *pArfcnCount = wValueCount;
      }
      else {
         /* Add orig_arfcn into list.  Adjust ARFCN count based on number of
            non-zero W values plus the orig_arfcn. */
         *pArfcnCount = wValueCount + 1;
         arfcns[*pArfcnCount-1] = origArfcn;
      }
   }

   LCHECKNAS (pctxt);

   return 0;
}
