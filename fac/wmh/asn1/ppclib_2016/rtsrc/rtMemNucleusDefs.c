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

/* Memory management utility functions, ver. 5.6.0 */

#include <stdarg.h>
#include <stdlib.h>
#include "rtxsrc/rtxMemHeap.hh"

#ifdef _NUCLEUS
static NU_MEMORY_POOL* nuPool = 0;
extern VOID ERC_System_Error(STATUS sts);

static void *rtNucleusMalloc (size_t size)
{
   VOID   *pointer;
   STATUS sts;

   if (nuPool == 0)
      NU_Memory_Pool_Pointers (&nuPool, 1);

   sts = NU_Allocate_Memory (nuPool, &pointer, size,
                             NU_NO_SUSPEND);
   if (sts != NU_SUCCESS) {
      ERC_System_Error(sts);
      return NULL;
   }
   return pointer;
}

static void rtNucleusFree (void *ptr)
{
   STATUS sts;

   sts = NU_Deallocate_Memory (ptr);
   if (sts != NU_SUCCESS) {
      ERC_System_Error(sts);
   }
}

OSMallocFunc  g_malloc_func = rtNucleusMalloc;
OSReallocFunc g_realloc_func = 0;
OSFreeFunc    g_free_func = rtNucleusFree;
#endif /* _NUCLEUS */

