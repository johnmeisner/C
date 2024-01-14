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

#include "rtbersrc/xse_common.hh"
#include "rtxsrc/rtxUtil.h"

static int berEncStrmIdentifier64 (OSCTXT *pctxt, OSUINT64 ident)
{
   OSUINT64 mask = 0x7f;
   int nshifts = 0, stat = 0;

   /* Find starting point in identifier value */
   nshifts = rtxGetIdent64ByteCount (ident);
   mask <<= (7 * nshifts);

   /* Encode bytes */
   if (nshifts > 0) {
      OSUINT64 lv;
      while (nshifts > 0) {
         mask >>= 7; nshifts--;
         lv = (ident & mask) >> (nshifts * 7);
         if (nshifts != 0) { lv |= 0x80; }
         stat = berEncStrmWriteOctet (pctxt, (OSOCTET)lv);
         if (stat != 0) break;
      }
   }
   else {
      /* encode a single zero byte */
      stat = berEncStrmWriteOctet (pctxt, 0);
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

int berEncStrmObjId64 (OSCTXT* pctxt, const ASN1OID64 *object_p,
                       ASN1TagType tagging)
{
   register unsigned i;
   int nbytes = 1; /* 1st 2 arcs require 1 byte */
   int stat;

   /* Validate given object ID by applying ASN.1 rules */

   if ((0 == object_p) ||
       (object_p->numids < 2) || (object_p->subid[0] > 2) ||
       (object_p->subid[0] != 2 && object_p->subid[1] > 39))
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Passed checks, calculate nbytes for object identifier */

   for (i = 2; i < object_p->numids; i++) {
      nbytes += rtxGetIdent64ByteCount (object_p->subid[i]);
   }

   /* Encode object identifier */

   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OBJID, nbytes);
   else
      stat = berEncStrmLength (pctxt, nbytes);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Munge first two sub ID's and encode */

   stat = berEncStrmIdentifier64
      (pctxt, ((object_p->subid[0] * 40) + object_p->subid[1]));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Encode the remainder of the OID value */

   for (i = 2; i < object_p->numids; i++) {
      stat = berEncStrmIdentifier64 (pctxt, object_p->subid[i]);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
