/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

/*****************************************************************************\
*
* rtxBigNumber
*
* Routines for manipulating Big Numbers.
*
\*****************************************************************************/

#include "rtxsrc/rtxBigNumber.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCommonDefs.h"

#define MAXBIGNUMSZ 256

int rtxAddBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* c, OSSIZE szC)
{
   const OSOCTET* p = 0;
   const OSOCTET* pEnd = 0;
   const OSOCTET* aEnd = a + szA;
   const OSOCTET* bEnd = b + szB;
   OSOCTET* cEnd = c + szC;
   unsigned carry = 0;

   while (a != aEnd && b != bEnd) {
      unsigned tm = *a++ + *b++ + carry;
      carry = tm >> 8;

      if (c != cEnd)
         *c++ = (OSOCTET) tm;
      else if ((OSOCTET) tm)
         return RTERR_BUFOVFLW; /* unsigned overflow */
   }

   if (a != aEnd) {
      p = a;
      pEnd = aEnd;
   }
   else if (b != bEnd) {
      p = b;
      pEnd = bEnd;
   }

   while (p != pEnd) {
      unsigned tm = *p++ + carry;
      carry = tm >> 8;

      if (c != cEnd)
         *c++ = (OSOCTET) tm;
      else if ((OSOCTET) tm)
         return RTERR_BUFOVFLW; /* unsigned overflow */
   }

   if (carry) {
      if (c != cEnd)
         *c++ = (OSOCTET) carry;
      else
         return RTERR_BUFOVFLW; /* unsigned overflow */
   }

   while (c != cEnd)
      *c++ = 0;

   return 0;
}

int rtxSubBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* c, OSSIZE szC)
{
   const OSOCTET* aEnd = a + szA;
   const OSOCTET* bEnd = b + szB;
   OSOCTET* cEnd = c + szC;
   unsigned carry = 0;

   while (a != aEnd && b != bEnd) {
      unsigned tm = *a++ - *b++ - carry;
      carry = (tm >> 8) & 1;

      if (c != cEnd)
         *c++ = (OSOCTET) tm;
      else if ((OSOCTET) tm)
         return RTERR_BUFOVFLW; /* unsigned overflow */
   }

   if (a != aEnd) {
      while (a != aEnd) {
         unsigned tm = *a++ - carry;
         carry = (tm >> 8) & 1;

         if (c != cEnd)
            *c++ = (OSOCTET) tm;
         else if ((OSOCTET) tm)
            return RTERR_BUFOVFLW; /* unsigned overflow */
      }
   }
   else if (b != bEnd) {
      while (b != bEnd) {
         unsigned tm = 0 - *b++ - carry;
         carry = (tm >> 8) & 1;

         if (c != cEnd)
            *c++ = (OSOCTET) tm;
         else if ((OSOCTET) tm)
            return RTERR_BUFOVFLW; /* unsigned overflow */
      }
   }

   if (carry) {
      if (c != cEnd)
         *c++ = (OSOCTET) (0 - carry);
      else
         return RTERR_BUFOVFLW; /* unsigned overflow */

      carry = 0xFF;
   }

   while (c != cEnd)
     *c++ = (OSOCTET) carry;

   return carry ? RTERR_BUFOVFLW : 0;
}

int rtxMulBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* c, OSSIZE szC)
{
   const OSOCTET* p = 0;
   OSOCTET* pd;
   const OSOCTET* aEnd = a + szA;
   const OSOCTET* bEnd = b + szB;
   OSOCTET* cEnd = c + szC;
   unsigned carry;

   for (pd = c; pd != cEnd; pd++)
      *pd = 0;

   while (b != bEnd) {
      OSOCTET m = *b++;

      p = a;
      pd = c++;
      carry = 0;

      while (p != aEnd) {
         unsigned tm = *p++ * m + carry;

         if (pd != cEnd) {
            tm += *pd;
            *pd++ = (OSOCTET) tm;
            carry = tm >> 8;
         }
         else if ((OSOCTET) tm)
            return RTERR_BUFOVFLW; /* unsigned overflow */
      }

      if (carry) {
         if (pd != cEnd)
            *pd = (OSOCTET) carry;
         else
            return RTERR_BUFOVFLW; /* unsigned overflow */
      }
   }

   return 0;
}

int rtxDivRemBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* c, OSSIZE szC, OSOCTET* rem, OSSIZE szRem)
{
   const OSOCTET* ps = 0;
   OSOCTET* p = 0;
   OSOCTET* pEnd;
   const OSOCTET* aEnd = a + szA;
   const OSOCTET* bEnd = b + szB;
   OSOCTET* cEnd = c + szC;
   OSSIZE offset = 0;
   unsigned carry;
   unsigned msb = 0;
   unsigned tmA;
   unsigned tmB;

   OSOCTET buff[MAXBIGNUMSZ];

   while (aEnd != a && aEnd[-1] == 0) {
      aEnd--;
      szA--;
   }

   while (bEnd != b && bEnd[-1] == 0) {
      bEnd--;
      szB--;
   }

   if (b == bEnd)
      return RTERR_FAILED; /* divide by zero */

   if (szB > szA)
      return 0; /* result is 0 as b > a */

   tmA = aEnd[-1];
   tmB = bEnd[-1];
   offset = (szA - szB) * 8;

   /* copy a to buffer and align */

   OSCRTLSAFEMEMCPY (buff, sizeof(buff), a, szA);

   if (tmA > tmB) {
      buff[szA] = 0;
      szA++;
      offset += 8;
   }

   carry = 0;
   pEnd = buff + szA;

   while (1) {
      unsigned tm;

      p = buff + szA - szB;
      ps = b;

      if (carry) {
         carry = 0;
         while (p != pEnd) {
            tm = *p + *ps++ + carry;
            *p++ = (OSOCTET) tm;
            carry = tm >> 8;
         }

         carry ^= 1;
      }
      else {
         while (p != pEnd) {
            tm = *p - *ps++ - carry;
            *p++ = (OSOCTET) tm;
            carry = (tm >> 8) & 1;
         }

         carry ^= msb;
      }

      if (!carry && c) {
         if ((offset >> 3) >= szC)
            return RTERR_BUFOVFLW; /* overflow */

         c[offset >> 3] |= (OSOCTET) (1 << (offset & 7));
      }

      tm = 0;

      if (offset-- == 0)
         break;

      for (p = buff; p != pEnd;) {
         tm |= (unsigned) *p << 1;
         *p++ = (OSOCTET) tm;
         tm >>= 8;
      }

      msb = tm & 1;
   }

   if (rem) { /* reminder */
      unsigned tm;

      p = buff + szA - szB;
      ps = b;

      if (carry) {
         carry = 0;
         while (p != pEnd) {
            tm = *p + *ps++ + carry;
            *p++ = (OSOCTET) tm;
            carry = tm >> 8;
         }
      }

      cEnd = rem + szRem;
      p = buff + szA - szB;

      while (rem != cEnd && p != pEnd)
         *rem++ = *p++;

      if (p != pEnd)
         return RTERR_BUFOVFLW; /* overflow */

      while (rem != cEnd)
         *rem++ = 0;
   }

   return 0;
}

int rtxDivBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* c, OSSIZE szC)
{
   return rtxDivRemBigNum (a, szA, b, szB, c, szC, 0, 0);
}

int rtxModBigNum
(const OSOCTET* a, OSSIZE szA, const OSOCTET* b, OSSIZE szB,
 OSOCTET* rem, OSSIZE szRem)
{
   return rtxDivRemBigNum (a, szA, b, szB, 0, 0, rem, szRem);
}

int rtxBigNumToStr
(const OSOCTET* a, OSSIZE szA, char* str, OSSIZE szStr)
{
   OSOCTET rem;
   OSOCTET b10 = 10;
   OSOCTET buff1[MAXBIGNUMSZ];
   OSOCTET buff2[MAXBIGNUMSZ];
   OSOCTET* p;
   const OSOCTET* aEnd = a + szA;
   char* strEnd = str + szStr - 1;
   char* strBeg = str;
   OSOCTET fl = 0;

   if (!a || szA == 0 || szStr < 2)
      return RTERR_INVPARAM;

   while (aEnd != a && aEnd[-1] == 0) {
      aEnd--;
      szA--;
   }

   if (szA == 0) {
      *str++ = '0';
      *str = 0;
   }

   p = buff1;
   while (a != aEnd)
      *p++ = *a++;

   do {
      if (fl) {
         rtxDivRemBigNum (buff2, szA, &b10, 1, buff1, szA, &rem, 1);

         if (buff1[szA - 1] == 0)
            szA--;
      }
      else {
         rtxDivRemBigNum (buff1, szA, &b10, 1, buff2, szA, &rem, 1);

         if (buff2[szA - 1] == 0)
            szA--;
      }

      if (str != strEnd)
         *str++ = (char) ('0' + rem);
      else {
         *str = 0;
         return RTERR_BUFOVFLW;
      }

      fl ^= 1;
   } while (szA != 0);

   *str-- = 0;

   /* reverse string */
   while (strBeg < str) {
      char tm = *strBeg;
      *strBeg++ = *str;
      *str-- = tm;
   }

   return 0;
}

int rtxStrToBigNum
(const char* str, OSOCTET* a, OSSIZE szA)
{
   OSOCTET b10 = 10;
   OSOCTET m;
   OSOCTET buff[MAXBIGNUMSZ];
   OSSIZE sz = 1;

   a[0] = 0;

   while (*str) {
      m = (OSOCTET) (*str++ - '0');
      if (m > 9)
         return RTERR_INVCHAR; /* invalid string format */

      rtxMulBigNum (a, sz, &b10, 1, buff, sz);
      rtxAddBigNum (buff, sz, &m, 1, a, sz);

      if (a[sz - 1] != 0) {
         a[sz] = 0;
         sz++;
      }
   }

   while (sz < szA)
      a[sz++] = 0;

   return 0;
}
