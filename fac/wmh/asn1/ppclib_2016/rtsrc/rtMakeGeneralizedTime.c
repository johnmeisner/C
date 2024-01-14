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

#include <stdio.h>
#include <stdlib.h>
#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxDateTime.hh"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

EXTRTMETHOD int rtMakeGeneralizedTime
(OSCTXT* pctxt, const OSNumDateTime* dateTime,
 char** outdata, size_t outdataSize)
{
   char dateTimeStr[100];
   size_t dateTimeStrLen;
   double frac;

   if (!rtxDateTimeIsValid (dateTime))
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

   /* YYYYMMDDHH[MM[SS[(.|,)SSSS]]][TZD] */
   os_snprintf (dateTimeStr, 100, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",
            dateTime->year, dateTime->mon, dateTime->day,
            dateTime->hour, dateTime->min, (int)dateTime->sec);

   frac = dateTime->sec - (int)dateTime->sec;
   if (frac > 0.00000001) {
      char tmpbuf[32]; int i, len;
      os_snprintf (tmpbuf, 32, "%.6f", frac);
      len = (int) OSCRTLSTRLEN (tmpbuf);

      /* cut trailing zeros off frac part */
      for (i = len - 1; i > 0; i--) {
         if (tmpbuf [i] == '0')
            tmpbuf [i] = '\0';
         else
            break;
      }

      if (i > 1) {
         rtxStrncat (dateTimeStr, 100, ".", 1);
         rtxStrncat (dateTimeStr, 100, tmpbuf + 2, len - 2);
      }
   }

   if (dateTime->tz_flag && dateTime->tzo == 0) {
      rtxStrncat (dateTimeStr, 100, "Z", 1);
   }
   else if (dateTime->tz_flag) {
      char tmpbuf[32];
      char tzd = (dateTime->tzo < 0) ? '-' : '+';
      int  tzo = dateTime->tzo;
      if (tzo < 0) tzo *= -1;
      os_snprintf (tmpbuf, 32, "%c%02d%02d", tzd, tzo/60, tzo%60);
      rtxStrncat (dateTimeStr, 100, tmpbuf, 5);
   }

   dateTimeStrLen = OSCRTLSTRLEN (dateTimeStr);
   if (outdataSize == 0) {
      *outdata = (char*) rtxMemAlloc (pctxt, dateTimeStrLen+1);
      if (0 == *outdata)
         return LOG_RTERR (pctxt, RTERR_NOMEM);

      outdataSize = dateTimeStrLen+1;
   }
   else {
      if (outdataSize <= dateTimeStrLen)
         return LOG_RTERR (pctxt, RTERR_STROVFLW);
   }
   rtxStrncpy (*outdata, outdataSize, dateTimeStr, dateTimeStrLen);

   return (0);
}

