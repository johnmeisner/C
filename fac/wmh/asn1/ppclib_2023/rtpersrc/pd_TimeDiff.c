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

EXTPERMETHOD int pd_TimeDiffToStrn
(OSCTXT* pctxt, char* string, size_t strSz)
{
   char* p = string;
   OSINT32 hours;
   OSUINT32 hoursSign;
   OSUINT8 minutes = 0;
   OSBOOL minutesPresent;
   size_t sz = 4; /* sHH */
   int stat = 0;

   PU_PUSHNAME (pctxt, "timeDiff");

   /* decode minutes presents flag */
   PU_NEWFIELD (pctxt, "minutesPresent");
   stat = PD_BIT (pctxt, &minutesPresent);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_SETBITCOUNT (pctxt);

   /* decode sign */
   PU_PUSHNAME (pctxt, "sign");
   stat = pd_ConsWholeNumber (pctxt, &hoursSign, 2);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   /* decode hours */
   PU_PUSHNAME (pctxt, "hours");
   stat = pd_ConsInteger (pctxt, &hours, 0, 15);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   PU_POPNAME (pctxt);

   if (0 != hoursSign) hours = -hours;

   if (minutesPresent) {
      /* decode minutes */
      PU_PUSHNAME (pctxt, "minutes");
      stat = pd_ConsUInt8 (pctxt, &minutes, OSUINTCONST(1), OSUINTCONST(59));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      PU_POPNAME (pctxt);

      sz += 3; /* :MM */
   }

   PU_POPNAME (pctxt);

   /* create time difference string */
   if (string) {
      if (strSz < sz)
         return LOG_RTERR (pctxt, RTERR_BUFOVFLW);

      if (hours < 0) {
         *p++ = '-';
         hours = -hours;
      }
      else
         *p++ = '+';

      stat = rtxIntToCharStr (hours, p, 3, '0');
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      p += 2;

      if (minutesPresent) {
         *p++ = ':';

         stat = rtxUIntToCharStr (minutes, p, 3, '0');
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         p += 2;
      }
   }

   return (int) (p - string);
}
