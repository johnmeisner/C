/*
The software in this module is open source software that has been
derived from various sources.  All applicable copyright notices are
contained within.

NOTE: the copyright notice below applies only to source and header files
that include this notice.  It does not apply to other Objective Systems
software with different attached notices.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is fur-
nished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
DANIEL VEILLARD BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUnicode.h"

/* Convert Unicode text string to C-string. */

EXTRTMETHOD char* rtxUCSToCStr (OSCTXT* pctxt, const OSUNICHAR* text)
{
   size_t length, i;
   char*  resStr;

   if (0 == text) return 0;
   length = rtxUCSStrLen (text);
   resStr = rtxMemAllocArray (pctxt, length + 1, char);
   if (resStr == 0) return 0;

   for (i = 0; i <= length; i++) {
      resStr[i] = (char)text[i];
   }

   return resStr;
}

/* Compare Unicode text strings */

EXTRTMETHOD OSBOOL rtxUCSStrCmp (const OSUNICHAR* text1, const OSUNICHAR* text2)
{
   if (0 != text1 && 0 != text2) {
      for (; *text1 != 0 && *text2 != 0; text1++, text2++) {
         if (*text1 != *text2)
            return FALSE;
      }
      return (OSBOOL)(*text1 == *text2);
   }
   return FALSE;
}

/* Compare Unicode text string to ASCII text string */

EXTRTMETHOD OSBOOL rtxUCSStrCmpAsc (const OSUNICHAR* text1, const char* text2)
{
   if (0 != text1 && 0 != text2) {
      for (; *text1 != 0 && *text2 != 0; text1++, text2++) {
         if (*text1 != *text2)
            return FALSE;
      }
      return (OSBOOL)(*text1 == *text2);
   }
   return FALSE;
}

/* Get Unicode string length in characters */

EXTRTMETHOD size_t rtxUCSStrLen (const OSUNICHAR* text)
{
   size_t length = 0;

   if (0 == text) return 0;

   for (; *text != 0; text++) {
      length ++;
   }

   return length;
}

/* Add a Unicode character string parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddUniStrParm
(OSCTXT* pctxt, const OSUNICHAR* pErrParm)
{
   OSRTErrInfo* pErrInfo = &pctxt->errInfo.reserved;
   if (pErrInfo != 0 && pErrInfo->parmcnt < OSRTMAXERRPRM) {
      pErrInfo->parms[pErrInfo->parmcnt] = (OSUTF8CHAR*)
         ((0 != pErrParm) ? rtxUCSToCStr (pctxt, pErrParm) : "");
      pErrInfo->parmcnt++;
      return TRUE;
   }
   else
      return FALSE;
}

/* Print Unicode string to standard output */
#if !defined(_ARMTCC)
EXTRTMETHOD void rtxUCSStrPrint (const OSUNICHAR* text)
{
   rtxUCSStrPrintFile (stdout, text);
}

EXTRTMETHOD void rtxDiagPrintUCS (OSCTXT* pctxt, const OSUNICHAR* chars)
{
   if (rtxDiagEnabled (pctxt)) {
      rtxUCSStrPrint (chars);
      printf ("\n");
   }
}
#endif

/* Print Unicode string to file */

EXTRTMETHOD void rtxUCSStrPrintFile (FILE* fp, const OSUNICHAR* text)
{
   size_t length, i;

   if (0 == text) return;
   length = rtxUCSStrLen (text);

   for (i = 0; i < length; i++) {
      if (OS_ISASCII(text[i]))
         fprintf (fp, "%c", text[i]);
      else
         fprintf (fp, "\\0x%x", text[i]);
   }
}

/*
 *
 *  Routine name: rtxUCSToUTF8
 *
 *  Description:  This routine converts a Unicode string
 *                into a UTF-8 encoded byte stream.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  pctxt       struct	        Pointer to context block structure
 *  inbuf       OSUNICHAR*      Unicode string to convert
 *  inlen       size_t          Number of Unicode chars in inbuf
 *  outbufsiz   int             Number of bytes the output buffer can hold.
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  stat        int             Byte count or error status.
 *  outbuf      OSOCTET*        Output buffer to hold UTF-8 encoded data.
 *
 */
#if !defined(_ARMTCC) && !defined(__SYMBIAN32__)
static const OSUINT32 encoding_mask[] = {
   0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
};
static const unsigned char encoding_byte[] = {
   0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};
#endif
/* Get number of bytes required for conversion of a Unicode string      */
/* to UTF-8                                                             */

EXTRTMETHOD int rtxUCSGetUTF8ByteCnt (const OSUNICHAR* inbuf)
{
   size_t numBytes = 0;
   size_t inbufx, step;

   for (inbufx = 0; inbuf[inbufx] != 0; inbufx++) {
      if (inbuf[inbufx] < 0x80) {
         /* One byte sequence */
         numBytes++;
      }
      else {
#if defined(_ARMTCC) || defined(__SYMBIAN32__)
         const OSUINT32 encoding_mask[] = {
            0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
         };
#endif
         for (step = 2; step < 6; ++step) {
            if ((inbuf[inbufx] & encoding_mask[step - 2]) == 0)
               break;
         }
         numBytes += step;
      }
   }

   return (int)numBytes;
}

EXTRTMETHOD long rtxUCSToUTF8
(OSCTXT* pctxt, const OSUNICHAR* inbuf, size_t inlen,
 OSOCTET* outbuf, size_t outbufsiz)
{
   size_t inbufx, outbufx = 0;
   size_t step, start;
   OSUNICHAR wc;

   for (inbufx = 0; inbufx < inlen; inbufx++) {
      wc = inbuf[inbufx];

      if (wc < 0x80) {
         /* One byte sequence */
         if (outbufx >= outbufsiz)
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
         else
            outbuf[outbufx++] = (char) wc;
      }
      else {
#if defined(_ARMTCC) || defined(__SYMBIAN32__)
         const OSUINT32 encoding_mask[] = {
            0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
         };
         const unsigned char encoding_byte[] = {
            0xc0, 0xe0, 0xf0, 0xf8, 0xfc
         };
#endif
         for (step = 2; step < 6; ++step)
            if ((wc & encoding_mask[step - 2]) == 0)
               break;

         if (outbufx + step >= outbufsiz)
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);

         /* Copy multi-byte sequence to output buffer */

         start = outbufx;
         outbufx += step;
         outbuf[start] = encoding_byte[step - 2];
         --step;
         do {
            outbuf[start + step] = (OSOCTET) (0x80 | (wc & 0x3f));
            wc >>= 6;
         }
         while (--step > 0);
         outbuf[start] |= wc;
      }
   }

   /* add terminating zero */
   if (outbufx < outbufsiz)
      outbuf[outbufx] = 0;
   else
      return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);

   return (long)(outbufx);
}

const OSUTF8CHAR* rtxUCSToDynUTF8 (OSCTXT* pctxt, const OSUNICHAR* inbuf)
{
   size_t inbufx, outbufx = 0;
   size_t step, start;
   int numBytes = rtxUCSGetUTF8ByteCnt (inbuf);
   OSUTF8CHAR* outbuf = rtxMemAllocArray (pctxt, numBytes+1, OSUTF8CHAR);
   OSUNICHAR wc;

   if (outbuf == 0) return 0;

   for (inbufx = 0; inbuf[inbufx] != 0; inbufx++) {
      wc = inbuf[inbufx];

      /* not valid for Unicode case; only UCS4 (ED, 2/25/03)
      if (wc >> 31) { * equivavlent of 'wc < 0 || wc > 0x7fffffff' *
         * This is not a correct ISO 10646 character *
#ifndef _COMPACT
         rtxErrAddUIntParm (pctxt, inbufx);
#endif
         return LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
      }
      */

      if (wc < 0x80) {
         /* One byte sequence */
         outbuf[outbufx++] = (OSUTF8CHAR) wc;
      }
      else {
#if defined(_ARMTCC) || defined(__SYMBIAN32__)
         const OSUINT32 encoding_mask[] = {
            0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
         };
         const unsigned char encoding_byte[] = {
            0xc0, 0xe0, 0xf0, 0xf8, 0xfc
         };
#endif
         for (step = 2; step < 6; ++step)
            if ((wc & encoding_mask[step - 2]) == 0)
               break;

         /* Copy multi-byte sequence to output buffer */

         start = outbufx;
         outbufx += step;
         outbuf[start] = encoding_byte[step - 2];
         --step;
         do {
            outbuf[start + step] = (OSUTF8CHAR) (0x80 | (wc & 0x3f));
            wc >>= 6;
         }
         while (--step > 0);
         outbuf[start] |= wc;
      }
   }

   outbuf[outbufx] = 0;

   return (outbuf);
}

/*

The following functions were derived from the GNOME libxml2 project.

NOTE: the copyright notice below applies only to source and header files
that include this notice.  It does not apply to other Objective Systems
software with different attached notices.

Except where otherwise noted in the source code (trio files, hash.c and list.c)
covered by a similar licence but with different Copyright notices:

 Copyright (C) 1998-2023 Daniel Veillard.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is fur-
nished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
DANIEL VEILLARD BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Daniel Veillard shall not
be used in advertising or otherwise to promote the sale, use or other deal-
ings in this Software without prior written authorization from him.
*/
/*
 * Changes made to original libxml source code by Objective Systems, Inc
 * are as follows:
 *
 * 1. Changed to fit Objective Systems run-time environment including
 * common type name changes and use of OSys mem mgmt.
 *
 * 2. Name change from xmlIs<something> to rtxUCSIs<something> to
 * reflect fact that code will be used in other non-XML environments.
 */
/*
 * rtxUCSIsChar:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
 *                  | [#x10000-#x10FFFF]
 * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 * Also available as a macro IS_CHAR()
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsChar (OS32BITCHAR c)
{
   return (OSBOOL)(
     ((c) == 0x09) || ((c) == 0x0A) || ((c) == 0x0D) ||
     (((c) >= 0x20) && ((c) <= 0xD7FF)) ||
     (((c) >= 0xE000) && ((c) <= 0xFFFD)) ||
     (((c) >= 0x10000) && ((c) <= 0x10FFFF)));
}

/*
 * rtxUCSIsBlank:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [3] S ::= (#x20 | #x9 | #xD | #xA)+
 * Also available as a macro IS_BLANK()
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBlank (OS32BITCHAR c)
{
   return (OSBOOL)(((c) == 0x20) || ((c) == 0x09) || ((c) == 0xA) || ((c) == 0x0D));
}

/* This function was introduced to eliminate dependence upon a writable static
 * array of the same name. */
static int xmlBaseArray(OS32BITCHAR c) {
   if (c <= 0x0040 ||
       (0x005B <= c && c <= 0x0060) ||
       (0x007B <= c && c <= 0x00BF) ||
       c == 0x00D7 || c == 0x00F7)
      return 0;
   else if ((0x0041 <= c && c <= 0x005A) ||
            (0x0061 <= c && c <= 0x007A) ||
            (0x00C0 <= c && c <= 0x00D6) ||
            (0x00D8 <= c && c <= 0x00F6) ||
            (0x00F8 <= c && c <= 0x00FF))
      return 1;
   else
      return -1;
}

/*
 * rtxUCSIsBaseChar:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [85] BaseChar ::= ... long list see REC ...
 *
 * VI is your friend !
 * :1,$ s/\[#x\([0-9A-Z]*\)-#x\([0-9A-Z]*\)\]/     (((c) >= 0x\1) \&\& ((c) <= 0x\2)) ||/
 * and
 * :1,$ s/#x\([0-9A-Z]*\)/     ((c) == 0x\1) ||/
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBaseChar (OS32BITCHAR c)
{
   return (OSBOOL)(
      (((c) < 0x0100) ? xmlBaseArray(c) :
      (	/* accelerator */
      (((c) >= 0x0100) && ((c) <= 0x0131)) ||
      (((c) >= 0x0134) && ((c) <= 0x013E)) ||
      (((c) >= 0x0141) && ((c) <= 0x0148)) ||
      (((c) >= 0x014A) && ((c) <= 0x017E)) ||
      (((c) >= 0x0180) && ((c) <= 0x01C3)) ||
      (((c) >= 0x01CD) && ((c) <= 0x01F0)) ||
      (((c) >= 0x01F4) && ((c) <= 0x01F5)) ||
      (((c) >= 0x01FA) && ((c) <= 0x0217)) ||
      (((c) >= 0x0250) && ((c) <= 0x02A8)) ||
      (((c) >= 0x02BB) && ((c) <= 0x02C1)) ||
      ((c) == 0x0386) ||
      (((c) >= 0x0388) && ((c) <= 0x038A)) ||
      ((c) == 0x038C) ||
      (((c) >= 0x038E) && ((c) <= 0x03A1)) ||
      (((c) >= 0x03A3) && ((c) <= 0x03CE)) ||
      (((c) >= 0x03D0) && ((c) <= 0x03D6)) ||
      ((c) == 0x03DA) ||
      ((c) == 0x03DC) ||
      ((c) == 0x03DE) ||
      ((c) == 0x03E0) ||
      (((c) >= 0x03E2) && ((c) <= 0x03F3)) ||
      (((c) >= 0x0401) && ((c) <= 0x040C)) ||
      (((c) >= 0x040E) && ((c) <= 0x044F)) ||
      (((c) >= 0x0451) && ((c) <= 0x045C)) ||
      (((c) >= 0x045E) && ((c) <= 0x0481)) ||
      (((c) >= 0x0490) && ((c) <= 0x04C4)) ||
      (((c) >= 0x04C7) && ((c) <= 0x04C8)) ||
      (((c) >= 0x04CB) && ((c) <= 0x04CC)) ||
      (((c) >= 0x04D0) && ((c) <= 0x04EB)) ||
      (((c) >= 0x04EE) && ((c) <= 0x04F5)) ||
      (((c) >= 0x04F8) && ((c) <= 0x04F9)) ||
      (((c) >= 0x0531) && ((c) <= 0x0556)) ||
      ((c) == 0x0559) ||
      (((c) >= 0x0561) && ((c) <= 0x0586)) ||
      (((c) >= 0x05D0) && ((c) <= 0x05EA)) ||
      (((c) >= 0x05F0) && ((c) <= 0x05F2)) ||
      (((c) >= 0x0621) && ((c) <= 0x063A)) ||
      (((c) >= 0x0641) && ((c) <= 0x064A)) ||
      (((c) >= 0x0671) && ((c) <= 0x06B7)) ||
      (((c) >= 0x06BA) && ((c) <= 0x06BE)) ||
      (((c) >= 0x06C0) && ((c) <= 0x06CE)) ||
      (((c) >= 0x06D0) && ((c) <= 0x06D3)) ||
      ((c) == 0x06D5) ||
      (((c) >= 0x06E5) && ((c) <= 0x06E6)) ||
     (((c) >= 0x905) && (	/* accelerator */
      (((c) >= 0x0905) && ((c) <= 0x0939)) ||
      ((c) == 0x093D) ||
      (((c) >= 0x0958) && ((c) <= 0x0961)) ||
      (((c) >= 0x0985) && ((c) <= 0x098C)) ||
      (((c) >= 0x098F) && ((c) <= 0x0990)) ||
      (((c) >= 0x0993) && ((c) <= 0x09A8)) ||
      (((c) >= 0x09AA) && ((c) <= 0x09B0)) ||
      ((c) == 0x09B2) ||
      (((c) >= 0x09B6) && ((c) <= 0x09B9)) ||
      (((c) >= 0x09DC) && ((c) <= 0x09DD)) ||
      (((c) >= 0x09DF) && ((c) <= 0x09E1)) ||
      (((c) >= 0x09F0) && ((c) <= 0x09F1)) ||
      (((c) >= 0x0A05) && ((c) <= 0x0A0A)) ||
      (((c) >= 0x0A0F) && ((c) <= 0x0A10)) ||
      (((c) >= 0x0A13) && ((c) <= 0x0A28)) ||
      (((c) >= 0x0A2A) && ((c) <= 0x0A30)) ||
      (((c) >= 0x0A32) && ((c) <= 0x0A33)) ||
      (((c) >= 0x0A35) && ((c) <= 0x0A36)) ||
      (((c) >= 0x0A38) && ((c) <= 0x0A39)) ||
      (((c) >= 0x0A59) && ((c) <= 0x0A5C)) ||
      ((c) == 0x0A5E) ||
      (((c) >= 0x0A72) && ((c) <= 0x0A74)) ||
      (((c) >= 0x0A85) && ((c) <= 0x0A8B)) ||
      ((c) == 0x0A8D) ||
      (((c) >= 0x0A8F) && ((c) <= 0x0A91)) ||
      (((c) >= 0x0A93) && ((c) <= 0x0AA8)) ||
      (((c) >= 0x0AAA) && ((c) <= 0x0AB0)) ||
      (((c) >= 0x0AB2) && ((c) <= 0x0AB3)) ||
      (((c) >= 0x0AB5) && ((c) <= 0x0AB9)) ||
      ((c) == 0x0ABD) ||
      ((c) == 0x0AE0) ||
      (((c) >= 0x0B05) && ((c) <= 0x0B0C)) ||
      (((c) >= 0x0B0F) && ((c) <= 0x0B10)) ||
      (((c) >= 0x0B13) && ((c) <= 0x0B28)) ||
      (((c) >= 0x0B2A) && ((c) <= 0x0B30)) ||
      (((c) >= 0x0B32) && ((c) <= 0x0B33)) ||
      (((c) >= 0x0B36) && ((c) <= 0x0B39)) ||
      ((c) == 0x0B3D) ||
      (((c) >= 0x0B5C) && ((c) <= 0x0B5D)) ||
      (((c) >= 0x0B5F) && ((c) <= 0x0B61)) ||
      (((c) >= 0x0B85) && ((c) <= 0x0B8A)) ||
      (((c) >= 0x0B8E) && ((c) <= 0x0B90)) ||
      (((c) >= 0x0B92) && ((c) <= 0x0B95)) ||
      (((c) >= 0x0B99) && ((c) <= 0x0B9A)) ||
      ((c) == 0x0B9C) ||
      (((c) >= 0x0B9E) && ((c) <= 0x0B9F)) ||
      (((c) >= 0x0BA3) && ((c) <= 0x0BA4)) ||
      (((c) >= 0x0BA8) && ((c) <= 0x0BAA)) ||
      (((c) >= 0x0BAE) && ((c) <= 0x0BB5)) ||
      (((c) >= 0x0BB7) && ((c) <= 0x0BB9)) ||
      (((c) >= 0x0C05) && ((c) <= 0x0C0C)) ||
      (((c) >= 0x0C0E) && ((c) <= 0x0C10)) ||
      (((c) >= 0x0C12) && ((c) <= 0x0C28)) ||
      (((c) >= 0x0C2A) && ((c) <= 0x0C33)) ||
      (((c) >= 0x0C35) && ((c) <= 0x0C39)) ||
      (((c) >= 0x0C60) && ((c) <= 0x0C61)) ||
      (((c) >= 0x0C85) && ((c) <= 0x0C8C)) ||
      (((c) >= 0x0C8E) && ((c) <= 0x0C90)) ||
      (((c) >= 0x0C92) && ((c) <= 0x0CA8)) ||
      (((c) >= 0x0CAA) && ((c) <= 0x0CB3)) ||
      (((c) >= 0x0CB5) && ((c) <= 0x0CB9)) ||
      ((c) == 0x0CDE) ||
      (((c) >= 0x0CE0) && ((c) <= 0x0CE1)) ||
      (((c) >= 0x0D05) && ((c) <= 0x0D0C)) ||
      (((c) >= 0x0D0E) && ((c) <= 0x0D10)) ||
      (((c) >= 0x0D12) && ((c) <= 0x0D28)) ||
      (((c) >= 0x0D2A) && ((c) <= 0x0D39)) ||
      (((c) >= 0x0D60) && ((c) <= 0x0D61)) ||
      (((c) >= 0x0E01) && ((c) <= 0x0E2E)) ||
      ((c) == 0x0E30) ||
      (((c) >= 0x0E32) && ((c) <= 0x0E33)) ||
      (((c) >= 0x0E40) && ((c) <= 0x0E45)) ||
      (((c) >= 0x0E81) && ((c) <= 0x0E82)) ||
      ((c) == 0x0E84) ||
      (((c) >= 0x0E87) && ((c) <= 0x0E88)) ||
      ((c) == 0x0E8A) ||
      ((c) == 0x0E8D) ||
      (((c) >= 0x0E94) && ((c) <= 0x0E97)) ||
      (((c) >= 0x0E99) && ((c) <= 0x0E9F)) ||
      (((c) >= 0x0EA1) && ((c) <= 0x0EA3)) ||
      ((c) == 0x0EA5) ||
      ((c) == 0x0EA7) ||
      (((c) >= 0x0EAA) && ((c) <= 0x0EAB)) ||
      (((c) >= 0x0EAD) && ((c) <= 0x0EAE)) ||
      ((c) == 0x0EB0) ||
      (((c) >= 0x0EB2) && ((c) <= 0x0EB3)) ||
      ((c) == 0x0EBD) ||
      (((c) >= 0x0EC0) && ((c) <= 0x0EC4)) ||
      (((c) >= 0x0F40) && ((c) <= 0x0F47)) ||
      (((c) >= 0x0F49) && ((c) <= 0x0F69)) ||
     (((c) >= 0x10A0) && (	/* accelerator */
      (((c) >= 0x10A0) && ((c) <= 0x10C5)) ||
      (((c) >= 0x10D0) && ((c) <= 0x10F6)) ||
      ((c) == 0x1100) ||
      (((c) >= 0x1102) && ((c) <= 0x1103)) ||
      (((c) >= 0x1105) && ((c) <= 0x1107)) ||
      ((c) == 0x1109) ||
      (((c) >= 0x110B) && ((c) <= 0x110C)) ||
      (((c) >= 0x110E) && ((c) <= 0x1112)) ||
      ((c) == 0x113C) ||
      ((c) == 0x113E) ||
      ((c) == 0x1140) ||
      ((c) == 0x114C) ||
      ((c) == 0x114E) ||
      ((c) == 0x1150) ||
      (((c) >= 0x1154) && ((c) <= 0x1155)) ||
      ((c) == 0x1159) ||
      (((c) >= 0x115F) && ((c) <= 0x1161)) ||
      ((c) == 0x1163) ||
      ((c) == 0x1165) ||
      ((c) == 0x1167) ||
      ((c) == 0x1169) ||
      (((c) >= 0x116D) && ((c) <= 0x116E)) ||
      (((c) >= 0x1172) && ((c) <= 0x1173)) ||
      ((c) == 0x1175) ||
      ((c) == 0x119E) ||
      ((c) == 0x11A8) ||
      ((c) == 0x11AB) ||
      (((c) >= 0x11AE) && ((c) <= 0x11AF)) ||
      (((c) >= 0x11B7) && ((c) <= 0x11B8)) ||
      ((c) == 0x11BA) ||
      (((c) >= 0x11BC) && ((c) <= 0x11C2)) ||
      ((c) == 0x11EB) ||
      ((c) == 0x11F0) ||
      ((c) == 0x11F9) ||
      (((c) >= 0x1E00) && ((c) <= 0x1E9B)) ||
      (((c) >= 0x1EA0) && ((c) <= 0x1EF9)) ||
      (((c) >= 0x1F00) && ((c) <= 0x1F15)) ||
      (((c) >= 0x1F18) && ((c) <= 0x1F1D)) ||
      (((c) >= 0x1F20) && ((c) <= 0x1F45)) ||
      (((c) >= 0x1F48) && ((c) <= 0x1F4D)) ||
      (((c) >= 0x1F50) && ((c) <= 0x1F57)) ||
      ((c) == 0x1F59) ||
      ((c) == 0x1F5B) ||
      ((c) == 0x1F5D) ||
      (((c) >= 0x1F5F) && ((c) <= 0x1F7D)) ||
      (((c) >= 0x1F80) && ((c) <= 0x1FB4)) ||
      (((c) >= 0x1FB6) && ((c) <= 0x1FBC)) ||
      ((c) == 0x1FBE) ||
      (((c) >= 0x1FC2) && ((c) <= 0x1FC4)) ||
      (((c) >= 0x1FC6) && ((c) <= 0x1FCC)) ||
      (((c) >= 0x1FD0) && ((c) <= 0x1FD3)) ||
      (((c) >= 0x1FD6) && ((c) <= 0x1FDB)) ||
      (((c) >= 0x1FE0) && ((c) <= 0x1FEC)) ||
      (((c) >= 0x1FF2) && ((c) <= 0x1FF4)) ||
      (((c) >= 0x1FF6) && ((c) <= 0x1FFC)) ||
      ((c) == 0x2126) ||
      (((c) >= 0x212A) && ((c) <= 0x212B)) ||
      ((c) == 0x212E) ||
      (((c) >= 0x2180) && ((c) <= 0x2182)) ||
      (((c) >= 0x3041) && ((c) <= 0x3094)) ||
      (((c) >= 0x30A1) && ((c) <= 0x30FA)) ||
      (((c) >= 0x3105) && ((c) <= 0x312C)) ||
      (((c) >= 0xAC00) && ((c) <= 0xD7A3))) /* accelerators */ ))))));
}

