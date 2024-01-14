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

#include "asn1mder.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxMemory.h"

int mderDecOctStr (OSCTXT *pctxt, OSOCTET *pbuf, OSUINT16 numocts)
{
   int stat = 0;

   if (0 == pbuf) {
      return LOG_RTERR (pctxt, RTERR_NULLPTR);
   }

   stat = rtxReadBytes (pctxt, pbuf, numocts);

   if (stat < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

int mderDecDynOctStr (OSCTXT *pctxt, const OSOCTET **pbuf, OSUINT16 *numocts)
{
   int stat = 0;

   stat = mderDecUInt16 (pctxt, numocts);
   if (stat < 0) {
      return LOG_RTERR (pctxt, stat);
   }

   *pbuf = (OSOCTET *)rtxMemAlloc(pctxt, *numocts);
   if (0 == *pbuf) {
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   stat = rtxReadBytes (pctxt, (OSOCTET*) *pbuf, *numocts);

   if (stat < 0) {
      rtxMemFreePtr (pctxt, *pbuf);
      return LOG_RTERR (pctxt, stat);
   }

   return 0;
}
