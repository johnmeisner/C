/*
 * Copyright (c) 2014-2023 by Objective Systems, Inc.
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

#include "rtxsrc/rtxDynBitSet.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

EXTRTMETHOD int rtxDynBitSetInit
(OSCTXT* pctxt, OSRTDynBitSet* pbitset, OSUINT16 segNumBytes)
{
   OSRTASSERT (0 != pbitset);
   pbitset->segsize = (OSUINT16)
      ((0 == segNumBytes) ? OSRTBITSETSEGSIZE : segNumBytes);
   pbitset->data = (OSOCTET*) rtxMemAllocZ (pctxt, pbitset->segsize);
   if (0 == pbitset->data) return LOG_RTERR (pctxt, RTERR_NOMEM);
   pbitset->nbytes = pbitset->segsize;
   pbitset->maxbit = 0;
   return 0;
}

EXTRTMETHOD void rtxDynBitSetFree (OSCTXT* pctxt, OSRTDynBitSet* pbitset)
{
   OSRTASSERT (0 != pbitset);
   rtxMemFreePtr (pctxt, pbitset->data);
   pbitset->data = 0;
   pbitset->nbytes = 0;
   pbitset->maxbit = 0;
}

EXTRTMETHOD int rtxDynBitSetCopy
(OSCTXT* pctxt, const OSRTDynBitSet* pSrcBitSet, OSRTDynBitSet* pDestBitSet)
{
   OSRTASSERT (0 != pSrcBitSet);
   OSRTASSERT (0 != pDestBitSet);
   pDestBitSet->data = (OSOCTET*) rtxMemAlloc (pctxt, pSrcBitSet->segsize);
   if (0 == pDestBitSet->data) return LOG_RTERR (pctxt, RTERR_NOMEM);
   pDestBitSet->nbytes = pSrcBitSet->nbytes;
   pDestBitSet->maxbit = pSrcBitSet->maxbit;
   pDestBitSet->segsize = pSrcBitSet->segsize;
   OSCRTLSAFEMEMCPY (pDestBitSet->data, pSrcBitSet->segsize,
         pSrcBitSet->data, pSrcBitSet->segsize);
   return 0;
}

EXTRTMETHOD int rtxDynBitSetSetBit
(OSCTXT* pctxt, OSRTDynBitSet* pbitset, OSUINT32 idx)
{
   if (idx > OSUINT16_MAX)
      return LOG_RTERR (pctxt, RTERR_OUTOFBND);

   if (idx >= (OSUINT32)(pbitset->nbytes * 8)) {
      /* expand buffer */
      size_t newSize = (idx / (pbitset->segsize * 8) + 1) * pbitset->segsize;
      OSOCTET* p = (OSOCTET *)rtxMemRealloc (pctxt, pbitset->data, newSize);
      if (0 != p) {
         OSCRTLMEMSET (p + pbitset->nbytes, 0, newSize - pbitset->nbytes);
         pbitset->data = p;
         pbitset->nbytes = (OSUINT16) newSize;
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   rtxSetBit (pbitset->data, pbitset->nbytes * 8, idx);
   if (idx >= pbitset->maxbit) pbitset->maxbit = (OSUINT16)(idx + 1);
   return 0;
}

EXTRTMETHOD int rtxDynBitSetClearBit (OSRTDynBitSet* pbitset, OSUINT32 idx)
{
   if (idx < (OSUINT32)(pbitset->nbytes * 8)) {
      rtxClearBit (pbitset->data, pbitset->nbytes * 8, idx);
      if ((idx + 1) == pbitset->maxbit) {
         /* compute the new maxbit value */
         OSOCTET* p = pbitset->data + idx / 8;
         OSOCTET* pEnd = pbitset->data;
         OSUINT32 maxbit = idx / 8 * 8 + 7;

         while (p != pEnd && *p == 0) {
            p--;
            maxbit -= 8;
         }

         if (*p == 0)
            maxbit = 0;
         else {
            OSOCTET c = *p;
            while ((c & 1) == 0) {
               maxbit--;
               c >>= 1;
            }
         }

         pbitset->maxbit = (OSUINT16) (maxbit + 1);
      }
   }
   return 0;
}

EXTRTMETHOD OSBOOL rtxDynBitSetTestBit
(const OSRTDynBitSet* pbitset, OSUINT32 idx)
{
   return (OSBOOL)
      ((idx >= pbitset->maxbit) ?
       FALSE : rtxTestBit (pbitset->data, pbitset->maxbit, idx));
}

EXTRTMETHOD int rtxDynBitSetSetBitToValue
(OSCTXT* pctxt, OSRTDynBitSet* pbitset, OSUINT32 idx, OSBOOL value)
{
   return (value) ?
      rtxDynBitSetSetBit (pctxt, pbitset, idx) :
      rtxDynBitSetClearBit (pbitset, idx);
}

EXTRTMETHOD int rtxDynBitSetInsertBit
(OSCTXT* pctxt, OSRTDynBitSet* pbitset, OSUINT32 idx, OSBOOL value)
{
   if (idx < pbitset->maxbit) {
      OSOCTET* p = pbitset->data + idx / 8;
      OSOCTET* pEnd = pbitset->data + pbitset->maxbit / 8;
      OSOCTET c;
      OSOCTET mask;

      if (pbitset->maxbit + 1 == pbitset->nbytes * 8) {
         int stat = rtxDynBitSetSetBitToValue (pctxt, pbitset,
                                               pbitset->maxbit + 1, TRUE);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
      }
      else
         pbitset->maxbit++;

      /* shift part of byte */
      c = *p;
      mask = (OSOCTET) (0xFF >> (idx % 8));
      *p = (OSOCTET) ((c & ~mask) | ((c & mask) >> 1));
      if (value) *p |= 0x80 >> (idx % 8);

      /* shift other bytes to maxbit */
      if (p != pEnd) {
         do {
            p++;
            *p = c = (OSOCTET) ((*p >> 1) | (c << 7));
         } while (p != pEnd);
      }
   }
   else {
      int stat = rtxDynBitSetSetBitToValue (pctxt, pbitset, idx, value);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

#ifdef _TRACE
EXTRTMETHOD void rtxDynBitSetPrint
(const OSRTDynBitSet* pbitset, const char* varname)
{
   OSUINT16 i;
   if (0 != varname) printf ("%s = ", varname);
   for (i = 0; i < pbitset->maxbit; i++) {
      printf (rtxDynBitSetTestBit (pbitset, i) ? "1" : "0");
   }
   printf ("\n");
}
#endif
