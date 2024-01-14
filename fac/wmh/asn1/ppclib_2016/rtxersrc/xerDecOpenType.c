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
// 06/20/02     ED      Modified to copy data directly
//
////////////////////////////////////////////////////////////////////////////
*/
#include "xed_common.hh"

int xerDecOpenType (OSCTXT* pctxt, ASN1OpenType* pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int i;

   /* Count octets */
   pvalue->numocts = 0;
   for (i = 0; inpdata[i] != 0; i++) {
      pvalue->numocts += sizeof (XMLCHAR);
   }

   pvalue->data = (OSOCTET*) rtxMemAlloc (pctxt, pvalue->numocts);
   if (pvalue->data != 0)
      memcpy (inpdata, pvalue->data, pvalue->numocts);
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   return 0;
}

