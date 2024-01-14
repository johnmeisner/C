/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

EXTPERMETHOD int pe_Interval
(OSCTXT* pctxt, const char* string, OSBOOL rec,
 unsigned startFlags, unsigned endFlags)
{
   const char* p = string;
   int stat;
   int i;
   OSUINT32 flags;

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

   flags = startFlags;

   for (i = 0; i < 2; i++) {
      PU_PUSHNAME (pctxt, (i) ? "end" : "start");

      if (flags & OSDURATION) {
         stat = pe_DurationFromStr (pctxt, p);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += stat;
      }
      else {
         if (flags & (OSYEAR|OSCENTURY|OSMONTH|OSWEEK|OSDAY)) {
            stat = pe_DateFromStr (pctxt, p, flags);
            if (stat < 0) return LOG_RTERR (pctxt, stat);

            p += stat;

            if (flags & (OSHOURS|OSMINUTES|OSSECONDS)) {
               if (*p++ != 'T')
                  return LOG_RTERR (pctxt, RTERR_INVFORMAT);
            }
         }

         if (flags & (OSHOURS|OSMINUTES|OSSECONDS)) {
            stat = pe_TimeFromStr (pctxt, p, flags);
            if (stat < 0) return LOG_RTERR (pctxt, stat);

            p += stat;
         }
      }

      if (i == 0) {
         if (*p++ != '/')
            return LOG_RTERR (pctxt, RTERR_INVFORMAT);

         flags = endFlags;
      }

      PU_POPNAME (pctxt);
   }

   return 0;
}
