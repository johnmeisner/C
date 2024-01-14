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
 *  Routine name: xd_octstr_s  (decode static octet string)
 *
 *  Description:  This routine decodes the octet string at the current
 *                message pointer location and returns its value.  The
 *                value is returned in the buffer pointed to by the given
 *                character buffer pointer.  This routine also advances
 *                the message pointer to the start of the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, the string is copied into the static buffer
 *                using xd_memcpy.  The numocts field is then set to the
 *                length parsed from the field.  The status result of
 *                the operation is returned.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object      char*	Pointer to decoded octet string value.
 *  numocts	int	Number of octets in string.
 *
 **********************************************************************/

int xd_octstr64_s (OSCTXT* pctxt,
                   OSOCTET* object_p, OSSIZE* pnumocts,
                   ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
   int stat = 0;

   if (tagging == ASN1EXPL) {
      stat = xd_match1_64 (pctxt, ASN_ID_OCTSTR, &length, &indefLen);
      if (stat < 0) {
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
      }
   }

   /* Check length */
   if (length > *pnumocts)
      return LOG_RTERR (pctxt, RTERR_STROVFLW);

   if (pctxt->flags & ASN1CONSTAG) {
      OSOCTET* ptr = OSRTBUFPTR(pctxt);
      OSSIZE idx = 0, len = length;

      for (;;) {
         /* replaces XD_CHKEND */
         if (indefLen) {
            if (XD_CHKEOB (pctxt)) break;
         }
         else if (((OSSIZE)(OSRTBUFPTR(pctxt) - ptr) >= length) ||
                  (pctxt->buffer.byteIndex >= pctxt->buffer.size)) break;

         len = length;
         stat = xd_octstr64_s
            (pctxt, object_p + idx, &len, ASN1EXPL, length, indefLen);

         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else idx += len;
      }

      /* If not indefinite length, make sure we consumed exactly as  */
      /* many bytes as specified in the constructed length field..   */

      if (!indefLen) {
         OSSIZE usedBytes = OSRTBUFPTR(pctxt) - ptr;
         if (usedBytes != length)
            return LOG_RTERR (pctxt, ASN_E_INVLEN);
      }

      *pnumocts = idx;
   }
   else if (length > 0) {
      if ((pctxt->buffer.byteIndex + length) <= pctxt->buffer.size) {
         OSCRTLMEMCPY (object_p, OSRTBUFPTR(pctxt), length);
         pctxt->buffer.byteIndex += length;
         *pnumocts = length;
      }
      else return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }
   else {
      *pnumocts = 0;
   }

   return (0);
}

int xd_octstr_s (OSCTXT* pctxt,
                 OSOCTET* object_p, OSUINT32* pnumocts,
                 ASN1TagType tagging, int length)
{
   OSSIZE numocts = *pnumocts;
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   OSSIZE len2 = (length > 0) ? (OSSIZE)length : 0;

   int ret = xd_octstr64_s
      (pctxt, object_p, &numocts, tagging, len2, indefLen);

   if (0 == ret) {
      if (sizeof(numocts) > 4 && numocts > OSUINT32_MAX)
         return (LOG_RTERR (pctxt, RTERR_TOOBIG));

      *pnumocts = (OSUINT32)numocts;
   }

   return ret;
}
