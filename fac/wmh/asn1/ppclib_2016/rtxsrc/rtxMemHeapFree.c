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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemHeap.hh"

EXTRTMETHOD void rtxMemHeapSysFreePtr (void** ppvMemHeap, void* mem_p)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0 || mem_p == 0)
      return;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   /* There's no way to zero this memory properly because we cannot determine
    * its extent.  This really isn't a problem since the memory isn't managed
    * by our heap anyway. */
   _g_free_func (pMemHeap, mem_p);
}

EXTRTMETHOD void rtxMemHeapFreePtr (void** ppvMemHeap, void* mem_p)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;
#ifdef _MEMDEBUG
   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapFreePtr: start\n");
#endif
   if (!(pMemHeap->flags & RT_MH_STATIC)) {
      rtxMemHeapAutoPtrUnref (ppvMemHeap, mem_p);
   }
#ifdef _MEMDEBUG
   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapFreePtr: end\n");
#endif
}

#ifndef _MEMCOMPACT

void _rtxMemHeapFreeElem (void** ppvMemHeap, OSMemElemDescr* pElem)
{
   OSMemHeap* pMemHeap;
   OSMemBlk* pMemBlk = GET_MEMBLK (pElem);
   OSMemElemDescr* pNextElem = GETNEXT (pElem);
   OSMemElemDescr* pPrevElem = GETPREV (pElem);
   OSMemElemDescr* p;
   OSFreeElemLink* pFreeLink;
   OSUINT16 nunits, offset;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   TRACEMEMELEM(pMemBlk, pElem, "Freed");

   if (ISSAVED (pElem))
      pMemBlk->nsaved--;

   if (ISFREE (pNextElem)) {
      if (ISFREE (pPrevElem)) {
         /* remove prev element from free list */
         OSUINT16 nextFree, prevFree;

         pFreeLink = GET_FREE_LINK (pPrevElem);
         if (0 == pFreeLink) {
            TRACEMEMELEM(pMemBlk, pPrevElem, "(0 == pFreeLink)!");
            return;
         }
         GET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, nextFree);

         GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, offset);

         if (pMemBlk->firstFree == offset)
            pMemBlk->firstFree = nextFree;

         SET_OSUINT16 (GET_PREV_FREE (pMemBlk, pFreeLink),
                       OSFreeElemLink_nextFree, nextFree);

         GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, prevFree);

         SET_OSUINT16 (GET_NEXT_FREE (pMemBlk, pFreeLink),
                       OSFreeElemLink_prevFree, prevFree);

         /* join three free elements */
         pElem = pPrevElem;

         if (pMemHeap->flags & RT_MH_ZEROONFREE) {
            /* pElem is already set to be free */
            _rtxMemHeapZeroElem (pElem);
         }
      }
      else {
         SET_FREE (pElem);
         if (pMemHeap->flags & RT_MH_ZEROONFREE) {
            _rtxMemHeapZeroElem (pElem);
         }
      }

      p = GETNEXT (pNextElem);
      GET_OFFSET_OSUINT16 (p, pElem, offset);
      SET_OSUINT16 (p, OSMemElemDescr_prevOff, offset);
      SET_OSUINT16 (pElem, OSMemElemDescr_nunits, offset);
   }
   else if (ISFREE (pPrevElem)) {
      OSUINT16 freeOff, nextFree, prevFree, u16;
      OSUINT16 firstFreeOff = pMemBlk->firstFree;
      OSFreeElemLink* pPrevFreeLink = GET_FREE_LINK (pPrevElem);
      if (0 == pPrevFreeLink) {
         /* This should never happen, but causes warnings in some code
          * verifiers. */
         TRACEMEMELEM(pMemBlk, pPrevElem, "(0 == pPrevFreeLink)!");
         return;
      }

      pFreeLink = GET_FREE_LINK (pElem);
      if (0 == pFreeLink) {
         TRACEMEMELEM(pMemBlk, pPrevElem, "(0 == pFreeLink)!");
         return;
      }
      GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, freeOff);
      GET_OFFSET_OSUINT16 (pPrevFreeLink, &pMemBlk->code, u16);

      if (firstFreeOff == u16)
         pMemBlk->firstFree = freeOff;

      GET_OSUINT16 (pPrevFreeLink, OSFreeElemLink_nextFree, nextFree);
      SET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, nextFree);

      GET_OSUINT16 (pPrevFreeLink, OSFreeElemLink_prevFree, prevFree);
      SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, prevFree);

      SET_OSUINT16 (GET_PREV_FREE (pMemBlk, pPrevFreeLink),
                    OSFreeElemLink_nextFree, freeOff);

      SET_OSUINT16 (GET_NEXT_FREE (pMemBlk, pPrevFreeLink),
                    OSFreeElemLink_prevFree, freeOff);

      GET_OFFSET_OSUINT16 (pNextElem, pPrevElem, u16);
      SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, u16);
      SET_OSUINT16 (pPrevElem, OSMemElemDescr_nunits, u16);

      pElem = pPrevElem;
      if (pMemHeap->flags & RT_MH_ZEROONFREE) {
         /* pElem already set free in the flags */
         _rtxMemHeapZeroElem (pElem);
      }
   }
   else {
      OSUINT16 freeOff = pMemBlk->firstFree;
      OSFreeElemLink* pFirstFreeLink = GET_FIRST_FREE (pMemBlk);

      SET_FREE (pElem);

      if (pMemHeap->flags & RT_MH_ZEROONFREE) {
         _rtxMemHeapZeroElem (pElem);
      }

      pFreeLink = GET_FREE_LINK (pElem);
      if (0 == pFreeLink) {
         TRACEMEMELEM(pMemBlk, pPrevElem, "(0 == pFreeLink)!");
         return;
      }
      SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, freeOff);
      GET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree, freeOff);
      SET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, freeOff);

      GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, freeOff);

      SET_OSUINT16 (GET_NEXT_FREE (pMemBlk, pFirstFreeLink),
                    OSFreeElemLink_prevFree, freeOff);

      SET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree, freeOff);
   }

   GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

   if (nunits > pMemBlk->maxFreeElem)
      pMemBlk->maxFreeElem = nunits;

   if (nunits == pMemBlk->nunits - 2 &&
       (pMemHeap->flags & RT_MH_DONTKEEPFREE) /*||
       (pMemHeap->keepFreeUnits > 0 &&
        pMemHeap->freeUnits + pMemBlk->nunits - 2 > pMemHeap->keepFreeUnits)*/)
   {
      TRACEMEMBLK(pMemBlk, "Freed");

      if (pMemBlk->link.pnext == pMemBlk->link.pprev &&
          &pMemBlk->link == pMemBlk->link.pnext)
      {
         /* last memory block */
         pMemHeap->u.blockList.curMemBlk = 0;
      }
      else {
         /* remove from mem block list */
         pMemBlk->link.pprev->pnext = pMemBlk->link.pnext;
         pMemBlk->link.pnext->pprev = pMemBlk->link.pprev;

         if (pMemHeap->u.blockList.curMemBlk == &pMemBlk->link)
            pMemHeap->u.blockList.curMemBlk = pMemBlk->link.pnext;
      }

      if (pMemHeap->flags & RT_MH_ZEROONFREE) {
         _rtxMemHeapZeroMemBlk (pMemBlk);

         RTMEMDIAGDBG2 (pMemHeap, "~I _rtxMemHeapFreeElem: zeroed out "
            "memory block %p\n", pMemBlk);
      }

      _g_free_func (pMemHeap, pMemBlk);
      RTMEMDIAGDBG2 (pMemHeap, "~D _rtxMemHeapFreeElem: freed memory "
                     "block %p\n", pMemBlk);
   }
}

