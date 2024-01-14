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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxBigInt.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xe_bigint
 *
 *  Description:  This routine encodes an ASN.1 universal integer field
 *                from a large (> 32 bits) integer value.  These integer
 *                values are represented as a string of hex characters.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to ASN.1 context block structure.
 *  object      char*   String representation of value to be encoded.
 *  tagging     enum    Explicit or implicit tagging specification.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_bigint (OSCTXT* pctxt, const char* pvalue, ASN1TagType tagging)
{
   if (0 == pvalue || *pvalue == 0)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   return xe_bigintn
      (pctxt, pvalue, OSCRTLSTRLEN(pvalue), tagging);
}

int xe_bigintn
(OSCTXT* pctxt, const char* object_p, size_t nchars, ASN1TagType tagging)
{
   int aal, stat;
   size_t len = nchars;

   if (0 == object_p || *object_p == 0)
      return LOG_RTERR (pctxt, RTERR_BADVALUE);

   /* Verify 1st two chars of string are "0x" or "0b" */

   if ((len == 1 && object_p[0] == '0') ||
       (len > 2 && object_p[0] == '0' &&
        (object_p[1] == 'x' || object_p[1] == 'b')))
   {
      /* do fast encoding for radices 2 and 16 */

      OSOCTET ub, val = 0, lastVal = 0, maxDig;
      size_t i, shift = 0, shiftShift, perByte, end = 2, numDig;
      OSBOOL positive = FALSE;

      if (len > 2) {

         /* set some variables according to radix */

         if (object_p[1] == 'b') { /* binary */
            shiftShift = 1;
            perByte = 8;
            maxDig = 0x1;
         }
         else {                    /* hexadecimal */
            shiftShift = 4;
            perByte = 2;
            maxDig = 0xF;
         }

         /* determine is the number positive or negative */

         if (((len - 2) & (perByte - 1)) != 0)
            positive = TRUE;
         else {
            HEXCHARTONIBBLE (object_p[2], ub);
            if (!(ub & (OSINTCONST(1) << (shiftShift - 1))))
               positive = TRUE;
         }

         /* skip all leading zeros in positive number */

         if (positive) {
            for (; end < len && object_p[end] == '0'; end++)
               ;
         }
         else { /* negative case */

            /* skip all 0xFFs (or '11111111') */

            while (end < len) {
               for (i = 0; i < perByte; i++) {
                  HEXCHARTONIBBLE (object_p[end + i], ub);
                  if (ub != maxDig)
                     break;
               }
               if (i != perByte)
                  break;
               end += i;
            }
         }

         /* perform encoding of significant part */

         for (i = len - 1, aal = 0, numDig = 0; i >= end; i--) {
            HEXCHARTONIBBLE (object_p[i], ub);
            if (ub > maxDig)
               return LOG_RTERR (pctxt, RTERR_BADVALUE);

            val |= (ub << shift);
            shift += shiftShift;

            numDig++;
            if ((numDig & (perByte - 1)) == 0) {
               if (pctxt->buffer.byteIndex == 0)
                  if ((stat = xe_expandBuffer(pctxt, 1)) != 0)
                     return LOG_RTERR (pctxt, stat);

               pctxt->buffer.byteIndex--;

               ASN1BUFCUR(pctxt) = val;
               lastVal = val;
               val = 0;
               shift = 0;
               aal++;
            }
         }
      }
      else { /* special zero case */
         positive = TRUE;
         numDig = aal = 0;
         perByte = 1;
      }

      /* if the last patial value should be stored */

      if ((numDig & (perByte - 1)) != 0) {
         if (pctxt->buffer.byteIndex == 0)
            if ((stat = xe_expandBuffer(pctxt, 1)) != 0)
               return LOG_RTERR (pctxt, stat);
         pctxt->buffer.byteIndex--;

         ASN1BUFCUR(pctxt) = val;
         aal++;
      }
      else {

         /* encode prefix (0 or 0xFF) if necessary */

         do {
            if (positive && (numDig == 0 || (lastVal & 0x80))) {
               val = 0;
            }
            else if (!positive && (numDig == 0 || !(lastVal & 0x80))) {
               val = 0xFF;
            }
            else break;

            if (pctxt->buffer.byteIndex == 0)
               if ((stat = xe_expandBuffer(pctxt, 1)) != 0)
                  return LOG_RTERR (pctxt, stat);
            pctxt->buffer.byteIndex--;

            ASN1BUFCUR(pctxt) = val;
            aal++;
         } while (0);
      }
   }
   else {
      OSBigInt bi;
      OSOCTET magbuf[512];
      const char* str = object_p;

      rtxBigIntInit (&bi);
      bi.mag = (OSOCTET*) magbuf;
      bi.allocated = sizeof (magbuf);

      if ((stat = rtxBigIntSetStr (pctxt, &bi, str, 0)) != 0)
         return LOG_RTERR (pctxt, stat);

      aal = (int)rtxBigIntGetDataLen (&bi);
      if ((int)pctxt->buffer.byteIndex - aal - 1 <= 0)
         if ((stat = xe_expandBuffer(pctxt, aal)) != 0)
            return LOG_RTERR (pctxt, stat);

      pctxt->buffer.byteIndex -= aal;
      if ((stat = rtxBigIntGetData (pctxt, &bi, ASN1BUFPTR (pctxt), aal)) < 0)
         return LOG_RTERR (pctxt, stat);

      rtxBigIntFree (pctxt, &bi);
   }

   if (tagging == ASN1EXPL)
      aal = xe_tag_len (pctxt, TM_UNIV|TM_PRIM|ASN_ID_INT, aal);

   LCHECKBER (pctxt);

   return (int)(aal);
}

