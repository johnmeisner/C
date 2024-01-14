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

#include "rtbersrc/asn1ber.h"
#include "rtsrc/asn1intl.h"

int xd_32BitCharStr64 (OSCTXT* pctxt, Asn132BitCharString* object_p,
                       ASN1TagType tagging, ASN1TAG tag, OSSIZE length,
                       OSBOOL indefLen)
{
   OSSIZE     i, nchars = 0;
   OSINT32    stat = 0;
   OSOCTET*   data = 0;
   OS32BITCHAR* data32 = 0;
   OSBOOL isConstructedTag;

   if (tagging == ASN1EXPL) {
      stat = xd_match1_64 (pctxt, ASN1TAG2BYTE(tag), &length, &indefLen);
      if (stat < 0)
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }
   isConstructedTag = (OSBOOL)((pctxt->flags & ASN1CONSTAG) != 0);

   if (0 == length) {
      nchars = 0;
      data32 = 0;
   }
   else {
      OSSIZE size, ll = 0;

      /* For an indefinite length message, need to get the actual 	*/
      /* length by parsing tags until the end of the message is 	*/
      /* reached..							*/
      if (indefLen) {  /* indefinite length message */
         stat = xd_consStrIndefLenAndSize
            (pctxt, ASN_ID_OCTSTR, &ll, &size);

         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      else ll = size = length;

      data = (OSOCTET*) rtxMemAlloc (pctxt, ll);
      data32 = (OS32BITCHAR*)data;

      if (0 != data) {
         ll = size;
         if ((stat = xd_octstr64_s
              (pctxt, data, &ll, ASN1IMPL, size, FALSE)) != 0)
            return LOG_RTERR_AND_FREE_MEM (pctxt, stat, data);

         nchars = ll / 4;
         for (i = 0; i < nchars; i++, data += 4) {
            data32[i] = (OS32BITCHAR) (((data[0] * 256) + data[1]) *
               65536UL + (((data[2] * 256) + data[3])));
         }
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   if (isConstructedTag && indefLen) {
      if (XD_MATCHEOC (pctxt)) XD_BUMPIDX (pctxt, 2);
      else return LOG_RTERR_AND_FREE_MEM (pctxt, ASN_E_INVLEN, data);
   }
   object_p->data = data32;
   object_p->nchars = nchars;

   return 0;
}

int xd_32BitCharStr (OSCTXT* pctxt, Asn132BitCharString* object_p,
                     ASN1TagType tagging, ASN1TAG tag, int length)
{
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   return xd_32BitCharStr64
      (pctxt, object_p, tagging, tag, (OSSIZE)length, indefLen);
}
