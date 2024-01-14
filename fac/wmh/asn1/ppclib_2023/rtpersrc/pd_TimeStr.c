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

int pd_TimeToStrn (OSCTXT* pctxt, char* string, size_t strSz, OSUINT32 flags)
{
   char* p;
   OSUINT8 hours;
   OSUINT8 minutes;
   OSUINT8 seconds;
   OSUINT32 fraction;
   OSBOOL extbit;
   int stat;
   OS_UNUSED_ARG(strSz);

   p = string;
   hours = 0;
   minutes = 0;
   seconds = 0;
   fraction = 0;
   stat = 0;

   /* decode hours */
   PU_PUSHNAME (pctxt, "hours");
   stat = pd_ConsUInt8 (pctxt, &hours, OSUINTCONST(0), OSUINTCONST(24));
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   if (flags & OSMINUTES) {
      /* decode minutes */
      PU_PUSHNAME (pctxt, "minutes");
      stat = pd_ConsUInt8 (pctxt, &minutes, OSUINTCONST(0), OSUINTCONST(59));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSSECONDS) {
      /* decode seconds */
      PU_PUSHNAME (pctxt, "seconds");
      stat = pd_ConsUInt8 (pctxt, &seconds, OSUINTCONST(0), OSUINTCONST(60));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   if (flags & OSFRACTION) {
      /* decode fraction */
      PU_PUSHNAME (pctxt, "fraction");
      PU_NEWFIELD (pctxt, "extension marker");
      stat = PD_BIT (pctxt, &extbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_SETBITCOUNT (pctxt);

      if (extbit)
         stat = pd_UnconsUnsigned (pctxt, &fraction);
      else
         stat = pd_ConsUnsigned (pctxt, &fraction, OSUINTCONST(0),
                                 OSUINTCONST(999));

      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);
   }

   /* create time string */
   if (string) {
      stat = rtxUIntToCharStr (hours, p, 3, '0');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      p += 2;

      if (flags & OSMINUTES) {
         *p++ = ':';

         stat = rtxUIntToCharStr (minutes, p, 3, '0');
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += 2;
      }

      if (flags & OSSECONDS) {
         *p++ = ':';

         stat = rtxUIntToCharStr (seconds, p, 3, '0');
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += 2;
      }

      if (flags & OSFRACTION) {
         *p++ = '.';

         stat = rtxUIntToCharStr (fraction, p, (flags & OSFRACTION) + 1, 0);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += stat;
      }

      if (flags & OSUTC)
         *p++ = 'Z';
      else if (flags & OSDIFF) {
         stat = pd_TimeDiffToStrn (pctxt, p, 7);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += stat;
      }

      *p = 0;
   }

   return (int) (p - string);
}

int pu_GetDateTimeStrSize (OSUINT32 flags);

EXTPERMETHOD int pd_TimeStr
(OSCTXT* pctxt, const char** string, OSUINT32 flags)
{
   char* timestr = 0;
   size_t strSz = 0;
   int stat;

   /* create date string */
   if (string) {
      strSz = pu_GetDateTimeStrSize(flags) + 1;
      timestr = (char*) rtxMemAlloc (pctxt, strSz);
      if (0 == timestr) return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   else timestr = 0;

   stat = pd_TimeToStrn (pctxt, timestr, strSz, flags);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (string) *string = timestr;

   return 0;
}