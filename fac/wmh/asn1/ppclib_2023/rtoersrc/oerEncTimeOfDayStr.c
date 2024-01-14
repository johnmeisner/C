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
#include "rtxsrc/rtxDateTime.hh"


/* This method encodes an optimized TIME-OF-DAY string, as outlined in
 * Table 2 of Section 29.1 of the X.696 specification.  As such,
 * it is expecting a string with properties of Basic=Time and TIME=HMS
 * (X.680, 38.4.2), so the string should look like HH:MM:SS...
 */
EXTOERMETHOD int oerEncTimeOfDayStr(OSCTXT *pctxt, const char *pString,
                                    OSUINT32 flags)
{
   OSUINT32 fraction = 0;
   unsigned fractionDigits = 0;
   OSUINT8 hours = 0, minutes = 0, seconds = 0;
   const char *p = pString;
   int stat = 0;

   // Walk through hours...
   PARSE_2_DIGITS(hours, p, stat);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   else if (hours > 24)
   {
      return LOG_RTERR(pctxt, RTERR_BADVALUE);
   }

   // Walk through minutes...
   if (flags & OSMINUTES)
   {
      if (*p++ != ':')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      PARSE_2_DIGITS(minutes, p, stat);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else if (minutes > 59)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   // Walk through seconds...
   if (flags & OSSECONDS)
   {
      if (*p++ != ':')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      PARSE_2_DIGITS(seconds, p, stat);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      else if (seconds > 60)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
   }

   if (flags & OSFRACTION)
   {
      fractionDigits = (flags & OSFRACTION);

      if (*p != '.' && *p != ',')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      p++;

      while (OS_ISDIGIT(*p) && fractionDigits)
      {
         fraction = fraction * 10 + (*p++ - '0');
         fractionDigits--;
      }

      if (fractionDigits)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
   }

   if (flags & OSUTC)
   {
      if (*p++ != 'Z')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
   }

   // Encode values...
   stat = oerEncUInt(pctxt, hours, 1);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (flags & OSMINUTES)
   {
      stat = oerEncUInt(pctxt, minutes, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSSECONDS)
   {
      stat = oerEncUInt(pctxt, seconds, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSFRACTION)
   {
      stat = oerEncUInt(pctxt, fraction, 4);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSDIFF)
   {
      stat = oerEncTimeDiffStr(pctxt, p);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      p += stat;
   }

   return (int)(p - pString);
}
