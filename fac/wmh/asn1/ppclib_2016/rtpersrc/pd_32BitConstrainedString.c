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

#include "rtpersrc/pd_common.hh"

/***********************************************************************
 *
 *  Routine name: pd_32BitConstrainedString
 *
 *  Description:  The following function decodes a 32-bit constrained
 *                string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  pCharSet    struct* pointer to character set structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pString     struct* pointer to 32-bit char structure to receive
 *                      decoded value
 *  stat        int     completion status
 *
 **********************************************************************/

EXTPERMETHOD int pd_32BitConstrainedString (OSCTXT* pctxt,
                               Asn132BitCharString* pString,
                               Asn132BitCharSet* pCharSet)
{
   OSUINT32 idx, nbits = pctxt->buffer.aligned ?
      pCharSet->alignedBits : pCharSet->unalignedBits;

   int stat;
   OSUINT32 i, nchars;
   OS32BITCHAR* data;

   /* Decode length */

   stat = pd_Length (pctxt, &nchars);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Byte-align */

   PU_NEWFIELD (pctxt, "data");

   if (pctxt->buffer.aligned) {
      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Decode data */

   data = (OS32BITCHAR*)rtxMemAlloc (pctxt, nchars*sizeof(OS32BITCHAR));

   if (data != 0) {
      for (i = 0; i < nchars; i++) {
         stat = pd_bits (pctxt, &idx, nbits);
         if (stat != 0)
            return LOG_RTERR_AND_FREE_MEM (pctxt, stat, data);

         data[i] = (pCharSet->charSet.data == 0) ?
            idx + pCharSet->firstChar : pCharSet->charSet.data[idx];
      }
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   PU_SETBITCOUNT (pctxt);

   pString->data = data;
   pString->nchars = nchars;

   return 0;
}
