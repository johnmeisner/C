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

#include <stdio.h>
#include <stdlib.h>
#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUtil.h"

 /**
 * Append the time of day part of the CER/DER canonical formatted
 string for the time point, with given settings.  Only call this if hours is
 present.  Be sure to have ASN1TIMEFIELD_DIFF clear if difference should be
 omitted.
 */
void append_tod(char* buf, OSSIZE bufsiz, const ASN1DateTime* pointVal,
                  OSUINT16 settings)
{
   OSSIZE curlen;    /* temporary; current length of string */

   curlen = OSCRTLSTRLEN(buf);
   os_snprintf(buf + curlen, bufsiz - curlen,
      "%.2u", pointVal->hour);

   if (settings & ASN1TIMEFIELD_MIN)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         ":%.2u", pointVal->min);

      if (settings & ASN1TIMEFIELD_SEC)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            ":%.2u", pointVal->sec);
      }
   }

   if (pointVal->fracDigits > 0)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         ".%.*u", pointVal->fracDigits, pointVal->fracValue);
   }

   if (settings & ASN1TIMEFIELD_DIFF)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         "+%.2d", pointVal->timezoneDifffHours);

      if (pointVal->timezoneDiffMins != 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            ":%.2d", pointVal->timezoneDiffMins);
      }
   }
   else if (settings & ASN1TIMEFIELD_Z) rtxStrncat(buf, bufsiz, "Z", 1);
}


/**
 * Return length of the time of day part of the CER/DER canonical formatted
   string for the time point, with given settings.  Only call this if hours is
   present.  Be sure to have ASN1TIMEFIELD_DIFF clear if difference should be
   omitted.
 */
OSSIZE get_tod_len(const ASN1DateTime* pointVal, OSUINT16 settings)
{
   OSSIZE result = 2;   /* 'hh' */

   if (settings & ASN1TIMEFIELD_MIN)
   {
      result += 3;   /* ':mm' */
      if (settings & ASN1TIMEFIELD_SEC)
      {
         result += 3;   /* ':ss' */
      }
   }

   result += pointVal->fracDigits;

   if ( settings & ASN1TIMEFIELD_DIFF)
   {
      result += 3;   /* '+/-hh' */
      if ( pointVal->timezoneDiffMins != 0 )
         result += 3; /* ':mm' */
   }
   else if ( settings & ASN1TIMEFIELD_Z) result += 1;

   return result;
}


/**
* Append the date part of the formatted string for the time point,
with given settings.  Only call this if century or year are present.
*/
void append_date(char* buf, OSSIZE bufsiz, const ASN1DateTime* pointVal,
                     OSUINT16 settings)
{
   OSSIZE curlen;    /* temporary; current length of string */
   if (settings & ASN1TIMEFIELD_CENTURY)
   {
      curlen = OSCRTLSTRLEN(buf);
      if (pointVal->century >= -99 && pointVal->century <= 99)
      {
         os_snprintf(buf + curlen, bufsiz - curlen,
            "%.2d", pointVal->century);
      }
      else
      {
         os_snprintf(buf + curlen, bufsiz - curlen,
            "%d", pointVal->century);
      }
      rtxStrncat(buf, bufsiz, "C", 1);
   }
   else
   {
      /* Must have year. */
      curlen = OSCRTLSTRLEN(buf);
      if (pointVal->year >= -9999 && pointVal->year <= 9999)
         os_snprintf(buf + curlen, bufsiz - curlen,
            "%.4d", pointVal->year);
      else
      {
         os_snprintf(buf + curlen, bufsiz - curlen,
            "%d", pointVal->year);
      }
   }

   if (settings & ASN1TIMEFIELD_WEEK)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         "-W%.2u", pointVal->week);

      if (settings & ASN1TIMEFIELD_DAY)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            "-%.1u", pointVal->day);
      }
   }
   else if (settings & ASN1TIMEFIELD_MONTH)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         "-%.2u", pointVal->month);
      if (settings & ASN1TIMEFIELD_DAY)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            "-%.2u", pointVal->day);
      }
   }
   else if (settings & ASN1TIMEFIELD_DAY)
   {
      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         "-%.3u", pointVal->day);
   }
}


/**
 * Return length of the date part of the formatted string for the time point,
   with given settings.  Only call this if century or year are present.
 */