/*
 * rtxUCSIsDigit:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [88] Digit ::= ... long list see REC ...
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsDigit (OS32BITCHAR c)
{
    return (OSBOOL)(
      (((c) >= 0x0030) && ((c) <= 0x0039)) ||
     (((c) >= 0x660) && (	/* accelerator */
      (((c) >= 0x0660) && ((c) <= 0x0669)) ||
      (((c) >= 0x06F0) && ((c) <= 0x06F9)) ||
      (((c) >= 0x0966) && ((c) <= 0x096F)) ||
      (((c) >= 0x09E6) && ((c) <= 0x09EF)) ||
      (((c) >= 0x0A66) && ((c) <= 0x0A6F)) ||
      (((c) >= 0x0AE6) && ((c) <= 0x0AEF)) ||
      (((c) >= 0x0B66) && ((c) <= 0x0B6F)) ||
      (((c) >= 0x0BE7) && ((c) <= 0x0BEF)) ||
      (((c) >= 0x0C66) && ((c) <= 0x0C6F)) ||
      (((c) >= 0x0CE6) && ((c) <= 0x0CEF)) ||
      (((c) >= 0x0D66) && ((c) <= 0x0D6F)) ||
      (((c) >= 0x0E50) && ((c) <= 0x0E59)) ||
      (((c) >= 0x0ED0) && ((c) <= 0x0ED9)) ||
      (((c) >= 0x0F20) && ((c) <= 0x0F29))) /* accelerator */ ));
}

/*
 * rtxUCSIsCombining:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [87] CombiningChar ::= ... long list see REC ...
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCombining (OS32BITCHAR c)
{
   return (OSBOOL)(
     (((c) >= 0x300) && (	/* accelerator */
      (((c) >= 0x0300) && ((c) <= 0x0345)) ||
      (((c) >= 0x0360) && ((c) <= 0x0361)) ||
      (((c) >= 0x0483) && ((c) <= 0x0486)) ||
      (((c) >= 0x0591) && ((c) <= 0x05A1)) ||
      (((c) >= 0x05A3) && ((c) <= 0x05B9)) ||
      (((c) >= 0x05BB) && ((c) <= 0x05BD)) ||
      ((c) == 0x05BF) ||
      (((c) >= 0x05C1) && ((c) <= 0x05C2)) ||
      ((c) == 0x05C4) ||
      (((c) >= 0x064B) && ((c) <= 0x0652)) ||
      ((c) == 0x0670) ||
      (((c) >= 0x06D6) && ((c) <= 0x06DC)) ||
      (((c) >= 0x06DD) && ((c) <= 0x06DF)) ||
      (((c) >= 0x06E0) && ((c) <= 0x06E4)) ||
      (((c) >= 0x06E7) && ((c) <= 0x06E8)) ||
      (((c) >= 0x06EA) && ((c) <= 0x06ED)) ||
     (((c) >= 0x0901) && (	/* accelerator */
      (((c) >= 0x0901) && ((c) <= 0x0903)) ||
      ((c) == 0x093C) ||
      (((c) >= 0x093E) && ((c) <= 0x094C)) ||
      ((c) == 0x094D) ||
      (((c) >= 0x0951) && ((c) <= 0x0954)) ||
      (((c) >= 0x0962) && ((c) <= 0x0963)) ||
      (((c) >= 0x0981) && ((c) <= 0x0983)) ||
      ((c) == 0x09BC) ||
      ((c) == 0x09BE) ||
      ((c) == 0x09BF) ||
      (((c) >= 0x09C0) && ((c) <= 0x09C4)) ||
      (((c) >= 0x09C7) && ((c) <= 0x09C8)) ||
      (((c) >= 0x09CB) && ((c) <= 0x09CD)) ||
      ((c) == 0x09D7) ||
      (((c) >= 0x09E2) && ((c) <= 0x09E3)) ||
     (((c) >= 0x0A02) && (	/* accelerator */
      ((c) == 0x0A02) ||
      ((c) == 0x0A3C) ||
      ((c) == 0x0A3E) ||
      ((c) == 0x0A3F) ||
      (((c) >= 0x0A40) && ((c) <= 0x0A42)) ||
      (((c) >= 0x0A47) && ((c) <= 0x0A48)) ||
      (((c) >= 0x0A4B) && ((c) <= 0x0A4D)) ||
      (((c) >= 0x0A70) && ((c) <= 0x0A71)) ||
      (((c) >= 0x0A81) && ((c) <= 0x0A83)) ||
      ((c) == 0x0ABC) ||
      (((c) >= 0x0ABE) && ((c) <= 0x0AC5)) ||
      (((c) >= 0x0AC7) && ((c) <= 0x0AC9)) ||
      (((c) >= 0x0ACB) && ((c) <= 0x0ACD)) ||
      (((c) >= 0x0B01) && ((c) <= 0x0B03)) ||
      ((c) == 0x0B3C) ||
      (((c) >= 0x0B3E) && ((c) <= 0x0B43)) ||
      (((c) >= 0x0B47) && ((c) <= 0x0B48)) ||
      (((c) >= 0x0B4B) && ((c) <= 0x0B4D)) ||
      (((c) >= 0x0B56) && ((c) <= 0x0B57)) ||
      (((c) >= 0x0B82) && ((c) <= 0x0B83)) ||
      (((c) >= 0x0BBE) && ((c) <= 0x0BC2)) ||
      (((c) >= 0x0BC6) && ((c) <= 0x0BC8)) ||
      (((c) >= 0x0BCA) && ((c) <= 0x0BCD)) ||
      ((c) == 0x0BD7) ||
      (((c) >= 0x0C01) && ((c) <= 0x0C03)) ||
      (((c) >= 0x0C3E) && ((c) <= 0x0C44)) ||
      (((c) >= 0x0C46) && ((c) <= 0x0C48)) ||
      (((c) >= 0x0C4A) && ((c) <= 0x0C4D)) ||
      (((c) >= 0x0C55) && ((c) <= 0x0C56)) ||
      (((c) >= 0x0C82) && ((c) <= 0x0C83)) ||
      (((c) >= 0x0CBE) && ((c) <= 0x0CC4)) ||
      (((c) >= 0x0CC6) && ((c) <= 0x0CC8)) ||
      (((c) >= 0x0CCA) && ((c) <= 0x0CCD)) ||
      (((c) >= 0x0CD5) && ((c) <= 0x0CD6)) ||
      (((c) >= 0x0D02) && ((c) <= 0x0D03)) ||
      (((c) >= 0x0D3E) && ((c) <= 0x0D43)) ||
      (((c) >= 0x0D46) && ((c) <= 0x0D48)) ||
      (((c) >= 0x0D4A) && ((c) <= 0x0D4D)) ||
      ((c) == 0x0D57) ||
     (((c) >= 0x0E31) && (	/* accelerator */
      ((c) == 0x0E31) ||
      (((c) >= 0x0E34) && ((c) <= 0x0E3A)) ||
      (((c) >= 0x0E47) && ((c) <= 0x0E4E)) ||
      ((c) == 0x0EB1) ||
      (((c) >= 0x0EB4) && ((c) <= 0x0EB9)) ||
      (((c) >= 0x0EBB) && ((c) <= 0x0EBC)) ||
      (((c) >= 0x0EC8) && ((c) <= 0x0ECD)) ||
      (((c) >= 0x0F18) && ((c) <= 0x0F19)) ||
      ((c) == 0x0F35) ||
      ((c) == 0x0F37) ||
      ((c) == 0x0F39) ||
      ((c) == 0x0F3E) ||
      ((c) == 0x0F3F) ||
      (((c) >= 0x0F71) && ((c) <= 0x0F84)) ||
      (((c) >= 0x0F86) && ((c) <= 0x0F8B)) ||
      (((c) >= 0x0F90) && ((c) <= 0x0F95)) ||
      ((c) == 0x0F97) ||
      (((c) >= 0x0F99) && ((c) <= 0x0FAD)) ||
      (((c) >= 0x0FB1) && ((c) <= 0x0FB7)) ||
      ((c) == 0x0FB9) ||
      (((c) >= 0x20D0) && ((c) <= 0x20DC)) ||
      ((c) == 0x20E1) ||
      (((c) >= 0x302A) && ((c) <= 0x302F)) ||
      ((c) == 0x3099) ||
      ((c) == 0x309A))))))))));
}

/*
 * rtxUCSIsExtender:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [89] Extender ::= #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 |
 *                   #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] |
 *                   [#x309D-#x309E] | [#x30FC-#x30FE]
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsExtender (OS32BITCHAR c)
{
    switch (c) {
    case 0x00B7: case 0x02D0: case 0x02D1: case 0x0387:
    case 0x0640: case 0x0E46: case 0x0EC6: case 0x3005:
    case 0x3031: case 0x3032: case 0x3033: case 0x3034:
    case 0x3035: case 0x309D: case 0x309E: case 0x30FC:
    case 0x30FD: case 0x30FE:
	return 1;
    default:
	return 0;
    }
}

/*
 * rtxUCSIsIdeographic:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [86] Ideographic ::= [#x4E00-#x9FA5] | #x3007 | [#x3021-#x3029]
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsIdeographic (OS32BITCHAR c)
{
    return (OSBOOL)(((c) < 0x0100) ? 0 :
     (((c) >= 0x4e00) && ((c) <= 0x9fa5)) ||
     (((c) >= 0xf900) && ((c) <= 0xfa2d)) ||
     (((c) >= 0x3021) && ((c) <= 0x3029)) ||
      ((c) == 0x3007));
}

/*
 * rtxUCSIsLetter:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [84] Letter ::= BaseChar | Ideographic
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLetter (OS32BITCHAR c)
{
   return (OSBOOL)(OS_IS_BASECHAR(c) || OS_IS_IDEOGRAPHIC(c));
}

/*
 * rtxUCSIsPubidChar:
 * @c:  an unicode character (int)
 *
 * Check whether the character is allowed by the production
 * [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
 *
 * Returns 0 if not, non-zero otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsPubidChar (OS32BITCHAR c)
{
   return (OSBOOL)(
     ((c) == 0x20) || ((c) == 0x0D) || ((c) == 0x0A) ||
     (((c) >= 'a') && ((c) <= 'z')) ||
     (((c) >= 'A') && ((c) <= 'Z')) ||
     (((c) >= '0') && ((c) <= '9')) ||
     ((c) == '-') || ((c) == '\'') || ((c) == '(') || ((c) == ')') ||
     ((c) == '+') || ((c) == ',') || ((c) == '.') || ((c) == '/') ||
     ((c) == ':') || ((c) == '=') || ((c) == '?') || ((c) == ';') ||
     ((c) == '!') || ((c) == '*') || ((c) == '#') || ((c) == '@') ||
     ((c) == '$') || ((c) == '_') || ((c) == '%'));
}

/*
 * this module implements the Unicode character APIs
 *
 * This file is automatically generated from the
 * UCS description files of the Unicode Character Database
 * http://www.unicode.org/Public/3.1-Update/UnicodeCharacterDatabase-3.1.0.html
 * using the genUnicode.py Python script.
 *
 * Generation date: Tue Apr 16 17:28:05 2002
 * Sources: Blocks-4.txt UnicodeData-3.1.0.txt
 * Daniel Veillard <veillard@redhat.com>
 */
/*
 * rtxUCSIsAlphabeticPresentationForms:
 * @code: UCS code point
 *
 * Check whether the character is part of AlphabeticPresentationForms UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsAlphabeticPresentationForms (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFB00) && (code <= 0xFB4F));
}

/*
 * rtxUCSIsArabic:
 * @code: UCS code point
 *
 * Check whether the character is part of Arabic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsArabic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0600) && (code <= 0x06FF));
}

/*
 * rtxUCSIsArabicPresentationFormsA:
 * @code: UCS code point
 *
 * Check whether the character is part of ArabicPresentationForms-A UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsArabicPresentationFormsA (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFB50) && (code <= 0xFDFF));
}

/*
 * rtxUCSIsArabicPresentationFormsB:
 * @code: UCS code point
 *
 * Check whether the character is part of ArabicPresentationForms-B UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsArabicPresentationFormsB (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFE70) && (code <= 0xFEFE));
}

/*
 * rtxUCSIsArmenian:
 * @code: UCS code point
 *
 * Check whether the character is part of Armenian UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsArmenian (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0530) && (code <= 0x058F));
}

/*
 * rtxUCSIsArrows:
 * @code: UCS code point
 *
 * Check whether the character is part of Arrows UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsArrows (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2190) && (code <= 0x21FF));
}

/*
 * rtxUCSIsBasicLatin:
 * @code: UCS code point
 *
 * Check whether the character is part of BasicLatin UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBasicLatin (OS32BITCHAR code)
{
    return /*(code >= 0x0000) &&*/ (OSBOOL)(code <= 0x007F);
}

/*
 * rtxUCSIsBengali:
 * @code: UCS code point
 *
 * Check whether the character is part of Bengali UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBengali (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0980) && (code <= 0x09FF));
}

/*
 * rtxUCSIsBlockElements:
 * @code: UCS code point
 *
 * Check whether the character is part of BlockElements UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBlockElements (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2580) && (code <= 0x259F));
}

/*
 * rtxUCSIsBopomofo:
 * @code: UCS code point
 *
 * Check whether the character is part of Bopomofo UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBopomofo (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3100) && (code <= 0x312F));
}

/*
 * rtxUCSIsBopomofoExtended:
 * @code: UCS code point
 *
 * Check whether the character is part of BopomofoExtended UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBopomofoExtended (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x31A0) && (code <= 0x31BF));
}

/*
 * rtxUCSIsBoxDrawing:
 * @code: UCS code point
 *
 * Check whether the character is part of BoxDrawing UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBoxDrawing (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2500) && (code <= 0x257F));
}

/*
 * rtxUCSIsBraillePatterns:
 * @code: UCS code point
 *
 * Check whether the character is part of BraillePatterns UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsBraillePatterns (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2800) && (code <= 0x28FF));
}

/*
 * rtxUCSIsByzantineMusicalSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of ByzantineMusicalSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsByzantineMusicalSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1D000) && (code <= 0x1D0FF));
}

/*
 * rtxUCSIsCJKCompatibility:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKCompatibility UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKCompatibility (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3300) && (code <= 0x33FF));
}

/*
 * rtxUCSIsCJKCompatibilityForms:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKCompatibilityForms UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKCompatibilityForms (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFE30) && (code <= 0xFE4F));
}

/*
 * rtxUCSIsCJKCompatibilityIdeographs:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKCompatibilityIdeographs UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKCompatibilityIdeographs (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xF900) && (code <= 0xFAFF));
}

/*
 * rtxUCSIsCJKCompatibilityIdeographsSupplement:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKCompatibilityIdeographsSupplement UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKCompatibilityIdeographsSupplement (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2F800) && (code <= 0x2FA1F));
}

/*
 * rtxUCSIsCJKRadicalsSupplement:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKRadicalsSupplement UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKRadicalsSupplement (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2E80) && (code <= 0x2EFF));
}

/*
 * rtxUCSIsCJKSymbolsandPunctuation:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKSymbolsandPunctuation UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKSymbolsandPunctuation (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3000) && (code <= 0x303F));
}

/*
 * rtxUCSIsCJKUnifiedIdeographs:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKUnifiedIdeographs UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKUnifiedIdeographs (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x4E00) && (code <= 0x9FFF));
}

/*
 * rtxUCSIsCJKUnifiedIdeographsExtensionA:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKUnifiedIdeographsExtensionA UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKUnifiedIdeographsExtensionA (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3400) && (code <= 0x4DB5));
}

/*
 * rtxUCSIsCJKUnifiedIdeographsExtensionB:
 * @code: UCS code point
 *
 * Check whether the character is part of CJKUnifiedIdeographsExtensionB UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCJKUnifiedIdeographsExtensionB (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x20000) && (code <= 0x2A6D6));
}

/*
 * rtxUCSIsCherokee:
 * @code: UCS code point
 *
 * Check whether the character is part of Cherokee UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCherokee (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x13A0) && (code <= 0x13FF));
}

/*
 * rtxUCSIsCombiningDiacriticalMarks:
 * @code: UCS code point
 *
 * Check whether the character is part of CombiningDiacriticalMarks UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCombiningDiacriticalMarks (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0300) && (code <= 0x036F));
}

/*
 * rtxUCSIsCombiningHalfMarks:
 * @code: UCS code point
 *
 * Check whether the character is part of CombiningHalfMarks UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCombiningHalfMarks (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFE20) && (code <= 0xFE2F));
}

/*
 * rtxUCSIsCombiningMarksforSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of CombiningMarksforSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCombiningMarksforSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x20D0) && (code <= 0x20FF));
}

/*
 * rtxUCSIsControlPictures:
 * @code: UCS code point
 *
 * Check whether the character is part of ControlPictures UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsControlPictures (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2400) && (code <= 0x243F));
}

/*
 * rtxUCSIsCurrencySymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of CurrencySymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCurrencySymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x20A0) && (code <= 0x20CF));
}

/*
 * rtxUCSIsCyrillic:
 * @code: UCS code point
 *
 * Check whether the character is part of Cyrillic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCyrillic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0400) && (code <= 0x04FF));
}

/*
 * rtxUCSIsDeseret:
 * @code: UCS code point
 *
 * Check whether the character is part of Deseret UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsDeseret (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x10400) && (code <= 0x1044F));
}

/*
 * rtxUCSIsDevanagari:
 * @code: UCS code point
 *
 * Check whether the character is part of Devanagari UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsDevanagari (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0900) && (code <= 0x097F));
}

/*
 * rtxUCSIsDingbats:
 * @code: UCS code point
 *
 * Check whether the character is part of Dingbats UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsDingbats (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2700) && (code <= 0x27BF));
}

/*
 * rtxUCSIsEnclosedAlphanumerics:
 * @code: UCS code point
 *
 * Check whether the character is part of EnclosedAlphanumerics UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsEnclosedAlphanumerics (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2460) && (code <= 0x24FF));
}

/*
 * rtxUCSIsEnclosedCJKLettersandMonths:
 * @code: UCS code point
 *
 * Check whether the character is part of EnclosedCJKLettersandMonths UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsEnclosedCJKLettersandMonths (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3200) && (code <= 0x32FF));
}

/*
 * rtxUCSIsEthiopic:
 * @code: UCS code point
 *
 * Check whether the character is part of Ethiopic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsEthiopic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1200) && (code <= 0x137F));
}

/*
 * rtxUCSIsGeneralPunctuation:
 * @code: UCS code point
 *
 * Check whether the character is part of GeneralPunctuation UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGeneralPunctuation (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2000) && (code <= 0x206F));
}

/*
 * rtxUCSIsGeometricShapes:
 * @code: UCS code point
 *
 * Check whether the character is part of GeometricShapes UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGeometricShapes (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x25A0) && (code <= 0x25FF));
}

/*
 * rtxUCSIsGeorgian:
 * @code: UCS code point
 *
 * Check whether the character is part of Georgian UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGeorgian (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x10A0) && (code <= 0x10FF));
}

/*
 * rtxUCSIsGothic:
 * @code: UCS code point
 *
 * Check whether the character is part of Gothic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGothic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x10330) && (code <= 0x1034F));
}

/*
 * rtxUCSIsGreek:
 * @code: UCS code point
 *
 * Check whether the character is part of Greek UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGreek (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0370) && (code <= 0x03FF));
}

/*
 * rtxUCSIsGreekExtended:
 * @code: UCS code point
 *
 * Check whether the character is part of GreekExtended UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGreekExtended (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1F00) && (code <= 0x1FFF));
}

/*
 * rtxUCSIsGujarati:
 * @code: UCS code point
 *
 * Check whether the character is part of Gujarati UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGujarati (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0A80) && (code <= 0x0AFF));
}

/*
 * rtxUCSIsGurmukhi:
 * @code: UCS code point
 *
 * Check whether the character is part of Gurmukhi UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsGurmukhi (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0A00) && (code <= 0x0A7F));
}

/*
 * rtxUCSIsHalfwidthandFullwidthForms:
 * @code: UCS code point
 *
 * Check whether the character is part of HalfwidthandFullwidthForms UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHalfwidthandFullwidthForms (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFF00) && (code <= 0xFFEF));
}

/*
 * rtxUCSIsHangulCompatibilityJamo:
 * @code: UCS code point
 *
 * Check whether the character is part of HangulCompatibilityJamo UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHangulCompatibilityJamo (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3130) && (code <= 0x318F));
}

/*
 * rtxUCSIsHangulJamo:
 * @code: UCS code point
 *
 * Check whether the character is part of HangulJamo UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHangulJamo (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1100) && (code <= 0x11FF));
}

/*
 * rtxUCSIsHangulSyllables:
 * @code: UCS code point
 *
 * Check whether the character is part of HangulSyllables UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHangulSyllables (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xAC00) && (code <= 0xD7A3));
}

/*
 * rtxUCSIsHebrew:
 * @code: UCS code point
 *
 * Check whether the character is part of Hebrew UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHebrew (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0590) && (code <= 0x05FF));
}

/*
 * rtxUCSIsHighPrivateUseSurrogates:
 * @code: UCS code point
 *
 * Check whether the character is part of HighPrivateUseSurrogates UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHighPrivateUseSurrogates (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xDB80) && (code <= 0xDBFF));
}

/*
 * rtxUCSIsHighSurrogates:
 * @code: UCS code point
 *
 * Check whether the character is part of HighSurrogates UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHighSurrogates (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xD800) && (code <= 0xDB7F));
}

/*
 * rtxUCSIsHiragana:
 * @code: UCS code point
 *
 * Check whether the character is part of Hiragana UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsHiragana (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3040) && (code <= 0x309F));
}

/*
 * rtxUCSIsIPAExtensions:
 * @code: UCS code point
 *
 * Check whether the character is part of IPAExtensions UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsIPAExtensions (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0250) && (code <= 0x02AF));
}

/*
 * rtxUCSIsIdeographicDescriptionCharacters:
 * @code: UCS code point
 *
 * Check whether the character is part of IdeographicDescriptionCharacters UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsIdeographicDescriptionCharacters (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2FF0) && (code <= 0x2FFF));
}

/*
 * rtxUCSIsKanbun:
 * @code: UCS code point
 *
 * Check whether the character is part of Kanbun UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsKanbun (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x3190) && (code <= 0x319F));
}

/*
 * rtxUCSIsKangxiRadicals:
 * @code: UCS code point
 *
 * Check whether the character is part of KangxiRadicals UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsKangxiRadicals (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2F00) && (code <= 0x2FDF));
}

/*
 * rtxUCSIsKannada:
 * @code: UCS code point
 *
 * Check whether the character is part of Kannada UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsKannada (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0C80) && (code <= 0x0CFF));
}

/*
 * rtxUCSIsKatakana:
 * @code: UCS code point
 *
 * Check whether the character is part of Katakana UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsKatakana (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x30A0) && (code <= 0x30FF));
}

/*
 * rtxUCSIsKhmer:
 * @code: UCS code point
 *
 * Check whether the character is part of Khmer UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsKhmer (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1780) && (code <= 0x17FF));
}

/*
 * rtxUCSIsLao:
 * @code: UCS code point
 *
 * Check whether the character is part of Lao UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLao (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0E80) && (code <= 0x0EFF));
}

/*
 * rtxUCSIsLatin1Supplement:
 * @code: UCS code point
 *
 * Check whether the character is part of Latin-1Supplement UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLatin1Supplement (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0080) && (code <= 0x00FF));
}

/*
 * rtxUCSIsLatinExtendedA:
 * @code: UCS code point
 *
 * Check whether the character is part of LatinExtended-A UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLatinExtendedA (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0100) && (code <= 0x017F));
}

/*
 * rtxUCSIsLatinExtendedB:
 * @code: UCS code point
 *
 * Check whether the character is part of LatinExtended-B UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLatinExtendedB (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0180) && (code <= 0x024F));
}

/*
 * rtxUCSIsLatinExtendedAdditional:
 * @code: UCS code point
 *
 * Check whether the character is part of LatinExtendedAdditional UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLatinExtendedAdditional (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1E00) && (code <= 0x1EFF));
}

/*
 * rtxUCSIsLetterlikeSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of LetterlikeSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLetterlikeSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2100) && (code <= 0x214F));
}

/*
 * rtxUCSIsLowSurrogates:
 * @code: UCS code point
 *
 * Check whether the character is part of LowSurrogates UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsLowSurrogates (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xDC00) && (code <= 0xDFFF));
}

/*
 * rtxUCSIsMalayalam:
 * @code: UCS code point
 *
 * Check whether the character is part of Malayalam UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMalayalam (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0D00) && (code <= 0x0D7F));
}

/*
 * rtxUCSIsMathematicalAlphanumericSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of MathematicalAlphanumericSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMathematicalAlphanumericSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1D400) && (code <= 0x1D7FF));
}

/*
 * rtxUCSIsMathematicalOperators:
 * @code: UCS code point
 *
 * Check whether the character is part of MathematicalOperators UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMathematicalOperators (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2200) && (code <= 0x22FF));
}

/*
 * rtxUCSIsMiscellaneousSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of MiscellaneousSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMiscellaneousSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2600) && (code <= 0x26FF));
}

/*
 * rtxUCSIsMiscellaneousTechnical:
 * @code: UCS code point
 *
 * Check whether the character is part of MiscellaneousTechnical UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMiscellaneousTechnical (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2300) && (code <= 0x23FF));
}

/*
 * rtxUCSIsMongolian:
 * @code: UCS code point
 *
 * Check whether the character is part of Mongolian UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMongolian (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1800) && (code <= 0x18AF));
}

/*
 * rtxUCSIsMusicalSymbols:
 * @code: UCS code point
 *
 * Check whether the character is part of MusicalSymbols UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMusicalSymbols (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1D100) && (code <= 0x1D1FF));
}

/*
 * rtxUCSIsMyanmar:
 * @code: UCS code point
 *
 * Check whether the character is part of Myanmar UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsMyanmar (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1000) && (code <= 0x109F));
}

/*
 * rtxUCSIsNumberForms:
 * @code: UCS code point
 *
 * Check whether the character is part of NumberForms UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsNumberForms (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2150) && (code <= 0x218F));
}

/*
 * rtxUCSIsOgham:
 * @code: UCS code point
 *
 * Check whether the character is part of Ogham UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsOgham (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1680) && (code <= 0x169F));
}

/*
 * rtxUCSIsOldItalic:
 * @code: UCS code point
 *
 * Check whether the character is part of OldItalic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsOldItalic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x10300) && (code <= 0x1032F));
}

/*
 * rtxUCSIsOpticalCharacterRecognition:
 * @code: UCS code point
 *
 * Check whether the character is part of OpticalCharacterRecognition UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsOpticalCharacterRecognition (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2440) && (code <= 0x245F));
}

/*
 * rtxUCSIsOriya:
 * @code: UCS code point
 *
 * Check whether the character is part of Oriya UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsOriya (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0B00) && (code <= 0x0B7F));
}

/*
 * rtxUCSIsPrivateUse:
 * @code: UCS code point
 *
 * Check whether the character is part of PrivateUse UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsPrivateUse (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x100000) && (code <= 0x10FFFD));
}

/*
 * rtxUCSIsRunic:
 * @code: UCS code point
 *
 * Check whether the character is part of Runic UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsRunic (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x16A0) && (code <= 0x16FF));
}

/*
 * rtxUCSIsSinhala:
 * @code: UCS code point
 *
 * Check whether the character is part of Sinhala UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSinhala (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0D80) && (code <= 0x0DFF));
}

/*
 * rtxUCSIsSmallFormVariants:
 * @code: UCS code point
 *
 * Check whether the character is part of SmallFormVariants UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSmallFormVariants (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFE50) && (code <= 0xFE6F));
}

/*
 * rtxUCSIsSpacingModifierLetters:
 * @code: UCS code point
 *
 * Check whether the character is part of SpacingModifierLetters UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSpacingModifierLetters (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x02B0) && (code <= 0x02FF));
}

/*
 * rtxUCSIsSpecials:
 * @code: UCS code point
 *
 * Check whether the character is part of Specials UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSpecials (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xFFF0) && (code <= 0xFFFD));
}

/*
 * rtxUCSIsSuperscriptsandSubscripts:
 * @code: UCS code point
 *
 * Check whether the character is part of SuperscriptsandSubscripts UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSuperscriptsandSubscripts (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x2070) && (code <= 0x209F));
}

/*
 * rtxUCSIsSyriac:
 * @code: UCS code point
 *
 * Check whether the character is part of Syriac UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsSyriac (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0700) && (code <= 0x074F));
}

/*
 * rtxUCSIsTags:
 * @code: UCS code point
 *
 * Check whether the character is part of Tags UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsTags (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xE0000) && (code <= 0xE007F));
}

/*
 * rtxUCSIsTamil:
 * @code: UCS code point
 *
 * Check whether the character is part of Tamil UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsTamil (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0B80) && (code <= 0x0BFF));
}

/*
 * rtxUCSIsTelugu:
 * @code: UCS code point
 *
 * Check whether the character is part of Telugu UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsTelugu (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0C00) && (code <= 0x0C7F));
}

/*
 * rtxUCSIsThaana:
 * @code: UCS code point
 *
 * Check whether the character is part of Thaana UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsThaana (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0780) && (code <= 0x07BF));
}

/*
 * rtxUCSIsThai:
 * @code: UCS code point
 *
 * Check whether the character is part of Thai UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsThai (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0E00) && (code <= 0x0E7F));
}

/*
 * rtxUCSIsTibetan:
 * @code: UCS code point
 *
 * Check whether the character is part of Tibetan UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsTibetan (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x0F00) && (code <= 0x0FFF));
}

/*
 * rtxUCSIsUnifiedCanadianAboriginalSyllabics:
 * @code: UCS code point
 *
 * Check whether the character is part of UnifiedCanadianAboriginalSyllabics UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsUnifiedCanadianAboriginalSyllabics (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0x1400) && (code <= 0x167F));
}

/*
 * rtxUCSIsYiRadicals:
 * @code: UCS code point
 *
 * Check whether the character is part of YiRadicals UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsYiRadicals (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xA490) && (code <= 0xA4CF));
}

/*
 * rtxUCSIsYiSyllables:
 * @code: UCS code point
 *
 * Check whether the character is part of YiSyllables UCS Block
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsYiSyllables (OS32BITCHAR code)
{
    return (OSBOOL)((code >= 0xA000) && (code <= 0xA48F));
}

/*
 * rtxUCSIsBlock:
 * @code: UCS code point
 * @block: UCS block name
 *
 * Check whether the caracter is part of the UCS Block
 *
 * Returns 1 if true, 0 if false and -1 on unknown block
 */
