/*
* Copyright (c) 1997-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxError.h"

/**
* Test the given bit in the bit string.
* idx is the index (0 based) of the bit to test.  Remaining parameters are as
* for rtAvnWriteBitStrNamed.
*
* @param idx Index of bit to test.
* Return is as for rtxTestBit
*/
int _rtTestBit(OSSIZE idx, OSSIZE nbits,
   const OSOCTET* pdata, OSSIZE datalen,
   const OSOCTET* pextdata)
{
   if ( idx >= nbits ) return FALSE;

   if (pextdata) {
      size_t maxbits = datalen * 8;   /* max possible bits in pdata */
      size_t extbits = nbits - maxbits; /* number of bits in pextdata IF
                                           nbits > datalen * 8 */

      if (idx >= maxbits)
      {
         /* We are testing a bit that is in pextdata. */
         return rtxTestBit(pextdata, extbits, idx - maxbits);
      }
      /* else: We are testing a bit in pdata. */
   }

   /* Testing bit in pdata. We know there are at least idx+1 bits in pdata. */
   return rtxTestBit(pdata, idx + 1, idx);
}


/**
 * Determine the bit index of the last bit set in a BIT STRING.
 * Arguments are as for rtAvnWriteBitStrNamed.
 * Return is as for rtxLastBitSet.
 */
OSSIZE _rtLastBitSet(OSSIZE nbits,
   const OSOCTET* pdata, OSSIZE datalen,
   const OSOCTET* pextdata)
{
   if (pextdata) {
      size_t maxbits = datalen * 8;   /* max possible bits in pdata */
      size_t extbits = nbits - maxbits; /* number of bits in pextdata IF
                                        nbits > datalen * 8 */

      if (nbits > maxbits)
      {
         size_t lastBitSet = OSNULLINDEX;

         /* Last bit set might be in pextdata. */
         lastBitSet = rtxLastBitSet(pextdata, extbits);
         if (lastBitSet == OSNULLINDEX)
         {
            /* Last bit set must be in pdata. */
            return rtxLastBitSet(pdata, maxbits);
         }
         else {
            /* Last bit is set in pextdata. Adjust index for # bits in pdata.*/
            return lastBitSet + maxbits;
         }
      }
      /* else: last bit is in pdata. */
   }

  /* Last bit set must be in pdata. We know there are nbits bits in pdata. */
  return rtxLastBitSet(pdata, nbits);
}


/**
 * Determines whether any unnamed bits in the BIT STRING are set.
 * Arguments are as for rtAvnWriteBitStrNamed.
 * @param lastBitSet If known, the index of the last bit that is set.
 *    If unknown, pass OSNULLINDEX
 * @return TRUE if an unnamed bit is set; FALSE otherwise.
 */
OSBOOL _rtUnnamedBitSet(OSSIZE nbits,
   const OSOCTET* pdata, OSSIZE datalen,
   const OSOCTET* pextdata,
   const OSBitMapItem* pdict,
   OSSIZE lastBitSet)
{
   size_t idx = 0;   /* index into pdict */
   size_t bidx = 0;  /* index into BIT STRING */

   if ( lastBitSet == OSNULLINDEX )
      lastBitSet = _rtLastBitSet(nbits, pdata, datalen, pextdata);

   if (lastBitSet == OSNULLINDEX)
      return FALSE;     /* no bits are set */

   /* Locate terminal entry in pdict. This method shouldn't be called when
   there are no entries in pdict besides the terminal entry, so idx should
   not end up as zero, but we watch out for that anyway. */
   while (pdict[idx].name != 0) { idx++; }

   /* We know some bit is set. If there are no named bits defined (idx == 0)
   or if the last bit set is beyond the last named bit defined, there are
   unnamed bits set.  Otherwise, there might or might not be unnamed bits
   set and we have to test bits to find out. */
   if ( idx == 0 || pdict[idx - 1].bitno < lastBitSet)
      return TRUE;

   /* If there are any unnamed bits set, they appear before some named bit.
      Search for an unnamed bit that is set. */
   bidx = 0;
   for(idx = 0;  pdict[idx].name; idx++) {
      OSUINT16 namedBit = pdict[idx].bitno;

      /* Test the (unnamed) bits from bidx up to but excluding the current
         named bit. */
      for (; bidx < namedBit; bidx++)
      {
         if (_rtTestBit(bidx, nbits, pdata, datalen, pextdata))
         {
            return TRUE;
         }
      }

      /* The next bit to test is the bit after the named bit. */
      bidx = namedBit + 1;
   }

   return FALSE;
}


