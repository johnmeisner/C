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
/*
* @file rtxRandTest.c
*/
#ifndef _NO_STREAM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rtxRandTest.h"
#include "rtxsrc/rtxMemory.h"

const char alphanum_str [] =
   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void rtxRandInit(unsigned int randSeed)
{
   srand(randSeed);
}

OSINT32 rtxRandInt32 (OSCTXT* pctxt, OSINT32 lower, OSINT32 upper)
{
   OSINT32 result;
   OSUINT32 range;
   OS_UNUSED_ARG(pctxt);

   result = rand() * 2147001325L + 715136305L;

   if(upper != lower)
   {
      range = upper;
      if (lower == OSINT32_MIN)
         range += (OSUINT32)OSINT32_MAX + 1;
      else
         range += -lower+1;
      result = (OSINT32) result%range + lower;
   } else
      result = (OSINT32) lower;

   return result;
}

OSINT64 rtxRandInt64 (OSCTXT* pctxt, OSINT64 lower, OSINT64 upper)
{
   OSINT32 resultLow = rand() * 2147001325L + 715136305L;
   OSINT32 resultHigh = rand() * 2147001325L + 715136305L;
   OSUINT64 range;
   OSINT64 result = (resultLow & 0x7fffffff) |
   (rtxRandUInt32(pctxt, 0, 1)? 0 :((OSINT64)(resultHigh & 0x7fffffff) << 32));

   if(upper != lower)
   {
      range = upper;
      if (lower == OSINT64MIN)
         range += (OSUINT64)OSINT64MAX + 1;
      else
         range += -lower+1;
      result = (OSINT64) result%range + lower;
   } else
      result = (OSINT64) lower;

   return result;
}

OSUINT32 rtxRandUInt32 (OSCTXT* pctxt, OSUINT32 lower, OSUINT32 upper)
{
   OSUINT32 result;
   OS_UNUSED_ARG(pctxt);

   result = rand() * 2147001325L + 715136305L;
   if (upper == OSUINT32_MAX && !lower)
      return lower + result%(upper);
   return lower + result%(upper-lower+1);

}

OSUINT64 rtxRandUInt64 (OSCTXT* pctxt, OSUINT64 lower, OSUINT64 upper)
{
   OSUINT32 resultLow = rand() * 2147001325L + 715136305L;
   OSUINT32 resultHigh = rand() * 2147001325L + 715136305L;
   OSUINT64 result = (resultLow & 0x7fffffff) |
   (rtxRandUInt32(pctxt, 0, 1)? 0 :((OSUINT64)(resultHigh & 0x7fffffff) << 32));

   if (upper == OSUINT64MAX && !lower)
      return lower + result%(upper);
   return lower + result%(upper-lower+1);
}

OSREAL rtxRandFloat (OSCTXT* pctxt, OSREAL lower, OSREAL upper)
{
   OSREAL result;
   OS_UNUSED_ARG(pctxt);

   result = rand()/((double)(RAND_MAX)+1);
   return lower + result*(upper-lower);
}

OSSIZE rtxRandSize (OSCTXT* pctxt, OSSIZE lower, OSSIZE upper)
{
   if (sizeof(OSSIZE) == 4)
      return rtxRandUInt32 (pctxt, (OSUINT32)lower, (OSUINT32)upper);
   else
      return (OSSIZE)rtxRandUInt64 (pctxt, lower, upper);
}

char* rtxRandString (OSCTXT* pctxt, OSSIZE lower, OSSIZE upper,
                     const char* charSet)
{
   char *resultStr = NULL;
   OSSIZE charSetLen = 0;
   OSSIZE strLength = lower, iPos;

   /* check if max length 0 */
   if (0 == upper) return 0;

   /* generate length */
   strLength = rtxRandSize (pctxt, lower, upper);

   /* check that length > 0 */
   if (0 == strLength) return 0;

   /* alloc mem for string */
   resultStr = (char*) rtxMemAlloc (pctxt, strLength+1);
   if (!resultStr)
   {
      return 0;
   }

   /* count charset len */
   if (charSet)
      charSetLen = strlen(charSet);

   /* generate string */
   for (iPos = 0; iPos < strLength; iPos++)
      if (charSetLen > 0)
         resultStr[iPos]=charSet[rtxRandSize(pctxt, 0, charSetLen-1)];
      else
         resultStr[iPos]=(char)alphanum_str[rtxRandSize
                            (pctxt, 0, strlen(alphanum_str)-1)];

   /* null terminate the string */
   resultStr[iPos]=0;

   return resultStr;
}

