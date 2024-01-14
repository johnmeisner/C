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
#include "rtxsrc/rtxContext.hh"

#ifndef XDFUNC
#define XDFUNC
#endif

/***********************************************************************
 *
 *  Routine name: xd_bitstr64Ext_s  (decode static bit string)
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
 *                structure.
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

XDFUNC int xd_bitstr64Ext_s (OSCTXT* pctxt, OSOCTET* object_p,
                             OSSIZE* numbits_p, OSOCTET** extdata,
                             ASN1TagType tagging, OSSIZE length,
                             OSBOOL indefLen)
{
   OSSIZE bufsiz;
   int stat = 0;
   OSOCTET b = 8;

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
      OSSIZE idx = 0, oidx = OSSIZE_MAX, extIndex = 0, len;
      OSBOOL indefLen2;

      for (;;) {
         /* replaces XD_CHKEND */
         if (indefLen) {
            if (XD_CHKEOB (pctxt)) break;
         }
         else if (((OSSIZE)(OSRTBUFPTR(pctxt) - ptr) >= length) ||
                  (pctxt->buffer.byteIndex >= pctxt->buffer.size)) break;

         stat = xd_match1_64 (pctxt, ASN_ID_BITSTR, &len, &indefLen2);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         if (NULL == extdata)
		 {
			 /* Only check for last segment/unused bit count here if there
			    is no extended data present.  Otherwise, the calculation
				will not be correct because we'll be looking at the wrong
				byte (i.e. within the extended data)... */
            if ((idx + len) == bufsiz + 1) { /* last segment detection */
               /* read unused bit count byte .. */
               if ((stat = XD_MEMCPY1 (pctxt, &b)) != 0)
                  return LOG_RTERR (pctxt, stat);
               len --;
               oidx = OSSIZE_MAX;
            }
            else
               oidx = idx;
		 }

         if (idx + len <= bufsiz) {
            if ((pctxt->buffer.byteIndex + len) <= pctxt->buffer.size) {
               OSCRTLMEMCPY (object_p + idx, OSRTBUFPTR(pctxt), len);
               pctxt->buffer.byteIndex += len;
            }
            else stat = RTERR_ENDOFBUF;
         }
         else if (0 != extdata) {
            OSSIZE extDataBufSize = (idx + len) - bufsiz;
            OSSIZE extlen;

            if (idx == extDataBufSize) { /* last segment detection */
               /* read unused bit count byte .. */
               if ((stat = XD_MEMCPY1 (pctxt, &b)) != 0)
                  return LOG_RTERR (pctxt, stat);
               len --;
               oidx = OSSIZE_MAX;
            }
            else
               oidx = idx;

            if (idx < bufsiz) {
               /* fill remainder of static buffer */
               OSCRTLMEMCPY (object_p + idx, OSRTBUFPTR(pctxt), bufsiz - idx);
               idx = bufsiz;
               extlen = len - bufsiz;
            }
            else extlen = len;

            pExtData = rtxMemRealloc (pctxt, pExtData, extDataBufSize);
            if (0 == pExtData) return LOG_RTERR (pctxt, RTERR_NOMEM);

            OSCRTLMEMCPY (pExtData + extIndex, OSRTBUFPTR(pctxt), extlen);
            pctxt->buffer.byteIndex += extlen;
            extIndex += extlen;
         }
         else {
            stat = RTERR_STROVFLW;
         }

         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else idx += len;
      }
      if (idx > oidx) {
         /* if unused bit count wasn't fetched... */
         b = object_p [oidx];
         OSCRTLMEMCPY (object_p + oidx, object_p + oidx + 1, idx - oidx - 1);
         idx --;
      }
      if (b <= 7) { /* initial octet should be 0..7 */
         *numbits_p = (idx * 8) - b;
         *extdata = pExtData;

         /* If not indefinite length, make sure we consumed exactly as  */
         /* many bytes as specified in the constructed length field..   */

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
                  OSOCTET* pExtData = rtxMemAlloc (pctxt, extlen);
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

XDFUNC int xd_bitstr64_s
(OSCTXT* pctxt, OSOCTET* object_p, OSSIZE* numbits_p,
 ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
   return xd_bitstr64Ext_s
      (pctxt, object_p, numbits_p, 0, tagging, length, indefLen);
}

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

