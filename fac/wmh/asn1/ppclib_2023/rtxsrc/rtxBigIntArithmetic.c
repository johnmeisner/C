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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxBigInt.hh"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

/*
 * Returns -1, 0 or +1 as big-endian unsigned int array arg1 is
 * less than, equal to, or greater than arg2.
 */
static int compareMagnitudes
(const OSOCTET* arg1, size_t arg1Size, const OSOCTET* arg2, size_t arg2Size)
{
   size_t i;

   if (arg1Size < arg2Size)
       return -1;
   if (arg1Size > arg2Size)
       return 1;

   /* Argument lengths are equal; compare the values */
   for (i = 0; i < arg1Size; i++) {
      int b1 = arg1[i] & LONG_MASK;
      int b2 = arg2[i] & LONG_MASK;
      if (b1 < b2)
         return -1;
      if (b1 > b2)
         return 1;
   }
   return 0;
}

 /*
  * Adds the contents of the octet arrays x and y. This method allocates
  * a new octet array to hold the answer and returns a reference to that
  * array.
  */
static OSOCTET* add
   (OSCTXT* pctxt, const OSOCTET* x, size_t xlength,
                   const OSOCTET* y, size_t ylength, size_t* resLength)
{
    int xIndex;
    int yIndex;
    OSOCTET* result;
    unsigned sum = 0;
    int carry;

    /* If x is shorter, swap the two arrays */
    if (xlength < ylength) {
       const OSOCTET* tmp = x;
       x = y;
       y = tmp;
       xIndex = (int)ylength;
       yIndex = (int)xlength;
    }
    else {
       xIndex = (int)xlength;
       yIndex = (int)ylength;
    }

    result = rtxMemAllocArray (pctxt, xIndex, OSOCTET);
    if (result == NULL) return NULL;
    *resLength = xIndex;

    /* Add common parts of both numbers */
    while(yIndex > 0) {
        sum = (x[--xIndex] & LONG_MASK) +
              (y[--yIndex] & LONG_MASK) + (sum >> 8);
        result[xIndex] = (OSOCTET)sum;
    }

    /* Copy remainder of longer number while carry propagation is required */
    carry = (sum >> 8 != 0);
    while (xIndex > 0 && carry) {
       result[xIndex-1] = (OSOCTET)(x[xIndex] + 1);
       xIndex--;
       carry = ((result[xIndex]) == 0);
    }

    /* Copy remainder of longer number */
    while (xIndex > 0) {
       result[xIndex - 1] = x[xIndex];
       xIndex--;
    }

    /* Grow result if necessary */
    if (carry) {
        size_t newLen = *resLength + 1;
        OSOCTET* newresult =
           rtxMemReallocArray (pctxt, result, newLen, OSOCTET);
        if (newresult == NULL) {
           rtxMemFreeArray (pctxt, result);
           return NULL;
        }
        result = newresult;
        OSCRTLSAFEMEMMOVE (result, newLen, result + 1, *resLength);
        result[0] = 0x01;
        *resLength = newLen;
    }
    return result;
}

/*
 * Subtracts the contents of the second octet arrays (little) from the
 * first (big).  The first octet array (big) must represent a larger number
 * than the second.  This method allocates the space necessary to hold the
 * answer.
 */
static OSOCTET* subtract
   (OSCTXT* pctxt, const OSOCTET* big, size_t bigLength,
                   const OSOCTET* little, size_t littleLength, size_t* resLength)
{
   size_t bigIndex = bigLength;
   OSOCTET* result = rtxMemAllocArray (pctxt, bigIndex, OSOCTET);
   size_t littleIndex = littleLength;
   int difference = 0, borrow;

   if (result == NULL) return NULL;
   *resLength = bigIndex;

   /* Subtract common parts of both numbers */
   while(littleIndex > 0) {
      difference = (int) ((big[--bigIndex] & LONG_MASK) -
                          (little[--littleIndex] & LONG_MASK) +
                          (difference >> 8));
      result[bigIndex] = (OSOCTET)difference;
   }

   /* Subtract remainder of longer number while borrow propagates */
   borrow = (difference >> 8 != 0);
   while (bigIndex > 0 && borrow) {
      bigIndex--;
      result[bigIndex] = (OSOCTET)(big[bigIndex] - 1);
      borrow = (result[bigIndex] == ((OSOCTET)-1));
   }

   /* Copy remainder of longer number */
   while (bigIndex > 0) {
      result[bigIndex - 1] = big[bigIndex];
      --bigIndex;
   }

   return result;
}

