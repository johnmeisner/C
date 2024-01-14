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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  DER ENCODE FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: xe_derCanonicalSort, xe_derCanSortSet
 *
 *  Description:  This routine sorts a list of encoded elements in
 *                descending canonical order.  The list of elements
 *                passed in will be consumed and not be available
 *                for subsequent use.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to ASN.1 context block structure.
 *  pList       list    Pointer to linked list containing pointers
 *                      and length of message components to be sorted.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      int     Total length of all elements copied to the
 *                      encode buffer or negative if error.
 *
 *
 **********************************************************************/

static int derCanonicalSort (OSCTXT* pctxt, OSRTSList* pList, OSBOOL isSet)
{
   OSBOOL swap = TRUE, swapped = FALSE;
   OSRTSListNode *pNode1, *pNode2;
   Asn1BufLocDescr *pComponent1, *pComponent2;
   OSOCTET *ptmpbuf, *pelem1, *pelem2;
   OSOCTET *pEOB = pctxt->buffer.data + pctxt->buffer.size;
   OSOCTET ub1, ub2;
   OSSIZE cmplen, offset, totalSize;
   int result;
   void* ptmp;

   /* Sort elements */

   while (swap) {
      swap = FALSE;
      pNode1 = 0;
      pNode2 = pList->head;

      while (pNode2) {
         if (pNode1) {
            pComponent1 = (Asn1BufLocDescr*) pNode1->data;
            pComponent2 = (Asn1BufLocDescr*) pNode2->data;

            cmplen = OSRTMIN (pComponent1->numocts, pComponent2->numocts);

            /* Calculate encode buffer location pointers */
            pelem1 = pEOB - pComponent1->offset;
            pelem2 = pEOB - pComponent2->offset;

            if (isSet) {
               /* Clear the form bit in the first tag byte of each of the
                  encoded TLV's */
               ub1 = *pelem1;
               ub2 = *pelem2;

               *pelem1 &= ~TM_FORM; /* clear form bit */
               *pelem2 &= ~TM_FORM;
            }
            else ub1 = ub2 = 0; /* to prevent VC++ warning */

            result = memcmp (pelem1, pelem2, cmplen);

            if (isSet) {
               *pelem1 = ub1;
               *pelem2 = ub2;
            }

            if (result < 0 || (result == 0 &&
                               pComponent1->numocts < pComponent2->numocts)) {
               /* swap the data, not node */
               ptmp = pNode1->data;
               pNode1->data = pNode2->data;
               pNode2->data = ptmp;
               swap = swapped = TRUE;
            }
         }
         pNode1 = pNode2;
         pNode2 = pNode2->next;
      }
   }

   /* Compute total size of all sorted elements */

   pNode1 = pList->head;
   totalSize = 0;

   while (pNode1) {
      pComponent1 = (Asn1BufLocDescr*) pNode1->data;
      totalSize += pComponent1->numocts;
      pNode1 = pNode1->next;
   }

   /* If at least one element was swapped.. */

   if (swapped) {

      /* Copy sorted elements to temp buffer */

      ptmpbuf = (OSOCTET*) rtxMemAlloc (pctxt, totalSize);
      if (ptmpbuf != 0) {
         pNode1 = pList->head;
         offset = totalSize;

         while (pNode1) {
            pComponent1 = (Asn1BufLocDescr*) pNode1->data;
            pelem1 = pEOB - pComponent1->offset;

            offset -= pComponent1->numocts;
            memcpy (&ptmpbuf[offset], pelem1, pComponent1->numocts);

            rtxMemFreePtr (pctxt, pComponent1);
            pNode2 = pNode1;
            pNode1 = pNode2->next;
            rtxMemFreePtr (pctxt, pNode2);
         }

         /* Now copy the temp buffer back into the encode buffer */

         memcpy (ASN1BUFPTR(pctxt), ptmpbuf, totalSize);
         rtxMemFreePtr (pctxt, ptmpbuf);
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   else {
      pNode1 = pList->head;
      while (pNode1) {
         pComponent1 = (Asn1BufLocDescr*) pNode1->data;
         rtxMemFreePtr (pctxt, pComponent1);
         pNode2 = pNode1;
         pNode1 = pNode2->next;
         rtxMemFreePtr (pctxt, pNode2);
      }
   }

   LCHECKBER (pctxt);

   return (int)totalSize;
}

int xe_derCanonicalSort (OSCTXT* pctxt, OSRTSList* pList)
{
   return derCanonicalSort (pctxt, pList, FALSE);
}

int xe_derCanSortSet (OSCTXT* pctxt, OSRTSList* pList)
{
   return derCanonicalSort (pctxt, pList, TRUE);
}
