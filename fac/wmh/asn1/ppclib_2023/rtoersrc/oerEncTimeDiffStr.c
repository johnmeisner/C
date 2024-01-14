/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxDateTime.hh"


/* This method encodes an optimized TIME-OF-DAY DIFF value, as outlined
 * in Table 2 of Section 29.1 of the X.696 specification.  As such, it
 * is expecting a diff string with property of Local-or-UTC=LD, so the
 * string should look like +/-HH[:MM]...
 */
EXTOERMETHOD int oerEncTimeDiffStr(OSCTXT *pctxt, const char *pString)
{
   OSUINT16 hours = 0;
   OSBOOL isValueNeg = FALSE;
   OSINT16 minutes = 0;
   const char *p = pString;
   int stat = 0;

   if (*p == '-')
   {
      isValueNeg = TRUE;
   }
   else if (*p != '+')
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   p++;

   PARSE_2_DIGITS(hours, p, stat);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (hours > 15)
   {
      return LOG_RTERR(pctxt, RTERR_BADVALUE);
   }

   if (*p++ == ':')
   {
      PARSE_2_DIGITS(minutes, p, stat);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      if (minutes > 59)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   // Convert values to minutes and encode...
   minutes += (hours * 60);
   if (isValueNeg)
   {
      minutes = -minutes;
   }

   stat = oerEncInt(pctxt, minutes, 2);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   return (int)(p - pString);
}
