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

#include "rtxsrc/rtxMemHeap.hh"

/* increments internal refCnt. use rtxMemHeapRelease to decrement and release */
EXTRTMETHOD void rtxMemHeapAddRef (void** ppvMemHeap)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0) return;
   pMemHeap = *(OSMemHeap**)ppvMemHeap;
   pMemHeap->refCnt++;
}

/* Frees all memory and heap structure as well (if was allocated) */
EXTRTMETHOD void rtxMemHeapRelease (void** ppvMemHeap)
{
   OSMemHeap** ppMemHeap = (OSMemHeap**)ppvMemHeap;

   if (ppMemHeap != 0 && *ppMemHeap != 0 && --(*ppMemHeap)->refCnt == 0) {
      rtxMemHeapFreeAll (ppvMemHeap);

      if ((*ppMemHeap)->flags & RT_MH_FREEHEAPDESC) {
         _g_free_func ((*ppMemHeap), *ppMemHeap);
      }

      *ppMemHeap = 0;
   }
}

EXTRTMETHOD int rtxMemHeapCheckPtr (void** ppvMemHeap, const void* mem_p)
{
   OSMemHeap* pMemHeap;
   OSMemLink* pMemLink;
   OSMemLink* pMemLinkEnd;

   if (mem_p == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0)
      return 0;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

   if (0 != (pMemHeap->flags & RT_MH_STATIC)) {
      return 0;
   }
   /* raw blocks */
   pMemLink = pMemHeap->u.blockList.rawBlkList.pnext;
   pMemLinkEnd = &pMemHeap->u.blockList.rawBlkList;

   while (pMemLink != pMemLinkEnd) {
      OSRawMemBlk* pRawBlk = TO_RAWMEMBLK (pMemLink);

      if (pRawBlk + 1 == mem_p)
         return 1;

      pMemLink = pMemLink->pnext;
   }

#ifndef _MEMCOMPACT
   /* memory blocks */
   pMemLink = pMemHeap->u.blockList.curMemBlk;
   pMemLinkEnd = pMemLink;

   if (pMemLink) {
      do {
         OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);
         OSMemElemDescr* pElem;
         OSMemElemDescr* pElemEnd;

         pElem = GET_ELEM_N (pMemBlk, 2);
         pElemEnd = GET_ELEM_N (pMemBlk, pMemBlk->nunits);

         if (pElem <= mem_p && mem_p <= pElemEnd) {
            while (pElem != pElemEnd) {
               if (!ISFREE(pElem) && pElem_data(pElem) == mem_p)
                  return 1;

               pElem = GETNEXT (pElem);
            }
         }

         pMemLink = pMemLink->pnext;
      } while (pMemLink != pMemLinkEnd);
   }

#endif
   return 0;
}

OSBOOL rtxMemInvalidPtr(OSMemHeap* pHeap, void* ptr)
{
   return ( ptr < (void*) pHeap->u.blockList.pMinAddress ||
            ptr > (void*) pHeap->u.blockList.pMaxAddress );
}

OSBOOL rtxMemHeapCheckCode (OSMemHeap* pHeap, const OSMemElemDescr* pElem)
{
   OSOCTET* octptr = (OSOCTET*)pElem;
#ifndef _MEMCOMPACT
   OSUINT32* pcode;
   OSUINT16 beginOff;

   GET_OSUINT16 (pElem, OSMemElemDescr_beginOff, beginOff);
   octptr -= (beginOff * 8u);
   pcode = (OSUINT32*) octptr;
#else
   OSUINT16* pcode = (OSUINT16*) octptr;
#endif
   return *pcode == pHeap->code;
}

#ifndef _MEMCOMPACT

OSMemElemDescr* rtxMemHeapGetFreeElem (const OSFreeElemLink* pFreeLink)
{
   OSUINT16 freeNUnits;
   OSOCTET* freeLinkOctPtr = (OSOCTET*) pFreeLink;

   GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff, freeNUnits);
   if (freeNUnits > 0) {
      /*  go back 1 unit less than freeNUnits because we are starting at */
      /*  the free link location and not the start of the next memory element. */
      /*  The free link pointer is already 1 unit back from the next memory */
      /*  element. */
      size_t freeByteOffset = (freeNUnits - 1) * 8u;
      return (OSMemElemDescr*)(freeLinkOctPtr - freeByteOffset);
   }
   else return 0;
}

