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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxDateTime.hh"

static OSUINT32 maxVal[9] = { 31, 15, 63, 31, 31, 63, 63, 3, 999 };
static char suffix[7] = { 'Y', 'M', 'W', 'D', 'H', 'M', 'S' };

#ifdef _TRACE
static const char* fieldNamePresents[8] = {
   "yearsPresents", "monthsPresents", "weeksPresents", "daysPresents",
   "hoursPresents", "minutesPresents", "secondsPresents", "fractionPresents"
};

#endif /* _TRACE */

int pe_DurationFromStr (OSCTXT* pctxt, const char* string)
{
   const char* p = string;
   OSUINT32 val[9]; /* y m w d h m s fracDigits fraction */
   OSBOOL optbit[9];
   int i;
   int iMax = 4; /* y m w d */
   int stat;
   OSBOOL last;

   /* parse string */
   OSCRTLMEMSET (optbit, FALSE, sizeof (optbit));
   OSCRTLMEMSET (val, 0, sizeof(val));

   if (*p++ != 'P')
      return LOG_RTERR (pctxt, RTERR_INVFORMAT);

   for (i = 0; i < 7 && !optbit[7]; i++) {
      OSUINT32 value = 0;
      char c;

      if (*p == 'T') {
         p++;
         i = 4;
         iMax = 7;
      }
      else if (*p == '/' || !*p) {
         if (i > 0)
            break;
         else /* all fields are absent */
            return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      }
      else if (!OS_ISDIGIT (*p))
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      while (OS_ISDIGIT (*p))
         value = value * 10 + (OSUINT32) (*p++ - '0');

      c = *p++;

      if (c == '.' || c == ',') {
         optbit[7] = optbit[8] = TRUE;
         val[7] = val[8] = 0;

         if (!OS_ISDIGIT (*p))
            return LOG_RTERR (pctxt, RTERR_INVFORMAT);

         while (OS_ISDIGIT (*p)) {
            val[7]++;
            val[8] = val[8] * 10 + (OSUINT32) (*p++ - '0');
         }

         c = *p++;
      }

      for (; i < iMax; i++) {
         if (c == suffix[i]) {
            optbit[i] = TRUE;
            val[i] = value;
            break;
         }
      }

      if (i == iMax)
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);
   }

   /* Locate the least significant field present.  Set any zero-value fields
   before that to absent.
   */
   last = FALSE;
   for (i = 6; i >= 0; i--)
   {
      if (optbit[i] && !last) {
         /* This is the least significant field */
         last = TRUE;
      }
      else if (optbit[i] && val[i] == 0) {
         /* Field is present, 0, and not least-significant field. Mark absent.*/
         optbit[i] = 0;
      }
   }

   /* encode duration */
   for (i = 0; i < 8; i++) {
#ifdef _TRACE
      PU_NEWFIELD (pctxt, fieldNamePresents[i]);
#endif
      stat = pe_bit (pctxt, optbit[i]);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);
   }

   for (i = 0; i < 9; i++) {
      if (optbit[i]) {
         OSBOOL extbit = (val[i] > maxVal[i]);

#ifdef _TRACE
         PU_PUSHNAME (pctxt, fieldName[i]);
#endif

         PU_NEWFIELD (pctxt, "extension marker");
         stat = pe_bit (pctxt, extbit);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         PU_SETBITCOUNT (pctxt);


         if (extbit)
            stat = pe_UnconsUnsigned (pctxt, val[i]);
         else
            stat = pe_ConsUnsigned (pctxt, val[i], (i == 7) ? 1 : 0,
                                    maxVal[i]);

         if (stat != 0) return LOG_RTERR (pctxt, stat);

         PU_POPNAME (pctxt);
      }
   }

   return (int) (p - string); /* parsed size */
}

EXTPERMETHOD int pe_Duration (OSCTXT* pctxt, const char* string, OSBOOL rec)
{
   const char* p;
   int stat;
   OS_UNUSED_ARG(rec);

   p = string;

   if (*p == 'R') {
      OSBOOL optbit = FALSE;
      OSINT32 recurrence = 0;

      p++;

      if (OS_ISDIGIT (*p)) {
         optbit = TRUE;

         while (OS_ISDIGIT (*p))
            recurrence = recurrence * 10 + (OSINT32) (*p++ - '0');
      }

      if (*p++ != '/')
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);

      PU_PUSHNAME (pctxt, "recurrence");

      PU_NEWFIELD (pctxt, "recurrencePresent");
      stat = pe_bit (pctxt, optbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);

      if (optbit) {
         /* encode recurrence */
         stat = pe_UnconsInteger (pctxt, recurrence);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      PU_POPNAME (pctxt);
   }

   stat = pe_DurationFromStr (pctxt, p);
   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
