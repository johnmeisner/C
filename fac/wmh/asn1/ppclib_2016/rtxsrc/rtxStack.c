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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStack.h"
#include "rtxsrc/rtxErrCodes.h"

/* Stack */

EXTRTMETHOD void rtxStackInit (OSCTXT* pctxt, OSRTStack* pStack)
{
   pStack->pctxt = pctxt;
   rtxDListInit (&pStack->dlist);
}

EXTRTMETHOD OSRTStack* rtxStackCreate (OSCTXT* pctxt)
{
   OSRTStack* pStack = rtxMemAllocType (pctxt, OSRTStack);
   if (pStack == 0) return 0;
   rtxStackInit (pctxt, pStack);
   return pStack;
}

EXTRTMETHOD int rtxStackPush (OSRTStack* pStack, void* pData)
{
   if (0 != pStack && 0 != pStack->pctxt) {
      rtxDListInsert (pStack->pctxt, &pStack->dlist, 0, pData);
      return 0;
   }
   return RTERR_NULLPTR;
}

EXTRTMETHOD void* rtxStackPop (OSRTStack* pStack)
{
   if (0 != pStack && 0 != pStack->pctxt && pStack->dlist.count > 0) {
      void* pdata = pStack->dlist.head->data;
      rtxDListFreeNode (pStack->pctxt, &pStack->dlist, pStack->dlist.head);
      return pdata;
   }
   return NULL;
}

EXTRTMETHOD void* rtxStackPeek (OSRTStack* pStack)
{
   return (pStack->dlist.count > 0) ? pStack->dlist.head->data : 0;
}

