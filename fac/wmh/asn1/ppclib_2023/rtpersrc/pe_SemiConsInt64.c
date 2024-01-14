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
#include "rtxsrc/rtxDiag.h"

/***********************************************************************
 *
 *  Routine name: pe_<integer>
 *
 *  Description:  The following functions encode integers.  Different
 *                routines handle signed/unsigned and constrained/
 *                unconstrained variations.  There is also a function
 *                to handle big (> 32 bit) integers.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  value       int     Integer value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      uint    Number of octets required
 *
 **********************************************************************/

#ifndef _NO_INT64_SUPPORT

EXTPERMETHOD int pe_SemiConsInt64 (OSCTXT* pctxt, OSINT64 value, OSINT64 lower)
{
   OSUINT64 tempValue = (OSUINT64) (value - lower);
   return pe_SemiConsUInt64 (pctxt, tempValue, OSUI64CONST(0));
}

static OSSIZE getIntLength(OSINT64 value)
{
   int shift = (int)(((sizeof(value) - 1) * 8) - 1);
   OSUINT64 tempValue;

   /* Calculate signed number value length */

   for ( ; shift > 0; shift -= 8) {
      tempValue = (value >> shift) & 0x1ff;
      if (tempValue == 0 || tempValue == 0x1ff) continue;
      else break;
   }

   return (shift + 9) / 8;
}

EXTPERMETHOD int pe_UnconsInt64Aligned (OSCTXT* pctxt, OSINT64 value)
{
   OSSIZE nbytes = getIntLength(value);
   int stat;

   /* Encode length */

   PU_NEWFIELD (pctxt, "length");

   stat = pe_octetAligned (pctxt, (OSUINT8)nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   if ((stat = pe_byte_align (pctxt)) != 0)
      return LOG_RTERR (pctxt, stat);

   /* Encode signed value */

   PU_NEWFIELD (pctxt, "UnconsInteger");

   stat = pe_2sCompBinInt64 (pctxt, value);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}

EXTPERMETHOD int pe_UnconsInt64Unaligned (OSCTXT* pctxt, OSINT64 value)
{
   OSSIZE nbytes = getIntLength(value);
   int stat;

   /* Encode length */

   PU_NEWFIELD (pctxt, "length");

   stat = pe_octet (pctxt, (OSUINT8)nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   /* Encode signed value */

   PU_NEWFIELD (pctxt, "UnconsInt64");

   stat = pe_2sCompBinInt64 (pctxt, value);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}

EXTPERMETHOD int pe_UnconsInt64 (OSCTXT* pctxt, OSINT64 value)
{
   return (pctxt->buffer.aligned) ?
      pe_UnconsInt64Aligned (pctxt, value) :
      pe_UnconsInt64Unaligned (pctxt, value);
}

#endif

