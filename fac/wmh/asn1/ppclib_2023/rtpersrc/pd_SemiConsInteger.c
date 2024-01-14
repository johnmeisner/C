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
#include "rtxsrc/rtxDiag.h"

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

EXTPERMETHOD int pd_SemiConsInteger (OSCTXT* pctxt, OSINT32* pvalue,
                                       OSINT64 lower)
{
   OSINT64 value;
   int stat;

   stat = pd_SemiConsInt64(pctxt, &value, lower);

   if ( stat != 0 ) return LOG_RTERR(pctxt, stat);
   else if (value < OSINT32_MIN || value > OSINT32_MAX)
      return LOG_RTERR(pctxt, RTERR_TOOBIG);
   else {
      *pvalue = (OSINT32) value;
      return 0;
   }
}

#define PUI_FNNAME pd_UnconsInteger
#define PUI_TYPE OSINT32
#include "pd_UnconsInteger.c"

#define PUI_FNNAME pd_UnconsUInt8
#define PUI_TYPE OSUINT8
#define PUI_UNSIGNED 1
#include "pd_UnconsInteger.c"

#define PUI_FNNAME pd_UnconsUInt16
#define PUI_TYPE OSUINT16
#define PUI_UNSIGNED 1
#include "pd_UnconsInteger.c"

#define PUI_FNNAME pd_UnconsInt8
#define PUI_TYPE OSINT8
#include "pd_UnconsInteger.c"

#define PUI_FNNAME pd_UnconsInt16
#define PUI_TYPE OSINT16
#include "pd_UnconsInteger.c"