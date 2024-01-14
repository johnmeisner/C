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

#include "rtbersrc/asn1berStream.h"

/***********************************************************************
 *
 *  Routine name: berDecStrmTag
 *
 *  Description:  This routine decodes an ASN tag into a standard 16 bit
 *                structure.  This structure represents a tag as follows:
 *
 *                 Bit#: 11 1 1110000000000
 *                       54 3 2109876543210
 *                      |__|_|_____________|
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
 *                statement in asn1type.h:
 *
 *                #define ASN1C_EXPANDED_TAGS
 *
 ***********************************************************************/

int berDecStrmTag (OSCTXT* pctxt, ASN1TAG *tag_p)
{
#ifndef _NO_STREAM
   ASN1TAG      class_form, id_code;
   int          lcnt = 0;
   int          b, stat;
   OSOCTET      ub;

   stat = rtxReadBytes (pctxt, &ub, 1);
   if (stat < 0) return stat;

   b = ub; *tag_p = 0;

   class_form = (ASN1TAG)(b & TM_CLASS_FORM);
   class_form <<= ASN1TAG_LSHIFT;

   if ((id_code = (b & TM_B_IDCODE)) == 31) {
      id_code = 0;
      do {
         stat = rtxReadBytes (pctxt, &ub, 1);
         if (stat < 0) return stat;

         b = ub;
         id_code = (id_code * 128) + (b & 0x7F);
         if (id_code > TM_IDCODE || lcnt++ > 8)
            return LOG_RTERR (pctxt, ASN_E_BADTAG);
      } while (b & 0x80);
   }

   *tag_p = class_form | id_code;

   /* Set constructed tag bit in context flags based on parsed value */
   SET_ASN1CONSTAG(pctxt, class_form);
#endif

   return (0);
}

/***********************************************************************
 *
 *  Routine name: berDecStrmLength
 *
 *  Description:  This routine decodes the length field component of an
 *                ASN.1 message.  It is called by the xd_tag_len routine
 *                which handles the decoding of both the ID and length
 *                fields.
 *
 **********************************************************************/