EXTRTMETHOD int rtxUCSIsBlock (OS32BITCHAR code, const char *block)
{
    if (!OSCRTLSTRCMP(block, "AlphabeticPresentationForms"))
        return(rtxUCSIsAlphabeticPresentationForms(code));
    if (!OSCRTLSTRCMP(block, "Arabic"))
        return(rtxUCSIsArabic(code));
    if (!OSCRTLSTRCMP(block, "ArabicPresentationForms-A"))
        return(rtxUCSIsArabicPresentationFormsA(code));
    if (!OSCRTLSTRCMP(block, "ArabicPresentationForms-B"))
        return(rtxUCSIsArabicPresentationFormsB(code));
    if (!OSCRTLSTRCMP(block, "Armenian"))
        return(rtxUCSIsArmenian(code));
    if (!OSCRTLSTRCMP(block, "Arrows"))
        return(rtxUCSIsArrows(code));
    if (!OSCRTLSTRCMP(block, "BasicLatin"))
        return(rtxUCSIsBasicLatin(code));
    if (!OSCRTLSTRCMP(block, "Bengali"))
        return(rtxUCSIsBengali(code));
    if (!OSCRTLSTRCMP(block, "BlockElements"))
        return(rtxUCSIsBlockElements(code));
    if (!OSCRTLSTRCMP(block, "Bopomofo"))
        return(rtxUCSIsBopomofo(code));
    if (!OSCRTLSTRCMP(block, "BopomofoExtended"))
        return(rtxUCSIsBopomofoExtended(code));
    if (!OSCRTLSTRCMP(block, "BoxDrawing"))
        return(rtxUCSIsBoxDrawing(code));
    if (!OSCRTLSTRCMP(block, "BraillePatterns"))
        return(rtxUCSIsBraillePatterns(code));
    if (!OSCRTLSTRCMP(block, "ByzantineMusicalSymbols"))
        return(rtxUCSIsByzantineMusicalSymbols(code));
    if (!OSCRTLSTRCMP(block, "CJKCompatibility"))
        return(rtxUCSIsCJKCompatibility(code));
    if (!OSCRTLSTRCMP(block, "CJKCompatibilityForms"))
        return(rtxUCSIsCJKCompatibilityForms(code));
    if (!OSCRTLSTRCMP(block, "CJKCompatibilityIdeographs"))
        return(rtxUCSIsCJKCompatibilityIdeographs(code));
    if (!OSCRTLSTRCMP(block, "CJKCompatibilityIdeographsSupplement"))
        return(rtxUCSIsCJKCompatibilityIdeographsSupplement(code));
    if (!OSCRTLSTRCMP(block, "CJKRadicalsSupplement"))
        return(rtxUCSIsCJKRadicalsSupplement(code));
    if (!OSCRTLSTRCMP(block, "CJKSymbolsandPunctuation"))
        return(rtxUCSIsCJKSymbolsandPunctuation(code));
    if (!OSCRTLSTRCMP(block, "CJKUnifiedIdeographs"))
        return(rtxUCSIsCJKUnifiedIdeographs(code));
    if (!OSCRTLSTRCMP(block, "CJKUnifiedIdeographsExtensionA"))
        return(rtxUCSIsCJKUnifiedIdeographsExtensionA(code));
    if (!OSCRTLSTRCMP(block, "CJKUnifiedIdeographsExtensionB"))
        return(rtxUCSIsCJKUnifiedIdeographsExtensionB(code));
    if (!OSCRTLSTRCMP(block, "Cherokee"))
        return(rtxUCSIsCherokee(code));
    if (!OSCRTLSTRCMP(block, "CombiningDiacriticalMarks"))
        return(rtxUCSIsCombiningDiacriticalMarks(code));
    if (!OSCRTLSTRCMP(block, "CombiningHalfMarks"))
        return(rtxUCSIsCombiningHalfMarks(code));
    if (!OSCRTLSTRCMP(block, "CombiningMarksforSymbols"))
        return(rtxUCSIsCombiningMarksforSymbols(code));
    if (!OSCRTLSTRCMP(block, "ControlPictures"))
        return(rtxUCSIsControlPictures(code));
    if (!OSCRTLSTRCMP(block, "CurrencySymbols"))
        return(rtxUCSIsCurrencySymbols(code));
    if (!OSCRTLSTRCMP(block, "Cyrillic"))
        return(rtxUCSIsCyrillic(code));
    if (!OSCRTLSTRCMP(block, "Deseret"))
        return(rtxUCSIsDeseret(code));
    if (!OSCRTLSTRCMP(block, "Devanagari"))
        return(rtxUCSIsDevanagari(code));
    if (!OSCRTLSTRCMP(block, "Dingbats"))
        return(rtxUCSIsDingbats(code));
    if (!OSCRTLSTRCMP(block, "EnclosedAlphanumerics"))
        return(rtxUCSIsEnclosedAlphanumerics(code));
    if (!OSCRTLSTRCMP(block, "EnclosedCJKLettersandMonths"))
        return(rtxUCSIsEnclosedCJKLettersandMonths(code));
    if (!OSCRTLSTRCMP(block, "Ethiopic"))
        return(rtxUCSIsEthiopic(code));
    if (!OSCRTLSTRCMP(block, "GeneralPunctuation"))
        return(rtxUCSIsGeneralPunctuation(code));
    if (!OSCRTLSTRCMP(block, "GeometricShapes"))
        return(rtxUCSIsGeometricShapes(code));
    if (!OSCRTLSTRCMP(block, "Georgian"))
        return(rtxUCSIsGeorgian(code));
    if (!OSCRTLSTRCMP(block, "Gothic"))
        return(rtxUCSIsGothic(code));
    if (!OSCRTLSTRCMP(block, "Greek"))
        return(rtxUCSIsGreek(code));
    if (!OSCRTLSTRCMP(block, "GreekExtended"))
        return(rtxUCSIsGreekExtended(code));
    if (!OSCRTLSTRCMP(block, "Gujarati"))
        return(rtxUCSIsGujarati(code));
    if (!OSCRTLSTRCMP(block, "Gurmukhi"))
        return(rtxUCSIsGurmukhi(code));
    if (!OSCRTLSTRCMP(block, "HalfwidthandFullwidthForms"))
        return(rtxUCSIsHalfwidthandFullwidthForms(code));
    if (!OSCRTLSTRCMP(block, "HangulCompatibilityJamo"))
        return(rtxUCSIsHangulCompatibilityJamo(code));
    if (!OSCRTLSTRCMP(block, "HangulJamo"))
        return(rtxUCSIsHangulJamo(code));
    if (!OSCRTLSTRCMP(block, "HangulSyllables"))
        return(rtxUCSIsHangulSyllables(code));
    if (!OSCRTLSTRCMP(block, "Hebrew"))
        return(rtxUCSIsHebrew(code));
    if (!OSCRTLSTRCMP(block, "HighPrivateUseSurrogates"))
        return(rtxUCSIsHighPrivateUseSurrogates(code));
    if (!OSCRTLSTRCMP(block, "HighSurrogates"))
        return(rtxUCSIsHighSurrogates(code));
    if (!OSCRTLSTRCMP(block, "Hiragana"))
        return(rtxUCSIsHiragana(code));
    if (!OSCRTLSTRCMP(block, "IPAExtensions"))
        return(rtxUCSIsIPAExtensions(code));
    if (!OSCRTLSTRCMP(block, "IdeographicDescriptionCharacters"))
        return(rtxUCSIsIdeographicDescriptionCharacters(code));
    if (!OSCRTLSTRCMP(block, "Kanbun"))
        return(rtxUCSIsKanbun(code));
    if (!OSCRTLSTRCMP(block, "KangxiRadicals"))
        return(rtxUCSIsKangxiRadicals(code));
    if (!OSCRTLSTRCMP(block, "Kannada"))
        return(rtxUCSIsKannada(code));
    if (!OSCRTLSTRCMP(block, "Katakana"))
        return(rtxUCSIsKatakana(code));
    if (!OSCRTLSTRCMP(block, "Khmer"))
        return(rtxUCSIsKhmer(code));
    if (!OSCRTLSTRCMP(block, "Lao"))
        return(rtxUCSIsLao(code));
    if (!OSCRTLSTRCMP(block, "Latin-1Supplement"))
        return(rtxUCSIsLatin1Supplement(code));
    if (!OSCRTLSTRCMP(block, "LatinExtended-A"))
        return(rtxUCSIsLatinExtendedA(code));
    if (!OSCRTLSTRCMP(block, "LatinExtended-B"))
        return(rtxUCSIsLatinExtendedB(code));
    if (!OSCRTLSTRCMP(block, "LatinExtendedAdditional"))
        return(rtxUCSIsLatinExtendedAdditional(code));
    if (!OSCRTLSTRCMP(block, "LetterlikeSymbols"))
        return(rtxUCSIsLetterlikeSymbols(code));
    if (!OSCRTLSTRCMP(block, "LowSurrogates"))
        return(rtxUCSIsLowSurrogates(code));
    if (!OSCRTLSTRCMP(block, "Malayalam"))
        return(rtxUCSIsMalayalam(code));
    if (!OSCRTLSTRCMP(block, "MathematicalAlphanumericSymbols"))
        return(rtxUCSIsMathematicalAlphanumericSymbols(code));
    if (!OSCRTLSTRCMP(block, "MathematicalOperators"))
        return(rtxUCSIsMathematicalOperators(code));
    if (!OSCRTLSTRCMP(block, "MiscellaneousSymbols"))
        return(rtxUCSIsMiscellaneousSymbols(code));
    if (!OSCRTLSTRCMP(block, "MiscellaneousTechnical"))
        return(rtxUCSIsMiscellaneousTechnical(code));
    if (!OSCRTLSTRCMP(block, "Mongolian"))
        return(rtxUCSIsMongolian(code));
    if (!OSCRTLSTRCMP(block, "MusicalSymbols"))
        return(rtxUCSIsMusicalSymbols(code));
    if (!OSCRTLSTRCMP(block, "Myanmar"))
        return(rtxUCSIsMyanmar(code));
    if (!OSCRTLSTRCMP(block, "NumberForms"))
        return(rtxUCSIsNumberForms(code));
    if (!OSCRTLSTRCMP(block, "Ogham"))
        return(rtxUCSIsOgham(code));
    if (!OSCRTLSTRCMP(block, "OldItalic"))
        return(rtxUCSIsOldItalic(code));
    if (!OSCRTLSTRCMP(block, "OpticalCharacterRecognition"))
        return(rtxUCSIsOpticalCharacterRecognition(code));
    if (!OSCRTLSTRCMP(block, "Oriya"))
        return(rtxUCSIsOriya(code));
    if (!OSCRTLSTRCMP(block, "PrivateUse"))
        return(rtxUCSIsPrivateUse(code));
    if (!OSCRTLSTRCMP(block, "Runic"))
        return(rtxUCSIsRunic(code));
    if (!OSCRTLSTRCMP(block, "Sinhala"))
        return(rtxUCSIsSinhala(code));
    if (!OSCRTLSTRCMP(block, "SmallFormVariants"))
        return(rtxUCSIsSmallFormVariants(code));
    if (!OSCRTLSTRCMP(block, "SpacingModifierLetters"))
        return(rtxUCSIsSpacingModifierLetters(code));
    if (!OSCRTLSTRCMP(block, "Specials"))
        return(rtxUCSIsSpecials(code));
    if (!OSCRTLSTRCMP(block, "SuperscriptsandSubscripts"))
        return(rtxUCSIsSuperscriptsandSubscripts(code));
    if (!OSCRTLSTRCMP(block, "Syriac"))
        return(rtxUCSIsSyriac(code));
    if (!OSCRTLSTRCMP(block, "Tags"))
        return(rtxUCSIsTags(code));
    if (!OSCRTLSTRCMP(block, "Tamil"))
        return(rtxUCSIsTamil(code));
    if (!OSCRTLSTRCMP(block, "Telugu"))
        return(rtxUCSIsTelugu(code));
    if (!OSCRTLSTRCMP(block, "Thaana"))
        return(rtxUCSIsThaana(code));
    if (!OSCRTLSTRCMP(block, "Thai"))
        return(rtxUCSIsThai(code));
    if (!OSCRTLSTRCMP(block, "Tibetan"))
        return(rtxUCSIsTibetan(code));
    if (!OSCRTLSTRCMP(block, "UnifiedCanadianAboriginalSyllabics"))
        return(rtxUCSIsUnifiedCanadianAboriginalSyllabics(code));
    if (!OSCRTLSTRCMP(block, "YiRadicals"))
        return(rtxUCSIsYiRadicals(code));
    if (!OSCRTLSTRCMP(block, "YiSyllables"))
        return(rtxUCSIsYiSyllables(code));
    return(-1);
}

/*
 * rtxUCSIsCatC:
 * @code: UCS code point
 *
 * Check whether the character is part of C UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatC (OS32BITCHAR code)
{
    return (OSBOOL)((/*(code >= 0x0) &&*/ (code <= 0x1f)) ||
           ((code >= 0x7f) && (code <= 0x9f)) ||
           (code == 0x70f) ||
           ((code >= 0x180b) && (code <= 0x180e)) ||
           ((code >= 0x200c) && (code <= 0x200f)) ||
           ((code >= 0x202a) && (code <= 0x202e)) ||
           ((code >= 0x206a) && (code <= 0x206f)) ||
           (code == 0xd800) ||
           ((code >= 0xdb7f) && (code <= 0xdb80)) ||
           ((code >= 0xdbff) && (code <= 0xdc00)) ||
           ((code >= 0xdfff) && (code <= 0xe000)) ||
           (code == 0xf8ff) ||
           (code == 0xfeff) ||
           ((code >= 0xfff9) && (code <= 0xfffb)) ||
           ((code >= 0x1d173) && (code <= 0x1d17a)) ||
           (code == 0xe0001) ||
           ((code >= 0xe0020) && (code <= 0xe007f)) ||
           (code == 0xf0000) ||
           (code == 0xffffd) ||
           (code == 0x100000) ||
           (code == 0x10fffd));
}

/*
 * rtxUCSIsCatCc:
 * @code: UCS code point
 *
 * Check whether the character is part of Cc UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatCc (OS32BITCHAR code)
{
    return (OSBOOL)((/*(code >= 0x0) &&*/ (code <= 0x1f)) ||
           ((code >= 0x7f) && (code <= 0x9f)));
}

