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

#include "rtbersrc/xse_common.hh"
#include "rtxsrc/rtxBigInt.h"

int berEncStrmBigInt (OSCTXT* pctxt,
                      const char* pvalue,
                      ASN1TagType tagging)
{
   if (0 == pvalue || *pvalue == 0)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   return berEncStrmBigIntNchars
      (pctxt, pvalue, OSCRTLSTRLEN(pvalue), tagging);
}

int berEncStrmBigIntNchars
(OSCTXT* pctxt, const char* pvalue, size_t nchars, ASN1TagType tagging)
{
   int stat, nbytes;
   size_t idx = 0;

   if (0 == pvalue || *pvalue == 0)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   if (tagging == ASN1EXPL) {
      stat = berEncStrmTag (pctxt, TM_UNIV|TM_PRIM|ASN_ID_INT);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Verify 1st two chars of string are "0x" or "0b" */

   if ((nchars == 1 && pvalue[0] == '0') ||
       (nchars > 2 && pvalue[0] == '0' &&
        (pvalue[1] == 'x' || pvalue[1] == 'b')))
   {
      /* do fast encoding for radices 2 and 16 */

      OSOCTET ub, val = 1;
      int i, numDig;
      OSBOOL positive = FALSE;
      int len = (int) nchars;

      /* default values for hexadecimal case */
      OSOCTET maxDig = 0xF;
      int shiftShift = 4, perByte = 2;

      if (nchars > 2) {
         len -= 2;

         /* set some variables according to radix */

         if (pvalue[1] == 'b') { /* binary */
            shiftShift = 1;
            perByte = 8;
            maxDig = 0x1;
         }
         idx += 2;

         /* determine is the number positive or negative */

         if ((len & (perByte - 1)) != 0)
            positive = TRUE;
         else {
            HEXCHARTONIBBLE (pvalue[idx], ub);
            if (!(ub & (OSINTCONST(1) << (shiftShift - 1))))
               positive = TRUE;
         }

         /* skip all leading zeros in positive number, except one */

         if (positive) {
            for (; pvalue[idx + 1] == '0'; idx++, len--)
               ;
            nbytes = (int)(len + perByte - 1) / perByte;
         }
         else { /* negative case */

            /* skip all 0xFFs (or '11111111') */

            while (idx < nchars) {
               for (i = 0; i < perByte; i++) {
                  HEXCHARTONIBBLE (pvalue[idx+i], ub);
                  if (ub != maxDig)
                     break;
               }
               if (i != perByte)
                  break;

               idx += i;
               len -= i;
            }

            nbytes = (int)(len + perByte - 1) / perByte;
            if (nbytes == 0) { /* if all octets are skipped, add one 0xFF */
               nbytes++;
               val = 0xFF;
            }
            else {

               /* if negative and most significant bit of next octet is not 1
                * add the 0xFF prefix. */

               HEXCHARTONIBBLE (pvalue[idx], ub);
               if (ub > maxDig)
                  return LOG_RTERR (pctxt, RTERR_BADVALUE);

               if (!(ub & (OSINTCONST(1) << (shiftShift - 1)))) {
                  nbytes++;
                  val = 0xFF;
               }
            }
         }
      }
      else { /* special case for zero */
         val = 0;
         idx++;
         nbytes = 1;
      }

      if ((stat = berEncStrmLength (pctxt, nbytes)) < 0)
         return LOG_RTERR (pctxt, stat);

      if (val != 1) {  /* encode first 0x00/0xFF octet if necessary */
         stat = berEncStrmWriteOctet (pctxt, val);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      val = 0;
      numDig = perByte - (int)(len & (perByte - 1));

      /* encode significant part of number */

      for (; idx < nchars; idx++) {
         HEXCHARTONIBBLE (pvalue[idx], ub);
         if (ub > maxDig)
            return LOG_RTERR (pctxt, RTERR_BADVALUE);

         val = (OSOCTET)((val << shiftShift) | ub);
         numDig++;

         if ((numDig & (perByte - 1)) == 0) {
            stat = berEncStrmWriteOctet (pctxt, val);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            val = 0;
         }
      }
   }
   else {

      /* encode decimal or octadecimal strings */

      OSOCTET buf[512], *p = (OSOCTET*)buf;
      OSBigInt bi;
      OSOCTET magbuf[512];

      rtxBigIntInit (&bi);
      bi.mag = (OSOCTET*) magbuf;
      bi.allocated = sizeof (magbuf);

      if ((stat = rtxBigIntSetStrn (pctxt, &bi, pvalue, nchars, 0)) != 0)
         return LOG_RTERR (pctxt, stat);

      nbytes = (int)rtxBigIntGetDataLen (&bi);

      stat = berEncStrmLength (pctxt, nbytes);

      if (stat >= 0 && nbytes > (int)sizeof (buf)) {
         p = (OSOCTET*) rtxMemAlloc (pctxt, nbytes);
         if (p == 0) stat = RTERR_NOMEM;
      }

      /* Get big integer bytes */
      if (stat >= 0)
         stat = rtxBigIntGetData (pctxt, &bi, p, nbytes);

      if (stat >= 0)
         stat = berEncStrmWriteOctets (pctxt, p, nbytes);

      /* Free memory if it was allocated */
      rtxBigIntFree (pctxt, &bi);

      if (nbytes > (int)sizeof (buf))
         rtxMemFreePtr (pctxt, p);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

