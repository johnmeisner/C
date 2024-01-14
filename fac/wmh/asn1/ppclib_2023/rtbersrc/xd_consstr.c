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

#include "rtbersrc/asn1ber.h"

/* Calculates the actual length of the octet string in constructed form.
 *
 * @param size The actual number of octets used for the storing constructed
 *             octet string
 * @param length The length necessary for the assembled string.
 * @param cerVioFlag Function will set this to FALSE if a CER rule is violated,
 *          regardless of whether ASN1CANON flag is set.
 *
 * This function is internal, used by xd_consStrIndefLenAndSize_internal.
 * Having this function would not be strictly necessary, but it does let us
 * ensure that we only log one error for a CER fragmentation violation.
 */
int xd_consStrIndefLenAndSize_internal
   (OSCTXT* pctxt, ASN1TAG expectedTag, OSSIZE* length, OSSIZE* size,
    OSBOOL* cerVioFlag)
{
   OSRTBufSave savedBufferInfo;
   ASN1TAG tag;
   OSSIZE  len, ll = 0;
   OSSIZE  prevLen = 0;    /* length of previous fragment. When 0, there was
                              no previous fragment or cerVioFlag must be set.
                              Once cerVioFlag is set, this might not be
                              accurate anymore.
                            */
   int     stat;
   OSBOOL  indefLen;

   xu_SaveBufferState (pctxt, &savedBufferInfo);

   for (;;) {
      stat = xd_tag_len_64 (pctxt, &tag, &len, &indefLen, XM_ADVANCE);
      if (stat != 0) LOG_RTERR (pctxt, stat);

      if ((pctxt->flags & ASN1CONSTAG) != 0)
      {
         /* CER requires all fragments to be primitive */
         *cerVioFlag = TRUE;
      }

      /* check tag value */

      if ( !(tag == 0 || tag == expectedTag) )
         return LOG_RTERR (pctxt, ASN_E_BADTAG);

      if (indefLen) {
         /* Indefinite length implies constructed encoding, and so cerVioFlag
            will be set. We don't set prevLen here, but it becomes irrelevant
            now.
         */
         OSSIZE sz = 0, ll1 = 0;

         if ((stat = xd_consStrIndefLenAndSize_internal
            (pctxt, expectedTag, &ll1, &sz, cerVioFlag)) != 0)
         {
            return LOG_RTERR (pctxt, stat);
         }
         ll += ll1;
         pctxt->buffer.byteIndex += sz;
      }
      else if (tag == 0 && len == 0) {
         /* EOC marker.
            CER requires last segment to be 1 - 1000 octets. */
         if ( prevLen == 0 || prevLen > 1000 ) *cerVioFlag = TRUE;
         break;
      }
      else if (len > 0) {
         if (prevLen > 0 && prevLen != 1000)
         {
            /* CER requires all but last segment to be exactly 1000 octets. */
            *cerVioFlag = TRUE;
         }

         prevLen = len;
         pctxt->buffer.byteIndex += len;
         ll += len;
      }
      else {
         /* len == 0.
            CER does not allow any fragments to be zero length */
         *cerVioFlag = TRUE;
      }
   }

   *size = (pctxt->buffer.byteIndex -
            savedBufferInfo.byteIndex - 2); /* exclude EOC */

   *length = ll;

   xu_RestoreBufferState (pctxt, &savedBufferInfo);

   return 0;
}


/* Calculates the actual length of the octet string in constructed form.
*
* @param size The actual number of octets used for the storing constructed
*             octet string
* @param length The length necessary for the assembled string.
*
* This function is internal, used by OCTET, CHARACTER and BIT STRINGs.
*/
int xd_consStrIndefLenAndSize
(OSCTXT* pctxt, ASN1TAG expectedTag, OSSIZE* length, OSSIZE* size)
{
   OSBOOL cerVioFlag = FALSE;

   int retval = xd_consStrIndefLenAndSize_internal(pctxt, expectedTag, length,
                                                   size, &cerVioFlag);

   /* The above will have validated all CER requirements except that the
      total string be > 1000 octets in order to be encoded in constructed
      form.
   */
   if ( *length <= 1000 ) cerVioFlag = TRUE;

   if ( cerVioFlag && rtxCtxtTestFlag(pctxt, ASN1CANON))
   {
      LOG_RTERRNEW(pctxt, ASN_E_FRAGMENTS);
      rtxErrSetNonFatal(pctxt);
   }

   return retval;
}
