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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxBigInt.h"

/***********************************************************************
 *
 *  Routine name: pe_BigInteger
 *
 *  Description:  This function encodes big (> 32 bit) integers suing PER.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pvalue      char*   Pointer to big integer string to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat         int    Completion status
 *
 **********************************************************************/

EXTPERMETHOD int pe_BigInteger (OSCTXT* pctxt, const char* pvalue)
{
   int stat;
   size_t nbytes, len;
   const char* str = pvalue;

   if (0 == pvalue || *pvalue == 0)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   /* Verify 1st two chars of string are "0x" or "0b" */

   len = strlen (pvalue);
   if ((len == 1 && pvalue[0] == '0') ||
      (len > 2 && pvalue[0] == '0' && (pvalue[1] == 'x' || pvalue[1] == 'b')))
   {
      /* do fast encoding for radices 2 and 16 */

      OSOCTET ub, val = 1, maxDig = 0;
      int numDig, shiftShift = 0;
      unsigned i, perByte = 0;
      OSBOOL positive = FALSE;

      if (len > 2) {
         /* set some variables according to radix */

         if (pvalue[1] == 'b') { /* binary */
            shiftShift = 1;
            perByte = 8;
            maxDig = 0x1;
         }
         else {                  /* hexadecimal */
            shiftShift = 4;
            perByte = 2;
            maxDig = 0xF;
         }
         str += 2;
         len -= 2;

         /* determine is the number positive or negative */

         if ((len & (perByte - 1)) != 0)
            positive = TRUE;
         else {
            HEXCHARTONIBBLE (*str, ub);
            if (!(ub & (OSINTCONST(1) << (shiftShift - 1))))
               positive = TRUE;
         }

         /* skip all leading zeros in positive number, except one */

         if (positive) {
            for (; *(str + 1) == '0'; str++, len--)
               ;
            nbytes = (len + perByte - 1) / perByte;
         }
         else { /* negative case */

            /* skip all 0xFFs (or '11111111') */

            while (*str != 0) {
               for (i = 0; i < perByte; i++) {
                  HEXCHARTONIBBLE (str[i], ub);
                  if (ub != maxDig)
                     break;
               }
               if (i != perByte)
                  break;
               str += i;
               len -= i;
            }

            nbytes = (len + perByte - 1) / perByte;
            if (nbytes == 0) { /* if all octets are skipped, add one 0xFF */
               nbytes++;
               val = 0xFF;
            }
            else {

               /* if negative and most significiant bit of next octet is not 1
                * add the 0xFF prefix. */

               HEXCHARTONIBBLE (*str, ub);
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
         str++;
         nbytes = 1;
      }

      if ((stat = pe_Length (pctxt, (OSUINT32)nbytes)) < 0)
         return LOG_RTERR (pctxt, stat);

      PU_NEWFIELD (pctxt, "BigInteger");

      if (pctxt->buffer.aligned) {
         if ((stat = pe_byte_align (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      /* Check buffer space and allocate more memory if necessary */

      stat = pe_CheckBuffer (pctxt, nbytes + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (val != 1) {  /* encode first 0x00/0xFF octet if necessary */
         stat = pe_octet (pctxt, val);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      val = 0;
      numDig = perByte - (len & (perByte - 1));

      /* encode significiant part of number */

      for (; *str != 0; str++) {
         HEXCHARTONIBBLE (*str, ub);
         if (ub > maxDig)
            return LOG_RTERR (pctxt, RTERR_BADVALUE);

         val = (OSOCTET)((val << shiftShift) | ub);
         numDig++;

         if ((numDig & (perByte - 1)) == 0) {
            stat = pe_octet (pctxt, val);
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

      if ((stat = rtxBigIntSetStr (pctxt, &bi, str, 0)) != 0)
         return LOG_RTERR (pctxt, stat);

      nbytes = rtxBigIntGetDataLen (&bi);

      do {
         if ((stat = pe_Length (pctxt, (OSUINT32)nbytes)) < 0) {
            LOG_RTERR (pctxt, stat);
            break;
         }

         PU_NEWFIELD (pctxt, "BigInteger");

         if (pctxt->buffer.aligned) {
            if ((stat = pe_byte_align (pctxt)) != 0) {
               stat = LOG_RTERR (pctxt, stat);
               break;
            }
         }

         /* Check buffer space and allocate more memory if necessary */

         stat = pe_CheckBuffer (pctxt, nbytes + 1);
         if (stat != 0) {
            stat = LOG_RTERR (pctxt, stat);
            break;
         }

         if (nbytes > sizeof (buf)) {
            p = (OSOCTET*) rtxMemAlloc (pctxt, nbytes);
            if (p == NULL) {
               stat = LOG_RTERR (pctxt, RTERR_NOMEM);
               break;
            }
         }

         /* Get big integer bytes */
         if ((stat = rtxBigIntGetData (pctxt, &bi, p, nbytes)) < 0) {
            stat = LOG_RTERR (pctxt, stat);
            break;
         }

         if ((stat = pe_octets (pctxt, p, (OSUINT32)(nbytes * 8))) != 0)
         {
            stat = LOG_RTERR (pctxt, stat);
            break;
         }
      } while (0);

      /* Free memory if it was allocated */
      rtxBigIntFree (pctxt, &bi);

      if (p != (OSOCTET*)buf)
         rtxMemFreePtr (pctxt, p);

      if (stat != 0) return  stat;
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}

