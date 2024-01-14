/*
 * Copyright (c) 2014-2023 by Objective Systems, Inc.
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

#include "rtxsrc/rtxDynPtrArray.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

EXTRTMETHOD int rtxDynPtrArrayInit
(OSCTXT* pctxt, OSRTDynPtrArray* pArray, OSUINT16 initialSize)
{
   /* Set default initial size (4 pointers) */
   if (initialSize == 0) initialSize = 4;

   pArray->capacity = initialSize;
   pArray->count = 0;
   pArray->data = (void**) rtxMemAlloc (pctxt, initialSize * sizeof(void*));

   if (0 == pArray->data) {
      pArray->capacity = 0;
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return 0;
}

EXTRTMETHOD int rtxDynPtrArrayAppend
(OSCTXT* pctxt, OSRTDynPtrArray* pArray, void* ptr)
{
   if (pArray->count >= pArray->capacity) {
      pArray->capacity = (OSUINT16)
         ((0 == pArray->capacity) ? 4 : pArray->capacity * 2);

      pArray->data = (void**) rtxMemRealloc
         (pctxt, pArray->data, pArray->capacity * sizeof(void*));

      if (0 == pArray->data) {
         pArray->capacity = 0;
         return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
   }
   pArray->data[pArray->count++] = ptr;

   return 0;
}