EXTRTMETHOD int rtAvnWriteBitStrNamed(OSCTXT* pctxt, OSSIZE nbits,
   OSOCTET* pdata, OSSIZE datalen,
   OSOCTET* pextdata,
   const OSBitMapItem* pdict)
{
   int ret;
   size_t maxbits = datalen * 8;   /* max possible bits in pdata, IF pextdata
                                       is not null. */
   size_t extbits = nbits - maxbits; /* number of bits in pextdata IF
                                       pextdata is not null and
                                       nbits > datalen * 8 */
   size_t idx = 0;   /* index into pdict */
   size_t lastBitSet = OSNULLINDEX;
   OSBOOL unnamedBitsSet = FALSE;

   lastBitSet = _rtLastBitSet(nbits, pdata, datalen, pextdata);

   if (lastBitSet == OSNULLINDEX) {
      /* No bits are set.  We can output {} and return. */
      OSRTSAFEPUTCHAR(pctxt, '{');
      OSRTSAFEPUTCHAR(pctxt, '}');
      return 0;
   }

   unnamedBitsSet = _rtUnnamedBitSet(nbits, pdata, datalen, pextdata, pdict,
                                    lastBitSet);

   if (unnamedBitsSet)
   {
      /* An unnamed bit is set.
      Adjust nbits up to multiple of 4, making sure that unused bits are
      clear.  Then, encode as hstring using  rtAvnWriteBitStr. */
      OSSIZE rembits = nbits % 4;
      if ( rembits > 0 ) {
         if (pextdata && nbits > maxbits)
         {
            /* final byte is in pextdata.  */
            rtxZeroUnusedBits(pextdata, extbits);
         }
         else {
            /* final byte is in pdata. */
            rtxZeroUnusedBits(pdata, nbits);
         }
         nbits = nbits + (4 - rembits);
      }
      return rtAvnWriteBitStr(pctxt, nbits, pdata, datalen, pextdata);
   }
   else {
      /* Only named bits are set.  Use identifier list. */
      OSBOOL first = TRUE;
      OSRTSAFEPUTCHAR(pctxt, '{');

      while (pdict[idx].name != 0)
      {
         OSBOOL bitset = FALSE;

         if ( pdict[idx].bitno >= nbits ) break;
         else if ( pextdata && pdict[idx].bitno >= maxbits ) {
            /* bit must be in pextdata */
            bitset = rtxTestBit(pextdata, extbits, pdict[idx].bitno - maxbits);
         }
         else {
            /* bit must be in pdata.  We know pdata has at least
               pdict[idx].bitno + 1 bits.  */
            bitset = rtxTestBit(pdata, pdict[idx].bitno + 1, pdict[idx].bitno);
         }

         if (bitset)
         {
            if (!first) {
               OSRTSAFEPUTCHAR(pctxt, ',');
               OSRTSAFEPUTCHAR(pctxt, ' ');
            }
            ret = rtxCopyAsciiText(pctxt, (const char *)pdict[idx].name);
            if ( ret < 0 ) return LOG_RTERR(pctxt, ret);
            first = FALSE;
         }

         idx++;
      }

      OSRTSAFEPUTCHAR(pctxt, '}');
   }

   return 0;
}

EXTRTMETHOD int rtAvnWriteBitStr(OSCTXT* pctxt, OSSIZE nbits,
   const OSOCTET* pdata, OSSIZE datalen,
   const OSOCTET* pextdata)
{
   int ret;

   if (nbits % 4 == 0)
   {
      /* Encode using hstring. */
      const OSOCTET* p;    /* pointer to current byte */
      const OSOCTET* end;  /* pointer to byte after last byte in current part */
      const OSOCTET* strend;  /* pointer to byte after last byte in bit sring */
      char c;

      ret = rtxCheckOutputBuffer(pctxt, nbits / 4 + 3);
      if (ret < 0) return LOG_RTERR(pctxt, ret);

      c = '\''; OSRTPUTCHAR(pctxt, c);

      /* Output hex chars for the nibbles in pdata. */
      p = pdata;

      if ( nbits == 0 ) strend = end = p;
      else if ( pextdata == NULL || nbits < datalen * 8 )
      {
         /* All nbits bits are in pdata. */
         end = p + (nbits - 1) / 8 + 1;
         strend = end;
      }
      else {
         /* Some bits are in pextdata, namely nbits - datalen * 8. */
         end = p + datalen;
         strend = pextdata + (nbits - datalen * 8 - 1) / 8 + 1;
      }

      for(;;)
      {
         for (; p < end; p++)
         {
            OSINT8 b = (OSINT8) (*p >> 4);
            NIBBLETOUCHEXCHAR(b, c);
            OSRTPUTCHAR(pctxt, c);

            if (p < end - 1 || nbits % 8 == 0)
            {
               b = (OSINT8)(0x0F & *p);
               NIBBLETOUCHEXCHAR(b, c);
               OSRTPUTCHAR(pctxt, c);
            }
         }

         if ( p < strend )
         {
            /* Switch to bytes in pextdata.*/
            p = pextdata;
            end = strend;
         }
         else break;    /* No data in pextdata or we processed it. */
      }

      c = '\''; OSRTPUTCHAR(pctxt, c);
      c = 'H'; OSRTPUTCHAR(pctxt, c);
   }
   else {
      /* Encode using bstring */
      OSSIZE idx;
      char c;

      ret = rtxCheckOutputBuffer(pctxt, nbits + 3);
      if ( ret < 0 ) return LOG_RTERR(pctxt, ret);

      c = '\''; OSRTPUTCHAR(pctxt, c);

      for (idx = 0; idx < nbits; idx++)
      {
         c = _rtTestBit(idx, nbits, pdata, datalen, pextdata) ? '1' :  '0';
         OSRTPUTCHAR(pctxt, c);
      }
      c = '\''; OSRTPUTCHAR(pctxt, c);
      c = 'B'; OSRTPUTCHAR(pctxt, c);
   }

   return 0;
}
