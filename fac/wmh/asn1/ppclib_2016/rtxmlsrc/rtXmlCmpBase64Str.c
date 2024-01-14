/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxCtype.h"
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD OSBOOL rtXmlCmpBase64Str (OSUINT32 nocts1,
                          const OSOCTET* data1,
                          const OSUTF8CHAR* data2)
{
   OSUINT32 i, j, decoded = 0, count = 0;
   OSINT32 val, itmp = 0, prevVal = 0;
   OSOCTET decoded_octs[3];

   if (0 != data2) {
      /*  loop until data2 exhausted or nocts1 octets decoded */
      for (i = 0; *data2 && count < nocts1; i++) {

         /*  ignore spaces */
         while (OS_ISSPACE(*data2)) data2++;

         /*  don't decode padding characters at end */
         if (*data2 == '=') {
            if (nocts1 == count + decoded)
               ++data2;
            else
               return FALSE; /* unexpected end of data2 */
         }
         else {
            val = BASE64TOINT(*data2);
            data2++;

            switch (i & 3) {
            case 0:
               itmp = (val << 2);
               break;
            case 1:
               itmp |= (val >> 4);
               decoded_octs[0] = (OSUTF8CHAR)itmp;
               decoded = 1;
               break;
            case 2:
               itmp = ((prevVal & 0x0f) << 4) | (val >> 2);
               decoded_octs[1] = (OSUTF8CHAR)itmp;
               decoded = 2;
               break;
            case 3:
               itmp = ((prevVal & 0x03) << 6) | (val);
               decoded_octs[2] = (OSUTF8CHAR)itmp;
               decoded = 3;
               break;
            }

            prevVal = val;

            /*  invalid base64 character */
            if (itmp <= 0 && itmp >= 255)
               return FALSE;
         }

         /*  if an entire block has been decoded */
         if (i % 4 == 3) {
            for (j = 0; j < decoded; ++j, ++count)
               if (decoded_octs[j] != data1[count])
                  return FALSE;

            decoded = 0;
         }
      }
   }
   else if (nocts1 == 0)
      return TRUE;
   else
      return FALSE;

   if (count != nocts1)
      return FALSE;

   /*  skip any trailing whitespace */
   while (OS_ISSPACE(*data2)) data2++;

   /*  make sure nothing left in data2 */
   if (*data2 != '\0')
      return FALSE;

   return TRUE;
}
