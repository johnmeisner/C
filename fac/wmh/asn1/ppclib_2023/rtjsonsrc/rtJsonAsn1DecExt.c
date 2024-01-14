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

#include "asn1json.h"
#include "rtxsrc/rtxCtype.h"

int rtJsonAsn1IsContentObject(OSCTXT* pctxt)
{
   OSSIZE savedPos;
   int fn_retval = 0;
   int retval = 0;
   OSUTF8CHAR* ident;

   if ( rtJsonDecPeekChar2(pctxt) != '{' ) return 0;

   retval = rtxMarkPos(pctxt, &savedPos);
   if ( retval < 0 ) return LOG_RTERR(pctxt, retval);

   retval = rtJsonDecMatchChar(pctxt, '{');
   if (retval != 0) {
      return LOG_RTERR(pctxt, retval);
   }

   retval = rtJsonDecStringValue(pctxt, &ident);
   if (retval != 0) return LOG_RTERR(pctxt, retval);

   fn_retval = !rtxUTF8Strcmp(ident, (OSUTF8CHAR *)"containing");

   rtxMemFreePtr(pctxt, ident);
   retval = rtxResetToPos(pctxt, savedPos);
   if ( retval != 0 ) return LOG_RTERR(pctxt, retval);

   return fn_retval;
}
