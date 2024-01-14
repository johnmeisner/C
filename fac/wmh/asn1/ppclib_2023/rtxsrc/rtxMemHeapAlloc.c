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

EXTRTMETHOD void* rtxMemHeapSysAlloc (void** ppvMemHeap, OSSIZE nbytes)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0)
      return 0;

   if (*ppvMemHeap == 0)
      if (rtxMemHeapCreate (ppvMemHeap) != 0)
         return 0;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

   return _g_malloc_func (pMemHeap, nbytes);
}

#ifndef _MEMCOMPACT
/* Encoded copyright string. */
#define copyright __s25x
const char copyright[] = {
   '\x00', '\xC1', '\x3B', '\xD6', '\xC6', '\xAF', '\xF6', '\x57',
   '\xAC', '\xB3', '\x4E', '\x3D', '\xC7', '\xBB', '\xF7', '\x5A',
   '\xEA', '\xF6', '\x45', '\x24', '\x99', '\xB3', '\xEA', '\x60',
   '\xA6', '\xE9', '\x5E', '\x12', '\xE9', '\xE3', '\x54', '\x22',
   '\xAF', '\xDA', '\x0D', '\x8B', '\x90', '\xF2', '\xCB', '\x2E',
   '\xED', '\xD5', '\x56', '\x03', '\xB1', '\xCA', '\x27', '\xCE',
   '\xA4', '\xC1', '\x69', '\xC1', '\xDE', '\x3D', '\xE9', '\x5E',
   '\xA7', '\xE7', '\x05', '\x73', '\x08', '\xED', '\x65', '\x22',
   '\x5D'
};

static void* getNextFreePtr (void* pMemBlk, OSFreeElemLink* pFreeLink)
{
   OSOCTET* pMemBlkCode = (OSOCTET*)pMemBlk + OSMemBlk_code;
   OSUINT16 nextFreeOffset;
   GET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, nextFreeOffset);

   return ((void*)(pMemBlkCode + (nextFreeOffset * 8u)));
}

static void* getPrevFreePtr (void* pMemBlk, OSFreeElemLink* pFreeLink)
{
   OSOCTET* pMemBlkCode = (OSOCTET*)pMemBlk + OSMemBlk_code;
   OSUINT16 prevFreeOffset;
   GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, prevFreeOffset);
   return ((void*)(pMemBlkCode + (prevFreeOffset * 8u)));
}

