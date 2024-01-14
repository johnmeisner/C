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

EXTOERMETHOD int oerDecChoiceExt(OSCTXT* pctxt, ASN1TAG tag, OSOCTET** ppvalue,
                                 OSSIZE* pnocts)
{
   int retval = 0, stat = 0;
   size_t len;          /* length of alternative's encoding */
   size_t totalLen;     /* total length of encoding - incl. tag & length det */
   size_t bufsize;      /* size of buffer to decode into */
   OSOCTET* pbuf;       /* buffer to decode into */

   /* Decode the length */
   stat = oerDecLen (pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Figure the length required to hold the tag, length, and decoded value. */
   totalLen = len;
   totalLen += oerTagLength(tag);
   totalLen += oerLenLength(len);

   *ppvalue = pbuf = (OSOCTET*) rtxMemAlloc (pctxt, totalLen);
   if (*ppvalue == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

   bufsize = *pnocts = totalLen;

   /* write the tag into the buffer */
   stat = oerWriteTag(tag, pbuf, bufsize);
   if ( stat < 0 ) return LOG_RTERR (pctxt, stat);
   else {
      /* adjust buffer pointer as size to account for tag */
      bufsize -= stat;
      pbuf += stat;
   }

   /* write the length determinant into the buffer */
   stat = oerWriteLen(len, pbuf, bufsize);
   if ( stat < 0 ) return LOG_RTERR (pctxt, stat);
   else {
      /* adjust buffer pointer as size to account for lenght determinant */
      bufsize -= stat;
      pbuf += stat;
   }

   /* decode the contents into the rest of the buffer.
      bufsize == len is expected.
   */
   stat = rtxReadBytes (pctxt, pbuf, bufsize);

   if (stat < 0) return LOG_RTERR (pctxt, stat);

   return retval;
}
