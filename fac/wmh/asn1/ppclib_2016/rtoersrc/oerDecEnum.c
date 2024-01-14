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

int oerDecUnrestInt32Value (OSCTXT* pctxt, OSINT32* pvalue, size_t len);
int oerDecUnrestUInt32Value (OSCTXT* pctxt, OSUINT32* pvalue, size_t len);

/* Decode a signed OER enumerated value */

EXTOERMETHOD int oerDecSignedEnum (OSCTXT* pctxt, OSINT32* pvalue)
{
   OSOCTET b;
   int retval = 0;
   int stat = rtxReadBytes (pctxt, &b, 1);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (b > 0x80) {
      OSOCTET nbytes = (OSOCTET)(b & 0x7F);

      /* Decode value */
      stat = oerDecUnrestInt32Value (pctxt, pvalue, nbytes);
      if (stat == ASN_E_NOTCANON)
         retval = stat;
      else
         if (0 != stat) return LOG_RTERR (pctxt, stat);

      if ( *pvalue >= 0 && *pvalue < 128 )
      {
         /* short form should have been used */
         retval = ASN_E_NOTCANON;
      }
   }
   else if (b < 0x80) {
      if (0 != pvalue) *pvalue = b;
   }
   else { /* b == 0x80 */
      /* Assume value of 0 */
      if (0 != pvalue) *pvalue = 0;
   }

   return retval;
}

/* Decode an unsigned OER enumerated value */

EXTOERMETHOD int oerDecUnsignedEnum (OSCTXT* pctxt, OSUINT32* pvalue)
{
   OSINT32 value;
   int retval = 0;
   int stat = oerDecSignedEnum(pctxt, &value);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   if ( value < 0 )
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   else if ( pvalue != 0 ) *pvalue = (OSUINT32) value;

   return retval;
}
