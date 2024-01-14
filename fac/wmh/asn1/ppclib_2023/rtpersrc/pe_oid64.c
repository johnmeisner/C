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

#ifndef _NO_INT64_SUPPORT

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxUtil.h"

/***********************************************************************
 *
 *  Routine name: pe_oid64
 *
 *  Description:  The following function encodes an object identifier
 *                value.  This object identifier uses the C ASN1OID64
 *                type that can hold 64-bit arc values.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        objid*  pointer to object ID structure to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_oid64 (OSCTXT* pctxt, ASN1OID64* pvalue)
{
   int len, stat;
   OSUINT64 temp;
   register int	numids, i;

   if (0 == pvalue) return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Calculate length in bytes and encode */

   len = 1;  /* 1st 2 arcs require 1 byte */
   numids = pvalue->numids;
   for (i = 2; i < numids; i++) {
      len += rtxGetIdent64ByteCount (pvalue->subid[i]);
   }

   /* PER encode length */

   if ((stat = pe_Length (pctxt, (OSUINT32)len)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   /* Copy octets to PER encode buffer */

   PU_NEWFIELD (pctxt, "ObjectId");

   /* Validate given object ID by applying ASN.1 rules */

   if (numids < 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] > 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] != 2 && pvalue->subid[1] > 39)
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   /* Passed checks, encode object identifier */

   /* Munge first two sub ID's and encode */

   temp = ((pvalue->subid[0] * 40) + pvalue->subid[1]);
   if ((stat = pe_identifier64 (pctxt, temp)) != 0)
      return LOG_RTERR (pctxt, stat);

   /* Encode the remainder of the OID value */

   for (i = 2; i < numids; i++) {
      if ((stat = pe_identifier64 (pctxt, pvalue->subid[i])) != 0)
         return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}

#endif

