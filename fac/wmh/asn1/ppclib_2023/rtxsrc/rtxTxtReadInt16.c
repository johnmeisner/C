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

#include "rtxError.h"
#include "rtxText.h"

int rtxTxtReadInt16 (OSCTXT* pctxt, OSINT16* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = rtxTxtReadInt32 (pctxt, &itmp);
   if (stat == 0) {
      if (itmp < -32768 || itmp > 32767)
         return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

      *pvalue = (OSINT16)itmp;
   }
   else return LOG_RTERR (pctxt, stat);

   return 0;
}
