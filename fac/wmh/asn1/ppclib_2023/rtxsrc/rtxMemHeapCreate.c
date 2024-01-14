/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#if !defined(_ARMTCC) && !defined(__SYMBIAN32__)

#if defined(_WIN32) && defined(_DLL)
/* DLL runtime */

static OSMallocFunc  g_malloc_func = 0;
static OSReallocFunc g_realloc_func = 0;
static OSFreeFunc    g_free_func = 0;

#else /* defined(_WIN32) && defined(_DLL) */
/* static runtime */

static OSMallocFunc  g_malloc_func = OSCRTLMALLOC;
static OSFreeFunc    g_free_func = OSCRTLFREE;

#ifndef _NO_REALLOC
static OSReallocFunc g_realloc_func = OSCRTLREALLOC;
#else /*_NO_REALLOC */
static OSReallocFunc g_realloc_func = 0;
#endif /*_NO_REALLOC */

#endif /* defined(_WIN32) && defined(_DLL) */

EXTRTMETHOD void rtxMemSetAllocFuncs
(OSMallocFunc malloc_func, OSReallocFunc realloc_func, OSFreeFunc free_func)
{
   g_malloc_func  = malloc_func;
   g_free_func    = free_func;

   /* the realloc func could be set to NULL. In this case,
    * malloc/OSCRTLMEMCPY/free will be used. */
   g_realloc_func = realloc_func;
}

#else /* _ARMTCC or __SYMBIAN32__ */

#define g_malloc_func  OSCRTLMALLOC
#define g_free_func    OSCRTLFREE

#ifndef _NO_REALLOC
#define g_realloc_func OSCRTLREALLOC
#else /*_NO_REALLOC */
#define g_realloc_func 0
#endif /*_NO_REALLOC */

#endif /* _ARMTCC or __SYMBIAN32__ */

static OSMemHeap* newHeap()
{
   OSMemHeap* pMemHeap;
#if defined(_WIN32) && defined(_DLL)
/* DLL runtime */
   pMemHeap = (g_malloc_func) ? g_malloc_func (sizeof (OSMemHeap)) :
                                OSCRTLMALLOC (sizeof (OSMemHeap));
#else /* defined(_WIN32) && defined(_DLL) */
/* static runtime */
   pMemHeap = (OSMemHeap*) g_malloc_func (sizeof (OSMemHeap));
#endif /* defined(_WIN32) && defined(_DLL) */

   if (0 != pMemHeap) {
      OSCRTLMEMSET (pMemHeap, 0, sizeof (OSMemHeap));
   }

   return pMemHeap;
}

EXTRTMETHOD int rtxMemHeapCreate (void** ppvMemHeap)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0) return RTERR_INVPARAM;

   pMemHeap = newHeap();
   if (pMemHeap == NULL) return RTERR_NOMEM;

#ifdef _MEMCOMPACT
   pMemHeap->code = (OSUINT16) rand();
#else
   pMemHeap->defBlkSize = rtxMemGetDefBlkSize();
   pMemHeap->code = (OSUINT32) rand();
#endif

   pMemHeap->refCnt = 1;
   pMemHeap->flags = RT_MH_FREEHEAPDESC /*| RT_MH_USEATOMBLOCK*/;
#ifdef _MEMDEBUG
   pMemHeap->flags |= RT_MH_TRACE;
   pMemHeap->flags |= RT_MH_DIAG;
   pMemHeap->flags |= RT_MH_DIAG_DEBUG;
   pMemHeap->flags |= RT_MH_CHECKHEAP;
   pMemHeap->flags |= RT_MH_VALIDATEPTR;
#endif
   pMemHeap->mallocFunc = g_malloc_func;
   pMemHeap->reallocFunc = g_realloc_func;
   pMemHeap->memFreeFunc = g_free_func;

   pMemHeap->u.blockList.rawBlkList.pnext =
      pMemHeap->u.blockList.rawBlkList.pprev =
      &pMemHeap->u.blockList.rawBlkList;

   *ppvMemHeap = (void*)pMemHeap;

   return 0;
}

#ifndef _MEMCOMPACT
int rtxMemStaticHeapCreate (void **ppvMemHeap, void* pmem, size_t memsize)
{
   OSMemHeap* pMemHeap;

   if (ppvMemHeap == 0) return RTERR_INVPARAM;

   pMemHeap = newHeap();
   if (pMemHeap == NULL) return RTERR_NOMEM;

   pMemHeap->refCnt = 1;
   pMemHeap->flags = RT_MH_STATIC | RT_MH_FREEHEAPDESC;
   pMemHeap->u.staticMem.data = (OSOCTET*)pmem;
   pMemHeap->u.staticMem.dataSize = memsize;
   pMemHeap->u.staticMem.freeIdx = 0;

   /* These are needed for rtxMemSys* functions.. */
   pMemHeap->mallocFunc = OSCRTLMALLOC;
   pMemHeap->reallocFunc = OSCRTLREALLOC;
   pMemHeap->memFreeFunc = OSCRTLFREE;

   *ppvMemHeap = (void*)pMemHeap;

   return 0;
}
#endif
