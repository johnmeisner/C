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

#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxStream.h"

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test flushing */
#endif

/* define rotate bits macro. MS VC before 2005 will use _rotl intrinsic. */
#if defined(_MSC_VER) && _MSC_VER < 1400 && defined(_M_IX86)
#define ROTL(value, shift) _rotl (value, shift)
#else
#define ROTL(value, shift) (value >> (32 - shift)) | (value << (shift))
#endif

EXTRTMETHOD int rtxEncBitsPattern(OSCTXT* pctxt, OSUINT8 pattern, OSSIZE nbits)
{
   int result = 0;

   /* pattern, repeated in 32 bits and aligned properly for use with
      rtxEncBits */
   OSUINT32 pattern32;
   OSBOOL bRotate = FALSE; /* true when rotating has a real effect */

   if ( nbits == 0 ) return 0;

   if ( pattern == 0x00 ) pattern32 = 0x00;
   else if (pattern == 0xFF ) pattern32 = 0xFFFFFFFF;
   else {
      pattern32 = (pattern << 24) | (pattern << 16) | (pattern << 8 )
                  | pattern;
      bRotate = TRUE;
   }

   if ( nbits >= 32 ) {
      /*
      We're going to encode 32 bits of the pattern until we have fewer than
      32 bits to encode.  If we rotate pattern left 8 - bitOffset, the pattern
      will be aligned so that the highest bit is the first bit to encode.
      */

      if ( bRotate )
         pattern32 = ROTL(pattern32, 8 - pctxt->buffer.bitOffset);

      do {
         result = rtxEncBits(pctxt, pattern32, 32);
         if ( result < 0 ) return LOG_RTERR(pctxt, result);
         nbits -= 32;
      } while ( nbits >= 32 );

      /*
      Prepapre pattern32 to be used for encoding remaining bits.
      rotate pattern32 left by remaining # of bits so what is currently the
      highest bit in the pattern (and would be the next bit to encode) will
      align with the highest bit rtxEncBits will encode.
      */
      if ( nbits > 0 )
         pattern32 = ROTL(pattern32, nbits);
   }
   else {
      /*
      The alignment of pattern is the same as above, except that it is done
      in a single rotation instead of a rotation to align for the case when
      all 32 bits will be used and then a rotation to align for the case when
      only some number of the lower bits will be used.
      */
      if ( bRotate )
         pattern32 = ROTL(pattern32, 8 - pctxt->buffer.bitOffset + nbits);
   }

   if ( nbits > 0 ) {
      result = rtxEncBits(pctxt, pattern32, nbits);
      if ( result < 0 ) return LOG_RTERR(pctxt, result);
   }

   return result;
}


EXTRTMETHOD int rtxEncBit (OSCTXT* pctxt, OSBOOL value)
{
   OSSIZE byteIndex = pctxt->buffer.byteIndex;
   unsigned bitOffset = pctxt->buffer.bitOffset - 1;
   OSOCTET mask;
   OSBOOL lvalue = (value != 0) ? 1 : 0;

   mask = (OSOCTET) (lvalue << bitOffset);

   if (bitOffset == 7) {
      if (byteIndex >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         byteIndex = pctxt->buffer.byteIndex;
      }

      pctxt->buffer.data[byteIndex] = mask;
   }
   else
      pctxt->buffer.data[byteIndex] |= mask;

   bitOffset = ((bitOffset - 1) & 7) + 1;
   pctxt->buffer.bitOffset = (OSINT16) bitOffset;
   pctxt->buffer.byteIndex = byteIndex + (bitOffset >> 3);

   return 0;
}

