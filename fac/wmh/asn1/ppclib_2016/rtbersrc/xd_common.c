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
 *  Routine name: xd_tag
 *
 *  Description:  This routine decodes an ASN tag into a standard 16 bit
 *                structure.  This structure represents a tag as follows:
 *
 *		   Bit#: 11 1 1110000000000
 *                       54 3 2109876543210
 *			|__|_|_____________|
 *                       ^  ^  ID code
 *                       |  |
 *                       |  +- Form:
 *                       |     0 = Primitive,
 *                       |     1 = Constructor
 *                       |
 *                       +---- Class:
 *                             0 (00) = Universal
 *                             1 (01) = Application
 *                             2 (10) = Context-specific
 *                             3 (11) = Private
 *
 *                Note: This tag structure can be expanded to 32 bits
 *                (29 for the ID code) by including the following #define
 *                statement in rtsrc/asn1type.h:
 *
 *                #define ASN1C_EXPANDED_TAGS
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  tag		struct	Structure containing class, form, and id_code.
 *
 ***********************************************************************/

int xd_tag (OSCTXT* pctxt, ASN1TAG *tag_p)
{
   ASN1TAG      class_form, id_code;
   int          lcnt = 0;
   OSOCTET    b = XD_FETCH1 (pctxt);

   *tag_p = 0;

   class_form = (ASN1TAG)(b & TM_CLASS_FORM);
   class_form <<= ASN1TAG_LSHIFT;

   if ((id_code = (b & TM_B_IDCODE)) == 31) {
      id_code = 0;
      do {
         b = XD_FETCH1 (pctxt);
         id_code = (id_code * 128) + (b & 0x7F);
         if (id_code > TM_IDCODE || lcnt++ > 8)
            return LOG_RTERR(pctxt, ASN_E_BADTAG);
      } while (b & 0x80);
   }

   *tag_p = class_form | id_code;

   /* Set constructed tag bit in context flags based on parsed value */
   SET_ASN1CONSTAG(pctxt, class_form);

   return (0);
}

/***********************************************************************
 *
 *  Routine name: xd_tag_len
 *
 *  Description:  This routine parses the ASN.1 tag and length fields
 *                located at the current message pointer position and
 *                advances the message pointer to the start of the
 *                contents field.
 *
 *                The routine first saves the current message pointer
 *                and length values on local variables.  It then takes
 *                the byte located at the message pointer and parses it
 *                according to the ASN.1 BER rules to get the class, form,
 *                and ID values.  The internal routine xd_len is then
 *                called to parse the length value.
 *
 *                xd_tag_len monitors indefinite length messages as
 *                follows:  Each time a length is parsed, it is checked
 *                to see if it is an indefinite length value.  If it is,
 *                an indefinite length section counter is incremented.
 *                Each time an end-of-contents (EOC) identifier is
 *                parsed, this counter is decremented.  When the counter
 *                goes to zero, end of message is signalled.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  flags	byte	Bit flags used to set the following options:
 *                      XM_ADVANCE: Advance decode pointer on match.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result
 *  tag_p	ushort	Structure containing class, form, and id_code
 *  len_p       int     Length of message component.  Returned as follows:
 *			>= 0 		component is fixed length
 *			ASN_K_INDEFLEN	component is indefinite length
 *
 ***********************************************************************/

int xd_tag_len_64
(OSCTXT *pctxt, ASN1TAG *tag_p, OSSIZE *len_p, OSBOOL* pIndefLen, OSOCTET flags)
{
   int stat;
   OSUINT16 mask = ASN1INDEFLEN | ASN1LASTEOC;

   if (0 == pctxt || 0 == tag_p || 0 == len_p || 0 == pIndefLen)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   /* Check for attempt to read past EOB */

   if (0 != (pctxt->flags & ASN1INDEFLEN)) {
      if ((pctxt->flags & mask) == mask)
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }
   else if (pctxt->buffer.byteIndex >= pctxt->buffer.size) {
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }

   /* Save context prior to parsing this tag and length.  It may be	*/
   /* used by generated decode functions to restore the decode point if	*/
   /* an operation is to be retried..					*/

   OSRTBUFSAVE (pctxt);

   if ((stat = XD_TAG (pctxt, tag_p)) == 0)
   {
      stat = xd_len64 (pctxt, len_p, pIndefLen);

      /* Indefinite length message check.. if indefinite length parsed,	*/
      /* bump indefinite length section counter; if EOC ID parsed, 	*/
      /* decrement count.  If overall message is of indefinite length 	*/
      /* and count goes to zero, signal end of message.			*/

      if (stat == 0)
      {
         if (*pIndefLen) {

            /* Verify form of tag is constructed.  If not, indefinite   */
            /* length is invalid ..                                     */

            if (0 == (pctxt->flags & ASN1CONSTAG))
               return LOG_RTERR(pctxt, ASN_E_INVLEN);
         }
      }
   }

   if (!(flags & XM_ADVANCE)) OSRTBUFRESTORE (pctxt);

   return (stat != 0) ? LOG_RTERR(pctxt, stat) : 0;
}

int xd_tag_len (OSCTXT *pctxt, ASN1TAG *tag_p, int *len_p, OSOCTET flags)
{
   OSSIZE len64 = 0;
   OSBOOL indefLen = FALSE;

   int ret = xd_tag_len_64 (pctxt, tag_p, &len64, &indefLen, flags);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (indefLen) *len_p = ASN_K_INDEFLEN;
   else if (len64 > OSINT32_MAX) return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else *len_p = (int)len64;

   return 0;
}
