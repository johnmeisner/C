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
#include "rtxsrc/rtxContext.hh"

#ifndef XDFUNC
#define XDFUNC
#endif


XDFUNC int xd_bitstr64Ext_s (OSCTXT* pctxt, OSOCTET* object_p,
                             OSSIZE* numbits_p, OSOCTET** extdata,
                             ASN1TagType tagging, OSSIZE length,
                             OSBOOL indefLen)
{
   OSSIZE bufsiz;    /* Size, in bytes, of object_p buffer, derived from
                        numbits_p. */
   int stat = 0;
   OSOCTET b = 0;

   if (0 == numbits_p)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);

   bufsiz = (*numbits_p == 0) ? 0 : (*numbits_p - 1) / 8 + 1;

   if (tagging == ASN1EXPL) {
      stat = xd_match1_64 (pctxt, ASN_ID_BITSTR, &length, &indefLen);
      if (stat < 0) {
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
      }
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSOCTET* ptr = OSRTBUFPTR(pctxt);
      OSOCTET* pExtData = 0;
      OSSIZE idx = 0;   /* byte index of next byte into the abstract ASN.1
                           value.  Since the abstract value may divided between
                           the static bit string object_p and a dynamic part,
                           extdata, this is not an index into program data. */
      OSSIZE extIndex = 0; /* byte index of next byte to set into extdata. */


      for (;;) {
         OSSIZE len;    /* content octets in segment; */
         OSBOOL indefLen2; /* will always be FALSE */

         /* Check for end of fragments. */
         if (indefLen) {
            if (XD_CHKEOB (pctxt)) break;
         }
         else if (((OSSIZE)(OSRTBUFPTR(pctxt) - ptr) >= length) ||
                  (pctxt->buffer.byteIndex >= pctxt->buffer.size)) break;

         if (b != 0) {
            /* The previous segment must have set b, but only the last
               segment should set b to non-zero. */
            return LOG_RTERR(pctxt, RTERR_BADVALUE);
         }
         /* Decode tag for next fragment. */
         stat = xd_match1_64 (pctxt, ASN_ID_BITSTR, &len, &indefLen2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* Confirm tag was primitive. We're not supporting nested
            fragmentation (constructed tags). Note this also guarantees that
            the length is definite. */
         if (pctxt->flags & ASN1CONSTAG) {
            rtxErrAddStrParm(pctxt,
               "Fragmentation using nested constructed tags.");
            return LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
         }

         /* Decode the unused bits octet. */
         if ((stat = XD_MEMCPY1(pctxt, &b)) != 0)
            return LOG_RTERR(pctxt, stat);
         len--;

         if (idx + len <= bufsiz) {
            /* There's room in the string for this segment's data. */
            if ((pctxt->buffer.byteIndex + len) <= pctxt->buffer.size) {
               /* The input buffer has the expected number of bytes. */
               OSCRTLMEMCPY (object_p + idx, OSRTBUFPTR(pctxt), len);
               pctxt->buffer.byteIndex += len;
            }
            else stat = RTERR_ENDOFBUF;
         }
         else if (0 != extdata) {
            /* The static buffer doesn't have room for all of this segment's
               data, but we have an array to receive extra data.
               extDataBufSize = new required size for extdata.
             */
            OSSIZE extDataBufSize = (idx + len) - bufsiz;
            OSSIZE extlen;

            if (idx < bufsiz) {
               /* Static buffer is not full yet. Fill it up; the rest will go
                  into extdata. */
               OSCRTLMEMCPY (object_p + idx, OSRTBUFPTR(pctxt), bufsiz - idx);
               extlen = len - (bufsiz - idx);
            }
            else {
               /* Static buffer is full.  All bytes from this segment go into
                  extdata. */
               extlen = len;
            }

            pExtData = (OSOCTET*)
               rtxMemRealloc (pctxt, pExtData, extDataBufSize);
            if (0 == pExtData) return LOG_RTERR (pctxt, RTERR_NOMEM);

            OSCRTLMEMCPY (pExtData + extIndex, OSRTBUFPTR(pctxt), extlen);
            pctxt->buffer.byteIndex += len;
            extIndex += extlen;
         }
         else {
            stat = RTERR_STROVFLW;
         }

         if (stat != 0) return LOG_RTERR (pctxt, stat);

         idx += len;
      }

      if (b <= 7) { /* initial octet should be 0..7 */
         *numbits_p = (idx * 8) - b;
         if ( extdata != NULL ) *extdata = pExtData;

         /* If not indefinite length, make sure we consumed exactly as
            many bytes as specified in the constructed length field.  We might
            have broken the loop above because we overstepped the enclosing
            length boundary; now we'll return an error for that. */
         if (!indefLen) {
            OSSIZE usedBytes = OSRTBUFPTR(pctxt) - ptr;
            if (usedBytes != length)
               return LOG_RTERR (pctxt, ASN_E_INVLEN);
         }
      }
      else
         return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }
   else if (length > 0) { /* primitive case */
      if ((stat = XD_MEMCPY1 (pctxt, &b)) == 0) {
         length--;  /* adjust by 1; first byte is unused bit count */
         if (length == 0) {
            if (0 == b)
               *numbits_p = 0;
            else
               stat = ASN_E_INVLEN;
         }

         if (stat == 0) {
            if (b <= 7) { /* initial octet should be 0..7 */
               if ((pctxt->buffer.byteIndex + length) > pctxt->buffer.size)
                  return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

               if (length <= bufsiz) {
                  OSCRTLMEMCPY (object_p, OSRTBUFPTR(pctxt), length);
                  pctxt->buffer.byteIndex += length;
               }
               else if (0 != extdata) {
                  OSSIZE extlen = length - bufsiz;
                  OSOCTET* pExtData = (OSOCTET*)rtxMemAlloc (pctxt, extlen);
                  if (0 == pExtData) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  /* Copy static and extended parts */
                  OSCRTLMEMCPY (object_p, OSRTBUFPTR(pctxt), bufsiz);
                  pctxt->buffer.byteIndex += bufsiz;
                  OSCRTLMEMCPY (pExtData, OSRTBUFPTR(pctxt), extlen);
                  pctxt->buffer.byteIndex += extlen;

                  *extdata = pExtData;
               }
               else stat = RTERR_STROVFLW;

               if (0 == stat) {
                  if (length < OSSIZE_MAX/8) {
                     *numbits_p = (length * 8) - b;
                  }
                  else return LOG_RTERR (pctxt, RTERR_TOOBIG);
               }
            }
         }
      }

      if (stat != 0)
      {
         return LOG_RTERR (pctxt, stat);
      }
   }
   else if (length == 0)
      *numbits_p = 0;
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   LCHECKBER (pctxt);

   return 0;
}

