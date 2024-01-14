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

/***********************************************************************
 *
 *  Routine name: xd_len
 *
 *  Description:  This routine decodes the length field component of an
 *                ASN.1 message.  It is called by the xd_tag_len routine
 *                which handles the decoding of both the ID and length
 *                fields.
 *
 *  Inputs:
 *
 *  None
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Operation status returned as function result
 *  length      int     Decoded length value returned as function result
 *                      as follows:
 *			>= 0 		fixed length
 *			ASN_K_INDEFLEN	indefinite length
 *
 **********************************************************************/

int xd_len64 (OSCTXT *pctxt, OSSIZE* len_p, OSBOOL* pindef)
{
   OSOCTET b = XD_FETCH1 (pctxt);
   register int	i;

   *len_p = 0;
   *pindef = FALSE;

   if (b == 0x80)
   {
      /* indefinite length case */
      *len_p = OSSIZE_MAX;
      *pindef = TRUE;

      if (rtxCtxtTestFlag(pctxt, ASN1DER))
      {
         /* DER requires definite length for constructed encodings */
         LOG_RTERRNEW(pctxt, ASN_E_LEN_FORM);
         rtxErrSetNonFatal(pctxt);
      }
   }
   else {
      /* definite length case */

      if (rtxCtxtTestFlag(pctxt, ASN1CONSTAG) &&
            rtxCtxtTestFlag(pctxt, ASN1CANON))
      {
         /* CER requires indefinite length for constructed encodings */
         LOG_RTERRNEW(pctxt, ASN_E_LEN_FORM);
         rtxErrSetNonFatal(pctxt);
      }

      if (b > 0x80) {
         /* long-form definite length */
         OSSIZE prevLen = 0;
         OSBOOL requireMinimal = (pctxt->flags & (ASN1CANON | ASN1DER) ) != 0;

         i = b & 0x7F;     /* i = # of octets for length; i > 0 */

         /* Do not check size here. It is possible length is left-padded with
            zero bytes and can still fit in a size variable. Check for
            integer overflow later to determine if length is too large
            (ED, 4/23/2021)..
         if ((OSSIZE)i > sizeof(OSSIZE))
            return LOG_RTERR (pctxt, ASN_E_INVLEN);
         */
         /* Get first length octet and check if minimal # of octets is used. */
         b = XD_FETCH1(pctxt);
         if ( requireMinimal && (b == 0 || (i == 1 && b < 0x80)) )
         {
            /* If first octet is zero, fewer octets could have been used.
               If only 1 octet is used and the high bit was not needed, then
               short form could have been used. */
            LOG_RTERRNEW(pctxt, ASN_E_LEN_NOT_MIN);
            rtxErrSetNonFatal(pctxt);
         }

         /* Update length with current octet and then continue to do the same
            for any remaining length octets. */
         for (;;) {
            *len_p = (*len_p * 256) + b;

            if (*len_p < prevLen) {
               /* overflow */
               return LOG_RTERR (pctxt, RTERR_TOOBIG);
            }
            prevLen = *len_p;

            if ( i == 1 ) break;
            else {
               i--;
               b = XD_FETCH1(pctxt);
            }
         }
      }
      else *len_p = b;     /* short-form definite length */
   }

   /* It should not be an error if the length exceeds the number of bytes
      available in the buffer.  That is because it is possible to decode
      large messages in parts which would be read into the buffer in
      stages.  It should only be an error if a read is attempted past
      the end of the buffer. (ED, 10/13/2016)
   if (!*pindef) {
      if (*len_p > (pctxt->buffer.size - pctxt->buffer.byteIndex))
         return LOG_RTERR (pctxt, ASN_E_INVLEN);
   }
   */

   return (0);
}

int xd_len (OSCTXT *pctxt, int *len_p)
{
   OSSIZE len;
   OSBOOL indef;
   int ret = xd_len64 (pctxt, &len, &indef);
   if (0 == ret) {
      if (indef) *len_p = ASN_K_INDEFLEN;
      else if (len > OSINT32_MAX) return LOG_RTERR (pctxt, RTERR_TOOBIG);
      else *len_p = (int)len;
   }
   return (0 == ret) ? 0: LOG_RTERR (pctxt, ret);
}

