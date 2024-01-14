/*
 * encoding.c : implements the encoding conversion functions needed for XML
 *
 * Related specs:
 * rfc2044        (UTF-8 and UTF-16) F. Yergeau Alis Technologies
 * rfc2781        UTF-16, an encoding of ISO 10646, P. Hoffman, F. Yergeau
 * [ISO-10646]    UTF-8 and UTF-16 in Annexes
 * [ISO-8859-1]   ISO Latin-1 characters codes.
 * [UNICODE]      The Unicode Consortium, "The Unicode Standard --
 *                Worldwide Character Encoding -- Version 1.0", Addison-
 *                Wesley, Volume 1, 1991, Volume 2, 1992.  UTF-8 is
 *                described in Unicode Technical Report #4.
 * [US-ASCII]     Coded Character Set--7-bit American Standard Code for
 *                Information Interchange, ANSI X3.4-1986.
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 *
 * Original code for IsoLatin1 and UTF-16 by "Martin J. Duerst" <duerst@w3.org>
 */
/*
NOTE: the copyright notice below applies only to source and header files
that include this notice.  It does not apply to other Objective Systems
software with different attached notices.

Except where otherwise noted in the source code (e.g. the files hash.c,
list.c and the trio files, which are covered by a similar licence but
with different Copyright notices) all the files are:

 Copyright (C) 1998-2003 Daniel Veillard.  All Rights Reserved.

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
 * Changes made to original libxml source file (xmlregexp.c) by
 * Objective Systems, Inc are as follows:
 *
 * 1. Changed to fit Objective Systems run-time environment including
 * common type name changes and use of OSys mem mgmt.
 *
 * 2. Name change from UTF8Toisolat1 to rtxUTF8ToLatin1, 
 * isolat1ToUTF8 to rtxLatin1ToUTF8, to reflect fact that the
 * code will be used in other non-XML environments.
 *
 * This revision reflects all changes till LIBXML2 version 2.7.3.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxLatin1.h"

/***********************************************************************
 *
 *  Routine name: rtxUTF8ToLatin1
 *
 *  Description:  This routine converts a UTF-8 encoded byte stream 
 *                into an ISO 8859-1 encoded string.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ------------    ----------- 
 *  inbuf       OSUTF8CHAR*     String of UTF-8 encoded characters
 *  inlen       int             Number of bytes of the input string
 *  outbufsiz   int             Number of latin 1 characters the output
 *                                buffer can hold.
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  stat        int             Character count or error status.  
 *  outbuf      OSUTF8CHAR*     Output buffer
 *
 ***********************************************************************/

EXTRTMETHOD int rtxUTF8ToLatin1
(const OSUTF8CHAR* inbuf, int inlen, OSUTF8CHAR* outbuf, int outbufsiz)
{
   int outlen = 0;

   const unsigned char* outend;
   const unsigned char* outstart = outbuf;
   const unsigned char* inend;
   unsigned int c, d;
   int trailing;

   if ((outbuf == NULL) || (outbufsiz == 0)) return RTERR_BUFOVFLW;
   if (inbuf == NULL) {
      /*
       * initialization nothing to do
       */
      return(0);
   }
   inend = inbuf + inlen;
   outend = outbuf + outbufsiz;
   while (inbuf < inend) {
      d = *inbuf++;
      if (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0) {
         /* trailing byte in leading position */
         return RTERR_INVUTF8;
      } 
      else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in IsoLat1 */
         return RTERR_INVCHAR;
      }

      if (inend - inbuf < trailing) {
         break;
      }

      for ( ; trailing; trailing--) {
         if (inbuf >= inend)
            break;
         if (((d= *inbuf++) & 0xC0) != 0x80) {
            return RTERR_INVCHAR;
         }
         c <<= 6;
         c |= d & 0x3F;
      }
      /* assertion: c is a single UTF-4 value */
      if (c <= 0xFF) {
         if (outbuf >= outend)
            break;
         *outbuf++ = (OSUTF8CHAR)c;
      } else {
         /* no chance for this in IsoLat1 */
         return RTERR_INVCHAR;
      }
   }
   outlen = (int) (outbuf - outstart);
   return (outlen);
}

