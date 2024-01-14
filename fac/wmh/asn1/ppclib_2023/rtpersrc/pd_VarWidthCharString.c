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

/***********************************************************************
 *
 *  Routine name: pd_VarWidthCharString
 *
 *  Description:  The following function decodes a value of the
 *                variable width character string useful type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        char**  pointer to pointer to receive decoded null-term string
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_VarWidthCharString (OSCTXT* pctxt, const char** pvalue)
{
   int        stat;
   OSOCTET* tmpstr;
   OSUINT32   len;

   /* note: need to save size constraint for use in pu_alignCharStr     */
   /* because it will be cleared in pd_Length from the context..        */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   /* Decode length */

   stat = pd_Length (pctxt, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   else if (len > (OSUINT32_MAX / 8)) return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Byte-align */

   PU_NEWFIELD (pctxt, "data");

   if (pu_alignCharStr (pctxt, len, 8, &sizeCnst)) {
      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Decode data */
   if (pvalue) {
      tmpstr = (OSOCTET*) rtxMemAlloc (pctxt, len + 1);
      if (0 != tmpstr) {
         if ((stat = pd_octets (pctxt, tmpstr, len, len * 8)) != 0)
            return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);

         tmpstr[len] = '\0';  /* add null-terminator */
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);

      *pvalue = (char*)tmpstr;
   }
   else {
      stat = rtxSkipBytes(pctxt, len);
      if (stat != 0) return LOG_RTERR(pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}
