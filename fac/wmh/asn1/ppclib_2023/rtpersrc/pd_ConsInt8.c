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
 *  Routine name: pd_<integer>
 *
 *  Description:  The following functions decode integers.  Different
 *                routines handle signed/unsigned and constrained/
 *                unconstrained variations.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pvalue      int*    Pointer to value to receive decoded result
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_ConsInt8 (OSCTXT* pctxt,
                 OSINT8* pvalue, OSINT64 lower, OSINT64 upper)
{
   OSINT64 value;
   int stat;

   stat = pd_ConsInt64(pctxt, &value, lower, upper);

   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);
   else if (value < OSINT8_MIN || value > OSINT8_MAX)
      return LOG_RTERR(pctxt, RTERR_TOOBIG);
   else {
      if (pvalue) *pvalue = (OSINT8) value;
      return 0;
   }
}
