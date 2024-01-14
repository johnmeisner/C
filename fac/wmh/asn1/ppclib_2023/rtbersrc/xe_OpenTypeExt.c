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

/***********************************************************************
 *
 *  Routine name: xe_OpenTypeExt
 *
 *  Description:  This routine encodes an ASN.1 open type extension.
 *                This is a lit of previously encoded message components
 *                which are to be included as is in the current encode
 *                buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      list*   Pointer to list of open elements to encode
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_OpenTypeExt (OSCTXT* pctxt, OSRTDList* pElemList)
{
   OSRTDListNode* pnode;
   ASN1OpenType* pOpenType;
   int aal = 0, ll;

   if (0 != pElemList) {
      pnode = pElemList->tail;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*)pnode->data;
            ll = xe_OpenType (pctxt, pOpenType->data, pOpenType->numocts);
            if (ll >= 0) aal += ll; else return LOG_RTERR (pctxt, ll);
         }
         pnode = pnode->prev;
      }
   }

   return (aal);
}


int xe_OpenTypeExtDer (OSCTXT* pctxt, OSRTDList* pElemList, OSRTSList* pBufList)
{
   OSRTDListNode* pnode;
   ASN1OpenType* pOpenType;
   int aal = 0, ll;

   if (0 != pElemList) {
      pnode = pElemList->tail;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*)pnode->data;
            ll = xe_OpenType (pctxt, pOpenType->data, pOpenType->numocts);
            if (ll >= 0)
            {
               Asn1BufLocDescr* pbuf = rtxMemAllocType (pctxt, Asn1BufLocDescr);
               xe_getBufLocDescr (pctxt, ll, pbuf);
               rtxSListAppend (pBufList, pbuf);

               aal += ll;
            }
            else return LOG_RTERR (pctxt, ll);
         }
         pnode = pnode->prev;
      }
   }

   return (aal);
}
