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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_ObjectIdentifier
 *
 *  Description:  The following function decodes an object identifier
 *                value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdata       objid*  pointer to OID value to receive decoded data
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_ObjectIdentifier (OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   OSUINT32  len;
   int       stat, j;
   unsigned  subid;
   OSUINT32  b;

   /* Decode unconstrained length */

   if ((stat = pd_Length (pctxt, &len)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   PU_NEWFIELD (pctxt, "ObjectID");

   if (pvalue) {
      /* Copy contents to a byte-aligned local buffer */

      j = 0;
      while (len > 0 && stat == 0)
      {
         if (j < ASN_K_MAXSUBIDS) {

            /* Parse a subidentifier out of the contents field */

            pvalue->subid[j] = 0;
            do {
               if ((stat = pd_bits (pctxt, &b, 8)) == 0)
               {
                  pvalue->subid[j] = (pvalue->subid[j] * 128) + (b & 0x7F);
                  len--;
               }
            } while (b & 0x80 && stat == 0);

            /* Handle the first subidentifier special case: the first two    */
            /* sub-id's are encoded into one using the formula (x * 40) + y  */

            if (j == 0)
            {
               subid = pvalue->subid[0];
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

      if (stat == 0 && len != 0) return LOG_RTERR(pctxt, ASN_E_INVLEN);

      pvalue->numids = j;
   }
   else {
      stat = rtxSkipBytes(pctxt, len);
   }

   PU_SETBITCOUNT(pctxt);

   LCHECKPER (pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}
