/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

EXTOERMETHOD int oerDecObjId (OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   size_t length;
   int stat, j, retval = 0;

   stat = oerDecLen (pctxt, &length);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (length == 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   /* Parse OID contents */

   j = 0;
   while (length > 0 && stat == 0)
   {
      if (j < ASN_K_MAXSUBIDS) {
         int b = 0; OSOCTET ub;

         /* Parse a subidentifier out of the contents field */

         pvalue->subid[j] = 0;
         do {
            stat = rtxReadBytes (pctxt, &ub, 1);
            if (stat >= 0) {
               b = ub;
               pvalue->subid[j] = (pvalue->subid[j] * 128) + (b & 0x7F);
               length--;
               stat = 0;
            }
         } while (b & 0x80 && length > 0 && stat == 0);

         /* Handle the first subidentifier special case: the first two 	*/
         /* sub-id's are encoded into one using the formula (x * 40) + y */

         if (j == 0)
         {
            OSUINT32 subid = pvalue->subid[0];
            pvalue->subid[0] = ((subid / 40) >= 2) ? 2 : subid / 40;
            pvalue->subid[1] = (pvalue->subid[0] == 2) ?
               subid - 80 : subid % 40;
            j = 2;
         }
         else j++;
      }
      else
         stat = ASN_E_INVOBJID;
   }

   pvalue->numids = j;
   if (stat == 0 && length != 0) stat = ASN_E_INVLEN;

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : retval;
}
