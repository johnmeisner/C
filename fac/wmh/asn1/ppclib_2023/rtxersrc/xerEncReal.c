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
#include "rtxsrc/rtxReal.h"

#ifndef DECIMAL_DIG
/* Should be defined in float.h, but some don't */
#define DECIMAL_DIG 10
#endif

int xerEncReal (OSCTXT* pctxt, OSREAL value,
                const char* elemName)
{
   char lbuf[256], lbuf2[256], *psrc = (char*)lbuf, *pdst, *psrc2;
   int  stat;
   size_t len;

   if (0 == elemName) elemName = "REAL";

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   if (value == 0.0) {
      stat = xerCopyText (pctxt, "0");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (value == rtxGetPlusInfinity ()) {
      stat = xerCopyText (pctxt, "<PLUS-INFINITY/>");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (value == rtxGetMinusInfinity ()) {
      stat = xerCopyText (pctxt, "<MINUS-INFINITY/>");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsNaN (value)) {
      stat = xerCopyText (pctxt, "<NOT-A-NUMBER/>");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /* specify maximum precision after point - 20.
         There is no reason to use more than DECIMAL_DIG - 1 digits in the
         fraction part.
      */
      sprintf (lbuf, "%1.*E", DECIMAL_DIG - 1, value);
      psrc = strchr (lbuf, ','); /* look for decimal point, if it is comma */
      if (psrc != NULL)
         *psrc = '.';            /* replace by period */
      psrc2 = psrc = strchr (lbuf, 'E');
      if (psrc2 != 0) {
         while (*(psrc2 - 1) == '0') /* skip all trailing zeros in fractional part */
            psrc2--;
         if (*(psrc2 - 1) == '.') psrc2++; /* ...if fractional part is zero, leave one zero in it */
         strncpy (lbuf2, lbuf, psrc2 - (char*)lbuf);  /* copy the first part before 'E' and exp sign */
         pdst = (char*)lbuf2 + (psrc2 - (char*)lbuf); /* set dest ptr to the end of buffer */
         psrc2 = psrc + 2;                            /* move src ptr behind 'E' and sign */
         /* skip all leading zeros in exponent */
         while (*psrc2 == '0') psrc2++;
         strncpy (pdst, psrc, 2); /* copy 'E+' or 'E-' */

         if (*psrc2 != '\0') {
            len = strlen (lbuf);
            strncpy (pdst + 2, psrc2, len - (psrc2 - (char*)lbuf)); /* copy an exponent part */
            *(pdst + 2 + (len - (psrc2 - (char*)lbuf))) = '\0';
         }
         else {
            strcpy (pdst + 2, "0"); /* exponent is zero */
         }
         stat = xerCopyText (pctxt, lbuf2);
      }
      else
         stat = xerCopyText (pctxt, "0"); /* if no exponent in the buffer then
                                           it may mean incorrect number (such
                                           as NaN). The question is: should we
                                           return error code or just encode as
                                           0? !AB 11/11/2003 */
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}
