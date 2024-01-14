/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

/* Decode a signed number into a 32-bit signed integer, allowing that if
 * pos is set true on return, that the value should be reinterpreted as an
 * unsigned number (i.e. the MSB is 1 but this represents a positive value).
 *
 * @param pos Flag indicating when returned value is negative but should be
 *       intepreted as positive.
 * @param len Number of octets to decode.  Must be greater than zero.
 */
int oerDecUnrestInt32ValuePlusSign (OSCTXT* pctxt, OSINT32* pvalue, OSBOOL* pos,
                                    size_t len)
{
   int      stat, retval = 0;
   OSOCTET  tmp;
   OSINT32  value;

   *pos = FALSE;

   /* read the first byte and assign it to value. Cast it to signed so that
      if the encoded value is negative, the sign bit will be extended to all
      the higher bits.  This is important if len < 4.
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

      if ( (value & 0xFF000000) == 0xFF000000 )
      {
         /* the high byte is 0xFF, meaning we can shift in another
            byte, but if the sign changes, then we had overflow */
         OSUINT32 highbit = value & 0x80000000;
         value = (value * 256) + tmp;
         if ( (value & 0x80000000) != highbit ) {
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
         }
      }
      else if ( (value & 0xFF000000) == 0 )
      {
         /* The high byte is 0x00, meaning we can shift in another
            byte, but if the sign changes, then we had overflow.
            Overflow is allowed in one case: when this is the last byte
            to be read - in this case, pos is set to indicate the value
            should be recast as unsigned.
         */
         OSUINT32 highbit = value & 0x80000000;
         value = (value * 256) + tmp;
         if ( (value & 0x80000000) != highbit ) {
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

int oerDecUnrestInt32Value (OSCTXT* pctxt, OSINT32* pvalue, size_t len)
{
   int      stat, retval = 0;
   OSBOOL   pos;

   stat = oerDecUnrestInt32ValuePlusSign(pctxt, pvalue, &pos, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( pos )
   {
      /* Encoded value was positive and out of range for signed integer. */
      return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
   }

   return retval;
}

EXTOERMETHOD int oerDecUnrestInt32 (OSCTXT* pctxt, OSINT32* pvalue)
{
   int      stat, retval = 0;
   OSSIZE len;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Decode value */

   stat = oerDecUnrestInt32Value (pctxt, pvalue, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   return retval;
}

EXTOERMETHOD int oerDecUnrestSignedUInt32 (OSCTXT* pctxt, OSUINT32* pvalue)
{
   int      stat, retval = 0;
   OSSIZE  len;
   OSINT32 value;
   OSBOOL pos;

   stat = oerDecLen(pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Decode value */
   stat = oerDecUnrestInt32ValuePlusSign (pctxt, &value, &pos, len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( value >= 0 || pos ) {
      /* cast to unsigned is correct thing to do */
      *pvalue = (OSUINT32) value;
      return retval;
   }
   else {
      /* encoded value was negative; doesn't fit int unsigned value */
      return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);
   }
}
