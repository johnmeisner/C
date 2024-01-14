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

EXTPERMETHOD int pd_ExtOptElemBits(OSCTXT* pctxt, OSDynOctStr* poctstr)
{
   OSUINT32 i;
   int stat;

   /* Bits are decoded into a dynamic octet string structure. Each bit is
      represented as a boolean byte value in the allocated byte array. */

   /* decode extension optional bits length */

   PU_NEWFIELD (pctxt, "ext opt bits len");

   stat = pd_SmallLength (pctxt, &poctstr->numocts);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   /* this should not happen */
   if (0 == poctstr->numocts) {
      poctstr->data = 0;
      return 0;
   }

   /* decode optional bit values */
   poctstr->data = (OSOCTET*) rtxMemAlloc (pctxt, poctstr->numocts);
   if (0 == poctstr->data) return LOG_RTERR (pctxt, RTERR_NOMEM);

   PU_NEWFIELD (pctxt, "ext opt bit mask");

   for (i = 0; i < poctstr->numocts; i++) {
      stat = DEC_BIT (pctxt, (OSOCTET*)&poctstr->data[i]);
      if (stat != 0)  {
         rtxMemFreePtr (pctxt, poctstr->data);
         return LOG_RTERR (pctxt, stat);
      }
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}


