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
 *  Routine name: pe_32BitConstrainedString
 *
 *  Description:  The following function encodes a 32-bit constrained
 *                string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        struct* pointer to 32-bit string structure to encode
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_32BitConstrainedString (OSCTXT* pctxt,
                               Asn132BitCharString value,
                               Asn132BitCharSet* pCharSet)
{
   OSUINT32 i, pos;
   OSUINT32 nbits = pctxt->buffer.aligned ?
      pCharSet->alignedBits : pCharSet->unalignedBits;
   int stat;

   /* Encode length */

   stat = pe_Length (pctxt, value.nchars);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   PU_NEWFIELD (pctxt, "data");

   /* Byte align */

   if (pctxt->buffer.aligned) {
      stat = pe_byte_align (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode data */

   for (i = 0; i < value.nchars; i++) {
      if (pCharSet->charSet.data == 0) {
         stat = pe_bits (pctxt, value.data[i] - pCharSet->firstChar,
                         nbits);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         for (pos = 0; pos < pCharSet->charSet.nchars; pos++) {
            if (value.data[i] == pCharSet->charSet.data[pos]) {
               stat = pe_bits (pctxt, pos, nbits);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
               break;
            }
         }
      }
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}
