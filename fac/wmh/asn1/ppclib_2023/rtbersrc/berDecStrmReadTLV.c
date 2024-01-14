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

int berDecStrmGetTLVLength (OSCTXT* pctxt)
{
#ifndef _NO_STREAM
   int len, stat, totalLen;
   OSSIZE byteIndex;
   ASN1TAG tag;

   if ((stat = rtxStreamMark (pctxt, 32)) != 0) {
      return LOG_RTERR (pctxt, stat);
   }
   byteIndex = OSRTSTREAM_BYTEINDEX(pctxt);

   if ((stat = berDecStrmTagAndLen (pctxt, &tag, &len)) != 0) {
      return LOG_RTERR (pctxt, stat);
   }
   totalLen = len;
   if (len > 0) {
      totalLen += (int)(OSRTSTREAM_BYTEINDEX(pctxt) - byteIndex);
   }
   else if (len != ASN_K_INDEFLEN) {
      return LOG_RTERR (pctxt, RTERR_INVLEN);
   }

   if ((stat = rtxStreamReset (pctxt)) != 0) {
      return LOG_RTERR (pctxt, stat);
   }

   return totalLen;
#else
   return 0;
#endif
}

int berDecStrmReadTLV (OSCTXT* pctxt, OSOCTET* buf, OSSIZE bufsiz)
{
#ifndef _NO_STREAM
   int len;

   if (0 == buf || bufsiz < 2)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   len = berDecStrmGetTLVLength (pctxt);
   if (len > 0) {
      if (len <= (int)bufsiz) {
         return rtxStreamRead (pctxt, buf, len);
      }
      else return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   }
   else if (len == ASN_K_INDEFLEN) {
      const OSOCTET* pOpenTypeBuf = 0;
      OSSIZE numocts;

      int ret = berDecStrmOpenType (pctxt, &pOpenTypeBuf, &numocts);
      if (0 == ret) {
         if (numocts <= bufsiz) {
            OSCRTLMEMCPY (buf, pOpenTypeBuf, numocts);
            rtxMemFreePtr (pctxt, (void*)pOpenTypeBuf);
            return (numocts <= OSINT32_MAX) ? (int)numocts : RTERR_TOOBIG;
         }
         else {
            rtxMemFreePtr (pctxt, (void*)pOpenTypeBuf);
            return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
         }
      }
      else return LOG_RTERR (pctxt, ret);
   }
   else return LOG_RTERR (pctxt, len);
#else
   return 0;
#endif
}

int berDecStrmReadDynTLV (OSCTXT* pctxt, OSOCTET** ppbuf)
{
#ifndef _NO_STREAM
   int len;

   if (0 == ppbuf) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   len = berDecStrmGetTLVLength (pctxt);

   if (len > 0) {
      *ppbuf = (OSOCTET*) rtxMemAlloc (pctxt, len);
      if (0 == *ppbuf) return LOG_RTERR (pctxt, RTERR_NOMEM);

      return rtxStreamRead (pctxt, *ppbuf, len);
   }
   else if (len == ASN_K_INDEFLEN) {
      OSSIZE numocts;

      int ret = berDecStrmOpenType (pctxt, (const OSOCTET**)ppbuf, &numocts);
      if (0 == ret) {
         if (numocts > OSINT32_MAX) {
            rtxMemFreePtr (pctxt, *ppbuf);
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         }
         else return (int)numocts;
      }
      else return LOG_RTERR (pctxt, ret);
   }
   else return LOG_RTERR (pctxt, len);
#else
   return 0;
#endif
}
