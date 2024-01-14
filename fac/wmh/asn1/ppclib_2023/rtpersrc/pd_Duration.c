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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxCharStr.h"

static OSUINT32 maxVal[9] = { 31, 15, 63, 31, 31, 63, 63, 3, 999 };
static char suffix[7] = { 'Y', 'M', 'W', 'D', 'H', 'M', 'S' };

#ifdef _TRACE
static const char* fieldNamePresents[8] = {
   "yearsPresents", "monthsPresents", "weeksPresents", "daysPresents",
   "hoursPresents", "minutesPresents", "secondsPresents", "fractionPresents"
};

#endif /* _TRACE */

int pd_DurationToStrn (OSCTXT* pctxt, char* string, size_t strSz)
{
   char* p;
   OSUINT32 val[9]; /* y m w d h m s fracDigits fraction */
   OSBOOL optbit[9];
   OSBOOL timePresents;
   int i;
   int stat;
   char suffixChar;
   OS_UNUSED_ARG(strSz);

   p = string;
   timePresents = TRUE;
   suffixChar = 'P';

   OSCRTLMEMSET (optbit, 0, sizeof(optbit));
   OSCRTLMEMSET (val, 0, sizeof(val));

   /* decode duration */
   for (i = 0; i < 8; i++) {
#ifdef _TRACE
      PU_NEWFIELD (pctxt, fieldNamePresents[i]);
#endif
      stat = PD_BIT (pctxt, &optbit[i]);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);
   }

   optbit[8] = optbit[7]; /* fraction presents bit */

   for (i = 0; i < 9; i++) {
      if (optbit[i]) {
         OSBOOL extbit;

#ifdef _TRACE
         PU_PUSHNAME (pctxt, fieldName[i]);
#endif

         PU_NEWFIELD (pctxt, "extension marker");
         stat = PD_BIT (pctxt, &extbit);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         PU_SETBITCOUNT (pctxt);

         if (extbit)
            stat = pd_UnconsUnsigned (pctxt, &val[i]);
         else
            stat = pd_ConsUnsigned (pctxt, &val[i], (i == 7) ? 1 : 0,
                                    maxVal[i]);

         if (stat != 0) return LOG_RTERR (pctxt, stat);

         PU_POPNAME (pctxt);
      }
   }

   if (string) {
      /* create string */
      for (i = 0; i < 7; i++) {
         if (optbit[i]) {
            *p++ = suffixChar;
            suffixChar = suffix[i];

            if (i >= 4 && i <= 6 && timePresents) {
               timePresents = FALSE;
               *p++ = 'T';
            }

            stat = rtxUIntToCharStr (val[i], p, 10, 0);
            if (stat < 0) return LOG_RTERR (pctxt, stat);

            p += OSCRTLSTRLEN (p);
         }
      }

      if (optbit[7]) {
         *p++ = '.';

         stat = rtxUIntToCharStr (val[8], p, val[7] + 1, '0');
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += val[7];
      }

      *p++ = suffixChar;
      *p = 0;
   }

   return (int) (p - string);
}

EXTPERMETHOD int pd_Duration (OSCTXT* pctxt, const char** string, OSBOOL rec)
{
   int stat;
   char buff[12 + 8 * 11 + 3]; /* Rn/P 11 nums with separators */
   char* p = buff;
   char* str;

   if (rec) {
      /* recursive time interval */
      OSBOOL optbit;
      OSINT32 recurrence;

      PU_PUSHNAME (pctxt, "recurrence");

      PU_NEWFIELD (pctxt, "recurrencePresent");
      stat = PD_BIT (pctxt, &optbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);

      *p++ = 'R';

      if (optbit) {
         /* decode recurrence */
         stat = pd_UnconsInteger (pctxt, &recurrence);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = rtxIntToCharStr (recurrence, p, 12, 0);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += stat;
      }

      PU_POPNAME (pctxt);

      *p++ = '/';
   }

   stat = pd_DurationToStrn (pctxt, p, sizeof(buff) - 12);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* create date string */
   if ( string ) {
      stat += (int) (p - buff);
      str = (char*) rtxMemAlloc (pctxt, (size_t) stat + 1);
      if (0 == str) return LOG_RTERR (pctxt, RTERR_NOMEM);

      OSCRTLSAFEMEMCPY (str, (size_t)(stat+1), buff, (size_t)(stat+1));
      *string = str;
   }

   return 0;
}
