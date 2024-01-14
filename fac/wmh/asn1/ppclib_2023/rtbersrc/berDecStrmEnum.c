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

#include "rtbersrc/asn1berStream.h"

int berDecStrmEnum (OSCTXT *pctxt, OSINT32 *object_p,
                    ASN1TagType tagging, int length)
{
   register int stat = 0;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_ENUM, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   return berDecStrmInt (pctxt, object_p, ASN1IMPL, length);
}

int berDecStrmEnumInt8 (OSCTXT *pctxt, OSINT8 *object_p,
                        ASN1TagType tagging, int length)
{
   OSINT32 tmpval;
   int ret = berDecStrmEnum(pctxt, &tmpval, tagging, length);
   if (0 == ret) {
      if (tmpval >= OSINT8_MIN && tmpval <= OSINT8_MAX)
         *object_p = (OSINT8)tmpval;
      else
         ret = LOG_RTERR(pctxt, RTERR_TOOBIG);
   }
   return ret;
}

int berDecStrmEnumInt16 (OSCTXT *pctxt, OSINT16 *object_p,
                         ASN1TagType tagging, int length)
{
   OSINT32 tmpval;
   int ret = berDecStrmEnum(pctxt, &tmpval, tagging, length);
   if (0 == ret) {
      if (tmpval >= OSINT16_MIN && tmpval <= OSINT16_MAX)
         *object_p = (OSINT16)tmpval;
      else
         ret = LOG_RTERR(pctxt, RTERR_TOOBIG);
   }
   return ret;
}

int berDecStrmEnumUInt (OSCTXT *pctxt, OSUINT32 *object_p,
                        ASN1TagType tagging, int length)
{
   register int stat = 0;

   if (tagging == ASN1EXPL) {
      stat = berDecStrmMatchTag (pctxt, ASN_ID_ENUM, &length, TRUE);
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
      if (stat != 0)
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   return berDecStrmUInt (pctxt, object_p, ASN1IMPL, length);
}

int berDecStrmEnumUInt8 (OSCTXT *pctxt, OSUINT8 *object_p,
                         ASN1TagType tagging, int length)
{
   OSUINT32 tmpval;
   int ret = berDecStrmEnumUInt(pctxt, &tmpval, tagging, length);
   if (0 == ret) {
      if (tmpval <= OSUINT8_MAX)
         *object_p = (OSUINT8)tmpval;
      else
         ret = LOG_RTERR(pctxt, RTERR_TOOBIG);
   }
   return ret;
}

int berDecStrmEnumUInt16 (OSCTXT *pctxt, OSUINT16 *object_p,
                          ASN1TagType tagging, int length)
{
   OSUINT32 tmpval;
   int ret = berDecStrmEnumUInt(pctxt, &tmpval, tagging, length);
   if (0 == ret) {
      if (tmpval <= OSUINT16_MAX)
         *object_p = (OSUINT16)tmpval;
      else
         ret = LOG_RTERR(pctxt, RTERR_TOOBIG);
   }
   return ret;
}
