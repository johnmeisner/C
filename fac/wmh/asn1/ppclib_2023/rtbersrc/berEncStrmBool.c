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

#include "rtbersrc/xse_common.hh"

int berEncStrmBool (OSCTXT* pctxt, OSBOOL value, ASN1TagType tagging)
{
   OSOCTET ub;
   int stat;

   ub = (value == 0) ? 0 : 0xff;
   if (pctxt->flags & ASN1BOOLTRUE1) ub = 0x01;

   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BOOL, 1);
   else
      stat = berEncStrmLength (pctxt, 1);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = berEncStrmWriteOctet (pctxt, ub);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

