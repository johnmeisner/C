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

#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDateTime.hh"

#define LOG_CANON(pctxt,msg) { \
   rtxErrAddStrParm(pctxt, msg); \
   LOG_RTERRNEW(pctxt, ASN_E_NOTCANON); \
   rtxErrSetNonFatal(pctxt); }


/**
* Parse unsigned integer from string.  There must be at least one digit.
*
* @param ppvalue *ppvalue points to the first digit of the integer.  It is
*                updated to point to the first character beyond the integer.
* @param pintval Receives the value.
* @returns 0 or negative for error.  Does not log errors.
*     RTERR_INVFORMAT if there is not a digit.
*     RTERR_TOOBIG if integer does not fit in *pintval.
*/
static int parseInt(const char** ppvalue, OSUINT32* pintval)
{
   if (!OS_ISDIGIT(**ppvalue)) return RTERR_INVFORMAT;

   *pintval = 0;

   while (OS_ISDIGIT(**ppvalue))
   {
      OSUINT32 newVal = *pintval * 10 + (**ppvalue - '0');
      if (newVal < *pintval) return RTERR_TOOBIG;
      *pintval = newVal;

      (*ppvalue)++;
   }

   return 0;
}

/**
 * Parse fraction digits.  There must be at least one fraction digit.
 * @param ppvalue *ppvalue points to the first digit of the fraction.  It is
 *                updated to point to the first character beyond the fraction.
 * @param fracDigits Receives the number of digits in the fraction.
 * @param fracValue Receives the value of the fraction.
 * @returns 0 or <0 for error.
 */
static int parseFraction(OSCTXT* pctxt, const char** ppvalue,
                     OSUINT8* fracDigits, OSUINT32* fracValue)
{
   int status;
   const char* pstart = *ppvalue;
   OSSIZE digits;

   status = parseInt(ppvalue, fracValue);
   if (status < 0) return LOG_RTERRNEW(pctxt, status);

   digits = *ppvalue - pstart;
   if ( digits > OSUINT8_MAX ) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

   *fracDigits = (OSUINT8) digits;

   return 0;
}


