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


int xe_datetimestr(OSCTXT* pctxt, const char* pvalue,
                   ASN1TagType tagging, ASN1TAG tag)
{
   OSUINT32 i, j = 0;
   char *p = NULL;
   const char *pval = pvalue;
   int stat = 0;

   if (NULL == pvalue)
   {
      return 0;
   }

   stat = berValidateIso8601DateStr(pctxt, &pval);
   if (0 != stat)
   {
      return LOG_RTERR(pctxt, stat);
   }

   // Date-time string must have a "T" separator...
   if (*pval == 'T')
   {
      pval++;
   }
   else
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = berValidateIso8601TimeStr(pctxt, &pval);
   if (0 != stat)
   {
      return LOG_RTERR(pctxt, stat);
   }

   // If we get here, the date-time string is valid.  Walk through it and pull
   //  out hyphen and colon characters, and the "T" separator...
   p = (char *)malloc(strlen(pvalue) + 1);
   for (i = 0; i < strlen(pvalue); i++)
   {
      if ((pvalue[i] != '-') && (pvalue[i] != ':') && (pvalue[i] != 'T'))
      {
         p[j++] = pvalue[i];
      }
   }
   p[j] = '\0';

   stat = xe_charstr(pctxt, p, tagging, tag);

   free(p);
   return stat;
}