EXTRTMETHOD int rtxEncBits (OSCTXT* pctxt, OSUINT32 value, OSSIZE nbits)
{
   /* The lowest nbits of value get encoded. */

   OSSIZE idx = pctxt->buffer.byteIndex;
   unsigned bitOff = pctxt->buffer.bitOffset;
   OSSIZE nmBytes;
   OSOCTET* p;

   if (nbits > sizeof(value)*8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (nbits == 0)
      return 0;

   if (bitOff == 8) {
      /* field aligned */

      /* nmBytes is # of bytes we'll write into beyond the current one */
      nmBytes = (nbits - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      pctxt->buffer.byteIndex = idx + (nbits >> 3);
      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nbits & 7));

      value <<= 32 - nbits; /* clear unused bits */

      value = ROTL (value, 8);

      while (nmBytes-- > 0) {
         *p++ = (OSOCTET) value;
         value = ROTL (value, 8);
      }

      *p = (OSOCTET) value;
   }
   else {
      /* field unaligned */

      /* If we number the bits in the buffer, starting with the highest bit
         in the current byte, beginning with 0, then nextOff identifies the
         bit position after nbits bits have been written.  Put another way,
         it is the number of bits we'll advance from the start of the current
         byte.
      */
      OSSIZE nextOff = nbits + 8 - bitOff;
      OSUINT32 mask;

      /* nmBytes is the number of bytes we'll write into beyond the current
         byte.  If nmBytes == 0, we will only touch the current byte.
      */
      nmBytes = (nextOff - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      value <<= 32 - nbits; /* clear unused bits */
      value = ROTL (value, bitOff);

      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nextOff & 0x7));

      /* nextOff now becomes the number of bytes we will advance from the
         current byte. */
      nextOff >>= 3;
      pctxt->buffer.byteIndex = idx + nextOff;

      /* Now update the current byte, and the next nmBytes bytes. */

      mask = ~0u << bitOff;
      *p = (*p & mask) | (OSOCTET) (value & ~mask);
      p++;
      value &= mask;

      while (nmBytes-- > 0) {
         value = ROTL (value, 8);
         *p++ = (OSOCTET) value;
      }
   }

   /* If memory stream, update I/O byte count */
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt) && pctxt->pStream->id == OSRTSTRMID_MEMORY) {
      pctxt->pStream->ioBytes += (pctxt->buffer.byteIndex - idx);
   }
#endif
   LCHECKX (pctxt);

   return 0;
}

EXTRTMETHOD int rtxEncBitsFromSize (OSCTXT* pctxt, OSSIZE value, OSSIZE nbits)
{
   /* The lowest nbits of value get encoded. */

   static const OSSIZE sizeTypeBits = sizeof(OSSIZE) * 8;
   OSSIZE idx = pctxt->buffer.byteIndex;
   unsigned bitOff = pctxt->buffer.bitOffset;
   OSSIZE nmBytes;
   OSOCTET* p;

   if (nbits > sizeof(value)*8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (nbits == 0)
      return 0;

   if (bitOff == 8) {
      /* field aligned */

      /* nmBytes is # of bytes we'll write into beyond the current one */
      nmBytes = (nbits - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      pctxt->buffer.byteIndex = idx + (nbits >> 3);
      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nbits & 7));

      value <<= sizeTypeBits - nbits; /* clear unused bits */

      /* rotate left one byte */
      value = (value >> (sizeTypeBits - 8)) | (value << 8);

      while (nmBytes-- > 0) {
         *p++ = (OSOCTET) value;
         value = (value >> (sizeTypeBits - 8)) | (value << 8);
      }

      *p = (OSOCTET) value;
   }
   else {
      /* field unaligned */

      /* If we number the bits in the buffer, starting with the highest bit
         in the current byte, beginning with 0, then nextOff identifies the
         bit position after nbits bits have been written.  Put another way,
         it is the number of bits we'll advance from the start of the current
         byte.
      */
      OSSIZE nextOff = nbits + 8 - bitOff;
      OSSIZE mask;

      /* nmBytes is the number of bytes we'll write into beyond the current
         byte.  If nmBytes == 0, we will only touch the current byte.
      */
      nmBytes = (nextOff - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      value <<= sizeTypeBits - nbits; /* clear unused bits */
      value = (value >> (sizeTypeBits - bitOff)) | (value << bitOff);

      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nextOff & 0x7));

      /* nextOff now becomes the number of bytes we will advance from the
         current byte. */
      nextOff >>= 3;
      pctxt->buffer.byteIndex = idx + nextOff;

      /* Now update the current byte, and the next nmBytes bytes. */

      mask = SIZE_MAX << bitOff;
      *p = (*p & mask) | (OSOCTET) (value & ~mask);
      p++;
      value &= mask;

      while (nmBytes-- > 0) {
         value = (value >> (sizeTypeBits - 8)) | (value << 8);
         *p++ = (OSOCTET) value;
      }
   }

   /* If memory stream, update I/O byte count */
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt) && pctxt->pStream->id == OSRTSTRMID_MEMORY) {
      pctxt->pStream->ioBytes += (pctxt->buffer.byteIndex - idx);
   }
