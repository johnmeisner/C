/*
 * Copyright (c) 2007-2018 Objective Systems, Inc.
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

      /* move forward */
      if (bitOffset < pctxt->buffer.bitOffset) {
         if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
            stat = rtxCheckInputBuffer (pctxt, 1);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);
         }

         pctxt->buffer.bitOffset =
            (OSINT16) (pctxt->buffer.bitOffset - bitOffset);

         if (pctxt->buffer.bitOffset == 0) {
            pctxt->buffer.bitOffset = 8;
            pctxt->buffer.byteIndex++;
         }
      }
      else {
         size_t skipOcts;
         size_t tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

         bitOffset -= pctxt->buffer.bitOffset;
         skipOcts = 1 + bitOffset / 8;

         if (skipOcts > tail) {
            skipOcts -= tail;

            /* skip data */
            stat = rtxStreamSkip (pctxt, skipOcts);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            /* empty buffer */
            pctxt->buffer.byteIndex = 0;
            pctxt->buffer.size = 0;
         }
         else
            pctxt->buffer.byteIndex += skipOcts;

         pctxt->buffer.bitOffset = (OSINT16) (8 - bitOffset % 8);
      }
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

EXTRTMETHOD int rtxDecBitsToSize (OSCTXT* pctxt, OSSIZE* pvalue, OSSIZE nbits)
{
   int stat;

   if (nbits == 0) {
      if (0 != pvalue) {
         *pvalue = 0;
      }
      return 0;
   }
   else if (nbits > sizeof(OSSIZE) * 8) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* If the number of bits is less than the current bit offset, mask   */
   /* off the required number of bits and return..                      */

   if (pctxt->buffer.bitOffset >= 0 &&
       nbits <= (OSSIZE)pctxt->buffer.bitOffset)
   {
      /* Check if buffer contains number of bits requested */

      if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
         stat = rtxCheckInputBuffer (pctxt, 1);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }

      pctxt->buffer.bitOffset =
         (OSINT16) (pctxt->buffer.bitOffset - nbits);

      if (0 != pvalue) {
         *pvalue = ((pctxt->buffer.data[pctxt->buffer.byteIndex]) >>
                    pctxt->buffer.bitOffset) & ((OSINTCONST(1) << nbits) - 1);
      }

      if (pctxt->buffer.bitOffset == 0) {
         pctxt->buffer.bitOffset = 8;
         pctxt->buffer.byteIndex++;
      }

      return 0;
   }

   /* Otherwise, we first need to mask off the remaining bits in the    */
   /* current byte, followed by a loop to extract bits from full bytes, */
   /* followed by logic to mask of remaining bits from the start of     */
   /* of the last byte..                                                */

   else {
      OSSIZE val;
      const OSOCTET* p;

      /* Check if buffer contains number of bits requested */

      OSSIZE nbytes = (nbits - pctxt->buffer.bitOffset + 15) / 8;

      if (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) {
         stat = rtxCheckInputBuffer (pctxt, nbytes);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }

      p = pctxt->buffer.data + pctxt->buffer.byteIndex;

      /* first read current byte remaining bits */
      val = (OSOCTET) (*p++ & ~(~0u << pctxt->buffer.bitOffset));

      nbits -= pctxt->buffer.bitOffset;
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex++;

      /* second read bytes from next byteIndex */
      while (nbits >= 8) {
         val <<= 8;
         val |= *p++;
         pctxt->buffer.byteIndex++;
         nbits -= 8;
      }

      /* third read bits & set bitoffset of the byteIndex */
      if (nbits > 0) {
         pctxt->buffer.bitOffset = (OSUINT16)(8 - nbits);
         val <<= nbits;
         val |= *p >> pctxt->buffer.bitOffset;
      }

      if (0 != pvalue) { *pvalue = val; }

      return 0;
   }
}

EXTRTMETHOD int rtxDecBits (OSCTXT* pctxt, OSUINT32* pvalue, OSSIZE nbits)
{
   if (nbits == 0) {
      if (0 != pvalue) {
         *pvalue = 0;
      }
   }
   else if (nbits > sizeof(OSUINT32) * 8) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else {
      OSSIZE tmpval;
      int ret = rtxDecBitsToSize (pctxt, &tmpval, nbits);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
      else if (0 != pvalue) *pvalue = (OSUINT32)tmpval;
   }
   return 0;
}

EXTRTMETHOD int rtxDecBitsToByte
(OSCTXT* pctxt, OSUINT8* pvalue, OSUINT8 nbits)
{
   if (nbits > 8) return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else if (nbits > 0) {
      OSUINT8 rshift = 8 - nbits;
      int stat = rtxDecBitsToByteArray (pctxt, pvalue, 1, nbits);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
      else { *pvalue >>= rshift; }
   }
   return 0;
}

EXTRTMETHOD int rtxDecBitsToUInt16
(OSCTXT* pctxt, OSUINT16* pvalue, OSUINT8 nbits)
{
   int stat;
   if (nbits > 16) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else if (0 == pvalue) {
      stat = rtxDecBits (pctxt, 0, nbits);
   }
   else {
      OSUINT32 tmp;
      stat = rtxDecBits (pctxt, &tmp, nbits);
      if (0 == stat) { *pvalue = (OSUINT16)tmp; }
   }
   return (0 == stat) ? 0 : LOG_RTERR (pctxt, stat);
}

EXTRTMETHOD int rtxDecBitsToByteArray (OSCTXT* pctxt, OSOCTET* pbuffer,
                                       OSSIZE bufsiz, OSSIZE nbits)
{
   OSUINT32 rshift = pctxt->buffer.bitOffset;
   OSUINT32 lshift = 8 - rshift;
   OSSIZE nbytes;
   int stat;

   if (nbits == 0)
      return 0;

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
            stat = rtxCheckInputBuffer (pctxt,
#ifndef _NO_STREAM
               OSRTMIN (fullOcts + 1, /* what if we've got a partial byte? */
                        (OSSIZE) (OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF)));
#else
               fullOcts + 1);
#endif
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            tail = pctxt->buffer.size - pctxt->buffer.byteIndex - 1;
            p = pctxt->buffer.data + pctxt->buffer.byteIndex;
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
