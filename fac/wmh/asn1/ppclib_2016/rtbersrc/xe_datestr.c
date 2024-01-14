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
#include "rtxsrc/rtxCtype.h"


EXTERNBER int berValidateIso8601DateStr(OSCTXT* pctxt, const char** ppvalue)
{
   OSUINT8 digit_count = 0;
   OSBOOL week = FALSE;

   // First character must be "+", "-", or a digit...
   if ((**ppvalue == '-') || (**ppvalue == '+'))
   {
      (*ppvalue)++;
   }
   else if (!OS_ISDIGIT(**ppvalue))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   // Walk through year...
   while (OS_ISDIGIT(**ppvalue))
   {
      (*ppvalue)++;
      digit_count++;
   }

   if ((**ppvalue == 'C') && (digit_count == 2) && (*(*ppvalue + 1) == '\0'))
   {
      // Valid Century string, so return ok...
      return 0;
   }
   else if ((digit_count != 4) || ((**ppvalue != '-') && (**ppvalue != '\0')))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (**ppvalue == '\0')
   {
      // Date only contained a year...
      return 0;
   }
   (*ppvalue)++;
   digit_count = 0;

   // Check for "W"....
   if (**ppvalue == 'W')
   {
      week = TRUE;
      (*ppvalue)++;
   }

   // Walk through month/week...
   while (OS_ISDIGIT(**ppvalue))
   {
      (*ppvalue)++;
      digit_count++;
   }

   if (week && (digit_count != 2))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   else if ((digit_count < 2) || (digit_count > 3))
   {
      // Month = 2 digits; ordinal day = 3 digits...
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if ((**ppvalue == '\0') || (**ppvalue == 'T'))
   {
      return 0;
   }
   else if (**ppvalue != '-')
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   (*ppvalue)++;
   digit_count = 0;

   // Walk through day/day-of-week...
   while (OS_ISDIGIT(**ppvalue))
   {
      (*ppvalue)++;
      digit_count++;
   }

   if ((digit_count < 1) || (digit_count > 2))
   {
      // day-of-week = 1 digit; day = 2 digits...
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   else if ((**ppvalue != '\0') && (**ppvalue != 'T') && (**ppvalue != '/'))
   {
      // Should be at end of string, or "T" separator, or a "/" character
      //  signifying the next part of a Duration string...
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   return 0;
}

int xe_datestr(OSCTXT* pctxt, const char* pvalue,
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

   // If we get here, the date string is valid.  Walk through it and pull out
   //  hyphen characters...
   p = (char *)malloc(strlen(pvalue) + 1);
   for (i = 0; i < strlen(pvalue); i++)
   {
      if (pvalue[i] != '-')
      {
         p[j++] = pvalue[i];
      }
   }
   p[j] = '\0';

   stat = xe_charstr(pctxt, p, tagging, tag);

   free(p);
   return stat;
}
