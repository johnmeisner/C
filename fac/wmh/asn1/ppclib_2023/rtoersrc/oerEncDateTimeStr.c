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


/* This method encodes an optimized DATE-TIME string, as outlined in
 * Tables 1 and 2 of Section 29.1 of the X.696 specification.  As such,
 * it is expecting a string with properties of Basic=Time and TIME=HMS,
 * and Basic=Date and DATE=YMD (X.680, 38.4.1 and .2), so the string
 * should look like YYY-MM-DDTHH:MM:SS...
 */
EXTOERMETHOD int oerEncDateTimeStr(OSCTXT *pctxt, const char *pString,
                                   OSUINT32 flags)
{
   int stat;

   stat = oerEncDateStr(pctxt, pString, flags);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (pString[stat] != 'T')
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   stat = oerEncTimeOfDayStr(pctxt, pString + stat + 1, flags);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   return 0;
}
