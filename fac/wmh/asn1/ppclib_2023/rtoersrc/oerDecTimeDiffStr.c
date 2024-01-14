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
#include "rtxsrc/rtxCharStr.h"


/* This method decodes an optimized TIME-OF_DAY DIFF value, as outlined in
 * Table 2 of Section 29.1 of the X.696 specification.  As such, the string
 * returned should be in the form of +/-HH[:MM]...
 */
EXTOERMETHOD int oerDecTimeDiffStr(OSCTXT *pctxt, char *pString)
{
   OSUINT16 hours = 0;
   OSINT16 minutes = 0;
   char *p = pString;
   int stat = 0;

   stat = oerDecInt16(pctxt, &minutes);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   // Build diff string...
   if (minutes < 0)
   {
      *p++ = '-';
      minutes = -minutes;
   }
   else
   {
      *p++ = '+';
   }

   hours = minutes / 60;
   minutes = minutes % 60;

   stat = rtxIntToCharStr(hours, p, 3, '0');
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   p += 2;

   if (minutes > 0)
   {
      *p++ = ':';

      stat = rtxUIntToCharStr(minutes, p, 3, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      p += 2;
   }

   return (int)(p - pString);
}
