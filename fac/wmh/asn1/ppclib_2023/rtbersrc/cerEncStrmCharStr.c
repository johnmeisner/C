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

int cerEncStrmCharStr (OSCTXT* pctxt, const char* object_p,
                       ASN1TagType tagging, ASN1TAG tag)
{
   int stat;
   int ll;

   if (0 == object_p) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   ll = (int)strlen(object_p);
   if (ll <= 1000) { /* primitive encoding */
      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndLen (pctxt, tag, ll);
      else
         stat = berEncStrmLength (pctxt, ll);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = berEncStrmWriteOctets (pctxt, (const OSOCTET*)object_p, ll);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else { /* segmented CER encoding, ITU-T X.680, clause 9.2 */
      int curOff;

      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndIndefLen (pctxt, (tag & (~TM_PRIM)) | TM_CONS);
      else
         stat = berEncStrmWriteOctet (pctxt, 0x80);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      for (curOff = 0; curOff < ll; curOff += 1000) {
         int len = ll - curOff;

         if (len > 1000) len = 1000;

         stat = berEncStrmTagAndLen (pctxt,
                                     TM_UNIV|TM_PRIM|ASN_ID_OCTSTR, len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = berEncStrmWriteOctets (pctxt,
            (const OSOCTET*)(object_p + curOff), len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      stat = berEncStrmEOC (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

