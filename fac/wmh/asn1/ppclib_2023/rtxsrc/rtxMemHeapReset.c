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

/* This function will set the free index in all blocks to zero thereby  */
/* allowing the blocks to be reused.                                    */

EXTRTMETHOD void rtxMemHeapReset (void** ppvMemHeap)
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

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapReset: start\n");

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);
#endif
   /* free raw blocks */
   pMemLink = pMemHeap->u.blockList.rawBlkList.pnext;
   pMemLinkEnd = &pMemHeap->u.blockList.rawBlkList;

   while (pMemLink != pMemLinkEnd) {
      OSRawMemBlk* pRawBlk = TO_RAWMEMBLK (pMemLink);
      void* pMem = pRawBlk + 1;
      pMemLink = pMemLink->pnext;

      if (!ISSAVED (GET_ELEM(pMem))) {
         /* remove from raw block list */
         TRACERAWMEMBLK(pRawBlk, "Freed");

         pRawBlk->link.pprev->pnext = pRawBlk->link.pnext;
         pRawBlk->link.pnext->pprev = pRawBlk->link.pprev;
#ifndef _MEMCOMPACT
         if (pMemHeap->flags & RT_MH_ZEROONFREE) {
            _rtxMemHeapZeroRawBlk (pRawBlk);

            RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapReset: zeroed raw block "
                        "%p\n", pRawBlk);
         }
#endif
         _g_free_func (pMemHeap, pRawBlk);
         RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapReset: freed raw block %p\n",
                     pRawBlk);
      }
   }

