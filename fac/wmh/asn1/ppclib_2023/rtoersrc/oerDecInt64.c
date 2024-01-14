/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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

#include "rtoersrc/asn1oer.h"

/* Decode a signed number into a 64-bit signed integer, allowing that if
 * pos is set true on return, that the value should be reinterpreted as an
 * unsigned number (i.e. the MSB is 1 but this represents a positive value).
 *
 * @param pos Flag indicating when returned value is negative but should be
 *       intepreted as positive.
 * @param len Number of octets to decode.  Must be greater than zero.
 */
int oerDecUnrestInt64ValuePlusSign (OSCTXT* pctxt, OSINT64* pvalue, OSBOOL* pos,
                                    size_t len)
{
   int      stat, retval = 0;
   OSOCTET  tmp;
   OSINT64  value;

   *pos = FALSE;

   /* read the first byte and assign it to value. Cast it to signed so that
      if the encoded value is negative, the sign bit will be extended to all
      the higher bits.  This is important if len < 8.
   */
   stat = rtxReadBytes(pctxt, &tmp, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   value = (signed char) tmp;
   len--;

   /* loop over remaining bytes (if any) and see if we can fit them in while
      also checking for non-canonical encoding.
   */
   while ( len > 0 ) {
      stat = rtxReadBytes(pctxt, &tmp, 1);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if ( ( value == -1 && tmp >= 0x80 ) ||
            ( value == 0 && tmp < 0x80 ) )
      {
         /* We've seen nothing but 0xFF bytes and the first non-0xFF byte has
            the high bit set OR we've seen nothing but 0x00 bytes and the first
            non-0x00 byte has the high bit clear.  This means the encoding
            used more bytes than necessary.
         */
         retval = ASN_E_NOTCANON;
      }

      if ( (value & 0xFF00000000000000) == 0xFF00000000000000 )
      {
         /* the high byte is 0xFF, meaning we can shift in another
            byte, but if the sign changes, then we had overflow */
         OSUINT64 highbit = value & 0x8000000000000000;
         value = (value * 256) + tmp;
         if ( (value & 0x8000000000000000) != highbit ) {
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
         }
      }
      else if ( (value & 0xFF00000000000000) == 0 )
      {
         /* The high byte is 0x00, meaning we can shift in another
            byte, but if the sign changes, then we had overflow.
            Overflow is allowed in one case: when this is the last byte
            to be read - in this case, pos is set to indicate the value
            should be recast as unsigned.
         */
         OSUINT32 highbit = value & 0x8000000000000000;
         value = (value * 256) + tmp;
         if ( (value & 0x8000000000000000) != highbit ) {
            if ( len == 1 ) *pos = TRUE;
            else return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
         }
      }
      else {
         /* high byte cannot be shifted off; value doesn't fit */
         return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
      }

      len--;
   }

   if (0 != pvalue) {
      *pvalue = value;
   }

   return retval;
}


int oerDecUnrestInt64 (OSCTXT* pctxt, OSINT64* pvalue)
{
   OSSIZE  len;
   OSBOOL  pos;
   int stat, retval = 0;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = oerDecUnrestInt64ValuePlusSign(pctxt, pvalue, &pos, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( pos ) {
      /* encoded value is positive and out of range of signed */
      return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
   }

   return retval;
}


/* Decode an OER 64-bit signed integer */

EXTOERMETHOD int oerDecInt64 (OSCTXT* pctxt, OSINT64* pvalue)
{
   OSOCTET tmpbuf[8];

   int stat = rtxReadBytes (pctxt, tmpbuf, 8);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      int i;
      *pvalue = (signed char) tmpbuf[0];
      for (i = 1; i < 8; i++) {
         *pvalue = (*pvalue * 256) + tmpbuf[i];
      }
   }

   return 0;
}

EXTOERMETHOD int oerDecUnrestSignedUInt64 (OSCTXT* pctxt, OSUINT64* pvalue)
{
   int      stat, retval = 0;
   OSSIZE  len;
   OSINT64 value;
   OSBOOL pos;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Decode value */
   stat = oerDecUnrestInt64ValuePlusSign (pctxt, &value, &pos, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( value >= 0 || pos ) {
      /* cast to unsigned is correct thing to do */
      *pvalue = (OSUINT64) value;
      return retval;
   }
   else {
      /* encoded value was negative; doesn't fit int unsigned value */
      return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
   }
}
