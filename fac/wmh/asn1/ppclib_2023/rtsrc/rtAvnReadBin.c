/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "rtAvn.h"
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxText.h"


static OSUINT32 FindBitMapItem(const OSBitMapItem* pBitMap,
   const OSOCTET* str)
{
   OSUINT32 i = 0;
   for (; pBitMap[i].name != 0; i++) {
      if (OSCRTLSTRCMP(
         (const char*) pBitMap[i].name, (const char*) str) == 0)
         return pBitMap[i].bitno;
   }

   return ~0u;
}


/**
 * Take nbits bits in *pbuffer and convert each bit to a nibble with the
 * same value (0 or 1).
 * If necessary, resize the buffer.
 */
int makeBitsNibbles(OSCTXT* pctxt, OSOCTET** pbuffer, OSSIZE* bufsize,
   OSBOOL dynamic, OSSIZE nbits)
{
   OSSIZE nocts;

   if ( nbits == 0 ) return 0;

   /* Each bit becomes a nibble, so treat nbits as # of nibbles and figure
      how many octets that would be, avoiding possibility of overflow. */
   nocts = (nbits - 1) / 2 + 1;

   if (nocts > *bufsize) {
      if (dynamic) {
         *pbuffer = rtxMemRealloc(pctxt, *pbuffer, nocts);
         *bufsize = nocts;
         if (*pbuffer == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
      }
      else return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
   }

   if (nbits % 2 == 1) {
      /* Odd number of octets.  Set high nibble using last bit. */
      *pbuffer[nocts-1] = rtxTestBit(*pbuffer, nbits, nbits - 1) << 4;
      nocts--;
      nbits--;
   }

   /* nbits is now even. */
   while (nbits > 0) {
      /* Set the last byte using the next to last and last bit. */
      *pbuffer[nocts - 1] = (rtxTestBit(*pbuffer, nbits, nbits - 2) << 4 ) |
                              rtxTestBit(*pbuffer, nbits, nbits - 1);
      nocts--;
      nbits-=2;
   }

   return 0;
}


/**
 * Convert nnibbles nibbles from nibbles to bits in the given buffer.
 * If a nibble is zero, it becomes a zero bit, otherwise it becomes a 1 bit.
 * In the context of this file, this function is only used when all nibbles are
 * zero or one.
 */
void makeNibblesBits(OSOCTET* buffer, OSSIZE nnibbles)
{
   OSSIZE bitIdx = 0;
   OSSIZE byteIdx = 0;

   while (bitIdx < nnibbles)
   {
      if (buffer[byteIdx] >> 4)
         rtxSetBit(buffer, bitIdx, bitIdx);
      else
         rtxClearBit(buffer, bitIdx, bitIdx);

      bitIdx++;
      if (bitIdx < nnibbles) {
         if (buffer[byteIdx] & 0x0F)
            rtxSetBit(buffer, bitIdx, bitIdx);
         else
            rtxClearBit(buffer, bitIdx, bitIdx);
      }
      bitIdx++;
      byteIdx++;
   }
}


/**
  * Internal function for decoding bstring and hstring from input.  This can
  * decode into either a static or dynamic array (where array can be resized).
  *
  * This assumes the opening single quote has been read already, and then reads
  * the rest of the string, including the closing single quote and B or H
  * designator.
  *
  * @param pbuffer Pointer to buffer pointer.  If dynamic, it is possible
  *         we'll shrink kit.
  * @param bufsize Size of buffer (in bytes).
  * @param dynamic TRUE if buffer can be resized and pbuffer reset.  If TRUE,
  *         you better pass non-null for at least one of pnbits, pnocts if you
  *         want to know the new size of the buffer (which could be < bufsize).
  * @param pnbits If not null, receives the size of the decoded string in bits.
         This should be null for OCTET STRING, since when non-null, if this
  *      overflows an error will be returned, which would happen for large
  *      OCTET STRINGs.
  * @param pnocts If not null, receives the size of the decoded octet string in
  *    bytes. If the input is not a multiple of 8 bits, this is rounded up to
  *    a whole number of octets.
  * @param hasNamedBits  TRUE if BIT STRING type has named bits.
  * @param minLen        Minimum length, if hasNamedBits is true; in that case,
  *                     the returned bit string will be at least this many bits
  *                     long and will have trailing zero bits beyond this length
  *                     trimmed off.
  * @return 0 for success, negative for error.
  */
int decodebhstring_internal(OSCTXT* pctxt, OSOCTET** pbuffer, OSSIZE bufsize,
      OSBOOL dynamic, OSSIZE* pnbits, OSSIZE* pnocts, OSBOOL hasNamedBits,
      OSSIZE minLen)
{
   /* General description of algorithm.
      If the user does not ask to receive the number of bits, we'll almost
      certainly be dealing with an OCTET STRING and an hstring, so we assume we
      have an hstring.  Otherwise, we assume we have a bstring.

      If we assume we have a bstring, on the first non-0/1 char, we switch to
      assuming we have an hstring, converting the data we have so far (this
      will expand the data).

      If we assume we have an hstring and get to the designator and find out
      it is a bstring, we convert the data (this will compress the data).

      We could have read the entire string into memory first, and converted
      after reading the designator, but that would equally require resizing
      memory to hold a string of unknown size and use more memory (1 byte per
      character instead of 1 bit or 4 bits per character). In the case of
      decoding into static memory, our approach will not require any dynamic
      memory allocations.  In the dynamic case, it will allocate the minimal
      amount of memory.  Also, it seems unlikely that a large (or any) OCTET
      STRING would be encoded as bstring so that we would have to convert n
      bytes to n/8 bytes, or that we'd get very far treating an hstring as a
      bstring before hitting some non-0/1 character; this makes it unlikely
      that this approach will result in wasted processing.
   */
   int ret;
   OSOCTET ub;
   OSSIZE nbits;   /* Used if pnbits is null */
   OSSIZE nocts;   /* Used if pnocts is null */
   OSBOOL countbits = pnbits != 0;  /* TRUE if must return # of bits. */
   OSBOOL decbits = TRUE;   /* TRUE if decoding as bstring. */
   OSBOOL legalbs = TRUE;   /* TRUE if input thus far consisted only of 0/1
                               chars */
   OSBOOL highNibble = TRUE;   /* TRUE when next hex char will be the high
                                 nibble for byte */
   OSSIZE minbuf = 32;     /* When allocating, allocate at least this many
                              bytes.  This makes sure we don't try to allocate
                              0 bytes (doubling a zero len buffer) and is
                              also used to ensure the buffer is sized for
                              the min length, if there are named bits. */

   if (hasNamedBits && minLen > 0) {
      if (!dynamic && (minLen - 1) / 8 + 1 > bufsize)
      {
         /* Static array is not large enough to hold the minimum size for
            BIT STRING with named bits. This should never happen, as the code
            generator should generate an array large enough for the minimum len.
         */
         return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
      }
      else if (dynamic) {
         minbuf = OS_MAX(minbuf, (minLen - 1) / 8 + 1);
      }
   }

   /* Note: pnbits and pnocts will only be kept updated when decbits and
      !decbits, respsectively.  We'll update bufsize if we resize the buffer.
   */

   if (pnbits == 0)
   {
      /* User doesn't care about # of bits; most likely will be hstring. */
      pnbits = &nbits;
      decbits = FALSE;
   }

   if (pnocts == 0) pnocts = &nocts;

   *pnocts = 0;
   *pnbits = 0;

   for (;;) {
      /* Read byte.  If single quote, we're done. */
      ret = rtxReadBytes(pctxt, &ub, 1);
      if (ret < 0) return LOG_RTERR(pctxt, ret);

      if (ub == '\'') break;

      /* Whitespace is allowed in bstring and hstring; skip whitespace. */
      if (OS_ISSPACE(ub)) continue;

      if (legalbs && ub != '0' && ub != '1' ) legalbs = FALSE;

      if (decbits && (ub == '0' ||  ub == '1'))
      {
         /* Currently decoding as bstring and character is consistent with
            that.  Add decoded bit to bstring result. */

         if (*pnbits == OSSIZE_MAX) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

         *pnbits = *pnbits + 1;

         if ((*pnbits - 1) / 8 + 1 > bufsize )
         {
            /* We'll overflow the buffer.  Resize it, if dynamic. */
            if (dynamic && bufsize < OSSIZE_MAX ) {
               if (bufsize < OSSIZE_MAX/2)
                  bufsize = OS_MAX(minbuf, bufsize * 2);
               else bufsize = OSSIZE_MAX;

               *pbuffer = rtxMemRealloc(pctxt, *pbuffer, bufsize);
               if (*pbuffer == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
            }
            else return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
         }
         if (ub == '1') rtxSetBit(*pbuffer, *pnbits, *pnbits-1);
         else rtxClearBit(*pbuffer, *pnbits, *pnbits - 1);
      }
      else if (!OS_ISHEXCHAR(ub)) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
      else {
         OSOCTET nibble;

         /* Hex char.  Must be hstring. */
         if (decbits) {
            if (*pnbits > 0) {
               /* We decoded thus far as bstring.  Convert the results thus far
                  to nibbles. *pnbits is the number of bits previously decoded,
                  but also the number of nibbles the data should have been
                  decoded as.
               */
               if (countbits && *pnbits > OSSIZE_MAX / 4) {
                  /* We won't be able to return # of bits to user. */
                  return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
               }
               makeBitsNibbles(pctxt, pbuffer, &bufsize, dynamic, *pnbits);
               highNibble = *pnbits % 2 == 0;
               *pnocts = (*pnbits - 1) / 2 + 1;    /* rounds up */
            }
            /* else: highNibble == TRUE and pnocts == 0, as desired. */
            decbits = FALSE;
         }


         /* Convert hex char to nibble and add to result.
            Resize if necessary.
            If user wants nbits back but it will overflow, report error. */
         OS_HEXCHARTONIBBLE(ub, nibble);
         if (highNibble) {
            if (*pnocts == OSSIZE_MAX ||
                  (countbits && *pnocts > OSSIZE_MAX / 8))
            {
               /* Too many octets or too many bits (and user wants # of bits) */
               return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
            }

            /* Make sure we have room in the buffer. */
            if (*pnocts + 1 > bufsize)
            {
               if (dynamic) {
                  if (bufsize < OSSIZE_MAX / 2)
                     bufsize = OS_MAX(minbuf, bufsize * 2);
                  else bufsize = OSSIZE_MAX;

                  *pbuffer = rtxMemRealloc(pctxt, *pbuffer, bufsize);
                  if (*pbuffer == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
               }
               else return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
            }

            /* Nibble goes into next byte as high nibble. */
            (*pbuffer)[*pnocts] = nibble << 4;
            *pnocts += 1;
         }
         else {
            (*pbuffer)[*pnocts-1] |= nibble;
         }
         highNibble = !highNibble;
      }
   }

   /* Read designator. */
   ret = rtxReadBytes(pctxt, &ub, 1);
   if (ret < 0) return LOG_RTERR(pctxt, ret);

   if (ub == 'H')
   {
      if (decbits) {
         /* Convert results from bits to nibbles. */
         if (countbits && *pnbits > OSSIZE_MAX / 4) {
            /* We won't be able to return # of bits to user. */
            return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
         }
         makeBitsNibbles(pctxt, pbuffer, &bufsize, dynamic, *pnbits);
         *pnocts = (*pnbits - 1) / 2 + 1;    /* rounds up */
      }
      else {
         /* Make sure we set pnbits in case caller wants it.
            We already know from above that *pnbits won't overflow. */
         *pnbits = *pnocts * 8;
         if (!highNibble) *pnbits -= 4;   /* 1/2 byte read. */
      }
   }
   else if (ub == 'B') {
      if (decbits) {
         rtxZeroUnusedBits(*pbuffer, *pnbits);
      }
      else {
         if (legalbs) {
            /* Convert nibbles (each being 0/1) to bits.
            Each octet will become two bits (one per nibble), so the number
            of bits will become nocts * 2.  If user wants bits, this could
            be overflow.  The number of octets, however, will shrink to
            (nocts * 2) / 8
            Actually, if highNibble is true, bits = octs * 2 but otherwise
            bits = octs * 2 - 1.
             */
            if (countbits && ((highNibble && *pnocts > OSSIZE_MAX / 2) ||
                              (!highNibble && *pnocts > (OSSIZE_MAX-1) / 2)))
            {
               /* We won't be able to return # bits to user. */
               return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
            }
            if (highNibble) *pnbits = *pnocts * 2;
            else *pnbits = *pnocts * 2 - 1;
            makeNibblesBits(*pbuffer, *pnbits);
         }
         else {
            /* bstring had other than 1/0 chars. */
            return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
         }
      }

      /* Make sure we set pnocts in case caller wants it. */
      if (*pnbits == 0) *pnocts = 0;
      else *pnocts = (*pnbits - 1) / 8 + 1;
   }
   else
   {
      /* Error: bad designator. */
      return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
   }

   if (hasNamedBits) {
      /* Add or remove trailing bits, in accordance with the minimum length. */
      if (*pnbits < minLen) {
         /* Increase *pnbits, making sure added bits are zeros.
            If we never allocated the buffer, it might need to be enlarged.
            If the buffer is static, we've already ensured it is large enough.
         */
         minbuf = (minLen - 1) / 8 + 1;   /* (32 is not a min size now.) */
         if (dynamic && bufsize < minbuf)
         {
            *pbuffer = rtxMemRealloc(pctxt, *pbuffer, minbuf);
            if (*pbuffer == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
            bufsize = minbuf;
         }

         /* First, set individual bits until we fill out a byte. */
         while (*pnbits < minLen && *pnbits % 8 != 0)
         {
            *pnbits += 1;
            rtxClearBit(*pbuffer, *pnbits, *pnbits -1);
         }

         /* Now, set whole bytes. */
         *pnocts = (*pnbits - 1) / 8 + 1;
         while (*pnocts < minbuf) {
            (*pbuffer)[*pnocts] = 0;
            *pnocts += 1;
         }
         *pnbits = minLen;
      }
      else if (*pnbits > minLen) {
         /* Trim any zero bits in excess of minLen */
         OSSIZE lastBitSet = rtxLastBitSet(*pbuffer, *pnbits);
         if (lastBitSet == OSNULLINDEX) *pnbits = minLen;
         else *pnbits = OS_MAX(minLen, lastBitSet + 1);

         if (*pnbits == 0) *pnocts = 0;
         else *pnocts = (*pnbits - 1) / 8 + 1;
      }
   }

   /* Resize dynamic string down to exact size. */
   if (dynamic && bufsize > *pnocts)
   {
      *pbuffer = rtxMemRealloc(pctxt, *pbuffer, *pnocts);
      if (*pbuffer == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
   }

   return 0;
}


EXTRTMETHOD int rtAvnReadBitString(OSCTXT* pctxt, OSOCTET* buffer, OSUINT32* pnbits,
                        OSSIZE bufsize, OSBOOL hasNamedBits, OSSIZE minLen)
{
   int ret;
   OSSIZE nbits;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &buffer, bufsize, FALSE, &nbits, 0,
                                 hasNamedBits, minLen);
   if ( ret != 0 ) return LOG_RTERR(pctxt, ret);

#if SIZE_MAX > OSUINT32_MAX
   if (nbits > OSUINT32_MAX)
      return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif
   *pnbits = (OSUINT32)nbits;

   return ret;
}


EXTRTMETHOD int rtAvnReadBitString64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnbits, OSSIZE bufsize, OSBOOL hasNamedBits, OSSIZE minLen)
{
   int ret;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &buffer, bufsize, FALSE, pnbits, 0,
      hasNamedBits, minLen);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   return ret;
}


EXTRTMETHOD int rtAvnReadBitStringExt(OSCTXT* pctxt, OSOCTET* buffer,
   OSUINT32* pnbits, OSSIZE bufsize, OSOCTET** pextdata, OSBOOL hasNamedBits,
   OSSIZE minLen)
{
   int ret;
   OSSIZE nbits;

   ret = rtAvnReadBitStringExt64(pctxt, buffer, &nbits, bufsize, pextdata,
            hasNamedBits, minLen);

#if SIZE_MAX > OSUINT32_MAX
   if (nbits > OSUINT32_MAX)
      return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif
   *pnbits = (OSUINT32)nbits;

   return ret;
}


EXTRTMETHOD int rtAvnReadBitStringExt64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnbits, OSSIZE bufsize, OSOCTET** pextdata, OSBOOL hasNamedBits,
   OSSIZE minLen)
{
   int ret;
   OSSIZE nocts;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   /* Decode into dynamic array. */
   ret = decodebhstring_internal(pctxt, pextdata, 0, TRUE, pnbits, &nocts,
      hasNamedBits, minLen);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   /* Now move bits that fit in static buffer from dynamic array to static
      array. */
   OSCRTLSAFEMEMCPY(buffer, bufsize, *pextdata, OS_MIN(bufsize, nocts));
   if (nocts <= bufsize) {
      rtxMemFreePtr(pctxt, *pextdata);
      *pextdata = 0;
   }
   else {
      *pextdata += bufsize;
   }

   return ret;
}


EXTRTMETHOD int rtAvnReadBitStringDyn(OSCTXT* pctxt, ASN1DynBitStr* pvalue,
   OSBOOL hasNamedBits, OSSIZE minLen)
{
   int ret;
   OSSIZE nbits;
   OSOCTET* pbuffer = 0;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &pbuffer, 0, TRUE, &nbits, 0,
                                 hasNamedBits, minLen);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

#if SIZE_MAX > OSUINT32_MAX
   if (nbits > OSUINT32_MAX)
      return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif

   pvalue->data = pbuffer;
   pvalue->numbits = (OSUINT32)nbits;

   return ret;
}

