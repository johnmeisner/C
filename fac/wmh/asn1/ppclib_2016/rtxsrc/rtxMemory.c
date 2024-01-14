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
/* Memory management utility functions, ver. 5.6.0 */

/* This is needed to disable VC++ Level 4 warning about empty translation unit */
#ifdef _MSC_VER
#pragma warning(disable: 4206)
#endif /* _MSC_VER */

#if 0
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxMemHeap.hh"

#ifdef rtxMemAlloc
#undef rtxMemAlloc
#endif /* rtxMemAlloc */

#ifdef rtxMemAllocZ
#undef rtxMemAllocZ
#endif /* rtxMemAllocZ */

#ifdef rtxMemRealloc
#undef rtxMemRealloc
#endif /* rtxMemRealloc */

#ifdef rtxMemFreePtr
#undef rtxMemFreePtr
#endif /* rtxMemFreePtr */

#ifdef rtxMemFree
#undef rtxMemFree
#endif /* rtxMemFree */

#ifdef rtxMemReset
#undef rtxMemReset
#endif /* rtxMemReset */

#ifdef __cplusplus
extern "C" {
#endif

EXTERNRT void* rtxMemAlloc (OSCTXT* pctxt, size_t nbytes);
EXTERNRT void* rtxMemAllocZ (OSCTXT* pctxt, size_t nbytes);
EXTERNRT void  rtxMemFreePtr (OSCTXT* pctxt, void* mem_p);
EXTERNRT void* rtxMemRealloc (OSCTXT* pctxt, void* mem_p, size_t nbytes);
EXTERNRT void  rtxMemReset (OSCTXT* pctxt);

#ifdef __cplusplus
}
#endif

void* rtxMemAlloc (OSCTXT* pctxt, size_t nbytes)
{
   return rtxMemHeapAlloc (&pctxt->pMemHeap, nbytes);
}

void* rtxMemAllocZ (OSCTXT* pctxt, size_t nbytes)
{
   void* ptr = rtxMemHeapAlloc (&pctxt->pMemHeap, nbytes);
   if (0 != ptr) OSCRTLMEMSET (ptr, 0, nbytes);
   return ptr;
}

void rtxMemFree (OSCTXT* pctxt)
{
   rtxMemHeapFreeAll (&pctxt->pMemHeap);
}

void rtxMemFreePtr (OSCTXT* pctxt, void* mem_p)
{
   rtxMemHeapFreePtr (&pctxt->pMemHeap, mem_p);
}

void* rtxMemRealloc (OSCTXT* pctxt, void* mem_p, size_t nbytes)
{
   return rtxMemHeapRealloc (&pctxt->pMemHeap, mem_p, nbytes);
}

void rtxMemReset (OSCTXT* pctxt)
{
   rtxMemHeapReset (&pctxt->pMemHeap);
}

/* Temporary */
EXTRTMETHOD void* rtxMemNewArray (size_t nbytes) { return 0; }
EXTRTMETHOD void* rtxMemNewArrayZ (size_t nbytes) { return 0; }
EXTRTMETHOD void rtxMemDeleteArray (void* mem_p) {}

#endif
