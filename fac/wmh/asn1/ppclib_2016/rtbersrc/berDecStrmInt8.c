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

int berDecStrmInt8 (OSCTXT *pctxt, OSINT8 *object_p,
                    ASN1TagType tagging, int length)
{
   register int stat = 0;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_INT, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   /* Make sure integer will fit in target variable */

   if (length > (int)(sizeof(OSINT8))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else if (length <= 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN); /* note: indef len not allowed */

   /* Copy first byte into a signed char variable and assign it to */
   /* object.  This should handle sign extension in the case of a  */
   /* negative number..                                            */

   stat = rtxReadBytes (pctxt, (OSOCTET*)object_p, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
