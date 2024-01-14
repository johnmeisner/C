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
#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxDateTime.hh"

#define MAX_SUFFIXES 7
#define MAX_VALUES 9
static char suffix[MAX_SUFFIXES] = { 'Y', 'M', 'W', 'D', 'H', 'M', 'S' };


/* This method encodes an optimized DURATION string, as outlined in
 * Table 3 of Section 29.1 of the X.696 specification.  As such,
 * it is expecting a string with properties of Basic=Interval,
 * Interval-type=D (X.680, 38.4.4), so the string should look like
 * either PnW or PnYnMnDTnHnMnS...
 */
EXTOERMETHOD int oerEncDurationStr(OSCTXT *pctxt, const char *pString)
{
   char c;
   int i, j, max = 4 /* y m w d */, stat = 0;
   OSBOOL optbit[MAX_VALUES];
   const char *p = pString;
   OSUINT32 val[MAX_VALUES]; /* y m w d h m s #-fraction-digits fraction */
   OSUINT32 value = 0;

   OSCRTLMEMSET(optbit, FALSE, sizeof(optbit));
   OSCRTLMEMSET(val, 0, sizeof(val));

   if (*p++ != 'P')
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   for (i = 0; i < MAX_SUFFIXES; i++)
   {
      value = 0;

      if (*p == 'T')
      {
         p++;
         i = 4;
         max = 7;
      }
      else if (*p == '\0')
      {
         if (i > 0)
         {
            break;
         }
         else /* all fields are absent */
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
      else if (!OS_ISDIGIT(*p))
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      while (OS_ISDIGIT(*p))
      {
         value = value * 10 + (*p++ - '0');
      }

      // Handle fractional value, if present...
      c = *p++;
      if ((c == '.') || (c == ','))
      {
         // 7 = number of fraction digits; 8 = actual fraction value...
         optbit[7] = optbit[8] = TRUE;

         if (!OS_ISDIGIT(*p))
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }

         while (OS_ISDIGIT(*p))
         {
            val[7]++;
            val[8] = val[8] * 10 + (*p++ - '0');
         }

         c = *p++;
      }

      // Check if we've found one of the suffixes...
      for (j = i; j < max; j++)
      {
         if (c == suffix[j])
         {
            optbit[j] = TRUE;
            val[j] = value;
            break;
         }
      }

      if (j == max)
      {
         // Found a non-digit that was not a proper suffix character...
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
   }

   // Encode preamble octet bits...
   for (i = 0; i < MAX_VALUES; i++)
   {
      stat = rtxEncBit(pctxt, optbit[i]);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Encode unused bits to round out second octet...
   for (i = MAX_VALUES; i < 16; i++)
   {
      stat = rtxEncBit(pctxt, 0);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Encode the actual date/time values...
   for (i = 0; i < MAX_VALUES; i++)
   {
      if (optbit[i])
      {
         stat = oerEncUInt(pctxt, val[i], 4);
         if (stat != 0)
         {
            return LOG_RTERR(pctxt, stat);
         }

      }
   }

   return (int)(p - pString);
}
