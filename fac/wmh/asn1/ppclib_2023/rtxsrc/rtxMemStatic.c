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

#include <stdlib.h>
#include "rtxMemStatic.h"
#include "rtxMemHeap.hh"

/* The static memory block is set here.  The size is adjusted by changing
 * the size of the byte array.  This array must be large enough to hold
 * all allocations prior to resetting the block.
 */
static OSOCTET gs_MemHeap[16*1024]; /* a 16K heap */
static size_t  gs_MemHeapFreeIdx;   /* index to next free segment */

/* Create a memory heap */
int rtxMemStaticHeapCreate (void **ppvMemHeap)
{
   static OSMemHeap memHeap;
   memHeap.rawBlkList.pnext = memHeap.rawBlkList.pprev = 0;
   memHeap.refCnt = 1;
   memHeap.flags = RT_MH_STATIC;

   /* These are needed for rtxMemSys* functions.. */
   memHeap.mallocFunc = OSCRTLMALLOC;
   memHeap.reallocFunc = OSCRTLREALLOC;
   memHeap.memFreeFunc = OSCRTLFREE;

   *ppvMemHeap = (void*)&memHeap;
   gs_MemHeapFreeIdx = 0;
   return 0;
}

/* Allocate memory */
void* rtxMemStaticHeapAlloc (size_t nbytes)
{
   if (gs_MemHeapFreeIdx + nbytes <= sizeof(gs_MemHeap)) {
      void* ptr = (void*)&gs_MemHeap[gs_MemHeapFreeIdx];
      gs_MemHeapFreeIdx += nbytes;
      return ptr;
   }
   else return 0;
}

/* Free heap memory, reset all heap variables. */
void rtxMemStaticHeapFreeAll()
{
   gs_MemHeapFreeIdx = 0;
}

/* Free heap memory, reset all heap variables, and free heap structure if
   it was allocated. */
void rtxMemStaticHeapRelease()
{
   gs_MemHeapFreeIdx = 0;
}