EXTRTMETHOD void* rtxMemHeapAlloc (void** ppvMemHeap, OSSIZE nbytes)
{
   OSMemHeap* pMemHeap;
   void* mem_p = 0;
   OSMemElemDescr* pElem;
   OSSIZE nunits;

   if (ppvMemHeap == 0)
      return 0;

   if (*ppvMemHeap == 0)
      if (rtxMemHeapCreate (ppvMemHeap) != 0)
         return 0;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

   if (pMemHeap->flags & RT_MH_STATIC) {
      OSMemHeapStatic* pStaticHeap = &pMemHeap->u.staticMem;
      if (pStaticHeap->freeIdx + nbytes <= pStaticHeap->dataSize) {
         void* ptr = (void*)&pStaticHeap->data[pStaticHeap->freeIdx];
         pStaticHeap->freeIdx += nbytes;
         return ptr;
      }
      else return 0;
   }

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAlloc: start\n");

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);

   /* Round number of bytes to nearest 8-byte boundary */
   nunits = nbytes >> 3;
   if ( nbytes % 8 > 0 ) nunits++;

   /* A non-raw block can handle a request for at most 2^16 - 4 units, since
      OSMemBlk->nunits is unsigned 16-bit int and we need 2 units for border
      elements and 1 unit for the element header.
      Requests for more than this many units are allocated as a RAW block.
   */
   if ((pMemHeap->flags & RT_MH_USEATOMBLOCK) || (nunits > (1UL<<16) - 4)) {
      OSRawMemBlk* pRawBlk;
      OSSIZE blockSize = nbytes + sizeof (OSRawMemBlk);

      if ( blockSize < nbytes ) {
         /* integer overflow; we can't allocate the requested amount of memory
         */
         pRawBlk = NULL;
      }
      else {
         pRawBlk = (OSRawMemBlk*) _g_malloc_func (pMemHeap, blockSize);
      }

      if (pRawBlk == NULL) {
         /* The following code will never be executed, it is
          * necessary only to prevent the dropping out the copyright
          * string by the linker */
         if (copyright [0] != 0) {
            mem_p = (void*) copyright;
         }

         RTMEMDIAG1 (pMemHeap, "~E rtxMemHeapAlloc: failed\n");
         return mem_p;
      }
      else {

         if (pMemHeap->u.blockList.pMaxAddress == 0)
         {
            /* The min address has never been set. */
            pMemHeap->u.blockList.pMinAddress = pRawBlk;
         }
         else if (pMemHeap->u.blockList.pMinAddress > (void*)pRawBlk)
         {
            pMemHeap->u.blockList.pMinAddress = pRawBlk;
         }

         if (pMemHeap->u.blockList.pMaxAddress <
            (void*)((OSOCTET*)pRawBlk + blockSize - 1))
         {
            pMemHeap->u.blockList.pMaxAddress =
               (void*)((OSOCTET*)pRawBlk + blockSize - 1);
         }

         /* fill raw mem block */
         mem_p = pRawBlk + 1;
         pElem = GET_ELEM (mem_p);
         ZERO_OSUINT16 (pElem, OSMemElemDescr_beginOff);
         SET_RAW (pElem);
         pElem_refcnt (pElem) = 1;

         pRawBlk->memsize = nbytes;
         pRawBlk->code = pMemHeap->code;

         /* insert to raw list */
         (pRawBlk->link.pprev =
          pMemHeap->u.blockList.rawBlkList.pprev)->pnext = &(pRawBlk->link);

         (pRawBlk->link.pnext =
          &(pMemHeap->u.blockList.rawBlkList))->pprev = &(pRawBlk->link);

         TRACERAWMEMBLK(pRawBlk, "Allocated");
      }
   }
   else {
      /* assert: 0 <= nunits <= 2^16 - 4 */

      /* find memory element in memory blocks  The list is circular; we stop
         when we come back to our starting point. */
      OSMemLink* pMemLink = pMemHeap->u.blockList.curMemBlk;
      OSMemLink* pMemLinkEnd = pMemLink; /* starting point; where to stop */

      OSUINT16 nunits16 = (OSUINT16) nunits;    /* cast will not truncate */

      if (nunits16 == 0)
         nunits16 = 2;  /* nbytes == 0*/
      else
         nunits16++; /* +1 unit for memory element record */

      /* assert: 2 <= nunits16 <= 2^16 - 3 */

      if (0 != pMemLink) {
         /* find free memory element */
         do {
            OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);

            /* Do we have enough space in this block for the request?  If so,
             * allocate and go. */
            if (pMemBlk->maxFreeElem >= nunits16) {
               OSFreeElemLink* pFreeLink;
               OSFreeElemLink* pFreeLinkEnd;
               OSUINT16 maxFreeElem = 0;

               /* The first free element points at
                *
                *    &pMemBlk+(pMemBlk->nunits+2)*8
                *
                * This tells us the extent of the entire block. */
               pFreeLinkEnd = pFreeLink = GET_FIRST_FREE (pMemBlk);

               do {
                  OSUINT16 sz;
                  OSFreeElemLink* pNextFreeLink;

                  /* We use this to find out how many 8-byte units live
                   * between the free link--which follows this block--and its
                   * previous offset.
                   *
                   * If this is the first allocation, this should point at
                   * the memory block code. */
                  GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff, sz);

                  pNextFreeLink = getNextFreePtr (pMemBlk, pFreeLink);

                  /* If this link has enough room to handle the request,
                   * fulfill it. Otherwise we'll try the next link. */
                  if (sz >= nunits16) {
                     OSUINT16 szNext, diffSz, offset;

                     /* To avoid excess fragmentation, we'll look at the size
                      * of the next link.  If we haven't allocated anything,
                      * the size will be zero. */
                     GET_OSUINT16
                        (pNextFreeLink, OSFreeElemLink_prevOff, szNext);

                     pMemBlk->maxFreeElem = OSUINT16_MAX; /* unknown */

                     /* Use the smaller block in preference to the larger as
                      * long as it can hold the space; this helps avoid
                      * fragmentation. */
                     if (szNext >= nunits16 && szNext < sz) {
                        /* use second free elem */

                        if (pFreeLink != pFreeLinkEnd) {
                           /* set first free elem */
                           GET_OFFSET_OSUINT16
                              (pFreeLink, &pMemBlk->code, pMemBlk->firstFree);
                        }

                        sz = szNext;
                        pFreeLink = pNextFreeLink;
                     }

                     diffSz = sz - nunits16;
                     pElem = GET_FREE_ELEM (pFreeLink);

                     if (0 == pElem) {
                        RTMEMDIAG1 (pMemHeap, "~E rtxMemHeapAlloc: failed; "
                              "heap reports that we have a free element, but "
                              "it is null\n");
                        return 0;
                     }

                     /* If we have enough room to continue using the element,
                      * chunk it by adding in the borders: we fill in the
                      * number of 8-byte units used by the current element,
                      * get the next one, and set its offsets and extent. */
                     if (diffSz >= 2) {
                        /* split free element */
                        OSMemElemDescr* pFreeElem;

                        SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits16);

                        pFreeElem = GETNEXT (pElem);
                        SET_OSUINT16
                           (pFreeElem, OSMemElemDescr_prevOff, nunits16);

                        SET_OSUINT16
                           (pFreeElem, OSMemElemDescr_nunits, diffSz);

                        SET_OSUINT16
                           (GETNEXT(pFreeElem), OSMemElemDescr_prevOff, diffSz);

                        SET_FREE (pFreeElem);
                        /* free link is saved */
                     }

                     /* Otherwise, we remove the element from the free list by
                      * finding the next and previous offsets from the current
                      * link and updating its neighbors to point at them. */
                     else {
                        /* remove from free list */
                        OSUINT16 nextFree, prevFree;

                        GET_OSUINT16 (pFreeLink,
                                      OSFreeElemLink_nextFree, nextFree);

                        GET_OSUINT16 (pFreeLink,
                                      OSFreeElemLink_prevFree, prevFree);

                        if (pFreeLink == pFreeLinkEnd) {
                           /* first free element allocated */
                           pMemBlk->firstFree = nextFree;
                        }

                        SET_OSUINT16 (getPrevFreePtr (pMemBlk, pFreeLink),
                                      OSFreeElemLink_nextFree, nextFree);

                        SET_OSUINT16 (getNextFreePtr (pMemBlk, pFreeLink),
                                      OSFreeElemLink_prevFree, prevFree);
                     }

                     /* We then update the current element so that its header
                      * accurately reflects how far away it is from the memory
                      * block code.  We null out the flags to indicate it's
                      * been allocated and set the reference count to one to
                      * ensure we don't accidentally free it. */
                     GET_OFFSET_OSUINT16 (pElem, &pMemBlk->code, offset);
                     SET_OSUINT16 (pElem, OSMemElemDescr_beginOff, offset);
                     pElem_flags (pElem) = 0; /* allocated */
                     pElem_refcnt (pElem) = 1;

                     /* This is the actual content of the data. */
                     mem_p = pElem_data (pElem);

                     /* And now we update the current memory block to point at
                      * this link so we can properly update the next and
                      * previous pointers in subsequent allocations. */
                     pMemHeap->u.blockList.curMemBlk = &pMemBlk->link;

                     TRACEMEMELEM(pMemBlk, pElem, "Allocated");
                     TRACEMEMBLK_FREELIST(pMemBlk, "post-elem-alloc");
                     RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAlloc: mem_p = %p, "
                                 "nbytes = %d\n", mem_p, nbytes);
                     RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAlloc: end\n");
                     return mem_p;
                  }

                  if (sz > maxFreeElem)
                     maxFreeElem = sz;

                  pFreeLink = pNextFreeLink;
               } while (pFreeLink != pFreeLinkEnd);

               pMemBlk->maxFreeElem = maxFreeElem;
            }

            /* else: cannot allocate from this memory block; try another */
            pMemLink = pMemLink->pnext;
         } while (pMemLink != pMemLinkEnd);
      }

      /* Could not allocate from any existing memory block.
         Create new memBlk.  */
      {
         OSMemBlk* pMemBlk;
         OSFreeElemLink* pFreeLink;
         OSUINT16 nextFree;
         OSUINT16 dataUnits;
         OSSIZE defBlkSize = pMemHeap->defBlkSize;

         /* The minimum bytes to allocate accounts for the units needed for
            the request (nunits16), +2 for two border elements, plus the size
            of the memory block structure.
            The maximum bytes to allocate is based on the maximum data units
            supported plus the size of the memory block structure.
            Since nunits16 + 2 <= 2^16 - 1, minAllocSize <= maxAllocSize.
         */
         OSSIZE minAllocSize = (nunits16 + 2) * 8u + sizeof (OSMemBlk);
         OSSIZE maxAllocSize = ((1ul<<16) - 1)* 8u + sizeof(OSMemBlk);

         /* Allocate a mulitiple of the default block size, not to exceed
            maxAllocSize.
         */
         OSSIZE allocSize = minAllocSize < defBlkSize ? defBlkSize :
            ((minAllocSize + defBlkSize - 1) / defBlkSize * defBlkSize);

         if ( allocSize > maxAllocSize ) allocSize = maxAllocSize;

         /* Set dataUnits to the total # of units for border and non-border
            elements; more units than necessary may have been allocated due to
            the block size.
            We know:
               (nunits16 + 2) * 8 + sizeof(OSMemBlk) <= allocSize and
               allocSize <= (2^16 - 1) * 8 + sizeof(OSMemBlk).
            Therefore:
               nunits16 + 2 <= dataUnits and
               dataUnits <= 2^16 - 1.
         */
         dataUnits = (OSUINT16) ((allocSize - sizeof(OSMemBlk) ) / 8);

         pMemBlk = (OSMemBlk*) _g_malloc_func (pMemHeap, allocSize);

         if (0 == pMemBlk) {
            RTMEMDIAG1 (pMemHeap, "~E rtxMemHeapAlloc: failed\n");
            return 0;  /* no memory */
         }

         RTMEMDIAGDBG2 (pMemHeap, "~D rtxMemHeapAlloc: allocated new memory "
                        "block %p\n", pMemBlk);

         /* maintain the doubly-linked circular list of OSMemBlks */
         if (0 == pMemHeap->u.blockList.curMemBlk) {
            pMemBlk->link.pnext = pMemBlk->link.pprev = &pMemBlk->link;
         }
         else {
            (pMemBlk->link.pprev =
             pMemHeap->u.blockList.curMemBlk->pprev)->pnext = &pMemBlk->link;

            (pMemBlk->link.pnext =
             pMemHeap->u.blockList.curMemBlk)->pprev = &pMemBlk->link;
         }

         pMemHeap->u.blockList.curMemBlk = &pMemBlk->link;
         if ( pMemHeap->u.blockList.pMaxAddress == 0 )
         {
            /* The min address has never been set. */
            pMemHeap->u.blockList.pMinAddress = pMemBlk;
         }
         else if ( pMemHeap->u.blockList.pMinAddress > (void*) pMemBlk )
         {
            pMemHeap->u.blockList.pMinAddress = pMemBlk;
         }

         if ( pMemHeap->u.blockList.pMaxAddress <
                              (void*) ((OSOCTET*) pMemBlk + allocSize - 1 ))
         {
            pMemHeap->u.blockList.pMaxAddress =
                              (void*) ((OSOCTET*) pMemBlk + allocSize - 1 );
         }

         pMemBlk->nunits = dataUnits;

         /* maxFreeElem = # units beyond what is needed for the requested
            allocation. It may be > 0 due to the block size resulting in
            a larger allocation than strictly required.
            We know from above that dataUnits - nunits16 - 2 >= 0.
         */
         pMemBlk->maxFreeElem = dataUnits - nunits16 - 2;

         if (pMemBlk->maxFreeElem == 1) {
            /* A free element of 1 unit is of no use (it would be all header).
               Use the entire space for the element we're allocating to the
               caller.
            */
            nunits16++;
            pMemBlk->maxFreeElem = 0;
         }

         if ( pMemBlk->maxFreeElem == 0 )
         {
            /* There will be no free element */
            nextFree = 0;
         }
         else {
            /* There will be a free element, it will follow the allocated
               element and will be at the end of the block.
               pMemBlk->nunits - 1 == dataUnits - 1 is the offset for the free
               element's link location.
            */
            nextFree = dataUnits - 1;
         }

         pMemBlk->firstFree = nextFree;
         pMemBlk->nsaved = 0;
         pMemBlk->code = pMemHeap->code;

         TRACEMEMBLK(pMemBlk, "Allocated");

         /* start border */
         /* n.b., GET_ELEM_N is relative to the code point; it works in units
            of 8 bytes */
         pElem = GET_ELEM_N (pMemBlk, 1);
         pElem_flags (pElem) = 0; /* allocated */
         ZERO_OSUINT16 (pElem, OSMemElemDescr_nunits);

         /* Set the nextFree and prevFree that are stored in the OSMemBlk
            itself.  Set the prevOff for the first memory element to 0 (there
            is no previous memory element).
         */
         /* n.b., this link points directly at the memory block code */
         pFreeLink = GET_FREE_LINK_0 (pMemBlk);
         SET_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree, nextFree);
         SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree, nextFree);
         ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff);

         /* allocated memory element */
         pElem = GET_ELEM_N (pMemBlk, 2);
         SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits16);
         SET_OSUINT16L (pElem, OSMemElemDescr_prevOff, 1);
         pElem_flags (pElem) = 0; /* allocated */
         pElem_refcnt (pElem) = 1;
         SET_OSUINT16L (pElem, OSMemElemDescr_beginOff, 2);
         mem_p = pElem_data (pElem);

         TRACEMEMELEM(pMemBlk, pElem, "Allocated");

         /* After allocating the element from the new block, we update the
          * memory descriptor of the next element to find the previous
          * element. */

         pElem = GETNEXT (pElem);
         SET_OSUINT16 (pElem, OSMemElemDescr_prevOff, nunits16);

         if (pMemBlk->maxFreeElem > 0) {
            /* setup the free element */
            nunits16 = pMemBlk->maxFreeElem;
            SET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits16);
            SET_FREE (pElem);

            /* GET_FREE_LINK returns a pointer to the last unit associated
            with pElem, which holds the OSFreeElemLink structure. It will return
            0 if and only if pElem's nunits is zero, which it clearly is not */
            pFreeLink = GET_FREE_LINK (pElem);

            OSRTASSERT(pFreeLink != 0);

            /*  This is the sole free element. */
            ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_nextFree);
            ZERO_OSUINT16 (pFreeLink, OSFreeElemLink_prevFree);

            /*  Assign prevOff in the next memory element. */
            SET_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff, nunits16);
         }
         else {
            /* No free element. pElem is the border element. Its size is set
               to zero even though it uses one unit for the header.
            */
            ZERO_OSUINT16 (pElem, OSMemElemDescr_nunits);
         }

         /* ending border element */
         pElem = GETNEXT (pElem);
         pElem_flags (pElem) = 0; /* allocated */

         TRACEMEMBLK_FREELIST(pMemBlk, "post-elem-allocated");
      }
   }

   RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAlloc: mem_p = %p, nbytes = %d\n",
               mem_p, nbytes);

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAlloc: end\n");

   return mem_p;
}

