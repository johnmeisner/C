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

#include <math.h>
#include <stdlib.h>
#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxReal.h"
/* START NOOSS */
#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif /* RTEVAL */
/* END NOOSS */

#define ANY_BASE 0
#define ANY_BINARY 1
#define BASE_2 2

/**
 * Decode REAL value.
 * @param baseflag Controls what bases this function allows for the encoding.
 *    ANY_BASE: any base is allowed, but if ASN1CANON or ASN1DER flags are set,
 *                this will log a non-fatal error if base 8 or 16 is used.
 *    ANY_BINARY: only bases 2, 8, 16 are allowed.  If ASN1CANON or ASN1DER
 *                flags are set, this will log a non-fatal error if bases 8
 *                or 16 are used.
 *    BASE_2: only base 2 is allowed.
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
   OSBOOL         canon;

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

   canon = (pctxt->flags & (ASN1CANON | ASN1DER)) != 0;

   /* Decode normal ASN.1 real value */

   if (firstOctet & REAL_BINARY) {
      unsigned char scaleFactor;

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

      /* Now get the mantissa. There must always be 1+ octets for the mantissa
         as zero is not encoded as a base 2 value.
      */

      status = XD_MEMCPY1(pctxt, &b);
      if (status != 0) return LOG_RTERR(pctxt, status);

      mantissa = b;
      length--;

      if (canon && b == 0)
      {
         rtxErrAddStrParm(pctxt, "mantissa uses extra octets");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }

      for (j = 0; j < length; j++) {
         status = XD_MEMCPY1 (pctxt, &b);
         if (status != 0) return LOG_RTERR (pctxt, status);

         mantissa *= (OSINTCONST(1) << 8);
         mantissa +=  b;
      }

      if (canon && mantissa != 0 && b % 2 == 0)
      {
         rtxErrAddStrParm(pctxt, "mantissa is even");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }

      /* adjust N by scaling factor */
      scaleFactor = (firstOctet & REAL_FACTOR_MASK) >> 2;
      mantissa *= OSINTCONST(1) << scaleFactor;

      if (canon && scaleFactor != 0)
      {
         rtxErrAddStrParm(pctxt, "scaling factor is nonzero");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }

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

      if (canon && base != 2)
      {
         rtxErrAddStrParm(pctxt, "base 2 encoding is required");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }

      *object_p = mantissa * pow ((double)base, (double)exponent);

      if (firstOctet & REAL_SIGN)
         *object_p = -*object_p;
   }
   else {  /* decimal version, process decoding according to ISO 6093 */
      char         *buf;

      if ( baseflag != ANY_BASE ) return LOG_RTERR (pctxt, RTERR_INVREAL);

      buf = (char*)ASN1BUFPTR (pctxt); /* get the pointer to string */
      if (pctxt->buffer.byteIndex + length <= pctxt->buffer.size) {
         int          form;

         pctxt->buffer.byteIndex += length; /* advance the pointer */
         form = firstOctet & REAL_ISO6093_MASK;
         status = xd_real_b10_content(pctxt, object_p, buf, length, form);
         if (status != 0) return LOG_RTERR(pctxt, status);
      }
      else
         return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   }

   return (0);
}


int xd_real_b10_content(OSCTXT *pctxt, OSREAL *object_p, const char* content,
   int length, int form)
{
   const char   *buf, *endp, *pb;
   register int i;
   int          sp = 0;
   OSBOOL minus = FALSE;
   OSREAL frac = 0.1, result = 0.0;
   OSBOOL spaceFlag = FALSE;
   OSBOOL canon = (pctxt->flags & (ASN1CANON | ASN1DER)) != 0;

   buf = content;

   if (canon && form != 3)
   {
      rtxErrAddStrParm(pctxt, "NR3 form required");
      LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
      rtxErrSetNonFatal(pctxt);
   }

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
         spaceFlag = TRUE;
         continue;
      }
      else {
         return LOG_RTERR(pctxt, RTERR_INVREAL);
      }
   }

   endp = buf + sp;

   if (canon && spaceFlag)
   {
      rtxErrAddStrParm(pctxt, "spaces not allowed");
      LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
      rtxErrSetNonFatal(pctxt);
   }


   /* Check for '-' first character */

   if (*buf == '-') {
      minus = TRUE; buf++;
   }
   else if (*buf == '+') {
      buf++;
      if (canon)
      {
         rtxErrAddStrParm(pctxt, "+ sign not allowed");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }
   }

   /* Convert integer part */

   for (; buf < endp && *buf != '.' && *buf != ','; buf++) {
      if (OS_ISDIGIT(*buf)) {
         if (canon && result == 0.0 && *buf == '0')
         {
            rtxErrAddStrParm(pctxt, "mantissa begins with zero digit");
            LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
            rtxErrSetNonFatal(pctxt);
         }

         result = (result * 10) + (*buf - '0');
      }
      else
         return LOG_RTERR(pctxt, RTERR_INVREAL);
   }

   if (canon && *(buf - 1) == '0')
   {
      rtxErrAddStrParm(pctxt, "mantissa ends with zero digit");
      LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
      rtxErrSetNonFatal(pctxt);
   }

   if (canon && (buf + 1 >= endp || *buf != '.' || *(buf + 1) != 'E'))
   {
      rtxErrAddStrParm(pctxt, "mantissa must be followed by '.E'");
      LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
      rtxErrSetNonFatal(pctxt);
   }

   /* Convert fractional part */

   if (*buf == '.' || *buf == ',') {
      for (buf++; buf < endp && *buf != 'E' && *buf != 'e'; buf++, frac *= 0.1) {
         if (OS_ISDIGIT(*buf))
            result += (*buf - '0') * frac;
         else
            return LOG_RTERR(pctxt, RTERR_INVREAL);
      }
   }

   /* Convert exponent */

   if (*buf == 'E' || *buf == 'e') {
      int sign = 0, expo = 0;

      buf++;
      if (*buf == '+') {
         sign = 1;
         buf++;
      }
      else if (*buf == '-') {
         sign = -1;
         buf++;
      }

      if (buf >= endp)
         return LOG_RTERR(pctxt, RTERR_INVREAL);

      if (canon && *buf == '0')
      {
         if (buf + 1 < endp)
         {
            rtxErrAddStrParm(pctxt, "exponent cannot have leading zeros");
            LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
            rtxErrSetNonFatal(pctxt);
         }
         else if (sign == -1 || sign == 0)
         {
            rtxErrAddStrParm(pctxt, "zero exponent must use '+' sign");
            LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
            rtxErrSetNonFatal(pctxt);
         }
      }
      else if (canon && sign == 1)
      {
         rtxErrAddStrParm(pctxt, "only zero exponent may use '+' sign");
         LOG_RTERRNEW(pctxt, ASN_E_NOTCANON);
         rtxErrSetNonFatal(pctxt);
      }

      if (sign == 0) sign = 1;

      for (; buf < endp; buf++) {
         if (OS_ISDIGIT(*buf))
            expo = (expo * 10) + (*buf - '0');
         else
            return LOG_RTERR(pctxt, RTERR_INVREAL);
      }

      result *= pow((OSREAL)10.0, expo * sign);
   }

   OSRT_CHECK_EVAL_DATE0 (pctxt);

   /* If negative, negate number */

   if (minus) result = 0 - result;

   *object_p = result;

   return 0;
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
