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

/***********************************************************************
 *
 *  Routine name: pd_BitString
 *
 *  Description:  The following function decodes a bit string value
 *                into a static buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  numbits     int*    pointer to variable to receive decoded number of bits
 *  buffer      octet*  pointer to buffer to receive decoded octet string data
 *  bufsiz      int     size of the data buffer
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_BitString64
(OSCTXT* pctxt, OSSIZE* numbits_p, OSOCTET* buffer, OSSIZE bufsiz)
{
   OSSIZE bitcnt;
   OSSIZE octidx = 0;
   int lstat, stat;

   /* Save size constraint for alignment test.  Version in context will be
      consumed by pd_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   if (numbits_p) *numbits_p = 0;

   for (;;) {
      lstat = pd_Length64 (pctxt, &bitcnt);
      if (lstat < 0) return LOG_RTERR (pctxt, lstat);

      PU_NEWFIELD (pctxt, "bits");

      if (bitcnt > 0) {
         if(numbits_p) *numbits_p += bitcnt;

         if (pctxt->buffer.aligned) {
            if (pu_BitAndOctetStringAlignmentTest (&sizeCnst, bitcnt, TRUE)) {
               stat = PD_BYTE_ALIGN (pctxt);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
         }

         if (buffer) stat = pd_octets (pctxt, &buffer[octidx], bufsiz - octidx,
                                       bitcnt);
         else stat = pd_octets(pctxt, 0, 0, bitcnt);

         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      PU_SETBITCOUNT (pctxt);

      if (lstat == RT_OK_FRAG) {
         octidx += (bitcnt / 8);
         if (octidx > bufsiz) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
      }
      else break;
   }

   return 0;
}

EXTPERMETHOD int pd_BitString
(OSCTXT* pctxt, OSUINT32* numbits_p, OSOCTET* buffer, OSSIZE bufsiz)
{
   OSSIZE len;
   int ret = pd_BitString64 (pctxt, &len, buffer, bufsiz);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   else if (sizeof(len) > 4 && len > OSUINT32_MAX) {
      return LOG_RTERR (pctxt, ret);
   }
   if (0 != numbits_p) *numbits_p = (OSUINT32) len;
   return 0;
}

EXTPERMETHOD int pd_BitString32
(OSCTXT* pctxt, ASN1BitStr32* pvalue, OSSIZE lower, OSSIZE upper)
{
   int ret;
   PU_SETSIZECONSTRAINT (pctxt, lower, upper, 0, 0);

   ret = pd_BitString
      (pctxt, &pvalue->numbits, pvalue->data, sizeof(pvalue->data));

   if (0 == ret) {
      if ((lower > 0 && pvalue->numbits < lower) || pvalue->numbits > upper) {
#ifndef _COMPACT
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddUIntParm (pctxt, pvalue->numbits);
         return LOG_RTERR (pctxt, RTERR_CONSVIO);
#else
         ret = RTERR_CONSVIO;
#endif
      }
   }

   return ret;
}
