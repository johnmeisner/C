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

#include "rtxsrc/rtxMemHeap.hh"

EXTRTMETHOD void* rtxMemHeapSysRealloc
(void** ppvMemHeap, void* mem_p, size_t nbytes)
{
   OSMemHeap* pMemHeap;
   void* newMem_p = 0;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return 0;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   if (is_realloc_available(pMemHeap)) {
      newMem_p = _g_realloc_func (pMemHeap, mem_p, nbytes);
   }

   return newMem_p;
}

#ifndef _MEMCOMPACT

EXTRTMETHOD void* rtxMemHeapRealloc
(void** ppvMemHeap, void* mem_p, size_t nbytes)
{
   OSMemHeap* pMemHeap;
   OSMemElemDescr* pElem;
   void* newMem_p = 0;
   OSUINT8 cnt;

   /* if mem_p == NULL - do rtxMemAlloc */

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return 0;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: start\n");

   if (mem_p == 0)
      return rtxMemHeapAlloc (ppvMemHeap, nbytes);
   else if (nbytes == 0) {
      rtxMemHeapFreePtr (ppvMemHeap, mem_p);
      return 0;
   }

   if (pMemHeap->flags & RT_MH_STATIC) {
      return 0;
   }

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   if ((pMemHeap->flags & RT_MH_VALIDATEPTR) &&
       rtxMemHeapCheckPtr (ppvMemHeap, mem_p) == 0)
   {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return 0;
   }

   pElem = GET_ELEM (mem_p);

   if (!rtxMemHeapCheckCode (pElem)) {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return 0;
   }

   cnt = pElem_refcnt (pElem);

   if (cnt == 0 || ISFREE (pElem)) {
      RTMEMDIAG2(pMemHeap, "~E rtxMemHeapRealloc(%p): "
                  "element already freed!\n", mem_p);
      return 0;
   }
   else if (cnt > 1 || (nbytes >= 524272 && !ISRAW(pElem))) {
      /*  12-06-28 MEM:  See ASN-4629 */
      /* more than one pointer refer to this memory element, or else we have
       * a non-raw block being reallocated into a raw block; each unit of
       * of memory 8 bytes wide, and we use two units up front in managed
       * blocks.  so if we request more then 2**19-16 bytes of memory, we need
       * a raw block. */
      RTMEMDIAG2(pMemHeap, "~I rtxMemHeapRealloc(%p): allocate copy\n", mem_p);

      /*  if the reference count is > 1, reduce it */
      if (cnt > 1) pElem_refcnt (pElem) = (OSUINT8) (cnt - 1);

      /* allocate copy */
      newMem_p = rtxMemHeapAlloc (ppvMemHeap, nbytes);

      if (newMem_p) {
         size_t sz;
         OSUINT16 nunits;

         if (ISRAW (pElem)) {
            OSRawMemBlk* pRawMemBlk = GET_RAWMEMBLK (pElem);
            sz = pRawMemBlk->memsize;
         }
         else {
            GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);
            sz = (nunits - 1) * 8u;
         }
         if (sz > nbytes)
            sz = nbytes;

         /* Here we guarantee that sz and nbytes are at worst equal, so we
          * use nbytes. */
         if (sz > 0)
            OSCRTLSAFEMEMCPY (newMem_p, nbytes, mem_p, sz);

         if (ISSAVED (pElem)) {
            pElem = GET_ELEM (newMem_p);
            SET_SAVED (pElem);
            if (!ISRAW (pElem)) {
               OSMemBlk* pMemBlk = GET_MEMBLK (pElem);
               pMemBlk->nsaved++;
            }
         }

         if (cnt <= 1) {
            /* a managed block is being reallocated into a raw block and does
             * not need to be preserved */
            rtxMemHeapFreePtr (ppvMemHeap, mem_p);
         }
      }

      RTMEMDIAG4(pMemHeap, "~I rtxMemHeapRealloc: mem_p = %p, newMem_p = %p,"
                  " nbytes = %d\n", mem_p, newMem_p, nbytes);
      RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: end\n");
      return newMem_p;
   }

   RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapRealloc(%p): change element size\n",
               mem_p);

   if (ISRAW (pElem)) {
      OSRawMemBlk* pRawBlk = GET_RAWMEMBLK (pElem);
      OSMemLink* pnext = pRawBlk->link.pnext;
      OSMemLink* pprev = pRawBlk->link.pprev;
      size_t blockSize = nbytes + sizeof (OSRawMemBlk);

      TRACERAWMEMBLK (pRawBlk, "Reallocating");

      if (is_realloc_available(pMemHeap)) {
         RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapRealloc: realloc raw block %p, "
                     "nbytes = %d\n", mem_p, nbytes);

         newMem_p = _g_realloc_func (pMemHeap, pRawBlk, blockSize);

         TRACERAWMEMBLK (((OSRawMemBlk*) newMem_p), "Reallocated");

         if (newMem_p == 0)
            return 0;

         if (newMem_p != pRawBlk) {
            pRawBlk = (OSRawMemBlk*) newMem_p;
            pRawBlk->link.pnext = pnext;
            pRawBlk->link.pprev = pprev;
            pprev->pnext = pnext->pprev = &pRawBlk->link;
            newMem_p = pRawBlk + 1;
         }
         else
            newMem_p = mem_p;
      }
      else {
         /* use malloc/OSCRTLSAFEMEMCPY/free sequence instead of realloc */
         size_t sz = pRawBlk->memsize;

         newMem_p = rtxMemHeapAlloc (ppvMemHeap, nbytes);

         if (newMem_p) {
            if (sz > nbytes)
               sz = nbytes;

            /* Here we guarantee that sz and nbytes are at worst equal, so we
             * use nbytes. */
            if (sz > 0)
               OSCRTLSAFEMEMCPY (newMem_p, nbytes, mem_p, sz);

            TRACERAWMEMBLK(pRawBlk, "Freed");

            /* remove from raw block list */
            pRawBlk->link.pprev->pnext = pRawBlk->link.pnext;
            pRawBlk->link.pnext->pprev = pRawBlk->link.pprev;
#if 0
            if (pMemHeap->flags & RT_MH_ZEROONFREE) {
               OSCRTLMEMSET (pRawBlk, 0, sizeof(pRawBlk));

               RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapRealloc: zeroed raw "
                  "block %p\n", pRawBlk);
            }
#endif
            _g_free_func (pMemHeap, pRawBlk);
            RTMEMDIAG2 (pMemHeap, "~I rtxMemHeapRealloc: freed raw block %p\n",
                        pRawBlk);
         }
      }
   }
   else {
      OSMemBlk* pMemBlk = GET_MEMBLK (pElem);
      OSMemElemDescr* pNextElem = GETNEXT (pElem);
      OSMemElemDescr* p;
      OSFreeElemLink* pFreeLink;
      OSUINT16 nunits = (OSUINT16)((nbytes + 7) >> 3);
      OSUINT16 oldNunits;

      GET_OSUINT16 (pElem, OSMemElemDescr_nunits, oldNunits);

      RTMEMDIAG3 (pMemHeap,
         "~I rtxMemHeapRealloc: old # units = %d, new # units = %d\n",
         oldNunits, nunits);

      if (nunits == 0)
         nunits = 2;  /* nbytes == 0*/
      else
         nunits++; /* unit for memory element record */

      if (nunits == oldNunits) {
         RTMEMDIAG1(pMemHeap,
            "~I rtxMemHeapRealloc: no change in number of units requested\n");
         return mem_p;
      }
      else if (nunits < oldNunits) {
         /* shrinking */
         if (ISFREE (pNextElem)) {
            /* expand free element */
            OSUINT16 freeNunits;

            GET_OSUINT16 (pNextElem, OSMemElemDescr_nunits, freeNunits);
            SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

            pNextElem = GETNEXT (pElem);
            SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, nunits);
            nunits = freeNunits + (oldNunits - nunits);
            SET_OSUINT16 (pNextElem, OSMemElemDescr_nunits, nunits);
            SET_FREE (pNextElem);

            pNextElem = GETNEXT (pNextElem);
            SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, nunits);
         }
         else if (oldNunits - nunits >= 2) {
            /* add new free element */
            OSUINT16 freeOff = pMemBlk->firstFree;
            OSFreeElemLink* pFirstFreeLink = GET_FIRST_FREE (pMemBlk);

            SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

            pNextElem = GETNEXT (pElem);
            SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, nunits);
            nunits = oldNunits - nunits;
            SET_OSUINT16 (pNextElem, OSMemElemDescr_nunits, nunits);
            SET_FREE (pNextElem);

            pFreeLink = GET_FREE_LINK (pNextElem);
            if (0 != pFreeLink) {
               SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff, nunits);
               SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, freeOff);
               GET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree, freeOff);
               SET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, freeOff);
            }
            else {
               return 0;
            }

            GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, freeOff);

            SET_OSUINT16 (GET_NEXT_FREE (pMemBlk, pFirstFreeLink),
                          OSFreeElemLink_prevFree, freeOff);

            SET_OSUINT16 (pFirstFreeLink, OSFreeElemLink_nextFree, freeOff);
         }

         if (nunits > pMemBlk->maxFreeElem)
            pMemBlk->maxFreeElem = (OSUINT16) nunits;

         TRACEMEMELEM (pMemBlk, pElem, "Shrinked");
         RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapRealloc: shrinked mem_p = %p, "
                     "nbytes = %d\n", mem_p, nbytes);

         RTMEMDIAG1(pMemHeap, "~I rtxMemHeapRealloc: mem_p is");
         if (!ISRAW(mem_p)) {
            RTMEMDIAG1(pMemHeap, " not");
         }
         RTMEMDIAG1(pMemHeap, " raw\n");

         RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: end\n");
         return mem_p;
      }
      else {
         /* expanding */

         pMemBlk->maxFreeElem = OSUINT16_MAX; /* unknown */

         if (ISFREE (pNextElem)) {
            OSUINT16 freeNunits;
            GET_OSUINT16 (pNextElem, OSMemElemDescr_nunits, freeNunits);

            if (freeNunits + oldNunits > nunits) {
               /* shrink free element */
               freeNunits = freeNunits + oldNunits - nunits;

               if (freeNunits >= 2) {
                  SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

                  pNextElem = GETNEXT (pElem);
                  SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, nunits);
                  SET_OSUINT16 (pNextElem, OSMemElemDescr_nunits, freeNunits);
                  SET_FREE (pNextElem);

                  pNextElem = GETNEXT (pNextElem);
                  SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, freeNunits);
               }
               else {
                  /* use all free element */
                  OSUINT16 nextFree, offset, prevFree;
                  nunits += freeNunits;

                  pFreeLink = GET_FREE_LINK (pNextElem);

                  if (0 == pFreeLink) return 0;

                  GET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, nextFree);

                  GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, offset);
                  if (pMemBlk->firstFree == offset)
                     pMemBlk->firstFree = nextFree;

                  SET_OSUINT16 (GET_PREV_FREE (pMemBlk, pFreeLink),
                                OSFreeElemLink_nextFree, nextFree);

                  GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, prevFree);
                  SET_OSUINT16 (GET_NEXT_FREE (pMemBlk, pFreeLink),
                                OSFreeElemLink_prevFree, prevFree);

                  SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

                  pNextElem = GETNEXT (pElem);
                  SET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, nunits);
               }

               TRACEMEMELEM (pMemBlk, pElem, "Expanded");
               RTMEMDIAG3(pMemHeap, "~I rtxMemHeapRealloc: expanded "
                           "mem_p = %p, nbytes = %d\n", mem_p, nbytes);
               RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: end\n");
               return mem_p;
            }
         }

         /* alloc/copy/free */
         TRACEMEMELEM (pMemBlk, pElem, "Reallocating");

         newMem_p = rtxMemHeapAlloc (ppvMemHeap, nbytes);

         if (newMem_p == 0)
            return 0;

         /* The actual number of bytes that will be allocated on the heap is
          * nunits*8--so rather than using nbytes, we use the nunits figure
          * here. */
         OSCRTLSAFEMEMCPY (newMem_p, nunits*8u, mem_p, oldNunits * 8u);

         if (ISSAVED (pElem)) {
            p = GET_ELEM (newMem_p);
            SET_SAVED (p);

            if (!ISRAW (p)) {
               OSMemBlk* pMemBlkTemp = GET_MEMBLK (p);
               pMemBlkTemp->nsaved++;
            }
         }

         _rtxMemHeapFreeElem (ppvMemHeap, pElem);
      }
   }

   RTMEMDIAG4(pMemHeap, "~I rtxMemHeapRealloc: mem_p = %p, newMem_p = %p, "
               "nbytes = %d\n", mem_p, newMem_p, nbytes);
   RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: end\n");
   return newMem_p;
}