#ifndef _MEMCOMPACT
   /* free mem blocks */
   pMemLink = pMemHeap->u.blockList.curMemBlk;

   if (pMemLink) {
      pMemLinkEnd = pMemLink->pprev;
      while (1) {
         OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);
         OSMemLink* pNextMemLink = pMemLink->pnext;

         if (pMemBlk->nsaved == 0) {
            if ((pMemHeap->flags & RT_MH_DONTKEEPFREE) /*||
                (pMemHeap->keepFreeUnits > 0 &&
                 pMemHeap->freeUnits + pMemBlk->nunits - 2 > pMemHeap->keepFreeUnits)*/)
            {
               TRACEMEMBLK(pMemBlk, "Freed");

               if (pMemBlk->link.pnext == pMemBlk->link.pprev) {
                  /* last memory block */
                  pMemHeap->u.blockList.curMemBlk = 0;
                  pMemHeap->u.blockList.pMinAddress = 0;
                  pMemHeap->u.blockList.pMaxAddress = 0;
               }
               else {
                  /* remove from mem block list */
                  pMemBlk->link.pprev->pnext = pMemBlk->link.pnext;
                  pMemBlk->link.pnext->pprev = pMemBlk->link.pprev;

                  if (pMemHeap->u.blockList.curMemBlk == pMemLink)
                     pMemHeap->u.blockList.curMemBlk = pMemLink->pnext;
               }

               if (pMemHeap->flags & RT_MH_ZEROONFREE) {
                  _rtxMemHeapZeroMemBlk (pMemBlk);

                  RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapReset: "
                     "zeroed memory block %p\n", pMemBlk);
               }

               _g_free_func (pMemHeap, pMemBlk);
               RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapReset: "
                           "freed memory block %p\n", pMemBlk);
            }
            else {
               OSMemElemDescr* pElemBeg = GET_ELEM_N (pMemBlk, 2);
               OSMemElemDescr* pElemEnd = GET_ELEM_N (pMemBlk, pMemBlk->nunits);
               OSFreeElemLink* pFreeLink;
               OSFreeElemLink* pFirstFreeLink = GET_FREE_LINK_0 (pMemBlk);
               OSUINT16 nunits = (OSUINT16) (pMemBlk->nunits - 2);

               pMemBlk->maxFreeElem = nunits;

               SET_OSUINT16 (pElemBeg, OSMemElemDescr_nunits, nunits);
               SET_FREE (pElemBeg);

               if (pMemHeap->flags & RT_MH_ZEROONFREE) {
                  _rtxMemHeapZeroElem (pElemBeg);
               }

               SET_OSUINT16 (pElemEnd, OSMemElemDescr_prevOff, nunits);

               pFreeLink = GET_FREE_LINK (pElemBeg);

               if (0 == pFreeLink) {
                  rtxErrAssertionFailed ("pFreeLink != 0", __LINE__, __FILE__);
               }

               ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree);
               ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree);

               GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code,
                                    pMemBlk->firstFree);

               SET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_prevFree,
                             pMemBlk->firstFree);

               SET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree,
                             pMemBlk->firstFree);

               TRACEMEMBLK_FREELIST(pMemBlk, "post-block-reset");
            }
         }
         else {
            OSMemElemDescr* pElem = GET_ELEM_N (pMemBlk, 2);
            OSMemElemDescr* pElemBeg = 0;
            OSMemElemDescr* pElemEnd = GET_ELEM_N (pMemBlk, pMemBlk->nunits);
            OSFreeElemLink* pFreeLink = GET_FREE_LINK_0 (pMemBlk);
            OSFreeElemLink* pFirstFreeLink = pFreeLink;
            OSUINT16 nsaved = pMemBlk->nsaved;
            OSUINT16 maxFreeElem = 0;

            pMemBlk->firstFree = 0;

            while (pElem != pElemEnd) {
               if (ISSAVED (pElem)) {
                  /* ISSAVED is false for free elem */
                  OSUINT16 nunits;
                  OSFreeElemLink* p;

                  if (pElemBeg) {
                     OSUINT16 offset;
                     GET_OFFSET_OSUINT16 (pElem, pElemBeg, nunits);

                     if (nunits > maxFreeElem)
                        maxFreeElem = nunits;

                     SET_OSUINT16 (pElemBeg, OSMemElemDescr_nunits, nunits);
                     SET_FREE (pElemBeg);

                     if (pMemHeap->flags & RT_MH_ZEROONFREE) {
                        _rtxMemHeapZeroElem (pElemBeg);
                     }

                     SET_OSUINT16 (pElem, OSMemElemDescr_prevOff, nunits);

                     p = GET_FREE_LINK (pElemBeg);

                     if (0 == p) {
                        /* This should never happen, but it causes warnings in
                         * some code verifiers. */
                        pElem = GETNEXT(pElem);
                        continue;
                     }

                     SET_OSUINT16 (p, OSFreeElemLink_nextFree,
                                   pMemBlk->firstFree);

                     GET_OFFSET_OSUINT16 (p, &pMemBlk->code, offset);
                     SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, offset);

                     pMemBlk->firstFree = offset;
                     pFreeLink = p;
                  }

                  if (nsaved > 0 && --nsaved > 0)
                     pElemBeg = 0;
                  else {
                     OSUINT16 offset;

                     /* all saved elements are passed */
                     pElem = GETNEXT (pElem);

                     if (pElem == pElemEnd)
                        break; /* saved was last element */

                     pElemBeg = pElem;

                     GET_OFFSET_OSUINT16 (pElemEnd, pElemBeg, nunits);

                     if (nunits > maxFreeElem)
                        maxFreeElem = nunits;

                     SET_OSUINT16 (pElemBeg, OSMemElemDescr_nunits, nunits);
                     SET_FREE (pElemBeg);

                     if (pMemHeap->flags & RT_MH_ZEROONFREE) {
                        _rtxMemHeapZeroElem (pElemBeg);
                     }

                     SET_OSUINT16 (pElemEnd, OSMemElemDescr_prevOff, nunits);

                     p = GET_FREE_LINK (pElemBeg);

                     if (0 == p) {
                        /* This should never happen, but it causes warnings in
                         * some code verifiers. */
                        pElem = GETNEXT(pElem);
                        continue;
                     }

                     GET_OFFSET_OSUINT16 (p, &pMemBlk->code, offset);
                     SET_OSUINT16 (p, OSFreeElemLink_nextFree,
                                   pMemBlk->firstFree);
                     SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, offset);
                     pMemBlk->firstFree = offset;
                     pFreeLink = p;

                     break;
                  }
               }
               else if (pElemBeg == 0) {
                  pElemBeg = pElem;
               }

               pElem = GETNEXT (pElem);
            }

            ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree);
            SET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree,
                          pMemBlk->firstFree);

            pMemBlk->maxFreeElem = maxFreeElem;

            TRACEMEMBLK_FREELIST(pMemBlk, "post-block-reset");
            RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapReset: "
                        "reset memory block %p\n", pMemBlk);
         }

         if (pMemLink == pMemLinkEnd)
            break;
         else
            pMemLink = pNextMemLink;
      }
   }
#endif /* _MEMCOMPACT */

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapReset: end\n");
}
