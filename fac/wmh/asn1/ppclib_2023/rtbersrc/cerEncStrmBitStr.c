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

int cerEncStrmBitStr (OSCTXT*  pctxt,
                      const OSOCTET* object_p,
                      OSUINT32   numbits,
                      ASN1TagType tagging)
{
   register OSUINT32 ib, numbytes = (numbits + 7)/8;
   int stat;

   ib = numbits % 8;
   if (ib != 0) {
      ib = 8 - ib;
   }

   if (numbytes + 1 <= 1000) { /* primitive encoding */
      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BITSTR,
                                     numbytes + 1);
      else
         stat = berEncStrmLength (pctxt, numbytes + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = berEncStrmWriteOctet (pctxt, (OSOCTET)ib);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = berEncStrmWriteOctets (pctxt, object_p, numbytes);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else { /* segmented CER encoding, ITU-T X.680, clause 9.2 */
      OSUINT32 curOff;

      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndIndefLen (pctxt, TM_UNIV|TM_CONS|ASN_ID_BITSTR);
      else
         stat = berEncStrmWriteOctet (pctxt, 0x80);   /* indef len */
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Encode 999 bytes of the string per segment. This will make for 1000
         contents octets (due to the leading octet with the # of unused bits),
         the size required by CER.
      */
      for (curOff = 0; curOff < numbytes; curOff += 999) {
         OSUINT32 len = numbytes - curOff;
         OSOCTET unused_bits;

         if (len > 999) {
            /* Not the last segment. */
            len = 999;
            unused_bits = 0;
         }
         else {
            /* The last segment. */
            unused_bits = (OSOCTET)ib;
         }

         stat = berEncStrmTagAndLen
               (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BITSTR, len + 1);
         if (stat != 0) return LOG_RTERR(pctxt, stat);

         stat = berEncStrmWriteOctet(pctxt, unused_bits);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = berEncStrmWriteOctets (pctxt, object_p + curOff, len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      stat = berEncStrmEOC (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

