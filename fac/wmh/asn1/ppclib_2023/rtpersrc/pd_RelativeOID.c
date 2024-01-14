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

/***********************************************************************
 *
 *  Routine name: pd_RelativeOID
 *
 *  Description:  The following function decodes a relative object identifier
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

EXTPERMETHOD int pd_RelativeOID (OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   OSUINT32  len;
   int       stat, j;
   OSUINT32  b;

   /* Decode unconstrained length */

   if ((stat = pd_Length (pctxt, &len)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   /* Copy contents to a byte-aligned local buffer */

   PU_NEWFIELD (pctxt, "RelOID");
   if (pvalue) {
      j = 0;
      while (len > 0 && stat == 0) /*!AB (09/01/2002) - check the stat as well. */
      {
         if (j < ASN_K_MAXSUBIDS) {     /*!AB (09/01/2002) */

            /* Parse a subidentifier out of the contents field */

            pvalue->subid[j] = 0;
            do {
               if ((stat = pd_bits (pctxt, &b, 8)) == 0)
               {
                  pvalue->subid[j] = (pvalue->subid[j] * 128) + (b & 0x7F);
                  len--;
               }
            } while (b & 0x80 && stat == 0);

            j++;
         }
         else
            stat = ASN_E_INVOBJID;
      }

      pvalue->numids = j;
      if (stat == 0 && len != 0)
         return LOG_RTERR(pctxt, ASN_E_INVLEN);
   }
   else {
      stat = rtxSkipBytes(pctxt, len);
   }

   PU_SETBITCOUNT (pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return (stat);
}
