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

#include "rtbersrc/asn1berStream.h"

int berDecStrmNextElement (OSCTXT *pctxt)
{
#ifndef _NO_STREAM
   ASN1TAG tag;
   int ilcnt = 0, len, stat;

   do {
      stat = berDecStrmTagAndLen (pctxt, &tag, &len);
      if (stat != 0) break;

      if (len > 0) {
         stat = rtxStreamSkip (pctxt, len);
         if (stat != 0) break;
      }
      else if (len == ASN_K_INDEFLEN) ilcnt++;
      else if (tag == 0 && len == 0) ilcnt--;
   } while (ilcnt > 0);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
#endif
   return 0;
}

