/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxDateTime.hh"

#define SEC_PRECISION 0.0000005

static void normalizeTimeZone(OSNumDateTime*);

 /**
  * rtxCmpDate:
  * This function compares date part of two OSNumDateTime structure and
  * return the result as integer.
  */
EXTRTMETHOD int rtxCmpDate (const OSNumDateTime* pvalue1, const OSNumDateTime* pvalue2)
{

   int stat = 0;
   if (pvalue1->year > pvalue2->year) {
      stat = 1;
   } else if (pvalue1->year < pvalue2->year) {
      stat = -1;
   } else {
      if (pvalue1->mon > pvalue2->mon) {
         stat = 1;
      } else if (pvalue1->mon < pvalue2->mon) {
         stat = -1;
      } else {
         if (pvalue1->day > pvalue2->day) {
            stat = 1;
         } else if (pvalue1->day < pvalue2->day) {
            stat = -1;
         } else {
            stat = 0;
         }
      }
   }
   return stat;
}

/**
 * rtxCmpTime:
 * This function compares time part of two OSNumDateTime structure and
 * return the result as integer.
 */
EXTRTMETHOD int rtxCmpTime (const OSNumDateTime* pvalue1, const OSNumDateTime* pvalue2)
{
   int stat = 0;
   if (pvalue1->hour  > pvalue2->hour) {
      stat = 1;
   } else if (pvalue1->hour < pvalue2->hour) {
      stat = -1;
   } else {
      if (pvalue1->min > pvalue2->min) {
         stat = 1;
      } else if (pvalue1->min < pvalue2->min) {
         stat = -1;
      } else {
         if (pvalue1->sec > pvalue2->sec + SEC_PRECISION) {
            stat = 1;
         } else if (pvalue1->sec < pvalue2->sec  - SEC_PRECISION) {
            stat = -1;
         } else {
            stat = 0;
         }
      }
   }
   return stat;
}

/**
 * rtxCmpDateTime:
 * This function compares date and time  of two OSNumDateTime structure and
 * return the result as integer.
 */
EXTRTMETHOD int rtxCmpDateTime (const OSNumDateTime* pvalue1, const OSNumDateTime* pvalue2)
{
   int stat = 0;
   OSNumDateTime tmpvalue1, tmpvalue2;
   if (pvalue1->tz_flag == (OSBOOL)TRUE) {
      OSCRTLSAFEMEMCPY (&tmpvalue1, sizeof(OSNumDateTime), pvalue1,
            sizeof(OSNumDateTime));
      normalizeTimeZone (&tmpvalue1);
      pvalue1 = &tmpvalue1;
   }
   if (pvalue2->tz_flag == (OSBOOL)TRUE) {
      OSCRTLSAFEMEMCPY (&tmpvalue2, sizeof(OSNumDateTime), pvalue2,
            sizeof(OSNumDateTime));
      normalizeTimeZone (&tmpvalue2);
      pvalue2 = &tmpvalue2;
   }

   stat = rtxCmpDate(pvalue1, pvalue2);
   if (stat == 0)  stat = rtxCmpTime(pvalue1, pvalue2);
   return stat;
}

 /**
  * normalizeTimeZone:
  * This function normalizes the Time Zone value of OSNumDateTime structure.
  */

static void normalizeTimeZone(OSNumDateTime* pvalue)
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
      pvalue->year = (OSINT8)(pvalue->year + FQUOTIENT_RANGE(tmp,1,13));
   }
   pvalue->day = (OSINT8)tmpdays;
}

