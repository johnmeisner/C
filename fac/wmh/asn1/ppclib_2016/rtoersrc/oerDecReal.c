/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

#include <math.h>
#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxIntDecode.h"
#include "rtxsrc/rtxReal.h"

#define ANY_BASE 1
#define BASE_2 2
#define BASE_10 3


/**
 * Decoder REAL value.
 * @param baseflag:
 *    ANY_BASE if the encoding may be in any base
 *    BASE_2 if the encoding must be in base 2
 *    BASE_10 if the encoding must be in base 10
 * @param base2Only: If true, only allow base 2 encoding.
 */
int internal_oerDecReal (OSCTXT* pctxt, OSREAL* pvalue, int baseflag)
{
   unsigned char  firstOctet = 0, b = 0;
   int            status;
   OSSIZE         j;
   unsigned int   expLen;
   double         mantissa;
   unsigned short base;
   int            exponent = 0;
   OSSIZE       length;
   
   status = oerDecLen(pctxt, &length);
   if (status != 0) return LOG_RTERR (pctxt, status);
      
   if (length == 0) {
      *pvalue = 0.0;
      return 0;
   }

   /* fetch first octet */
   status = rtxReadBytesSafe (pctxt, &firstOctet, 1, 1);
   if (status != 0) return LOG_RTERR (pctxt, status);

   /* check for special real values (plus/minus infinity) */

   if (length == 1) {
      switch (firstOctet) {
         case ASN1_K_PLUS_INFINITY:
            *pvalue = rtxGetPlusInfinity ();
            return (0);

         case ASN1_K_MINUS_INFINITY:
            *pvalue = rtxGetMinusInfinity ();
            return (0);

         case ASN1_K_NOT_A_NUMBER:
            *pvalue = rtxGetNaN();
            return (0);

         case ASN1_K_MINUS_ZERO:
            *pvalue = rtxGetMinusZero();
            return (0);

         default: return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   --length;

   /* Decode normal ASN.1 real value */

   if (firstOctet & REAL_BINARY) {
      if (baseflag == BASE_10) return LOG_RTERR(pctxt, RTERR_INVREAL);

      switch (firstOctet & REAL_EXPLEN_MASK) {
         case (REAL_EXPLEN_1): expLen = 1; break;
         case (REAL_EXPLEN_2): expLen = 2; break;
         case (REAL_EXPLEN_3): expLen = 3; break;

         default: {  /* longer exponent */
            OSOCTET b;
            status = rtxReadBytesSafe (pctxt, &b, 1, 1);            
            if (status != 0) return LOG_RTERR (pctxt, status);
            expLen = b;
            --length;
         }
      }

      if ( expLen > 4 ) return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
      
      /* Decode exponent as 2's complement integer */
      status = rtxDecInt32(pctxt, &exponent, expLen);
      if (status != 0) return LOG_RTERR (pctxt, status);
                        
      length -= expLen;

      /* Now get the mantissa */

      mantissa = 0.0;

      for (j = 0; j < length; j++) {
         status = rtxReadBytesSafe (pctxt, &b, 1, 1);            
         if (status != 0) return LOG_RTERR (pctxt, status);

         mantissa *= (OSINTCONST(1) << 8);
         mantissa +=  b;
      }

      /* adjust N by scaling factor */

      mantissa *= (OSINTCONST(1) << ((firstOctet & REAL_FACTOR_MASK) >> 2));

      switch(firstOctet & REAL_BASE_MASK) {
         case (REAL_BASE_2):  base = 2;  break;
         default: return LOG_RTERR (pctxt, RTERR_INVREAL);
      }

      *pvalue = mantissa * pow ((double)base, (double)exponent);

      if (firstOctet & REAL_SIGN)
         *pvalue = -*pvalue;
   }
   else {  /* decimal version, process decoding according to ISO 6093 */
      char         *buf, *pb;
      register OSSIZE i;
      int          form, sp = 0;

      if ( baseflag == BASE_2 ) return LOG_RTERR (pctxt, RTERR_INVREAL);
      
      buf = (char*)ASN1BUFPTR (pctxt); /* get the pointer to string */
      if (pctxt->buffer.byteIndex + length <= pctxt->buffer.size) {
         OSBOOL minus = FALSE;
         OSREAL frac = 0.1, result = 0.0;
         char* endp;

         pctxt->buffer.byteIndex += length; /* advance the pointer */
         form = firstOctet & REAL_ISO6093_MASK;
         for (pb = buf, i = length; i > 0; i--, pb++) {
            if (form >= 2 && (*pb == '.' || *pb == ',')) {
               sp++;
            }
            else if ((form >= 1 && ((*pb >= '0' && *pb <= '9') ||
                                    (*pb == '+' || *pb == '-'))) ||
                     (form == 3 && (*pb == 'E' || *pb == 'e')))
            {
               sp++;
            }
            else if (sp == 0 && *pb == ' ') {
               buf++;
               continue;
            }
            else {
               return LOG_RTERR (pctxt, RTERR_INVREAL);
            }

         }

         endp = buf + sp;

         /* Check for '-' first character */

         if (*buf == '-') {
            minus = TRUE; buf++;
         }
         else if (*buf == '+') {
            buf++;
         }

         /* Convert integer part */

         for ( ; buf < endp && *buf != '.' && *buf != ','; buf++ ) {
            if (OS_ISDIGIT (*buf))
               result = (result * 10) + (*buf - '0');
            else
               return LOG_RTERR (pctxt, RTERR_INVREAL);
         }

         /* Convert fractional part */

         if (*buf == '.' || *buf == ',') {
            for (buf++ ; buf < endp && *buf != 'E' && *buf != 'e'; buf++, frac *= 0.1) {
               if (OS_ISDIGIT (*buf))
                  result += (*buf - '0') * frac;
               else
                  return LOG_RTERR (pctxt, RTERR_INVREAL);
            }
         }

         /* Convert exponent */

         if (*buf == 'E' || *buf == 'e') {
            int sign, expo = 0;

            buf++;
            if (*buf == '+')
               sign = 1;
            else if (*buf == '-')
               sign = -1;
            else
               return LOG_RTERR (pctxt, RTERR_INVREAL);
            buf++;

            if (buf >= endp)
               return LOG_RTERR (pctxt, RTERR_INVREAL);

            for ( ; buf < endp; buf++ ) {
               if (OS_ISDIGIT (*buf))
                  expo = (expo * 10) + (*buf - '0');
               else
                  return LOG_RTERR (pctxt, RTERR_INVREAL);
            }

            result *= pow ((OSREAL)10.0, expo * sign);
         }

         /* If negative, negate number */

         if (minus) result = 0 - result;

         *pvalue = result;
      }
      else
         return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   }

   return (0);
}


EXTOERMETHOD int oerDecReal (OSCTXT* pctxt, OSREAL* pvalue)
{
   return internal_oerDecReal(pctxt, pvalue, ANY_BASE);
}


EXTOERMETHOD int oerDecReal2 (OSCTXT* pctxt, OSREAL* pvalue)
{
   return internal_oerDecReal(pctxt, pvalue, BASE_2);
}

/* Note: ASN1VE uses this function because it uses OSREAL even for base 10
   REAL values, unlike asn1c which uses char* */
EXTOERMETHOD int oerDecReal10(OSCTXT* pctxt, OSREAL* pvalue)
{
   return internal_oerDecReal(pctxt, pvalue, BASE_10);
}


EXTOERMETHOD int oerDecRealNTCIP (OSCTXT* pctxt, OSREAL* pvalue)
{
   OSOCTET tmpbuf[256];
   OSBOOL minus = FALSE;
   OSREAL frac = 0.1, result = 0.0;
   char* inpdata = (char*) tmpbuf;
   size_t len; int stat, retval = 0;

   /* Decode length */

   stat = oerDecLen (pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (len == 0)
      return LOG_RTERR (pctxt, RTERR_INVREAL);

   /* Read value into holding buffer, reserving space in tmpbuf for a
      terminating null character.
   */

   stat = rtxReadBytesSafe (pctxt, tmpbuf, sizeof(tmpbuf) - 1, len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   
   tmpbuf[len] = 0;  /* put in a terminating null character */

   /* Check for special values - PLUS-INFINITY and MINUS-INFINITY */

   if (len == 13 &&
       0 == OSCRTLSTRNCMP (inpdata, "PLUS-INFINITY", len)) {
      *pvalue = rtxGetPlusInfinity ();
      return retval;
   }
   else if (len == 14 &&
            0 == OSCRTLSTRNCMP (inpdata, "MINUS-INFINITY", len)) {
      *pvalue = rtxGetMinusInfinity ();
      return retval;
   }

   /* Check for '-' first character */

   if (*inpdata == '-') {
      minus = TRUE; inpdata++;
      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   /* Convert integer part */

   for ( ; *inpdata != 0 && *inpdata != '.'; inpdata++ ) {
      if (OS_ISDIGIT (*inpdata))
         result = (result * 10) + (*inpdata - '0');
      else
         return LOG_RTERR (pctxt, RTERR_INVREAL);
   }

   /* Convert fractional part */

   if (*inpdata == '.') {
      for (inpdata++ ; *inpdata != 0 &&
         *inpdata != 'E' && *inpdata != 'e'; inpdata++, frac *= 0.1)
      {
         if (OS_ISDIGIT (*inpdata))
            result += (*inpdata - '0') * frac;
         else
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   /* Convert exponent */

   if (*inpdata == 'E' || *inpdata == 'e') {
      /* To be clear, only 'E' should be accepted in XER,
         but who knows?.. (AB) */
      int sign, exp = 0;

      inpdata++;

      if (*inpdata == '+')
      	sign = 1;
      else if (*inpdata == '-')
         sign = -1;
      else
         return LOG_RTERR (pctxt, RTERR_INVREAL);

      inpdata++;

      if (*inpdata == 0)
         return LOG_RTERR (pctxt, RTERR_INVREAL);

      for ( ; *inpdata != 0; inpdata++ ) {
         if (OS_ISDIGIT (*inpdata))
            exp = (exp * 10) + (*inpdata - '0');
         else
            return LOG_RTERR (pctxt, RTERR_INVREAL);
      }

      result *= pow ((OSREAL)10.0, exp * sign);
   }

   /* If negative, negate number */

   if (minus) result = 0 - result;

   *pvalue = result;

   return retval;
}


EXTOERMETHOD int oerDecFloat (OSCTXT* pctxt, OSREAL* pvalue)
{
   union {
      OSFLOAT  f;
      OSUINT32 u;
   } u32;
   int ret = rtxDecUInt32 (pctxt, &u32.u, 4);
   if (0 == ret && 0 != pvalue) {
      *pvalue = u32.f;
   }

   return (ret < 0) ? LOG_RTERR (pctxt, ret) : 0;
}

EXTOERMETHOD int oerDecDouble (OSCTXT* pctxt, OSREAL* pvalue)
{
   union {
      OSDOUBLE d;
      OSUINT64 u;
   } u64;
   int ret;

   /* Make sure decoded value will fit in a variable of type OSREAL.  It 
      may not if OSREAL was redefined to be a float type. */
   if (0 != pvalue && sizeof(OSREAL) != 8) 
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   ret = oerDecUInt64 (pctxt, &u64.u);
   if (0 == ret && 0 != pvalue) {
      *pvalue = u64.d;
   }

   return (ret < 0) ? LOG_RTERR (pctxt, ret) : 0;
}