OSUTF8CHAR* rtxRandUnicodeString (OSCTXT* pctxt, OSSIZE lower,
                                  OSSIZE upper, const char* charSet)
{
   OSUTF8CHAR *resultStr = NULL;
   OSSIZE charSetLen = 0;
   OSSIZE strLength = lower, iPos;

   /* check if max length 0 */
   if (0 == upper) return 0;

   /* generate length */
   strLength = rtxRandSize (pctxt, lower, upper);

   /* check that length > 0 */
   if (!strLength)
      return 0;

   /* alloc mem for string */
   resultStr = (OSUTF8CHAR*) rtxMemAlloc (pctxt, strLength+1);
   if (!resultStr)
   {
      return 0;
   }

   /* count charset len */
   if (charSet)
      charSetLen = strlen(charSet);
   /* generate string */
   for (iPos = 0; iPos < strLength; iPos++)
      if (charSetLen > 0)
         resultStr[iPos]=charSet[rtxRandSize(pctxt, 0, charSetLen-1)];
      else
      {
         if (rtxRandUInt32(pctxt, 0, 1))
            resultStr[iPos]=(char)rtxRandUInt32(pctxt, 32, 126);
         else
            resultStr[iPos]=(char)rtxRandUInt32(pctxt, 161, 255);
      }

   /* null terminate the string */
   resultStr[iPos]=0;

   return resultStr;
}

void rtxRandOctetString
(OSCTXT* pctxt, OSDynOctStr* pvalue, OSSIZE lower, OSSIZE upper)
{
   OSSIZE strLength = lower, iPos;
   OSOCTET* p = 0;

   if (0 == pvalue) return;

   pvalue->data = 0;
   pvalue->numocts = 0;

   /* check if max length 0 */
   if (0 == upper || upper > OSUINT32_MAX || lower > upper) {
      return;
   }

   /* generate length */
   strLength = rtxRandSize (pctxt, lower, upper);

   /* check that length > 0 */
   if (0 == strLength) return;

   p = (OSOCTET*) rtxMemAlloc (pctxt, strLength);
   if (0 != p) {
      pvalue->data = p;
      pvalue->numocts = (OSUINT32) strLength;

      /* generate string */
      for (iPos = 0; iPos < pvalue->numocts; iPos++)
         p[iPos] = (OSOCTET)rtxRandUInt32(pctxt, 0, 255);
   }
}

const signed char gDaysInMonthRand[12] =
{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void rtxRandDateTime
(OSCTXT* pctxt, OSNumDateTime* pvalue, OSUINT32 lower, OSUINT32 upper)
{
   if (!pvalue)
   {
      return;
   }

   pvalue->year = (OSUINT32)rtxRandUInt32(pctxt, lower, upper);
   pvalue->mon  = (OSUINT8)rtxRandInt32(pctxt, 1, 12);
   if ( (pvalue->mon == 2) && (pvalue->year%4))
      pvalue->day  = (OSUINT8)rtxRandInt32(pctxt, 1, gDaysInMonthRand[pvalue->mon-1]+1);
   else
      pvalue->day  = (OSUINT8)rtxRandInt32(pctxt, 1, gDaysInMonthRand[pvalue->mon-1]);
   pvalue->hour = (OSUINT8)rtxRandInt32(pctxt, 0, 23);
   pvalue->min  = (OSUINT8)rtxRandInt32(pctxt, 0, 59);
   pvalue->sec  = rtxRandFloat(pctxt, 0, 59);
   pvalue->tz_flag = (OSBOOL)rtxRandInt32(pctxt, 0, 1);
   pvalue->tzo  = rtxRandInt32(pctxt, -840, 840);
}

#endif