int berDecStrmLength2 (OSCTXT* pctxt, OSSIZE* pLength, OSBOOL* pIndefLen)
{
#ifndef _NO_STREAM
   OSSIZE i, j;
   int stat;
   OSOCTET b;

   if (0 == pLength || 0 == pIndefLen)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   *pIndefLen = FALSE;

   stat = rtxReadBytes (pctxt, &b, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat); else stat = b;

   if (stat > 0x80) {
      OSOCTET lbuf[16];

      *pLength = 0;
      i = stat & 0x7F;

      if (i > sizeof(OSSIZE)+1) {
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      stat = rtxReadBytes (pctxt, lbuf, i);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      for (j = 0; j < i; j++) {
         *pLength = (*pLength * 256) + lbuf[j];
      }
   }
   else if (stat == 0x80) {
      *pLength = OSSIZE_MAX;
      *pIndefLen = TRUE;
   }
   else *pLength = (OSSIZE)stat;
#endif

   return (0);
}

int berDecStrmLength (OSCTXT *pctxt, int *len_p)
{
#ifndef _NO_STREAM
   OSSIZE len;
   OSBOOL indef = FALSE;
   int ret;

   if (0 == len_p) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   ret = berDecStrmLength2 (pctxt, &len, &indef);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (indef) {
      *len_p = ASN_K_INDEFLEN;
   }
   else if (len > OSINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else *len_p = (int)len;
#endif

   return 0;
}

int berDecStrmTagAndLen2
(OSCTXT* pctxt, ASN1TAG* tag_p, OSSIZE* len_p, OSBOOL* pIndefLen)
{
#ifndef _NO_STREAM
   int stat;
   OSUINT16 mask = ASN1INDEFLEN | ASN1LASTEOC;

   /* Check for attempt to read past EOB */

   if (0 != (pctxt->flags & ASN1INDEFLEN)) {
      if ((pctxt->flags & mask) == mask)
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }

   if ((stat = berDecStrmTag (pctxt, tag_p)) == 0)
   {
      stat = berDecStrmLength2 (pctxt, len_p, pIndefLen);

      /* Indefinite length message check.. if indefinite length parsed, */
      /* bump indefinite length section counter; if EOC ID parsed,      */
      /* decrement count.  If overall message is of indefinite length   */
      /* and count goes to zero, signal end of message.                 */

      if (stat == 0)
      {
         if (*pIndefLen) {

            /* Verify form of tag is constructed.  If not, indefinite   */
            /* length is invalid..                                      */

            if (0 == (pctxt->flags & ASN1CONSTAG))
               return LOG_RTERR (pctxt, ASN_E_INVLEN);
         }
      }
      else
         return LOG_RTERR (pctxt, stat);
   }
   else
      return LOG_RTERR (pctxt, stat);
#endif

   return 0;
}

int berDecStrmTagAndLen (OSCTXT *pctxt, ASN1TAG *tag_p, int *len_p)
{
#ifndef _NO_STREAM
   OSSIZE len = 0;
   OSBOOL indef = FALSE;
   int ret;

   if (0 == len_p || 0 == tag_p) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   ret = berDecStrmTagAndLen2 (pctxt, tag_p, &len, &indef);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   if (indef) {
      *len_p = ASN_K_INDEFLEN;
   }
   else if (len > OSINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else *len_p = (int)len;
#endif

   return 0;
}

int berDecStrmMatchTag2
(OSCTXT* pctxt, ASN1TAG tag, OSSIZE *len_p, OSBOOL* pIndefLen, OSBOOL advance)
{
#ifndef _NO_STREAM
   int stat;
   ASN1TAG parsed_tag = 0;
   OSSIZE  parsed_len = 0;
   OSBOOL  indefLen = FALSE;

   if (0 == pctxt->buffer.data) {
      stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) LOG_RTERRNEW (pctxt, stat);
   }
   stat = rtxStreamMark (pctxt, 16);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = berDecStrmTagAndLen2 (pctxt, &parsed_tag, &parsed_len, &indefLen);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   parsed_tag &= ~TM_CONS;
   if ((tag & ~TM_CONS) != parsed_tag) {
      rtxStreamReset (pctxt);
#ifndef _COMPACT
      berErrAddTagParm (pctxt, tag);         /* expected tag */
      berErrAddTagParm (pctxt, parsed_tag);  /* parsed tag */
#endif
      return RTERR_IDNOTFOU;
   }
   else {
      if (len_p != 0) *len_p = parsed_len;
      if (pIndefLen != 0) *pIndefLen = indefLen;
      if (!advance) {
         stat = rtxStreamReset (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
#endif

   return 0;
}

int berDecStrmMatchTag (OSCTXT* pctxt, ASN1TAG tag, int *len_p, OSBOOL advance)
{
#ifndef _NO_STREAM
   OSSIZE len = 0;
   OSBOOL indefLen = FALSE;

   int ret = berDecStrmMatchTag2 (pctxt, tag, &len, &indefLen, advance);
   if (0 != ret) return ((ret == RTERR_IDNOTFOU) ? ret : LOG_RTERR (pctxt, ret));

   if (0 != len_p) {
      if (indefLen) {
         *len_p = ASN_K_INDEFLEN;
      }
      else if (len > OSINT32_MAX) {
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      else *len_p = (int)len;
   }
#endif

   return 0;
}

int berDecStrmFindTag2
(OSCTXT* pctxt, ASN1TAG tag, OSSIZE* len_p, OSBOOL* pIndefLen, OSBOOL advance)
{
#ifndef _NO_STREAM
   int stat = 0;
   OSSIZE pos = 0;
   ASN1TAG parsed_tag = 0;
   OSSIZE parsed_len = 0;
   OSBOOL indefLen = FALSE;

   if (0 == pctxt->buffer.data) {
      stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) LOG_RTERRNEW (pctxt, stat);
   }
   stat = rtxStreamGetPos (pctxt, &pos);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   for (;;) {
      stat = rtxStreamMark (pctxt, 16);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      stat = berDecStrmTagAndLen2 (pctxt, &parsed_tag, &parsed_len, &indefLen);
      if (stat == RTERR_ENDOFBUF || stat == RTERR_ENDOFFILE) {
         stat = rtxStreamSetPos (pctxt, pos);
         if (0 == stat) {
            rtxErrReset (pctxt);
            return RTERR_IDNOTFOU;
         }
         else return LOG_RTERR (pctxt, stat);
      }
      else if (stat != 0) return LOG_RTERR (pctxt, stat);

      if ((tag & ~TM_CONS) != (parsed_tag & ~TM_CONS)) {
         if ((parsed_tag & TM_CONS) == 0) { /* primitive */
            stat = rtxStreamSkip (pctxt, parsed_len);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
      }
      else { /* tag found */
         if (0 != len_p) *len_p = parsed_len;
         if (0 != pIndefLen) *pIndefLen = indefLen;
         if (!advance) {
            stat = rtxStreamReset (pctxt);
            if (stat != 0) return LOG_RTERR (pctxt, stat);
         }
         break;
      }
   }
#endif

   return 0;
}

int berDecStrmFindTag (OSCTXT* pctxt, ASN1TAG tag, int* len_p, OSBOOL advance)
{
#ifndef _NO_STREAM
   OSSIZE len = 0;
   OSBOOL indefLen = FALSE;

   int ret = berDecStrmFindTag2 (pctxt, tag, &len, &indefLen, advance);
   if (0 != ret) {
      return (ret != RTERR_IDNOTFOU) ? LOG_RTERR (pctxt, ret) : ret;
   }

   if (0 != len_p) {
      if (indefLen) {
         *len_p = ASN_K_INDEFLEN;
      }
      else if (len > OSINT32_MAX) {
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      else *len_p = (int)len;
   }
#endif

   return 0;
}

int berDecStrmMatchEOC (OSCTXT* pctxt)
{
#ifndef _NO_STREAM
   int stat;

   stat = rtxCheckInputBuffer (pctxt, 2);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   if (!(pctxt->buffer.data[pctxt->buffer.byteIndex] == 0 &&
         pctxt->buffer.data[pctxt->buffer.byteIndex+1] == 0)) {
      rtxErrAddStrParm (pctxt, "EOC");
      rtxErrAddStrParm (pctxt, "non-EOC");
      return LOG_RTERR (pctxt, RTERR_IDNOTFOU);
   }
   pctxt->buffer.byteIndex += 2;
   if (0 != pctxt->pStream) {
      if (!(pctxt->pStream->flags & OSRTSTRMF_BUFFERED)) {
         pctxt->pStream->bytesProcessed += 2;
      }
   }
#endif
   return 0;
}

OSBOOL berDecStrmTestEOC2 (OSCTXT* pctxt)
{
#ifndef _NO_STREAM
   int stat = rtxCheckInputBuffer (pctxt, 2);
   if (stat != 0) return FALSE;

   return (pctxt->buffer.data[pctxt->buffer.byteIndex] == 0 &&
           pctxt->buffer.data[pctxt->buffer.byteIndex+1] == 0);
#else
   return FALSE;
#endif
}

/* This exists for backward compatibility, the ccp_p argument
   is not used */

OSBOOL berDecStrmTestEOC (OSCTXT* pctxt, ASN1CCB* ccb_p)
{
   OS_UNUSED_ARG (ccb_p);
   return berDecStrmTestEOC2 (pctxt);
}

/* Check for end-of-context */

OSBOOL berDecStrmCheckEnd (OSCTXT* pctxt, ASN1CCB* pccb)
{
#ifndef _NO_STREAM
   pccb->stat = 0;
   if (pccb->len == ASN_K_INDEFLEN) {
      return berDecStrmTestEOC (pctxt, pccb);
   }
   else {
      size_t curOffset = OSRTSTREAM_BYTEINDEX (pctxt);
      return ((curOffset - pccb->bytes) >= (size_t)pccb->len);
   }
#else
   return FALSE;
#endif
}
