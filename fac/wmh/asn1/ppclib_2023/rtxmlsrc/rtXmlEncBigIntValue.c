/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxBigInt.h"
#include "rtxsrc/rtxErrCodes.h"

EXTXMLMETHOD int rtXmlEncBigIntValue (OSCTXT* pctxt, const OSUTF8CHAR* value)
{
   int stat = 0, radix = 10;
   char str[1000], *pdestval;
   const OSUTF8CHAR* pval = value;

   if (value == 0 || *value == 0) {
      return 0;
   }
   else {
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
         OSSIZE  len;

         rtxBigIntInit (&bi);
         bi.mag = (OSOCTET*) magbuf;
         bi.allocated = sizeof (magbuf);

         stat = rtxBigIntSetStr (pctxt, &bi, (const char*)pval, radix);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         len = rtxBigIntDigitsNum (&bi, 10);
         if (len < sizeof(str))
            pdestval = (char*) str;
         else
            pdestval = (char*) rtxMemAlloc (pctxt, len + 1);

         if (pdestval != 0) {
            stat = rtxBigIntToString (pctxt, &bi, 10, pdestval, len + 1);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         else
            stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);

         rtxBigIntFree (pctxt, &bi);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)pdestval,
                                 OSCRTLSTRLEN((const char*)pdestval));
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         if (pdestval != (char*) str)
            rtxMemFreePtr (pctxt, pdestval);
      }
      else {
         const OSUTF8CHAR* pval2;

         if (*pval == '-') {  /* treat minus sign */
            if ((stat = rtXmlPutChar (pctxt, '-')) != 0)
               return LOG_RTERR (pctxt, stat);
            pval ++;
         }
         else if (*pval == '+') /* treat plus sign */
            pval ++;

         /* remove leading zeros */
         while (*pval == '0' && *(pval + 1) != '\0') pval++;

         pval2 = pval;

         /* check the decimal number's integrity */
         while (*pval2 != '\0') {
            if (!OS_ISDIGIT (*pval2))
               return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
            pval2 ++;
         }

         stat = rtXmlWriteChars (pctxt, pval, OSCRTLSTRLEN((const char*)pval));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      if (pctxt->state != OSXMLATTR)
         pctxt->state = OSXMLDATA;
   }

   return (stat);
}

