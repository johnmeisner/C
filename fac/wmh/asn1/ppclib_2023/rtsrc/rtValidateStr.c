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

#include "asn1type.h"
#include "asn1CharSet.h"

static int helpValidateStr
(const char* str, OSSIZE nchars, const OSOCTET* charset, OSSIZE chSetSize,
 OSBOOL contig);

int rtValidateChars (ASN1TAG tag, const char *pdata, size_t nchars)
{
   if (!pdata)
   {
      // If the string is null, just return.  There's obviously nothing to
      //  validate in this case anyway, and worse, the strlen call in the
      //  helpValidateStr method below will segfault on a null string...
      return 0;
   }

   switch (tag) {
      case ASN_ID_NumericString:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)NUM_CANSET,
                                 sizeof(NUM_CANSET) - 1, FALSE);

      case ASN_ID_PrintableString:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)PRN_CANSET,
                                 sizeof(PRN_CANSET) - 1, FALSE);

      case ASN_ID_T61String:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)T61_CANSET,
                                 sizeof(T61_CANSET) - 1, FALSE);

      case ASN_ID_IA5String:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)IA5_CANSET,
                                 sizeof(IA5_CANSET) - 1, TRUE);

      case ASN_ID_VisibleString:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)VIS_CANSET,
                                 sizeof(VIS_CANSET) - 1, TRUE);

      case ASN_ID_GeneralString:
         return helpValidateStr (pdata, nchars, (const OSOCTET*)GEN_CANSET,
                                 sizeof(GEN_CANSET) - 1, TRUE);

      default: break;
   }

   return 0;
}

int rtValidateStr (ASN1TAG tag, const char *pdata)
{
   /* Note:  we assume that the check for lax processing has already been
    * done when this function is called.  We also assume the string is
    * null-terminated. */

   return rtValidateChars (tag, pdata, 0);
}

static int helpValidateStr
(const char* str, OSSIZE nchars, const OSOCTET* charset, OSSIZE chSetSize,
 OSBOOL contig)
{
   OSSIZE i;
   nchars = (0 == nchars || (OSSIZE)-1 == nchars) ?
      OSCRTLSTRLEN(str) : OS_MIN(nchars, OSCRTLSTRLEN(str));

   for (i = 0; i < nchars; i++) {
      OSOCTET c = (OSOCTET)*str++;

      /* If character set is contiguous, can check if character is within
         range of first and last character; otherwise, need to search set. */
      if (contig) {
         if (c >= charset[0] && c <= charset[chSetSize-1]) {
            continue;
         }
      }
      else if (memchr((void*)charset, c, chSetSize)) {
         continue;
      }

      return c;
   }

   return 0;
}
