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

#include "rtpersrc/pe_common.hh"

/***********************************************************************
 *
 *  Routine name: pe_VarWidthCharString
 *
 *  Description:  The following function encodes a value of the
 *                variable width character string useful type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        char*   pointer to null-term string to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_VarWidthCharString (OSCTXT* pctxt, const char* value)
{
   int         stat;
   OSUINT32    len = (OSUINT32) OSCRTLSTRLEN (value);
   /* note: need to save size constraint for use in pu_alignCharStr     */
   /* because it will be cleared in pe_Length from the context..        */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   /* Encode length */

   stat = pe_Length (pctxt, len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Byte align */

   PU_NEWFIELD (pctxt, "data");

   if (pu_alignCharStr (pctxt, len, 8, &sizeCnst)) {
      stat = pe_byte_align (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode data */

   if ((stat = pe_octets (pctxt, (const OSOCTET*)value, len * 8)) != 0)
      return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}
