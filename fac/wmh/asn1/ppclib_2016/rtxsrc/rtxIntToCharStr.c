/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"

#ifndef XXFUNC
#define XXFUNC rtxIntToCharStr
#define XXTYPE OSINT32
#endif

int XXFUNC (XXTYPE value, char* dest, size_t bufsiz, char padchar)
{
   OSUINT32 nchars = 0;
   XXTYPE  rem, tmpval = value;
   OSINT32 i;
#ifndef XXUNSIGNED
   OSBOOL  minus = (OSBOOL)(value < 0);
#endif
   if (dest == 0 || bufsiz < 2) return RTERR_INVPARAM;

   /* Compute number of digits */
   if (value == 0) nchars = 1;
   else {
      while (tmpval != 0) {
         tmpval /= 10L;
         nchars++;
      }
#ifndef XXUNSIGNED
      if (value < 0) {
         nchars++; /* for '-' sign */
         /* Note: do not try to make value positive; */
         /* if value == MIN (e.g. INT_MIN), you can't. */
         /* It is better to just handle how % deals with the negative value */
         /* as is done below. */
      }
#endif
   }

   if (nchars >= bufsiz) return RTERR_BUFOVFLW;

   i = (padchar == '\0') ? (OSINT32)nchars : (OSINT32)(bufsiz - 1);
   nchars = (OSUINT32) i;
   dest[i--] = '\0'; /* set null-terminator */

   if (0 == value) {
      dest[i--] = '0';
   }
   else {
      tmpval = value;
      while (tmpval != 0) {
         rem = tmpval % 10L;
#ifndef XXUNSIGNED
         if ( minus ) {
            /*  % operator varies by implementation. */
            /*  Implementations giving a negative remainder give the digit we */
            /*  want, but with the wrong sign. */
            /*  Implementations giving a positive remainder give us x where */
            /*  x + correct digit = 10 */
            if ( rem > 0 ) rem = 10 - rem;
            else if ( rem < 0 ) rem = -rem;
         }
#endif
         dest[i--] = (char)(rem + '0');
         tmpval /= 10L;
      }
   }

#ifndef XXUNSIGNED
   if (minus && padchar != '0') dest[i--] = '-';
#endif
   if (padchar != '\0') {
      while (i >= 0) dest[i--] = padchar;
   }
#ifndef XXUNSIGNED
   if (minus && padchar == '0') dest[0] = '-';
#endif

   return (int)nchars;
}