OSSIZE get_date_len(const ASN1DateTime* pointVal, OSUINT16 settings)
{
   OSSIZE result = 0;

   if (settings & ASN1TIMEFIELD_CENTURY)
   {
      result += 1;      /* 'C' */
      if (pointVal->century >= 0 && pointVal->century <= 99)
         result += 2;   /* basic or proleptic */
      else
      {
         /* negative or Ln */
         result += 1;   /* sign */
         if (pointVal->century < 0 && pointVal->century >= -99)
            result += 2;
         else if (pointVal->century < 0)
            result += rtxLog10Floor((OSUINT32)-pointVal->century) + 1;
         else
            result += rtxLog10Floor((OSUINT32)pointVal->century) + 1;
      }
      return result;
   }
   else
   {
      /* Must have year. */
      if (pointVal->year >= 0 && pointVal->year <= 9999)
         result += 4;   /* basic or proleptic */
      else
      {
         /* negative or Ln */
         result += 1;   /* sign */
         if (pointVal->year < 0 && pointVal->year >= -9999)
            result += 4;
         else if (pointVal->year < 0)
            result += rtxLog10Floor((OSUINT32)-pointVal->year) + 1;
         else
            result += rtxLog10Floor((OSUINT32)pointVal->year) + 1;
      }
   }

   if (settings & ASN1TIMEFIELD_WEEK)
   {
      result += 4;   /* '-Www' */
      if ( settings & ASN1TIMEFIELD_DAY )
         result += 2;   /* '-D' */
   }
   else if (settings & ASN1TIMEFIELD_MONTH)
   {
      result += 3;   /* '-MM' */
      if (settings & ASN1TIMEFIELD_DAY)
         result += 3;    /* '-DD' */
   }
   else if (settings & ASN1TIMEFIELD_DAY)
   {
      result += 4;   /* -DDD*/
   }

   return result;
}


/**
* Append CER/DER canonically formatted string for given time point,
with given settings.  Be sure to have ASN1TIMEFIELD_DIFF clear if the
time difference should be omitted.
*/
void append_point(char* buf, OSSIZE bufsiz, const ASN1DateTime* pointVal,
                  OSUINT16 settings)
{
   if (settings & (ASN1TIMEFIELD_CENTURY | ASN1TIMEFIELD_YEAR))
   {
      append_date(buf, bufsiz, pointVal, settings);

      if (settings & ASN1TIMEFIELD_HOUR)
      {
         rtxStrncat(buf, bufsiz, "T", 1);
      }
   }

   if (settings & ASN1TIMEFIELD_HOUR)
   {
      append_tod(buf, bufsiz, pointVal, settings);
   }
}


/**
 * Return length of CER/DER canonically formatted string for given time point,
   with given settings.  Be sure to have ASN1TIMEFIELD_DIFF clear if the
   time difference should be omitted.
 */
OSSIZE get_point_len(const ASN1DateTime* pointVal, OSUINT16 settings)
{
   OSSIZE result = 0;

   if (settings & (ASN1TIMEFIELD_CENTURY | ASN1TIMEFIELD_YEAR))
   {
      result += get_date_len(pointVal, settings);

      if (settings & ASN1TIMEFIELD_HOUR)
      {
         result += 1;   /* 'T' */
      }
   }

   if (settings & ASN1TIMEFIELD_HOUR)
   {
      result += get_tod_len(pointVal, settings);
   }

   return result;
}

#define DIGITS(x) ((x == 0) ? 1 : rtxLog10Floor(x) + 1)

