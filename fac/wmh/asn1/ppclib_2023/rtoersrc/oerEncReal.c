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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxIntEncode.h"
#include "rtxsrc/rtxReal.h"
#include "rtxsrc/rtxUtil.h"
#include <math.h>

/* Encode an OER real (i.e. floating point) value */

EXTOERMETHOD int oerEncReal (OSCTXT* pctxt, OSREAL value)
{
   double         mant, tmp;
   unsigned int   imant, last = 0, mult;
   int            e, sign;
   int            i;
   int stat;
   OSOCTET  buf[8];     /* encoding of N value or, in special cases, the entire
                           encoding of the REAL */
   OSSIZE mantLen;      /* length of encoding of N value */
   OSOCTET firstOctet;  /* first octet of encoding; the format octet */
   OSSIZE len;          /* total length, incl. the length determinant */
   OSUINT32 explen;     /* length of exponent encoding */

   /* save space for length determinant. We can assume short form */

   /* special real values for +/- oo, NAN, -0 */
   if (rtxIsMinusInfinity(value)) {
      buf[0] = 1;    /* length */
      buf[1] = ASN1_K_MINUS_INFINITY;
      stat = rtxWriteBytes (pctxt, buf, 2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsPlusInfinity(value)) {
      buf[0] = 1;    /* length */
      buf[1] = ASN1_K_PLUS_INFINITY;
      stat = rtxWriteBytes (pctxt, buf, 2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsNaN(value)) {
      buf[0] = 1;    /* length */
      buf[1] = ASN1_K_NOT_A_NUMBER;
      stat = rtxWriteBytes (pctxt, buf, 2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsMinusZero(value)) {
      buf[0] = 1;    /* length */
      buf[1] = ASN1_K_MINUS_ZERO;
      stat = rtxWriteBytes (pctxt, buf, 2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (value == 0.0) {
      /* no contents for 0.0 reals */
      buf[0] = 0;    /* length */
      stat = rtxWriteBytes (pctxt, buf, 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /* encode a binary real value */
      /* this is what frexp gets from value:
       * value == mantissa * 2 ^ e
       * where 0.5 <= |manitissa| < 1.0  */

      mant = frexp (value, &e);

      /* set sign and make mantissa = | mantissa | */
      if (mant < 0.0) {
         sign = -1;
         mant *= -1;
      }
      else
         sign = 1;

      tmp = mant;

      /* calculate first multiplier to produce normalized REAL.
       * Normalized REAL is necessary for DER/CER.
       * REAL is normalized, if mantissa is either zero or odd. !AB */
      for (i = 0; i < (int)sizeof(double); i++) {
         /* shift 16 bits worth to the left of the decimal */
         tmp *= (OSINTCONST(1) << 16);

         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         if (imant) last = imant;
         else break;
      }

      /* convert mantissa into an unsigned integer */
      mult = (last & 0xFF) ? rtxOctetBitLen ((OSOCTET)(last & 0xFF)) :
         rtxOctetBitLen ((OSOCTET)(last >> 8));
      mantLen = 1;
      /* shift 'mult' bits worth to the left of the decimal */
      tmp = mant * (OSINTCONST(1) << mult);
      e -= mult; /* correct exponent */

      for (i = 0; i < (int)sizeof(double); i++) {
         /* grab only (octet sized) the integer part */
         imant = (unsigned int) tmp;

         /* remove part to left of decimal now for next iteration */
         tmp -= imant;

         /* write into tmp buffer */
         buf[i] = (unsigned char) imant;


         if (tmp == 0.0)
            break;

         /* shift 8 bits worth to the left of the decimal
            (for the next iteration) */
         tmp *= (OSINTCONST(1) << 8);
         mantLen ++;
         e -= 8; /* correct exponent */
      }

      explen = rtxIntByteCount(e);

      /* how many octets to encode the value? */
      len = 1 /* for format */ + mantLen + explen;
      if ( explen > 3  ) len++;     /* for exponent length octet */

      stat = oerEncLen (pctxt, len);
      if (stat != 0 ) return LOG_RTERR (pctxt, stat);

      /*
       * write format octet  (first octet of content)
       *  field  1 S bb ff ee
       *  bit#   8 7 65 43 21
       *
       * 1 in bit#1 means binary rep
       * 1 in bit#2 means the mantissa is neg, 0 pos
       * bb is the base:    65  base
       *                    00    2
       *                    01    8
       *                    10    16
       *                    11    future ext.
       *
       * ff is the Value of F where  Mantissa = sign x N x 2^F
       *    FF can be one of 0 to 3 inclusive. (used to save re-alignment)
       *
       * ee is the length of the exponent:  21   length
       *                                    00     1
       *                                    01     2
       *                                    10     3
       *                                    11     long form
       *
       *
       * encoded binary real value looks like
       *
       *     fmt oct
       *   --------------------------------------------------------
       *   |1Sbbffee|  exponent (2's comp)  |   N (unsigned int)  |
       *   --------------------------------------------------------
       *    87654321
       */

      firstOctet = REAL_BINARY;
      if (sign == -1)
         firstOctet |= REAL_SIGN;

      /* bb is 00 since base is 2 so do nothing */
      /* ff is 00 since no other shifting is nec */

      /* set exponent length indication in format octet */
      switch (explen) {
         case (1): firstOctet |= REAL_EXPLEN_1; break;
         case (2): firstOctet |= REAL_EXPLEN_2; break;
         case (3): firstOctet |= REAL_EXPLEN_3; break;
         default:
            firstOctet |= REAL_EXPLEN_LONG;
            break;
      }

      /* write the format octet */
      stat = rtxWriteBytes(pctxt, &firstOctet, 1);
      if (stat != 0 ) return LOG_RTERR (pctxt, stat);

      if ( explen > 3 )
      {
         /* encode exponent length */
         stat = rtxEncBits(pctxt, explen, 8);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      /* encode e (exponent) as 2's complement int */
      stat = rtxEncUInt32(pctxt, (OSUINT32) e, explen);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* encode N (from mantissa) as unsigned binary int */
      stat = rtxWriteBytes (pctxt, buf, mantLen);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}


EXTOERMETHOD int oerEncReal10 (OSCTXT *pctxt, const char* object_p)
{
   int stat = 0;
   const char* p;
   const char* mantBeg = 0;
   const char* mantEnd = 0;
   int digits = 0;
   int trailZeros = 0;
   int mantExpVal = 0;
   int expVal = 0;
   int tm;
   char mantMinus = 0;
   char expMinus = 0;
   char flFrac = 0;
   char c;
   OSSIZE len;
   OSOCTET expStr[10];

   if (!object_p)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   /* skip ws and leading zeros and plus */
   for (p = object_p; (c = *p) != 0; p++) {
      if (c == '-')
         mantMinus = 1;       /* this adds one to output len */
      else if (c != ' ' && c != '0' && c != '+')
         break;
   }

   mantBeg = mantEnd = p;

   /* count mantissa digits */
   for (; (c = *p) != 0; p++) {
      if (c == '0')
         trailZeros++;
      else if (c > '0' && c <= '9') {
         if (digits == 0 && flFrac) {
            mantExpVal = -trailZeros;
            digits = 1;
            mantBeg = p;
         }
         else
            digits += trailZeros + 1;

         trailZeros = 0;
         mantEnd = p + 1;
      }
      else if (c == '.') {
         mantExpVal = digits + trailZeros;
         flFrac = 1;
      }
      else if (c == 'e' || c == 'E') {
         p++;
         break;
      }
      else if (c != ' ')
         break;
   }

   if (digits > 0) {
      if (!flFrac)
         mantExpVal = digits + trailZeros;

      /* skip ws and leading zeros and plus */
      for (; (c = *p) != 0; p++) {
         if (c == '-')
            expMinus = 1;
         else if (c != ' ' && c != '0' && c != '+')
            break;
      }

      /* get exponent */
      for (; (c = *p) != 0; p++) {
         if (c >= '0' && c <= '9')
            expVal = expVal * 10 + (c - '0');
         else if (c != ' ')
            break;
      }

      if (expMinus)
         expVal = -expVal;

      expVal += mantExpVal;

      expVal -= digits;

      if (expVal < 0) {
         expMinus = 1;
         expVal = -expVal;
      }
      else
         expMinus = 0;

      /* figure out the length of the string and encode it */
      len = mantMinus + digits + 3; /* +3 is for ".E" and format octet */
      if ( expVal == 0 ) len += 2;  /* +2 is for "+0" */
      else len += rtxLog10Floor(expVal) + 1 + expMinus ;

      stat = oerEncLen(pctxt, len);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      tm = digits + 3 + mantMinus;
      stat = rtxCheckOutputBuffer (pctxt, (unsigned)(tm));
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* format octet */
      c = 3;
      OSRTPUTCHAR(pctxt, c);

      if (mantMinus) {
         c = '-';
         OSRTPUTCHAR(pctxt, c);
      }

      /* write mantissa */
      do {
         c = *(mantBeg++);
         if (c < '0' || c > '9')
            continue;

         OSRTPUTCHAR(pctxt, c);
      } while (mantEnd != mantBeg);

      c = '.'; OSRTPUTCHAR(pctxt, c);
      c = 'E'; OSRTPUTCHAR(pctxt, c);

      /* write exponent into expStr buffer, then write exponent string to
         buffer.  Fill chars into expStr from right to left. */
      if (expVal != 0)  {
         OSOCTET* pExp = expStr + sizeof(expStr);  /*start of written exponent*/

         while (expVal) {
            c = expVal % 10;
            expVal /= 10;

            *(--pExp) = '0' + c;
         }

         if (expMinus) {
            *(--pExp) = '-';
         }

         stat = rtxWriteBytes(pctxt, pExp, sizeof(expStr) - (pExp - expStr));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else {
         stat = rtxCheckOutputBuffer (pctxt, 2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         c = '+'; OSRTPUTCHAR(pctxt, c);
         c = '0'; OSRTPUTCHAR(pctxt, c);
      }
   }

   return stat;
}


EXTOERMETHOD int oerEncRealNTCIP (OSCTXT* pctxt, OSREAL value)
{
   int stat = 0;

   if (value == 0.0) {
      static const OSOCTET buf[] = { 0x01, 0x30 };
      stat = rtxWriteBytes (pctxt, buf, sizeof(buf));
   }
   else if (value == rtxGetPlusInfinity ()) {
      OSOCTET len = 13;
      stat = rtxWriteBytes (pctxt, &len, 1);
      if (stat >= 0) {
         stat = rtxWriteBytes (pctxt, (OSOCTET*)"PLUS-INFINITY", len);
      }
   }
   else if (value == rtxGetMinusInfinity ()) {
      OSOCTET len = 14;
      stat = rtxWriteBytes (pctxt, &len, 1);
      if (stat >= 0) {
         stat = rtxWriteBytes (pctxt, (OSOCTET*)"MINUS-INFINITY", len);
      }
   }
   else {
      char lbuf[256], lbuf2[256], *psrc = lbuf, *psrc2;

      /* specify maximum precision after point - 20 */
      os_snprintf (lbuf, sizeof(lbuf), "%1.20E", value);

       /* look for comma decimal point */
      psrc = OSCRTLSTRCHR (lbuf, ',');
      if (psrc != NULL) {
         *psrc = '.';   /* replace with period */
      }

      psrc2 = psrc = OSCRTLSTRCHR (lbuf, 'E');
      if (psrc2 != 0) {
         size_t len;

          /* skip all trailing zeros in fractional part */
         while (*(psrc2 - 1) == '0')
            psrc2--;

         /* ...if fractional part is zero, leave one zero in it */
         if (*(psrc2 - 1) == '.') psrc2++;

         /* copy the first part before 'E' and exp sign */
         rtxStrncpy (lbuf2, sizeof(lbuf2), lbuf, psrc2 - (char*)lbuf);

         /* move src ptr behind 'E' and sign */
         psrc2 = psrc + 2;

         /* skip all leading zeros in exponent */
         while (*psrc2 == '0') psrc2++;
         rtxStrncat (lbuf2, sizeof(lbuf2), psrc, 2); /* copy 'E+' or 'E-' */

         if (*psrc2 != '\0') {
            len = OSCRTLSTRLEN (lbuf);

            /* copy exponent part */
            rtxStrncat (lbuf2, sizeof(lbuf2), psrc2,
                        len - (psrc2 - (char*)lbuf));
         }
         else {
            rtxStrcat (lbuf2, sizeof(lbuf2), "0"); /* exponent is zero */
         }

         len = OSCRTLSTRLEN(lbuf2);

         stat = oerEncLen (pctxt, len);
         if (stat >= 0) {
            stat = rtxWriteBytes (pctxt, (OSOCTET*)lbuf2, len);
         }
      }
      else {
         /* If no exponent in the buffer then it may mean incorrect number
            (such as NaN) */
         return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}


EXTOERMETHOD int oerEncFloat (OSCTXT* pctxt, OSREAL value)
{
   union {
      float f;
      OSUINT32 u;
   } u32;
   int stat;

   u32.f = (float)value;

   /* Test for whether the double value could be cast to float with only
      a loss of precision.
   */
   if ( rtxIsPlusInfinity(u32.f) || rtxIsMinusInfinity(u32.f) )
      return LOG_RTERR (pctxt, RTERR_INVREAL);

   stat = rtxEncUInt32 (pctxt, u32.u, 4);

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}

EXTOERMETHOD int oerEncDouble (OSCTXT* pctxt, OSREAL value)
{
   union {
      double d;
      OSUINT64 u;
   } u64;
   OSSIZE  tmpidx = sizeof(double);
   OSOCTET lb, tmpbuf[sizeof(double)];
   int stat;

   u64.d = value;

   /* Encode value to temp buffer */
   do {
      lb = (OSOCTET) (u64.u % 256);
      u64.u /= 256;
      tmpbuf[--tmpidx] = lb;
   } while (u64.u != 0 && tmpidx > 0);

   /* Add padding */
   while (tmpidx > 0) {
      tmpbuf[--tmpidx] = 0;
   }

   /* Write to stream or memory buffer */
   stat = rtxWriteBytes (pctxt, tmpbuf, sizeof(double));

   return (stat < 0) ? LOG_RTERR (pctxt, stat) : 0;
}
