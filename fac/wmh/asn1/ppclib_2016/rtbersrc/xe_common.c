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

/***********************************************************************
 *
 *  COMMON ENCODE FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: xe_identifier
 *
 *  Description:  This routine encodes an ASN identifier as used in ASN
 *                tag and ASN object identifier definitions.  The
 *                encoding of an identifier is accomplished through a
 *                series of octets, each of which contains a 7 bit
 *                unsigned number. The 8th bit (MSB) of each octet is
 *                used as a continuation flag to indicate that more
 *                octets follow in the sequence.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  ident	uint	Identifier to encode.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  msglen      int     Number of bytes generated to represent given
 *			identifier value.  Returned as function result.
 *
 **********************************************************************/

int xe_identifier (OSCTXT *pctxt, OSUINT32 ident)
{
   register int aal = 0, ll;
   OSOCTET b, first_byte = TRUE;

   do
   {
      b = (OSOCTET) ((first_byte) ?
             (ident % 128) : ((ident % 128) | 0x80));

      aal = ((ll = xe_memcpy (pctxt, &b, 1)) >= 0) ? aal + ll : ll;
      if (aal < 0) break;

      ident /= 128; first_byte = FALSE;
   }
   while (ident > 0);

   return (aal);
}
/***********************************************************************
 *
 *  Routine name: xe_tag_len
 *
 *  Description:  This routine encodes an ASN.1 tag and length value onto
 *		  a message.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  ASN1TAG	short   ASN tag to be encoded into the message.
 *  length      int     Length of message component.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_tag (OSCTXT *pctxt, ASN1TAG tag)
{
   int          aal = 0, ll, rshift;
   ASN1TAG      ltag;
   OSUINT32	id_code = tag & TM_IDCODE;
   OSOCTET	b, class_form;

   /* Split tag into class/form and ID code components */

   rshift = (sizeof(ltag) * 8) - 3;
   ltag = tag >> rshift;
   class_form = (OSOCTET)(ltag << 5);

   /* Encode components */

   if (id_code < 31) {
      b = (OSOCTET) (class_form + id_code);
      aal = ((ll = xe_memcpy (pctxt, &b, 1)) >= 0) ? aal + ll : ll;
   }
   else {
      aal = ((ll = xe_identifier (pctxt, id_code)) >= 0) ? aal + ll : ll;
      if (aal > 0) {
         b = (OSOCTET) (class_form | TM_B_IDCODE);
         aal = ((ll = xe_memcpy (pctxt, &b, 1)) >= 0) ? aal + ll : ll;
      }
   }

   return aal;
}

int xe_tag_len (OSCTXT *pctxt, ASN1TAG tag, int length)
{
   int aal, ll;

   if (length < 0 && length != ASN_K_INDEFLEN) return (length);

   aal = ((ll = xe_len (pctxt, length)) >= 0) ?
      (length == ASN_K_INDEFLEN) ? ll : length + ll : ll;

   if (aal > 0) {
      ll = xe_tag (pctxt, tag);
      aal = (ll >= 0) ? aal + ll : ll;
   }

   return (aal);
}

/***********************************************************************
 *
 *  Routine name: xe_len
 *
 *  Description:  This routine encodes the length field component of an
 *                ASN.1 message.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  length	int	Length value to encode.  A value of zero means
 *                      an indefinite length value should be encoded.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Number of bytes generated to represent given
 *			length value.  Returned as function result.
 *
 **********************************************************************/

int xe_len (OSCTXT *pctxt, int length)
{
   OSBOOL indef = (length == ASN_K_INDEFLEN) ? TRUE : FALSE;
   if (length < 0 && !indef) return LOG_RTERR (pctxt, RTERR_INVLEN);
   else return xe_len64 (pctxt, (OSSIZE)length, indef);
}

int xe_len64 (OSCTXT *pctxt, OSSIZE length, OSBOOL indef)
{
   int aal = 0, ll;
   OSOCTET i = 0, b;
   OSBOOL  extlen = FALSE;

   if (!indef) {
      extlen  = (OSBOOL)(length > 127);
      do {
         b   = (OSOCTET) (length % 256); /* shave off LSB of length */
         aal = ((ll = xe_memcpy (pctxt, &b, 1)) >= 0) ? aal + ll : ll;
         length /= 256; i++;
      }
      while (length > 0 && aal >= 0);
   }
   else
      extlen = TRUE;  /* indefinite length */

   if (extlen && aal >= 0)
   {
      i |= 0x80;
      aal = ((ll = xe_memcpy (pctxt, &i, 1)) >= 0) ? aal + ll : ll;
   }

   return (aal);
}

