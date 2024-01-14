/*
 * Copyright (c) 2007-2023 Objective Systems, Inc.
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

#include "rtxsrc/rtxBitDecode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxStream.h"

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test flushing */
#endif

/* This macro calculates the relative bit offset to the current buffer
   position. */

#define PU_GETCTXTBITOFFSET(pctxt) \
(((pctxt)->buffer.byteIndex * 8) + (8 - (pctxt)->buffer.bitOffset))

/* This macro sets the bit offset to the given value within the context */

#define PU_SETCTXTBITOFFSET(pctxt,_bitOffset) \
do { \
(pctxt)->buffer.byteIndex = (_bitOffset / 8); \
(pctxt)->buffer.bitOffset = (OSUINT16)(8 - (_bitOffset % 8)); \
} while(0)

EXTRTMETHOD int rtxMoveBitCursor (OSCTXT* pctxt, int bitOffset)
{
#ifndef _NO_STREAM
   int stat;

   if (OSRTISSTREAM (pctxt)) {
      if (bitOffset < 0) {
         /* move backward */
         size_t currBitOffset = PU_GETCTXTBITOFFSET (pctxt);

         if (currBitOffset < (size_t) -bitOffset)
            return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

         currBitOffset += bitOffset;
         PU_SETCTXTBITOFFSET (pctxt, currBitOffset);

         return 0;
      }
      else if (bitOffset == 0)
         return 0;

      /* moving forward */
      if (bitOffset >= pctxt->buffer.bitOffset) {
         /* We're skipping all of the remaining bits in the current byte,
            and possibly more. */
         size_t skipOcts;

         if (pctxt->buffer.bitOffset < 8) {
            /* Skip the remaining bits in the current byte. */
            bitOffset -= pctxt->buffer.bitOffset;
            pctxt->buffer.bitOffset = 8;
            pctxt->buffer.byteIndex++;
         }

         skipOcts = bitOffset / 8;   /* number of whole octets left to skip */
         if (skipOcts > 0) {
            stat = rtxStreamSkip(pctxt, skipOcts);
            if (stat < 0) return LOG_RTERR(pctxt, stat);
         }

         bitOffset = bitOffset % 8; /* remaining bits to skip */

         if (bitOffset == 0) return 0;
      }

      /* Now, 0 < bitOffset < 8 and bitOffset < pctxt->buffer.bitOffset
         Move into the current byte, first making sure there is a current byte.
      */
      if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
         stat = rtxCheckInputBuffer (pctxt, 1);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }

      pctxt->buffer.bitOffset =
            (OSINT16) (pctxt->buffer.bitOffset - bitOffset);

   }
   else
#endif /* _NO_STREAM */
   { /* input from buffer */
      size_t currBitOffset = PU_GETCTXTBITOFFSET (pctxt);

      if (bitOffset < 0 && currBitOffset < (size_t) -bitOffset)
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

      currBitOffset += bitOffset;
      PU_SETCTXTBITOFFSET (pctxt, currBitOffset);

      if (pctxt->buffer.byteIndex > pctxt->buffer.size ||
          (pctxt->buffer.byteIndex == pctxt->buffer.size &&
           pctxt->buffer.bitOffset != 8))
      {
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
      }
   }

   return 0;
}

EXTRTMETHOD int rtxDecBit (OSCTXT* pctxt, OSBOOL* pvalue)
{
   int stat;

   if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
      stat = rtxCheckInputBuffer (pctxt, 1);
      if (stat < 0)
         return LOG_RTERR (pctxt, stat);
   }

   pctxt->buffer.bitOffset--;

   if (pvalue)
      *pvalue = (OSOCTET) ((pctxt->buffer.data[pctxt->buffer.byteIndex] >>
         pctxt->buffer.bitOffset) & 1);

   if (pctxt->buffer.bitOffset == 0) {
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;
   }

   return 0;
}

/* Decode bits to byte */

EXTRTMETHOD int rtxDecBitsToByte
(OSCTXT* pctxt, OSUINT8* pvalue, OSUINT8 nbits)
{
   if (nbits > 8) return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else if (nbits == 8 && pctxt->buffer.bitOffset == 8 &&
            pctxt->buffer.byteIndex < pctxt->buffer.size) {
      /* optimization to fetch byte directly from context buffer */
      if (pvalue) *pvalue = OSRTBUFCUR(pctxt);
      pctxt->buffer.byteIndex++;
   }
   else if (nbits > 0) {
      OSUINT8 rshift = 8 - nbits;
      int stat = rtxDecBitsToByteArray (pctxt, pvalue, 1, nbits);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else if (pvalue) { *pvalue >>= rshift; }
   }
   else if (pvalue) { /* nbits is zero */
      *pvalue = 0;
   }

   return 0;
}

