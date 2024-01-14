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

#include "asn1ber.h"
#include "rtsrc/rtBCD.h"

/*
 * Decode binary encoded decimal (BCD) string.
 * Note 1: this assumes the base ASN.1 type of the string is OCTET STRING.
 * Note 2: constructed form of string is currently not supported.
 */
int xd_bcdstr (OSCTXT* pctxt, const char** ppvalue,
               ASN1TagType tagging, int length)
{
   int ll, stat = 0;
   char* pstr;

   if (tagging == ASN1EXPL) {
      if ((stat = xd_match1 (pctxt, ASN_ID_OCTSTR, &length)) < 0)
         /* ASN_E_IDNOTFOU will be logged later, by the generated code,
            or reset by rtErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (((pctxt->flags & ASN1CONSTAG) != 0) || (length == ASN_K_INDEFLEN)) {
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);
   }

   ll = (length * 2) + 1;

   pstr = (char*) rtxMemAlloc (pctxt, ll);
   if (0 == pstr) return LOG_RTERR (pctxt, RTERR_NOMEM);

   *ppvalue = rtBCDToString (length, ASN1BUFPTR(pctxt), pstr, ll, FALSE);

   pctxt->buffer.byteIndex += length;

   return 0;
}