EXTRTMETHOD int rtxMemHeapAutoPtrUnref (void** ppvMemHeap, void* mem_p)
{
   OSMemHeap* pMemHeap;
   OSMemElemDescr* pElem;
   OSUINT8 cnt;

   if (mem_p == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0)
      return RTERR_NULLPTR;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrUnref: start\n");

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   if ((pMemHeap->flags & RT_MH_VALIDATEPTR) &&
       rtxMemHeapCheckPtr (ppvMemHeap, mem_p) == 0)
   {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return RTERR_NOMEM;
   }

   pElem = GET_ELEM (mem_p);

   if (!rtxMemHeapCheckCode (pElem)) {
      /* rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p); */
      RTMEMDIAG2 (pMemHeap, "~E rtxMemHeapAutoPtrUnref(%p): "
                  "invalid pointer\n", mem_p);

      return RTERR_NOMEM;
   }

   cnt = pElem_refcnt (pElem);

   if (cnt == 0 || ISFREE (pElem)) {
      RTMEMDIAG2 (pMemHeap, "~E rtxMemHeapAutoPtrUnref(%p): "
                  "element already freed!\n", mem_p);
      return 0;
   }
   else if (cnt > 1) {
      pElem_refcnt (pElem) = --cnt;

      RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAutoPtrUnref(%p): "
                  "refcnt decremented to %d\n", mem_p, cnt);
      RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrUnref: end\n");
      return (int) cnt;
   }
   else
      pElem_refcnt (pElem) = 0;

   RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref(%p): free element\n",
               mem_p);

   if (ISRAW (pElem)) {
      OSRawMemBlk* pRawBlk = GET_RAWMEMBLK (pElem);

      /* remove from raw block list */
      pRawBlk->link.pprev->pnext = pRawBlk->link.pnext;
      pRawBlk->link.pnext->pprev = pRawBlk->link.pprev;

      TRACERAWMEMBLK(pRawBlk, "Freed");

      if (pMemHeap->flags & RT_MH_ZEROONFREE) {
         _rtxMemHeapZeroRawBlk (pRawBlk);

         RTMEMDIAGDBG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref: zeroed out "
            "raw block %p\n", pRawBlk);
      }

      _g_free_func (pMemHeap, pRawBlk);
      RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref: freed raw block %p\n",
                  pRawBlk);
   }
   else {
      _rtxMemHeapFreeElem (ppvMemHeap, pElem);
   }

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrUnref: end\n");
   return 0;
}

