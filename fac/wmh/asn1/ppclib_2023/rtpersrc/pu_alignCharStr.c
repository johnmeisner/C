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

/* Determine if character string needs to be aligned or not */

EXTPERMETHOD OSBOOL pu_alignCharStr
(OSCTXT* pctxt, OSUINT32 len, OSUINT32 nbits, Asn1SizeCnst* pSize)
{
   if (pctxt->buffer.aligned) {
      OSSIZE lower, upper;
      OSBOOL doAlign = (OSBOOL)(len > 0), extendable;

      pSize = pu_checkSize (pSize, len, &extendable);

      if (0 != pSize) {
         lower = pSize->root.lower;
         upper = pSize->root.upper;
      }
      else {
         lower = 0;
         upper = OSUINT32_MAX;
      }

      if (!extendable && upper < OSUINTCONST(65536)) {
         OSSIZE bitRange = upper * nbits;
         if (upper == lower) {
            /* X.691, clause 26.5.6 */
            if (bitRange <= 16) doAlign = FALSE;
         }
         else {
            /* X.691, clause 26.5.7 */
            if (bitRange < 16) doAlign = FALSE;
         }
      }

      return doAlign;
   }
   else
      return FALSE;
}
