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

EXTPERMETHOD int pd_Interval
(OSCTXT* pctxt, const char** string, OSBOOL rec,
 unsigned startFlags, unsigned endFlags)
{
   int stat;
   char buff[12 + 8 * 11 + 3 + 43]; /* Rn/P 11 nums + datetime */
   char* p = buff;
   char* str;
   int i;
   OSUINT32 flags;

   if ((startFlags & OSDURATION) && (endFlags & OSDURATION))
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

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

   flags = startFlags;

   for (i = 0; i < 2; i++) {
      PU_PUSHNAME (pctxt, (i) ? "end" : "start");

      if (flags & OSDURATION) {
         stat = pd_DurationToStrn (pctxt, p, sizeof(buff) - 12);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += stat;
      }
      else {
         if (flags & (OSYEAR|OSCENTURY|OSMONTH|OSWEEK|OSDAY)) {
            stat = pd_DateToStrn (pctxt, p, 18, flags);
            if (stat < 0) return LOG_RTERR (pctxt, stat);

            p += stat;

            if (flags & (OSHOURS|OSMINUTES|OSSECONDS))
               *p++ = 'T';
         }

         if (flags & (OSHOURS|OSMINUTES|OSSECONDS)) {
            stat = pd_TimeToStrn (pctxt, p, 26, flags);
            if (stat < 0) return LOG_RTERR (pctxt, stat);

            p += stat;
         }
      }

      if (i == 0) {
         *p++ = '/';
         flags = endFlags;
      }

      PU_POPNAME (pctxt);
   }

   /* create interval string */
   stat = (int) (p - buff);
   str = (char*) rtxMemAlloc (pctxt, (size_t) stat + 1);
   if (0 == str) return LOG_RTERR (pctxt, RTERR_NOMEM);

   OSCRTLSAFEMEMCPY (str, (size_t)(stat+1), buff, (size_t)(stat+1));
   *string = str;

   return 0;
}