/**
* Parse duration unit, which consists of an integer part and an optional
* fraction part.
*
* @param ppvalue *ppvalue points to the first digit of the integer part.  It is
*                updated to point to the first character beyond the unit.
* @param pIntVal  Receives the integer part.
* @param pFracDigits Receives the number of digits in the fraction (0 if none)
* @param pFracValue Receives the value of the fraction (0 if none).
* @param checkCanon    If true, canonicity will be checked. If violated,
*                 ASN_E_NOTCANON will be returned and if logCanon is TRUE,
*                 a non-fatal error will be logged.
* @param logCanon See checkCanon.
* @returns 0 or <0 for error.
*/
static int parseDurationUnit(OSCTXT* pctxt, const char** ppvalue,
   OSUINT32* pIntVal, OSUINT8* pFracDigits, OSUINT32* pFracValue,
   OSBOOL checkCanon, OSBOOL logCanon)
{
   int status;
   const char* pstart = *ppvalue;
   OSBOOL zero = **ppvalue == '0';     /* TRUE if first digit is zero. */
   OSBOOL notcanon = FALSE;

   status = parseInt(ppvalue, pIntVal);
   if (status != 0)  return LOG_RTERRNEW(pctxt, status);

   if (zero && *ppvalue - pstart > 1)
   {
      /* Integer had leading zeros. It started with 0 and involved more than
      one digit. */
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (**ppvalue == '.' || **ppvalue == ',')
   {
      /* There is a fraction part. */
      if (checkCanon && **ppvalue == ',')
      {
         notcanon = TRUE;
         if (logCanon) LOG_CANON(pctxt, "Must use '.' for decimal point.");
      }
      (*ppvalue)++;
      status = parseFraction(pctxt, ppvalue, pFracDigits, pFracValue);
      if (status != 0) return LOG_RTERRNEW(pctxt, status);
   }
   else {
      *pFracDigits = 0;
      *pFracValue = 0;
   }

   if ( notcanon ) return ASN_E_NOTCANON;
   else return 0;
}


/* This function will parse the various hour/minute/second parts of an ASN.1
* ISO 8601 Time string.
*/
/**
 * Parse 2 digit integers from the given string.
 * @param ppvalue Pointer to string address to be parsed.  This will be
 *                   updated to point to the character beyond the parsed
 *                   input.
 * @param max     Maximum allowed value (or else an error).
 * @param pvalue  Receives the value.
 * @return 0 or negative for error.
 */
static int parseTimeUnit(OSCTXT* pctxt, const char** ppvalue,
   OSUINT8 max, OSUINT8* pvalue)
{
   int stat = 0;

   PARSE_2_DIGITS(*pvalue, (*ppvalue), stat);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (*pvalue > max)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   return stat;
}


/**
* Parses a duration from given string.
*
* If the ASN1CANON or ASN1DER flag is set, this also checks whether the
* canonical value representation is used.  If not, logCanon controls whether
* a non-fatal error is logged or not.
*
* @param ppvalue *ppvalue is the string.  This is updated to point to the
*                first character beyond matched input.  It should initially
*                point to the first character after the 'P' that a duration
*                begins with.
* @param duration  Object to receive parsed values.
*/
static int parseDuration(OSCTXT* pctxt, const char** ppvalue,
   ASN1Duration* duration, OSBOOL logCanon)
{
   OSBOOL hadT = FALSE;    /* TRUE if we've parsed the T separator. */
   int position = 0;       /* Tracks where we are among possible designators. */
   OSBOOL checkCanon = (pctxt->flags & (ASN1CANON | ASN1DER)) != 0;
   OSBOOL notcanon = FALSE;
   OSBOOL hadZeroUnit = FALSE;     /* TRUE if a prior unit value was zero. */

   duration->weeks = 0;
   duration->years = 0;
   duration->months = 0;
   duration->days = 0;
   duration->hours = 0;
   duration->mins = 0;
   duration->secs = 0;
   duration->fracDigits = 0;
   duration->fracValue = 0;
   duration->precision = 0;

   /* Must begin with a digit or T or string is not valid. */
   if (**ppvalue != 'T' && !OS_ISDIGIT(**ppvalue))
      return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

   for(;;)
   {
      if (**ppvalue == 'T')
      {
         /* T can appear at any time, but can only appear once. */
         if ( hadT ) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

         hadT = TRUE;
         (*ppvalue)++;
      }
      else if (OS_ISDIGIT(**ppvalue))
      {
         OSUINT32 value;
         int status;

         if (checkCanon && hadZeroUnit)
         {
            notcanon = TRUE;
         }

         /* Parse the unit (integer and possible fraction). */
         status = parseDurationUnit(pctxt, ppvalue, &value,
                                 &duration->fracDigits, &duration->fracValue,
                                 checkCanon, logCanon);
         if ( status != 0 ) return LOG_RTERR(pctxt, status);

         if ( value == 0 && duration->fracValue == 0 ) hadZeroUnit = TRUE;

         /* Examine the designator and apply to correct field. */

         switch(**ppvalue) {
            case 'W': {
               if ( hadT || duration->precision != 0 )
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               duration->weeks = value;
               duration->precision = 3;
               break;
            }
            case 'Y': {
               if (hadT || duration->precision >= 1)
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               duration->years = value;
               duration->precision = 1;
               break;
            }
            case 'M': {
               if ((hadT && (duration->precision >= 6)) ||
                   (!hadT && (duration->precision >= 2)))
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               if ( hadT ) {
                  duration->mins = value;
                  duration->precision = 6;
               }
               else {
                  duration->months = value;
                  duration->precision = 2;
               }
               break;
            }
            case 'D': {
               if (hadT || duration->precision >= 4)
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               duration->days = value;
               duration->precision = 4;
               break;
            }
            case 'H': {
               if (!hadT || duration->precision >= 5)
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               duration->hours = value;
               duration->precision = 5;
               break;
            }
            case 'S' : {
               if (!hadT || duration->precision >= 7)
                  return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);

               duration->secs = value;
               duration->precision = 7;
               break;
            }
            default: {
               return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
            }
         }

         (*ppvalue)++;     /* move past designator */

         if (*(*ppvalue -1) == 'W' || duration->fracDigits > 0 || position == 6)
         {
            /* This is the end of the duration string. */
            break;
         }
      }
      else {
         /* End of duration string. */
         break;
      }
   }

   if ( notcanon ) {
      /* There is only one canonical rule. */
      if (logCanon) LOG_CANON(pctxt,
         "Duration has a non-smallest unit with a zero value.")
      return ASN_E_NOTCANON;
   }
   else return 0;
}


