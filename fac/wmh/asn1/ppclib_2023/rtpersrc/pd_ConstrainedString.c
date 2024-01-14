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

#include "rtpersrc/asn1per.h"
#include "rtsrc/asn1intl.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_ConstrainedString
 *
 *  Description:  The following function decodes a constrained
 *                string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        char**  pointer to pointer to receive null-term string
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status
 *
 **********************************************************************/

EXTPERMETHOD int pd_ConstrainedString
(OSCTXT* pctxt, const char** string, Asn1CharSet* pCharSet)
{
   if (0 == pCharSet)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
   else {
#ifndef ASN1UPER
      return pd_ConstrainedStringEx (pctxt, string, pCharSet->charSet.data,
                                     pCharSet->charSetAlignedBits,
                                     pCharSet->charSetUnalignedBits,
                                     pCharSet->canonicalSetBits);
#else
      return uperDecConstrString (pctxt, string, pCharSet->charSet.data,
                                  pCharSet->charSetUnalignedBits,
                                  pCharSet->canonicalSetBits);
#endif
   }
}

EXTPERMETHOD int pd_ConstrainedStringData
(OSCTXT* pctxt, char* strbuf, OSSIZE bufsize, OSSIZE len,
 const char* charSet, OSSIZE nbits,  OSSIZE canSetBits)
{
   int stat = 0;
   size_t i = 0;
   OSUINT32 idx;

   if ((len + 1) > bufsize) {
      return RTERR_STROVFLW;
   }
   if ( strbuf ) {
      if (nbits >= canSetBits && canSetBits > 4) {
         for (; i < len; i++) {
            if ((stat = pd_bits (pctxt, &idx, nbits)) == 0) {
               strbuf[i] = (char) idx;
            }
            else break;
         }
      }
      else if (0 != charSet) {
         size_t nchars = strlen (charSet);
         for (; i < len; i++) {
            if ((stat = pd_bits (pctxt, &idx, nbits)) == 0) {
               if (idx < nchars) {
                  strbuf[i] = charSet[idx];
               }
               else return RTERR_CONSVIO;
            }
            else break;
         }
      }
      else return RTERR_INVPARAM;
   }
   else stat = rtxSkipBits(pctxt, len * nbits);

   if (0 == stat && strbuf) {
      strbuf[i] = '\0';  /* add null-terminator */
   }

   return stat;
}

EXTPERMETHOD int pd_DynConstrainedStringData
(OSCTXT* pctxt, const char** ppstr, OSSIZE len,
 const char* charSet, OSSIZE nbits,  OSSIZE canSetBits)
{
   int ret = 0;
   OSSIZE bufsize = len + 1;
   char* pstring;

   if (ppstr) {
      pstring = (char*)rtxMemAlloc(pctxt, bufsize);
      if (!pstring) return RTERR_NOMEM;
   }
   else pstring = 0;

   ret = pd_ConstrainedStringData
      (pctxt, pstring, bufsize, len, charSet, nbits, canSetBits);

   if (ppstr) *ppstr = pstring;

   return ret;
}

#ifndef ASN1UPER
EXTPERMETHOD int pd_ConstrainedStringEx
(OSCTXT* pctxt, const char** string, const char* charSet,
 /* aligned bits */ OSUINT32 abits,  /* unaligned bits */ OSUINT32 ubits,
 OSUINT32 canSetBits)
{
   int   stat;
   char* tmpstr;

   OSUINT32 len, nbits = pctxt->buffer.aligned ? abits : ubits;

   /* note: need to save size constraint for use in pu_alignCharStr     */
   /* because it will be cleared in pd_Length from the context..        */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   /* Decode length */

   stat = pd_Length (pctxt, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   else if (sizeCnst.ext.upper > 0) {
      if (len > sizeCnst.ext.upper) {
         return LOG_RTERR (pctxt, RTERR_STROVFLW);
      }
   } else if (sizeCnst.root.upper > 0) {
      if (len > sizeCnst.root.upper) {
         return LOG_RTERR (pctxt, RTERR_STROVFLW);
      }
   }

   /* Byte-align */

   PU_NEWFIELD (pctxt, "data");

   if (pu_alignCharStr (pctxt, len, nbits, &sizeCnst)) {
      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Decode data */

   if (string) {
      tmpstr = (char*) ((len < OSUINT32_MAX) ? rtxMemAlloc (pctxt, len+1) : 0);
      if (tmpstr == 0) return LOG_RTERR(pctxt, RTERR_NOMEM);
   }
   else tmpstr = 0;


   stat = pd_ConstrainedStringData
      (pctxt, tmpstr, len+1, len, charSet, nbits, canSetBits);

   if (stat != 0)
      return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);


   if (string) *string = tmpstr;

   PU_SETBITCOUNT (pctxt);

   LCHECKPER (pctxt);

   return 0;
}

EXTPERMETHOD int pd_ConstrFixedLenStringEx
(OSCTXT* pctxt, char* strbuf, size_t bufsiz, const char* charSet,
 /* aligned bits */ OSUINT32 abits,  /* unaligned bits */ OSUINT32 ubits,
 OSUINT32 canSetBits)
{
   int   stat;
   OSUINT32 len, nbits = pctxt->buffer.aligned ? abits : ubits;

   /* note: need to save size constraint for use in pu_alignCharStr     */
   /* because it will be cleared in pd_Length from the context..        */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   /* Decode length */

   stat = pd_Length (pctxt, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   else if (len >= bufsiz) return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Byte-align */

   PU_NEWFIELD (pctxt, "data");

   if (pu_alignCharStr (pctxt, len, nbits, &sizeCnst)) {
      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Decode data */

   stat = pd_ConstrainedStringData
      (pctxt, strbuf, bufsiz, len, charSet, nbits, canSetBits);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}
#endif

/* Unaligned PER versions */

EXTPERMETHOD int uperDecConstrString
(OSCTXT* pctxt, const char** string, const char* charSet,
 /* unaligned bits */ OSUINT32 nbits, OSUINT32 canSetBits)
{
   int   stat;
   char* tmpstr;
   OSUINT32 len;

   /* Decode length */

   stat = pd_Length (pctxt, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Decode data */

   PU_NEWFIELD (pctxt, "data");

   if (string) {
      tmpstr = (char*) rtxMemAlloc (pctxt, len+1);
      if (tmpstr == 0) return LOG_RTERR(pctxt, RTERR_NOMEM);
   }
   else tmpstr = 0;

   stat = pd_ConstrainedStringData
      (pctxt, tmpstr, len+1, len, charSet, nbits, canSetBits);

   if (stat != 0)
      return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);

   if (string) *string = tmpstr;

   PU_SETBITCOUNT (pctxt);

   LCHECKPER (pctxt);

   return 0;
}

EXTPERMETHOD int uperDecConstrFixedLenString
(OSCTXT* pctxt, char* strbuf, size_t bufsiz, const char* charSet,
 /* unaligned bits */ OSUINT32 nbits, OSUINT32 canSetBits)
{
   int   stat;
   OSUINT32 len;

   /* Decode length */

   stat = pd_Length (pctxt, &len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   else if (len >= bufsiz) return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Decode data */

   PU_NEWFIELD (pctxt, "data");

   stat = pd_ConstrainedStringData
      (pctxt, strbuf, len+1, len, charSet, nbits, canSetBits);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}