OSFreeElemLink* rtxMemHeapGetFreeLink (const OSMemElemDescr* pElem)
{
   OSUINT16 nunits;
   OSOCTET* elemOctPtr = (OSOCTET*) pElem;

   GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);
   if (nunits > 0) {
      size_t byteOffset = (nunits - 1) * 8u;
      return (OSFreeElemLink*)(elemOctPtr + byteOffset);
   }
   else return 0;
}

void* rtxMemHeapGetMemBlk (const OSMemElemDescr* pElem)
{
   const OSOCTET* octptr = (const OSOCTET*) pElem;
   OSUINT16 beginOff;

   GET_OSUINT16 (pElem, OSMemElemDescr_beginOff, beginOff);

   return (void*)(octptr - (beginOff * 8u) - (sizeof(OSMemBlk) - 8u));
}

void* rtxMemHeapGetRawMemBlk (const OSMemElemDescr* pElem)
{
   const OSOCTET* octptr = (const OSOCTET*) pElem;
   OSUINT16 beginOff;

   GET_OSUINT16 (pElem, OSMemElemDescr_beginOff, beginOff);

   return (void*)
      (octptr - (beginOff * 8u) - (sizeof(OSRawMemBlk) - 8u));
}

OSMemElemDescr* rtxMemHeapGetNextElem (const OSMemElemDescr* pCurrElem)
{
   OSUINT16 nunits;
   OSOCTET* currElemOctPtr = (OSOCTET*) pCurrElem;

   GET_OSUINT16 (pCurrElem, OSMemElemDescr_nunits, nunits);

   return (OSMemElemDescr*)(currElemOctPtr + (nunits * 8u));
}

OSMemElemDescr* rtxMemHeapGetPrevElem (const OSMemElemDescr* pCurrElem)
{
   OSUINT16 prevOff;
   OSOCTET* currElemOctPtr = (OSOCTET*) pCurrElem;

   GET_OSUINT16 (pCurrElem, OSMemElemDescr_prevOff, prevOff);

   return (OSMemElemDescr*)(currElemOctPtr - (prevOff * 8u));
}

OSFreeElemLink* rtxMemHeapGetNextFree
(const OSMemBlk* pMemBlk, const OSFreeElemLink* pFree)
{
   const OSOCTET* octptr = (const OSOCTET*) &pMemBlk->code;
   OSUINT16 nextFree;

   GET_OSUINT16 (pFree, OSFreeElemLink_nextFree, nextFree);

   return (OSFreeElemLink*)(octptr + (nextFree * 8u));
}

OSFreeElemLink* rtxMemHeapGetPrevFree
(const OSMemBlk* pMemBlk, const OSFreeElemLink* pFree)
{
   const OSOCTET* octptr = (const OSOCTET*) &pMemBlk->code;
   OSUINT16 prevFree;

   GET_OSUINT16 (pFree, OSFreeElemLink_prevFree, prevFree);

   return (OSFreeElemLink*)(octptr + (prevFree * 8u));
}

void _rtxMemHeapZeroElem (OSMemElemDescr* pElem)
{
   OSUINT16 nunits;
   GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

   /* Recall that the memory element has a header and footer, each of which
    * consumes a single 8-byte unit. The header consumes a byte, so we will
    * zero out nunits-1 8-byte units. N.b., the nunits value does not account
    * for one of the header or the footer. */
   OSCRTLMEMSET (pElem_data(pElem), 0, (nunits-1)*8);
#ifdef _DEBUG
   rtxDiagPrint (0, "~I rtxMemHeapZeroElem: zeroed out "
      "memory element %p (%p)\n", pElem, pElem_data(pElem));
#endif
}

void _rtxMemHeapZeroMemBlk (OSMemBlk *pMemBlk)
{
   /* The size of the data is indicated by the number of 8-byte units; total
    * extent in memory is the size of the block plus the data it contains. */
   size_t blockSize = sizeof(OSMemBlk) + pMemBlk->nunits * 8;
   OSCRTLMEMSET (pMemBlk, 0, blockSize);
}

void _rtxMemHeapZeroRawBlk (OSRawMemBlk *pRawBlk)
{
   /* We do not store the memory block size in compact memory management,
    * so this evaluates to a NOOP here. */
   size_t blockSize = sizeof(OSRawMemBlk) + pRawBlk->memsize;
   OSCRTLMEMSET (pRawBlk, 0, blockSize);
}

#endif /* ifndef _MEMCOMPACT */
