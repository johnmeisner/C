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

#include "xee_common.hh"

int xerEncObjId64 (OSCTXT* pctxt, const ASN1OID64 *pvalue,
                   const char* elemName)
{
   char lbuf[40], *p;
   int  stat;
   OSUINT32 i;

   /* Validate given object ID by applying ASN.1 rules */

   if (0 == pvalue) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->numids < 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] > 2) return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   if (pvalue->subid[0] != 2 && pvalue->subid[1] > 39)
      return LOG_RTERR (pctxt, ASN_E_INVOBJID);

   if (0 == elemName) elemName = "OBJECT_IDENTIFIER";

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;
   for (i = 0; i < pvalue->numids; i++) {
      p = (char*) lbuf;
      if (i != 0)
         *p++ = '.';
      sprintf (p, OSUINT64FMT, pvalue->subid[i]);

      stat = xerCopyText (pctxt, lbuf);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return (0);
}
