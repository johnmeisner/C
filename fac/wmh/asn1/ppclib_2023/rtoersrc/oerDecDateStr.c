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


/* This method decodes an optimized DATE string, as outlined in Table 1 of
 * Section 29.1 of the X.696 specification.  As such, the string returned
 *  should be in the form of YYYY-MM-DD (X.680, 38.4.1)...
 */
int oerDecDate(OSCTXT *pctxt, char *pString, OSUINT32 flags)
{
   OSUINT8 day, month;
   OSSIZE len;
   char *p = pString;
   int stat = 0;
   OSINT16 year;

   stat = oerDecLen(pctxt, &len);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = oerDecInt16(pctxt, &year);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (flags & OSMONTH)
   {
      stat = oerDecUInt8(pctxt, &month);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (flags & OSDAY)
   {
      stat = oerDecUInt8(pctxt, &day);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Build date string...
   if ((flags & OSANY_MASK) && (year < 0))
   {
      *p++ = '-';
      year = -year;
   }

   stat = rtxIntToCharStr(year, p, 5, '0');
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   p += 4;

   if (flags & OSMONTH)
   {
      *p++ = '-';
      stat = rtxUIntToCharStr(month, p, 3, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += 2;
   }

   if (flags & OSDAY)
   {
      *p++ = '-';
      stat = rtxUIntToCharStr(day, p, 3, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += 2;
   }

   *p = 0;
   return (int)(p - pString);
}

EXTOERMETHOD int oerDecDateStr(OSCTXT *pctxt, char **ppString, OSUINT32 flags)
{
   char *p;
   int stat;

   p = (char *)rtxMemAlloc(pctxt, 12); // Enough to hold -YYYY-MM-DD...
   if (p == NULL)
   {
      return LOG_RTERR(pctxt, RTERR_NOMEM);
   }

   stat = oerDecDate(pctxt, p, flags);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   *ppString = p;
   return 0;
}
