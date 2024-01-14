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
 *  Routine name: pd_OpenType
 *
 *  Description:  The following function decodes an ASN.1 open type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  pdata       struct* pointer to open type structure to encode
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_OpenType
(OSCTXT* pctxt, const OSOCTET** object_p2, OSSIZE* pnumocts)
{
   int stat;

   if (0 != object_p2) {
      ASN1DynOctStr octStr;

      stat = pd_DynOctetString (pctxt, &octStr);
      if (stat == 0) {
         *pnumocts = octStr.numocts;
         *object_p2 = octStr.data;
      }
   }
   else {
      /* If no buffer given, skip field */
      stat = pd_DynOctetString (pctxt, 0);
   }

   return (0 == stat) ? 0 : LOG_RTERR (pctxt, stat);
}
