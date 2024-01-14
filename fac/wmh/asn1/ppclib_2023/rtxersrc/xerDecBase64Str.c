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

#include "xed_common.hh"

int xerDecBase64Str (OSCTXT* pctxt,
                     OSOCTET* pvalue, OSUINT32* pnocts,
                     OSINT32 bufsize)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int stat;
   int encLen = xerTextLength (inpdata);

   stat = xerDecBase64StrValue (pctxt, pvalue, pnocts, bufsize, encLen);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKXER (pctxt);

   return (0);
}

int xerDecBase64Str64 (OSCTXT* pctxt,
                       OSOCTET* pvalue, OSSIZE* pnocts,
                       OSSIZE bufsize)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int stat;
   OSSIZE encLen = xerTextLength (inpdata);

   stat = xerDecBase64StrValue64 (pctxt, pvalue, pnocts, bufsize, encLen);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   LCHECKXER (pctxt);

   return (0);
}

