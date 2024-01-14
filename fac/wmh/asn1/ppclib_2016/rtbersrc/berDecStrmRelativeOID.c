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

#include "rtbersrc/asn1berStream.h"

int berDecStrmRelativeOID (OSCTXT *pctxt, ASN1OBJID *object_p,
                           ASN1TagType tagging, int length)
{
   register int	stat = 0, j;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_RELOID, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (length <= 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN); /* note: indef len not allowed */

   /* Parse OID contents */

   j = 0;
   while (length > 0 && stat == 0)
   {
      if (j < ASN_K_MAXSUBIDS) {
         int b = 0; OSOCTET ub;

         /* Parse a subidentifier out of the contents field */

         object_p->subid[j] = 0;
         do {
            stat = rtxReadBytes (pctxt, &ub, 1);
            if (stat >= 0) {
               b = ub;
               object_p->subid[j] = (object_p->subid[j] * 128) + (b & 0x7F);
               length--;
               stat = 0;
            }
         } while (b & 0x80 && length > 0 && stat == 0);

         j++;
      }
      else
         stat = ASN_E_INVOBJID;
   }

   object_p->numids = j;
   if (stat == 0 && length != 0) stat = ASN_E_INVLEN;

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}
