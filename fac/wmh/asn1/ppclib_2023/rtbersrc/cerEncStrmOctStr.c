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

int cerEncStrmOctStr (OSCTXT*  pctxt,
                      const OSOCTET* object_p,
                      OSUINT32   numocts,
                      ASN1TagType tagging)
{
   int stat;

   if (numocts <= 1000) { /* primitive encoding */
      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndLen (pctxt, TM_UNIV|TM_PRIM|ASN_ID_OCTSTR,
                                     numocts);
      else
         stat = berEncStrmLength (pctxt, numocts);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = berEncStrmWriteOctets (pctxt, object_p, numocts);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else { /* segmented CER encoding, ITU-T X.680, clause 9.2 */
      OSUINT32 curOff;

      if (tagging == ASN1EXPL)
         stat = berEncStrmTagAndIndefLen (pctxt,
                                          TM_UNIV|TM_CONS|ASN_ID_OCTSTR);
      else
         stat = berEncStrmWriteOctet (pctxt, 0x80);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      for (curOff = 0; curOff < numocts; curOff += 1000) {
         OSUINT32 len = numocts - curOff;

         if (len > 1000) len = 1000;

         stat = berEncStrmTagAndLen (pctxt,
                                     TM_UNIV|TM_PRIM|ASN_ID_OCTSTR, len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         stat = berEncStrmWriteOctets (pctxt, object_p + curOff, len);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      stat = berEncStrmEOC (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   return 0;
}

