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

#ifndef _BERMACROS_H_
#define _BERMACROS_H_

#define ASN_K_INDEFLEN   -9999   /* indefinite length message indicator  */

#define XU_DUMP(msg)                xu_dump(msg,0,0)
#define xd_resetp(pctxt)            rtxResetContext(pctxt)

#define ASN1TAG2BYTE(tag) \
((OSOCTET)(((tag)&TM_B_IDCODE)|((tag)>>ASN1TAG_LSHIFT)))

#define _GENASN1TAG(t) \
(((ASN1TAG)((t) & TM_CLASS_FORM) << ASN1TAG_LSHIFT) | ((t) & TM_B_IDCODE))

/* This macro will fetch the next byte in the input stream.  A test is  */
/* done for buffer overrun.                                             */

#define XD_MEMCPY1(pctxt, object_p) \
((ASN1BUF_INDEFLEN(pctxt) || \
((pctxt)->buffer.byteIndex < (pctxt)->buffer.size)) ? \
(*object_p = *OSRTBUFPTR(pctxt), (pctxt)->buffer.byteIndex ++, 0) : \
RTERR_ENDOFBUF)

/* This macro will fetch the next byte in the input stream.  No test is */
/* done for buffer overrun..                                            */

#define XD_FETCH1(pctxt) ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex++])

/* This macro will match a single-byte tag. It does not advance the     */
/* decode cursor.                                                       */

#define XD_PEEKTAG(pctxt, tag) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] & (~0x20)) == (tag & (~0x20)))

/* This macro returns true if the next tag is constructed.              */
/* It does not advance the decode cursor.                               */

#define XD_PEEKPC(pctxt) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] & 0x20) == 0x20)

/* This macro will match a single-byte tag (assumes it is primitive only)
   and advances the decode cursor if it matches. It also clears the
   ASN1CONSTAG flag bit in the context. */

#define XD_MATCH1(pctxt, tag) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == tag) ? \
((pctxt)->flags &= (~ASN1CONSTAG), (pctxt)->buffer.byteIndex++, TRUE) : FALSE)

/* This macro will test for an EOC (end-of-context) marker */

#define XD_MATCHEOC(pctxt) \
( ( (pctxt)->buffer.byteIndex + 2 <= (pctxt)->buffer.size ) && \
( (pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == 0 ) && \
( (pctxt)->buffer.data[(pctxt)->buffer.byteIndex + 1] == 0 ) )

/* These macros will test a series of bytes in the decode buffer for a  */
/* match.                                                               */
#define XD_MATCHBYTES1(pctxt, b1) \
((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == b1)

#define XD_MATCHBYTES2(pctxt, b1, b2) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == b1) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+1] == b2))

#define XD_MATCHBYTES3(pctxt, b1, b2, b3) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == b1) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+1] == b2) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+2] == b3))

#define XD_MATCHBYTES4(pctxt, b1, b2, b3, b4) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == b1) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+1] == b2) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+2] == b3) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+3] == b4))

#define XD_MATCHBYTES5(pctxt, b1, b2, b3, b4, b5) \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == b1) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+1] == b2) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+2] == b3) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+3] == b4) && \
 ((pctxt)->buffer.data[(pctxt)->buffer.byteIndex+4] == b5))

/* This macro will advance the decode buffer index by the given number  */
/* number of bytes.                                                     */

#define XD_BUMPIDX(pctxt, nbytes) ((pctxt)->buffer.byteIndex += nbytes)

/* This macro will test for buffer overrun */

#define XD_CHKBUFEND(pctxt) \
(((ASN1BUF_INDEFLEN(pctxt)) || \
((pctxt)->buffer.byteIndex <= (pctxt)->buffer.size)) ? 0 : RTERR_ENDOFBUF)

/* This macro will test if the given length value is within the current */
/* bounds of the message buffer.  The given length value is assumed to  */
/* be a definite length (i.e != ASN_K_INDEFLEN)..                       */

#define XD_CHKDEFLEN(pctxt,len) \
(((ASN1BUF_INDEFLEN(pctxt)) || \
(((pctxt)->buffer.byteIndex + len) <= (pctxt)->buffer.size)) ? \
0 : RTERR_ENDOFBUF)

/* This macro will test for EOB */

#define XD_CHKEOB(pctxt) \
(((pctxt)->buffer.byteIndex + 2 > (pctxt)->buffer.size) ? TRUE : \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] == 0 && \
(pctxt)->buffer.data[(pctxt)->buffer.byteIndex + 1] == 0 ) ? \
TRUE : FALSE))

