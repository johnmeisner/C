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
 *  Routine name: pe_OpenType
 *
 *  Description:  The following function encodes an ASN.1 open type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  pdata       struct* pointer to open type structure to receive
 *                      decoded value
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_OpenType
(OSCTXT* pctxt, OSSIZE numocts, const OSOCTET* data)
{
   int enclen, octidx = 0, stat;
   OSOCTET zeroByte = 0x00;
   ASN1OpenType openType;

   /* If open type contains length zero, add a single zero byte (10.1) */

   if (numocts == 0) {
      openType.numocts = 1;
      openType.data = &zeroByte;
   }
   else {
      openType.numocts = numocts;
      openType.data = data;
   }

   /* Encode the open type */

   for (;;) {
      if ((enclen = pe_Length (pctxt, openType.numocts)) < 0) {
         return LOG_RTERR (pctxt, enclen);
      }

      PU_NEWFIELD (pctxt, "OpenType");

      if (pctxt->buffer.aligned) {
         stat = pe_byte_align (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      stat = pe_octets (pctxt, &openType.data[octidx], enclen * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      PU_SETBITCOUNT (pctxt);

      if (enclen < (int)openType.numocts) {
         openType.numocts -= enclen;
         octidx += enclen;
      }
      else break;
   }

   /* 12-06-12 MEM:  ASN-4435. */
   if (numocts > 0 && numocts % 16384 == 0) {
      pe_Length (pctxt, 0);
   }

   return 0;
}