#else /* _MEMCOMPACT */

EXTRTMETHOD int rtxMemHeapAutoPtrUnref (void** ppvMemHeap, void* mem_p)
{
   OSMemHeap* pMemHeap;
   OSMemElemDescr* pElem;
   OSRawMemBlk* pRawBlk;
   OSUINT8 cnt;

   if (mem_p == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0)
      return RTERR_NULLPTR;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrUnref: start\n");
#ifdef _MEMDEBUG
   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   if ((pMemHeap->flags & RT_MH_VALIDATEPTR) &&
       rtxMemHeapCheckPtr (ppvMemHeap, mem_p) == 0)
   {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return RTERR_NOMEM;
   }
#endif
   pElem = GET_ELEM (mem_p);

   if (!rtxMemHeapCheckCode (pElem)) {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return RTERR_NOMEM;
   }

   cnt = pElem_refcnt (pElem);

   if (cnt == 0) {
      RTMEMDIAG2 (pMemHeap, "~E rtxMemHeapAutoPtrUnref(%p): "
                  "element already freed!\n", mem_p);
      return 0;
   }
   else if (cnt > 1) {
      pElem_refcnt (pElem) = --cnt;

      RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAutoPtrUnref(%p): "
                  "refcnt decremented to %d\n", mem_p, cnt);

      return (int) cnt;
   }
   else
      pElem_refcnt (pElem) = 0;

   RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref(%p): free element\n",
               mem_p);

   pRawBlk = GET_RAWMEMBLK (mem_p);
   TRACERAWMEMBLK(pRawBlk, "Freed");

   /* remove from raw block list */
   pRawBlk->link.pprev->pnext = pRawBlk->link.pnext;
   pRawBlk->link.pnext->pprev = pRawBlk->link.pprev;

   if (pMemHeap->flags & RT_MH_ZEROONFREE) {
      RTMEMDIAGDBG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref: cannot zero raw "
         "block %p with compact memheap\n", pRawBlk);
   }

   _g_free_func (pMemHeap, pRawBlk);
   RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapAutoPtrUnref: freed raw block %p\n",
               pRawBlk);

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrUnref: end\n");
   return 0;
}

#endif /* _MEMCOMPACT */

/* Clears heap memory (frees all memory, reset all heap's variables) */
EXTRTMETHOD void rtxMemHeapFreeAll (void** ppvMemHeap)
{
   OSMemHeap* pMemHeap;
   OSMemLink* pMemLink;
   OSMemLink* pMemLinkEnd;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

#ifndef _MEMCOMPACT
   if (pMemHeap->flags & RT_MH_STATIC) {
      pMemHeap->u.staticMem.freeIdx = 0;
      return;
   }

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapFreeAll: start\n");
#endif
   /* free raw blocks */
   pMemLink = pMemHeap->u.blockList.rawBlkList.pnext;
   pMemLinkEnd = &pMemHeap->u.blockList.rawBlkList;

   while (pMemLink != pMemLinkEnd) {
      OSRawMemBlk* pRawBlk = TO_RAWMEMBLK (pMemLink);
      TRACERAWMEMBLK(pRawBlk, "Freed");
      pMemLink = pMemLink->pnext;
#ifndef _MEMCOMPACT
      if (pMemHeap->flags & RT_MH_ZEROONFREE) {
         _rtxMemHeapZeroRawBlk (pRawBlk);

         RTMEMDIAGDBG2 (pMemHeap, "~I rtxMemHeapFreeAll: zeroed out "
            "raw block %p\n", pRawBlk);
      }
#endif
      _g_free_func (pMemHeap, pRawBlk);
      RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapFreeAll: freed raw block %p\n",
                  pRawBlk);
   }

   pMemHeap->u.blockList.rawBlkList.pnext =
      pMemHeap->u.blockList.rawBlkList.pprev =
      &pMemHeap->u.blockList.rawBlkList;

#ifndef _MEMCOMPACT
   /* free mem blocks */
   pMemLink = pMemHeap->u.blockList.curMemBlk;
   pMemLinkEnd = pMemLink;

   if (pMemLink) {
      do {
         OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);
         TRACEMEMBLK(pMemBlk, "Freed");
         pMemLink = pMemLink->pnext;

         if (pMemHeap->flags & RT_MH_ZEROONFREE) {
            _rtxMemHeapZeroMemBlk (pMemBlk);

            RTMEMDIAGDBG2 (pMemHeap, "~I rtxMemHeapFreeAll: zeroed out "
               "mem block %p\n", pMemBlk);
         }

         _g_free_func (pMemHeap, pMemBlk);
         RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapFreeAll: "
                     "freed memory block %p\n", pMemBlk);
      } while (pMemLink != pMemLinkEnd);

      pMemHeap->u.blockList.curMemBlk = 0;
   }
#endif

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapFreeAll: end\n");
}
