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

#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxBitDecode.h"

int rtDecOIDSubIds
(OSCTXT* pctxt, OSUINT32 subidArray[], OSSIZE subidArraySize, int enclen)
{
   int stat = 0, j = 0;
   while (enclen > 0)
   {
      if ((OSSIZE)j < subidArraySize) {
         OSOCTET ub;

         /* Parse a subidentifier out of the contents field */

         subidArray[j] = 0;
         do {
            stat = rtxDecBitsToByte(pctxt, &ub, 8);
            if (0 != stat) return LOG_RTERR(pctxt, stat);

            subidArray[j] = (subidArray[j] * 128) + (ub & 0x7F);
            enclen--;
         } while (ub & 0x80 && enclen > 0);

         /* If extension bit is set in last parsed octet, it is an error */

         if ((ub & 0x80) != 0) return LOG_RTERR (pctxt, ASN_E_INVOBJID);

         /* Handle the first subidentifier special case: the first two 	*/
         /* sub-id's are encoded into one using the formula (x * 40) + y */

         if (j == 0)
         {
            OSUINT32 subid = subidArray[0];
            subidArray[0] = ((subid / 40) >= 2) ? 2 : subid / 40;
            subidArray[1] = (subidArray[0] == 2) ?
               subid - 80 : subid % 40;
            j = 2;
         }
         else j++;
      }
      else
         stat = ASN_E_INVOBJID;
   }

   if (enclen != 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   /* If successful, return number of decoded subidentifiers */

   return (stat >= 0) ? j : stat;
}