/**
* Parses a time-of-day from given string.
*
* If the ASN1CANON or ASN1DER flag is set, this also checks whether the
* canonical value representation is used.  If not, logCanon controls whether
* a non-fatal error is logged or not.
*
* @param ppvalue Pointer to pointer to string.  This is updated to point to
*                first character beyond matched input.
* @param timeVal  Object to receive parsed values.
* @param flags Updated to reflect the fields that were parsed.
*/
static int parseTimeOfDay(OSCTXT* pctxt, const char** ppvalue,
   ASN1DateTime* timeVal, OSUINT16* pflags,
   OSBOOL logCanon)
{
   int stat = 0;
   OSBOOL checkCanon = (pctxt->flags & (ASN1CANON|ASN1DER)) != 0;
   OSBOOL notcanon = FALSE;

   timeVal->hour = 0;
   timeVal->min = 0;
   timeVal->sec = 0;
   timeVal->fracDigits = 0;
   timeVal->fracValue = 0;
   timeVal->timezoneDifffHours = 0;
   timeVal->timezoneDiffMins = 0;

   stat = parseTimeUnit(pctxt, ppvalue, 24, &timeVal->hour);
   if (stat != 0) return LOG_RTERR(pctxt, stat);

   *pflags  |= ASN1TIMEFIELD_HOUR;

   if (**ppvalue == ':')
   {
      /* Minutes */
      (*ppvalue)++;
      stat = parseTimeUnit(pctxt, ppvalue, 59, &timeVal->min);
      if (stat != 0 ) return LOG_RTERR(pctxt, stat);

      *pflags |= ASN1TIMEFIELD_MIN;
   }

   if (**ppvalue == ':')
   {
      /* Seconds */
      (*ppvalue)++;
      stat = parseTimeUnit(pctxt, ppvalue, 59, &timeVal->sec);
      if (stat != 0) return LOG_RTERR(pctxt, stat);

      *pflags |= ASN1TIMEFIELD_SEC;
   }

   if (**ppvalue == '.' || **ppvalue ==',')
   {
      /* Fraction of hours, minutes, or seconds. */
      if (checkCanon && **ppvalue == ',')
      {
         notcanon = TRUE;
         if (logCanon) LOG_CANON(pctxt, "Must use '.' for decimal point.");
      }

      (*ppvalue)++;
      stat = parseFraction(pctxt, ppvalue, &timeVal->fracDigits,
                           &timeVal->fracValue);
      if (stat != 0) return LOG_RTERR(pctxt, stat);
   }

   if (**ppvalue == '-' || **ppvalue == '+')
   {
      /* Timezone difference. */
      (*ppvalue)++;
      stat = parseTimeUnit(pctxt, ppvalue, 14,
                           (OSUINT8*)&timeVal->timezoneDifffHours);
      if (stat != 0) return LOG_RTERR(pctxt, stat);

      *pflags |= ASN1TIMEFIELD_DIFF;

      if (**ppvalue == ':')
      {
         /* Timezone diff includes minutes */
        (*ppvalue)++;

         stat = parseTimeUnit(pctxt, ppvalue, 59, &timeVal->timezoneDiffMins);
         if (stat < 0) return LOG_RTERR(pctxt, stat);

         if (checkCanon && timeVal->timezoneDiffMins == 0)
         {
            notcanon = TRUE;
            if ( logCanon ) LOG_CANON(pctxt,
               "Timezone difference should not include zero minutes.");
         }
      }
   }
   else if (**ppvalue == 'Z')
   {
      /* UTC Time indicator */
      *pflags |= ASN1TIMEFIELD_Z;
      (*ppvalue)++;
   }

   if ( notcanon ) return ASN_E_NOTCANON;
   else return 0;
}


