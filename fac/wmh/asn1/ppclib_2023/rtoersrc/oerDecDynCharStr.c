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
#include "rtxsrc/rtxBuffer.h"

/* Decode an OER character string into a dynamic buffer */

EXTOERMETHOD int oerDecDynCharStr (OSCTXT *pctxt, char** ppvalue)
{
   size_t len;
   int retval = 0;
   int stat = oerDecLen (pctxt, &len);
   if (stat == ASN_E_NOTCANON)
      retval = stat;
   else
      if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Decode contents */
   if (0 != ppvalue) {
      char* pstr = (char*) rtxMemAlloc (pctxt, len+1);
      if (pstr == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

      stat = rtxReadBytes (pctxt, (OSOCTET*)pstr, len);
      pstr[len] = '\0';
      *ppvalue = pstr;
   }
   else {
      stat = rtxReadBytes (pctxt, 0, len);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   return retval;
}
