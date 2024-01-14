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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxCommonDefs.h"

/***********************************************************************
 *
 *  Routine name: pd_BigIntegerEx
 *
 *  Description:  This function decodes big (> 32 bit) integer using PER.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ppvalue     char**  Pointer to value to receive decoded result
 *  radix       int     Number radix (2, 8, 10 or 16)
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Completion status of operation
 *
 **********************************************************************/

static const OSOCTET mult6[20] = {
   0, 0, 0, 6, 1, 2, 1, 8, 2, 4, 3, 0, 3, 6, 4, 2, 4, 8, 5, 4
};

EXTPERMETHOD int pd_BigIntegerValue
(OSCTXT *pctxt, const char** ppvalue, int radix, OSUINT32 nbytes)
{
   OSUINT32 i = 0;
   int bufsiz = 0, stat, ii;
   OSBOOL minus = FALSE;
   char* tmpstr;
   char* p;
   OSUINT32 ub = 0;
   char prefix = 0;
   unsigned dlen = 1;

   if (radix != 2 && radix != 8 && radix != 10 && radix != 16)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (nbytes > MAX_BIGINTBYTES)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* check number sign */

   if (nbytes > 0) {
      stat = pd_bits (pctxt, &ub, 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (ub & 0x80) minus = TRUE;

      if (ub == 0 && nbytes > 1)
         i = 1;
   }

   /* Allocate memory for character string representation of value */

   switch (radix) {
      case 2:
         prefix = 'b';
         bufsiz = (nbytes - i) * 8;
         break;
      case 8:
         prefix = 'o';
         bufsiz = ((nbytes - i) * 8 + 2) / 3;
         break;
      case 10:
         bufsiz = (nbytes - i) / 2 * 5;
         if (nbytes & 1)
            bufsiz += 3;
         break;
      case 16:
         prefix = 'x';
         bufsiz = (nbytes - i) * 2;
         break;
   }

   bufsiz += 4;
   tmpstr = (char*) rtxMemAlloc (pctxt, bufsiz);
   if (tmpstr == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);
   p = tmpstr;

   if (radix != 10) {
      *p++ = '0';
      *p++ = prefix;
      if (i > 0 || nbytes == 0)
         *p++ = '0'; /* leading zero */
   }
   else if (minus)
      *p++ = '-';

   switch (radix) {
      case 2:
         for (; i < nbytes; i++) {
            int j;

            if (i > 0) {
               stat = pd_bits (pctxt, &ub, 8);
               if (stat != 0)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);
            }

            for (j = 0; j < 8; j++) {
               *p++ = (ub & 0x80) ? '1' : '0';
               ub <<= 1;
            }
         }

         break;
      case 8:
         if (nbytes - i > 0) {
            OSUINT32 bits = 8;
            OSUINT32 bitOffs;
            OSUINT32 tm;

            bitOffs = (nbytes - i) * 8;
            bitOffs -= bitOffs / 3 * 3;

            if (i > 0) {
               stat = pd_bits (pctxt, &ub, 8);
               if (stat != 0)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);
            }

            tm = ub << bitOffs;
            bits -= bitOffs;

            for (; bits > 0 || i < nbytes;) {
               *p++ = (char) (((tm >> 8) & 7) + '0');

               if (bits >= 3) {
                  tm <<= 3;
                  bits -= 3;
               }
               else {
                  tm <<= bits;

                  i++;
                  if (i < nbytes) {
                     stat = pd_bits (pctxt, &ub, 8);
                     if (stat != 0)
                        return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);

                     tm |= ub;
                     tm <<= 3 - bits;
                     bits += 5;
                  }
               }
            }
         }
         break;
      case 10:
         if (minus)
            ub ^= 0xFF;

         OSCRTLMEMSET (p, 0, tmpstr + bufsiz - p);

         for (; i < nbytes; i++) {
            int k;

            if (i > 0) {
               stat = pd_bits (pctxt, &ub, 8);
               if (stat != 0)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);

               if (minus)
                  ub ^= 0xFF;
            }

            for (k = 0; k < 2; k++) {
               unsigned x0 = (unsigned) ((ub >> 4) & 0xF);
               unsigned x1 = 0;
               char* pd = tmpstr + bufsiz - 2;
               unsigned tm;
               unsigned carry;
               unsigned j;

               ub <<= 4;

               if (i == nbytes - 1 && k == 1 && minus)
                  x0++;

               if (x0 >= 10) {
                  x0 -= 10;
                  x1 = 1;
               }

               tm = *pd;
               *pd = (OSOCTET) (mult6[tm * 2 + 1] + x0);
               carry = mult6[tm * 2] + tm + x1;

               if (*pd >= 10) {
                  carry++;
                  *pd -= 10;
               }

               pd--;

               for (j = 1; j < dlen || carry > 0; j++) {
                  tm = *pd;
                  *pd = (OSOCTET) (mult6[tm * 2 + 1] + carry);
                  carry = mult6[tm * 2] + tm;

                  if (*pd >= 20) {
                     carry += 2;
                     *pd -= 20;
                  }
                  else if (*pd >= 10) {
                     carry++;
                     *pd -= 10;
                  }

                  pd--;
               }

               dlen = (int) (tmpstr + bufsiz - 2 - pd);
            }
         }

         { /* digits -> symbols */
            char* pe = tmpstr + bufsiz - 1;
            char* pd = pe - dlen;

            while (pd != pe) {
               *p++ = (char) (*pd++ + '0');
            }
         }

         break;
      case 16:
         for (; i < nbytes; i++) {
            if (i > 0) {
               stat = pd_bits (pctxt, &ub, 8);
               if (stat != 0)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);
            }

            ii = (ub >> 4) & 0x0f;
            NIBBLETOHEXCHAR (ii, *p++);

            ii = (ub & 0x0f);
            NIBBLETOHEXCHAR (ii, *p++);
         }

         break;
   }

   *p++ = '\0';
   *ppvalue = tmpstr;

   return 0;
}

EXTPERMETHOD int pd_BigIntegerEx
(OSCTXT *pctxt, const char** ppvalue, int radix)
{
   OSUINT32 nbytes;
   int stat;

   /* Decode length of value */

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   PU_NEWFIELD (pctxt, "BigInteger");

   if (pctxt->buffer.aligned) {
      if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
         return LOG_RTERR (pctxt, stat);
   }

   stat = pd_BigIntegerValue (pctxt, ppvalue, radix, nbytes);

   PU_SETBITCOUNT (pctxt);

   return stat;
}
