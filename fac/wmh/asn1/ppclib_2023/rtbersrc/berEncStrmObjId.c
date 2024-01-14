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
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxUtil.h"

static int encodeStrmOID
(OSCTXT* pctxt, OSSIZE numids, const OSUINT32* pSubIds, ASN1TagType tagging)
{
   OSSIZE i;
   int nbytes;
   int stat;

   /* Validate given object ID by applying ASN.1 rules */

   if (numids < 2 || pSubIds[0] > 2 ||
       (pSubIds[0] != 2 && pSubIds[1] > 39))
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Passed checks, calculate nbytes for object identifier */

   nbytes = rtxGetIdentByteCount (pSubIds[0] * 40 + pSubIds[1]);
   for (i = 2; i < numids; i++) {
      nbytes += rtxGetIdentByteCount (pSubIds[i]);
   }

   /* Encode object identifier */

   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OBJID, nbytes);
   else
      stat = berEncStrmLength (pctxt, nbytes);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Munge first two sub ID's and encode */

   stat = berEncStrmIdentifier
      (pctxt, ((pSubIds[0] * 40) + pSubIds[1]));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Encode the remainder of the OID value */

   for (i = 2; i < numids; i++) {
      stat = berEncStrmIdentifier (pctxt, pSubIds[i]);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   LCHECKBER (pctxt);

   return 0;
}

int berEncStrmObjId
(OSCTXT* pctxt, const ASN1OBJID *object_p, ASN1TagType tagging)
{
   return (0 == object_p) ? LOG_RTERR(pctxt, ASN_E_INVOBJID) :
      encodeStrmOID(pctxt, object_p->numids, object_p->subid, tagging);
}

int berEncStrmDynObjId
(OSCTXT* pctxt, const ASN1DynOBJID *object_p, ASN1TagType tagging)
{
   return (0 == object_p) ? LOG_RTERR(pctxt, ASN_E_INVOBJID) :
      encodeStrmOID(pctxt, object_p->numids, object_p->pSubIds, tagging);
}

