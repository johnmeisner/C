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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemHeap.hh"

EXTRTMETHOD void* rtxMemHeapAutoPtrRef (void **ppvMemHeap, void* mem_p)
{
#ifdef _MEMDEBUG
   OSMemHeap* pMemHeap;
#endif
   OSMemElemDescr* pElem;
   OSUINT8 cnt;

   if (mem_p == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0) return 0;

#ifdef _MEMDEBUG
   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrRef: start\n");

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

#ifdef _MEMDEBUG
   if (!rtxMemHeapCheckCode (pMemHeap, pElem)) {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return 0;
   }
#endif
   cnt = pElem_refcnt (pElem);

#ifdef _MEMCOMPACT
   if (cnt == 0) {
#else
   if (cnt == 0 || ISFREE (pElem)) {
#endif
      RTMEMDIAG2 (pMemHeap, "~E rtxMemHeapAutoPtrRef(%p): "
                  "element already freed!\n", mem_p);
      mem_p = 0;
   }
   else if (cnt == OSUINT8_MAX) {
      RTMEMDIAG2 (pMemHeap, "~E rtxMemHeapAutoPtrRef(%p): "
                  "element has maximum refcnt\n", mem_p);
      mem_p = 0;
   }
   else {
      pElem_refcnt (pElem) = (OSUINT8) (cnt + 1);

      RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAutoPtrRef(%p): refcnt "
                  "incremented to %d\n", mem_p, cnt + 1);
   }

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrRef: end\n");
   return mem_p;
}

EXTRTMETHOD int rtxMemHeapAutoPtrGetRefCount (void **ppvMemHeap, void* mem_p)
{
#ifdef _MEMDEBUG
   OSMemHeap* pMemHeap;
#endif
   OSMemElemDescr* pElem;

   if (mem_p == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0)
      return RTERR_NULLPTR;

#ifdef _MEMDEBUG
   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrGetRefCount: start\n");

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

#ifdef _MEMDEBUG
   if (!rtxMemHeapCheckCode (pMemHeap, pElem)) {
      rtxMemHeapInvalidPtrHook (ppvMemHeap, mem_p);
      return RTERR_NOMEM;
   }
#endif
   RTMEMDIAG3 (pMemHeap, "~I rtxMemHeapAutoPtrGetRefCount (%p): "
               "refcnt  = %d\n", mem_p, (int) pElem_refcnt (pElem));

   RTMEMDIAGDBG1 (pMemHeap, "~D rtxMemHeapAutoPtrGetRefCount: end\n");

   return (int) pElem_refcnt (pElem);
}
