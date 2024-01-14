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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxIntStack.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxErrCodes.h"

/* Stack */

EXTRTMETHOD int rtxIntStackInit
(OSCTXT* pctxt, OSRTIntStack* pstack, size_t capacity)
{
   pstack->pctxt = pctxt;
   pstack->index = 0;
   pstack->size = (0 == capacity) ? OSRTISTK_DEFAULT_CAPACITY : capacity;
   pstack->pdata = rtxMemAllocArray (pctxt, pstack->size, OSINT32);
   return (0 == pstack->pdata) ? RTERR_NOMEM : 0;
}

EXTRTMETHOD int rtxIntStackPush (OSRTIntStack* pstack, OSINT32 value)
{
   if (pstack->index >= pstack->size) {
      pstack->size *= 2; /* double stack size */
      pstack->pdata = rtxMemReallocArray
         (pstack->pctxt, pstack->pdata, pstack->size, OSINT32);
      if (0 == pstack->pdata) return RTERR_NOMEM;
   }

   pstack->pdata[pstack->index++] = value;

   return 0;
}

EXTRTMETHOD int rtxIntStackPop (OSRTIntStack* pstack, OSINT32* pvalue)
{
   if (pstack->index > 0) {
      pstack->index--;
      if (0 != pvalue) { *pvalue = pstack->pdata[pstack->index]; }
      return 0;
   }
   else return RTERR_ENDOFBUF;
}

EXTRTMETHOD int rtxIntStackPeek (OSRTIntStack* pstack, OSINT32* pvalue)
{
   if (pstack->index > 0) {
      if (0 != pvalue) { *pvalue = pstack->pdata[pstack->index - 1]; }
      return 0;
   }
   else return RTERR_ENDOFBUF;
}
