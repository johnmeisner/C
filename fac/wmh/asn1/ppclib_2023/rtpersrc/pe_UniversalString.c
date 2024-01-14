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
#include "rtsrc/asn1CharSet.h"

/***********************************************************************
 *
 *  Routine name: pe_UniversalString
 *
 *  Description:  The following function encodes a value of the
 *                Universal string useful type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        struct* pointer to 32-bit char string structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_UniversalString (OSCTXT* pctxt, ASN1UniversalString value,
                  Asn132BitCharSet* permCharSet)
{
   Asn132BitCharSet charSet;
   int stat;

   /* Set character set */

   pu_init32BitCharSet (&charSet, UCS_FIRST, UCS_LAST, UCS_ABITS, UCS_UBITS);

   if (permCharSet) {
      pu_set32BitCharSet (pctxt, &charSet, permCharSet);
   }

   /* Encode constrained string */

   stat = pe_32BitConstrainedString (pctxt, value, &charSet);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
