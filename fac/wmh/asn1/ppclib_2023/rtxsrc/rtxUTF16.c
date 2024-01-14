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
 * Changes made to original libxml source file (xmlregexp.c) by
 * Objective Systems, Inc are as follows:
 *
 * 1. Changed to fit Objective Systems run-time environment including
 * common type name changes and use of OSys mem mgmt.
 *
 * 2. Name change from
 *        UTF16LEToUTF8 to rtxUTF16LEToUTF8
 *        UTF8ToUTF16LE to rtxUTF8ToUTF16LE
 *        UTF8ToUTF16 to rtxUTF8ToUTF16
 *        UTF16BEToUTF8 to rtxUTF16BEToUTF8
 *        UTF8ToUTF16BE to rtxUTF8ToUTF16BE
 * to reflect fact that the code will be used in other non-XML environments.
 *
 * This revision reflects all changes till LIBXML2 version 2.7.3.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxUTF16.h"

static const OSUINT32 endian = 0x01020304;
static const OSOCTET *pEndian = (OSOCTET*)&endian;

/**
 * rtxUTF16LEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16LE passed as a byte array
 * @inlenb:  the length of @in in UTF-16LE chars
 *
 * Take a block of UTF-16LE ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 */
EXTRTMETHOD int
rtxUTF16LEToUTF8(unsigned char* out, int outlen,
            const unsigned char* inb, int inlenb)
{
   unsigned char* outstart = out;
   unsigned char* outend = out + outlen;
   unsigned int c, d;
   int idx = 0, bits;

   if ((inlenb % 2) == 1)
      (inlenb)--;

   while ((idx < inlenb) && (out - outstart + 5 < outlen)) {
      if (pEndian[0] == 4) { /* little endian */
         c = inb[idx] + (inb[idx+1] * 256);
      }
      else {
         c = (inb[idx] * 256) + inb[idx+1];
      }
      idx += 2;

      if ((c & 0xFC00) == 0xD800) {    /* surrogates */
         if (idx >= inlenb) {           /* (in > inend) shouldn't happens */
            break;
         }
         if (pEndian[0] == 4) { /* little endian */
            d = inb[idx] + (inb[idx+1] * 256);
         }
         else {
            d = (inb[idx] * 256) + inb[idx+1];
         }
         idx += 2;

         if ((d & 0xFC00) == 0xDC00) {
            c &= 0x03FF;
            c <<= 10;
            c |= d & 0x03FF;
            c += 0x10000;
         }
         else {
            return RTERR_INVCHAR;
         }
      }

      /* assertion: c is a single UTF-4 value */
      if (out >= outend)
         break;
      if      (c <    0x80) {  *out++= (unsigned char) c; bits= -6; }
      else if (c <   0x800) {  *out++= (unsigned char)
            (((c >>  6) & 0x1F) | 0xC0);  bits=  0; }
      else if (c < 0x10000) {  *out++= (unsigned char)
            (((c >> 12) & 0x0F) | 0xE0);  bits=  6; }
      else                  {  *out++= (unsigned char)
            (((c >> 18) & 0x07) | 0xF0);  bits= 12; }

      for ( ; bits >= 0; bits-= 6) {
         if (out >= outend)
            break;
         *out++= (unsigned char) (((c >> bits) & 0x3F) | 0x80);
      }
   }
   outlen = (int)(out - outstart);
   return (outlen);
}

/**
 * rtxUTF8ToUTF16LE:
 * @outb:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @outb
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16LE
 * block of chars out.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxUTF8ToUTF16LE(unsigned char* outb, int outlen,
            const unsigned char* in, int inlen)
{
    unsigned short* out = (unsigned short*) outb;
    unsigned short* outstart= out;
    unsigned short* outend;
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;
    unsigned char *tmp;
    unsigned short tmp1, tmp2;

    /* UTF16LE encoding has no BOM */
    if ((out == NULL) || (outlen == 0) || (inlen == 0)) return RTERR_BUFOVFLW;
    if (in == NULL) {
	return(0);
    }
    inend= in + inlen;
    outend = out + (outlen / 2);
    while (in < inend) {
      d= *in++;
      if      (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0) {
          /* trailing byte in leading position */
          return RTERR_INVUTF8;
      } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in UTF-16 */
         return RTERR_INVCHAR;
      }

      if (inend - in < trailing) {
          break;
      }

      for ( ; trailing; trailing--) {
          if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
	      break;
          c <<= 6;
          c |= d & 0x3F;
      }

      /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            if (out >= outend)
	        break;
            if (pEndian[0] == 4) { /* little endian */
		*out++ = (unsigned char)c;
	    } else {
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) c ;
		*(tmp + 1) = (unsigned char) (c >> 8) ;
		out++;
	    }
        }
        else if (c < 0x110000) {
            if (out+1 >= outend)
	        break;
            c -= 0x10000;
            if (pEndian[0] == 4) { /* little endian */
		*out++ = (unsigned char) (0xD800 | (c >> 10));
		*out++ = (unsigned char) (0xDC00 | (c & 0x03FF));
	    } else {
		tmp1 = (unsigned short) (0xD800 | (c >> 10));
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) tmp1;
		*(tmp + 1) = (unsigned char) (tmp1 >> 8);
		out++;

		tmp2 = (unsigned short) (0xDC00 | (c & 0x03FF));
		tmp = (unsigned char *) out;
		*tmp  = (unsigned char) tmp2;
		*(tmp + 1) = (unsigned char) (tmp2 >> 8);
		out++;
	    }
        }
        else
	    break;
    }
    outlen = (int) (out - outstart) * 2;
    return(outlen);
}

