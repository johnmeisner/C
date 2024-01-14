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

#ifndef _NO_ASN1REAL

#include <math.h>
#include <stdlib.h>
#include "rtpersrc/asn1per.h"
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

static int pd_integer (OSCTXT *pctxt, OSINT32 *object_p, int length)
{
   int status;
   signed char  b;      /* signed   */
   OSUINT32     ub;     /* unsigned */

   /* Copy first byte into a signed char variable and assign it to   */
   /* object.  This should handle sign extension in the case of a    */
   /* negative number..                                              */

   if (length > 0) {
      if ((status = pd_octets (pctxt, (OSOCTET*)&b, 1, 8)) == 0) {
         *object_p = b;
         length--;
      }
      else return LOG_RTERR (pctxt, status);
   }

   /* Now use unsigned bytes to add in the rest of the integer..     */

   while (length > 0) {
      if ((status = pd_bits (pctxt, &ub, 8)) == 0) {
         *object_p = (*object_p * 256) + ub;
         length--;
      }
      else return LOG_RTERR (pctxt, status);
   }

   return 0;
}


/**
 * Decode REAL value.
 * @param base2Only If TRUE, this function will report an error if the REAL
 *    encoding uses any base other than base 2.
 */
int internal_pd_Real (OSCTXT* pctxt, OSREAL* pvalue, OSBOOL base2Only)
{
   OSUINT32 i, len;
   int      stat;
   OSUINT32 firstOctet, b;
   unsigned int expLen;
   double mantissa;
   unsigned short base;
   OSINT32 exponent = 0;

   /* Decode unconstrained length */

   if ((stat = pd_Length (pctxt, &len)) < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   /* Decode contents using BER function */

   PU_NEWFIELD (pctxt, "Real");

   if (len == 0) {
      if (pvalue) *pvalue = 0.0;
      return 0;
   }

   /* fetch first octet */

   stat = pd_bits (pctxt, &firstOctet, 8);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* check for special real values (plus/minus infinity) */

   if (len == 1) {
      switch (firstOctet) {
         case ASN1_K_PLUS_INFINITY:
            if (pvalue) *pvalue = rtxGetPlusInfinity ();
            return (0);

         case ASN1_K_MINUS_INFINITY:
            if (pvalue) *pvalue = rtxGetMinusInfinity ();
            return (0);

         case ASN1_K_NOT_A_NUMBER:
            if (pvalue) *pvalue = rtxGetNaN ();
            return (0);

         case ASN1_K_MINUS_ZERO:
            if (pvalue) *pvalue = rtxGetMinusZero ();
            return (0);

         default: return LOG_RTERR (pctxt, RTERR_INVREAL);
      }
   }

   --len;

   /* Decode normal ASN.1 real value */

   if (firstOctet & REAL_BINARY) {
      switch (firstOctet & REAL_EXPLEN_MASK) {
         case (REAL_EXPLEN_1): expLen = 1; break;
         case (REAL_EXPLEN_2): expLen = 2; break;
         case (REAL_EXPLEN_3): expLen = 3; break;

         default: {  /* long form */
            stat = pd_bits (pctxt, &b, 8);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            expLen = b;
            --len;
         }
      }

      switch (firstOctet & REAL_BASE_MASK) {
      case (REAL_BASE_2):  base = 2;  break;
      default: return LOG_RTERR(pctxt, RTERR_INVREAL);
      }

      /* Decode exponent integer */

      stat = pd_integer (pctxt, &exponent, expLen);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      len -= expLen;

      /* Now get the mantissa */
      if (pvalue) {
         mantissa = 0.0;

         for (i = 0; i < len; i++) {
            stat = pd_bits (pctxt, &b, 8);
            if (stat != 0) return LOG_RTERR (pctxt, stat);

            mantissa *= (OSINTCONST(1) << 8);
            mantissa +=  b;
         }

         /* adjust N by scaling factor */

         mantissa *= (OSINTCONST(1) << ((firstOctet & REAL_FACTOR_MASK) >> 2));

         *pvalue = mantissa * pow ((double)base, (double)exponent);

         if (firstOctet & REAL_SIGN)
            *pvalue = -*pvalue;
      }
      else {
         stat = rtxSkipBytes(pctxt, len);
         if (stat != 0) return LOG_RTERR(pctxt, stat);
      }
   }
   else if (pvalue) {
      /* decimal version, process decoding according to ISO 6093 */
      char         mark, sbuf[256], *buf, *pb;

      if ( base2Only ) return LOG_RTERR (pctxt, RTERR_INVREAL);

      /* this trick is necessary to get the symbol currently used
       * for decimal mark in current locale. For example, some europeans
       * use ',' but americans use '.'.
       */
      os_snprintf (sbuf, 256, "%1.1f", 0.0);
      mark = sbuf[1];
      if (len < sizeof (sbuf))
         buf = (char*) sbuf;
      else {
         buf = (char*) rtxMemAlloc (pctxt, len + 1);
         if (buf == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
      stat = pd_octets (pctxt, (OSOCTET*) buf, len, len * 8);
      if (stat == 0) {
         /* PER REAL in decimal form must be encoded only in NR3 form */
         if ((firstOctet & REAL_ISO6093_MASK) != 3) {
            stat = RTERR_INVREAL;
         }
         else {
            for (pb = buf, i = len; i > 0; i--, pb++) {
               if (*pb == '.' || *pb == ',') {
                  *pb = mark;
               }
               else if (!((*pb >= '0' && *pb <= '9') ||
                          (*pb == '+' || *pb == '-') ||
                          (*pb == 'E' || *pb == 'e')))
               {
                  stat = RTERR_INVREAL;
                  break;
               }
            }
            if (stat == 0) {
               char *pe;
               *pb = 0;
               *pvalue = strtod (buf, &pe);
               if (pb != pe) { /* strtod didn't finish parsing - error */
                  stat = RTERR_INVREAL;
               }
            }
         }
      }
      if (buf != sbuf)
         rtxMemFreePtr (pctxt, buf);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /* Decimal version; pvalue is null. */
      stat = rtxSkipBytes(pctxt, len);
      if (stat != 0) return LOG_RTERR(pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   OSRT_CHECK_EVAL_DATE0 (pctxt);

   return 0;
}


/***********************************************************************
 *
 *  Routine name: pd_Real
 *
 *  Description:  The following function decodes a real value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdata       float*  pointer to real value to receive decoded data
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_Real (OSCTXT* pctxt, OSREAL* pvalue)
{
   return internal_pd_Real(pctxt, pvalue, FALSE);
}


EXTPERMETHOD int pd_Real2 (OSCTXT* pctxt, OSREAL* pvalue)
{
   return internal_pd_Real(pctxt, pvalue, TRUE);
}


#endif
