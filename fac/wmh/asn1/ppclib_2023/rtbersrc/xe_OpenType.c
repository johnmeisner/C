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
 *  Routine name: xe_OpenType
 *
 *  Description:  This routine encodes an ASN.1 open type field.
 *                An open type is assumed to be a previously encoded
 *                message which is to be included as is in the current
 *                encode buffer.
 *
 *                An open type is represented by a byte pointer in a C
 *                structure.  This routine first checks this pointer to
 *                see if it is null.  If it is, control is immediately
 *                returned to the caller.  If not, it is assumed that it
 *                points to an encoded message.  A tag/length decode
 *                function is called to validate the ID and parse the
 *                length field.  The parsed length value is then used to
 *                copy the encoded message into the current encode buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  object      void *  Pointer to ASN.1 value to be encoded.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_OpenType (OSCTXT* pctxt, const OSOCTET* object_p, OSSIZE numocts)
{
   OSCTXT  lctxt;
   int     aal = 0, ilen, stat;
   ASN1TAG tag;
   OSSIZE  len;
   OSBOOL  already_encoded, indefLen;

   if (numocts > 0) {
      if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

      if ((stat = rtInitSubContext (&lctxt, pctxt)) != 0)
         return LOG_RTERR(pctxt, stat);

      stat = xd_setp64 (&lctxt, object_p, numocts, &tag, &len, &indefLen);
      rtxFreeContext (&lctxt);

      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (tag == 0 && len == 0) /* EOC is not allowed as OpenType */
         return LOG_RTERR(pctxt, RTERR_BADVALUE);

      /* An any field is considered to be already encoded if the given	*/
      /* pointer is equal to the current encode pointer.  This will be	*/
      /* the case when an extra field is being prepended onto an	*/
      /* existing ASN.1 message (for example, the X.410 PDU)..		*/

      already_encoded = (OSBOOL)(object_p == OSRTBUFPTR(pctxt));

      /* For an indefinite length message, need to get the actual 	*/
      /* length by parsing tags until the end of the message is 	*/
      /* reached..							*/

      if (indefLen) {  /* indefinite length message */
         if ((stat = xd_indeflen64 (object_p, pctxt->buffer.size, &len)) < 0)
            return LOG_RTERR (pctxt, stat);
      }

      if (len > OSINT32_MAX) return LOG_RTERR (pctxt, RTERR_TOOBIG);
      else ilen = (int) len;

      /* If not already copied, copy message component to encode buffer	*/

      aal = (already_encoded) ?
	 ilen : xe_memcpy (pctxt, object_p, ilen);
   }
   else {
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   return (aal);
}