/**
 * rtxUTF8ToUTF16:
 * @outb:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @outb
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16
 * block of chars out.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxUTF8ToUTF16(unsigned char* outb, int outlen,
            const unsigned char* in, int inlen)
{
    if (in == NULL) {
	/*
	 * initialization, add the Byte Order Mark for UTF-16LE
	 */
        if (outlen >= 2) {
	    outb[0] = 0xFF;
	    outb[1] = 0xFE;
#ifdef DEBUG_ENCODING
            xmlGenericError(xmlGenericErrorContext,
		    "Added FFFE Byte Order Mark\n");
#endif
	    return(2);
	}
	return(0);
    }
    return (rtxUTF8ToUTF16LE(outb, outlen, in, inlen));
}

/**
 * rtxUTF16BEToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @inb:  a pointer to an array of UTF-16 passed as a byte array
 * @inlenb:  the length of @in in UTF-16 chars
 *
 * Take a block of UTF-16 ushorts in and try to convert it to an UTF-8
 * block of chars out. This function assumes the endian property
 * is the same between the native type of this machine and the
 * inputed one.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding fails (if *in is not a valid utf16 string)
 *     if the return value is positive, else unpredictable.
 */
EXTRTMETHOD int
rtxUTF16BEToUTF8(unsigned char* out, int outlen,
            const unsigned char* inb, int inlenb)
{
    unsigned char* outstart = out;
    unsigned char* outend = out + outlen;
    unsigned short* in = (unsigned short*) inb;
    unsigned short* inend;
    unsigned int c, d, inlen;
    unsigned char *tmp;
    int bits;

    if ((inlenb % 2) == 1)
        (inlenb)--;
    inlen = inlenb / 2;
    inend= in + inlen;
    while (in < inend) {
        if (pEndian[0] == 4) { /* little endian */
	    tmp = (unsigned char *) in;
	    c = *tmp++;
	    c = c << 8;
	    c = c | (unsigned int) *tmp;
	    in++;
	} else {
	    c= *in++;
	}
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
	    if (in >= inend) {           /* (in > inend) shouldn't happens */
               return RTERR_INVCHAR;
	    }
            if (pEndian[0] == 4) { /* little endian */
		tmp = (unsigned char *) in;
		d = *tmp++;
		d = d << 8;
		d = d | (unsigned int) *tmp;
		in++;
	    } else {
		d= *in++;
	    }
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            }
            else {
               return RTERR_INVCHAR;
	    }
        }

	/* assertion: c is a single UTF-4 value */
        if (out >= outend)
	    break;
        if      (c <    0x80) {  *out++= (unsigned char) c; bits= -6; }
        else if (c <   0x800) {  *out++= (unsigned char)
                                       (((c >>  6) & 0x1F) | 0xC0);  bits= 0; }
        else if (c < 0x10000) {  *out++= (unsigned char)
                                       (((c >> 12) & 0x0F) | 0xE0);  bits= 6; }
        else                  {  *out++= (unsigned char)
                                       (((c >> 18) & 0x07) | 0xF0);  bits= 12; }

        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend)
	        break;
            *out++= (unsigned char) (((c >> bits) & 0x3F) | 0x80);
        }
    }
    outlen = (int) (out - outstart);
    return(outlen);
}

/**
 * rtxUTF8ToUTF16BE:
 * @outb:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @outb
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16BE
 * block of chars out.
 *
 * Returns the number of byte written, or -1 by lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxUTF8ToUTF16BE(unsigned char* outb, int outlen,
            const unsigned char* in, int inlen)
{
    unsigned short* out = (unsigned short*) outb;
    unsigned short* outstart= out;
    unsigned short* outend;
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;
    unsigned char *tmp;
    unsigned short tmp1, tmp2;

    /* UTF-16BE has no BOM */
    if ((outb == NULL) || (outlen == 0) || (inlen == 0)) return RTERR_BUFOVFLW;
    if (in == NULL) {
	return(0);
    }

    inend= in + inlen;
    outend = out + (outlen / 2);
    while (in < inend) {
      d= *in++;
      if      (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0)  {
         /* trailing byte in leading position */
         return RTERR_INVUTF8;
      } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in UTF-16 */
         return RTERR_INVCHAR;
      }

      if (inend - in < trailing) {
          break;
      }

      for ( ; trailing; trailing--) {
          if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))  break;
          c <<= 6;
          c |= d & 0x3F;
      }

      /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            if (out >= outend)  break;
            if (pEndian[0] == 4) { /* little endian */
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) (c >> 8);
		*(tmp + 1) = (unsigned char) c;
		out++;
	    } else {
		*out++ = (unsigned char) c;
	    }
        }
        else if (c < 0x110000) {
            if (out+1 >= outend)  break;
            c -= 0x10000;
            if (pEndian[0] == 4) { /* little endian */
		tmp1 = (unsigned short) (0xD800 | (c >> 10));
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) (tmp1 >> 8);
		*(tmp + 1) = (unsigned char) tmp1;
		out++;

		tmp2 = (unsigned short) (0xDC00 | (c & 0x03FF));
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) (tmp2 >> 8);
		*(tmp + 1) = (unsigned char) tmp2;
		out++;
	    } else {
		*out++ = (unsigned char) (0xD800 | (c >> 10));
		*out++ = (unsigned char) (0xDC00 | (c & 0x03FF));
	    }
        }
        else
	    break;
    }
    outlen = (int)(out - outstart) * 2;
    return(outlen);
}