/* Decode bits to a 16-bit unsigned integer */

#define RTXDECBITSFN rtxDecBitsToUInt16
#define RTXDECBITSINTTYPE OSUINT16
#include "rtxBitDecTmpl.c"

/* Decode bits to a 32-bit unsigned integer */

#define RTXDECBITSFN rtxDecBits
#define RTXDECBITSINTTYPE OSUINT32
#include "rtxBitDecTmpl.c"

/* Decode bits to a 64-bit unsigned integer */

#define RTXDECBITSFN rtxDecBitsToUInt64
#define RTXDECBITSINTTYPE OSUINT64
#include "rtxBitDecTmpl.c"

/* Decode bits to size type */

#define RTXDECBITSFN rtxDecBitsToSize
#define RTXDECBITSINTTYPE OSSIZE
#include "rtxBitDecTmpl.c"

/* Decode bits to byte array */

EXTRTMETHOD int rtxDecBitsToByteArray (OSCTXT* pctxt, OSOCTET* pbuffer,
                                       OSSIZE bufsiz, OSSIZE nbits)
{
   OSUINT32 rshift = pctxt->buffer.bitOffset;
   OSUINT32 lshift = 8 - rshift;
   OSSIZE nbytes;
   int stat;

   if (nbits == 0)
      return 0;

   if (!pbuffer) {
      return rtxMoveBitCursor(pctxt, (int)nbits);
   }

   /* Check to make sure buffer is big enough to hold requested         */
   /* number of bits..                                                  */

   nbytes = nbits / 8;
   if (nbits % 8 != 0) nbytes++;
   if (nbytes > bufsiz)
      return LOG_RTERR (pctxt, RTERR_STROVFLW);

   if (rshift == 8) { /* aligned */
      OSSIZE tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
      OSSIZE n = nbits / 8;

      if (tail > n)
         tail = n;

      if (tail > 0) {
         OSCRTLSAFEMEMCPY (pbuffer, bufsiz, OSRTBUFPTR(pctxt), tail);
         n -= tail;
         pbuffer += tail;
         pctxt->buffer.byteIndex += tail;
      }

      if (0 != n) {
#ifndef _NO_STREAM
         if (OSRTISSTREAM (pctxt)) {
            if (n <= OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF) {
               /* refill buffer */
               stat = rtxCheckInputBuffer (pctxt, n);
               if (stat < 0)
                  return LOG_RTERR (pctxt, stat);

               OSCRTLSAFEMEMCPY (pbuffer, bufsiz, OSRTBUFPTR(pctxt), n);
               pctxt->buffer.byteIndex += n;
               pbuffer += n;
            }
            else {
               n -= MIN_STREAM_BACKOFF;
               pctxt->pStream->bytesProcessed += n + pctxt->buffer.byteIndex;
               pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;

               while (n) {
                  /* read data */
                  stat = rtxStreamRead (pctxt, pbuffer, n);

                  if (stat == 0)
                     return LOG_RTERR (pctxt, RTERR_ENDOFFILE);
                  else if (stat < 0)
                     return LOG_RTERR (pctxt, stat);

                  n -= (OSUINT32) stat;
                  pbuffer += stat;
               }

#if defined(MIN_STREAM_BACKOFF) && (MIN_STREAM_BACKOFF > 0)
               /* clear buffer */
               pctxt->buffer.size = 0;
               pctxt->buffer.byteIndex = 0;

               /* refill buffer */
               stat = rtxCheckInputBuffer (pctxt, MIN_STREAM_BACKOFF);
               if (stat < 0)
                  return LOG_RTERR (pctxt, stat);

               OSCRTLSAFEMEMCPY (pbuffer, bufsiz, OSRTBUFPTR(pctxt),
                                 MIN_STREAM_BACKOFF);

               pctxt->buffer.byteIndex += MIN_STREAM_BACKOFF;
               pbuffer += MIN_STREAM_BACKOFF;
#endif
            }
         }
         else
#endif /* _NO_STREAM */
            return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
      }

      nbits &= 0x7; /* nbits %= 8 */

      if (nbits) {
         pctxt->buffer.bitOffset = (OSUINT16)(8 - nbits);
         tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

         if (tail == 0) { /* read data */
            stat = rtxCheckInputBuffer (pctxt, 1);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);
         }

         *pbuffer = (OSOCTET) (OSRTBUFCUR(pctxt) & (0xFF00u >> nbits));
      }
   }
   else { /* unaligned */
      OSSIZE tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
      OSSIZE fullOcts = nbits / 8;
      const OSOCTET* p;
      OSOCTET prevOct;
#ifndef _NO_STREAM
      if (tail <= 1) { /* read data */
         stat = rtxCheckInputBuffer (pctxt,
            OSRTMIN (fullOcts + 1,
                     (OSSIZE) (OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF)));

         if (stat < 0)
            return LOG_RTERR (pctxt, stat);

         tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
      }
#endif
      tail--;
      p = pctxt->buffer.data + pctxt->buffer.byteIndex;
      prevOct = *p++;

      while (fullOcts) {
         if (tail > fullOcts)
            tail = fullOcts;

         fullOcts -= tail;
         pctxt->buffer.byteIndex += tail;

         while (tail--) {
            OSOCTET tm = *p++;

            *pbuffer++ = (OSOCTET) (prevOct << lshift | tm >> rshift);
            prevOct = tm;
         }

         if (fullOcts) {
            OSUINT8 pad = (pctxt->buffer.bitOffset > 0) ? 1 : 0;
                /* handle partial byte */

            stat = rtxCheckInputBuffer (pctxt,
#ifndef _NO_STREAM
               OSRTMIN (fullOcts + 1,
                        (OSSIZE) (OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF)));
#else
               fullOcts + 1);
#endif
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            tail = pctxt->buffer.size - pctxt->buffer.byteIndex - 1;
            p = pctxt->buffer.data + pctxt->buffer.byteIndex + pad;
         }
      }

      nbits &= 0x7; /* nbits %= 8 */

      if (nbits) {
         pctxt->buffer.bitOffset = (OSUINT16) (8 - ((lshift + nbits) & 7));

         if (rshift >= nbits) {
            *pbuffer = (OSOCTET) ((prevOct << lshift) & (0xFF00u >> nbits));

            if (rshift == nbits)
               pctxt->buffer.byteIndex++;
         }
         else {
            tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

            if (tail <= 1) { /* read data */
               stat = rtxCheckInputBuffer (pctxt, 2);
               if (stat < 0)
                  return LOG_RTERR (pctxt, stat);
            }

            pctxt->buffer.byteIndex++;
            p = pctxt->buffer.data + pctxt->buffer.byteIndex;

            *pbuffer = (OSOCTET) (((prevOct << lshift) | (*p >> rshift)) &
                                  (0xFF00u >> nbits));
         }
      }
   }

   LCHECKX (pctxt);

   return 0;
}

