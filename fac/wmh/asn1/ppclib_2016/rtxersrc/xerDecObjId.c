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

#include "xed_common.hh"

int xerDecObjId (OSCTXT* pctxt, ASN1OBJID *pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   OSUINT32 val;

   pvalue->numids = 0;

   /* Consume leading white space */

   while (*inpdata != 0 && OS_ISSPACE (*inpdata)) {
      inpdata ++;
   }

   val = 0;
   for ( ; ; inpdata++ ) {

      if (pvalue->numids >= ASN_K_MAXSUBIDS)
         return LOG_RTERR (pctxt, ASN_E_INVOBJID);

      if (OS_ISDIGIT (*inpdata))
         val = (val * 10) + (*inpdata - '0');
      else if (*inpdata == '.') {
         if (!OS_ISDIGIT (*(inpdata + 1))) /* check the next char - should be digit only */
            return LOG_RTERR (pctxt, ASN_E_INVOBJID);
         pvalue->subid[pvalue->numids++] = val;
         val = 0;
      }
      else if (OS_ISSPACE (*inpdata)) {
         /* skip trailing white space */

         while (*inpdata != 0 && OS_ISSPACE (*inpdata)) {
            inpdata ++;
         }
         if (*inpdata != 0) /* if not end - error */
            return LOG_RTERR (pctxt, ASN_E_INVOBJID);
         else {
            pvalue->subid[pvalue->numids++] = val;
            break;
         }
      }
      else if (*inpdata == 0) {
         pvalue->subid[pvalue->numids++] = val;
         break;
      }
      else
         return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   }

   /* Validate given object ID by applying ASN.1 rules */

   if (0 == pvalue) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->numids < 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] > 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] != 2 && pvalue->subid[1] > 39)
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   LCHECKXER (pctxt);

   return (0);
}
