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

#include "rtbersrc/xse_common.hh"

int cerEncStrmBMPStr (OSCTXT* pctxt, const Asn116BitCharString* object_p,
                      ASN1TagType tagging)
{
   OSOCTET lbuf[2];
   int stat = 0;
   OSUINT32 i;

   if (0 == object_p) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   if (object_p->nchars <= 1000/2) {
      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_BMPString,
                                     (OSUINT32)(object_p->nchars * 2));
      else
         stat = berEncStrmLength (pctxt, (OSUINT32)(object_p->nchars * 2));
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      for (i = 0; i < object_p->nchars; i++) {
         lbuf[0] = (OSOCTET) (object_p->data[i] / 256);
         lbuf[1] = (OSOCTET) (object_p->data[i] % 256);

         stat = berEncStrmWriteOctets (pctxt, lbuf, 2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else { /* segmented CER encoding, ITU-T X.680, clause 9.2 */
      OSUINT32 curOff;

      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndIndefLen (pctxt,
                                          TM_UNIV|TM_CONS|ASN_ID_BMPString);
      else
         stat = berEncStrmWriteOctet (pctxt, 0x80);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      for (curOff = 0; curOff < object_p->nchars; curOff += 1000/2) {
         OSSIZE len = object_p->nchars - curOff;

         if (len > 1000/2) len = 1000/2;

         stat = berEncStrmTagAndLen (pctxt,
                                     TM_UNIV|TM_PRIM|ASN_ID_OCTSTR,
                                     (OSUINT32)(len * 2));
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         for (i = curOff, len += curOff; i < len; i++) {
            lbuf[0] = (OSOCTET) (object_p->data[i] / 256);
            lbuf[1] = (OSOCTET) (object_p->data[i] % 256);

            stat = berEncStrmWriteOctets (pctxt, lbuf, 2);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
      }
      stat = berEncStrmEOC (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   return 0;
}

