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

#include "rtbersrc/asn1ber.h"
#include "rtsrc/rtBCD.h"

/*
 * Encode binary encoded decimal (BCD) string.
 * Note: this assumes the base ASN.1 type of the string is OCTET STRING.
 */
int xe_bcdstr (OSCTXT* pctxt, const char* value, ASN1TagType tagging)
{
   int aal = 0;

   if (0 != value) {
      OSSIZE numocts = (OSCRTLSTRLEN(value)/2) + 1;
      int len;

      if (numocts > pctxt->buffer.byteIndex) {
         int stat = xe_expandBuffer (pctxt, numocts);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      pctxt->buffer.byteIndex -= numocts;

      len = rtStringToBCD (value, ASN1BUFPTR(pctxt), numocts, FALSE);
      if (len < 0) return LOG_RTERR (pctxt, len);
      else if ((OSSIZE)len != numocts)
         return LOG_RTERR (pctxt, RTERR_BADVALUE);
      else aal += len;

      if (tagging == ASN1EXPL && aal >= 0)
         aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OCTSTR, aal);
   }

   return (aal);
}

