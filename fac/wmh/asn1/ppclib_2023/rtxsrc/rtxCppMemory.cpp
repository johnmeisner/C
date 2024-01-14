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
#if 0
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxCommonDefs.h"

void* rtxMemAlloc (OSCTXT* , size_t nbytes)
{
   return new OSOCTET [nbytes];
}

void* rtxMemAllocZ (OSCTXT* , size_t nbytes)
{
   OSOCTET* ptr = new OSOCTET [nbytes];
   if (0 != ptr) memset (ptr, 0, nbytes);
   return ptr;
}

void rtxMemFreePtr (OSCTXT* , void* mem_p)
{
   delete [] (OSOCTET*)mem_p;
}

void* rtxMemRealloc (OSCTXT* , void* mem_p, size_t nbytes)
{
   OSOCTET* ptr = new OSOCTET [nbytes];
   if (0 != mem_p) {
      //memcpy (ptr, mem_p, OSRTMIN(oldnbytes, nbytes)); FIXME
      memcpy (ptr, mem_p, nbytes);
      delete [] (OSOCTET*)mem_p;
   }
   return ptr;
}

void rtxMemReset (OSCTXT*)
{
   // nothing to do! Unsupported for C++
}

/* Temporary */
void* rtxMemNewArray (size_t nbytes) { return 0; }
void* rtxMemNewArrayZ (size_t nbytes) { return 0; }
void rtxMemDeleteArray (void* mem_p) {}

#endif