EXTRTMETHOD int rtAvnReadBitStringDyn64(OSCTXT* pctxt, ASN1DynBitStr64* pvalue,
   OSBOOL hasNamedBits, OSSIZE minLen)
{
   int ret;
   OSOCTET* pbuffer = 0;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &pbuffer, 0, TRUE, &pvalue->numbits, 0,
      hasNamedBits, minLen);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   pvalue->data = pbuffer;

   return ret;
}


EXTRTMETHOD int rtAvnReadNamedBits(OSCTXT* pctxt, const OSBitMapItem* pBitMap,
   OSOCTET* pvalue, OSUINT32* pnbits, OSUINT32 bufsize)
{
   OSSIZE nbits;
   int ret = rtAvnReadNamedBits64(pctxt, pBitMap, pvalue, &nbits, bufsize);

   if (pnbits)
   {
      if (nbits > OSUINT32_MAX) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

      *pnbits = (OSUINT32)nbits;
   }

   return ret;
}

#define BUFF_SZ 40

EXTRTMETHOD int rtAvnReadNamedBits64(OSCTXT* pctxt,
   const OSBitMapItem* pBitMap,
   OSOCTET* pvalue, OSSIZE* pnbits,
   OSSIZE bufsize)
{
   int ret;
   char c;

   OSSIZE numbits = bufsize * 8;
   OSUINT32 maxbit = 0;
   OSOCTET buffer[BUFF_SZ];
   OSRTMEMBUF memBuf;

   if (!pBitMap)
      return LOG_RTERRNEW(pctxt, RTERR_INVPARAM);

   rtxMemBufInitBuffer(pctxt, &memBuf, buffer, BUFF_SZ, 1);

   if (pnbits)
      *pnbits = 0;

   if (pvalue && bufsize)
      OSCRTLMEMSET(pvalue, 0, bufsize);

   ret = rtxTxtMatchChar(pctxt, '{', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   c = rtxTxtPeekChar2(pctxt, TRUE);
   if (c != '}') {
      for(;;) {
         OSUINT32 bitno;

         ret = rtAvnReadIdentifierToBuffer(pctxt, &memBuf);
         if (ret != 0) break;

         bitno = FindBitMapItem(pBitMap, OSMEMBUFPTR(&memBuf));

         if (bitno == ~0u) {
            rtxErrAddStrParm(pctxt, (const char*) OSMEMBUFPTR(&memBuf));
            ret = LOG_RTERRNEW(pctxt, RTERR_UNEXPELEM);
            break;
         }
         else {
            if (pvalue)
               rtxSetBit(pvalue, numbits, bitno);
            if (bitno > maxbit)
               maxbit = bitno;
         }

         c = rtxTxtPeekChar2(pctxt, TRUE);
         if (c == ',') {
            ret = rtxTxtMatchChar(pctxt, ',', TRUE);
            if (ret != 0) break;
         }
         else break;

         rtxMemBufReset(&memBuf);
      }
   }

   rtxMemBufFree(&memBuf);

   if (ret == 0) {
      ret = rtxTxtMatchChar(pctxt, '}', TRUE);
      if (ret != 0) return LOG_RTERR(pctxt, ret);
   }


   if (ret >= 0) {
      ret = 0;
      if (pnbits)
         *pnbits = maxbit + 1;
   }

   return ret;
}


EXTRTMETHOD int rtAvnReadOctString(OSCTXT* pctxt, OSOCTET* buffer,
   OSUINT32* pnocts, OSSIZE bufsize)
{
   int ret;
   OSSIZE nocts;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &buffer, bufsize, FALSE, 0, &nocts,
            FALSE, 0);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

#if SIZE_MAX > OSUINT32_MAX
   if (nocts > OSUINT32_MAX)
      return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif
   *pnocts = (OSUINT32)nocts;

   return ret;
}



EXTRTMETHOD int rtAvnReadOctString64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnocts, OSSIZE bufsize)
{
   int ret;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &buffer, bufsize, FALSE, 0, pnocts,
      FALSE, 0);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   return ret;
}


EXTRTMETHOD int rtAvnReadOctStringDyn(OSCTXT* pctxt, OSDynOctStr* pvalue)
{
   int ret;
   OSSIZE nocts;
   OSOCTET* pbuffer = 0;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = decodebhstring_internal(pctxt, &pbuffer, 0, TRUE, 0, &nocts,
      FALSE, 0);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

#if SIZE_MAX > OSUINT32_MAX
   if (nocts > OSUINT32_MAX)
      return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif

   pvalue->data = pbuffer;
   pvalue->numocts = (OSUINT32)nocts;

   return ret;
}


EXTRTMETHOD int rtAvnReadOctStringDyn64(OSCTXT* pctxt, OSDynOctStr64* pvalue)
{
   int ret;

   ret = rtxTxtMatchChar(pctxt, '\'', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   pvalue->data = 0;

   ret = decodebhstring_internal(pctxt, &pvalue->data, 0, TRUE, 0,
         &pvalue->numocts, FALSE, 0);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   return ret;
}
