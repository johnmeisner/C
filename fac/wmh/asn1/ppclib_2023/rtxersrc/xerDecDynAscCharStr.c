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

#include "asn1xer.h"

int xerDecDynAscCharStr (OSCTXT* pctxt, const char** outdata)
{
   OSUTF8CHAR* inpdata = (OSUTF8CHAR*) ASN1BUFPTR (pctxt);
   char* pbuf = 0;
   OSSIZE i, nchars = 0;
   int ch;

   nchars = rtxUTF8Len (inpdata);

   if (nchars > 0) {
      /* Allocate memory for output data */

      pbuf = (char*)rtxMemAlloc (pctxt, nchars + 1);
      if (pbuf != 0) {
         /* Convert Unicode characters to ASCII */

         for (i = 0; i < nchars; i++) {
            int sz = INT_MAX;

            ch = rtxUTF8DecodeChar (pctxt, inpdata + i, &sz);

            if (ch < 0)
               return LOG_RTERR_AND_FREE_MEM (pctxt, ch, pbuf);

            if (sz != 1) return LOG_RTERR (pctxt, RTERR_INVCHAR);

            pbuf[i] = (char) ch;
         }
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);

      pbuf[i] = '\0';
   }

   *outdata = pbuf;

   return (0);
}

