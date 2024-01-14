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

#include <stdio.h>
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemHeap.hh"
#include "rtxsrc/rtxPrintStream.h"

#ifdef _MEMDEBUG
   static OSCTXT _g_dbg_ctxt;
   #define g_dbg_ctxt &_g_dbg_ctxt

   #ifdef WIN32
      #include <windows.h>
      #define DEBUG_BREAK do { DebugBreak (); abort (); } while (0)
   #else
      #define DEBUG_BREAK \
      rtxErrAssertionFailed("memory heap broken", 0, "rtxMemHeapDebug.c")
      #endif
#else /* _MEMDEBUG */
   #define DEBUG_BREAK \
   rtxErrAssertionFailed("memory heap broken", 0, "rtxMemHeapDebug.c")
   #define g_dbg_ctxt 0
#endif /* _MEMDEBUG */

void rtxMemSetDebugPrintStream (rtxPrintCallback myCallback, void* pStrmInfo)
{
#ifdef _MEMDEBUG
   rtxSetPrintStream (g_dbg_ctxt, myCallback, pStrmInfo);
#endif
}

void _rtxMemHeapPrint (const char* name) {
   rtxPrintToStream (g_dbg_ctxt, "%s\n", name);
}

void _rtxMemPrintRawMemBlk (OSRawMemBlk* pRawBlk, const char* name) {
   rtxPrintToStream(g_dbg_ctxt, "%s ", name);
#ifdef _MEMCOMPACT
   if (pRawBlk == 0)
      rtxPrintToStream (g_dbg_ctxt, "Mem block = NULL\n");
   else {
      OSMemElemDescr* pElem = (OSMemElemDescr*) &pRawBlk->elem;

      rtxPrintToStream (g_dbg_ctxt, "Mem block, ptr = %p, refCnt = %u %s\n",
               pRawBlk + 1, pElem_refcnt (pElem),
               (ISSAVED (pElem) ? ", saved" : ""));
   }
#else
   if (pRawBlk == 0)
      rtxPrintToStream (g_dbg_ctxt, "Raw mem block = NULL\n");
   else {
      OSMemElemDescr* pElem = (OSMemElemDescr*) &pRawBlk->code;

      rtxPrintToStream
         (g_dbg_ctxt, "Raw mem block, ptr = %p, size = %u, refCnt = %u %s\n",
          pRawBlk + 1, pRawBlk->memsize, pElem_refcnt (pElem),
          (ISSAVED (pElem) ? ", saved" : ""));
   }
#endif
}

#ifndef _MEMCOMPACT
void _rtxMemPrintMemBlk (OSMemBlk* pMemBlk, const char* name) {
   rtxPrintToStream(g_dbg_ctxt, "%s ", name);
   if (pMemBlk == 0) {
      rtxPrintToStream (g_dbg_ctxt, "pMemBlk = NULL\n");
      return;
   }

   rtxPrintToStream (g_dbg_ctxt, "Mem block, ptr = %p, size = %u, nsaved = %u, "
            "maxFreeElem = ",
            pMemBlk, (pMemBlk->nunits - 2) * 8u, pMemBlk->nsaved);

   if (pMemBlk->maxFreeElem == OSUINT16_MAX)
      rtxPrintToStream (g_dbg_ctxt, "?\n");
   else
      rtxPrintToStream (g_dbg_ctxt, "%u\n", (pMemBlk->maxFreeElem - 1) * 8u);
}

void _rtxMemPrintMemElem (OSMemBlk* pMemBlk, OSMemElemDescr* pElem,
   const char* elemName)
{
   OSUINT16 nunits;

   if (pElem == 0) {
      rtxPrintToStream (g_dbg_ctxt, "pMemBlk = %p: %s = NULL\n", pMemBlk, elemName);
      return;
   }

   GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

   if (ISFREE (pElem))
      rtxPrintToStream (g_dbg_ctxt, "%s = %p, data = %p, size = %i free\n",
               elemName, pElem, pElem_data (pElem), (nunits - 1) * 8u);
   else
      rtxPrintToStream (g_dbg_ctxt, "%s = %p, data = %p, size = %i, refCnt = %u%s\n",
               elemName, pElem, pElem_data (pElem), (nunits - 1) * 8u,
               pElem_refcnt (pElem),(ISSAVED (pElem) ? ", saved" : ""));
}

void _rtxMemCheckReport (OSMemBlk* pMemBlk, OSMemElemDescr* pElem,
                         const char* file, int line,
                         int err, const char* errStr)
{
   rtxPrintToStream (g_dbg_ctxt, "Memory report at %s, line %i\n", file, line);
   rtxPrintToStream (g_dbg_ctxt, "Error %d: %s.\n", err, errStr);
   _rtxMemPrintMemBlk (pMemBlk, "pMemBlk");

   if (pElem != 0) {
      _rtxMemPrintMemElem (pMemBlk, pElem, "pElem");
   }
   rtxPrintToStream (g_dbg_ctxt, "\n");
}
#endif /* _MEMCOMPACT */

