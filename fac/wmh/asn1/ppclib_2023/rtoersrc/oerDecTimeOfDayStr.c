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


/* This method decodes an optimized TIME-OF_DAY string, as outlined in
 * Table 1 of Section 29.1 of the X.696 specification.  As such, the
 * string returned should be in the form of HH:MM:SS (X.680, 38.4.2)...
 */
int oerDecTimeOfDay(OSCTXT *pctxt, char *pString, OSUINT32 flags)
{
   OSUINT32 fraction;
   OSUINT8 hours, minutes, seconds;
   char *p = pString;
   int stat = 0;

   stat = oerDecUInt8(pctxt, &hours);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (flags & OSMINUTES)
   {
      stat = oerDecUInt8(pctxt, &minutes);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSSECONDS)
   {
      stat = oerDecUInt8(pctxt, &seconds);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSFRACTION)
   {
      stat = oerDecUInt32(pctxt, &fraction);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Build string...
   stat = rtxUIntToCharStr(hours, p, 3, '0');
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   p += 2;

   if (flags & OSMINUTES)
   {
      *p++ = ':';
      stat = rtxUIntToCharStr(minutes, p, 3, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += 2;
   }

   if (flags & OSSECONDS)
   {
      *p++ = ':';
      stat = rtxUIntToCharStr(seconds, p, 3, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += 2;
   }

   if (flags & OSFRACTION)
   {
      *p++ = '.';
      stat = rtxUIntToCharStr(fraction, p, (flags & OSFRACTION) + 1, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += stat;
   }

   if (flags & OSUTC)
   {
      *p++ = 'Z';
   }
   else if (flags & OSDIFF)
   {
      stat = oerDecTimeDiffStr(pctxt, p);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      p += stat;
   }

   *p = 0;
   return (int)(p - pString);
}

EXTOERMETHOD int oerDecTimeOfDayStr(OSCTXT *pctxt, char **ppString,
                                    OSUINT32 flags)
{
   char *p;
   int stat;

   p = (char *)rtxMemAlloc(pctxt, 20); // Enough to hold HH:MM:SS and fraction...
   if (p == NULL)
   {
      return LOG_RTERR(pctxt, RTERR_NOMEM);
   }

   stat = oerDecTimeOfDay(pctxt, p, flags);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   *ppString = p;
   return 0;
}
