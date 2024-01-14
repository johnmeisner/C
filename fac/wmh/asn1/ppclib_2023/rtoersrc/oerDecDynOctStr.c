/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

#include "rtoersrc/asn1oer.h"
#include "rtxsrc/rtxBuffer.h"

/* Decode an OER octet string into a dynamic buffer */

EXTOERMETHOD int oerDecDynOctStr64
(OSCTXT *pctxt, OSOCTET** ppvalue, OSSIZE* pnocts, size_t len)
{
   int stat = 0, retval = 0;

   /* If length passed in is zero, it indicate a variable length string.
      Decode length. */
   if (len == 0) {
      stat = oerDecLen (pctxt, &len);
      if (stat == ASN_E_NOTCANON)
         retval = stat;
      else
         if (0 != stat) return LOG_RTERR (pctxt, stat);

      if (len == 0) {
         if (0 != pnocts) *pnocts = 0;
         if (0 != ppvalue) *ppvalue = 0;
         return retval;
      }
   }

   /* Decode contents */
   if (0 != ppvalue) {
      if ((pctxt->flags & ASN1FASTCOPY) != 0 && !OSRTISSTREAM (pctxt)) {
         *ppvalue = OSRTBUFPTR (pctxt);
         pctxt->buffer.byteIndex += len;
      }
      else {
         *ppvalue = (OSOCTET*) rtxMemAlloc (pctxt, len);
         if (*ppvalue == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

         stat = rtxReadBytes (pctxt, *ppvalue, len);
      }
   }
   else {
      stat = rtxReadBytes (pctxt, 0, len);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pnocts) *pnocts = len;

   return retval;
}


EXTOERMETHOD int oerDecDynOctStr
(OSCTXT *pctxt, OSOCTET** ppvalue, OSUINT32* pnocts, size_t len)
{
   OSSIZE nocts;
   int ret;

   ret = oerDecDynOctStr64(pctxt, ppvalue, &nocts, len);
   if (ret != ASN_E_NOTCANON)
      if (0 != ret) return LOG_RTERR (pctxt, ret);

#if (SIZE_MAX > LLONG_MAX )
   if ( nocts > OSUINT32_MAX ) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);
#endif

   if ( pnocts != 0 ) *pnocts = (OSUINT32) nocts;

   return ret;
}
