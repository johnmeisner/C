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

#include "xed_common.hh"

int xerDecCopyBitStr (OSCTXT* pctxt,
                      OSOCTET* pvalue, OSUINT32* pnbits,
                      OSINT32 bufsize, int lastBitOffset)
{
   if ((OSINT32)pctxt->buffer.size > bufsize)
      return LOG_RTERR (pctxt, RTERR_STROVFLW);

   if (lastBitOffset == 0) lastBitOffset = 8;
   *pnbits = (OSUINT32)(pctxt->buffer.size * 8 - (8 - lastBitOffset));
   memcpy (pvalue, pctxt->buffer.data, pctxt->buffer.size);

   return (0);
}