#else /* _MEMCOMPACT */

EXTRTMETHOD void* rtxMemHeapAlloc (void** ppvMemHeap, OSSIZE nbytes)
{
   void* mem_p = 0;
   OSMemHeap* pMemHeap;
   OSRawMemBlk* pRawBlk;
   OSMemElemDescr* pElem;
   OSSIZE blockSize;

   if (ppvMemHeap == 0)
      return NULL;

   if (*ppvMemHeap == 0)
      if (rtxMemHeapCreate (ppvMemHeap) != 0)
         return NULL;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;
#ifdef _MEMDEBUG
   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAlloc: start\n");

   if (pMemHeap->flags & RT_MH_CHECKHEAP)
      rtxMemHeapCheck (ppvMemHeap, __FILE__, __LINE__);
#endif
   /* check for possible overflow */
   if (nbytes >= (OSSIZE_MAX - sizeof(OSRawMemBlk))) {
      return NULL;
   }
   blockSize = nbytes + sizeof (OSRawMemBlk);
   if ( blockSize > nbytes ) {
      pRawBlk = (OSRawMemBlk*) _g_malloc_func (pMemHeap, blockSize);
   }
   else {
      /* integer overflow; nbytes too large */
      pRawBlk = NULL;
   }

   if (pRawBlk == NULL) {
      RTMEMDIAG1 (pMemHeap, "~E rtxMemHeapAlloc: failed\n");
      return 0;
   }

   if ( pMemHeap->u.blockList.pMaxAddress == 0 )
   {
      /* The min address has never been set. */
      pMemHeap->u.blockList.pMinAddress = pRawBlk;
   }
   else if ( pMemHeap->u.blockList.pMinAddress > (void*) pRawBlk )
   {
      pMemHeap->u.blockList.pMinAddress = pRawBlk;
   }

   if ( pMemHeap->u.blockList.pMaxAddress <
                        (void*) ((OSOCTET*) pRawBlk + blockSize - 1 ))
   {
      pMemHeap->u.blockList.pMaxAddress =
                        (void*) ((OSOCTET*) pRawBlk + blockSize - 1 );
   }

   /* Advance sizeof(OSMemRawBlk) bytes; this gets us past the raw block
    * header. */
   mem_p = pRawBlk + 1;

   /* Get the element and set the flags, reference count, and the link
    * code. */
   pElem = GET_ELEM (mem_p);
   pElem_flags (pElem) = 0;
   pElem_code (pElem) = pMemHeap->code;
   pElem_refcnt (pElem) = 1;

   /* insert to raw list */
   (pRawBlk->link.pprev = pMemHeap->u.blockList.rawBlkList.pprev)->pnext =
      &(pRawBlk->link);
   (pRawBlk->link.pnext = &(pMemHeap->u.blockList.rawBlkList))->pprev =
      &(pRawBlk->link);

   RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAlloc: mem_p = %p, nbytes = %d\n",
               mem_p, nbytes);
   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAlloc: end\n");
   return mem_p;
}

#endif
