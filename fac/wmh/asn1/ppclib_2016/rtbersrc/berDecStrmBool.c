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

int berDecStrmBool (OSCTXT *pctxt, OSBOOL *object_p,
                    ASN1TagType tagging, int length)
{
   int stat;

   if (tagging == ASN1EXPL) {
      if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_BOOL, &length, TRUE)) != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (length == 1) {
      OSOCTET b;
      stat = rtxReadBytes (pctxt, &b, 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
      *object_p = (OSBOOL)b;
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   return 0;
}

