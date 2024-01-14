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
 *  Routine name: pe_SmallNonNegWholeNumber
 *
 *  Description:  This routine encompasses the rules to encode a
 *                small non-negative whole number as specified in
 *                section 10.6 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  value       uint    Value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
  **********************************************************************/

EXTPERMETHOD int pe_SmallNonNegWholeNumber (OSCTXT* pctxt, OSUINT32 value)
{
   OSUINT32 len = 4;
   int stat;

   if (value < 64) {
      stat = pe_bits (pctxt, value, 7);
   }
   else {
      /* Encode extension bit */
      stat = pe_bit (pctxt, TRUE);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (pctxt->buffer.aligned) {
         if ((stat = pe_byte_align (pctxt)) != 0)
            return LOG_RTERR (pctxt, stat);
      }

      /* Encode a one-byte length determinant value */
      if (value < 256) len = 1;
      else if (value < OSINTCONST(65536)) len = 2;
      else if (value < OSINTCONST(0x1000000)) len = 3;
      else {
         rtxErrAddStrParm (pctxt, "very large SmallNonNegWholeNumber");
         return LOG_RTERR (pctxt, RTERR_NOTSUPP);
      }

      stat = pe_bits (pctxt, len, 8);

      /* Encode the value */
      if (stat == 0) {
         stat = pe_bits (pctxt, value, len*8);
      }
   }

   LCHECKPER (pctxt);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

