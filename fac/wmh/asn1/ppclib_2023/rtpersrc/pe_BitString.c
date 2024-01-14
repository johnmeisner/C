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

/***********************************************************************
 *
 *  Routine name: pe_BitString
 *
 *  Description:  The following function encodes a bit string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  numbits     int     number of bits in the bit string
 *  data        octet*  pointer to buffer containing bits to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_BitString
(OSCTXT* pctxt, OSSIZE numbits, const OSOCTET* data)
{
   int enclen, octidx = 0, stat;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pe_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   for (;;) {
      if ((enclen = pe_Length (pctxt, numbits)) < 0) {
         return LOG_RTERR (pctxt, enclen);
      }

      stat = pe_BinaryStringData
         (pctxt, numbits, enclen, &data[octidx], &sizeCnst, TRUE);

      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (enclen < (int)numbits) {
         numbits -= enclen;
         octidx += (enclen/8);
      }
      else break;
   }

   return 0;
}

EXTPERMETHOD int pe_BitString32
(OSCTXT* pctxt, ASN1BitStr32* pvalue, OSUINT32 lower, OSUINT32 upper)
{
   PU_SETSIZECONSTRAINT (pctxt, lower, upper, 0, 0);
   return pe_BitString (pctxt, pvalue->numbits, pvalue->data);
}

EXTPERMETHOD int pe_BitStringExt (OSCTXT* pctxt, OSSIZE numbits,
   const OSOCTET* data, OSSIZE dataSize, const OSOCTET* extData)
{
   int enclen, stat;
   OSSIZE octidx = 0, bitidx = 0;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pe_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   for (;;) {
      if ((enclen = pe_Length (pctxt, numbits)) < 0) {
         return LOG_RTERR (pctxt, enclen);
      }

      PU_NEWFIELD (pctxt, "BitString");

      if (enclen > 0) {
         if (pctxt->buffer.aligned &&
             pu_BitAndOctetStringAlignmentTest (&sizeCnst, numbits, TRUE)) {
            stat = pe_byte_align (pctxt);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

         if (octidx < dataSize) {
            if ((bitidx + (OSSIZE)enclen) > dataSize*8) {
               /* Encode remaining data in root */
               stat = pe_octets (pctxt, &data[octidx], dataSize*8 - bitidx);
               if (stat != 0) return LOG_RTERR (pctxt, stat);

               /* Encode extension data */
               stat = pe_octets (pctxt, &extData[0],
                                 (OSSIZE)enclen - dataSize*8);
            }
            else {
               stat = pe_octets (pctxt, &data[octidx], enclen);
            }
         }
         else {
            /* Encode extension data */
            stat = pe_octets (pctxt, &extData[octidx-dataSize],
                              (OSSIZE)enclen - dataSize*8);
         }

         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      PU_SETBITCOUNT (pctxt);

      if (enclen < (int)numbits) {
         numbits -= enclen;
         bitidx += enclen;
         octidx = (bitidx/8);
      }
      else break;
   }

   return 0;
}