/**
* Append duration to buffer.
*/
void append_duration(char* buf, OSSIZE bufsiz, const ASN1Duration* duration)
{
   OSSIZE curlen;
   rtxStrncat(buf, bufsiz, "P", 1);

   if (duration->precision == 3)
   {
      char* format = duration->fracDigits > 0 ?
         "%u.%.*uW" : "%uW";

      curlen = OSCRTLSTRLEN(buf);
      os_snprintf(buf + curlen, bufsiz - curlen,
         format, duration->weeks, duration->fracDigits, duration->fracValue);
   }
   else {
      /* Include a component if it is non-zero OR precision calls for it.
         formats provide format strings to use for each of the fields,
         both with and without a fraction part.  Exactly one of the fields
         should include a fraction part if the fraction digits are non-zero.
      */
      char* formats[][2] =
      {
         {"%uY", "%u.%.*uY"},
         { "%uM", "%u.%.*uM" },
         { "%uD", "%u.%.*uD" },
         { "%uH", "%u.%.*uH" },
         { "%uM", "%u.%.*uM" },
         { "%uS", "%u.%.*uS" },
      };

      if (duration->precision == 1 || duration->years > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[0][duration->precision == 1 && duration->fracDigits > 0],
            duration->years, duration->fracDigits, duration->fracValue);
      }
      if (duration->precision == 2 || duration->months > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[1][duration->precision == 2 && duration->fracDigits > 0],
            duration->months, duration->fracDigits, duration->fracValue);
      }
      if (duration->precision == 4 || duration->days > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[2][duration->precision == 4 && duration->fracDigits > 0],
            duration->days, duration->fracDigits, duration->fracValue);
      }

      if (duration->precision >= 5 || duration->hours > 0 ||
         duration->mins > 0 || duration->secs > 0)
      {
         rtxStrncat(buf, bufsiz, "T", 1);
      }

      if (duration->precision == 5 || duration->hours > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[3][duration->precision == 5 && duration->fracDigits > 0],
            duration->hours, duration->fracDigits, duration->fracValue);
      }
      if (duration->precision == 6 || duration->mins > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[4][duration->precision == 6 && duration->fracDigits > 0],
            duration->mins, duration->fracDigits, duration->fracValue);
      }
      if (duration->precision == 7 || duration->secs > 0)
      {
         curlen = OSCRTLSTRLEN(buf);
         os_snprintf(buf + curlen, bufsiz - curlen,
            formats[5][duration->precision == 7 && duration->fracDigits > 0],
            duration->secs, duration->fracDigits, duration->fracValue);
      }
   }
}


/**
 * Return length of formatted string for duration with given settings.
 */
OSSIZE get_duration_len(const ASN1Duration* duration)
{
   OSSIZE result = 1;   /* 'P' */
   if (duration->precision == 3)
   {
      if ( duration->weeks == 0 ) result += 1;
      else result += rtxLog10Floor(duration->weeks) + 1;
      result += 1;   /* 'W' */
   }
   else {
      /* Include a component if it is non-zero OR precision calls for it.
         Include +1 for the field designator.
       */
      if (duration->precision == 1 || duration->years > 0 )
         result += DIGITS(duration->years) + 1;
      if (duration->precision == 2 || duration->months > 0)
         result += DIGITS(duration->months) + 1;
      if (duration->precision == 4 || duration->days > 0)
         result += DIGITS(duration->days) + 1;
      if (duration->precision == 5 || duration->hours > 0)
         result += DIGITS(duration->hours) + 1;
      if (duration->precision == 6 || duration->mins > 0)
         result += DIGITS(duration->mins) + 1;
      if (duration->precision == 7 || duration->secs > 0)
         result += DIGITS(duration->secs) + 1;
      if (duration->precision >= 5 || duration->hours > 0 ||
         duration->mins > 0 || duration->secs > 0)
      {
         result += 1;   /* 'T' */
      }
   }

   if (duration->fracDigits > 0) {
      result += duration->fracDigits + 1; /* '.nnn' */
   }

   return result;
}


/**
 * Return length of formatted string for timeVal.
 */
OSSIZE get_time_len(const ASN1Time* timeVal)
{
   OSSIZE result = 0;
   if ( timeVal->settings & ASN1TIMEFIELD_RECUR_INF)
      result += 2;      /* 'R/' */
   else if (timeVal->settings & (ASN1TIMEFIELD_INTERVAL_SE |
      ASN1TIMEFIELD_DURATION |
      ASN1TIMEFIELD_INTERVAL_SD |
      ASN1TIMEFIELD_INTERVAL_DE) &&
      timeVal->val.interval.recurrences > 0)
   {
      result += 2;   /* 'R/' */
      result += rtxLog10Floor(timeVal->val.interval.recurrences) + 1;
   }

   if (timeVal->settings & (ASN1TIMEFIELD_INTERVAL_SE))
   {
      OSINT16 endSettings = timeVal->settings;

      result += 1;   /* '/' */
      result += get_point_len(&timeVal->val.interval.spec.se.start,
                                    timeVal->settings);
      if (timeVal->val.interval.spec.se.start.timezoneDifffHours ==
         timeVal->val.interval.spec.se.end.timezoneDifffHours &&
         timeVal->val.interval.spec.se.start.timezoneDiffMins ==
         timeVal->val.interval.spec.se.end.timezoneDiffMins)
      {
         /* omit duplicate diff */
         endSettings = endSettings & ~ASN1TIMEFIELD_DIFF;
      }
      result += get_point_len(&timeVal->val.interval.spec.se.end,
                                 endSettings);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_INTERVAL_SD)
   {
      result += 1;   /* '/' */
      result += get_point_len(&timeVal->val.interval.spec.sd.start,
         timeVal->settings);
      result += get_duration_len(&timeVal->val.interval.spec.sd.duration);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_INTERVAL_DE)
   {
      result += 1;   /* '/' */
      result += get_point_len(&timeVal->val.interval.spec.de.end,
         timeVal->settings);
      result += get_duration_len(&timeVal->val.interval.spec.de.duration);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_DURATION)
   {
      result += get_duration_len(&timeVal->val.interval.spec.duration);
   }
   else
   {
      result += get_point_len(&timeVal->val.dateTime, timeVal->settings);
   }

   return result;
}


