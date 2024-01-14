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
#include "rtxsrc/rtxBitDecode.h"
#include "rtxsrc/rtxCharStr.h"

#define MAX_SUFFIXES 7
#define MAX_VALUES 9
static char suffix[MAX_SUFFIXES] = { 'Y', 'M', 'W', 'D', 'H', 'M', 'S' };


/* This method decodes an optimized DURATION string, as outlined in Table 3
 * of Section 29.1 of the X.696 specification.  As such, the string returned
 *  should be in the form of either PnW or PnYnMnDTnHnMnS (X.680, 38.4.4)...
 */
int oerDecDuration(OSCTXT *pctxt, char *pString)
{
   OSBOOL addTimeSep = TRUE; // For adding 'T' character...
   int i, stat;
   OSBOOL optbit[MAX_VALUES], unused;
   char *p = pString;
   char suffixChar = 'P';
   OSUINT32 val[MAX_VALUES]; /* y m w d h m s #-fraction-digits fraction */

   OSCRTLMEMSET(optbit, FALSE, sizeof(optbit));
   OSCRTLMEMSET(val, 0, sizeof(val));

   // Decode preamble octet bits...
   for (i = 0; i < MAX_VALUES; i++)
   {
      stat = rtxDecBit(pctxt, &optbit[i]);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Decode remaining unused bits from second preamble octet...
   for (i = MAX_VALUES; i < 16; i++)
   {
      stat = rtxDecBit(pctxt, &unused);
      if (stat != 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   // Decode the actual date/time values...
   for (i = 0; i < MAX_VALUES; i++)
   {
      if (optbit[i])
      {
         stat = oerDecUInt32(pctxt, &val[i]);
         if (stat != 0)
         {
            return LOG_RTERR(pctxt, stat);
         }
      }
   }

   // Create string...
   for (i = 0; i < MAX_SUFFIXES; i++)
   {
      if (optbit[i])
      {
         *p++ = suffixChar;
         suffixChar = suffix[i];

         if ((i >= 4) && (i <= 6) && addTimeSep)
         {
            *p++ = 'T';
            addTimeSep = FALSE;
         }

         stat = rtxUIntToCharStr(val[i], p, 5, 0);
         if (stat < 0)
         {
            return LOG_RTERR(pctxt, stat);
         }

         p += OSCRTLSTRLEN(p);
      }
   }

   if (optbit[7])
   {
      *p++ = '.';

      stat = rtxUIntToCharStr(val[8], p, val[7] + 1, '0');
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      p += val[7];
   }

   *p++ = suffixChar;
   *p = 0;
   return (int)(p - pString);
}

EXTOERMETHOD int oerDecDurationStr(OSCTXT *pctxt, char **ppString)
{
   char *p;
   int stat;

   p = (char *)rtxMemAlloc(pctxt, 60);
   if (p == NULL)
   {
      return LOG_RTERR(pctxt, RTERR_NOMEM);
   }

   stat = oerDecDuration(pctxt, p);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   *ppString = p;
   return 0;
}