/*
 * rtxUCSIsCatCf:
 * @code: UCS code point
 *
 * Check whether the character is part of Cf UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatCf (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x70f) ||
           ((code >= 0x180b) && (code <= 0x180e)) ||
           ((code >= 0x200c) && (code <= 0x200f)) ||
           ((code >= 0x202a) && (code <= 0x202e)) ||
           ((code >= 0x206a) && (code <= 0x206f)) ||
           (code == 0xfeff) ||
           ((code >= 0xfff9) && (code <= 0xfffb)) ||
           ((code >= 0x1d173) && (code <= 0x1d17a)) ||
           (code == 0xe0001) ||
           ((code >= 0xe0020) && (code <= 0xe007f)));
}

/*
 * rtxUCSIsCatCo:
 * @code: UCS code point
 *
 * Check whether the character is part of Co UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatCo (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0xe000) ||
           (code == 0xf8ff) ||
           (code == 0xf0000) ||
           (code == 0xffffd) ||
           (code == 0x100000) ||
           (code == 0x10fffd));
}

/*
 * rtxUCSIsCatCs:
 * @code: UCS code point
 *
 * Check whether the character is part of Cs UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatCs (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0xd800) ||
           ((code >= 0xdb7f) && (code <= 0xdb80)) ||
           ((code >= 0xdbff) && (code <= 0xdc00)) ||
           (code == 0xdfff));
}

/*
 * rtxUCSIsCatL:
 * @code: UCS code point
 *
 * Check whether the character is part of L UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatL (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x41) && (code <= 0x5a)) ||
           ((code >= 0x61) && (code <= 0x7a)) ||
           (code == 0xaa) ||
           (code == 0xb5) ||
           (code == 0xba) ||
           ((code >= 0xc0) && (code <= 0xd6)) ||
           ((code >= 0xd8) && (code <= 0xf6)) ||
           ((code >= 0xf8) && (code <= 0x21f)) ||
           ((code >= 0x222) && (code <= 0x233)) ||
           ((code >= 0x250) && (code <= 0x2ad)) ||
           ((code >= 0x2b0) && (code <= 0x2b8)) ||
           ((code >= 0x2bb) && (code <= 0x2c1)) ||
           ((code >= 0x2d0) && (code <= 0x2d1)) ||
           ((code >= 0x2e0) && (code <= 0x2e4)) ||
           (code == 0x2ee) ||
           (code == 0x37a) ||
           (code == 0x386) ||
           ((code >= 0x388) && (code <= 0x38a)) ||
           (code == 0x38c) ||
           ((code >= 0x38e) && (code <= 0x3a1)) ||
           ((code >= 0x3a3) && (code <= 0x3ce)) ||
           ((code >= 0x3d0) && (code <= 0x3d7)) ||
           ((code >= 0x3da) && (code <= 0x3f5)) ||
           ((code >= 0x400) && (code <= 0x481)) ||
           ((code >= 0x48c) && (code <= 0x4c4)) ||
           ((code >= 0x4c7) && (code <= 0x4c8)) ||
           ((code >= 0x4cb) && (code <= 0x4cc)) ||
           ((code >= 0x4d0) && (code <= 0x4f5)) ||
           ((code >= 0x4f8) && (code <= 0x4f9)) ||
           ((code >= 0x531) && (code <= 0x556)) ||
           (code == 0x559) ||
           ((code >= 0x561) && (code <= 0x587)) ||
           ((code >= 0x5d0) && (code <= 0x5ea)) ||
           ((code >= 0x5f0) && (code <= 0x5f2)) ||
           ((code >= 0x621) && (code <= 0x63a)) ||
           ((code >= 0x640) && (code <= 0x64a)) ||
           ((code >= 0x671) && (code <= 0x6d3)) ||
           (code == 0x6d5) ||
           ((code >= 0x6e5) && (code <= 0x6e6)) ||
           ((code >= 0x6fa) && (code <= 0x6fc)) ||
           (code == 0x710) ||
           ((code >= 0x712) && (code <= 0x72c)) ||
           ((code >= 0x780) && (code <= 0x7a5)) ||
           ((code >= 0x905) && (code <= 0x939)) ||
           (code == 0x93d) ||
           (code == 0x950) ||
           ((code >= 0x958) && (code <= 0x961)) ||
           ((code >= 0x985) && (code <= 0x98c)) ||
           ((code >= 0x98f) && (code <= 0x990)) ||
           ((code >= 0x993) && (code <= 0x9a8)) ||
           ((code >= 0x9aa) && (code <= 0x9b0)) ||
           (code == 0x9b2) ||
           ((code >= 0x9b6) && (code <= 0x9b9)) ||
           ((code >= 0x9dc) && (code <= 0x9dd)) ||
           ((code >= 0x9df) && (code <= 0x9e1)) ||
           ((code >= 0x9f0) && (code <= 0x9f1)) ||
           ((code >= 0xa05) && (code <= 0xa0a)) ||
           ((code >= 0xa0f) && (code <= 0xa10)) ||
           ((code >= 0xa13) && (code <= 0xa28)) ||
           ((code >= 0xa2a) && (code <= 0xa30)) ||
           ((code >= 0xa32) && (code <= 0xa33)) ||
           ((code >= 0xa35) && (code <= 0xa36)) ||
           ((code >= 0xa38) && (code <= 0xa39)) ||
           ((code >= 0xa59) && (code <= 0xa5c)) ||
           (code == 0xa5e) ||
           ((code >= 0xa72) && (code <= 0xa74)) ||
           ((code >= 0xa85) && (code <= 0xa8b)) ||
           (code == 0xa8d) ||
           ((code >= 0xa8f) && (code <= 0xa91)) ||
           ((code >= 0xa93) && (code <= 0xaa8)) ||
           ((code >= 0xaaa) && (code <= 0xab0)) ||
           ((code >= 0xab2) && (code <= 0xab3)) ||
           ((code >= 0xab5) && (code <= 0xab9)) ||
           (code == 0xabd) ||
           (code == 0xad0) ||
           (code == 0xae0) ||
           ((code >= 0xb05) && (code <= 0xb0c)) ||
           ((code >= 0xb0f) && (code <= 0xb10)) ||
           ((code >= 0xb13) && (code <= 0xb28)) ||
           ((code >= 0xb2a) && (code <= 0xb30)) ||
           ((code >= 0xb32) && (code <= 0xb33)) ||
           ((code >= 0xb36) && (code <= 0xb39)) ||
           (code == 0xb3d) ||
           ((code >= 0xb5c) && (code <= 0xb5d)) ||
           ((code >= 0xb5f) && (code <= 0xb61)) ||
           ((code >= 0xb85) && (code <= 0xb8a)) ||
           ((code >= 0xb8e) && (code <= 0xb90)) ||
           ((code >= 0xb92) && (code <= 0xb95)) ||
           ((code >= 0xb99) && (code <= 0xb9a)) ||
           (code == 0xb9c) ||
           ((code >= 0xb9e) && (code <= 0xb9f)) ||
           ((code >= 0xba3) && (code <= 0xba4)) ||
           ((code >= 0xba8) && (code <= 0xbaa)) ||
           ((code >= 0xbae) && (code <= 0xbb5)) ||
           ((code >= 0xbb7) && (code <= 0xbb9)) ||
           ((code >= 0xc05) && (code <= 0xc0c)) ||
           ((code >= 0xc0e) && (code <= 0xc10)) ||
           ((code >= 0xc12) && (code <= 0xc28)) ||
           ((code >= 0xc2a) && (code <= 0xc33)) ||
           ((code >= 0xc35) && (code <= 0xc39)) ||
           ((code >= 0xc60) && (code <= 0xc61)) ||
           ((code >= 0xc85) && (code <= 0xc8c)) ||
           ((code >= 0xc8e) && (code <= 0xc90)) ||
           ((code >= 0xc92) && (code <= 0xca8)) ||
           ((code >= 0xcaa) && (code <= 0xcb3)) ||
           ((code >= 0xcb5) && (code <= 0xcb9)) ||
           (code == 0xcde) ||
           ((code >= 0xce0) && (code <= 0xce1)) ||
           ((code >= 0xd05) && (code <= 0xd0c)) ||
           ((code >= 0xd0e) && (code <= 0xd10)) ||
           ((code >= 0xd12) && (code <= 0xd28)) ||
           ((code >= 0xd2a) && (code <= 0xd39)) ||
           ((code >= 0xd60) && (code <= 0xd61)) ||
           ((code >= 0xd85) && (code <= 0xd96)) ||
           ((code >= 0xd9a) && (code <= 0xdb1)) ||
           ((code >= 0xdb3) && (code <= 0xdbb)) ||
           (code == 0xdbd) ||
           ((code >= 0xdc0) && (code <= 0xdc6)) ||
           ((code >= 0xe01) && (code <= 0xe30)) ||
           ((code >= 0xe32) && (code <= 0xe33)) ||
           ((code >= 0xe40) && (code <= 0xe46)) ||
           ((code >= 0xe81) && (code <= 0xe82)) ||
           (code == 0xe84) ||
           ((code >= 0xe87) && (code <= 0xe88)) ||
           (code == 0xe8a) ||
           (code == 0xe8d) ||
           ((code >= 0xe94) && (code <= 0xe97)) ||
           ((code >= 0xe99) && (code <= 0xe9f)) ||
           ((code >= 0xea1) && (code <= 0xea3)) ||
           (code == 0xea5) ||
           (code == 0xea7) ||
           ((code >= 0xeaa) && (code <= 0xeab)) ||
           ((code >= 0xead) && (code <= 0xeb0)) ||
           ((code >= 0xeb2) && (code <= 0xeb3)) ||
           (code == 0xebd) ||
           ((code >= 0xec0) && (code <= 0xec4)) ||
           (code == 0xec6) ||
           ((code >= 0xedc) && (code <= 0xedd)) ||
           (code == 0xf00) ||
           ((code >= 0xf40) && (code <= 0xf47)) ||
           ((code >= 0xf49) && (code <= 0xf6a)) ||
           ((code >= 0xf88) && (code <= 0xf8b)) ||
           ((code >= 0x1000) && (code <= 0x1021)) ||
           ((code >= 0x1023) && (code <= 0x1027)) ||
           ((code >= 0x1029) && (code <= 0x102a)) ||
           ((code >= 0x1050) && (code <= 0x1055)) ||
           ((code >= 0x10a0) && (code <= 0x10c5)) ||
           ((code >= 0x10d0) && (code <= 0x10f6)) ||
           ((code >= 0x1100) && (code <= 0x1159)) ||
           ((code >= 0x115f) && (code <= 0x11a2)) ||
           ((code >= 0x11a8) && (code <= 0x11f9)) ||
           ((code >= 0x1200) && (code <= 0x1206)) ||
           ((code >= 0x1208) && (code <= 0x1246)) ||
           (code == 0x1248) ||
           ((code >= 0x124a) && (code <= 0x124d)) ||
           ((code >= 0x1250) && (code <= 0x1256)) ||
           (code == 0x1258) ||
           ((code >= 0x125a) && (code <= 0x125d)) ||
           ((code >= 0x1260) && (code <= 0x1286)) ||
           (code == 0x1288) ||
           ((code >= 0x128a) && (code <= 0x128d)) ||
           ((code >= 0x1290) && (code <= 0x12ae)) ||
           (code == 0x12b0) ||
           ((code >= 0x12b2) && (code <= 0x12b5)) ||
           ((code >= 0x12b8) && (code <= 0x12be)) ||
           (code == 0x12c0) ||
           ((code >= 0x12c2) && (code <= 0x12c5)) ||
           ((code >= 0x12c8) && (code <= 0x12ce)) ||
           ((code >= 0x12d0) && (code <= 0x12d6)) ||
           ((code >= 0x12d8) && (code <= 0x12ee)) ||
           ((code >= 0x12f0) && (code <= 0x130e)) ||
           (code == 0x1310) ||
           ((code >= 0x1312) && (code <= 0x1315)) ||
           ((code >= 0x1318) && (code <= 0x131e)) ||
           ((code >= 0x1320) && (code <= 0x1346)) ||
           ((code >= 0x1348) && (code <= 0x135a)) ||
           ((code >= 0x13a0) && (code <= 0x13f4)) ||
           ((code >= 0x1401) && (code <= 0x166c)) ||
           ((code >= 0x166f) && (code <= 0x1676)) ||
           ((code >= 0x1681) && (code <= 0x169a)) ||
           ((code >= 0x16a0) && (code <= 0x16ea)) ||
           ((code >= 0x1780) && (code <= 0x17b3)) ||
           ((code >= 0x1820) && (code <= 0x1877)) ||
           ((code >= 0x1880) && (code <= 0x18a8)) ||
           ((code >= 0x1e00) && (code <= 0x1e9b)) ||
           ((code >= 0x1ea0) && (code <= 0x1ef9)) ||
           ((code >= 0x1f00) && (code <= 0x1f15)) ||
           ((code >= 0x1f18) && (code <= 0x1f1d)) ||
           ((code >= 0x1f20) && (code <= 0x1f45)) ||
           ((code >= 0x1f48) && (code <= 0x1f4d)) ||
           ((code >= 0x1f50) && (code <= 0x1f57)) ||
           (code == 0x1f59) ||
           (code == 0x1f5b) ||
           (code == 0x1f5d) ||
           ((code >= 0x1f5f) && (code <= 0x1f7d)) ||
           ((code >= 0x1f80) && (code <= 0x1fb4)) ||
           ((code >= 0x1fb6) && (code <= 0x1fbc)) ||
           (code == 0x1fbe) ||
           ((code >= 0x1fc2) && (code <= 0x1fc4)) ||
           ((code >= 0x1fc6) && (code <= 0x1fcc)) ||
           ((code >= 0x1fd0) && (code <= 0x1fd3)) ||
           ((code >= 0x1fd6) && (code <= 0x1fdb)) ||
           ((code >= 0x1fe0) && (code <= 0x1fec)) ||
           ((code >= 0x1ff2) && (code <= 0x1ff4)) ||
           ((code >= 0x1ff6) && (code <= 0x1ffc)) ||
           (code == 0x207f) ||
           (code == 0x2102) ||
           (code == 0x2107) ||
           ((code >= 0x210a) && (code <= 0x2113)) ||
           (code == 0x2115) ||
           ((code >= 0x2119) && (code <= 0x211d)) ||
           (code == 0x2124) ||
           (code == 0x2126) ||
           (code == 0x2128) ||
           ((code >= 0x212a) && (code <= 0x212d)) ||
           ((code >= 0x212f) && (code <= 0x2131)) ||
           ((code >= 0x2133) && (code <= 0x2139)) ||
           ((code >= 0x3005) && (code <= 0x3006)) ||
           ((code >= 0x3031) && (code <= 0x3035)) ||
           ((code >= 0x3041) && (code <= 0x3094)) ||
           ((code >= 0x309d) && (code <= 0x309e)) ||
           ((code >= 0x30a1) && (code <= 0x30fa)) ||
           ((code >= 0x30fc) && (code <= 0x30fe)) ||
           ((code >= 0x3105) && (code <= 0x312c)) ||
           ((code >= 0x3131) && (code <= 0x318e)) ||
           ((code >= 0x31a0) && (code <= 0x31b7)) ||
           (code == 0x3400) ||
           (code == 0x4db5) ||
           (code == 0x4e00) ||
           (code == 0x9fa5) ||
           ((code >= 0xa000) && (code <= 0xa48c)) ||
           (code == 0xac00) ||
           (code == 0xd7a3) ||
           ((code >= 0xf900) && (code <= 0xfa2d)) ||
           ((code >= 0xfb00) && (code <= 0xfb06)) ||
           ((code >= 0xfb13) && (code <= 0xfb17)) ||
           (code == 0xfb1d) ||
           ((code >= 0xfb1f) && (code <= 0xfb28)) ||
           ((code >= 0xfb2a) && (code <= 0xfb36)) ||
           ((code >= 0xfb38) && (code <= 0xfb3c)) ||
           (code == 0xfb3e) ||
           ((code >= 0xfb40) && (code <= 0xfb41)) ||
           ((code >= 0xfb43) && (code <= 0xfb44)) ||
           ((code >= 0xfb46) && (code <= 0xfbb1)) ||
           ((code >= 0xfbd3) && (code <= 0xfd3d)) ||
           ((code >= 0xfd50) && (code <= 0xfd8f)) ||
           ((code >= 0xfd92) && (code <= 0xfdc7)) ||
           ((code >= 0xfdf0) && (code <= 0xfdfb)) ||
           ((code >= 0xfe70) && (code <= 0xfe72)) ||
           (code == 0xfe74) ||
           ((code >= 0xfe76) && (code <= 0xfefc)) ||
           ((code >= 0xff21) && (code <= 0xff3a)) ||
           ((code >= 0xff41) && (code <= 0xff5a)) ||
           ((code >= 0xff66) && (code <= 0xffbe)) ||
           ((code >= 0xffc2) && (code <= 0xffc7)) ||
           ((code >= 0xffca) && (code <= 0xffcf)) ||
           ((code >= 0xffd2) && (code <= 0xffd7)) ||
           ((code >= 0xffda) && (code <= 0xffdc)) ||
           ((code >= 0x10300) && (code <= 0x1031e)) ||
           ((code >= 0x10330) && (code <= 0x10349)) ||
           ((code >= 0x10400) && (code <= 0x10425)) ||
           ((code >= 0x10428) && (code <= 0x1044d)) ||
           ((code >= 0x1d400) && (code <= 0x1d454)) ||
           ((code >= 0x1d456) && (code <= 0x1d49c)) ||
           ((code >= 0x1d49e) && (code <= 0x1d49f)) ||
           (code == 0x1d4a2) ||
           ((code >= 0x1d4a5) && (code <= 0x1d4a6)) ||
           ((code >= 0x1d4a9) && (code <= 0x1d4ac)) ||
           ((code >= 0x1d4ae) && (code <= 0x1d4b9)) ||
           (code == 0x1d4bb) ||
           ((code >= 0x1d4bd) && (code <= 0x1d4c0)) ||
           ((code >= 0x1d4c2) && (code <= 0x1d4c3)) ||
           ((code >= 0x1d4c5) && (code <= 0x1d505)) ||
           ((code >= 0x1d507) && (code <= 0x1d50a)) ||
           ((code >= 0x1d50d) && (code <= 0x1d514)) ||
           ((code >= 0x1d516) && (code <= 0x1d51c)) ||
           ((code >= 0x1d51e) && (code <= 0x1d539)) ||
           ((code >= 0x1d53b) && (code <= 0x1d53e)) ||
           ((code >= 0x1d540) && (code <= 0x1d544)) ||
           (code == 0x1d546) ||
           ((code >= 0x1d54a) && (code <= 0x1d550)) ||
           ((code >= 0x1d552) && (code <= 0x1d6a3)) ||
           ((code >= 0x1d6a8) && (code <= 0x1d6c0)) ||
           ((code >= 0x1d6c2) && (code <= 0x1d6da)) ||
           ((code >= 0x1d6dc) && (code <= 0x1d6fa)) ||
           ((code >= 0x1d6fc) && (code <= 0x1d714)) ||
           ((code >= 0x1d716) && (code <= 0x1d734)) ||
           ((code >= 0x1d736) && (code <= 0x1d74e)) ||
           ((code >= 0x1d750) && (code <= 0x1d76e)) ||
           ((code >= 0x1d770) && (code <= 0x1d788)) ||
           ((code >= 0x1d78a) && (code <= 0x1d7a8)) ||
           ((code >= 0x1d7aa) && (code <= 0x1d7c2)) ||
           ((code >= 0x1d7c4) && (code <= 0x1d7c9)) ||
           (code == 0x20000) ||
           (code == 0x2a6d6) ||
           ((code >= 0x2f800) && (code <= 0x2fa1d)));
}

/*
 * rtxUCSIsCatLl:
 * @code: UCS code point
 *
 * Check whether the character is part of Ll UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatLl (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x61) && (code <= 0x7a)) ||
           (code == 0xaa) ||
           (code == 0xb5) ||
           (code == 0xba) ||
           ((code >= 0xdf) && (code <= 0xf6)) ||
           ((code >= 0xf8) && (code <= 0xff)) ||
           (code == 0x101) ||
           (code == 0x103) ||
           (code == 0x105) ||
           (code == 0x107) ||
           (code == 0x109) ||
           (code == 0x10b) ||
           (code == 0x10d) ||
           (code == 0x10f) ||
           (code == 0x111) ||
           (code == 0x113) ||
           (code == 0x115) ||
           (code == 0x117) ||
           (code == 0x119) ||
           (code == 0x11b) ||
           (code == 0x11d) ||
           (code == 0x11f) ||
           (code == 0x121) ||
           (code == 0x123) ||
           (code == 0x125) ||
           (code == 0x127) ||
           (code == 0x129) ||
           (code == 0x12b) ||
           (code == 0x12d) ||
           (code == 0x12f) ||
           (code == 0x131) ||
           (code == 0x133) ||
           (code == 0x135) ||
           ((code >= 0x137) && (code <= 0x138)) ||
           (code == 0x13a) ||
           (code == 0x13c) ||
           (code == 0x13e) ||
           (code == 0x140) ||
           (code == 0x142) ||
           (code == 0x144) ||
           (code == 0x146) ||
           ((code >= 0x148) && (code <= 0x149)) ||
           (code == 0x14b) ||
           (code == 0x14d) ||
           (code == 0x14f) ||
           (code == 0x151) ||
           (code == 0x153) ||
           (code == 0x155) ||
           (code == 0x157) ||
           (code == 0x159) ||
           (code == 0x15b) ||
           (code == 0x15d) ||
           (code == 0x15f) ||
           (code == 0x161) ||
           (code == 0x163) ||
           (code == 0x165) ||
           (code == 0x167) ||
           (code == 0x169) ||
           (code == 0x16b) ||
           (code == 0x16d) ||
           (code == 0x16f) ||
           (code == 0x171) ||
           (code == 0x173) ||
           (code == 0x175) ||
           (code == 0x177) ||
           (code == 0x17a) ||
           (code == 0x17c) ||
           ((code >= 0x17e) && (code <= 0x180)) ||
           (code == 0x183) ||
           (code == 0x185) ||
           (code == 0x188) ||
           ((code >= 0x18c) && (code <= 0x18d)) ||
           (code == 0x192) ||
           (code == 0x195) ||
           ((code >= 0x199) && (code <= 0x19b)) ||
           (code == 0x19e) ||
           (code == 0x1a1) ||
           (code == 0x1a3) ||
           (code == 0x1a5) ||
           (code == 0x1a8) ||
           ((code >= 0x1aa) && (code <= 0x1ab)) ||
           (code == 0x1ad) ||
           (code == 0x1b0) ||
           (code == 0x1b4) ||
           (code == 0x1b6) ||
           ((code >= 0x1b9) && (code <= 0x1ba)) ||
           ((code >= 0x1bd) && (code <= 0x1bf)) ||
           (code == 0x1c6) ||
           (code == 0x1c9) ||
           (code == 0x1cc) ||
           (code == 0x1ce) ||
           (code == 0x1d0) ||
           (code == 0x1d2) ||
           (code == 0x1d4) ||
           (code == 0x1d6) ||
           (code == 0x1d8) ||
           (code == 0x1da) ||
           ((code >= 0x1dc) && (code <= 0x1dd)) ||
           (code == 0x1df) ||
           (code == 0x1e1) ||
           (code == 0x1e3) ||
           (code == 0x1e5) ||
           (code == 0x1e7) ||
           (code == 0x1e9) ||
           (code == 0x1eb) ||
           (code == 0x1ed) ||
           ((code >= 0x1ef) && (code <= 0x1f0)) ||
           (code == 0x1f3) ||
           (code == 0x1f5) ||
           (code == 0x1f9) ||
           (code == 0x1fb) ||
           (code == 0x1fd) ||
           (code == 0x1ff) ||
           (code == 0x201) ||
           (code == 0x203) ||
           (code == 0x205) ||
           (code == 0x207) ||
           (code == 0x209) ||
           (code == 0x20b) ||
           (code == 0x20d) ||
           (code == 0x20f) ||
           (code == 0x211) ||
           (code == 0x213) ||
           (code == 0x215) ||
           (code == 0x217) ||
           (code == 0x219) ||
           (code == 0x21b) ||
           (code == 0x21d) ||
           (code == 0x21f) ||
           (code == 0x223) ||
           (code == 0x225) ||
           (code == 0x227) ||
           (code == 0x229) ||
           (code == 0x22b) ||
           (code == 0x22d) ||
           (code == 0x22f) ||
           (code == 0x231) ||
           (code == 0x233) ||
           ((code >= 0x250) && (code <= 0x2ad)) ||
           (code == 0x390) ||
           ((code >= 0x3ac) && (code <= 0x3ce)) ||
           ((code >= 0x3d0) && (code <= 0x3d1)) ||
           ((code >= 0x3d5) && (code <= 0x3d7)) ||
           (code == 0x3db) ||
           (code == 0x3dd) ||
           (code == 0x3df) ||
           (code == 0x3e1) ||
           (code == 0x3e3) ||
           (code == 0x3e5) ||
           (code == 0x3e7) ||
           (code == 0x3e9) ||
           (code == 0x3eb) ||
           (code == 0x3ed) ||
           ((code >= 0x3ef) && (code <= 0x3f3)) ||
           (code == 0x3f5) ||
           ((code >= 0x430) && (code <= 0x45f)) ||
           (code == 0x461) ||
           (code == 0x463) ||
           (code == 0x465) ||
           (code == 0x467) ||
           (code == 0x469) ||
           (code == 0x46b) ||
           (code == 0x46d) ||
           (code == 0x46f) ||
           (code == 0x471) ||
           (code == 0x473) ||
           (code == 0x475) ||
           (code == 0x477) ||
           (code == 0x479) ||
           (code == 0x47b) ||
           (code == 0x47d) ||
           (code == 0x47f) ||
           (code == 0x481) ||
           (code == 0x48d) ||
           (code == 0x48f) ||
           (code == 0x491) ||
           (code == 0x493) ||
           (code == 0x495) ||
           (code == 0x497) ||
           (code == 0x499) ||
           (code == 0x49b) ||
           (code == 0x49d) ||
           (code == 0x49f) ||
           (code == 0x4a1) ||
           (code == 0x4a3) ||
           (code == 0x4a5) ||
           (code == 0x4a7) ||
           (code == 0x4a9) ||
           (code == 0x4ab) ||
           (code == 0x4ad) ||
           (code == 0x4af) ||
           (code == 0x4b1) ||
           (code == 0x4b3) ||
           (code == 0x4b5) ||
           (code == 0x4b7) ||
           (code == 0x4b9) ||
           (code == 0x4bb) ||
           (code == 0x4bd) ||
           (code == 0x4bf) ||
           (code == 0x4c2) ||
           (code == 0x4c4) ||
           (code == 0x4c8) ||
           (code == 0x4cc) ||
           (code == 0x4d1) ||
           (code == 0x4d3) ||
           (code == 0x4d5) ||
           (code == 0x4d7) ||
           (code == 0x4d9) ||
           (code == 0x4db) ||
           (code == 0x4dd) ||
           (code == 0x4df) ||
           (code == 0x4e1) ||
           (code == 0x4e3) ||
           (code == 0x4e5) ||
           (code == 0x4e7) ||
           (code == 0x4e9) ||
           (code == 0x4eb) ||
           (code == 0x4ed) ||
           (code == 0x4ef) ||
           (code == 0x4f1) ||
           (code == 0x4f3) ||
           (code == 0x4f5) ||
           (code == 0x4f9) ||
           ((code >= 0x561) && (code <= 0x587)) ||
           (code == 0x1e01) ||
           (code == 0x1e03) ||
           (code == 0x1e05) ||
           (code == 0x1e07) ||
           (code == 0x1e09) ||
           (code == 0x1e0b) ||
           (code == 0x1e0d) ||
           (code == 0x1e0f) ||
           (code == 0x1e11) ||
           (code == 0x1e13) ||
           (code == 0x1e15) ||
           (code == 0x1e17) ||
           (code == 0x1e19) ||
           (code == 0x1e1b) ||
           (code == 0x1e1d) ||
           (code == 0x1e1f) ||
           (code == 0x1e21) ||
           (code == 0x1e23) ||
           (code == 0x1e25) ||
           (code == 0x1e27) ||
           (code == 0x1e29) ||
           (code == 0x1e2b) ||
           (code == 0x1e2d) ||
           (code == 0x1e2f) ||
           (code == 0x1e31) ||
           (code == 0x1e33) ||
           (code == 0x1e35) ||
           (code == 0x1e37) ||
           (code == 0x1e39) ||
           (code == 0x1e3b) ||
           (code == 0x1e3d) ||
           (code == 0x1e3f) ||
           (code == 0x1e41) ||
           (code == 0x1e43) ||
           (code == 0x1e45) ||
           (code == 0x1e47) ||
           (code == 0x1e49) ||
           (code == 0x1e4b) ||
           (code == 0x1e4d) ||
           (code == 0x1e4f) ||
           (code == 0x1e51) ||
           (code == 0x1e53) ||
           (code == 0x1e55) ||
           (code == 0x1e57) ||
           (code == 0x1e59) ||
           (code == 0x1e5b) ||
           (code == 0x1e5d) ||
           (code == 0x1e5f) ||
           (code == 0x1e61) ||
           (code == 0x1e63) ||
           (code == 0x1e65) ||
           (code == 0x1e67) ||
           (code == 0x1e69) ||
           (code == 0x1e6b) ||
           (code == 0x1e6d) ||
           (code == 0x1e6f) ||
           (code == 0x1e71) ||
           (code == 0x1e73) ||
           (code == 0x1e75) ||
           (code == 0x1e77) ||
           (code == 0x1e79) ||
           (code == 0x1e7b) ||
           (code == 0x1e7d) ||
           (code == 0x1e7f) ||
           (code == 0x1e81) ||
           (code == 0x1e83) ||
           (code == 0x1e85) ||
           (code == 0x1e87) ||
           (code == 0x1e89) ||
           (code == 0x1e8b) ||
           (code == 0x1e8d) ||
           (code == 0x1e8f) ||
           (code == 0x1e91) ||
           (code == 0x1e93) ||
           ((code >= 0x1e95) && (code <= 0x1e9b)) ||
           (code == 0x1ea1) ||
           (code == 0x1ea3) ||
           (code == 0x1ea5) ||
           (code == 0x1ea7) ||
           (code == 0x1ea9) ||
           (code == 0x1eab) ||
           (code == 0x1ead) ||
           (code == 0x1eaf) ||
           (code == 0x1eb1) ||
           (code == 0x1eb3) ||
           (code == 0x1eb5) ||
           (code == 0x1eb7) ||
           (code == 0x1eb9) ||
           (code == 0x1ebb) ||
           (code == 0x1ebd) ||
           (code == 0x1ebf) ||
           (code == 0x1ec1) ||
           (code == 0x1ec3) ||
           (code == 0x1ec5) ||
           (code == 0x1ec7) ||
           (code == 0x1ec9) ||
           (code == 0x1ecb) ||
           (code == 0x1ecd) ||
           (code == 0x1ecf) ||
           (code == 0x1ed1) ||
           (code == 0x1ed3) ||
           (code == 0x1ed5) ||
           (code == 0x1ed7) ||
           (code == 0x1ed9) ||
           (code == 0x1edb) ||
           (code == 0x1edd) ||
           (code == 0x1edf) ||
           (code == 0x1ee1) ||
           (code == 0x1ee3) ||
           (code == 0x1ee5) ||
           (code == 0x1ee7) ||
           (code == 0x1ee9) ||
           (code == 0x1eeb) ||
           (code == 0x1eed) ||
           (code == 0x1eef) ||
           (code == 0x1ef1) ||
           (code == 0x1ef3) ||
           (code == 0x1ef5) ||
           (code == 0x1ef7) ||
           (code == 0x1ef9) ||
           ((code >= 0x1f00) && (code <= 0x1f07)) ||
           ((code >= 0x1f10) && (code <= 0x1f15)) ||
           ((code >= 0x1f20) && (code <= 0x1f27)) ||
           ((code >= 0x1f30) && (code <= 0x1f37)) ||
           ((code >= 0x1f40) && (code <= 0x1f45)) ||
           ((code >= 0x1f50) && (code <= 0x1f57)) ||
           ((code >= 0x1f60) && (code <= 0x1f67)) ||
           ((code >= 0x1f70) && (code <= 0x1f7d)) ||
           ((code >= 0x1f80) && (code <= 0x1f87)) ||
           ((code >= 0x1f90) && (code <= 0x1f97)) ||
           ((code >= 0x1fa0) && (code <= 0x1fa7)) ||
           ((code >= 0x1fb0) && (code <= 0x1fb4)) ||
           ((code >= 0x1fb6) && (code <= 0x1fb7)) ||
           (code == 0x1fbe) ||
           ((code >= 0x1fc2) && (code <= 0x1fc4)) ||
           ((code >= 0x1fc6) && (code <= 0x1fc7)) ||
           ((code >= 0x1fd0) && (code <= 0x1fd3)) ||
           ((code >= 0x1fd6) && (code <= 0x1fd7)) ||
           ((code >= 0x1fe0) && (code <= 0x1fe7)) ||
           ((code >= 0x1ff2) && (code <= 0x1ff4)) ||
           ((code >= 0x1ff6) && (code <= 0x1ff7)) ||
           (code == 0x207f) ||
           (code == 0x210a) ||
           ((code >= 0x210e) && (code <= 0x210f)) ||
           (code == 0x2113) ||
           (code == 0x212f) ||
           (code == 0x2134) ||
           (code == 0x2139) ||
           ((code >= 0xfb00) && (code <= 0xfb06)) ||
           ((code >= 0xfb13) && (code <= 0xfb17)) ||
           ((code >= 0xff41) && (code <= 0xff5a)) ||
           ((code >= 0x10428) && (code <= 0x1044d)) ||
           ((code >= 0x1d41a) && (code <= 0x1d433)) ||
           ((code >= 0x1d44e) && (code <= 0x1d454)) ||
           ((code >= 0x1d456) && (code <= 0x1d467)) ||
           ((code >= 0x1d482) && (code <= 0x1d49b)) ||
           ((code >= 0x1d4b6) && (code <= 0x1d4b9)) ||
           (code == 0x1d4bb) ||
           ((code >= 0x1d4bd) && (code <= 0x1d4c0)) ||
           ((code >= 0x1d4c2) && (code <= 0x1d4c3)) ||
           ((code >= 0x1d4c5) && (code <= 0x1d4cf)) ||
           ((code >= 0x1d4ea) && (code <= 0x1d503)) ||
           ((code >= 0x1d51e) && (code <= 0x1d537)) ||
           ((code >= 0x1d552) && (code <= 0x1d56b)) ||
           ((code >= 0x1d586) && (code <= 0x1d59f)) ||
           ((code >= 0x1d5ba) && (code <= 0x1d5d3)) ||
           ((code >= 0x1d5ee) && (code <= 0x1d607)) ||
           ((code >= 0x1d622) && (code <= 0x1d63b)) ||
           ((code >= 0x1d656) && (code <= 0x1d66f)) ||
           ((code >= 0x1d68a) && (code <= 0x1d6a3)) ||
           ((code >= 0x1d6c2) && (code <= 0x1d6da)) ||
           ((code >= 0x1d6dc) && (code <= 0x1d6e1)) ||
           ((code >= 0x1d6fc) && (code <= 0x1d714)) ||
           ((code >= 0x1d716) && (code <= 0x1d71b)) ||
           ((code >= 0x1d736) && (code <= 0x1d74e)) ||
           ((code >= 0x1d750) && (code <= 0x1d755)) ||
           ((code >= 0x1d770) && (code <= 0x1d788)) ||
           ((code >= 0x1d78a) && (code <= 0x1d78f)) ||
           ((code >= 0x1d7aa) && (code <= 0x1d7c2)) ||
           ((code >= 0x1d7c4) && (code <= 0x1d7c9)));
}

/*
 * rtxUCSIsCatLm:
 * @code: UCS code point
 *
 * Check whether the character is part of Lm UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatLm (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x2b0) && (code <= 0x2b8)) ||
           ((code >= 0x2bb) && (code <= 0x2c1)) ||
           ((code >= 0x2d0) && (code <= 0x2d1)) ||
           ((code >= 0x2e0) && (code <= 0x2e4)) ||
           (code == 0x2ee) ||
           (code == 0x37a) ||
           (code == 0x559) ||
           (code == 0x640) ||
           ((code >= 0x6e5) && (code <= 0x6e6)) ||
           (code == 0xe46) ||
           (code == 0xec6) ||
           (code == 0x1843) ||
           (code == 0x3005) ||
           ((code >= 0x3031) && (code <= 0x3035)) ||
           ((code >= 0x309d) && (code <= 0x309e)) ||
           ((code >= 0x30fc) && (code <= 0x30fe)) ||
           (code == 0xff70) ||
           ((code >= 0xff9e) && (code <= 0xff9f)));
}

/*
 * rtxUCSIsCatLo:
 * @code: UCS code point
 *
 * Check whether the character is part of Lo UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatLo (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x1bb) ||
           ((code >= 0x1c0) && (code <= 0x1c3)) ||
           ((code >= 0x5d0) && (code <= 0x5ea)) ||
           ((code >= 0x5f0) && (code <= 0x5f2)) ||
           ((code >= 0x621) && (code <= 0x63a)) ||
           ((code >= 0x641) && (code <= 0x64a)) ||
           ((code >= 0x671) && (code <= 0x6d3)) ||
           (code == 0x6d5) ||
           ((code >= 0x6fa) && (code <= 0x6fc)) ||
           (code == 0x710) ||
           ((code >= 0x712) && (code <= 0x72c)) ||
           ((code >= 0x780) && (code <= 0x7a5)) ||
           ((code >= 0x905) && (code <= 0x939)) ||
           (code == 0x93d) ||
           (code == 0x950) ||
           ((code >= 0x958) && (code <= 0x961)) ||
           ((code >= 0x985) && (code <= 0x98c)) ||
           ((code >= 0x98f) && (code <= 0x990)) ||
           ((code >= 0x993) && (code <= 0x9a8)) ||
           ((code >= 0x9aa) && (code <= 0x9b0)) ||
           (code == 0x9b2) ||
           ((code >= 0x9b6) && (code <= 0x9b9)) ||
           ((code >= 0x9dc) && (code <= 0x9dd)) ||
           ((code >= 0x9df) && (code <= 0x9e1)) ||
           ((code >= 0x9f0) && (code <= 0x9f1)) ||
           ((code >= 0xa05) && (code <= 0xa0a)) ||
           ((code >= 0xa0f) && (code <= 0xa10)) ||
           ((code >= 0xa13) && (code <= 0xa28)) ||
           ((code >= 0xa2a) && (code <= 0xa30)) ||
           ((code >= 0xa32) && (code <= 0xa33)) ||
           ((code >= 0xa35) && (code <= 0xa36)) ||
           ((code >= 0xa38) && (code <= 0xa39)) ||
           ((code >= 0xa59) && (code <= 0xa5c)) ||
           (code == 0xa5e) ||
           ((code >= 0xa72) && (code <= 0xa74)) ||
           ((code >= 0xa85) && (code <= 0xa8b)) ||
           (code == 0xa8d) ||
           ((code >= 0xa8f) && (code <= 0xa91)) ||
           ((code >= 0xa93) && (code <= 0xaa8)) ||
           ((code >= 0xaaa) && (code <= 0xab0)) ||
           ((code >= 0xab2) && (code <= 0xab3)) ||
           ((code >= 0xab5) && (code <= 0xab9)) ||
           (code == 0xabd) ||
           (code == 0xad0) ||
           (code == 0xae0) ||
           ((code >= 0xb05) && (code <= 0xb0c)) ||
           ((code >= 0xb0f) && (code <= 0xb10)) ||
           ((code >= 0xb13) && (code <= 0xb28)) ||
           ((code >= 0xb2a) && (code <= 0xb30)) ||
           ((code >= 0xb32) && (code <= 0xb33)) ||
           ((code >= 0xb36) && (code <= 0xb39)) ||
           (code == 0xb3d) ||
           ((code >= 0xb5c) && (code <= 0xb5d)) ||
           ((code >= 0xb5f) && (code <= 0xb61)) ||
           ((code >= 0xb85) && (code <= 0xb8a)) ||
           ((code >= 0xb8e) && (code <= 0xb90)) ||
           ((code >= 0xb92) && (code <= 0xb95)) ||
           ((code >= 0xb99) && (code <= 0xb9a)) ||
           (code == 0xb9c) ||
           ((code >= 0xb9e) && (code <= 0xb9f)) ||
           ((code >= 0xba3) && (code <= 0xba4)) ||
           ((code >= 0xba8) && (code <= 0xbaa)) ||
           ((code >= 0xbae) && (code <= 0xbb5)) ||
           ((code >= 0xbb7) && (code <= 0xbb9)) ||
           ((code >= 0xc05) && (code <= 0xc0c)) ||
           ((code >= 0xc0e) && (code <= 0xc10)) ||
           ((code >= 0xc12) && (code <= 0xc28)) ||
           ((code >= 0xc2a) && (code <= 0xc33)) ||
           ((code >= 0xc35) && (code <= 0xc39)) ||
           ((code >= 0xc60) && (code <= 0xc61)) ||
           ((code >= 0xc85) && (code <= 0xc8c)) ||
           ((code >= 0xc8e) && (code <= 0xc90)) ||
           ((code >= 0xc92) && (code <= 0xca8)) ||
           ((code >= 0xcaa) && (code <= 0xcb3)) ||
           ((code >= 0xcb5) && (code <= 0xcb9)) ||
           (code == 0xcde) ||
           ((code >= 0xce0) && (code <= 0xce1)) ||
           ((code >= 0xd05) && (code <= 0xd0c)) ||
           ((code >= 0xd0e) && (code <= 0xd10)) ||
           ((code >= 0xd12) && (code <= 0xd28)) ||
           ((code >= 0xd2a) && (code <= 0xd39)) ||
           ((code >= 0xd60) && (code <= 0xd61)) ||
           ((code >= 0xd85) && (code <= 0xd96)) ||
           ((code >= 0xd9a) && (code <= 0xdb1)) ||
           ((code >= 0xdb3) && (code <= 0xdbb)) ||
           (code == 0xdbd) ||
           ((code >= 0xdc0) && (code <= 0xdc6)) ||
           ((code >= 0xe01) && (code <= 0xe30)) ||
           ((code >= 0xe32) && (code <= 0xe33)) ||
           ((code >= 0xe40) && (code <= 0xe45)) ||
           ((code >= 0xe81) && (code <= 0xe82)) ||
           (code == 0xe84) ||
           ((code >= 0xe87) && (code <= 0xe88)) ||
           (code == 0xe8a) ||
           (code == 0xe8d) ||
           ((code >= 0xe94) && (code <= 0xe97)) ||
           ((code >= 0xe99) && (code <= 0xe9f)) ||
           ((code >= 0xea1) && (code <= 0xea3)) ||
           (code == 0xea5) ||
           (code == 0xea7) ||
           ((code >= 0xeaa) && (code <= 0xeab)) ||
           ((code >= 0xead) && (code <= 0xeb0)) ||
           ((code >= 0xeb2) && (code <= 0xeb3)) ||
           (code == 0xebd) ||
           ((code >= 0xec0) && (code <= 0xec4)) ||
           ((code >= 0xedc) && (code <= 0xedd)) ||
           (code == 0xf00) ||
           ((code >= 0xf40) && (code <= 0xf47)) ||
           ((code >= 0xf49) && (code <= 0xf6a)) ||
           ((code >= 0xf88) && (code <= 0xf8b)) ||
           ((code >= 0x1000) && (code <= 0x1021)) ||
           ((code >= 0x1023) && (code <= 0x1027)) ||
           ((code >= 0x1029) && (code <= 0x102a)) ||
           ((code >= 0x1050) && (code <= 0x1055)) ||
           ((code >= 0x10d0) && (code <= 0x10f6)) ||
           ((code >= 0x1100) && (code <= 0x1159)) ||
           ((code >= 0x115f) && (code <= 0x11a2)) ||
           ((code >= 0x11a8) && (code <= 0x11f9)) ||
           ((code >= 0x1200) && (code <= 0x1206)) ||
           ((code >= 0x1208) && (code <= 0x1246)) ||
           (code == 0x1248) ||
           ((code >= 0x124a) && (code <= 0x124d)) ||
           ((code >= 0x1250) && (code <= 0x1256)) ||
           (code == 0x1258) ||
           ((code >= 0x125a) && (code <= 0x125d)) ||
           ((code >= 0x1260) && (code <= 0x1286)) ||
           (code == 0x1288) ||
           ((code >= 0x128a) && (code <= 0x128d)) ||
           ((code >= 0x1290) && (code <= 0x12ae)) ||
           (code == 0x12b0) ||
           ((code >= 0x12b2) && (code <= 0x12b5)) ||
           ((code >= 0x12b8) && (code <= 0x12be)) ||
           (code == 0x12c0) ||
           ((code >= 0x12c2) && (code <= 0x12c5)) ||
           ((code >= 0x12c8) && (code <= 0x12ce)) ||
           ((code >= 0x12d0) && (code <= 0x12d6)) ||
           ((code >= 0x12d8) && (code <= 0x12ee)) ||
           ((code >= 0x12f0) && (code <= 0x130e)) ||
           (code == 0x1310) ||
           ((code >= 0x1312) && (code <= 0x1315)) ||
           ((code >= 0x1318) && (code <= 0x131e)) ||
           ((code >= 0x1320) && (code <= 0x1346)) ||
           ((code >= 0x1348) && (code <= 0x135a)) ||
           ((code >= 0x13a0) && (code <= 0x13f4)) ||
           ((code >= 0x1401) && (code <= 0x166c)) ||
           ((code >= 0x166f) && (code <= 0x1676)) ||
           ((code >= 0x1681) && (code <= 0x169a)) ||
           ((code >= 0x16a0) && (code <= 0x16ea)) ||
           ((code >= 0x1780) && (code <= 0x17b3)) ||
           ((code >= 0x1820) && (code <= 0x1842)) ||
           ((code >= 0x1844) && (code <= 0x1877)) ||
           ((code >= 0x1880) && (code <= 0x18a8)) ||
           ((code >= 0x2135) && (code <= 0x2138)) ||
           (code == 0x3006) ||
           ((code >= 0x3041) && (code <= 0x3094)) ||
           ((code >= 0x30a1) && (code <= 0x30fa)) ||
           ((code >= 0x3105) && (code <= 0x312c)) ||
           ((code >= 0x3131) && (code <= 0x318e)) ||
           ((code >= 0x31a0) && (code <= 0x31b7)) ||
           (code == 0x3400) ||
           (code == 0x4db5) ||
           (code == 0x4e00) ||
           (code == 0x9fa5) ||
           ((code >= 0xa000) && (code <= 0xa48c)) ||
           (code == 0xac00) ||
           (code == 0xd7a3) ||
           ((code >= 0xf900) && (code <= 0xfa2d)) ||
           (code == 0xfb1d) ||
           ((code >= 0xfb1f) && (code <= 0xfb28)) ||
           ((code >= 0xfb2a) && (code <= 0xfb36)) ||
           ((code >= 0xfb38) && (code <= 0xfb3c)) ||
           (code == 0xfb3e) ||
           ((code >= 0xfb40) && (code <= 0xfb41)) ||
           ((code >= 0xfb43) && (code <= 0xfb44)) ||
           ((code >= 0xfb46) && (code <= 0xfbb1)) ||
           ((code >= 0xfbd3) && (code <= 0xfd3d)) ||
           ((code >= 0xfd50) && (code <= 0xfd8f)) ||
           ((code >= 0xfd92) && (code <= 0xfdc7)) ||
           ((code >= 0xfdf0) && (code <= 0xfdfb)) ||
           ((code >= 0xfe70) && (code <= 0xfe72)) ||
           (code == 0xfe74) ||
           ((code >= 0xfe76) && (code <= 0xfefc)) ||
           ((code >= 0xff66) && (code <= 0xff6f)) ||
           ((code >= 0xff71) && (code <= 0xff9d)) ||
           ((code >= 0xffa0) && (code <= 0xffbe)) ||
           ((code >= 0xffc2) && (code <= 0xffc7)) ||
           ((code >= 0xffca) && (code <= 0xffcf)) ||
           ((code >= 0xffd2) && (code <= 0xffd7)) ||
           ((code >= 0xffda) && (code <= 0xffdc)) ||
           ((code >= 0x10300) && (code <= 0x1031e)) ||
           ((code >= 0x10330) && (code <= 0x10349)) ||
           (code == 0x20000) ||
           (code == 0x2a6d6) ||
           ((code >= 0x2f800) && (code <= 0x2fa1d)));
}

/*
 * rtxUCSIsCatLt:
 * @code: UCS code point
 *
 * Check whether the character is part of Lt UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatLt (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x1c5) ||
           (code == 0x1c8) ||
           (code == 0x1cb) ||
           (code == 0x1f2) ||
           ((code >= 0x1f88) && (code <= 0x1f8f)) ||
           ((code >= 0x1f98) && (code <= 0x1f9f)) ||
           ((code >= 0x1fa8) && (code <= 0x1faf)) ||
           (code == 0x1fbc) ||
           (code == 0x1fcc) ||
           (code == 0x1ffc));
}

/*
 * rtxUCSIsCatLu:
 * @code: UCS code point
 *
 * Check whether the character is part of Lu UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatLu (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x41) && (code <= 0x5a)) ||
           ((code >= 0xc0) && (code <= 0xd6)) ||
           ((code >= 0xd8) && (code <= 0xde)) ||
           (code == 0x100) ||
           (code == 0x102) ||
           (code == 0x104) ||
           (code == 0x106) ||
           (code == 0x108) ||
           (code == 0x10a) ||
           (code == 0x10c) ||
           (code == 0x10e) ||
           (code == 0x110) ||
           (code == 0x112) ||
           (code == 0x114) ||
           (code == 0x116) ||
           (code == 0x118) ||
           (code == 0x11a) ||
           (code == 0x11c) ||
           (code == 0x11e) ||
           (code == 0x120) ||
           (code == 0x122) ||
           (code == 0x124) ||
           (code == 0x126) ||
           (code == 0x128) ||
           (code == 0x12a) ||
           (code == 0x12c) ||
           (code == 0x12e) ||
           (code == 0x130) ||
           (code == 0x132) ||
           (code == 0x134) ||
           (code == 0x136) ||
           (code == 0x139) ||
           (code == 0x13b) ||
           (code == 0x13d) ||
           (code == 0x13f) ||
           (code == 0x141) ||
           (code == 0x143) ||
           (code == 0x145) ||
           (code == 0x147) ||
           (code == 0x14a) ||
           (code == 0x14c) ||
           (code == 0x14e) ||
           (code == 0x150) ||
           (code == 0x152) ||
           (code == 0x154) ||
           (code == 0x156) ||
           (code == 0x158) ||
           (code == 0x15a) ||
           (code == 0x15c) ||
           (code == 0x15e) ||
           (code == 0x160) ||
           (code == 0x162) ||
           (code == 0x164) ||
           (code == 0x166) ||
           (code == 0x168) ||
           (code == 0x16a) ||
           (code == 0x16c) ||
           (code == 0x16e) ||
           (code == 0x170) ||
           (code == 0x172) ||
           (code == 0x174) ||
           (code == 0x176) ||
           ((code >= 0x178) && (code <= 0x179)) ||
           (code == 0x17b) ||
           (code == 0x17d) ||
           ((code >= 0x181) && (code <= 0x182)) ||
           (code == 0x184) ||
           ((code >= 0x186) && (code <= 0x187)) ||
           ((code >= 0x189) && (code <= 0x18b)) ||
           ((code >= 0x18e) && (code <= 0x191)) ||
           ((code >= 0x193) && (code <= 0x194)) ||
           ((code >= 0x196) && (code <= 0x198)) ||
           ((code >= 0x19c) && (code <= 0x19d)) ||
           ((code >= 0x19f) && (code <= 0x1a0)) ||
           (code == 0x1a2) ||
           (code == 0x1a4) ||
           ((code >= 0x1a6) && (code <= 0x1a7)) ||
           (code == 0x1a9) ||
           (code == 0x1ac) ||
           ((code >= 0x1ae) && (code <= 0x1af)) ||
           ((code >= 0x1b1) && (code <= 0x1b3)) ||
           (code == 0x1b5) ||
           ((code >= 0x1b7) && (code <= 0x1b8)) ||
           (code == 0x1bc) ||
           (code == 0x1c4) ||
           (code == 0x1c7) ||
           (code == 0x1ca) ||
           (code == 0x1cd) ||
           (code == 0x1cf) ||
           (code == 0x1d1) ||
           (code == 0x1d3) ||
           (code == 0x1d5) ||
           (code == 0x1d7) ||
           (code == 0x1d9) ||
           (code == 0x1db) ||
           (code == 0x1de) ||
           (code == 0x1e0) ||
           (code == 0x1e2) ||
           (code == 0x1e4) ||
           (code == 0x1e6) ||
           (code == 0x1e8) ||
           (code == 0x1ea) ||
           (code == 0x1ec) ||
           (code == 0x1ee) ||
           (code == 0x1f1) ||
           (code == 0x1f4) ||
           ((code >= 0x1f6) && (code <= 0x1f8)) ||
           (code == 0x1fa) ||
           (code == 0x1fc) ||
           (code == 0x1fe) ||
           (code == 0x200) ||
           (code == 0x202) ||
           (code == 0x204) ||
           (code == 0x206) ||
           (code == 0x208) ||
           (code == 0x20a) ||
           (code == 0x20c) ||
           (code == 0x20e) ||
           (code == 0x210) ||
           (code == 0x212) ||
           (code == 0x214) ||
           (code == 0x216) ||
           (code == 0x218) ||
           (code == 0x21a) ||
           (code == 0x21c) ||
           (code == 0x21e) ||
           (code == 0x222) ||
           (code == 0x224) ||
           (code == 0x226) ||
           (code == 0x228) ||
           (code == 0x22a) ||
           (code == 0x22c) ||
           (code == 0x22e) ||
           (code == 0x230) ||
           (code == 0x232) ||
           (code == 0x386) ||
           ((code >= 0x388) && (code <= 0x38a)) ||
           (code == 0x38c) ||
           ((code >= 0x38e) && (code <= 0x38f)) ||
           ((code >= 0x391) && (code <= 0x3a1)) ||
           ((code >= 0x3a3) && (code <= 0x3ab)) ||
           ((code >= 0x3d2) && (code <= 0x3d4)) ||
           (code == 0x3da) ||
           (code == 0x3dc) ||
           (code == 0x3de) ||
           (code == 0x3e0) ||
           (code == 0x3e2) ||
           (code == 0x3e4) ||
           (code == 0x3e6) ||
           (code == 0x3e8) ||
           (code == 0x3ea) ||
           (code == 0x3ec) ||
           (code == 0x3ee) ||
           (code == 0x3f4) ||
           ((code >= 0x400) && (code <= 0x42f)) ||
           (code == 0x460) ||
           (code == 0x462) ||
           (code == 0x464) ||
           (code == 0x466) ||
           (code == 0x468) ||
           (code == 0x46a) ||
           (code == 0x46c) ||
           (code == 0x46e) ||
           (code == 0x470) ||
           (code == 0x472) ||
           (code == 0x474) ||
           (code == 0x476) ||
           (code == 0x478) ||
           (code == 0x47a) ||
           (code == 0x47c) ||
           (code == 0x47e) ||
           (code == 0x480) ||
           (code == 0x48c) ||
           (code == 0x48e) ||
           (code == 0x490) ||
           (code == 0x492) ||
           (code == 0x494) ||
           (code == 0x496) ||
           (code == 0x498) ||
           (code == 0x49a) ||
           (code == 0x49c) ||
           (code == 0x49e) ||
           (code == 0x4a0) ||
           (code == 0x4a2) ||
           (code == 0x4a4) ||
           (code == 0x4a6) ||
           (code == 0x4a8) ||
           (code == 0x4aa) ||
           (code == 0x4ac) ||
           (code == 0x4ae) ||
           (code == 0x4b0) ||
           (code == 0x4b2) ||
           (code == 0x4b4) ||
           (code == 0x4b6) ||
           (code == 0x4b8) ||
           (code == 0x4ba) ||
           (code == 0x4bc) ||
           (code == 0x4be) ||
           ((code >= 0x4c0) && (code <= 0x4c1)) ||
           (code == 0x4c3) ||
           (code == 0x4c7) ||
           (code == 0x4cb) ||
           (code == 0x4d0) ||
           (code == 0x4d2) ||
           (code == 0x4d4) ||
           (code == 0x4d6) ||
           (code == 0x4d8) ||
           (code == 0x4da) ||
           (code == 0x4dc) ||
           (code == 0x4de) ||
           (code == 0x4e0) ||
           (code == 0x4e2) ||
           (code == 0x4e4) ||
           (code == 0x4e6) ||
           (code == 0x4e8) ||
           (code == 0x4ea) ||
           (code == 0x4ec) ||
           (code == 0x4ee) ||
           (code == 0x4f0) ||
           (code == 0x4f2) ||
           (code == 0x4f4) ||
           (code == 0x4f8) ||
           ((code >= 0x531) && (code <= 0x556)) ||
           ((code >= 0x10a0) && (code <= 0x10c5)) ||
           (code == 0x1e00) ||
           (code == 0x1e02) ||
           (code == 0x1e04) ||
           (code == 0x1e06) ||
           (code == 0x1e08) ||
           (code == 0x1e0a) ||
           (code == 0x1e0c) ||
           (code == 0x1e0e) ||
           (code == 0x1e10) ||
           (code == 0x1e12) ||
           (code == 0x1e14) ||
           (code == 0x1e16) ||
           (code == 0x1e18) ||
           (code == 0x1e1a) ||
           (code == 0x1e1c) ||
           (code == 0x1e1e) ||
           (code == 0x1e20) ||
           (code == 0x1e22) ||
           (code == 0x1e24) ||
           (code == 0x1e26) ||
           (code == 0x1e28) ||
           (code == 0x1e2a) ||
           (code == 0x1e2c) ||
           (code == 0x1e2e) ||
           (code == 0x1e30) ||
           (code == 0x1e32) ||
           (code == 0x1e34) ||
           (code == 0x1e36) ||
           (code == 0x1e38) ||
           (code == 0x1e3a) ||
           (code == 0x1e3c) ||
           (code == 0x1e3e) ||
           (code == 0x1e40) ||
           (code == 0x1e42) ||
           (code == 0x1e44) ||
           (code == 0x1e46) ||
           (code == 0x1e48) ||
           (code == 0x1e4a) ||
           (code == 0x1e4c) ||
           (code == 0x1e4e) ||
           (code == 0x1e50) ||
           (code == 0x1e52) ||
           (code == 0x1e54) ||
           (code == 0x1e56) ||
           (code == 0x1e58) ||
           (code == 0x1e5a) ||
           (code == 0x1e5c) ||
           (code == 0x1e5e) ||
           (code == 0x1e60) ||
           (code == 0x1e62) ||
           (code == 0x1e64) ||
           (code == 0x1e66) ||
           (code == 0x1e68) ||
           (code == 0x1e6a) ||
           (code == 0x1e6c) ||
           (code == 0x1e6e) ||
           (code == 0x1e70) ||
           (code == 0x1e72) ||
           (code == 0x1e74) ||
           (code == 0x1e76) ||
           (code == 0x1e78) ||
           (code == 0x1e7a) ||
           (code == 0x1e7c) ||
           (code == 0x1e7e) ||
           (code == 0x1e80) ||
           (code == 0x1e82) ||
           (code == 0x1e84) ||
           (code == 0x1e86) ||
           (code == 0x1e88) ||
           (code == 0x1e8a) ||
           (code == 0x1e8c) ||
           (code == 0x1e8e) ||
           (code == 0x1e90) ||
           (code == 0x1e92) ||
           (code == 0x1e94) ||
           (code == 0x1ea0) ||
           (code == 0x1ea2) ||
           (code == 0x1ea4) ||
           (code == 0x1ea6) ||
           (code == 0x1ea8) ||
           (code == 0x1eaa) ||
           (code == 0x1eac) ||
           (code == 0x1eae) ||
           (code == 0x1eb0) ||
           (code == 0x1eb2) ||
           (code == 0x1eb4) ||
           (code == 0x1eb6) ||
           (code == 0x1eb8) ||
           (code == 0x1eba) ||
           (code == 0x1ebc) ||
           (code == 0x1ebe) ||
           (code == 0x1ec0) ||
           (code == 0x1ec2) ||
           (code == 0x1ec4) ||
           (code == 0x1ec6) ||
           (code == 0x1ec8) ||
           (code == 0x1eca) ||
           (code == 0x1ecc) ||
           (code == 0x1ece) ||
           (code == 0x1ed0) ||
           (code == 0x1ed2) ||
           (code == 0x1ed4) ||
           (code == 0x1ed6) ||
           (code == 0x1ed8) ||
           (code == 0x1eda) ||
           (code == 0x1edc) ||
           (code == 0x1ede) ||
           (code == 0x1ee0) ||
           (code == 0x1ee2) ||
           (code == 0x1ee4) ||
           (code == 0x1ee6) ||
           (code == 0x1ee8) ||
           (code == 0x1eea) ||
           (code == 0x1eec) ||
           (code == 0x1eee) ||
           (code == 0x1ef0) ||
           (code == 0x1ef2) ||
           (code == 0x1ef4) ||
           (code == 0x1ef6) ||
           (code == 0x1ef8) ||
           ((code >= 0x1f08) && (code <= 0x1f0f)) ||
           ((code >= 0x1f18) && (code <= 0x1f1d)) ||
           ((code >= 0x1f28) && (code <= 0x1f2f)) ||
           ((code >= 0x1f38) && (code <= 0x1f3f)) ||
           ((code >= 0x1f48) && (code <= 0x1f4d)) ||
           (code == 0x1f59) ||
           (code == 0x1f5b) ||
           (code == 0x1f5d) ||
           (code == 0x1f5f) ||
           ((code >= 0x1f68) && (code <= 0x1f6f)) ||
           ((code >= 0x1fb8) && (code <= 0x1fbb)) ||
           ((code >= 0x1fc8) && (code <= 0x1fcb)) ||
           ((code >= 0x1fd8) && (code <= 0x1fdb)) ||
           ((code >= 0x1fe8) && (code <= 0x1fec)) ||
           ((code >= 0x1ff8) && (code <= 0x1ffb)) ||
           (code == 0x2102) ||
           (code == 0x2107) ||
           ((code >= 0x210b) && (code <= 0x210d)) ||
           ((code >= 0x2110) && (code <= 0x2112)) ||
           (code == 0x2115) ||
           ((code >= 0x2119) && (code <= 0x211d)) ||
           (code == 0x2124) ||
           (code == 0x2126) ||
           (code == 0x2128) ||
           ((code >= 0x212a) && (code <= 0x212d)) ||
           ((code >= 0x2130) && (code <= 0x2131)) ||
           (code == 0x2133) ||
           ((code >= 0xff21) && (code <= 0xff3a)) ||
           ((code >= 0x10400) && (code <= 0x10425)) ||
           ((code >= 0x1d400) && (code <= 0x1d419)) ||
           ((code >= 0x1d434) && (code <= 0x1d44d)) ||
           ((code >= 0x1d468) && (code <= 0x1d481)) ||
           (code == 0x1d49c) ||
           ((code >= 0x1d49e) && (code <= 0x1d49f)) ||
           (code == 0x1d4a2) ||
           ((code >= 0x1d4a5) && (code <= 0x1d4a6)) ||
           ((code >= 0x1d4a9) && (code <= 0x1d4ac)) ||
           ((code >= 0x1d4ae) && (code <= 0x1d4b5)) ||
           ((code >= 0x1d4d0) && (code <= 0x1d4e9)) ||
           ((code >= 0x1d504) && (code <= 0x1d505)) ||
           ((code >= 0x1d507) && (code <= 0x1d50a)) ||
           ((code >= 0x1d50d) && (code <= 0x1d514)) ||
           ((code >= 0x1d516) && (code <= 0x1d51c)) ||
           ((code >= 0x1d538) && (code <= 0x1d539)) ||
           ((code >= 0x1d53b) && (code <= 0x1d53e)) ||
           ((code >= 0x1d540) && (code <= 0x1d544)) ||
           (code == 0x1d546) ||
           ((code >= 0x1d54a) && (code <= 0x1d550)) ||
           ((code >= 0x1d56c) && (code <= 0x1d585)) ||
           ((code >= 0x1d5a0) && (code <= 0x1d5b9)) ||
           ((code >= 0x1d5d4) && (code <= 0x1d5ed)) ||
           ((code >= 0x1d608) && (code <= 0x1d621)) ||
           ((code >= 0x1d63c) && (code <= 0x1d655)) ||
           ((code >= 0x1d670) && (code <= 0x1d689)) ||
           ((code >= 0x1d6a8) && (code <= 0x1d6c0)) ||
           ((code >= 0x1d6e2) && (code <= 0x1d6fa)) ||
           ((code >= 0x1d71c) && (code <= 0x1d734)) ||
           ((code >= 0x1d756) && (code <= 0x1d76e)) ||
           ((code >= 0x1d790) && (code <= 0x1d7a8)));
}

/*
 * rtxUCSIsCatM:
 * @code: UCS code point
 *
 * Check whether the character is part of M UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatM (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x300) && (code <= 0x34e)) ||
           ((code >= 0x360) && (code <= 0x362)) ||
           ((code >= 0x483) && (code <= 0x486)) ||
           ((code >= 0x488) && (code <= 0x489)) ||
           ((code >= 0x591) && (code <= 0x5a1)) ||
           ((code >= 0x5a3) && (code <= 0x5b9)) ||
           ((code >= 0x5bb) && (code <= 0x5bd)) ||
           (code == 0x5bf) ||
           ((code >= 0x5c1) && (code <= 0x5c2)) ||
           (code == 0x5c4) ||
           ((code >= 0x64b) && (code <= 0x655)) ||
           (code == 0x670) ||
           ((code >= 0x6d6) && (code <= 0x6e4)) ||
           ((code >= 0x6e7) && (code <= 0x6e8)) ||
           ((code >= 0x6ea) && (code <= 0x6ed)) ||
           (code == 0x711) ||
           ((code >= 0x730) && (code <= 0x74a)) ||
           ((code >= 0x7a6) && (code <= 0x7b0)) ||
           ((code >= 0x901) && (code <= 0x903)) ||
           (code == 0x93c) ||
           ((code >= 0x93e) && (code <= 0x94d)) ||
           ((code >= 0x951) && (code <= 0x954)) ||
           ((code >= 0x962) && (code <= 0x963)) ||
           ((code >= 0x981) && (code <= 0x983)) ||
           (code == 0x9bc) ||
           ((code >= 0x9be) && (code <= 0x9c4)) ||
           ((code >= 0x9c7) && (code <= 0x9c8)) ||
           ((code >= 0x9cb) && (code <= 0x9cd)) ||
           (code == 0x9d7) ||
           ((code >= 0x9e2) && (code <= 0x9e3)) ||
           (code == 0xa02) ||
           (code == 0xa3c) ||
           ((code >= 0xa3e) && (code <= 0xa42)) ||
           ((code >= 0xa47) && (code <= 0xa48)) ||
           ((code >= 0xa4b) && (code <= 0xa4d)) ||
           ((code >= 0xa70) && (code <= 0xa71)) ||
           ((code >= 0xa81) && (code <= 0xa83)) ||
           (code == 0xabc) ||
           ((code >= 0xabe) && (code <= 0xac5)) ||
           ((code >= 0xac7) && (code <= 0xac9)) ||
           ((code >= 0xacb) && (code <= 0xacd)) ||
           ((code >= 0xb01) && (code <= 0xb03)) ||
           (code == 0xb3c) ||
           ((code >= 0xb3e) && (code <= 0xb43)) ||
           ((code >= 0xb47) && (code <= 0xb48)) ||
           ((code >= 0xb4b) && (code <= 0xb4d)) ||
           ((code >= 0xb56) && (code <= 0xb57)) ||
           ((code >= 0xb82) && (code <= 0xb83)) ||
           ((code >= 0xbbe) && (code <= 0xbc2)) ||
           ((code >= 0xbc6) && (code <= 0xbc8)) ||
           ((code >= 0xbca) && (code <= 0xbcd)) ||
           (code == 0xbd7) ||
           ((code >= 0xc01) && (code <= 0xc03)) ||
           ((code >= 0xc3e) && (code <= 0xc44)) ||
           ((code >= 0xc46) && (code <= 0xc48)) ||
           ((code >= 0xc4a) && (code <= 0xc4d)) ||
           ((code >= 0xc55) && (code <= 0xc56)) ||
           ((code >= 0xc82) && (code <= 0xc83)) ||
           ((code >= 0xcbe) && (code <= 0xcc4)) ||
           ((code >= 0xcc6) && (code <= 0xcc8)) ||
           ((code >= 0xcca) && (code <= 0xccd)) ||
           ((code >= 0xcd5) && (code <= 0xcd6)) ||
           ((code >= 0xd02) && (code <= 0xd03)) ||
           ((code >= 0xd3e) && (code <= 0xd43)) ||
           ((code >= 0xd46) && (code <= 0xd48)) ||
           ((code >= 0xd4a) && (code <= 0xd4d)) ||
           (code == 0xd57) ||
           ((code >= 0xd82) && (code <= 0xd83)) ||
           (code == 0xdca) ||
           ((code >= 0xdcf) && (code <= 0xdd4)) ||
           (code == 0xdd6) ||
           ((code >= 0xdd8) && (code <= 0xddf)) ||
           ((code >= 0xdf2) && (code <= 0xdf3)) ||
           (code == 0xe31) ||
           ((code >= 0xe34) && (code <= 0xe3a)) ||
           ((code >= 0xe47) && (code <= 0xe4e)) ||
           (code == 0xeb1) ||
           ((code >= 0xeb4) && (code <= 0xeb9)) ||
           ((code >= 0xebb) && (code <= 0xebc)) ||
           ((code >= 0xec8) && (code <= 0xecd)) ||
           ((code >= 0xf18) && (code <= 0xf19)) ||
           (code == 0xf35) ||
           (code == 0xf37) ||
           (code == 0xf39) ||
           ((code >= 0xf3e) && (code <= 0xf3f)) ||
           ((code >= 0xf71) && (code <= 0xf84)) ||
           ((code >= 0xf86) && (code <= 0xf87)) ||
           ((code >= 0xf90) && (code <= 0xf97)) ||
           ((code >= 0xf99) && (code <= 0xfbc)) ||
           (code == 0xfc6) ||
           ((code >= 0x102c) && (code <= 0x1032)) ||
           ((code >= 0x1036) && (code <= 0x1039)) ||
           ((code >= 0x1056) && (code <= 0x1059)) ||
           ((code >= 0x17b4) && (code <= 0x17d3)) ||
           (code == 0x18a9) ||
           ((code >= 0x20d0) && (code <= 0x20e3)) ||
           ((code >= 0x302a) && (code <= 0x302f)) ||
           ((code >= 0x3099) && (code <= 0x309a)) ||
           (code == 0xfb1e) ||
           ((code >= 0xfe20) && (code <= 0xfe23)) ||
           ((code >= 0x1d165) && (code <= 0x1d169)) ||
           ((code >= 0x1d16d) && (code <= 0x1d172)) ||
           ((code >= 0x1d17b) && (code <= 0x1d182)) ||
           ((code >= 0x1d185) && (code <= 0x1d18b)) ||
           ((code >= 0x1d1aa) && (code <= 0x1d1ad)));
}

/*
 * rtxUCSIsCatMc:
 * @code: UCS code point
 *
 * Check whether the character is part of Mc UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatMc (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x903) ||
           ((code >= 0x93e) && (code <= 0x940)) ||
           ((code >= 0x949) && (code <= 0x94c)) ||
           ((code >= 0x982) && (code <= 0x983)) ||
           ((code >= 0x9be) && (code <= 0x9c0)) ||
           ((code >= 0x9c7) && (code <= 0x9c8)) ||
           ((code >= 0x9cb) && (code <= 0x9cc)) ||
           (code == 0x9d7) ||
           ((code >= 0xa3e) && (code <= 0xa40)) ||
           (code == 0xa83) ||
           ((code >= 0xabe) && (code <= 0xac0)) ||
           (code == 0xac9) ||
           ((code >= 0xacb) && (code <= 0xacc)) ||
           ((code >= 0xb02) && (code <= 0xb03)) ||
           (code == 0xb3e) ||
           (code == 0xb40) ||
           ((code >= 0xb47) && (code <= 0xb48)) ||
           ((code >= 0xb4b) && (code <= 0xb4c)) ||
           (code == 0xb57) ||
           (code == 0xb83) ||
           ((code >= 0xbbe) && (code <= 0xbbf)) ||
           ((code >= 0xbc1) && (code <= 0xbc2)) ||
           ((code >= 0xbc6) && (code <= 0xbc8)) ||
           ((code >= 0xbca) && (code <= 0xbcc)) ||
           (code == 0xbd7) ||
           ((code >= 0xc01) && (code <= 0xc03)) ||
           ((code >= 0xc41) && (code <= 0xc44)) ||
           ((code >= 0xc82) && (code <= 0xc83)) ||
           (code == 0xcbe) ||
           ((code >= 0xcc0) && (code <= 0xcc4)) ||
           ((code >= 0xcc7) && (code <= 0xcc8)) ||
           ((code >= 0xcca) && (code <= 0xccb)) ||
           ((code >= 0xcd5) && (code <= 0xcd6)) ||
           ((code >= 0xd02) && (code <= 0xd03)) ||
           ((code >= 0xd3e) && (code <= 0xd40)) ||
           ((code >= 0xd46) && (code <= 0xd48)) ||
           ((code >= 0xd4a) && (code <= 0xd4c)) ||
           (code == 0xd57) ||
           ((code >= 0xd82) && (code <= 0xd83)) ||
           ((code >= 0xdcf) && (code <= 0xdd1)) ||
           ((code >= 0xdd8) && (code <= 0xddf)) ||
           ((code >= 0xdf2) && (code <= 0xdf3)) ||
           ((code >= 0xf3e) && (code <= 0xf3f)) ||
           (code == 0xf7f) ||
           (code == 0x102c) ||
           (code == 0x1031) ||
           (code == 0x1038) ||
           ((code >= 0x1056) && (code <= 0x1057)) ||
           ((code >= 0x17b4) && (code <= 0x17b6)) ||
           ((code >= 0x17be) && (code <= 0x17c5)) ||
           ((code >= 0x17c7) && (code <= 0x17c8)) ||
           ((code >= 0x1d165) && (code <= 0x1d166)) ||
           ((code >= 0x1d16d) && (code <= 0x1d172)));
}

/*
 * rtxUCSIsCatMe:
 * @code: UCS code point
 *
 * Check whether the character is part of Me UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatMe (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x488) && (code <= 0x489)) ||
           ((code >= 0x6dd) && (code <= 0x6de)) ||
           ((code >= 0x20dd) && (code <= 0x20e0)) ||
           ((code >= 0x20e2) && (code <= 0x20e3)));
}

/*
 * rtxUCSIsCatMn:
 * @code: UCS code point
 *
 * Check whether the character is part of Mn UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatMn (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x300) && (code <= 0x34e)) ||
           ((code >= 0x360) && (code <= 0x362)) ||
           ((code >= 0x483) && (code <= 0x486)) ||
           ((code >= 0x591) && (code <= 0x5a1)) ||
           ((code >= 0x5a3) && (code <= 0x5b9)) ||
           ((code >= 0x5bb) && (code <= 0x5bd)) ||
           (code == 0x5bf) ||
           ((code >= 0x5c1) && (code <= 0x5c2)) ||
           (code == 0x5c4) ||
           ((code >= 0x64b) && (code <= 0x655)) ||
           (code == 0x670) ||
           ((code >= 0x6d6) && (code <= 0x6dc)) ||
           ((code >= 0x6df) && (code <= 0x6e4)) ||
           ((code >= 0x6e7) && (code <= 0x6e8)) ||
           ((code >= 0x6ea) && (code <= 0x6ed)) ||
           (code == 0x711) ||
           ((code >= 0x730) && (code <= 0x74a)) ||
           ((code >= 0x7a6) && (code <= 0x7b0)) ||
           ((code >= 0x901) && (code <= 0x902)) ||
           (code == 0x93c) ||
           ((code >= 0x941) && (code <= 0x948)) ||
           (code == 0x94d) ||
           ((code >= 0x951) && (code <= 0x954)) ||
           ((code >= 0x962) && (code <= 0x963)) ||
           (code == 0x981) ||
           (code == 0x9bc) ||
           ((code >= 0x9c1) && (code <= 0x9c4)) ||
           (code == 0x9cd) ||
           ((code >= 0x9e2) && (code <= 0x9e3)) ||
           (code == 0xa02) ||
           (code == 0xa3c) ||
           ((code >= 0xa41) && (code <= 0xa42)) ||
           ((code >= 0xa47) && (code <= 0xa48)) ||
           ((code >= 0xa4b) && (code <= 0xa4d)) ||
           ((code >= 0xa70) && (code <= 0xa71)) ||
           ((code >= 0xa81) && (code <= 0xa82)) ||
           (code == 0xabc) ||
           ((code >= 0xac1) && (code <= 0xac5)) ||
           ((code >= 0xac7) && (code <= 0xac8)) ||
           (code == 0xacd) ||
           (code == 0xb01) ||
           (code == 0xb3c) ||
           (code == 0xb3f) ||
           ((code >= 0xb41) && (code <= 0xb43)) ||
           (code == 0xb4d) ||
           (code == 0xb56) ||
           (code == 0xb82) ||
           (code == 0xbc0) ||
           (code == 0xbcd) ||
           ((code >= 0xc3e) && (code <= 0xc40)) ||
           ((code >= 0xc46) && (code <= 0xc48)) ||
           ((code >= 0xc4a) && (code <= 0xc4d)) ||
           ((code >= 0xc55) && (code <= 0xc56)) ||
           (code == 0xcbf) ||
           (code == 0xcc6) ||
           ((code >= 0xccc) && (code <= 0xccd)) ||
           ((code >= 0xd41) && (code <= 0xd43)) ||
           (code == 0xd4d) ||
           (code == 0xdca) ||
           ((code >= 0xdd2) && (code <= 0xdd4)) ||
           (code == 0xdd6) ||
           (code == 0xe31) ||
           ((code >= 0xe34) && (code <= 0xe3a)) ||
           ((code >= 0xe47) && (code <= 0xe4e)) ||
           (code == 0xeb1) ||
           ((code >= 0xeb4) && (code <= 0xeb9)) ||
           ((code >= 0xebb) && (code <= 0xebc)) ||
           ((code >= 0xec8) && (code <= 0xecd)) ||
           ((code >= 0xf18) && (code <= 0xf19)) ||
           (code == 0xf35) ||
           (code == 0xf37) ||
           (code == 0xf39) ||
           ((code >= 0xf71) && (code <= 0xf7e)) ||
           ((code >= 0xf80) && (code <= 0xf84)) ||
           ((code >= 0xf86) && (code <= 0xf87)) ||
           ((code >= 0xf90) && (code <= 0xf97)) ||
           ((code >= 0xf99) && (code <= 0xfbc)) ||
           (code == 0xfc6) ||
           ((code >= 0x102d) && (code <= 0x1030)) ||
           (code == 0x1032) ||
           ((code >= 0x1036) && (code <= 0x1037)) ||
           (code == 0x1039) ||
           ((code >= 0x1058) && (code <= 0x1059)) ||
           ((code >= 0x17b7) && (code <= 0x17bd)) ||
           (code == 0x17c6) ||
           ((code >= 0x17c9) && (code <= 0x17d3)) ||
           (code == 0x18a9) ||
           ((code >= 0x20d0) && (code <= 0x20dc)) ||
           (code == 0x20e1) ||
           ((code >= 0x302a) && (code <= 0x302f)) ||
           ((code >= 0x3099) && (code <= 0x309a)) ||
           (code == 0xfb1e) ||
           ((code >= 0xfe20) && (code <= 0xfe23)) ||
           ((code >= 0x1d167) && (code <= 0x1d169)) ||
           ((code >= 0x1d17b) && (code <= 0x1d182)) ||
           ((code >= 0x1d185) && (code <= 0x1d18b)) ||
           ((code >= 0x1d1aa) && (code <= 0x1d1ad)));
}

/*
 * rtxUCSIsCatN:
 * @code: UCS code point
 *
 * Check whether the character is part of N UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatN (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x30) && (code <= 0x39)) ||
           ((code >= 0xb2) && (code <= 0xb3)) ||
           (code == 0xb9) ||
           ((code >= 0xbc) && (code <= 0xbe)) ||
           ((code >= 0x660) && (code <= 0x669)) ||
           ((code >= 0x6f0) && (code <= 0x6f9)) ||
           ((code >= 0x966) && (code <= 0x96f)) ||
           ((code >= 0x9e6) && (code <= 0x9ef)) ||
           ((code >= 0x9f4) && (code <= 0x9f9)) ||
           ((code >= 0xa66) && (code <= 0xa6f)) ||
           ((code >= 0xae6) && (code <= 0xaef)) ||
           ((code >= 0xb66) && (code <= 0xb6f)) ||
           ((code >= 0xbe7) && (code <= 0xbf2)) ||
           ((code >= 0xc66) && (code <= 0xc6f)) ||
           ((code >= 0xce6) && (code <= 0xcef)) ||
           ((code >= 0xd66) && (code <= 0xd6f)) ||
           ((code >= 0xe50) && (code <= 0xe59)) ||
           ((code >= 0xed0) && (code <= 0xed9)) ||
           ((code >= 0xf20) && (code <= 0xf33)) ||
           ((code >= 0x1040) && (code <= 0x1049)) ||
           ((code >= 0x1369) && (code <= 0x137c)) ||
           ((code >= 0x16ee) && (code <= 0x16f0)) ||
           ((code >= 0x17e0) && (code <= 0x17e9)) ||
           ((code >= 0x1810) && (code <= 0x1819)) ||
           (code == 0x2070) ||
           ((code >= 0x2074) && (code <= 0x2079)) ||
           ((code >= 0x2080) && (code <= 0x2089)) ||
           ((code >= 0x2153) && (code <= 0x2183)) ||
           ((code >= 0x2460) && (code <= 0x249b)) ||
           (code == 0x24ea) ||
           ((code >= 0x2776) && (code <= 0x2793)) ||
           (code == 0x3007) ||
           ((code >= 0x3021) && (code <= 0x3029)) ||
           ((code >= 0x3038) && (code <= 0x303a)) ||
           ((code >= 0x3192) && (code <= 0x3195)) ||
           ((code >= 0x3220) && (code <= 0x3229)) ||
           ((code >= 0x3280) && (code <= 0x3289)) ||
           ((code >= 0xff10) && (code <= 0xff19)) ||
           ((code >= 0x10320) && (code <= 0x10323)) ||
           (code == 0x1034a) ||
           ((code >= 0x1d7ce) && (code <= 0x1d7ff)));
}

/*
 * rtxUCSIsCatNd:
 * @code: UCS code point
 *
 * Check whether the character is part of Nd UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatNd (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x30) && (code <= 0x39)) ||
           ((code >= 0x660) && (code <= 0x669)) ||
           ((code >= 0x6f0) && (code <= 0x6f9)) ||
           ((code >= 0x966) && (code <= 0x96f)) ||
           ((code >= 0x9e6) && (code <= 0x9ef)) ||
           ((code >= 0xa66) && (code <= 0xa6f)) ||
           ((code >= 0xae6) && (code <= 0xaef)) ||
           ((code >= 0xb66) && (code <= 0xb6f)) ||
           ((code >= 0xbe7) && (code <= 0xbef)) ||
           ((code >= 0xc66) && (code <= 0xc6f)) ||
           ((code >= 0xce6) && (code <= 0xcef)) ||
           ((code >= 0xd66) && (code <= 0xd6f)) ||
           ((code >= 0xe50) && (code <= 0xe59)) ||
           ((code >= 0xed0) && (code <= 0xed9)) ||
           ((code >= 0xf20) && (code <= 0xf29)) ||
           ((code >= 0x1040) && (code <= 0x1049)) ||
           ((code >= 0x1369) && (code <= 0x1371)) ||
           ((code >= 0x17e0) && (code <= 0x17e9)) ||
           ((code >= 0x1810) && (code <= 0x1819)) ||
           ((code >= 0xff10) && (code <= 0xff19)) ||
           ((code >= 0x1d7ce) && (code <= 0x1d7ff)));
}

/*
 * rtxUCSIsCatNl:
 * @code: UCS code point
 *
 * Check whether the character is part of Nl UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatNl (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x16ee) && (code <= 0x16f0)) ||
           ((code >= 0x2160) && (code <= 0x2183)) ||
           (code == 0x3007) ||
           ((code >= 0x3021) && (code <= 0x3029)) ||
           ((code >= 0x3038) && (code <= 0x303a)) ||
           (code == 0x1034a));
}

/*
 * rtxUCSIsCatNo:
 * @code: UCS code point
 *
 * Check whether the character is part of No UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatNo (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0xb2) && (code <= 0xb3)) ||
           (code == 0xb9) ||
           ((code >= 0xbc) && (code <= 0xbe)) ||
           ((code >= 0x9f4) && (code <= 0x9f9)) ||
           ((code >= 0xbf0) && (code <= 0xbf2)) ||
           ((code >= 0xf2a) && (code <= 0xf33)) ||
           ((code >= 0x1372) && (code <= 0x137c)) ||
           (code == 0x2070) ||
           ((code >= 0x2074) && (code <= 0x2079)) ||
           ((code >= 0x2080) && (code <= 0x2089)) ||
           ((code >= 0x2153) && (code <= 0x215f)) ||
           ((code >= 0x2460) && (code <= 0x249b)) ||
           (code == 0x24ea) ||
           ((code >= 0x2776) && (code <= 0x2793)) ||
           ((code >= 0x3192) && (code <= 0x3195)) ||
           ((code >= 0x3220) && (code <= 0x3229)) ||
           ((code >= 0x3280) && (code <= 0x3289)) ||
           ((code >= 0x10320) && (code <= 0x10323)));
}

/*
 * rtxUCSIsCatP:
 * @code: UCS code point
 *
 * Check whether the character is part of P UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatP (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x21) && (code <= 0x23)) ||
           ((code >= 0x25) && (code <= 0x2a)) ||
           ((code >= 0x2c) && (code <= 0x2f)) ||
           ((code >= 0x3a) && (code <= 0x3b)) ||
           ((code >= 0x3f) && (code <= 0x40)) ||
           ((code >= 0x5b) && (code <= 0x5d)) ||
           (code == 0x5f) ||
           (code == 0x7b) ||
           (code == 0x7d) ||
           (code == 0xa1) ||
           (code == 0xab) ||
           (code == 0xad) ||
           (code == 0xb7) ||
           (code == 0xbb) ||
           (code == 0xbf) ||
           (code == 0x37e) ||
           (code == 0x387) ||
           ((code >= 0x55a) && (code <= 0x55f)) ||
           ((code >= 0x589) && (code <= 0x58a)) ||
           (code == 0x5be) ||
           (code == 0x5c0) ||
           (code == 0x5c3) ||
           ((code >= 0x5f3) && (code <= 0x5f4)) ||
           (code == 0x60c) ||
           (code == 0x61b) ||
           (code == 0x61f) ||
           ((code >= 0x66a) && (code <= 0x66d)) ||
           (code == 0x6d4) ||
           ((code >= 0x700) && (code <= 0x70d)) ||
           ((code >= 0x964) && (code <= 0x965)) ||
           (code == 0x970) ||
           (code == 0xdf4) ||
           (code == 0xe4f) ||
           ((code >= 0xe5a) && (code <= 0xe5b)) ||
           ((code >= 0xf04) && (code <= 0xf12)) ||
           ((code >= 0xf3a) && (code <= 0xf3d)) ||
           (code == 0xf85) ||
           ((code >= 0x104a) && (code <= 0x104f)) ||
           (code == 0x10fb) ||
           ((code >= 0x1361) && (code <= 0x1368)) ||
           ((code >= 0x166d) && (code <= 0x166e)) ||
           ((code >= 0x169b) && (code <= 0x169c)) ||
           ((code >= 0x16eb) && (code <= 0x16ed)) ||
           ((code >= 0x17d4) && (code <= 0x17da)) ||
           (code == 0x17dc) ||
           ((code >= 0x1800) && (code <= 0x180a)) ||
           ((code >= 0x2010) && (code <= 0x2027)) ||
           ((code >= 0x2030) && (code <= 0x2043)) ||
           ((code >= 0x2045) && (code <= 0x2046)) ||
           ((code >= 0x2048) && (code <= 0x204d)) ||
           ((code >= 0x207d) && (code <= 0x207e)) ||
           ((code >= 0x208d) && (code <= 0x208e)) ||
           ((code >= 0x2329) && (code <= 0x232a)) ||
           ((code >= 0x3001) && (code <= 0x3003)) ||
           ((code >= 0x3008) && (code <= 0x3011)) ||
           ((code >= 0x3014) && (code <= 0x301f)) ||
           (code == 0x3030) ||
           (code == 0x30fb) ||
           ((code >= 0xfd3e) && (code <= 0xfd3f)) ||
           ((code >= 0xfe30) && (code <= 0xfe44)) ||
           ((code >= 0xfe49) && (code <= 0xfe52)) ||
           ((code >= 0xfe54) && (code <= 0xfe61)) ||
           (code == 0xfe63) ||
           (code == 0xfe68) ||
           ((code >= 0xfe6a) && (code <= 0xfe6b)) ||
           ((code >= 0xff01) && (code <= 0xff03)) ||
           ((code >= 0xff05) && (code <= 0xff0a)) ||
           ((code >= 0xff0c) && (code <= 0xff0f)) ||
           ((code >= 0xff1a) && (code <= 0xff1b)) ||
           ((code >= 0xff1f) && (code <= 0xff20)) ||
           ((code >= 0xff3b) && (code <= 0xff3d)) ||
           (code == 0xff3f) ||
           (code == 0xff5b) ||
           (code == 0xff5d) ||
           ((code >= 0xff61) && (code <= 0xff65)));
}

/*
 * rtxUCSIsCatPc:
 * @code: UCS code point
 *
 * Check whether the character is part of Pc UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPc (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x5f) ||
           ((code >= 0x203f) && (code <= 0x2040)) ||
           (code == 0x30fb) ||
           ((code >= 0xfe33) && (code <= 0xfe34)) ||
           ((code >= 0xfe4d) && (code <= 0xfe4f)) ||
           (code == 0xff3f) ||
           (code == 0xff65));
}

/*
 * rtxUCSIsCatPd:
 * @code: UCS code point
 *
 * Check whether the character is part of Pd UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPd (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x2d) ||
           (code == 0xad) ||
           (code == 0x58a) ||
           (code == 0x1806) ||
           ((code >= 0x2010) && (code <= 0x2015)) ||
           (code == 0x301c) ||
           (code == 0x3030) ||
           ((code >= 0xfe31) && (code <= 0xfe32)) ||
           (code == 0xfe58) ||
           (code == 0xfe63) ||
           (code == 0xff0d));
}

/*
 * rtxUCSIsCatPe:
 * @code: UCS code point
 *
 * Check whether the character is part of Pe UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPe (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x29) ||
           (code == 0x5d) ||
           (code == 0x7d) ||
           (code == 0xf3b) ||
           (code == 0xf3d) ||
           (code == 0x169c) ||
           (code == 0x2046) ||
           (code == 0x207e) ||
           (code == 0x208e) ||
           (code == 0x232a) ||
           (code == 0x3009) ||
           (code == 0x300b) ||
           (code == 0x300d) ||
           (code == 0x300f) ||
           (code == 0x3011) ||
           (code == 0x3015) ||
           (code == 0x3017) ||
           (code == 0x3019) ||
           (code == 0x301b) ||
           ((code >= 0x301e) && (code <= 0x301f)) ||
           (code == 0xfd3f) ||
           (code == 0xfe36) ||
           (code == 0xfe38) ||
           (code == 0xfe3a) ||
           (code == 0xfe3c) ||
           (code == 0xfe3e) ||
           (code == 0xfe40) ||
           (code == 0xfe42) ||
           (code == 0xfe44) ||
           (code == 0xfe5a) ||
           (code == 0xfe5c) ||
           (code == 0xfe5e) ||
           (code == 0xff09) ||
           (code == 0xff3d) ||
           (code == 0xff5d) ||
           (code == 0xff63));
}

/*
 * rtxUCSIsCatPf:
 * @code: UCS code point
 *
 * Check whether the character is part of Pf UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPf (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0xbb) ||
           (code == 0x2019) ||
           (code == 0x201d) ||
           (code == 0x203a));
}

/*
 * rtxUCSIsCatPi:
 * @code: UCS code point
 *
 * Check whether the character is part of Pi UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPi (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0xab) ||
           (code == 0x2018) ||
           ((code >= 0x201b) && (code <= 0x201c)) ||
           (code == 0x201f) ||
           (code == 0x2039));
}

/*
 * rtxUCSIsCatPo:
 * @code: UCS code point
 *
 * Check whether the character is part of Po UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPo (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0x21) && (code <= 0x23)) ||
           ((code >= 0x25) && (code <= 0x27)) ||
           (code == 0x2a) ||
           (code == 0x2c) ||
           ((code >= 0x2e) && (code <= 0x2f)) ||
           ((code >= 0x3a) && (code <= 0x3b)) ||
           ((code >= 0x3f) && (code <= 0x40)) ||
           (code == 0x5c) ||
           (code == 0xa1) ||
           (code == 0xb7) ||
           (code == 0xbf) ||
           (code == 0x37e) ||
           (code == 0x387) ||
           ((code >= 0x55a) && (code <= 0x55f)) ||
           (code == 0x589) ||
           (code == 0x5be) ||
           (code == 0x5c0) ||
           (code == 0x5c3) ||
           ((code >= 0x5f3) && (code <= 0x5f4)) ||
           (code == 0x60c) ||
           (code == 0x61b) ||
           (code == 0x61f) ||
           ((code >= 0x66a) && (code <= 0x66d)) ||
           (code == 0x6d4) ||
           ((code >= 0x700) && (code <= 0x70d)) ||
           ((code >= 0x964) && (code <= 0x965)) ||
           (code == 0x970) ||
           (code == 0xdf4) ||
           (code == 0xe4f) ||
           ((code >= 0xe5a) && (code <= 0xe5b)) ||
           ((code >= 0xf04) && (code <= 0xf12)) ||
           (code == 0xf85) ||
           ((code >= 0x104a) && (code <= 0x104f)) ||
           (code == 0x10fb) ||
           ((code >= 0x1361) && (code <= 0x1368)) ||
           ((code >= 0x166d) && (code <= 0x166e)) ||
           ((code >= 0x16eb) && (code <= 0x16ed)) ||
           ((code >= 0x17d4) && (code <= 0x17da)) ||
           (code == 0x17dc) ||
           ((code >= 0x1800) && (code <= 0x1805)) ||
           ((code >= 0x1807) && (code <= 0x180a)) ||
           ((code >= 0x2016) && (code <= 0x2017)) ||
           ((code >= 0x2020) && (code <= 0x2027)) ||
           ((code >= 0x2030) && (code <= 0x2038)) ||
           ((code >= 0x203b) && (code <= 0x203e)) ||
           ((code >= 0x2041) && (code <= 0x2043)) ||
           ((code >= 0x2048) && (code <= 0x204d)) ||
           ((code >= 0x3001) && (code <= 0x3003)) ||
           (code == 0xfe30) ||
           ((code >= 0xfe49) && (code <= 0xfe4c)) ||
           ((code >= 0xfe50) && (code <= 0xfe52)) ||
           ((code >= 0xfe54) && (code <= 0xfe57)) ||
           ((code >= 0xfe5f) && (code <= 0xfe61)) ||
           (code == 0xfe68) ||
           ((code >= 0xfe6a) && (code <= 0xfe6b)) ||
           ((code >= 0xff01) && (code <= 0xff03)) ||
           ((code >= 0xff05) && (code <= 0xff07)) ||
           (code == 0xff0a) ||
           (code == 0xff0c) ||
           ((code >= 0xff0e) && (code <= 0xff0f)) ||
           ((code >= 0xff1a) && (code <= 0xff1b)) ||
           ((code >= 0xff1f) && (code <= 0xff20)) ||
           (code == 0xff3c) ||
           (code == 0xff61) ||
           (code == 0xff64));
}

/*
 * rtxUCSIsCatPs:
 * @code: UCS code point
 *
 * Check whether the character is part of Ps UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatPs (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x28) ||
           (code == 0x5b) ||
           (code == 0x7b) ||
           (code == 0xf3a) ||
           (code == 0xf3c) ||
           (code == 0x169b) ||
           (code == 0x201a) ||
           (code == 0x201e) ||
           (code == 0x2045) ||
           (code == 0x207d) ||
           (code == 0x208d) ||
           (code == 0x2329) ||
           (code == 0x3008) ||
           (code == 0x300a) ||
           (code == 0x300c) ||
           (code == 0x300e) ||
           (code == 0x3010) ||
           (code == 0x3014) ||
           (code == 0x3016) ||
           (code == 0x3018) ||
           (code == 0x301a) ||
           (code == 0x301d) ||
           (code == 0xfd3e) ||
           (code == 0xfe35) ||
           (code == 0xfe37) ||
           (code == 0xfe39) ||
           (code == 0xfe3b) ||
           (code == 0xfe3d) ||
           (code == 0xfe3f) ||
           (code == 0xfe41) ||
           (code == 0xfe43) ||
           (code == 0xfe59) ||
           (code == 0xfe5b) ||
           (code == 0xfe5d) ||
           (code == 0xff08) ||
           (code == 0xff3b) ||
           (code == 0xff5b) ||
           (code == 0xff62));
}

/*
 * rtxUCSIsCatS:
 * @code: UCS code point
 *
 * Check whether the character is part of S UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatS (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x24) ||
           (code == 0x2b) ||
           ((code >= 0x3c) && (code <= 0x3e)) ||
           (code == 0x5e) ||
           (code == 0x60) ||
           (code == 0x7c) ||
           (code == 0x7e) ||
           ((code >= 0xa2) && (code <= 0xa9)) ||
           (code == 0xac) ||
           ((code >= 0xae) && (code <= 0xb1)) ||
           (code == 0xb4) ||
           (code == 0xb6) ||
           (code == 0xb8) ||
           (code == 0xd7) ||
           (code == 0xf7) ||
           ((code >= 0x2b9) && (code <= 0x2ba)) ||
           ((code >= 0x2c2) && (code <= 0x2cf)) ||
           ((code >= 0x2d2) && (code <= 0x2df)) ||
           ((code >= 0x2e5) && (code <= 0x2ed)) ||
           ((code >= 0x374) && (code <= 0x375)) ||
           ((code >= 0x384) && (code <= 0x385)) ||
           (code == 0x482) ||
           (code == 0x6e9) ||
           ((code >= 0x6fd) && (code <= 0x6fe)) ||
           ((code >= 0x9f2) && (code <= 0x9f3)) ||
           (code == 0x9fa) ||
           (code == 0xb70) ||
           (code == 0xe3f) ||
           ((code >= 0xf01) && (code <= 0xf03)) ||
           ((code >= 0xf13) && (code <= 0xf17)) ||
           ((code >= 0xf1a) && (code <= 0xf1f)) ||
           (code == 0xf34) ||
           (code == 0xf36) ||
           (code == 0xf38) ||
           ((code >= 0xfbe) && (code <= 0xfc5)) ||
           ((code >= 0xfc7) && (code <= 0xfcc)) ||
           (code == 0xfcf) ||
           (code == 0x17db) ||
           (code == 0x1fbd) ||
           ((code >= 0x1fbf) && (code <= 0x1fc1)) ||
           ((code >= 0x1fcd) && (code <= 0x1fcf)) ||
           ((code >= 0x1fdd) && (code <= 0x1fdf)) ||
           ((code >= 0x1fed) && (code <= 0x1fef)) ||
           ((code >= 0x1ffd) && (code <= 0x1ffe)) ||
           (code == 0x2044) ||
           ((code >= 0x207a) && (code <= 0x207c)) ||
           ((code >= 0x208a) && (code <= 0x208c)) ||
           ((code >= 0x20a0) && (code <= 0x20af)) ||
           ((code >= 0x2100) && (code <= 0x2101)) ||
           ((code >= 0x2103) && (code <= 0x2106)) ||
           ((code >= 0x2108) && (code <= 0x2109)) ||
           (code == 0x2114) ||
           ((code >= 0x2116) && (code <= 0x2118)) ||
           ((code >= 0x211e) && (code <= 0x2123)) ||
           (code == 0x2125) ||
           (code == 0x2127) ||
           (code == 0x2129) ||
           (code == 0x212e) ||
           (code == 0x2132) ||
           (code == 0x213a) ||
           ((code >= 0x2190) && (code <= 0x21f3)) ||
           ((code >= 0x2200) && (code <= 0x22f1)) ||
           ((code >= 0x2300) && (code <= 0x2328)) ||
           ((code >= 0x232b) && (code <= 0x237b)) ||
           ((code >= 0x237d) && (code <= 0x239a)) ||
           ((code >= 0x2400) && (code <= 0x2426)) ||
           ((code >= 0x2440) && (code <= 0x244a)) ||
           ((code >= 0x249c) && (code <= 0x24e9)) ||
           ((code >= 0x2500) && (code <= 0x2595)) ||
           ((code >= 0x25a0) && (code <= 0x25f7)) ||
           ((code >= 0x2600) && (code <= 0x2613)) ||
           ((code >= 0x2619) && (code <= 0x2671)) ||
           ((code >= 0x2701) && (code <= 0x2704)) ||
           ((code >= 0x2706) && (code <= 0x2709)) ||
           ((code >= 0x270c) && (code <= 0x2727)) ||
           ((code >= 0x2729) && (code <= 0x274b)) ||
           (code == 0x274d) ||
           ((code >= 0x274f) && (code <= 0x2752)) ||
           (code == 0x2756) ||
           ((code >= 0x2758) && (code <= 0x275e)) ||
           ((code >= 0x2761) && (code <= 0x2767)) ||
           (code == 0x2794) ||
           ((code >= 0x2798) && (code <= 0x27af)) ||
           ((code >= 0x27b1) && (code <= 0x27be)) ||
           ((code >= 0x2800) && (code <= 0x28ff)) ||
           ((code >= 0x2e80) && (code <= 0x2e99)) ||
           ((code >= 0x2e9b) && (code <= 0x2ef3)) ||
           ((code >= 0x2f00) && (code <= 0x2fd5)) ||
           ((code >= 0x2ff0) && (code <= 0x2ffb)) ||
           (code == 0x3004) ||
           ((code >= 0x3012) && (code <= 0x3013)) ||
           (code == 0x3020) ||
           ((code >= 0x3036) && (code <= 0x3037)) ||
           ((code >= 0x303e) && (code <= 0x303f)) ||
           ((code >= 0x309b) && (code <= 0x309c)) ||
           ((code >= 0x3190) && (code <= 0x3191)) ||
           ((code >= 0x3196) && (code <= 0x319f)) ||
           ((code >= 0x3200) && (code <= 0x321c)) ||
           ((code >= 0x322a) && (code <= 0x3243)) ||
           ((code >= 0x3260) && (code <= 0x327b)) ||
           (code == 0x327f) ||
           ((code >= 0x328a) && (code <= 0x32b0)) ||
           ((code >= 0x32c0) && (code <= 0x32cb)) ||
           ((code >= 0x32d0) && (code <= 0x32fe)) ||
           ((code >= 0x3300) && (code <= 0x3376)) ||
           ((code >= 0x337b) && (code <= 0x33dd)) ||
           ((code >= 0x33e0) && (code <= 0x33fe)) ||
           ((code >= 0xa490) && (code <= 0xa4a1)) ||
           ((code >= 0xa4a4) && (code <= 0xa4b3)) ||
           ((code >= 0xa4b5) && (code <= 0xa4c0)) ||
           ((code >= 0xa4c2) && (code <= 0xa4c4)) ||
           (code == 0xa4c6) ||
           (code == 0xfb29) ||
           (code == 0xfe62) ||
           ((code >= 0xfe64) && (code <= 0xfe66)) ||
           (code == 0xfe69) ||
           (code == 0xff04) ||
           (code == 0xff0b) ||
           ((code >= 0xff1c) && (code <= 0xff1e)) ||
           (code == 0xff3e) ||
           (code == 0xff40) ||
           (code == 0xff5c) ||
           (code == 0xff5e) ||
           ((code >= 0xffe0) && (code <= 0xffe6)) ||
           ((code >= 0xffe8) && (code <= 0xffee)) ||
           ((code >= 0xfffc) && (code <= 0xfffd)) ||
           ((code >= 0x1d000) && (code <= 0x1d0f5)) ||
           ((code >= 0x1d100) && (code <= 0x1d126)) ||
           ((code >= 0x1d12a) && (code <= 0x1d164)) ||
           ((code >= 0x1d16a) && (code <= 0x1d16c)) ||
           ((code >= 0x1d183) && (code <= 0x1d184)) ||
           ((code >= 0x1d18c) && (code <= 0x1d1a9)) ||
           ((code >= 0x1d1ae) && (code <= 0x1d1dd)) ||
           (code == 0x1d6c1) ||
           (code == 0x1d6db) ||
           (code == 0x1d6fb) ||
           (code == 0x1d715) ||
           (code == 0x1d735) ||
           (code == 0x1d74f) ||
           (code == 0x1d76f) ||
           (code == 0x1d789) ||
           (code == 0x1d7a9) ||
           (code == 0x1d7c3));
}

/*
 * rtxUCSIsCatSc:
 * @code: UCS code point
 *
 * Check whether the character is part of Sc UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatSc (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x24) ||
           ((code >= 0xa2) && (code <= 0xa5)) ||
           ((code >= 0x9f2) && (code <= 0x9f3)) ||
           (code == 0xe3f) ||
           (code == 0x17db) ||
           ((code >= 0x20a0) && (code <= 0x20af)) ||
           (code == 0xfe69) ||
           (code == 0xff04) ||
           ((code >= 0xffe0) && (code <= 0xffe1)) ||
           ((code >= 0xffe5) && (code <= 0xffe6)));
}

/*
 * rtxUCSIsCatSk:
 * @code: UCS code point
 *
 * Check whether the character is part of Sk UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatSk (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x5e) ||
           (code == 0x60) ||
           (code == 0xa8) ||
           (code == 0xaf) ||
           (code == 0xb4) ||
           (code == 0xb8) ||
           ((code >= 0x2b9) && (code <= 0x2ba)) ||
           ((code >= 0x2c2) && (code <= 0x2cf)) ||
           ((code >= 0x2d2) && (code <= 0x2df)) ||
           ((code >= 0x2e5) && (code <= 0x2ed)) ||
           ((code >= 0x374) && (code <= 0x375)) ||
           ((code >= 0x384) && (code <= 0x385)) ||
           (code == 0x1fbd) ||
           ((code >= 0x1fbf) && (code <= 0x1fc1)) ||
           ((code >= 0x1fcd) && (code <= 0x1fcf)) ||
           ((code >= 0x1fdd) && (code <= 0x1fdf)) ||
           ((code >= 0x1fed) && (code <= 0x1fef)) ||
           ((code >= 0x1ffd) && (code <= 0x1ffe)) ||
           ((code >= 0x309b) && (code <= 0x309c)) ||
           (code == 0xff3e) ||
           (code == 0xff40) ||
           (code == 0xffe3));
}

/*
 * rtxUCSIsCatSm:
 * @code: UCS code point
 *
 * Check whether the character is part of Sm UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatSm (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x2b) ||
           ((code >= 0x3c) && (code <= 0x3e)) ||
           (code == 0x7c) ||
           (code == 0x7e) ||
           (code == 0xac) ||
           (code == 0xb1) ||
           (code == 0xd7) ||
           (code == 0xf7) ||
           (code == 0x2044) ||
           ((code >= 0x207a) && (code <= 0x207c)) ||
           ((code >= 0x208a) && (code <= 0x208c)) ||
           ((code >= 0x2190) && (code <= 0x2194)) ||
           ((code >= 0x219a) && (code <= 0x219b)) ||
           (code == 0x21a0) ||
           (code == 0x21a3) ||
           (code == 0x21a6) ||
           (code == 0x21ae) ||
           ((code >= 0x21ce) && (code <= 0x21cf)) ||
           (code == 0x21d2) ||
           (code == 0x21d4) ||
           ((code >= 0x2200) && (code <= 0x22f1)) ||
           ((code >= 0x2308) && (code <= 0x230b)) ||
           ((code >= 0x2320) && (code <= 0x2321)) ||
           (code == 0x25b7) ||
           (code == 0x25c1) ||
           (code == 0x266f) ||
           (code == 0xfb29) ||
           (code == 0xfe62) ||
           ((code >= 0xfe64) && (code <= 0xfe66)) ||
           (code == 0xff0b) ||
           ((code >= 0xff1c) && (code <= 0xff1e)) ||
           (code == 0xff5c) ||
           (code == 0xff5e) ||
           (code == 0xffe2) ||
           ((code >= 0xffe9) && (code <= 0xffec)) ||
           (code == 0x1d6c1) ||
           (code == 0x1d6db) ||
           (code == 0x1d6fb) ||
           (code == 0x1d715) ||
           (code == 0x1d735) ||
           (code == 0x1d74f) ||
           (code == 0x1d76f) ||
           (code == 0x1d789) ||
           (code == 0x1d7a9) ||
           (code == 0x1d7c3));
}

/*
 * rtxUCSIsCatSo:
 * @code: UCS code point
 *
 * Check whether the character is part of So UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatSo (OS32BITCHAR code)
{
    return (OSBOOL)(((code >= 0xa6) && (code <= 0xa7)) ||
           (code == 0xa9) ||
           (code == 0xae) ||
           (code == 0xb0) ||
           (code == 0xb6) ||
           (code == 0x482) ||
           (code == 0x6e9) ||
           ((code >= 0x6fd) && (code <= 0x6fe)) ||
           (code == 0x9fa) ||
           (code == 0xb70) ||
           ((code >= 0xf01) && (code <= 0xf03)) ||
           ((code >= 0xf13) && (code <= 0xf17)) ||
           ((code >= 0xf1a) && (code <= 0xf1f)) ||
           (code == 0xf34) ||
           (code == 0xf36) ||
           (code == 0xf38) ||
           ((code >= 0xfbe) && (code <= 0xfc5)) ||
           ((code >= 0xfc7) && (code <= 0xfcc)) ||
           (code == 0xfcf) ||
           ((code >= 0x2100) && (code <= 0x2101)) ||
           ((code >= 0x2103) && (code <= 0x2106)) ||
           ((code >= 0x2108) && (code <= 0x2109)) ||
           (code == 0x2114) ||
           ((code >= 0x2116) && (code <= 0x2118)) ||
           ((code >= 0x211e) && (code <= 0x2123)) ||
           (code == 0x2125) ||
           (code == 0x2127) ||
           (code == 0x2129) ||
           (code == 0x212e) ||
           (code == 0x2132) ||
           (code == 0x213a) ||
           ((code >= 0x2195) && (code <= 0x2199)) ||
           ((code >= 0x219c) && (code <= 0x219f)) ||
           ((code >= 0x21a1) && (code <= 0x21a2)) ||
           ((code >= 0x21a4) && (code <= 0x21a5)) ||
           ((code >= 0x21a7) && (code <= 0x21ad)) ||
           ((code >= 0x21af) && (code <= 0x21cd)) ||
           ((code >= 0x21d0) && (code <= 0x21d1)) ||
           (code == 0x21d3) ||
           ((code >= 0x21d5) && (code <= 0x21f3)) ||
           ((code >= 0x2300) && (code <= 0x2307)) ||
           ((code >= 0x230c) && (code <= 0x231f)) ||
           ((code >= 0x2322) && (code <= 0x2328)) ||
           ((code >= 0x232b) && (code <= 0x237b)) ||
           ((code >= 0x237d) && (code <= 0x239a)) ||
           ((code >= 0x2400) && (code <= 0x2426)) ||
           ((code >= 0x2440) && (code <= 0x244a)) ||
           ((code >= 0x249c) && (code <= 0x24e9)) ||
           ((code >= 0x2500) && (code <= 0x2595)) ||
           ((code >= 0x25a0) && (code <= 0x25b6)) ||
           ((code >= 0x25b8) && (code <= 0x25c0)) ||
           ((code >= 0x25c2) && (code <= 0x25f7)) ||
           ((code >= 0x2600) && (code <= 0x2613)) ||
           ((code >= 0x2619) && (code <= 0x266e)) ||
           ((code >= 0x2670) && (code <= 0x2671)) ||
           ((code >= 0x2701) && (code <= 0x2704)) ||
           ((code >= 0x2706) && (code <= 0x2709)) ||
           ((code >= 0x270c) && (code <= 0x2727)) ||
           ((code >= 0x2729) && (code <= 0x274b)) ||
           (code == 0x274d) ||
           ((code >= 0x274f) && (code <= 0x2752)) ||
           (code == 0x2756) ||
           ((code >= 0x2758) && (code <= 0x275e)) ||
           ((code >= 0x2761) && (code <= 0x2767)) ||
           (code == 0x2794) ||
           ((code >= 0x2798) && (code <= 0x27af)) ||
           ((code >= 0x27b1) && (code <= 0x27be)) ||
           ((code >= 0x2800) && (code <= 0x28ff)) ||
           ((code >= 0x2e80) && (code <= 0x2e99)) ||
           ((code >= 0x2e9b) && (code <= 0x2ef3)) ||
           ((code >= 0x2f00) && (code <= 0x2fd5)) ||
           ((code >= 0x2ff0) && (code <= 0x2ffb)) ||
           (code == 0x3004) ||
           ((code >= 0x3012) && (code <= 0x3013)) ||
           (code == 0x3020) ||
           ((code >= 0x3036) && (code <= 0x3037)) ||
           ((code >= 0x303e) && (code <= 0x303f)) ||
           ((code >= 0x3190) && (code <= 0x3191)) ||
           ((code >= 0x3196) && (code <= 0x319f)) ||
           ((code >= 0x3200) && (code <= 0x321c)) ||
           ((code >= 0x322a) && (code <= 0x3243)) ||
           ((code >= 0x3260) && (code <= 0x327b)) ||
           (code == 0x327f) ||
           ((code >= 0x328a) && (code <= 0x32b0)) ||
           ((code >= 0x32c0) && (code <= 0x32cb)) ||
           ((code >= 0x32d0) && (code <= 0x32fe)) ||
           ((code >= 0x3300) && (code <= 0x3376)) ||
           ((code >= 0x337b) && (code <= 0x33dd)) ||
           ((code >= 0x33e0) && (code <= 0x33fe)) ||
           ((code >= 0xa490) && (code <= 0xa4a1)) ||
           ((code >= 0xa4a4) && (code <= 0xa4b3)) ||
           ((code >= 0xa4b5) && (code <= 0xa4c0)) ||
           ((code >= 0xa4c2) && (code <= 0xa4c4)) ||
           (code == 0xa4c6) ||
           (code == 0xffe4) ||
           (code == 0xffe8) ||
           ((code >= 0xffed) && (code <= 0xffee)) ||
           ((code >= 0xfffc) && (code <= 0xfffd)) ||
           ((code >= 0x1d000) && (code <= 0x1d0f5)) ||
           ((code >= 0x1d100) && (code <= 0x1d126)) ||
           ((code >= 0x1d12a) && (code <= 0x1d164)) ||
           ((code >= 0x1d16a) && (code <= 0x1d16c)) ||
           ((code >= 0x1d183) && (code <= 0x1d184)) ||
           ((code >= 0x1d18c) && (code <= 0x1d1a9)) ||
           ((code >= 0x1d1ae) && (code <= 0x1d1dd)));
}

/*
 * rtxUCSIsCatZ:
 * @code: UCS code point
 *
 * Check whether the character is part of Z UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatZ (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x20) ||
           (code == 0xa0) ||
           (code == 0x1680) ||
           ((code >= 0x2000) && (code <= 0x200b)) ||
           ((code >= 0x2028) && (code <= 0x2029)) ||
           (code == 0x202f) ||
           (code == 0x3000));
}

/*
 * rtxUCSIsCatZl:
 * @code: UCS code point
 *
 * Check whether the character is part of Zl UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatZl (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x2028));
}

/*
 * rtxUCSIsCatZp:
 * @code: UCS code point
 *
 * Check whether the character is part of Zp UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatZp (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x2029));
}

/*
 * rtxUCSIsCatZs:
 * @code: UCS code point
 *
 * Check whether the character is part of Zs UCS Category
 *
 * Returns 1 if true 0 otherwise
 */
