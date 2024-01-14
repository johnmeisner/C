/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD int rtXmlDecHexStr (OSCTXT* pctxt,
                    OSOCTET* pvalue, OSUINT32* pnocts,
                    OSINT32 bufsize)
{
   OSSIZE numocts;
   int ret;

   ret = rtXmlDecHexStr64(pctxt, pvalue, &numocts, bufsize);
   if (ret != 0) return LOG_RTERR (pctxt, ret);
   else if ( numocts > OSUINT32_MAX ) return LOG_RTERRNEW(pctxt, RTERR_TOOBIG);

   *pnocts = (OSUINT32) numocts;
   return 0;
}


EXTXMLMETHOD int rtXmlDecHexStr64 (OSCTXT* pctxt,
                    OSOCTET* pvalue, OSSIZE* pnocts,
                    OSSIZE bufsize)
{
   int stat;
   OSSIZE nbits = 0;

   stat = rtXmlDecHexStrValue64
      (pctxt, OSRTBUFPTR (pctxt), OSRTBUFSIZE(pctxt), pvalue, &nbits, bufsize);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   *pnocts = (nbits + 7) / 8;

   return 0;
}


