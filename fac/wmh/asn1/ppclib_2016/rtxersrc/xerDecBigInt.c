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
#include "rtxsrc/rtxBigInt.h"

int xerDecBigInt (OSCTXT* pctxt, char **pvalue, int radix)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   char* pbuf;
   int stat;
   OSSIZE i, nchars = 0;

   /* Consume leading white space */

   while (*inpdata != 0 && OS_ISSPACE (*inpdata)) {
      inpdata ++;
   }

   /* Count characters */

   for (i = 0; inpdata[i] != 0 && !OS_ISSPACE (inpdata[i]); i++) {
      nchars++;
   }

   if (radix != 10) { /* need to perform conversion */
      OSBigInt bi;
      OSSIZE len;
      char str[512], *pstr;

      if (nchars > sizeof (str) - 1) {
         pstr = (char*)rtxMemAlloc (pctxt, nchars + 1);
         if (pstr == NULL)
            return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
      else
         pstr = (char*)str;

      if (pstr != 0) {
         for (i = 0; i < nchars; i++) {
            pstr[i] = (char)inpdata[i];
         }
         pstr[i] = '\0';

         rtxBigIntInit (&bi);
         if ((stat = rtxBigIntSetStr (pctxt, &bi, pstr, 10)) != 0)
            return LOG_RTERR (pctxt, stat);

         len = rtxBigIntDigitsNum (&bi, radix);

         pbuf = (char*)rtxMemAlloc (pctxt, len + 1 + 2);
         if (pbuf != 0) {

            switch (radix) {
               case 16: strcpy (pbuf, "0x"); break;
               case 8:  strcpy (pbuf, "0o"); break;
               case 2:  strcpy (pbuf, "0b"); break;
            }

            if ((stat = rtxBigIntToString
                         (pctxt, &bi, radix, pbuf + 2, len + 1)) != 0)
            {
               stat = LOG_RTERR (pctxt, stat);
               rtxMemFreePtr (pctxt, pbuf);
            }
         }
         else
            stat = LOG_RTERR (pctxt, RTERR_NOMEM);

         rtxBigIntFree (pctxt, &bi);

         if ((char*)str != pstr)
            rtxMemFreePtr (pctxt, pstr);

         if (stat != 0)
            return  stat;
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   else {
      /* Allocate memory for output data */

      pbuf = (char*)rtxMemAlloc (pctxt, nchars + 1);

      if (pbuf != 0) {
         /* treat first char */
         if ((char) *inpdata != '-' && !OS_ISDIGIT ((char) *inpdata))
            return LOG_RTERR_AND_FREE_MEM (pctxt, RTERR_BADVALUE, pbuf);

         *pbuf = (char) *inpdata;

         /* Convert Unicode characters to ASCII */
         for (i = 1; i < nchars; i++) {
            char c = (char) inpdata[i];
            if (!OS_ISDIGIT (c))
               return LOG_RTERR_AND_FREE_MEM (pctxt, RTERR_BADVALUE, pbuf);
            pbuf[i] = c;
         }

         pbuf[i] = '\0';
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   *pvalue = pbuf;
   return 0;
}

