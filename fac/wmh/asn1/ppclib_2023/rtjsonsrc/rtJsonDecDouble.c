/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include <math.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxReal.h"
#include "rtjsonsrc/osrtjson.h"

/* these are not used..
static const char nanStr[3] = { 'N', 'a', 'N' };
static const char infStr[3] = { 'I', 'N', 'F' };
*/
int rtJsonDecDouble (OSCTXT* pctxt, OSREAL* pvalue)
{
   OSBOOL minus = FALSE;
   OSREAL value = 0.;
   OSREAL fracPos = 0.1;
   OSINT32 expVal = 0;
   OSBOOL expMinus = FALSE;
   int pos = 0;
   int stat = 0;
   OSOCTET ub;
   OSBOOL end = FALSE;

   int mode = 0;

   if (pvalue)
      *pvalue = 0.;

   rtJsonDecSkipWhitespace (pctxt);

   for (;;) {
      stat = rtxReadBytes (pctxt, &ub, 1);
      if (stat == RTERR_ENDOFBUF) break;
      else if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (ub == ',' || ub == ']' || ub == '}') {
         /* Push character back to be reread again */
         OSRTASSERT (pctxt->buffer.byteIndex > 0);
         pctxt->buffer.byteIndex--;
         break;
      }

      /* Read and process numeric digit characters.  End of input is assumed
      to be first invalid char */
      /* 0 - sign, 1 - detect, 2 - int, 3 - frac,
         4 - expSign, 5 - exp, 6 - INF, 7 - NaN */
      switch (mode) {
      case 0: /* sign */
         if (ub == '+') {
            mode = 1;
         }
         else if (ub == '-') {
            minus = TRUE;
            mode = 1;
         }
         else if (ub == 'N') {
            pos++;
            mode = 7;
         }
         else if (ub == 'I') {
            pos++;
            mode = 6;
         }
         else if (OS_ISDIGIT (ub)) {
            OSINT32 tm = ub - '0';
            value = value * 10. + tm;
            mode = 2;
         }
         else if (ub == '.') {
            mode = 3;
         }
         else {
            return LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
         }
         break;
      case 1: /* detect */
         if (ub == 'I') {
            pos++;
            mode = 6;
         }
         else if (OS_ISDIGIT (ub)) {
            OSINT32 tm = ub - '0';
            value = value * 10. + tm;
            mode = 2;
         }
         else if (ub == '.') {
            mode = 3;
         }
         else {
            /* Push character back to be reread again */
            OSRTASSERT (pctxt->buffer.byteIndex > 0);
            pctxt->buffer.byteIndex--;
            end = TRUE;
         }
         break;
      case 2: /* integer */
         if (OS_ISDIGIT (ub)) {
            OSINT32 tm = ub - '0';
            value = value * 10. + tm;
         }
         else if (ub == '.') {
            mode = 3;
         }
         else if (ub == 'E' || ub == 'e') {
            mode = 4;
         }
         else {
            /* Push character back to be reread again */
            OSRTASSERT (pctxt->buffer.byteIndex > 0);
            pctxt->buffer.byteIndex--;
            end = TRUE;
         }
         break;
      case 3: /* fractional */
         if (OS_ISDIGIT (ub)) {
            OSINT32 tm = ub - '0';
            value += tm * fracPos;
            fracPos *= 0.1;
         }
         else if (ub == 'E' || ub == 'e') {
            mode = 4;
         }
         else {
            /* Push character back to be reread again */
            OSRTASSERT (pctxt->buffer.byteIndex > 0);
            pctxt->buffer.byteIndex--;
            end = TRUE;
         }
         break;
      case 4: /* expSign */
         if (ub == '+') {
            mode = 5;
         }
         else if (ub == '-') {
            expMinus = TRUE;
            mode = 5;
         }
         else if (OS_ISDIGIT (ub)) {
            /* Push character back to be reread again */
            OSRTASSERT (pctxt->buffer.byteIndex > 0);
            pctxt->buffer.byteIndex--;
            mode = 5;
         }
         else {
            /* Push character back to be reread again */
            OSRTASSERT (pctxt->buffer.byteIndex > 0);
            pctxt->buffer.byteIndex--;
            end = TRUE;
         }
         break;
      case 5: /* exp */
         expVal = expVal * 10 + (ub - '0');
         if (expVal > 325) {
            /* in double ~ 15 digits ( limit 309 + 15 ) */
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
         }
         break;
      case 6: /* INF */
           return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
/* YW: NaN/INF/-INF is not supported in JSON */
/*         if (pos >= 3 || ub != infStr[pos]) {
            return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
         }

         pos++;
         break;
*/
      case 7: /* NaN */
           return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
/* YW: NaN/INF/-INF is not supported in JSON */
/*         if (pos >= 3 || ub != nanStr[pos]) {
            return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
         }

         pos++;
         break;
*/
      }
      if (end) break;
   }

   if (stat >= 0) {
      stat = 0;
      /* If negative, negate number */
      if (mode == 6) {
         return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
/* YW: NaN/INF/-INF is not supported in JSON */
/*         if (minus)
            value = rtxGetMinusInfinity ();
         else
            value = rtxGetPlusInfinity ();
*/
      }
      else if (mode == 7) {
         return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
/* YW: NaN/INF/-INF is not supported in JSON */
/*         value = rtxGetNaN (); */
      }
      else {
         if (minus)
            value = -value;

         if (expMinus)
            expVal = -expVal;

         if (expVal != 0)
            value *= pow (10., expVal);

         if (minus) {
            if (rtxIsMinusInfinity (value)) {
               return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
            }
         }
         else if (rtxIsPlusInfinity (value)) {
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
         }
      }

      if (pvalue)
         *pvalue = value;
   }

   return stat;
}