void _rtxMemCheckReportRaw (OSRawMemBlk* pRawBlk, const char* file, int line,
   int err, const char* errStr)
{
   rtxPrintToStream (g_dbg_ctxt, "Memory report at %s, line %i\n", file, line);
   rtxPrintToStream (g_dbg_ctxt, "Error %d: %s.\n", err, errStr);
   _rtxMemPrintRawMemBlk (pRawBlk, "pRawBlk");
   rtxPrintToStream (g_dbg_ctxt, "\n");
}

void rtxMemHeapCheck (void **ppvMemHeap, const char* file, int line)
{
   OSMemHeap* pMemHeap;
   OSMemLink* pMemLink;
   OSMemLink* pMemLinkEnd;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

   /* raw blocks */
   pMemLink = pMemHeap->u.blockList.rawBlkList.pnext;
   pMemLinkEnd = &pMemHeap->u.blockList.rawBlkList;

   while (pMemLink != pMemLinkEnd) {
      OSRawMemBlk* pRawBlk = TO_RAWMEMBLK (pMemLink);
#ifdef _MEMCOMPACT
      OSMemElemDescr* pElem = (OSMemElemDescr*) &pRawBlk->elem;
#else
      OSMemElemDescr* pElem = (OSMemElemDescr*) &pRawBlk->code;
#endif

      if (pMemLink == 0 || pMemLink->pnext == 0 ||
          pMemLink->pnext == pMemLink ||
          pMemLink->pnext->pprev != pMemLink)
      {
         _rtxMemCheckReportRaw (pRawBlk, file, line,
                                1, "invalid MemLink");
         DEBUG_BREAK;
      }

#ifndef _MEMCOMPACT
      if (!ISRAW (pElem)) {
         _rtxMemCheckReportRaw (pRawBlk, file, line,
                                2, "absent raw block type flag");
         DEBUG_BREAK;
      }
#endif

      if (!rtxMemHeapCheckCode (pElem)) {
         _rtxMemCheckReportRaw (pRawBlk, file, line,
                                3, "invalid code guard value");
         DEBUG_BREAK;
      }

      pMemLink = pMemLink->pnext;
   }

#ifndef _MEMCOMPACT
   /* memory blocks */
   pMemLink = pMemHeap->u.blockList.curMemBlk;
   pMemLinkEnd = pMemLink;

   if (pMemLink) {
      do {
         OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);
         OSFreeElemLink* pFreeLink;
         OSFreeElemLink* pNextFreeLink;
         OSFreeElemLink* pFreeLinkEnd;
         OSMemElemDescr* pElem;
         OSMemElemDescr* pElemEnd;
         OSUINT32 nsaved = 0;
         OSUINT32 nfree = 0;
         OSUINT32 nfreeInList = 0;
         OSUINT32 maxFreeElem = 0;
         OSUINT16 nunits, u16;

         if (pMemLink->pnext == 0 ||
             pMemLink->pnext->pprev != pMemLink)
         {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                4, "invalid MemLink");
            DEBUG_BREAK;
         }

         if (pMemBlk->code != OSMEMLINKCODE) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                5, "invalid code guard value");
            DEBUG_BREAK;
         }

         if (pMemBlk->nunits <= 2) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                6, "invalid mem block size");
            DEBUG_BREAK;
         }

         pElem = GET_ELEM_N (pMemBlk, 1);
         GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);
         if (nunits != 0 || ISFREE (pElem)) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                7, "invalid start border elem");
            DEBUG_BREAK;
         }

         pElem = GET_ELEM_N (pMemBlk, 2);
         pElemEnd = GET_ELEM_N (pMemBlk, pMemBlk->nunits);

         if (ISFREE (pElemEnd)) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                8, "invalid end border elem");
            DEBUG_BREAK;
         }

         while (pElem < pElemEnd) {
            OSMemElemDescr* pNextElem = GETNEXT (pElem);

            if (ISRAW (pElem)) {
               _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                   9, "raw flag on");
               DEBUG_BREAK;
            }

            GET_OSUINT16 (pElem, OSMemElemDescr_nunits, nunits);

            if (nunits < 2) {
               _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                   10, "invalid mem elem size");
               DEBUG_BREAK;
            }

            GET_OSUINT16 (pNextElem, OSMemElemDescr_prevOff, u16);

            if (nunits != u16) {
               _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                   11, "pElem->nunits != pNextElem->prevOff");
               DEBUG_BREAK;
            }

            if (ISFREE (pElem)) {
               nfree++;

               if (ISFREE (pNextElem)) {
                  _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                      12, "free elems is not joined");
                  DEBUG_BREAK;
               }

               if (nunits > maxFreeElem)
                  maxFreeElem = nunits;
            }
            else {
               OSUINT16 offset;
               GET_OSUINT16 (pElem, OSMemElemDescr_beginOff, u16);
               GET_OFFSET_OSUINT16 (pElem, &pMemBlk->code, offset);
               if (u16 != offset) {
                  _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                      13, "invalid beginOff field");
                  DEBUG_BREAK;
               }

               if (ISSAVED (pElem))
                  nsaved++;
            }

            pElem = pNextElem;
         }

         if (pMemBlk->nsaved != nsaved) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                14, "invalid nsaved field");
            DEBUG_BREAK;
         }

         if (pMemBlk->maxFreeElem != OSUINT16_MAX &&
             pMemBlk->maxFreeElem < maxFreeElem)
         {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                15, "invalid maxFreeElem field");
            DEBUG_BREAK;
         }

         pFreeLink = GET_FIRST_FREE (pMemBlk);
         pFreeLinkEnd = pFreeLink;

         do {
            OSUINT16 offset;

            if ( pFreeLink != &pMemBlk->code ) {
               pElem = GET_FREE_ELEM (pFreeLink);

               if (0 == pElem) {
                  _rtxMemCheckReport (pMemBlk, 0, file, line, 15,
                          "invalid free element");
                  DEBUG_BREAK;
               }
            }
            /* else: fake free link w/o an associated memory element */

            pNextFreeLink = GET_NEXT_FREE (pMemBlk, pFreeLink);

            if (nfree < nfreeInList++) {
               _rtxMemCheckReport (pMemBlk, 0, file, line,
                                   16, "broken free list");
               DEBUG_BREAK;
            }

            GET_OSUINT16 (pFreeLink, OSFreeElemLink_prevOff, u16);
            if (u16 > 0 && !ISFREE (pElem)) {
               _rtxMemCheckReport (pMemBlk, pElem, file, line,
                                   17, "free list contains allocated elem");
               DEBUG_BREAK;
            }

            GET_OSUINT16 (pNextFreeLink, OSFreeElemLink_prevFree, u16);
            GET_OFFSET_OSUINT16 (pFreeLink, &pMemBlk->code, offset);
            if (u16 != offset) {
               _rtxMemCheckReport (pMemBlk, 0, file, line,
                                   18, "invalid prevFree/nextFree");
               DEBUG_BREAK;
            }

            pFreeLink = pNextFreeLink;
         } while (pFreeLink != pFreeLinkEnd);

         if (nfree != nfreeInList - 1) {
            _rtxMemCheckReport (pMemBlk, 0, file, line,
                                19, "free list contains not all free elems");
            DEBUG_BREAK;
         }

         pMemLink = pMemLink->pnext;
      } while (pMemLink != pMemLinkEnd);
   }
