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
/*
// CHANGE LOG
// Date         Init    Description
//
////////////////////////////////////////////////////////////////////////////
*/
#include "xee_common.hh"
#if OS_ASN1RT_VERSION >= 600
#include "rtxsrc/rtxBuffer.h"
#endif

static OSBOOL isBinaryData (const OSOCTET *data, OSSIZE nocts);

int xerEncOpenTypeExt (OSCTXT* pctxt, OSRTDList* pElemList)
{
   OSRTDListNode* pnode;
   ASN1OpenType* pOpenType;
   int stat;

   if (0 != pElemList) {
      pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*)pnode->data;

            if (isBinaryData (pOpenType->data, pOpenType->numocts) != 0) {
               stat = xerEncStartElement (pctxt, "binext", 0);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               stat = xerEncHexStrValue
                  (pctxt, pOpenType->numocts, pOpenType->data);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               stat = xerEncEndElement (pctxt, "binext");
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
            else {
               stat = xerEncIndent (pctxt);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               stat = rtxWriteBytes
                  (pctxt, pOpenType->data, pOpenType->numocts);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
         }

         pnode = pnode->next;
      }
   }
   pctxt->state = XEREND;

   return (0);
}

static OSBOOL isBinaryData (const OSOCTET *data, OSSIZE nocts)
{
   OSSIZE i = 0;

   if (data[0] == '<') {
      return FALSE;
   }

   for (i = 0; i < nocts; i ++) {
      if (data[i] < 32 || data[i] > 127) {
         return TRUE;
      }
   }

   return FALSE;
}
