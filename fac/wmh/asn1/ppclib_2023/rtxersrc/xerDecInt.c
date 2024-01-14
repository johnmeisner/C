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

#include "xed_common.hh"

int xerDecInt (OSCTXT* pctxt, OSINT32* pvalue)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int i;
   OSBOOL minus = FALSE;
   register OSINT32 value = 0, prevVal = 0;

   *pvalue = 0;

   /* Consume leading white space */

   for (i = 0; inpdata[i] != 0; i++) {
      if (!OS_ISSPACE (inpdata[i])) break;
   }

   /* Check for '-' first character */

   if (inpdata[i] == '-') {
      minus = TRUE; i++;
   }

   /* Convert Unicode characters to an integer value */

   for ( ; inpdata[i] != 0; i++, prevVal = value) {
      if (OS_ISDIGIT (inpdata[i]))
         value = (value * 10) + (inpdata[i] - '0');
      else
         return LOG_RTERR (pctxt, RTERR_INVCHAR);
      if (value/10 != prevVal)  /* test for int overflow AB, 09/27/02 */
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }

   /* If negative, negate number */

   if (minus) *pvalue = 0 - value;
   else *pvalue = value;

   LCHECKXER (pctxt);

   return (0);
}

int xerDecInt8 (OSCTXT* pctxt, OSINT8* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = xerDecInt (pctxt, &itmp);
   if (stat == 0) {
      if (itmp < -128 || itmp > 127)
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      *pvalue = (OSINT8)itmp;
   }
   else return LOG_RTERR (pctxt, stat);
   return 0;
}

int xerDecInt16 (OSCTXT* pctxt, OSINT16* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = xerDecInt (pctxt, &itmp);
   if (stat == 0) {
      if (itmp < -32768 || itmp > 32767)
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      *pvalue = (OSINT16)itmp;
   }
   else return LOG_RTERR (pctxt, stat);
   return 0;
}