EXTRTMETHOD OSBOOL rtxUCSIsCatZs (OS32BITCHAR code)
{
    return (OSBOOL)((code == 0x20) ||
           (code == 0xa0) ||
           (code == 0x1680) ||
           ((code >= 0x2000) && (code <= 0x200b)) ||
           (code == 0x202f) ||
           (code == 0x3000));
}

/*
 * rtxUCSIsCat:
 * @code: UCS code point
 * @cat: UCS Category name
 *
 * Check whether the caracter is part of the UCS Category
 *
 * Returns 1 if true, 0 if false and -1 on unknown category
 */
EXTRTMETHOD int rtxUCSIsCat (OS32BITCHAR code, const char *cat)
{
    if (!OSCRTLSTRCMP(cat, "C"))
        return(rtxUCSIsCatC(code));
    if (!OSCRTLSTRCMP(cat, "Cc"))
        return(rtxUCSIsCatCc(code));
    if (!OSCRTLSTRCMP(cat, "Cf"))
        return(rtxUCSIsCatCf(code));
    if (!OSCRTLSTRCMP(cat, "Co"))
        return(rtxUCSIsCatCo(code));
    if (!OSCRTLSTRCMP(cat, "Cs"))
        return(rtxUCSIsCatCs(code));
    if (!OSCRTLSTRCMP(cat, "L"))
        return(rtxUCSIsCatL(code));
    if (!OSCRTLSTRCMP(cat, "Ll"))
        return(rtxUCSIsCatLl(code));
    if (!OSCRTLSTRCMP(cat, "Lm"))
        return(rtxUCSIsCatLm(code));
    if (!OSCRTLSTRCMP(cat, "Lo"))
        return(rtxUCSIsCatLo(code));
    if (!OSCRTLSTRCMP(cat, "Lt"))
        return(rtxUCSIsCatLt(code));
    if (!OSCRTLSTRCMP(cat, "Lu"))
        return(rtxUCSIsCatLu(code));
    if (!OSCRTLSTRCMP(cat, "M"))
        return(rtxUCSIsCatM(code));
    if (!OSCRTLSTRCMP(cat, "Mc"))
        return(rtxUCSIsCatMc(code));
    if (!OSCRTLSTRCMP(cat, "Me"))
        return(rtxUCSIsCatMe(code));
    if (!OSCRTLSTRCMP(cat, "Mn"))
        return(rtxUCSIsCatMn(code));
    if (!OSCRTLSTRCMP(cat, "N"))
        return(rtxUCSIsCatN(code));
    if (!OSCRTLSTRCMP(cat, "Nd"))
        return(rtxUCSIsCatNd(code));
    if (!OSCRTLSTRCMP(cat, "Nl"))
        return(rtxUCSIsCatNl(code));
    if (!OSCRTLSTRCMP(cat, "No"))
        return(rtxUCSIsCatNo(code));
    if (!OSCRTLSTRCMP(cat, "P"))
        return(rtxUCSIsCatP(code));
    if (!OSCRTLSTRCMP(cat, "Pc"))
        return(rtxUCSIsCatPc(code));
    if (!OSCRTLSTRCMP(cat, "Pd"))
        return(rtxUCSIsCatPd(code));
    if (!OSCRTLSTRCMP(cat, "Pe"))
        return(rtxUCSIsCatPe(code));
    if (!OSCRTLSTRCMP(cat, "Pf"))
        return(rtxUCSIsCatPf(code));
    if (!OSCRTLSTRCMP(cat, "Pi"))
        return(rtxUCSIsCatPi(code));
    if (!OSCRTLSTRCMP(cat, "Po"))
        return(rtxUCSIsCatPo(code));
    if (!OSCRTLSTRCMP(cat, "Ps"))
        return(rtxUCSIsCatPs(code));
    if (!OSCRTLSTRCMP(cat, "S"))
        return(rtxUCSIsCatS(code));
    if (!OSCRTLSTRCMP(cat, "Sc"))
        return(rtxUCSIsCatSc(code));
    if (!OSCRTLSTRCMP(cat, "Sk"))
        return(rtxUCSIsCatSk(code));
    if (!OSCRTLSTRCMP(cat, "Sm"))
        return(rtxUCSIsCatSm(code));
    if (!OSCRTLSTRCMP(cat, "So"))
        return(rtxUCSIsCatSo(code));
    if (!OSCRTLSTRCMP(cat, "Z"))
        return(rtxUCSIsCatZ(code));
    if (!OSCRTLSTRCMP(cat, "Zl"))
        return(rtxUCSIsCatZl(code));
    if (!OSCRTLSTRCMP(cat, "Zp"))
        return(rtxUCSIsCatZp(code));
    if (!OSCRTLSTRCMP(cat, "Zs"))
        return(rtxUCSIsCatZs(code));
    return(-1);
}
