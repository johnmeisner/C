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
#include "rtxsrc/rtxBuffer.h"

/* Encode an OER signed unrestricted integer.
 * @param buffer buffer to fill with encoding.  The right-most bytes are
 *          populated with the encoded value.  The first byte cannot be
 *          encoded into; at least one byte must be kept for the caller to
 *          use for the length.
 */
int oerEncUnrestInt32Value
(OSINT32 value, OSOCTET* buffer, size_t bufsiz, size_t* pidx)
{
   /* Encode from the lowest to the highest byte, until any bytes left to
      encode are merely sign bytes.
   */

   OSINT32 temp = value;
   size_t  i = bufsiz;
   int     len = 0;
   OSOCTET lb;

   do {
      lb = (OSOCTET) (temp % 256);

      /* Right shifting a negative value has implementation-defined behavior.
         Instead, divide by 256 (truncates toward zero) and subtact one if
         the value is negative and not divisible by 256 to adjust for the
         truncation toward zero.
      */
      temp /= 256;
      if (temp < 0 && lb != 0) temp--;
      buffer[--i] = lb;
      len++;
   } while (temp != 0 && temp != -1 && i > 0);

   if (i == 0) return RTERR_BUFOVFLW;

   /* If the value is positive and bit 8 of the leading byte is set,    */
   /* copy a zero byte to the contents to signal a positive number..    */

   if (value > 0 && (lb & 0x80)) {
      buffer[--i] = 0;
      len++;
   }

   /* If the value is negative and bit 8 of the leading byte is clear,  */
   /* copy a -1 byte (0xFF) to the contents to signal a negative        */
   /* number..                                                          */

   else if (value < 0 && ((lb & 0x80) == 0)) {
      buffer[--i] = 0xFF;
      len++;
   }

   *pidx = i;

   return len;
}

EXTOERMETHOD int oerEncUnrestInt32 (OSCTXT* pctxt, OSINT32 value)
{
   OSOCTET tmpbuf[sizeof(OSINT32) + 4];
   size_t  i;
   int     len, stat;

   len = oerEncUnrestInt32Value (value, tmpbuf, sizeof(tmpbuf), &i);
   if (len < 0) return LOG_RTERR (pctxt, len);

   if (i == 0) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
   else tmpbuf[--i] = (OSOCTET) len;

   stat = rtxWriteBytes (pctxt, (OSOCTET*)&tmpbuf[i], len + 1);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
