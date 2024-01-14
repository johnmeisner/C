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

#include "rtbersrc/asn1ber.h"


int xe_timestr(OSCTXT* pctxt, const char* pvalue,
               ASN1TagType tagging, ASN1TAG tag)
{
   if (NULL == pvalue)
   {
      return 0;
   }

   if (pctxt->flags & (ASN1CANON | ASN1DER))
   {
      ASN1Time timeVal;
      int stat = rtParseTime(pctxt, pvalue, &timeVal, FALSE);
      if (stat == ASN_E_NOTCANON)
      {
         /* Input is not canonical value. Get & encode canonical value. */
         char canonValue[100];
         char* value = canonValue;
         stat = rtMakeTime(pctxt, &timeVal, &value, sizeof(canonValue));
         if ( stat < 0 ) return LOG_RTERR(pctxt, stat);
         return xe_charstr(pctxt, canonValue, tagging, tag);
      }
      else if ( stat < 0 ) return LOG_RTERR(pctxt, stat);
   }

   return xe_charstr(pctxt, pvalue, tagging, tag);
}