#endif
}

void rtxMemHeapPrint (void **ppvMemHeap)
{
   OSMemHeap* pMemHeap;
   OSMemLink* pMemLink;
   OSMemLink* pMemLinkEnd;

   if (ppvMemHeap == 0 || *ppvMemHeap == 0)
      return;

   pMemHeap = (OSMemHeap*) *ppvMemHeap;

   /* raw blocks */
   pMemLink = pMemHeap->u.blockList.rawBlkList.pnext;
   pMemLinkEnd = &pMemHeap->u.blockList.rawBlkList;

   while (pMemLink != pMemLinkEnd) {
      OSRawMemBlk* pRawBlk = TO_RAWMEMBLK (pMemLink);
      _rtxMemPrintRawMemBlk (pRawBlk, "pRawBlk");
      pMemLink = pMemLink->pnext;
   }

   if (pMemHeap->u.blockList.rawBlkList.pnext !=
       &pMemHeap->u.blockList.rawBlkList)
      rtxPrintToStream (g_dbg_ctxt, "\n");

#ifndef _MEMCOMPACT
   /* memory blocks */
   pMemLink = pMemHeap->u.blockList.curMemBlk;
   pMemLinkEnd = pMemLink;

   if (pMemLink) {
      do {
         OSMemBlk* pMemBlk = TO_MEMBLK (pMemLink);
         OSMemElemDescr* pElem;
         OSMemElemDescr* pElemEnd;

         _rtxMemPrintMemBlk (pMemBlk, "pMemBlk");

         pElem = GET_ELEM_N (pMemBlk, 2);
         pElemEnd = GET_ELEM_N (pMemBlk, pMemBlk->nunits);

         while (pElem != pElemEnd) {
            _rtxMemPrintMemElem (pMemBlk, pElem, "  pElem");
            pElem = GETNEXT (pElem);
         }

         rtxPrintToStream (g_dbg_ctxt, "\n");
         pMemLink = pMemLink->pnext;
      } while (pMemLink != pMemLinkEnd);
   }
#endif
   rtxPrintToStream (g_dbg_ctxt, "\n");
}