#endif
   LCHECKX (pctxt);

   return 0;
}

EXTRTMETHOD int rtxEncBitsFromByteArray
(OSCTXT* pctxt, const OSOCTET* pvalue, OSSIZE nbits)
{
   OSSIZE idx = pctxt->buffer.byteIndex;
   unsigned bitOff = pctxt->buffer.bitOffset;
   OSSIZE nmBytes;

   if (nbits == 0)
      return 0;

   /* Note: this code previously used rtxCheckOutputBuffer and assumed that
      in the streaming case, requesting buffer.size bytes would cause the
      stream to be flushed and leave byteIndex == 0.  This is not necessarily
      the case.
   */

   if (bitOff == 8) {
      /* field aligned */

      /* I don't know why this is done, but nmBytes is set such that
         nmBytes + 1 is the number of bytes needed to hold the full nbits.
         The result of this is that if nbits is a multiple of 8, we get
         nmBytes = one less than the number of full bytes, and so in the case
         where we write directly to the stream (see below), we don't write as
         many bytes as we could, saving one byte to go into the buffer.
         I am leaving that behavior in place, in case it is somehow significant.
      */
      nmBytes = (nbits - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer; it can't hold all the data we want to write.
            Note the test uses >= instead of > b/c of how nmBytes was set. */

#ifndef _NO_STREAM
         if (OSRTISSTREAM (pctxt) && rtxIsOutputBufferFlushable(pctxt) )
         {
            /* We have a stream and the buffer is flushable.
               We'll ensure the buffer is empty and has room to hold 1 byte,
               and then write direct to the stream.
               Note: we're assuming that writing direct to the stream won't
               write to the buffer.  That would be a bad assumption for
               a memory stream, but memory streams are not flushable.
            */
            if ( pctxt->buffer.byteIndex > 0 )
            {
               int stat = rtxFlushOutputBuffer(pctxt);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
            else {
               /* buffer empty; ensure buffer.size >= 1 (for last byte) */
               int stat = rtxCheckOutputBuffer(pctxt, 1);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }

            /* assert: buffer.byteIndex == 0 && buffer.size >= 1 */

            if (nmBytes >= pctxt->buffer.size)
            {
               /* The buffer cannot hold all nbits.
                  We write directly to the stream.
                  If nbits is a multiple of 8, we'll write one less byte than
                  we could. This is probably just to simplify the logic
                  so that we always have some remaining bits to put into the
                  buffer in all cases.
               */
               int stat = rtxStreamWrite (pctxt, pvalue, nmBytes);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               pvalue += nmBytes;
               nbits -= nmBytes << 3;

               /* We now have 1 <= nbits <= 8
                  These remaining bits will go into the buffer; we have
                  ensured the buffer has room for at least one byte. */
            }
            /* else:
               nmBytes < size    =>
               nmBytes + 1 <= size =>
               the buffer has room enough for nmBytes + 1 bytes, which is
               the space needed to hold all nbits.  We'll write to buffer
               below.
            */
         }
         else
#endif /* _NO_STREAM */
         {
            /* There is no streaming, no stream, or the buffer can not be
               flushed.

               We need nmBytes + 1 bytes available to hold all nbits (see note
               on nmBytes)
             */
            int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);

            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         idx = pctxt->buffer.byteIndex;
      }
      /* else: buffer can hold the nmBytes + 1 bytes needed for all nbits */

      /* nmBytes now becomes the number of full bytes we have left to write;
         these all go into the buffer. */
      nmBytes = nbits >> 3;
      pctxt->buffer.byteIndex = idx + nmBytes;

      if (nmBytes == 1)
         pctxt->buffer.data[idx] = *pvalue;
      else if (nmBytes > 1)
         OSCRTLMEMCPY (pctxt->buffer.data + idx, pvalue, nmBytes);

      /* We now have (nbits % 8) bits remaining.  They are in the high bits of
         the final byte.  We now let nbits equal the number of the low bits in
         the final byte that we *don't* want (i.e., it becomes our left shift
         value).
      */
      nbits = 8 - (nbits & 7);
      pctxt->buffer.bitOffset = (OSUINT16) nbits;

      if (nbits != 8)
         pctxt->buffer.data[idx + nmBytes] =
            (OSOCTET) (pvalue[nmBytes] & (0xFF << nbits));
   }
   else {
      /* field unaligned */
      unsigned shiftR = 8 - bitOff;
      unsigned shiftL = bitOff;

      /* nextOff is the number of bits that will be in the buffer, including
         the bits already written to the current byte, after writing nbits.
         From the current byte, it is the 0-based bit offset for the next
         bit after nbits have been written.
      */
      OSSIZE nextOff = nbits + shiftR;

      /* set bitOff to the value buffer.bitOffset will have after all nbits
         have been written.
      */
      bitOff = 8 - ((OSUINT32)nextOff & 0x7);

      if ( nbits <= (OSSIZE)pctxt->buffer.bitOffset )
      {
         /* The bits all fit in the current partial byte.
            If nextOff == 8, the current partial byte is completely filled.
         */
         pctxt->buffer.bitOffset = (OSUINT16) bitOff;
         bitOff &= 7; /* 8 is changed to 0 */

         pctxt->buffer.data[pctxt->buffer.byteIndex] |=
                           (OSOCTET) ((*pvalue >> shiftR) & (0xFF << bitOff));

         if ( nextOff == 8 ) pctxt->buffer.byteIndex++;
      }
      else {
         OSOCTET* p;
         OSOCTET tm;

         /*
         nmBytes: nmBytes = # of bytes that will be touched/addressed in the
         buffer, not including the current byte.  This means we can fill the
         remainder of the current byte, then write nmBytes - 1 (possibly zero)
         full bytes and then have a final byte to (possibly partially) fill;
         this is what we're going to do.

         Given nbits > buffer.bitOffset, it follows that nmBytes >= 1.
         */
         nmBytes = (nextOff - 1) >> 3;

         /* We're going to fill the remaining (partial byte), then fill
            whole bytes, then write a partial byte if there are any bits left.
         */
         if ( !OSRTISSTREAM (pctxt) || !rtxIsOutputBufferFlushable(pctxt) )
         {
            /* Not a stream or can't flush to stream.  The buffer will have to
               hold all the data or its an error.  We need it to have nmBytes
               full bytes available.
            */
            int stat = rtxCheckOutputBuffer (pctxt, nmBytes);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            idx = pctxt->buffer.byteIndex; /* byteIndex should not have changed;
                                                being safe */
         }
         else if ( pctxt->buffer.size == 0 )
         {
            /* ensure buffer size >= 1 b/c we always put at least one byte into
               buffer */
            int stat = rtxCheckOutputBuffer (pctxt, 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         /* assert: pctxt->buffer.size >= 1 */

         p = pctxt->buffer.data + idx;


         /* The code used to set buffer.byteIndex and buffer.bitOffset to their
            final values here.  That was wrong because rtxCheckOutputBuffer
            uses those values and we used to use rtxCheckOutputBuffer below.
            Also it would set buffer.byteIndex to zero sometimes, overriding the
            final value that was set here.  We now use rtxFlushOutputBuffer,
            and not rtxCheckOutputBuffer, that that can also reset
            buffer.byteIndex.
         */

         tm = *pvalue++;

         *p++ |= tm >> shiftR;   /* fill remainder of current byte */

         pctxt->buffer.bitOffset = 8;
         pctxt->buffer.byteIndex++;

         if (OSRTISSTREAM (pctxt) && idx + nmBytes >= pctxt->buffer.size
               && rtxIsOutputBufferFlushable(pctxt) )
         {
            /* We have a flushable stream and the buffer is not able to
               hold all the data without a flush.  We're going to repeatedly
               fill the buffer to being full and then flush it to the stream,
               until we have written nmBytes - 1 (possibly zero) bytes.
               We will leave the buffer empty and we know size >= 1, so we'll
               have room for the final byte.
            */

            /* Initialize tail to the number of full bytes we can safely write
               to buffer.  The -1 is b/c the current byte (buffer.byteIndex)
               was partially full (even though we have now filled it, that was
               not reflected in the value of idx.
            */
            OSSIZE tail = pctxt->buffer.size - idx - 1;

            if ( nmBytes == 1 )
            {
               /* The buffer is already full. We won't enter the below loop
                  so we have to flush here.
               */
               int stat = rtxFlushOutputBuffer(pctxt);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               /* Reinitialize p to point to the correct place in the buffer
                  where the next byte(s) should go. */
               p = pctxt->buffer.data;
            }

            while (nmBytes > 1) {
               /* tail now becomes the actual number of bytes to write,
                  before calling rtxCheckOutputBuffer */
               if (tail > nmBytes - 1)
                  tail = nmBytes - 1;

               /* Since tail is going to be used to countdown, we have to
                  adjust nmBytes and the buffer.byteIndex BEFORE the loop.  Just
                  note that these values now reflect the values they ought to
                  have AFTER the loop is finished.
               */
               nmBytes -= tail;
               pctxt->buffer.byteIndex += tail;

               while (tail--) {
                  OSOCTET value = *pvalue++;
                  *p++ = (OSOCTET) ((tm << shiftL) | (value >> shiftR));
                  tm = value;
               }

               if (pctxt->buffer.byteIndex == pctxt->buffer.size)
               {
                  /* The buffer is now full. Flush the buffer.  Even if we
                     break out of this loop, we still have the final (possibly
                     partial) byte that we need room for.
                   */
                  int stat = rtxFlushOutputBuffer(pctxt);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);

                  /* reinitialize tail to the number of full bytes we can
                     safely write to buffer.

                     I am not updating idx since it is not used for the
                     remainder of this function.  Otherwise, it ought to be
                     assigned pctxt->buffer.byteIndex.
                   */
                  tail = pctxt->buffer.size;

                  /* Reinitialize p to point to the correct place in the buffer
                     where the next byte(s) should go. */
                  p = pctxt->buffer.data;
               }
               /* else: there is still room in the buffer for more data.
                  This will happen when nmBytes was small enough that we
                  couldn't fill the buffer.  We're going to break the loop.
               */
            }
         }
         else {
            /* Not a stream or we can go nmBytes bytes beyond the current byte
               (the partial byte which we already actually filled) or
               the buffer cannot be flushed (so the data must go into buffer).

               Note that we have already ensured that if it is not a stream
               or if the stream is not flushable then the buffer can hold at
               least nmBytes beyond the current (partial) byte by calling
               rtxCheckOutputBuffer above.
             */

            /* Write all but the final (possibly partial) byte into the buffer.
               Update buffer.byteIndex using nmBytes before decrementing it.
            */
            pctxt->buffer.byteIndex += nmBytes - 1;
            while (nmBytes-- > 1) {
               OSOCTET value = *pvalue++;
               *p++ = (OSOCTET) ((tm << shiftL) | (value >> shiftR));
               tm = value;
            }
         }

         /* Write the remaining (<=8) bits into the buffer.  We might fill the
            byte we're writing into or we might not.
         */
         if ( bitOff == 8 ) {
            /* We're filling the byte, so byteIndex needs to be incremented. */
            pctxt->buffer.byteIndex++;
         }
         /* else: the current byte is not being filled. */

         pctxt->buffer.bitOffset = (OSUINT16) bitOff;
         bitOff &= 7; /* 8 is changed to 0 */

         /* Put the final, possibly partial, byte into the buffer */
         if (bitOff >= shiftL) { /* eqv: shiftR >= 8 - bitOff */
            /* tm contains enough bits */
            *p = (OSOCTET) ((tm << shiftL) & (0xFF << bitOff));
         }
         else {
            /* need additional bits */
            *p = (OSOCTET) (((tm << shiftL) | (*pvalue >> shiftR)) &
                            (0xFF << bitOff));
         }
      }
   }

   /* If memory stream, update I/O byte count */
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt) && pctxt->pStream->id == OSRTSTRMID_MEMORY) {
      pctxt->pStream->ioBytes += (pctxt->buffer.byteIndex - idx);
   }
#endif
   LCHECKX (pctxt);

   return 0;
}

