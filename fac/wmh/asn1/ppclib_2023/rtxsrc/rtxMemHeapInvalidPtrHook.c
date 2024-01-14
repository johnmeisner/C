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

#include <stdio.h>
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemHeap.hh"

#ifdef _MEMDEBUG

#ifdef WIN32
#include <windows.h>
#define DEBUG_BREAK do { DebugBreak (); abort (); } while (0)
#else
#define DEBUG_BREAK \
rtxErrAssertionFailed("mem pointer valid", 0, "rtxMemHeapInvalidPtrHook.c")
#endif

#else /* _MEMDEBUG */
#define DEBUG_BREAK \
rtxErrAssertionFailed("mem pointer valid", 0, "rtxMemHeapInvalidPtrHook.c")
#endif /* _MEMDEBUG */

EXTRTMETHOD void rtxMemHeapInvalidPtrHook
(void** ppvMemHeap, const void* mem_p)
{
   OS_UNUSED_ARG(ppvMemHeap);
   OS_UNUSED_ARG(mem_p);

   DEBUG_BREAK;
}

