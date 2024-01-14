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

int xmlEncReal (OSCTXT* pctxt, OSREAL value,
                const char* elemName)
{
   char lbuf[256], lbuf2[256], *psrc = (char*)lbuf, *pdst, *psrc2;
   int  stat;
   size_t len;

   if (0 == elemName) elemName = "REAL";

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   if (rtxIsMinusZero (value)) {
      stat = xerCopyText (pctxt, "-0");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsPlusInfinity (value)) {
      stat = xerCopyText (pctxt, "INF");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsMinusInfinity (value)) {
      stat = xerCopyText (pctxt, "-INF");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsNaN (value)) {
      stat = xerCopyText (pctxt, "NaN");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (value == 0.0) {
      stat = xerCopyText (pctxt, "0");
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      char formatStr[10];

      sprintf (formatStr, "%%1.20E");
      sprintf (lbuf, formatStr, value); /* specify maximum precision after point */

      psrc = strchr (lbuf, ','); /* look for decimal point, if it is comma */
      if (psrc != NULL)
         *psrc = '.';            /* replace by period */
      psrc2 = psrc = strchr (lbuf, 'E');
      while (*(psrc2 - 1) == '0') /* skip all trailing zeros in fractional part */
         psrc2--;
      if (*(psrc2 - 1) == '.') psrc2++; /* ...if fractional part is zero, leave one zero in it */
      strncpy (lbuf2, lbuf, psrc2 - (char*)lbuf);  /* copy the first part before 'E' and exp sign */
      pdst = (char*)lbuf2 + (psrc2 - (char*)lbuf); /* set dest ptr to the end of buffer */
      psrc++; /* skip 'E' */
      psrc2 = psrc + 1;                            /* move src ptr behind 'E' and sign */
      /* skip all leading zeros in exponent */
      while (*psrc2 == '0') psrc2++;

      if (*psrc2 != '\0') {
         *pdst++ = 'E'; /* put E */

         if (*psrc == '-')
            *pdst++ = '-'; /* if exp is negative, put '-' sign */

         len = strlen (lbuf);
         strncpy (pdst, psrc2, len - (psrc2 - (char*)lbuf)); /* copy an exponent part */
         *(pdst + (len - (psrc2 - (char*)lbuf))) = '\0';
      }
      else {
         *pdst++ = 'E'; /* put E */
         strcpy (pdst, "0"); /* exponent is zero */
      }
      stat = xerCopyText (pctxt, lbuf2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}
