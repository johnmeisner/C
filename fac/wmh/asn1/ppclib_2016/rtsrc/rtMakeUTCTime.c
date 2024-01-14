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
#include "rtxsrc/rtxMemory.h"

EXTRTMETHOD int rtMakeUTCTime
(OSCTXT* pctxt, const OSNumDateTime* dateTime,
 char** outdata, size_t outdataSize)
{
   char dateTimeStr[100];
   size_t dateTimeStrLen;

   if (!rtxDateTimeIsValid (dateTime))
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

   /* Year is more restricted in UTC time */
   if (dateTime->year < 1950 || dateTime->year > 2049)
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

   /* YYYYMMDDHH[MM[SS[(.|,)SSSS]]][TZD] */
   os_snprintf (dateTimeStr, 100, "%.2d%.2d%.2d%.2d%.2d",
            dateTime->year%100, dateTime->mon, dateTime->day,
            dateTime->hour, dateTime->min);

   if (dateTime->sec != 0) {
      char tmpstr[32];
      os_snprintf (tmpstr, 32, "%.2d", (int)dateTime->sec);
      rtxStrncat (dateTimeStr, 100, tmpstr, 2);
   }

   /* the tz_flag should always be set for a UTCTime. */
   if ( !dateTime->tz_flag )
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);

   OSRTASSERT(dateTime->tz_flag);

   if (dateTime->tzo == 0) {
      rtxStrncat (dateTimeStr, 100, "Z", 1);
   }
   else {
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

EXTRTMETHOD void normalizeTimeZone(OSNumDateTime* pvalue)
{
   OSINT32 tmp, carry, tmpdays;
   tmp = pvalue->min - pvalue->tzo;
   pvalue->min = (OSINT8)(MODULO(tmp,60));
   carry = FQUOTIENT(tmp,60);
   tmp = pvalue->hour + carry;
   pvalue->hour = (OSINT8)(MODULO(tmp,24));
   carry = FQUOTIENT(tmp,24);
   tmpdays = pvalue->day + carry;

   for (;;) {
      if(tmpdays < 1){
         int tmon = MODULO_RANGE(pvalue->mon-1, 1, 13);
         int tyr  = pvalue->year + FQUOTIENT_RANGE(pvalue->mon-1, 1, 13);
         if (tyr == 0)
            tyr--;
         tmpdays += MAX_DAY_INMONTH(tyr, tmon);
         carry = -1;
      } else if (tmpdays > MAX_DAY_INMONTH(pvalue->year, pvalue->mon)){
         tmpdays = tmpdays - MAX_DAY_INMONTH(pvalue->year, pvalue->mon);
         carry = 1;
      } else
         break;
      tmp = pvalue->mon + carry;
      pvalue->mon = (OSINT8)(MODULO_RANGE(tmp,1,13));
      pvalue->year = (OSINT32)(pvalue->year + FQUOTIENT_RANGE(tmp,1,13));
   }
   pvalue->day = (OSINT8)tmpdays;
   pvalue->tzo = 0;
}

