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

/***********************************************************************
 *
 *  Routine name: xe_charstr
 *
 *  Description:  The following function encodes a value of a
 *                useful character string type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to context block structure
 *  data        char*   pointer to null-term string to be encoded
 *  tagging     bool    Explicit or implicit tagging specification.
 *  tag         ASN1TAG ASN.1 tag to be applied to the encoded type.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  aal         int     Overall message length.  Returned as function
 *                      result.  Will be negative if encoding fails.
 *
 **********************************************************************/

int xe_charstr (OSCTXT* pctxt, const char* pvalue,
                ASN1TagType tagging, ASN1TAG tag)
{
   int len = 0;

   if (0 != pvalue) {
      OSSIZE slen = OSCRTLSTRLEN (pvalue);

      /* If escape null-terminator flag is set (ASN1ESCZTERM),
         check if string ends in "\\0".  If yes, remove this and
         end null-terminator byte at end of string.. */
      if (rtxCtxtTestFlag (pctxt, ASN1ESCZTERM) &&
          slen >= 2 && pvalue[slen-2] == '\\' && pvalue[slen-1] == '0') {
         char* pstr = (char*) rtxMemAlloc (pctxt, slen+1);
         if (0 == pstr) return LOG_RTERR (pctxt, RTERR_NOMEM);

         rtxStrcpy (pstr, slen+1, pvalue);
         pstr[slen-2] = '\0';

         len = xe_octstr (pctxt, (OSOCTET*)pstr, (int)(slen - 1), ASN1IMPL);

         rtxMemFreePtr (pctxt, pstr);
      }
      else {
         len = xe_octstr (pctxt, (OSOCTET*)pvalue, (int)slen, ASN1IMPL);
      }
   }
   if (len >= 0 && tagging == ASN1EXPL)
      len = xe_tag_len (pctxt, tag, len);

   return (len);
}

