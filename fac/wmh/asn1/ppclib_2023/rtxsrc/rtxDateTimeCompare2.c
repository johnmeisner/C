/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxDateTime.hh"

 /**
  * rtxCmpDate2:
  * This function compares date part of OSNumDateTime structure and
  * return the result as integer.
  */
EXTRTMETHOD int rtxCmpDate2 (
   const OSNumDateTime* pvalue,
   OSINT32 year, OSUINT8 mon, OSUINT8 day, OSBOOL tzflag, OSINT32 tzo)
{
   OSNumDateTime value2;
   value2.year = year;
   value2.mon = mon;
   value2.day = day;
   value2.tz_flag = tzflag;
   value2.tzo = tzo;
   return rtxCmpDate (pvalue, &value2);
}

/**
 * rtxCmpTime2:
 * This function compares time part of OSNumDateTime structure and
 * return the result as integer.
 */
EXTRTMETHOD int rtxCmpTime2 (
   const OSNumDateTime* pvalue,
   OSUINT8 hour, OSUINT8 min, OSREAL sec, OSBOOL tzflag, OSINT32 tzo)
{
   OSNumDateTime value2;
   value2.hour = hour;
   value2.min = min;
   value2.sec = sec;
   value2.tz_flag = tzflag;
   value2.tzo = tzo;
   return rtxCmpTime (pvalue, &value2);
}

/**
 * rtxCmpDateTime2:
 * This function compares date and time  of two OSNumDateTime structure and
 * return the result as integer.
 */
EXTRTMETHOD int rtxCmpDateTime2 (
   const OSNumDateTime* pvalue,
   OSINT32 year, OSUINT8 mon, OSUINT8 day,
   OSUINT8 hour, OSUINT8 min, OSREAL sec, OSBOOL tzflag, OSINT32 tzo)
{
   OSNumDateTime value2;
   value2.year = year;
   value2.mon = mon;
   value2.day = day;
   value2.hour = hour;
   value2.min = min;
   value2.sec = sec;
   value2.tz_flag = tzflag;
   value2.tzo = tzo;
   return rtxCmpDateTime (pvalue, &value2);
}

