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
#include "rtxsrc/rtxContext.hh"
#include "rtxmlsrc/osrtxml.hh"


EXTXMLMETHOD int rtXmlEncCanonicalSort(OSCTXT* pctxt, OSCTXT* pBufCtxt,
                                    OSRTSList* pList)
{
   int ret = 0;
   OSRTSListNode *pNode1, *pNode2;
   OSRTBufLocDescr *pComponent1;
   OSBOOL modified;
   OSOCTET* pBuf = pBufCtxt->buffer.data;

   modified = rtxCanonicalSort(pBufCtxt->buffer.data, pList, TRUE /* normal */);
   if (modified) {

      /* Write sorted elements */

      pNode1 = pList->head;
      while (pNode1) {
         pComponent1 = (OSRTBufLocDescr*) pNode1->data;

         if (ret == 0 && pComponent1->numocts > 0) {
            /* don't leave state as OSXMLSTART or we'll think the parent element
               is empty and the empty element tag (<myelem/>) logic will
               fail */
            pctxt->state = OSXMLDATA;

            ret = rtXmlWriteChars (pctxt,
               pBuf + pComponent1->offset, pComponent1->numocts);
         }

         rtxMemFreePtr (pBufCtxt, pComponent1);
         pNode2 = pNode1;
         pNode1 = pNode2->next;
         rtxMemFreePtr (pBufCtxt, pNode2);
      }
   }
   else {
      /* No reordering.  We can write buffer "as is".*/

      /* Don't leave state as OSXMLSTART or we'll think the parent element
         is empty and the empty element tag (<myelem/>) logic will
         fail */
      pctxt->state = OSXMLDATA;

      ret = rtXmlWriteChars (pctxt, pBuf, pBufCtxt->buffer.byteIndex);

      /* Clean up memory */
      pNode1 = pList->head;
      while (pNode1) {
         pComponent1 = (OSRTBufLocDescr*) pNode1->data;
         rtxMemFreePtr (pBufCtxt, pComponent1);
         pNode2 = pNode1;
         pNode1 = pNode2->next;
         rtxMemFreePtr (pBufCtxt, pNode2);
      }
   }

   if ( ret < 0 ) return LOG_RTERR(pctxt, ret);
   else return ret;
}