EXTRTMETHOD int rtxCopyBits
(OSCTXT* pctxt, const OSOCTET* pvalue, OSSIZE nbits, OSUINT32 bitOffset)
{
   int stat;
   pvalue += bitOffset >> 3;
   bitOffset &= 7;

   if (bitOffset != 0) {
      /* copy bits to align source */
      OSUINT32 value = *pvalue;
      OSSIZE n = 8 - bitOffset;

      if (n > nbits) {
         value >>= n - nbits;
         n = nbits;
      }

      stat = rtxEncBits (pctxt, value, n);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      nbits -= n;
      pvalue++;
   }

   if (nbits > 0) {
      stat = rtxEncBitsFromByteArray (pctxt, pvalue, nbits);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

/**
 * Set *p to its current value, merged with some bits from value.  The details
 * of this are descibed below.
 *
 * If nbits == 0, this function does nothing.
 * Othewise, if nbits is a multiple of 8, *p is set to value (all 8 bits are
 * set).
 * Otherwise, let k = nbits % 8.  The high k bits of *p are set to the high
 * k bits of value.  The low 8-k bits of value MUST be zero, or else some of
 * the low 8-k bits of *p might be altered.
 */
static void mergeBits (OSOCTET* p, OSOCTET value, OSSIZE nbits)
{
   if (0 == nbits) {
      return;
   }
   else if (0 == nbits % 8) {
      *p = (OSOCTET) value;
   }
   else {
      /*
      mask is set so that the high k bits are zero and the rest are 1.

      Note: nbits is size_t, so 8-nbits is NOT negative. In the event that
      nbits > 8, nbits - 8 wraps around (integer overflow), but the result can
      be shown to give the desired result after the % is applied.

      ub will be p with the high k bits cleared.  As long as the low 8-k bits
      are clear in value, as required, ub | value gives the desired result.
      */

      /* merge last value with what already exists in the buffer */
      OSOCTET mask = (OSOCTET)((1 << ((8 - nbits) % 8)) - 1);
      OSOCTET ub = *p & mask;
      *p = ub | value;
   }
}

EXTRTMETHOD int rtxMergeBits (OSCTXT* pctxt, OSUINT32 value, OSSIZE nbits)
{
   OSSIZE   idx = pctxt->buffer.byteIndex, nmBytes;
   OSUINT32 bitOff = pctxt->buffer.bitOffset;
   OSOCTET* p;

   if (nbits > sizeof(value)*8)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (nbits == 0)
      return 0;

   if (bitOff == 8) {
      /* field aligned */

      /* nmBytes is the number of bytes that will be touched beyond the
         current byte.  Since this is the aligned case, this means it is
         also the number of full bytes that will be written EXCLUDING the final
         byte, which may be either full or partial.
         If nmBytes == 0, the current byte is the only byte we'll touch.
      */
      nmBytes = (nbits - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      pctxt->buffer.byteIndex = idx + (nbits >> 3);
      pctxt->buffer.bitOffset = (OSUINT16) (8 - (nbits & 7));

      value <<= 32 - nbits; /* clear unused bits */
      value = ROTL (value, 8);

      /* Write the bytes that are guaranteed to be full bytes; this does not
         include the final byte that we'll be touching. */
      while (nmBytes-- > 0) {
         *p++ = (OSOCTET) value;
         value = ROTL (value, 8);
      }

      /* Now write the final byte, which may be full or partial.
         If nbits is not divisble by 8, the above code has cleared any bits
         in value that should not be set, as required by mergeBits.
      */
      mergeBits (p, (OSOCTET)value, nbits);
   }
   else {
      /* field unaligned */

      /* nextOff is the number of bits that will be in the buffer, starting
         at the current byte, after we've written the nbits.
         If is also the 0-based bit offset, from the current byte, of the next
         bit to be written after the nbits have been written.
      */
      OSSIZE nextOff = nbits + 8 - bitOff;

      /* nbitsInLastOctets is the number of bits that will be written in the
         last byte, if the last byte is partially filled; it is 0 if the last
         byte is full.
      */
      OSSIZE nbitsInLastOctet = nextOff % 8;
      OSUINT32 mask;

      /* nmBytes is the number of bytes we will touch in the buffer, beyond
         the current byte.
      */
      nmBytes = (nextOff - 1) >> 3;

      if (idx + nmBytes >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = rtxCheckOutputBuffer (pctxt, nmBytes + 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx = pctxt->buffer.byteIndex;
      }

      p = pctxt->buffer.data + idx;

      /* Clear unused bits and align the bits to be written first with
         the position they will be written into. */
      value <<= 32 - nbits;
      value = ROTL (value, bitOff);

      pctxt->buffer.bitOffset = (OSUINT16) (8 - nbitsInLastOctet);

      /* nextOff now becomes the number of bytes the byteIndex will have
         advanced after having written the nbits.  The byte at index
         idx + nextOff might or might not be touched; it will be touched when
         nbitsInLastOctet > 0.
         If nextOff >= 1, the current byte is filled.
         If nextOff > 1, nextOff - 1 bytes beyond the current byte will be
         filled.
         If nbitsInLastOctet > 0, the byte idx + nextOff is partly filled.
      */
      nextOff >>= 3;
      pctxt->buffer.byteIndex = idx + nextOff;

      if (nextOff > 0) {
         /* We are going to fill out the rest of the bits in the current byte */

         /* Set the first byte, keeping the original high bits and setting the
            low bits from the bits to be encoded. */
         mask = ~0u << bitOff;
         *p = (*p & mask) | (OSOCTET) (value & ~mask);
         p++;

         /* clear the bits in value that we just used */
         value &= mask;

         /* Now set all the full bytes, if there are any */
         while (nextOff-- > 1) {
            value = ROTL (value, 8);
            *p++ = (OSOCTET) value;
         }

         if (nbitsInLastOctet > 0)
         {
            /* There is a final, partial byte; merge it.
               The above has ensured that any unused lower bits in value will
               be clear, as required by mergeBits.
            */
            value = ROTL(value, 8);
            mergeBits(p, (OSOCTET)value, nbitsInLastOctet);
         }
      }
      else {
         /* We are not going to fill out the rest of the current byte.
         Only the current byte gets updated.
         From the above, we know nbits < bitOff < 8.
         *p should be set as follows:
            - highest 8 - bitOff bits: unchanged
            - next lowest nbits bits: taken from value
            - lowest bitOff - nbits bits: unchanged
         Any unused bits in value have already been cleared.
         */

         OSOCTET ub;
         mask = ~0u << bitOff;      /* mask for high 8-bitOff bits */
         ub =  (OSOCTET) *p & mask; /* ub is *p with lowest bits cleared */

         /* ub becomes *p, with lowest bitOff bits set from value */
         ub |= (OSOCTET) (value & ~mask);

         /* use mergBits to set highest 8-bitOff + nbits bits of *p from ub,
            leaving the lowest bitOff - nbits bits unchanged; those bits will be
            clear in both ub and value.
         */
         mergeBits (p, ub, nbits + 8 - bitOff);
      }
   }

   LCHECKX (pctxt);

   return 0;
}

