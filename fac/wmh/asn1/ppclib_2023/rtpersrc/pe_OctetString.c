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
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pe_OctetString
 *
 *  Description:  The following function encodes an octet string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  numocts     int     number of octets in octet string
 *  data        octet*  pointer to buffer containing string to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

int pe_BinaryStringData
(OSCTXT* pctxt, OSSIZE itemCount, OSSIZE segSizeBits, const OSOCTET* data,
 const Asn1SizeCnst* pSizeCnst, OSBOOL bitStrFlag)
{
#ifdef _TRACE
   const char* fieldName = bitStrFlag ? "BitString" : "OctetString";
   PU_NEWFIELD (pctxt, fieldName);
#endif
   if (itemCount > 0) {
      int stat = 0;
      if (pctxt->buffer.aligned &&
          pu_BitAndOctetStringAlignmentTest
             (pSizeCnst, itemCount, bitStrFlag))
         {
            stat = pe_byte_align (pctxt);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }

      stat = rtxEncBitsFromByteArray (pctxt, data, segSizeBits);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}

EXTPERMETHOD int pe_OctetString
(OSCTXT* pctxt, OSSIZE numocts, const OSOCTET* data)
{
   int enclen, octidx = 0, stat;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pe_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   for (;;) {
      if ((enclen = pe_Length (pctxt, numocts)) < 0) {
         return LOG_RTERR (pctxt, enclen);
      }

      stat = pe_BinaryStringData
         (pctxt, numocts, enclen * 8,  &data[octidx], &sizeCnst, FALSE);

      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (enclen < (int)numocts) {
         numocts -= enclen;
         octidx += enclen;
      }
      else break;
   }

   /* if numocts % 16384 == 0, we must encode a byte of zeros for length */
   if (numocts % 16384 == 0 && numocts > 0) {
      pe_Length (pctxt, 0);
   }

   LCHECKPER (pctxt);

   return 0;
}
