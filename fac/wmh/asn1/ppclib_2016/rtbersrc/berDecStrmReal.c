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

#include "rtbersrc/asn1berStream.h"

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
int internal_berDecStrmReal (OSCTXT *pctxt, OSREAL *object_p,
                    ASN1TagType tagging, int length, int baseflag)
{
   OSRTBuffer savedBuf;
   OSOCTET tmpbuf[16];
   register int stat = 0;

   if (tagging == ASN1EXPL) {
      if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_REAL, &length, TRUE)) != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   /* Read encoded value into memory */

   if (length > (int)(sizeof(tmpbuf))) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   stat = rtxReadBytes (pctxt, tmpbuf, length);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Decode */

   memcpy (&savedBuf, &pctxt->buffer, sizeof(OSRTBuffer));

   /* Null the buffer's data so that a free() is not performed in
      rtxInitContextBuffer... */
   pctxt->buffer.data = NULL;
   rtxInitContextBuffer (pctxt, tmpbuf, length);

   switch ( baseflag ) {
      case ANY_BASE: 
         stat = xd_real (pctxt, object_p, ASN1IMPL, length); break;
      case ANY_BINARY: 
         stat = xd_real_bin (pctxt, object_p, ASN1IMPL, length); break;
      case BASE_2:
         stat = xd_real_der (pctxt, object_p, ASN1IMPL, length); break;
      default:
         return LOG_RTERR(pctxt, RTERR_INVPARAM);
   }

   /* Reset context buffer */

   memcpy (&pctxt->buffer, &savedBuf, sizeof(OSRTBuffer));

   return stat;
}


int berDecStrmReal (OSCTXT *pctxt, OSREAL *object_p,
                    ASN1TagType tagging, int length)
{
   return internal_berDecStrmReal(pctxt, object_p, tagging, length, ANY_BASE);
}

int berDecStrmRealBin (OSCTXT *pctxt, OSREAL *object_p,
                    ASN1TagType tagging, int length)
{
   return internal_berDecStrmReal(pctxt, object_p, tagging, length, ANY_BINARY);
}

int berDecStrmRealDer (OSCTXT *pctxt, OSREAL *object_p,
                    ASN1TagType tagging, int length)
{
   return internal_berDecStrmReal(pctxt, object_p, tagging, length, BASE_2);
}