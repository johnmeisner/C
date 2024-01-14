/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

/* Decode a signed integer value. User must define the following items:

RTXDECUINTFN - Decode function name
RTXDECUINTTYPE - Integer type of value

and then include this template file.
*/
EXTRTMETHOD int RTXDECUINTFN
(OSCTXT* pctxt, RTXDECUINTTYPE* pvalue, OSSIZE nbytes)
{
   int stat;
   OSOCTET tmpbuf[sizeof(RTXDECUINTTYPE)+1];
   OSSIZE bufidx = 0;

   /* Make sure integer will fit in target variable */

   if (nbytes > (sizeof(tmpbuf)))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Read encoded integer contents into memory */

   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      stat = rtxReadBytes (pctxt, tmpbuf, nbytes);
   }
   else {
      stat = rtxDecBitsToByteArray (pctxt, tmpbuf, sizeof(tmpbuf), nbytes*8);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      if (nbytes == (sizeof(tmpbuf))) {
         /* first byte must be zero */
         if (0 != tmpbuf[0])
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         else
            bufidx++;
      }

      for (*pvalue = 0; bufidx < nbytes; bufidx++) {
         *pvalue = (*pvalue * 256) + tmpbuf[bufidx];
      }
   }

   return 0;
}

#undef RTXDECUINTFN
#undef RTXDECUINTTYPE
