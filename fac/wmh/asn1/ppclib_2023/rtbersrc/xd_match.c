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
 *  Routine name: xd_match
 *
 *  Description:  This routine compares the tag ID field located at
 *                the current message pointer position with the given
 *                class and ID value.  If a match occurs, the length
 *                field is decoded and the length is returned to the
 *                caller.  If the input parameter 'advance' is set to
 *                TRUE, the message pointer is advanced to the beginning
 *                of the contents field.  A subsequent decode call
 *                can then be used to decode the data.
 *
 *                If a match does not occur, the routine will skip to
 *                subsequent fields in search of a match.  If a match
 *                is eventually found, the processing described above
 *                is done; otherwise, a not found status is returned to
 *                the caller.
 *
 *                The routine first takes the byte located at the message
 *                pointer and parses it according to the ASN.1 BER rules to
 *                get the class, form, and ID values.  It then compares
 *                these values with the given values.  If a match occurs,
 *                the internal routine xd_len is called to parse the
 *                length value.  This value is returned to the caller
 *                and the message pointer is advanced to point to the
 *                start of the data field.
 *
 *                If the match operation fails, the message pointer is
 *                advanced to the next field by adding the parsed length
 *                value to the pointer.  The comparison is then attempted
 *                again.  This is repeated until either a match occurs or
 *                the end of the message is encountered.  If end of message
 *                is encountered, the message pointer is returned to its
 *                original location and a not found status is returned to
 *                the caller.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  tag		ushort	Tag to match
 *  flags	byte	Bit flags used to set the following options:
 *                      XM_ADVANCE: Advance decode pointer on match.
 *			XM_SEEK   : Seek until match found or EOM.
 *                      XM_SKIP   : Skip to next tag before search
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  length      int     Length of message component.  Returned as follows:
 *			>= 0 		component is fixed length
 *			ASN_K_INDEFLEN	component is indefinite length
 *
 ***********************************************************************/

int xd_match64 (OSCTXT *pctxt, ASN1TAG tag, OSSIZE* len_p,
                OSBOOL* pindef, OSOCTET flags)
{
   OSSIZE       parsed_len;
   ASN1TAG      parsed_tag;
   OSRTBufSave  savedBufferInfo;
   int          found = FALSE, status;
   OSBOOL       constructed, indefLen;

   tag &= ~TM_CONS;

   /* If skip flag set, advance decode pointer to next field */

   if (flags & XM_SKIP)
   {
      status = xd_tag_len_64
         (pctxt, &parsed_tag, &parsed_len, &indefLen, XM_ADVANCE);

      if (status == 0)
      {
         if (!(parsed_tag & TM_CONS)) {
            pctxt->buffer.byteIndex += parsed_len;
         }
      }
      else {
         OSRTBUFRESTORE (pctxt);
         if (status == RTERR_ENDOFBUF) {
            rtxErrReset (pctxt);

            /* If element is optional, do not set error params */
#ifndef _COMPACT
            if (!(flags & XM_OPTIONAL)) {
               berErrAddTagParm (pctxt, tag);          /* expected tag */
               berErrAddTagParm (pctxt, parsed_tag);   /* parsed tag   */
            }
#endif

            /* Only return status, do not log error.  Error will be     */
            /* logged by caller..                                       */
            return RTERR_IDNOTFOU;
         }
         else
            return LOG_RTERR(pctxt, status);
      }
   }

   /* Save the context here in the event a seek is to be performed.	*/
   /* If element being seeked is not found, this will be needed to 	*/
   /* return to the original starting point..				*/

   xu_SaveBufferState (pctxt, &savedBufferInfo);

   do
   {
      status = xd_tag_len_64
         (pctxt, &parsed_tag, &parsed_len, &indefLen, XM_ADVANCE);

      if (status == 0) {
         constructed = (OSBOOL)(0 != (parsed_tag & TM_CONS));
         parsed_tag &= ~TM_CONS;
         if (tag == parsed_tag) {
            found = TRUE;
            if (len_p) *len_p = parsed_len;
            if (pindef) *pindef = indefLen;
            if (!(flags & XM_ADVANCE))
               OSRTBUFRESTORE (pctxt);
         }
         else if ((flags & XM_SEEK) && (!constructed)) {
            /* advance decode pointer */
            pctxt->buffer.byteIndex += parsed_len;
         }
      }
   } while (!found  && (flags & XM_SEEK) && status == 0);

   if (!found)
   {
      xu_RestoreBufferState (pctxt, &savedBufferInfo);

      if (status == RTERR_ENDOFBUF || status == 0)
      {
         rtxErrReset (pctxt);
#ifndef _COMPACT
         berErrAddTagParm (pctxt, tag);	/* expected tag */
         berErrAddTagParm (pctxt, parsed_tag);	/* parsed tag */
#endif
         return RTERR_IDNOTFOU; /* it will be logged later, by the generated
            code, or reset by rtxErrReset (for optional seq elements). */
      }
   }

   return (status != 0) ? LOG_RTERR (pctxt, status) : 0;
}

int xd_match (OSCTXT *pctxt, ASN1TAG tag, int *len_p, OSOCTET flags)
{
   OSSIZE len64;
   OSBOOL indefLen;
   int ret = xd_match64 (pctxt, tag, &len64, &indefLen, flags);
   if (0 == ret) {
      if (0 != len_p) {
         if (indefLen) *len_p = ASN_K_INDEFLEN;
         else if (len64 > OSINT32_MAX) {
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         }
         else *len_p = (int) len64;
      }
      return 0;
   }
   else  return ((ret == RTERR_IDNOTFOU) ? ret : LOG_RTERR (pctxt, ret));
}