/* This macro will test for CCB overrun */
/* see xd_octstr_s for sample of code to be used in 64-bit case */

#define XD_CHKEND(pctxt,ccb_p) \
(((ccb_p)->len == ASN_K_INDEFLEN) ? XD_CHKEOB(pctxt) : \
((OSRTBUFPTR(pctxt) - (ccb_p)->ptr >= (ccb_p)->len) || \
((pctxt)->buffer.byteIndex >= (pctxt)->buffer.size)))

/* This macro will check if the given amount of space is available      */
/* in the encode buffer and attempt tp expand the buffer if necessary   */

#define XE_CHKBUF(pctxt,len) \
if (len > (pctxt)->buffer.byteIndex) { \
int xstat = xe_expandBuffer (pctxt, len); \
if (xstat != 0) return xstat; }

/* This macro will put one byte into the encode buffer.  It is assumed  */
/* that there is enough space available for the byte.                   */

#define XE_PUT1(pctxt,ch) \
(pctxt)->buffer.data[--(pctxt)->buffer.byteIndex] = ch;

/* This macro will put two bytes into the encode buffer.  It is assumed */
/* that there is enough space available for the bytes.                  */

#define XE_PUT2(pctxt,ch1,ch2) \
(pctxt)->buffer.byteIndex -= 2; \
(pctxt)->buffer.data[(pctxt)->buffer.byteIndex] = ch1; \
(pctxt)->buffer.data[(pctxt)->buffer.byteIndex+1] = ch2;

/*
 * This macro will put one byte into the encode buffer.  It will first
 * check to see if enough space is available for the byte and attempt
 * to expand the buffer if necessary.
*/
#define XE_SAFEPUT1(pctxt,ch) \
XE_CHKBUF(pctxt,1); (pctxt)->buffer.data[--(pctxt)->buffer.byteIndex] = ch;

/* This macro is optimized version of xd_len function */

#define XD_LEN(pctxt,len_p) \
(((pctxt)->buffer.byteIndex < (pctxt)->buffer.size) ? \
(((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] < 0x80) ? \
(*(len_p)=(pctxt)->buffer.data[(pctxt)->buffer.byteIndex++], 0) : \
 xd_len ((pctxt),len_p)) : RTERR_ENDOFBUF)

/* This macro is an optimized version of the xd_tag function */

#define XD_TAG(pctxt,tag_p) \
(((pctxt)->buffer.byteIndex < (pctxt)->buffer.size) ? \
((((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] & TM_B_IDCODE) != 31) ? \
(*(tag_p) = (((ASN1TAG)((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] & \
TM_CLASS_FORM) << ASN1TAG_LSHIFT) | \
((pctxt)->buffer.data[(pctxt)->buffer.byteIndex] & TM_B_IDCODE)), \
SET_ASN1CONSTAG((pctxt),*(tag_p)), (pctxt)->buffer.byteIndex++, 0) : \
xd_tag((pctxt), (tag_p))) : RTERR_ENDOFBUF)

/* These macros set the constructed tag bit in the context flags based on
   the parsed value */

#define SET_ASN1CONSTAG(pctxt,tag) \
((pctxt)->flags = (OSRTFLAGS)(((pctxt)->flags & (~ASN1CONSTAG)) \
| ((!(((tag) & TM_CONS) >> (ASN1TAG_LSHIFT - 8 + 5)) - 1) & ASN1CONSTAG)))

#define SET_ASN1CONSTAG_BYTE(pctxt,tag) \
((pctxt)->flags = (OSRTFLAGS)(((pctxt)->flags & (~ASN1CONSTAG)) | \
((!(((tag) & TM_FORM) >> 5) - 1) & ASN1CONSTAG)))

#endif

