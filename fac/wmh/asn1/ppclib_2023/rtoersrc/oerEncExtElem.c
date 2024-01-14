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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBuffer.h"

/* Encode an OER extension element.  The element is assumed to be encoded
   in the passed buffer object.  This adds the wrapper and encodes to the
   context. */

EXTOERMETHOD int oerEncExtElem (OSCTXT* pctxt, OSRTBuffer* pbuffer)
{
   int stat = oerEncLen (pctxt,  pbuffer->byteIndex);

   if (stat >= 0) {
      stat = rtxWriteBytes (pctxt,  pbuffer->data, pbuffer->byteIndex);
   }

   if (stat < 0) {
      rtxMemFreePtr (pctxt, pbuffer->data);
   }
   else pbuffer->byteIndex = 0;

   return stat;
}

/* Encode an OER open extension element */

EXTOERMETHOD int oerEncOpenExt (OSCTXT* pctxt, OSRTDList* pElemList)
{
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;

      while (0 != pnode) {
         if (0 != pnode->data) {
            int stat;
            ASN1OpenType* pOpenType = (ASN1OpenType*)pnode->data;

            if (pOpenType->numocts > 0) {
               stat = oerEncLen (pctxt, pOpenType->numocts);

               if (stat >= 0) {
                  stat = rtxWriteBytes
                     (pctxt, pOpenType->data, pOpenType->numocts);
               }

               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
         }
         pnode = pnode->next;
      }
   }

   return 0;
}