/***********************************************************************
 *
 *  Routine name: xd_bitstr64_s  (decode static bit string)
 *
 *  Description:  This routine decodes the bit string at the current
 *                message pointer location and returns its value.  The
 *                value is returned in the "ASNBITSTR" structure.  This
 *                structure contains a value for the number of bits and
 *                a pointer to the decoded bit string.  The routine
 *                also advances the message pointer to the start of the
 *                the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, the first byte of the data value is fetched.
 *                This byte contains a value indicating the number of bits
 *                in the last octet that are unused.  Dynamic memory is
 *                then allocated for the bit string and xd_memcpy
 *                is called to copy the string from the message to the
 *                allocated memory.  The number of bits is calculated by
 *                multiplying the number of octets times eight bits/octet
 *                and then subtracting the number of unused bits obtained
 *                above.  The number of bits and dynamic memory pointer
 *                are passed back to the caller via the "ASNBITSTR"
 *                structure.  The actual length of the string in bytes is
 *                returned as the function result.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt       struct  Pointer to ASN.1 context block structure
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of operation.  Returned as function result.
 *  object      char*   Decoded bit string value.
 *  numbits     int     Number of bits in string.
 *
 **********************************************************************/

XDFUNC int xd_bitstr_s
(OSCTXT* pctxt, OSOCTET* object_p, OSUINT32* numbits_p,
 ASN1TagType tagging, int length)
{
   OSSIZE numbits = *numbits_p;
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   OSSIZE len2 = (length > 0) ? (OSSIZE)length : 0;

   int ret = xd_bitstr64Ext_s
      (pctxt, object_p, &numbits, 0, tagging, len2, indefLen);

   if (0 == ret) {
      if (sizeof(numbits) > 4 && numbits > OSUINT32_MAX)
         return (LOG_RTERR (pctxt, RTERR_TOOBIG));

      *numbits_p = (OSUINT32)numbits;
   }

   return ret;
}

#ifndef _BUILD_LICOBJ
XDFUNC int xd_bitstr64_s
(OSCTXT* pctxt, OSOCTET* object_p, OSSIZE* numbits_p,
 ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
   return xd_bitstr64Ext_s
      (pctxt, object_p, numbits_p, 0, tagging, length, indefLen);
}

XDFUNC int xd_bitstrExt_s(OSCTXT* pctxt, OSOCTET* object_p,
                          OSUINT32* numbits_p, OSOCTET** extdata,
                          ASN1TagType tagging, int length)
{
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   OSSIZE len2 = (length > 0) ? (OSSIZE)length : 0;
   OSSIZE numbits = *numbits_p;
   int ret;

   ret = xd_bitstr64Ext_s
      (pctxt, object_p, &numbits, extdata, tagging, len2, indefLen);

   if (0 == ret) {
      if (sizeof(numbits) > 4 && numbits > OSUINT32_MAX)
         return (LOG_RTERR (pctxt, RTERR_TOOBIG));

      *numbits_p = (OSUINT32)numbits;
   }

   return ret;
}
#endif