EXTRTMETHOD int rtxPeekBit (OSCTXT* pctxt, OSBOOL* pvalue)
{
   int ret = rtxDecBit (pctxt, pvalue);
   if (0 == ret) {
      ret = rtxMoveBitCursor (pctxt, -1);
   }
   return ret;
}

EXTRTMETHOD int rtxSkipBits (OSCTXT* pctxt, OSSIZE nbits)
{
   return rtxMoveBitCursor (pctxt, (int)nbits);
}

EXTRTMETHOD int rtxSkipBytes(OSCTXT* pctxt, OSSIZE nbytes)
{
#ifndef _NO_STREAM
   int stat;

   if (nbytes == 0) return 0;

   if (OSRTISSTREAM(pctxt)) {
      /* rtxStreamSkip will behave as if buffer.bitOffset == 8.  It will
         advance buffer.byteIndex to the correct value.  The bitOffset
         will remain unchanged.  If the bitOffset != 8, we just need to
         make sure there is a current byte after skipping.
      */
      stat = rtxStreamSkip(pctxt, nbytes);
      if (stat < 0) return LOG_RTERR(pctxt, stat);

      if (pctxt->buffer.bitOffset != 8 &&
            pctxt->buffer.byteIndex >= pctxt->buffer.size)
      {
         stat = rtxCheckInputBuffer(pctxt, 1);
         if (stat < 0)
            return LOG_RTERR(pctxt, stat);
      }
   }
   else
#endif /* _NO_STREAM */
   { /* input from buffer */
      pctxt->buffer.byteIndex += nbytes;

      if (pctxt->buffer.byteIndex > pctxt->buffer.size ||
         (pctxt->buffer.byteIndex == pctxt->buffer.size &&
            pctxt->buffer.bitOffset != 8))
      {
         return LOG_RTERR(pctxt, RTERR_ENDOFBUF);
      }
   }

   return 0;
}
