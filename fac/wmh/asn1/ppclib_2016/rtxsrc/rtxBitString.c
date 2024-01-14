/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include <stdarg.h>
#include <stdlib.h>
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemory.h"

/* Sets bit in bit string */
EXTRTMETHOD int rtxSetBit (OSOCTET* pBits, OSSIZE numbits, OSSIZE bitIndex)
{
   OSOCTET prev, mask = 0x80, *pbyte;

   if (bitIndex >= numbits)
      return RTERR_OUTOFBND;
   pbyte = pBits + bitIndex / 8;
   mask >>= bitIndex % 8;
   prev = *pbyte;
   *pbyte = (OSOCTET)(prev | mask);
   return prev & mask;
}

/* Clears bit in bit string */
EXTRTMETHOD int rtxClearBit
(OSOCTET* pBits, OSSIZE numbits, OSSIZE bitIndex)
{
   OSOCTET prev, mask = 0x80, *pbyte;

   if (bitIndex >= numbits)
      return RTERR_OUTOFBND;
   pbyte = pBits + bitIndex / 8;
   mask >>= bitIndex % 8;
   prev = *pbyte;
   *pbyte = (OSOCTET)(prev & (~mask));
   return prev & mask;
}

/* Tests bit in bit string */
EXTRTMETHOD OSBOOL rtxTestBit
(const OSOCTET* pBits, OSSIZE numbits, OSSIZE bitIndex)
{
   return (OSBOOL)((bitIndex >= numbits) ? FALSE :
      (pBits [bitIndex / 8] & (0x80 >> bitIndex % 8)));
}

/* Set or clears bits in a flags word using a mask */
EXTRTMETHOD OSUINT32 rtxSetBitFlags
(OSUINT32 flags, OSUINT32 mask, OSBOOL action)
{
   if (action) flags |= mask; else flags &= ~mask;
   return flags;
}

/* Returns the index of the last bit set in the bit string; use for trimming
 * a bit string. */
EXTRTMETHOD OSSIZE rtxLastBitSet (const OSOCTET *pBits, OSSIZE numbits)
{
   OSSIZE ct;
   for (ct = numbits - 1; ct != OSNULLINDEX; ct --) {
      if (rtxTestBit (pBits, numbits, ct)) return ct;
   }

   return OSNULLINDEX;
}

EXTRTMETHOD int rtxCheckBitBounds(OSCTXT* pctxt, OSOCTET** ppBits,
   OSSIZE* pNumocts, OSSIZE minRequiredBits, OSSIZE preferredLimitBits)
{
   OSOCTET* pBits = *ppBits;
   OSSIZE numocts = *pNumocts;
   OSSIZE minOctets;

   if ( pBits == 0 ) numocts = 0;   /* avoid parameter inconsistency */

   minOctets = (minRequiredBits + 7 ) / 8;
   if ( minOctets > numocts ) {
      /* The current string is too small & needs to be expanded */
      /* The rules here are: */
      /* 1) must have at least minOctets */
      /* 2) we'll generally grow by at least half-again (minOctets can require */
      /*    more growth), except we only grow beyond preferredLimitOcts when */
      /*    minOctets demands it. */
      /* 3) we don't create a string of less than 32 bytes unless */
      /*    preferredLimitOcts calls for this */
      OSOCTET* newBuffer;
      OSSIZE newNumOcts = numocts + numocts / 2;
      OSSIZE preferredLimitOcts = (preferredLimitBits + 7) / 8;

      if ( newNumOcts < 32 ) newNumOcts = 32;
      if ( newNumOcts >= minOctets )
      {
         if ( preferredLimitOcts > 0 && newNumOcts > preferredLimitOcts ) {
            if ( preferredLimitOcts >= minOctets )
               newNumOcts = preferredLimitOcts;
            else
               newNumOcts = minOctets;
         }
      }
      else newNumOcts = minOctets;

      newBuffer = rtxMemAlloc(pctxt, newNumOcts);
      if ( newBuffer == NULL ) return RTERR_NOMEM;

      if ( numocts > 0 )
         OSCRTLSAFEMEMCPY(newBuffer, newNumOcts, pBits, numocts);

      OSCRTLMEMSET(newBuffer + numocts, 0, newNumOcts - numocts);

      rtxMemFreePtr(pctxt, pBits);
      *ppBits = newBuffer;
      *pNumocts = newNumOcts;
   }

   return 0;
}

#define UINTBITS (sizeof(OSUINT32)*8)

EXTRTMETHOD OSUINT32 rtxGetBitCount (OSUINT32 value)
{
   /* Binary search - decision tree (5 tests, rarely 6) */
   return
      ((value < 1UL<<15) ?
       ((value < 1UL<<7) ?
        ((value < 1UL<<3) ?
         ((value < 1UL<<1) ? ((value < 1UL<<0) ? 0 : 1) : ((value < 1UL<<2) ? 2 : 3)) :
         ((value < 1UL<<5) ? ((value < 1UL<<4) ? 4 : 5) : ((value < 1UL<<6) ? 6 : 7))) :
        ((value < 1UL<<11) ?
         ((value < 1UL<<9) ? ((value < 1UL<<8) ? 8 : 9) : ((value < 1UL<<10) ? 10 : 11)) :
         ((value < 1UL<<13) ? ((value < 1UL<<12) ? 12 : 13) : ((value < 1UL<<14) ? 14 : 15)))) :
       ((value < 1UL<<23) ?
        ((value < 1UL<<19) ?
         ((value < 1UL<<17) ? ((value < 1UL<<16) ? 16 : 17) : ((value < 1UL<<18) ? 18 : 19)) :
         ((value < 1UL<<21) ? ((value < 1UL<<20) ? 20 : 21) : ((value < 1UL<<22) ? 22 : 23))) :
        ((value < 1UL<<27) ?
         ((value < 1UL<<25) ? ((value < 1UL<<24) ? 24 : 25) : ((value < 1UL<<26) ? 26 : 27)) :
         ((value < 1UL<<29) ? ((value < 1UL<<28) ? 28 : 29) : ((value < 1UL<<30) ? 30 :
         ((value < 1UL<<31) ? 31 : 32))))));
}

EXTRTMETHOD int rtxZeroUnusedBits (OSOCTET* pBits, OSSIZE numbits)
{
   if (0 != (numbits % 8)) {
      OSOCTET mask = 0xFF;
      OSSIZE idx = numbits / 8;
      OSSIZE lshift = 8 - numbits;
      mask <<= lshift;
      if (0 != pBits)
         pBits[idx] &= mask;
      else
         return RTERR_INVPARAM;
   }

   return 0;
}

EXTRTMETHOD int rtxCheckUnusedBitsZero (const OSOCTET* pBits, OSSIZE numbits)
{
   if (0 != (numbits % 8)) {
      OSOCTET mask = 0xFF;
      OSSIZE idx = numbits / 8;
      OSSIZE rshift = numbits % 8;
      mask >>= rshift;
      if (0 != pBits) {
         if (0 != (pBits[idx] & mask)) {
            return RTERR_INVBITS;
         }
      }
      else
         return RTERR_INVPARAM;
   }

   return 0;
}
