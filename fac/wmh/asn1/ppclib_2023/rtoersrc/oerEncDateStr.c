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
#include "rtpersrc/asn1per.h" // For date/time format flags...
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDateTime.hh"


/* This method encodes an optimized DATE string, as outlined in Table 1 of
 * Section 29.1 of the X.696 specification.  As such, it is expecting a
 * string with properties of Basic=Date and Date=YMD (X.680, 38.4.1), so
 * the string should look like YYYY-MM-DD...
 */
EXTOERMETHOD int oerEncDateStr(OSCTXT *pctxt, const char *pString,
                               OSUINT32 flags)
{
   OSUINT8 day = 0, digit_count = 0, month = 0;
   OSBOOL isYearNeg = FALSE;
   const char *p = pString;
   int stat = 0;
   OSINT16 year = 0;

   // Walk through year...
   if ((flags & OSANY_MASK) && (*p == '-'))
   {
      isYearNeg = TRUE;
      p++;
   }

   while (OS_ISDIGIT(*p))
   {
      year = year * 10 + (*p++ - '0');
      digit_count++;
   }

   if (isYearNeg)
   {
      year = -year;
   }

   if (digit_count != 4)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (flags & OSMONTH)
   {
      if (*p++ != '-')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      PARSE_2_DIGITS(month, p, stat);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else if (month > 12)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   if (flags & OSDAY)
   {
      if (*p++ != '-')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      PARSE_2_DIGITS(day, p, stat);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else if (day > 31)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   // Encode values (and their lengths if necessary, per X.696 Section 10)...
   stat = oerEncLen(pctxt, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = oerEncInt(pctxt, year, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (flags & OSMONTH)
   {
      stat = oerEncUInt(pctxt, month, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSDAY)
   {
      stat = oerEncUInt(pctxt, day, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   return (int)(p - pString);
}