/***********************************************************************
 *
 *  Routine name: rtxLatin1ToUTF8
 *
 *  Description:  This routine converts an ISO 8859-1 encoded string 
 *                into a UT-8 encoded byte stream.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ------------    ----------- 
 *  inbuf       OSUTF8CHAR*     String of Latin 1 characters
 *  inlen       int             Number of Latin 1 characters to be converted
 *  outbufsiz   int             Number of bytes the output buffer can hold
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  stat        int             Character count or error status.  
 *  outbuf      OSUTF8CHAR*     Output buffer
 *
 ***********************************************************************/

EXTRTMETHOD int rtxLatin1ToUTF8
(const OSUTF8CHAR* inbuf, int inlen, OSUTF8CHAR* outbuf, int outbufsiz)
{
   int outlen = 0;

   unsigned char* outstart = outbuf;
   unsigned char* outend;
   const unsigned char* inend;
   const unsigned char* instop;

   if ((outbuf == NULL) || (inbuf == NULL) || (outbufsiz == 0))
      return RTERR_BUFOVFLW;

   outend = outbuf + outbufsiz;
   inend = inbuf + inlen;
   instop = inend;

   while (inbuf < inend && outbuf < outend - 1) {
      if (*inbuf >= 0x80) {
         *outbuf++ = (OSUTF8CHAR) ((((*inbuf) >>  6) & 0x1F) | 0xC0);
         *outbuf++ = (OSUTF8CHAR) (((*inbuf) & 0x3F) | 0x80);
         ++inbuf;
      }
      if (instop - inbuf > outend - outbuf) instop = inbuf + (outend - outbuf);
      while (inbuf < instop && *inbuf < 0x80) {
         *outbuf++ = *inbuf++;
      }
   }
   if (inbuf < inend && outbuf < outend && *inbuf < 0x80) {
        *outbuf++ = *inbuf++;
   }
   outlen = (int) (outbuf - outstart);
   return (outlen);
}

/***********************************************************************
 *
 *  Routine name: rtxStreamUTF8ToLatin1
 *
 *  Description:  This routine converts a UTF-8 encoded byte stream 
 *                into an ISO 8859-1 encoded string, and write it to
 *                stream.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ------------    ----------- 
 *  pctxt       struct          Pointer to context block structure
 *  inbuf       OSUTF8CHAR*     String of UTF-8 encoded characters
 *  inlen       size_t          Number of bytes of the input string
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  stat        int             Character count or error status.  
 *
 ***********************************************************************/

EXTRTMETHOD int rtxStreamUTF8ToLatin1
(OSCTXT* pctxt, const OSUTF8CHAR* inbuf, size_t inlen)
{
   int outlen = 0;

   const unsigned char* inend;
   unsigned int c, d;
   int trailing;

   if (inbuf == NULL) {
      /*
       * initialization nothing to do
       */
      return(0);
   }
   inend = inbuf + inlen;
   while (inbuf < inend) {
      d = *inbuf++;
      if (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0) {
         /* trailing byte in leading position */
         return RTERR_INVUTF8;
      } 
      else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in IsoLat1 */
         return RTERR_INVCHAR;
      }

      if (inend - inbuf < trailing) {
         break;
      }

      for ( ; trailing; trailing--) {
         if (inbuf >= inend)
            break;
         if (((d= *inbuf++) & 0xC0) != 0x80) {
            return RTERR_INVCHAR;
         }
         c <<= 6;
         c |= d & 0x3F;
      }
      /* assertion: c is a single UTF-4 value */
      if (c <= 0xFF) {
         OSRTSAFEPUTCHAR (pctxt, (OSUTF8CHAR)c);
         outlen ++;
      } else {
         /* no chance for this in IsoLat1 */
         return RTERR_INVCHAR;
      }
   }
   return (outlen);
}