EXTRTMETHOD int rtMakeTime
(OSCTXT* pctxt, const ASN1Time* timeVal,
 char** outdata, size_t outdataSize)
{
   OSSIZE curlen;    /* current length; temporary use. */
   OSSIZE len = get_time_len(timeVal) + 1;  /* +1 for null term */

   if (outdataSize > 0 && outdataSize < len )
   {
      return LOG_RTERR(pctxt, RTERR_STROVFLW);
   }
   else if (outdataSize == 0)
   {
      /* Allocate the required space. */
      *outdata = (char*)rtxMemAlloc(pctxt, len);
      if (0 == *outdata)
         return LOG_RTERR(pctxt, RTERR_NOMEM);
   }
   else
   {
      len = outdataSize;
   }


   /* Now, build the string */
   (*outdata)[0] = 0;

   if (timeVal->settings & ASN1TIMEFIELD_RECUR_INF)
      rtxStrncat(*outdata, len, "R/", 2);
   else if (timeVal->settings & (ASN1TIMEFIELD_INTERVAL_SE |
      ASN1TIMEFIELD_DURATION |
      ASN1TIMEFIELD_INTERVAL_SD |
      ASN1TIMEFIELD_INTERVAL_DE) &&
      timeVal->val.interval.recurrences > 0)
   {
      rtxStrncat(*outdata, len, "R", 1);
      curlen = OSCRTLSTRLEN(*outdata);
      os_snprintf(*outdata + curlen, len - curlen,
                  "%d", timeVal->val.interval.recurrences);
      rtxStrncat(*outdata, len, "/", 1);
   }

   if (timeVal->settings & (ASN1TIMEFIELD_INTERVAL_SE))
   {
      OSINT16 endSettings = timeVal->settings;

      append_point(*outdata, len, &timeVal->val.interval.spec.se.start,
                     timeVal->settings);
      rtxStrncat(*outdata, len, "/", 1);

      if (timeVal->val.interval.spec.se.start.timezoneDifffHours ==
         timeVal->val.interval.spec.se.end.timezoneDifffHours &&
         timeVal->val.interval.spec.se.start.timezoneDiffMins ==
         timeVal->val.interval.spec.se.end.timezoneDiffMins)
      {
         /* omit duplicate diff */
         endSettings = endSettings & ~ASN1TIMEFIELD_DIFF;
      }
      append_point(*outdata, len, &timeVal->val.interval.spec.se.end,
                     endSettings);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_INTERVAL_SD)
   {
      append_point(*outdata, len, &timeVal->val.interval.spec.sd.start,
                     timeVal->settings);
      rtxStrncat(*outdata, len, "/", 1);
      append_duration(*outdata, len, &timeVal->val.interval.spec.sd.duration);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_INTERVAL_DE)
   {
      append_duration(*outdata, len, &timeVal->val.interval.spec.de.duration);
      rtxStrncat(*outdata, len, "/", 1);
      append_point(*outdata, len, &timeVal->val.interval.spec.de.end,
         timeVal->settings);
   }
   else if (timeVal->settings & ASN1TIMEFIELD_DURATION)
   {
      append_duration(*outdata, len, &timeVal->val.interval.spec.duration);
   }
   else
   {
      append_point(*outdata, len, &timeVal->val.dateTime, timeVal->settings);
   }

   return (0);
}

