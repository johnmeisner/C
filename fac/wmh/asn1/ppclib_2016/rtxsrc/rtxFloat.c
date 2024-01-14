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
/**
 * @file rtxFloat.c
 *
 * Implements the IEEE 754 float/double equality tests using ULPs
 * (Units in the Last Place) as proposed by Bruce Dawson
 * in "Comparing floating point numbers".
 * http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 */

#include "osSysTypes.h"
#include "rtxFloat.h"

/* We tolerate at most one ULP as we want to test if
 * a given real number is close to a boundary. */
#define MAXULPS 1

/* Float: sign[31] exp[30:23] fraction[22:0] */
#define SIGN_MASK_32 ((OSINT32)1 << 31)
#define  EXP_MASK_32 ((OSINT32)255 << 23)
#define FRAC_MASK_32 (~(SIGN_MASK_32|EXP_MASK_32))

typedef union {
   OSINT32 i;
   float f;
} Float2Int;

#ifndef __SYMBIAN32__
/* Double: sign[63] exp[62:52] fraction[51:0] */
#define SIGN_MASK_64 ((OSINT64)1 << 63)
#define  EXP_MASK_64 ((OSINT64)2047 << 52)
#define FRAC_MASK_64 (~(SIGN_MASK_64|EXP_MASK_64))

typedef union {
   OSINT64 i;
   double d;
} Double2Int;
#endif

OSBOOL rtxFloatIsInfinite (float A)
{
   Float2Int param;
   param.f = A;

    /* All exponent bits set to 1, fraction bits to 0 */
   if ((param.i & ~SIGN_MASK_32) == EXP_MASK_32) return TRUE;
   else return FALSE;
}

OSBOOL rtxFloatIsNan (float A)
{
   OSINT32 exp;
   OSINT32 frac;
   Float2Int param;
   param.f = A;

   /* All exponent bits set to 1, non-zero fraction */
   exp = param.i & EXP_MASK_32;
   frac = param.i & FRAC_MASK_32;

   if (exp == EXP_MASK_32 && frac != 0) return TRUE;
   else return FALSE;
}

int rtxFloatSign (float A)
{
   Float2Int param;
   param.f = A;

   return (int)(param.i & SIGN_MASK_32);
}

#ifndef __SYMBIAN32__
OSBOOL rtxDoubleIsInfinite (double A)
{
   Double2Int param;
   param.d = A;

   /* All exponent bits set to 1, fraction bits to 0 */
   if ((param.i & ~SIGN_MASK_64) == EXP_MASK_64) return TRUE;
   else return FALSE;
}

OSBOOL rtxDoubleIsNan (double A)
{
   OSINT64 exp;
   OSINT64 frac;
   Double2Int param;
   param.d = A;

   /* All exponent bits set to 1, non-zero fraction */
   exp = param.i & EXP_MASK_64;
   frac = param.i & FRAC_MASK_64;

   if (exp == EXP_MASK_64 && frac != 0) return TRUE;
   else return FALSE;
}

int rtxDoubleSign (double A)
{
   Double2Int param;
   param.d = A;

   return (int)(param.i & SIGN_MASK_64);
}
#endif

EXTRTMETHOD OSBOOL rtxFloatEqualImpl (float a, float b)
{
   OSINT32 delta;
   OSINT32 aInt;
   OSINT32 bInt;
   Float2Int param;

   param.f = a;
   aInt = param.i;

   param.f = b;
   bInt = param.i;

   if ((aInt & EXP_MASK_32) == EXP_MASK_32) {
      /* a is infinity or NaN */
      if (aInt & FRAC_MASK_32)
         return FALSE; /* NaN */
      else
         return (OSBOOL) (aInt == bInt);
   }
   else if ((bInt & EXP_MASK_32) == EXP_MASK_32)
      return FALSE; /* b - infinity or NaN, a - number */

   /* Make a and b lexicographically ordered as a 2s-complement integer */
   if (aInt < 0) aInt = SIGN_MASK_32 - aInt;
   if (bInt < 0) bInt = SIGN_MASK_32 - bInt;

   delta = aInt - bInt;
   if (delta < 0) delta = -delta;
   if (delta <= MAXULPS) return TRUE;
   else return FALSE;
}

#ifndef __SYMBIAN32__
EXTRTMETHOD OSBOOL rtxDoubleEqualImpl (double a, double b)
{
   OSINT64 delta;
   OSINT64 aInt;
   OSINT64 bInt;
   Double2Int param;

   param.d = a;
   aInt = param.i;

   param.d = b;
   bInt = param.i;

   if ((aInt & EXP_MASK_64) == EXP_MASK_64) {
      /* a is infinity or NaN */
      if (aInt & FRAC_MASK_64)
         return FALSE; /* NaN */
      else
         return (OSBOOL) (aInt == bInt);
   }
   else if ((bInt & EXP_MASK_64) == EXP_MASK_64)
      return FALSE; /* b - infinity or NaN, a - number */

   /* Make a and b lexicographically ordered as a 2s-complement integer */
   if (aInt < 0) aInt = SIGN_MASK_64 - aInt;
   if (bInt < 0) bInt = SIGN_MASK_64 - bInt;

   delta = aInt - bInt;
   if (delta < 0) delta = -delta;
   if (delta <= MAXULPS) return TRUE;
   else return FALSE;
}
#endif

