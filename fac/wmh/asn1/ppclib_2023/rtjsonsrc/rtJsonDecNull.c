/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "osrtjson.h"

int rtJsonDecNull (OSCTXT *pctxt)
{
   char nullb[5];

   int ret = rtxReadBytes (pctxt, (OSOCTET *)nullb, 4);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   nullb[4] = '\0';

   if (OSCRTLSTRNCMP ("null", nullb, sizeof(nullb)) != 0) {
      rtxErrAddStrParm (pctxt, "null");
      rtxErrAddStrParm (pctxt, nullb);
      return LOG_RTERRNEW (pctxt, RTERR_IDNOTFOU);
   }

   return 0;
}

int rtJsonTryDecNull (OSCTXT *pctxt)
{
   char nullb[5];
   OSSIZE pos;
   int ret = rtxMarkPos (pctxt, &pos);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   ret = rtxReadBytes (pctxt, (OSOCTET *)nullb, 4);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   nullb[4] = '\0';

   if (OSCRTLSTRNCMP ("null", nullb, sizeof(nullb)) != 0) {
      ret = rtxResetToPos (pctxt, pos);
      return (0 != ret) ? LOG_RTERR (pctxt, ret) : RTERR_IDNOTFOU;
   }

   return 0;
}