/**
 * rtxStreamUTF8ToUTF16LE:
 * @pctxt:  a pointer to context block structure
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16LE
 * block of chars, and write the converted chars to stream.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxStreamUTF8ToUTF16LE
(OSCTXT* pctxt, const unsigned char* in, size_t inlen)
{
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;
    unsigned short tmp1, tmp2;
    size_t outlen = 0;

    /* UTF16LE encoding has no BOM */
    if (inlen == 0) return RTERR_INVCHAR;
    if (in == NULL) return(0);

    inend= in + inlen;
    while (in < inend) {
      d= *in++;
      if      (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0) {
         /* trailing byte in leading position */
         return RTERR_INVUTF8;
      } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in UTF-16 */
         return RTERR_INVCHAR;
      }

      if (inend - in < trailing) {
          break;
      }

      for ( ; trailing; trailing--) {
          if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
	      break;
          c <<= 6;
          c |= d & 0x3F;
      }

      /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            OSRTSAFEPUTCHAR (pctxt, (unsigned char)c);
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (c >> 8));
            outlen += 2;
        }
        else if (c < 0x110000) {
            c -= 0x10000;
            tmp1 = (unsigned short) (0xD800 | (c >> 10));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) tmp1);
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (tmp1 >> 8));

            tmp2 = (unsigned short) (0xDC00 | (c & 0x03FF));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) tmp2);
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (tmp2 >> 8));

            outlen += 4;
        }
        else
	    break;
    }
    return((int)outlen);
}

/**
 * rtxStreamUTF8ToUTF16:
 * @pctxt:  a pointer to context block structure
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16
 * block of chars out. write it to stream.
 *
 * Returns the number of bytes written, or -1 if lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxStreamUTF8ToUTF16(OSCTXT* pctxt, const unsigned char* in, size_t inlen)
{
    if (in == NULL) {
	/*
	 * initialization, add the Byte Order Mark for UTF-16LE
	 */
/*	OSRTSAFEPUTCHAR (pctxt, 0xFF);
	OSRTSAFEPUTCHAR (pctxt, 0xFE);
#ifdef DEBUG_ENCODING
        xmlGenericError(xmlGenericErrorContext,
		    "Added FFFE Byte Order Mark\n");
#endif
	return(2);
*/
        return (0);
    }
    return (rtxStreamUTF8ToUTF16LE(pctxt, in, inlen));
}

/**
 * rtxStreamUTF8ToUTF16BE:
 * @pctxt:  a pointer to context block structure
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 *
 * Take a block of UTF-8 chars in and try to convert it to an UTF-16BE
 * block of chars out. Write it to stream.
 *
 * Returns the number of byte written, or -1 by lack of space, or -2
 *     if the transcoding failed.
 */
EXTRTMETHOD int
rtxStreamUTF8ToUTF16BE(OSCTXT* pctxt, const unsigned char* in, size_t inlen)
{
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;
    unsigned short tmp1, tmp2;
    size_t outlen = 0;

    /* UTF-16BE has no BOM */
    if (inlen == 0) return RTERR_INVCHAR;
    if (in == NULL) return(0);

    inend= in + inlen;
    while (in < inend) {
      d= *in++;
      if      (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0)  {
         /* trailing byte in leading position */
         return RTERR_INVUTF8;
      } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
         /* no chance for this in UTF-16 */
         return RTERR_INVCHAR;
      }

      if (inend - in < trailing) {
          break;
      }

      for ( ; trailing; trailing--) {
          if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))  break;
          c <<= 6;
          c |= d & 0x3F;
      }

      /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (c >> 8));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) c);
            outlen += 2;
        }
        else if (c < 0x110000) {
            c -= 0x10000;
            tmp1 = (unsigned short) (0xD800 | (c >> 10));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (tmp1 >> 8));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) tmp1);

            tmp2 = (unsigned short) (0xDC00 | (c & 0x03FF));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) (tmp2 >> 8));
            OSRTSAFEPUTCHAR (pctxt, (unsigned char) tmp2);

            outlen += 4;
        }
        else
	    break;
    }
    return((int)outlen);
}