/**
* Parses a date from given string.
*
* If the ASN1CANON or ASN1DER flag is set, this also checks whether the
* canonical value representation is used.  If not, logCanon controls whether
* a non-fatal error is logged or not.
*
* @param pvalue Pointer to pointer to string.  This is updated to point to
*                first character beyond matched input.
* @param timeVal  Object to receive parsed values.
* @param flags Updated to reflect the fields that were parsed.
*/
static int parseDate (OSCTXT* pctxt, const char** pvalue,
                        ASN1DateTime* timeVal, OSUINT16* flags,
                        OSBOOL logCanon)
{
   OSUINT8 digit_count;
   OSBOOL week;
   const char* p;
   OSBOOL negative;
   OSINT32 intval;
   OS_UNUSED_ARG(logCanon);

   digit_count = 0;
   week = FALSE;
   p = *pvalue;
   negative = FALSE;

   timeVal->century = 0;
   timeVal->year = 0;
   timeVal->week = 0;
   timeVal->month = 0;
   timeVal->day = 0;

   /* First character must be "+", "-", or a digit... */
   if ( *p == '+') p++;
   else if (*p == '-')
   {
      p++;
      negative = TRUE;
   }
   else if (!OS_ISDIGIT(*p))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   /* Walk through year. */
   intval = 0;
   while (OS_ISDIGIT(*p))
   {
      OSINT32 newval = intval * 10 + *p - '0';
      if ( newval < intval )
         return LOG_RTERR(pctxt, RTERR_TOOBIG); /* overflow */
      intval = newval;
      p++;
   }

   if ( negative ) intval *= -1;

   if (*p == 'C')
   {
      timeVal->century = intval;
      *flags = ASN1TIMEFIELD_CENTURY;
      *pvalue = p + 1;
      return 0;
   }
   else {
      timeVal->year = intval;
      *flags = ASN1TIMEFIELD_YEAR;
   }

   if (*p == '-')
   {
      /* Year to be followed by W, M, or D. Advance past hyphen. */
      p++;
   }
   else {
      /* Nothing more expected. */
      *pvalue = p;
      return 0;
   }

   /* Check for "W" week indicator. */
   if (*p == 'W')
   {
      week = TRUE;
      p++;
   }

   /* Walk through month/week/day. */
   intval = 0;
   digit_count = 0;
   while (OS_ISDIGIT(*p))
   {
      OSINT32 newval = intval * 10 + *p - '0';
      if (newval < intval)
         return LOG_RTERR(pctxt, RTERR_TOOBIG); /* overflow */
      intval = newval;
      p++;
      digit_count++;
   }

   if (week)
   {
      if (digit_count == 2)
      {
         *flags |= ASN1TIMEFIELD_WEEK;
         timeVal->week = (OSUINT8)intval;
      }
      else return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }
   else if (digit_count == 2)
   {
      /* month */
      *flags |= ASN1TIMEFIELD_MONTH;
      timeVal->month = (OSUINT8)intval;
   }
   else if (digit_count == 3)
   {
      /* day */
      *flags |= ASN1TIMEFIELD_DAY;
      *pvalue = p;
      return 0;      /* parsed YD */
   }
   else {
      /* Wrong number of digits for either month or day */
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (*p == '-')
   {
      /* week or month to be followed by day */
      p++;
   }

   /* Walk through day/day-of-week; this might not be present. */
   digit_count = 0;
   intval = 0;
   while (OS_ISDIGIT(*p))
   {
      OSINT32 newval = intval * 10 + *p - '0';
      if (newval < intval)
         return LOG_RTERR(pctxt, RTERR_TOOBIG); /* overflow */
      intval = newval;
      p++;
      digit_count++;
   }

   if (((*flags & ASN1TIMEFIELD_WEEK) && digit_count == 1) ||
      ((*flags & ASN1TIMEFIELD_MONTH) && digit_count == 2))
   {
      /* Day of week uses 1 digit; day of month uses 2 digits. */
      *flags |= ASN1TIMEFIELD_DAY;
      timeVal->day = (OSUINT8)intval;
   }
   else if ( digit_count != 0 ) {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   *pvalue = p;
   return 0;
}


/**
 * Parses a point (date, time-of-day, or both) from given string.
 * If the ASN1CANON or ASN1DER flag is set, this also checks whether the
 * canonical value representation is used.  If not, logCanon controls whether
 * a non-fatal error is logged or not.
 *
 * @param pvalue Pointer to pointer to string.  This is updated to point to
 *                first character beyond matched input.
 * @param timeVal Receives parsed values.
 * @param flags Updated to reflect the fields that were parsed.
 */
static int parsePoint (OSCTXT* pctxt, const char** pvalue,
                        ASN1DateTime* timeVal, OSUINT16* flags,
                        OSBOOL logCanon)
{
   const char* p = *pvalue;
   OSBOOL notcanon = FALSE;
   int status;

   /* A date must begin with one of the following, and none of these could
      be a time-of-day:
      + or -
      two digits followed by C
      three or more digits
   */
   if (p[0] == '+' || p[0] == '-' ||
      ( OS_ISDIGIT(p[0]) && OS_ISDIGIT(p[1]) &&
         (OS_ISDIGIT(p[2]) || p[2] == 'C') ) )
   {
      status = parseDate(pctxt, pvalue, timeVal, flags, logCanon);
      if ( status == ASN_E_NOTCANON ) {
         notcanon = TRUE;
         status = 0;
      }
      else if ( status != 0 ) return LOG_RTERR(pctxt, status);

      if (**pvalue == 'T')
         *pvalue += 1;        /* time of day should follow */
      else return 0;          /* no time of day to follow */
   }

   status = parseTimeOfDay(pctxt, pvalue, timeVal, flags, logCanon);
   if ( status != 0 && status != ASN_E_NOTCANON )
      return LOG_RTERR(pctxt, status);

   if ( notcanon ) status = ASN_E_NOTCANON;
   return status;
}



EXTRTMETHOD int rtParseTime
(OSCTXT* pctxt, const char* value, ASN1Time* timeVal, OSBOOL logCanon)
{
   const char* p = value;
   OSBOOL checkCanon = (pctxt->flags & (ASN1CANON | ASN1DER)) != 0;
   OSBOOL notcanon = FALSE;   /* true if value is non-canonical */
   int status;
   ASN1DateTime* pdatetime;   /* Where to put time point data. We must take
                                 care not to set values into more than
                                 one alternative of the union. */
   OSBOOL finiteRecurs = FALSE;
   timeVal->settings = 0;


   if ( *p == 'R' ) {
      /* Recurring interval. */
      pdatetime = &timeVal->val.interval.spec.se.start;

      p++;
      if (*p == '/') {
         /* Infinite recurrences. */
         timeVal->settings |= ASN1TIMEFIELD_RECUR_INF;
         p++;
      }
      else {
         /* Parse recurrences */
         status = parseInt(&p, &timeVal->val.interval.recurrences);
         if ( status != 0 ) return LOG_RTERRNEW(pctxt, status);

         if ( *p == '/') p++;
         else return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
         finiteRecurs = TRUE;
      }
   }
   else {
      pdatetime = &timeVal->val.dateTime;
      timeVal->val.interval.recurrences = 0;
   }

   if ( *p == 'P' ) {
      /* We have a duration or an interval with a duration.
         Parse into interval.spec.duration for now. */
      p++;
      status = parseDuration(pctxt, &p, &timeVal->val.interval.spec.duration,
                                 logCanon);
      if (status == ASN_E_NOTCANON) {
         notcanon = TRUE;
         status = 0;
      }
      else if (status != 0) return LOG_RTERR(pctxt, status);

      if (*p == '/')
      {
         /* We have DE interval */
         p++;
         timeVal->settings |= ASN1TIMEFIELD_INTERVAL_DE;
         timeVal->val.interval.spec.de.duration =
                                       timeVal->val.interval.spec.duration;

         status = parsePoint(pctxt, &p, &timeVal->val.interval.spec.de.end,
                               &timeVal->settings, logCanon);
         if (status == ASN_E_NOTCANON) {
            notcanon = TRUE;
            status = 0;
         }
         else if (status != 0) return LOG_RTERR(pctxt, status);
      }
      else {
         /* Must be duration only. */
         timeVal->settings |= ASN1TIMEFIELD_DURATION;
      }
   }
   else {
      /* Must begin with a point.  Parse into pdatetime for now. */
      status = parsePoint(pctxt, &p, pdatetime, &timeVal->settings, logCanon);
      if (status == ASN_E_NOTCANON) {
         notcanon = TRUE;
         status = 0;
      }
      else if (status != 0 ) return LOG_RTERR(pctxt, status);

      if (*p == '/') {
         /* We have an SE or SD interval. */
         p++;

         if (*p == 'P') {
            /* SD interval. */
            timeVal->settings |= ASN1TIMEFIELD_INTERVAL_SD;
            timeVal->val.interval.spec.sd.start = *pdatetime;
            p++;
            status = parseDuration(pctxt, &p,
                                       &timeVal->val.interval.spec.sd.duration,
                                       logCanon);
            if (status == ASN_E_NOTCANON) {
               notcanon = TRUE;
               status = 0;
            }
            else if (status != 0) return LOG_RTERR(pctxt, status);
         }
         else {
            /* Must be SE interval */
            OSUINT16 startSettings;
            OSUINT16 endSettings;

            timeVal->settings |= ASN1TIMEFIELD_INTERVAL_SE;
            startSettings = timeVal->settings;

            if ( &timeVal->val.interval.spec.se.start != pdatetime )
               timeVal->val.interval.spec.se.start = *pdatetime;
            status = parsePoint(pctxt, &p, &timeVal->val.interval.spec.se.end,
                                 &timeVal->settings, logCanon);
            if (status == ASN_E_NOTCANON) {
               notcanon = TRUE;
               status = 0;
            }
            else if (status != 0) return LOG_RTERR(pctxt, status);

            endSettings = timeVal->settings;

            if (( (startSettings & ASN1TIMEFIELD_POINT) !=
                  (endSettings & ASN1TIMEFIELD_POINT)) ||
               ((startSettings & ASN1TIMEFIELD_DIFF) == 0 &&
                  (endSettings & ASN1TIMEFIELD_DIFF) != 0) ||
               (timeVal->val.interval.spec.se.start.fracDigits !=
                  timeVal->val.interval.spec.se.end.fracDigits ))
            {
               /* The two time points are not using the same settings.
                  If start does not have a time diff and end does, we've
                  detected that.  It is legal for start to have a time diff
                  and end to not.
                  We should also check that the year setting is the same (e.g.
                  both using Year=Basic), but I've not done so.
                  */
               return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
            }

            if (checkCanon && (endSettings & ASN1TIMEFIELD_DIFF) != 0 &&
               timeVal->val.interval.spec.se.start.timezoneDifffHours ==
               timeVal->val.interval.spec.se.end.timezoneDifffHours &&
               timeVal->val.interval.spec.se.start.timezoneDiffMins ==
               timeVal->val.interval.spec.se.end.timezoneDiffMins
               )
            {
               /* From the above, we know both start and end are using
                  a timezone difference component, and that they are
                  equal. */
               notcanon = TRUE;
               if ( logCanon ) LOG_CANON(pctxt,
                           "Duplicate timezone difference must be omitted.");
            }
         }
      }
   }

   /* At this point, we should have reached the end of the string. */
   if (*p != 0)
   {
      return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
   }

   /*If this is an interval and a finite # of recursion wasn't specified,
      set recurrences to 0. */
   if (!finiteRecurs && timeVal->settings & (ASN1TIMEFIELD_DURATION |
      ASN1TIMEFIELD_INTERVAL_DE |
      ASN1TIMEFIELD_INTERVAL_SD |
      ASN1TIMEFIELD_INTERVAL_SE))
   {
      timeVal->val.interval.recurrences = 0;
   }

   if (notcanon) return ASN_E_NOTCANON;
   else return 0;
}
