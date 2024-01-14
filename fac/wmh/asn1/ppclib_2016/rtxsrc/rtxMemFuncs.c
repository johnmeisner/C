/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

/* Memory management utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxMemHeap.hh"
#include "rtxsrc/rtxError.h"

#if !defined(_ARMTCC) && !defined(__SYMBIAN32__)
static OSUINT32 g_defBlkSize = OSRTXM_K_MEMBLKSIZ;

EXTRTMETHOD void rtxMemSetDefBlkSize (OSUINT32 blkSize)
{
   g_defBlkSize = blkSize;
}
#else
#define g_defBlkSize 1024
#endif

EXTRTMETHOD OSUINT32 rtxMemGetDefBlkSize ()
{
   return g_defBlkSize;
}

EXTRTMETHOD void* rtxMemHeapAllocZ (void** ppvMemHeap, size_t nbytes)
{
   void* ptr = rtxMemHeapAlloc (ppvMemHeap, nbytes);
   if (0 != ptr) OSCRTLMEMSET (ptr, 0, nbytes);
   return ptr;
}

EXTRTMETHOD void* rtxMemHeapSysAllocZ (void** ppvMemHeap, size_t nbytes)
{
   void* ptr = rtxMemHeapSysAlloc (ppvMemHeap, nbytes);
   if (0 != ptr) OSCRTLMEMSET (ptr, 0, nbytes);
   return ptr;
}

EXTRTMETHOD OSUINT32 rtxMemHeapGetDefBlkSize (OSCTXT* pctxt)
{
#ifndef _MEMCOMPACT
   if (pctxt == 0 || pctxt->pMemHeap == 0) return g_defBlkSize;
   return ((OSMemHeap*)pctxt->pMemHeap)->defBlkSize;
#else
   return g_defBlkSize;
#endif
}

/* Determine if a block of memory is all zero */
EXTRTMETHOD OSBOOL rtxMemIsZero (const void* pmem, size_t memsiz)
{
   size_t i;
   const OSOCTET* pbytes = (const OSOCTET*) pmem;
   for (i = 0; i < memsiz; i++) {
      if (pbytes[i] != 0) return FALSE;
   }
   return TRUE;
}

EXTRTMETHOD void rtxMemHeapSetProperty
(void** ppvMemHeap, OSUINT32 propId, void* pProp)
{
   OSMemHeap* pMemHeap;

   if (pProp == 0 || ppvMemHeap == 0 || *ppvMemHeap == 0) return;

   pMemHeap = *(OSMemHeap**)ppvMemHeap;

   if (propId == OSRTMH_PROPID_SETFLAGS) {
      pMemHeap->flags |= *(OSUINT32*) pProp;
   }
   else if (propId == OSRTMH_PROPID_CLEARFLAGS) {
      pMemHeap->flags &= ~(*(OSUINT32*) pProp);
   }
#ifndef _MEMCOMPACT
   else if (propId == OSRTMH_PROPID_KEEPFREEUNITS) {
      pMemHeap->keepFreeUnits = *(OSUINT32*) pProp;
   }
   if (propId == OSRTMH_PROPID_DEFBLKSIZE) {
      pMemHeap->defBlkSize = *(OSUINT32*) pProp;
   }
#endif
}

EXTRTMETHOD OSBOOL rtxMemHeapIsEmpty (OSCTXT* pctxt)
{
   OSMemHeap* pMemHeap = (OSMemHeap*) pctxt->pMemHeap;
   if (0 == pMemHeap) return TRUE;
#ifndef _MEMCOMPACT
   if (0 != (pMemHeap->flags & RT_MH_STATIC)) {
      return (OSBOOL)(0 == pMemHeap->u.staticMem.freeIdx);
   }
   else {
      OSMemLink* pRawBlkList = &pMemHeap->u.blockList.rawBlkList;
      return (OSBOOL)
         (pRawBlkList->pnext == pRawBlkList->pprev &&
          pRawBlkList->pprev == pRawBlkList &&
          pMemHeap->u.blockList.curMemBlk == 0);
   }
#else
   return (OSBOOL)
      (pMemHeap->u.blockList.rawBlkList.pnext ==
       pMemHeap->u.blockList.rawBlkList.pprev &&
       pMemHeap->refCnt == 0);
#endif
}

EXTRTMETHOD void rtxMemFree (OSCTXT* pctxt)
{
   rtxErrReset(pctxt);
   rtxMemHeapFreeAll(&(pctxt)->pMemHeap);
}

EXTRTMETHOD void rtxMemReset (OSCTXT* pctxt)
{
   rtxErrReset(pctxt);
   rtxMemHeapReset(&(pctxt)->pMemHeap);
}

EXTRTMETHOD void* rtxMemAllocArray2
(OSCTXT* pctxt, OSSIZE numElements, OSSIZE typeSize, OSUINT32 flags)
{
   void* ptr;
   OSSIZE arraySize = numElements * typeSize;
   if (arraySize < numElements) { /* integer overflow */
      LOG_RTERR (pctxt, RTERR_TOOBIG);
      return 0;
   }

   if (0 != (flags & RT_MH_SYSALLOC)) {
      ptr = rtxMemHeapSysAlloc (&pctxt->pMemHeap, arraySize);
   }
   else {
      ptr = rtxMemHeapAlloc (&pctxt->pMemHeap, arraySize);
   }

   if (0 == ptr) LOG_RTERR (pctxt, RTERR_NOMEM);
   else if (0 != (flags & RT_MH_ZEROARRAY)) OSCRTLMEMSET (ptr, 0, arraySize);

   return ptr;
}