EXTRTMETHOD int rtxBigIntAdd
   (OSCTXT* pctxt,
    OSBigInt* result, const OSBigInt* arg1, const OSBigInt* arg2)
{
   size_t resLength;
   OSOCTET* mag;
   int cmp;

   if (arg2->sign == 0) { /* arg2 is zero */
      rtxBigIntFree (pctxt, result);
      rtxBigIntCopy (pctxt, arg1, result);
      return 0;
   }
   if (arg1->sign == 0) { /* arg1 is zero */
      rtxBigIntFree (pctxt, result);
      rtxBigIntCopy (pctxt, arg2, result);
      return 0;
   }
   if (arg1->sign == arg2->sign) {
      mag = add (pctxt, arg1->mag, arg1->numocts,
                        arg2->mag, arg2->numocts, &resLength);
      rtxBigIntFree (pctxt, result);
      if (mag == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
      result->sign = arg1->sign;
      result->mag = mag;
      result->dynamic = 1;
      result->numocts = result->allocated = (int)resLength;
      return 0;
   }

   cmp = compareMagnitudes (arg1->mag, arg1->numocts, arg2->mag, arg2->numocts);
   if (cmp == 0) {
      rtxBigIntFree (pctxt, result);
      result->sign = 0;
      result->numocts = result->allocated = 0;
      result->mag = 0;
      return 0;
   }

   mag = (cmp > 0 ? subtract(pctxt, arg1->mag, arg1->numocts,
                                    arg2->mag, arg2->numocts, &resLength)
                  : subtract(pctxt, arg2->mag, arg2->numocts,
                                    arg1->mag, arg1->numocts, &resLength));
   rtxBigIntFree (pctxt, result);
   if (mag == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   result->sign = cmp * arg1->sign;
   result->mag = mag;
   result->dynamic = 1;
   result->numocts = result->allocated = (int)resLength;
   rtxBigIntRemoveLeadingZeroBytes (result);
   return 0;
}

EXTRTMETHOD int rtxBigIntSubtract
   (OSCTXT* pctxt,
    OSBigInt* result, const OSBigInt* arg1, const OSBigInt* arg2)
{
   size_t resLength;
   OSOCTET* mag;
   int cmp;

   if (arg1->sign == 0) { /* arg1 is zero */
      rtxBigIntFree (pctxt, result);
      rtxBigIntCopy (pctxt, arg2, result);
      result->sign = -result->sign;
      return 0;
   }
   if (arg2->sign == 0) { /* arg2 is zero */
      rtxBigIntFree (pctxt, result);
      rtxBigIntCopy (pctxt, arg1, result);
      return 0;
   }
   if (arg1->sign != arg2->sign) {
      mag = add (pctxt, arg1->mag, arg1->numocts, arg2->mag, arg2->numocts, &resLength);
      rtxBigIntFree (pctxt, result);
      result->sign = arg1->sign;
      result->mag = mag;
      result->dynamic = 1;
      result->numocts = result->allocated = (int)resLength;
      return 0;
   }

   cmp = compareMagnitudes (arg1->mag, arg1->numocts, arg2->mag, arg2->numocts);
   if (cmp == 0) {
      rtxBigIntFree (pctxt, result);
      result->sign = 0;
      result->numocts = result->allocated = 0;
      result->mag = 0;
      return 0;
   }

   mag = (cmp > 0 ? subtract(pctxt, arg1->mag, arg1->numocts,
                                    arg2->mag, arg2->numocts, &resLength)
                  : subtract(pctxt, arg2->mag, arg2->numocts,
                                    arg1->mag, arg1->numocts, &resLength));
   rtxBigIntFree (pctxt, result);
   if (mag == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   result->sign = cmp * arg1->sign;
   result->mag = mag;
   result->dynamic = 1;
   result->numocts = result->allocated = (int)resLength;
   rtxBigIntRemoveLeadingZeroBytes (result);
   return 0;
}

/*
 * Multiplies int arrays x and y to the specified lengths and places
 * the result into z.
 */
static OSOCTET* multiplyToLen
   (OSCTXT* pctxt, const OSOCTET* x, size_t xlen,
                   const OSOCTET* y, size_t ylen, size_t* resLength)
{
   int xstart = (int)xlen - 1;
   int ystart = (int)ylen - 1;
   OSOCTET* z;
   int i, j, k;
   int carry = 0;

   *resLength = xlen + ylen;
   z = rtxMemAllocArray (pctxt, *resLength, OSOCTET);
   if (z == NULL) return NULL;

   for (j = ystart, k = ystart + 1 + xstart; j >= 0; j--, k--) {
      int product = (y[j] & LONG_MASK) *
                     (x[xstart] & LONG_MASK) + carry;
      z[k] = (OSOCTET)product;
      carry = product >> 8;
   }
   z[xstart] = (OSOCTET)carry;

   for (i = xstart-1; i >= 0; i--) {
      carry = 0;
      for (j = ystart, k = ystart+1+i; j >= 0; j--, k--) {
          int product = (y[j] & LONG_MASK) *
                         (x[i] & LONG_MASK) +
                         (z[k] & LONG_MASK) + carry;
          z[k] = (OSOCTET)product;
          carry = product >> 8;
      }
      z[i] = (OSOCTET)carry;
   }
   return z;
}

EXTRTMETHOD int rtxBigIntMultiply
   (OSCTXT* pctxt,
    OSBigInt* result, const OSBigInt* arg1, const OSBigInt* arg2)
{
   size_t resLength;
   OSOCTET* mag;

   if (arg1->sign == 0 || arg2->sign == 0) {
      rtxBigIntFree (pctxt, result);
      result->sign = 0;
      result->numocts = result->allocated = 0;
      result->mag = 0;
      return 0;
   }

   mag = multiplyToLen(pctxt, arg1->mag, arg1->numocts,
                              arg2->mag, arg2->numocts, &resLength);
   rtxBigIntFree (pctxt, result);
   if (mag == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   result->sign = arg1->sign * arg2->sign;
   result->mag = mag;
   result->dynamic = 1;
   result->numocts = result->allocated = (int)resLength;
   rtxBigIntRemoveLeadingZeroBytes (result);
   return 0;
}

#if 0
static int divide (OSCTXT* pctxt, OSBigInt* a, OSBigInt* b, OSBigInt* quotient, OSBigInt* rem)
{
   int cmp;
   OSOCTET* d;
   size_t dlen;

   if (b->sign == 0 || b->numocts == 0)
      return LOG_RTERR (pctxt, OSERR_INVPARM);

   rtxBigIntInit (pctxt, quotient);
   rtxBigIntInit (pctxt, rem);

   /* Dividend is zero */
   if (b->sign == 0 || b->numocts == 0) {
      return 0;
   }

   cmp = rtxBigIntCompare (a, b);

   /* Dividend less than divisor */
   if (cmp < 0) {
      rtxBigIntCopy (pctxt, a, rem);
      return 0;
   }
   /* Dividend equal to divisor */
   if (cmp == 0) {
      static OSOCTET b[] = { 1 };
      rtxBigIntSetBytes (pctxt, quotient, b, 1);
      return 0;
   }

   /* Copy divisor value to protect divisor */
   d = rtxMemAllocArray (pctxt, b->numocts, OSOCTET);
   if (d == NULL) return NULL;
   dlen = b->numocts;
   OSCRTLMEMCPY (d, b->mag, dlen);

   /* Remainder starts as dividend with space for a leading zero */
   rem->mag = rtxMemAllocArray (pctxt, a->numocts + 1, OSOCTET);

   for (int i=0; i<intLen; i++)
       rem.value[i+1] = value[i+offset];
   rem.intLen = intLen;
   rem.offset = 1;

   int nlen = rem.intLen;
}
#endif

#if 0
int rtxBigIntDivide
   (OSCTXT* pctxt,
    OSBigInt* result, const OSBigInt* arg1, const OSBigInt* arg2)
{
   return 0;
}

    /**
     * Returns an array of two BigIntegers containing <tt>(this / val)</tt>
     * followed by <tt>(this % val)</tt>.
     *
     * @param  val value by which this BigInteger is to be divided, and the
     *	       remainder computed.
     * @return an array of two BigIntegers: the quotient <tt>(this / val)</tt>
     *	       is the initial element, and the remainder <tt>(this % val)</tt>
     *	       is the final element.
     * @throws ArithmeticException <tt>val==0</tt>
     */
    public BigInteger[] divideAndRemainder(BigInteger val) {
        BigInteger[] result = new BigInteger[2];
        MutableBigInteger q = new MutableBigInteger(),
                          r = new MutableBigInteger(),
                          a = new MutableBigInteger(this.mag),
                          b = new MutableBigInteger(val.mag);
        a.divide(b, q, r);
        result[0] = new BigInteger(q, this.signum * val.signum);
        result[1] = new BigInteger(r, this.signum);
        return result;
    }


    /**
     * Calculates the quotient and remainder of this div b and places them
     * in the MutableBigInteger objects provided.
     *
     * Uses Algorithm D in Knuth section 4.3.1.
     * Many optimizations to that algorithm have been adapted from the Colin
     * Plumb C library.
     * It special cases one word divisors for speed.
     * The contents of a and b are not changed.
     *
     */
    void divide(MutableBigInteger b,
                        MutableBigInteger quotient, MutableBigInteger rem) {
        if (b.intLen == 0)
            throw new ArithmeticException("BigInteger divide by zero");

        /* Dividend is zero */
        if (intLen == 0) {
            quotient.intLen = quotient.offset = rem.intLen = rem.offset = 0;
            return;
        }

        int cmp = compare(b);

        /* Dividend less than divisor */
        if (cmp < 0) {
            quotient.intLen = quotient.offset = 0;
            rem.copyValue(this);
            return;
        }
        /* Dividend equal to divisor */
        if (cmp == 0) {
            quotient.value[0] = quotient.intLen = 1;
            quotient.offset = rem.intLen = rem.offset = 0;
            return;
        }

        quotient.clear();

        /* Special case one word divisor */
        if (b.intLen == 1) {
            rem.copyValue(this);
            rem.divideOneWord(b.value[b.offset], quotient);
            return;
        }

        /* Copy divisor value to protect divisor */
        int[] d = new int[b.intLen];
        for(int i=0; i<b.intLen; i++)
            d[i] = b.value[b.offset+i];
        int dlen = b.intLen;

        /* Remainder starts as dividend with space for a leading zero */
        if (rem.value.length < intLen +1)
            rem.value = new int[intLen+1];

        for (int i=0; i<intLen; i++)
            rem.value[i+1] = value[i+offset];
        rem.intLen = intLen;
        rem.offset = 1;

        int nlen = rem.intLen;

        /* Set the quotient size */
        int limit = nlen - dlen + 1;
        if (quotient.value.length < limit) {
            quotient.value = new int[limit];
            quotient.offset = 0;
        }
        quotient.intLen = limit;
        int[] q = quotient.value;

        /* D1 normalize the divisor */
        int shift = 32 - BigInteger.bitLen(d[0]);
        if (shift > 0) {
            /*  First shift will not grow array */
            BigInteger.primitiveLeftShift(d, dlen, shift);
            /*  But this one might */
            rem.leftShift(shift);
        }

        /*  Must insert leading 0 in rem if its length did not change */
        if (rem.intLen == nlen) {
            rem.offset = 0;
            rem.value[0] = 0;
            rem.intLen++;
        }

        int dh = d[0];
        long dhLong = dh & LONG_MASK;
        int dl = d[1];
        int[] qWord = new int[2];

        /*  D2 Initialize j */
        for(int j=0; j<limit; j++) {
            /*  D3 Calculate qhat */
            /*  estimate qhat */
            int qhat = 0;
            int qrem = 0;
            boolean skipCorrection = false;
            int nh = rem.value[j+rem.offset];
            int nh2 = nh + 0x80000000;
            int nm = rem.value[j+1+rem.offset];

            if (nh == dh) {
                qhat = ~0;
                qrem = nh + nm;
                skipCorrection = qrem + 0x80000000 < nh2;
            } else {
                long nChunk = (((long)nh) << 32) | (nm & LONG_MASK);
                if (nChunk >= 0) {
                    qhat = (int) (nChunk / dhLong);
                    qrem = (int) (nChunk - (qhat * dhLong));
                } else {
                    divWord(qWord, nChunk, dh);
                    qhat = qWord[0];
                    qrem = qWord[1];
                }
            }

            if (qhat == 0)
                continue;

            if (!skipCorrection) { /*  Correct qhat */
                long nl = rem.value[j+2+rem.offset] & LONG_MASK;
                long rs = ((qrem & LONG_MASK) << 32) | nl;
                long estProduct = (dl & LONG_MASK) * (qhat & LONG_MASK);

                if (unsignedLongCompare(estProduct, rs)) {
                    qhat--;
                    qrem = (int)((qrem & LONG_MASK) + dhLong);
                    if ((qrem & LONG_MASK) >=  dhLong) {
                        estProduct = (dl & LONG_MASK) * (qhat & LONG_MASK);
                        rs = ((qrem & LONG_MASK) << 32) | nl;
                        if (unsignedLongCompare(estProduct, rs))
                            qhat--;
                    }
                }
            }

            /*  D4 Multiply and subtract */
            rem.value[j+rem.offset] = 0;
            int borrow = mulsub(rem.value, d, qhat, dlen, j+rem.offset);

            /*  D5 Test remainder */
            if (borrow + 0x80000000 > nh2) {
                /*  D6 Add back */
                divadd(d, rem.value, j+1+rem.offset);
                qhat--;
            }

            /*  Store the quotient digit */
            q[j] = qhat;
        } /*  D7 loop on j */

        /*  D8 Unnormalize */
        if (shift > 0)
            rem.rightShift(shift);

        rem.normalize();
        quotient.normalize();
    }
#endif
