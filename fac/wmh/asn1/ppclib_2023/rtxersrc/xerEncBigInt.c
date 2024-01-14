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

#include "xee_common.hh"
#include "rtxsrc/rtxBigInt.h"

int xerEncBigInt (OSCTXT* pctxt, const char* value, const char* elemName)
{
   int stat, radix = 10;
   char str[1000];
   char* pval = (char*) value;

   if (0 == elemName) elemName = "INTEGER";

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   if (pval[0] == '0') {
      if (pval[1] == 'x') { /* hexadecimal string */
         pval += 2;
         radix = 16;
      }
      else if (pval[1] == 'b') { /* binary string */
         pval += 2;
         radix = 2;
      }
      else if (pval[1] == 'o') { /* octal string */
         pval += 2;
         radix = 8;
      }
   }

   if (radix != 10) { /* convert to decimal */
      OSBigInt bi;
      OSOCTET magbuf[512];
      OSSIZE len;

      rtxBigIntInit (&bi);
      bi.mag = (OSOCTET*) magbuf;
      bi.allocated = sizeof (magbuf);

      if ((stat = rtxBigIntSetStr (pctxt, &bi, pval, radix)) != 0)
         return LOG_RTERR (pctxt, stat);

      len = rtxBigIntDigitsNum (&bi, 10);
      if (len > sizeof (str))
         pval = (char*) rtxMemAlloc (pctxt, len + 1);
      else
         pval = (char*) str;

      if (pval != 0) {
         if ((stat = rtxBigIntToString (pctxt, &bi, 10, pval, len + 1)) != 0)
            LOG_RTERR (pctxt, stat);
      }
      else
         stat = LOG_RTERR (pctxt, RTERR_NOMEM);

      rtxBigIntFree (pctxt, &bi);
      if (pval != (char*) str)
         rtxMemFreePtr (pctxt, pval);

      if (stat != 0) return  stat;
   }
   else {
      char* pval2;

      if (*pval == '-') {  /* treat minus sign */
         if ((stat = xerCopyText (pctxt, "-")) != 0)
            return LOG_RTERR (pctxt, stat);
         pval ++;
      }

      /* remove leading zeros */
      while (*pval == '0') pval++;

      pval2 = pval;

      /* check the decimal number's integrity */
      while (*pval2 != '\0') {
         if (!OS_ISDIGIT (*pval2))
         	return LOG_RTERR (pctxt, RTERR_BADVALUE);
         pval2 ++;
      }
   }
   stat = xerCopyText (pctxt, pval);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
