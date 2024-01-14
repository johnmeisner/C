/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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
#include <stdlib.h>
#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxReal.h"

#define ANY_BASE 0
#define ANY_BINARY 1
#define BASE_2 2

/**
 * Decode REAL value.
 * @param baseflag Controls what bases this function allows for the encoding.
 *    ANY_BASE: any base is allowed
 *    ANY_BINARY: only bases 2, 8, 16 are allowed
 *    BASE_2: only base 2 is allowed
 */
int internal_xd_real (OSCTXT *pctxt, OSREAL *object_p, ASN1TagType tagging,
   int length, int baseflag)
{   
   unsigned char  firstOctet = 0, b = 0;
   int            j, status;
   unsigned int   expLen;
   double         mantissa;
   unsigned short base;
   int            exponent = 0;

   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_REAL)) {
         return berErrUnexpTag (pctxt, ASN_ID_REAL);
      }
      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   if (length == 0) {
      *object_p = 0.0;
      return 0;
   }

   /* fetch first octet */

   status = XD_MEMCPY1 (pctxt, &firstOctet);
   if (status != 0) return LOG_RTERR (pctxt, status);

   /* check for special real values (plus/minus infinity) */

   if (length == 1) {
      switch (firstOctet) {
         case ASN1_K_PLUS_INFINITY:
            *object_p = rtxGetPlusInfinity ();
            return (0);

         case ASN1_K_MINUS_INFINITY:
            *object_p = rtxGetMinusInfinity ();
            return (0);

         case ASN1_K_NOT_A_NUMBER:
            *object_p = rtxGetNaN();
            return (0);

         case ASN1_K_MINUS_ZERO:
            *object_p = rtxGetMinusZero();
            return (0);

         default: return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   --length;

   /* Decode normal ASN.1 real value */

   if (firstOctet & REAL_BINARY) {
      switch (firstOctet & REAL_EXPLEN_MASK) {
         case (REAL_EXPLEN_1): expLen = 1; break;
         case (REAL_EXPLEN_2): expLen = 2; break;
         case (REAL_EXPLEN_3): expLen = 3; break;

         default: {  /* long form */
            status = XD_MEMCPY1 (pctxt, &b);
            if (status != 0) return LOG_RTERR (pctxt, status);
            expLen = b;
            --length;
         }
      }

      /* Decode exponent integer */

      status = xd_integer (pctxt, &exponent, ASN1IMPL, expLen);
      if (status != 0) return LOG_RTERR (pctxt, status);

      length -= expLen;

      /* Now get the mantissa */

      mantissa = 0.0;

      for (j = 0; j < length; j++) {
         status = XD_MEMCPY1 (pctxt, &b);
         if (status != 0) return LOG_RTERR (pctxt, status);

         mantissa *= (OSINTCONST(1) << 8);
         mantissa +=  b;
      }

      /* adjust N by scaling factor */

      mantissa *= (OSINTCONST(1) << ((firstOctet & REAL_FACTOR_MASK) >> 2));

      switch(firstOctet & REAL_BASE_MASK) {
         case (REAL_BASE_2):  base = 2;  break;
         case (REAL_BASE_8): {
            if ( baseflag == BASE_2 ) return LOG_RTERR (pctxt, RTERR_INVREAL);
            else base = 8;
            break;
         }
         case (REAL_BASE_16): {
            if ( baseflag == BASE_2 ) return LOG_RTERR (pctxt, RTERR_INVREAL);
            else base = 16;
            break;
         }
         default: return LOG_RTERR (pctxt, RTERR_INVREAL);
      }

      *object_p = mantissa * pow ((double)base, (double)exponent);

      if (firstOctet & REAL_SIGN)
         *object_p = -*object_p;
   }
   else {  /* decimal version, process decoding according to ISO 6093 */
      char         *buf, *pb;
      register int i;
      int          form, sp = 0;

      if ( baseflag != ANY_BASE ) return LOG_RTERR (pctxt, RTERR_INVREAL);
      
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

         *object_p = result;
      }
      else
         return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   }

   return (0);
}   

/***********************************************************************
 *
 *  Routine name: xd_real
 *
 *  Description:  This routine decodes an ASN.1 real value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat int   Status of operation.  Returned as function result.
 *  object      int     Decoded enumerated value.
 *
 **********************************************************************/
int xd_real (OSCTXT *pctxt, OSREAL *object_p, ASN1TagType tagging,
   int length)
{
   return internal_xd_real(pctxt, object_p, tagging, length, ANY_BASE);
}

int xd_real_bin (OSCTXT *pctxt, OSREAL *object_p, ASN1TagType tagging,
   int length)
{
   return internal_xd_real(pctxt, object_p, tagging, length, ANY_BINARY);
}

int xd_real_der (OSCTXT *pctxt, OSREAL *object_p, ASN1TagType tagging,
   int length)
{
   return internal_xd_real(pctxt, object_p, tagging, length, BASE_2);
}