#else

EXTRTMETHOD void* rtxMemHeapRealloc
(void** ppvMemHeap, void* mem_p, size_t nbytes)
{
   OSMemHeap* pMemHeap;
   OSRawMemBlk* pRawBlk;
   OSRawMemBlk* pOldRawBlk;
   size_t blockSize;
   OSMemLink* pnext;
   OSMemLink* pprev;
   OSMemElemDescr* pElem;
   void* newMem_p = mem_p;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0) return 0;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1(pMemHeap, "~D rtxMemHeapRealloc: start\n");

   if (mem_p == 0)
      return rtxMemHeapAlloc (ppvMemHeap, nbytes);
   else if (nbytes == 0) {
      rtxMemHeapFreePtr (ppvMemHeap, mem_p);
      return 0;
   }

#ifdef _MEMDEBUG
   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   if ((pMemHeap->flags & RT_MH_VALIDATEPTR) &&
       rtxMemHeapCheckPtr (ppvMemHeap, mem_p) == 0)
   {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return 0;
   }
#endif
   pElem = GET_ELEM (mem_p);

   if (pElem_refcnt (pElem) != 1) {
      /* only once referenced element may be resized */
      RTMEMDIAGDBG1 (pMemHeap, "~E rtxMemHeapRealloc: "
                  "failed; more then one reference\n");
      return 0;
   }

   pOldRawBlk = GET_RAWMEMBLK (mem_p);
   pnext = pOldRawBlk->link.pnext;
   pprev = pOldRawBlk->link.pprev;

   TRACERAWMEMBLK(pOldRawBlk, "Reallocating");

   blockSize = nbytes + sizeof (OSRawMemBlk);
   pRawBlk = (OSRawMemBlk*) _g_realloc_func (pMemHeap, pOldRawBlk, blockSize);

   TRACERAWMEMBLK(pRawBlk, "Reallocated");

   if (pRawBlk == 0) {
      RTMEMDIAGDBG1 (pMemHeap, "~E rtxMemHeapRealloc: failed\n");
      return 0;
   }

   if (pRawBlk != pOldRawBlk) {
      newMem_p = pRawBlk + 1;
      pRawBlk->link.pnext = pnext;
      pRawBlk->link.pprev = pprev;
      pprev->pnext = pnext->pprev = &pRawBlk->link;
   }

   RTMEMDIAGDBG4 (pMemHeap, "~I rtxMemHeapRealloc: mem_p = %p, newMem_p = %p, "
               "nbytes = %d\n", mem_p, newMem_p, nbytes);
   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapRealloc: end\n");
   return newMem_p;
}

#endif
