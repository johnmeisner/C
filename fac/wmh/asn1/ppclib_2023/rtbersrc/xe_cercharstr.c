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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxCharStr.h"

int xe_cerCharstr(OSCTXT* pctxt, const char* pvalue,
   ASN1TagType tagging, ASN1TAG tag)
{
   int len = 0;
   OSBOOL indeflen = FALSE;

   if (0 != pvalue) {
      OSSIZE slen = OSCRTLSTRLEN(pvalue);
      char* pstr = 0;

      if (slen > 1000 && tagging == ASN1EXPL)
      {
         /* Make sure there is room for EOC.  We might not encode one because
            slen might be decremented, but either way we'll be encoding
            at least 2 bytes. */
         XE_CHKBUF(pctxt, 2);    /* may return; must be before allocation */
      }

      /* If escape null-terminator flag is set (ASN1ESCZTERM),
      check if string ends in "\\0".  If yes, remove this and
      end null-terminator byte at end of string.. */
      if (rtxCtxtTestFlag(pctxt, ASN1ESCZTERM) &&
         slen >= 2 && pvalue[slen - 2] == '\\' && pvalue[slen - 1] == '0')
      {
         pstr = (char*)rtxMemAlloc(pctxt, slen + 1);
         if (0 == pstr) return LOG_RTERR(pctxt, RTERR_NOMEM);

         rtxStrcpy(pstr, slen + 1, pvalue);
         pstr[slen - 2] = '\0';
         slen = slen - 1;
         pvalue = pstr;
      }

      if (slen > 1000 && tagging == ASN1EXPL)
      {
         /* Encoding will be fragmented and we're required to encode the
            full TLV-EOC. Since we call xe_cerOctStr with tagging == ASN1IMPL,
            it won't encode the tag or EOC for us; we have to do it. */
         indeflen = TRUE;
         XE_PUT2(pctxt, 0, 0);   /* EOC */
      }

      len = xe_cerOctstr(pctxt, (OSOCTET*)pvalue, (int)slen, ASN1IMPL);

      if ( pstr ) rtxMemFreePtr(pctxt, pstr);
   }

   if (len >= 0 && tagging == ASN1EXPL)
   {
      if (indeflen) {
         len = xe_TagAndIndefLen(pctxt, tag | TM_CONS, len);
      }
      else len = xe_tag_len(pctxt, tag, len);
   }

   return (len);
}
