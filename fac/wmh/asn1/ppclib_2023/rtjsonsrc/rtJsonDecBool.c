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

#include "rtjsonsrc/osrtjson.h"

static int decJsonBoolValue (OSCTXT* pctxt, OSBOOL* pvalue, OSBOOL logError)
{
   int stat = 0;
   OSOCTET ub, us[5];
   OSBOOL value = FALSE;
   OSSIZE bytesRead = 1;
   static const OSSIZE falseLen = 5;
   static const OSSIZE trueLen = 4;

   if (pvalue)
      *pvalue = FALSE;

   rtJsonDecSkipWhitespace (pctxt);

   /* First character must be a 0, 1, t, or f */
   stat = rtxReadBytes (pctxt, &ub, 1);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (ub == '0') {
      value = FALSE;
   }
   else if (ub == '1') {
      value = TRUE;
   }
   else if (ub == 'f') {
      us[0] = ub;
      stat = rtxReadBytes (pctxt, us+1, falseLen-1);
      if (stat == 0 && OSCRTLSTRNCMP ((char*)us, "false", 5) == 0)
         value = FALSE;
      else {
         bytesRead = falseLen;
         stat = RTERR_IDNOTFOU;
      }
   }
   else if (ub == 't') {
      us[0] = ub;
      stat = rtxReadBytes (pctxt, us+1, trueLen-1);
      if (stat == 0 && OSCRTLSTRNCMP ((char*)us, "true", 4) == 0)
         value = TRUE;
      else {
         bytesRead = trueLen;
         stat = RTERR_IDNOTFOU;
      }
   }
   else stat = RTERR_IDNOTFOU;

   if (0 == stat) {
      if (pvalue) *pvalue = value;
   }
   else if (RTERR_IDNOTFOU == stat && logError) {
      rtxErrAddStrParm (pctxt, "true|false");
      rtxErrAddStrnParm (pctxt, (const char*)us, bytesRead);
      return LOG_RTERRNEW (pctxt, RTERR_IDNOTFOU);
   }

   return stat;
}

int rtJsonDecBool (OSCTXT* pctxt, OSBOOL* pvalue)
{
   return decJsonBoolValue (pctxt, pvalue, TRUE);
}

int rtJsonTryDecBool (OSCTXT* pctxt, OSBOOL* pvalue)
{
   OSSIZE pos;
   int ret, ret2 = rtxMarkPos (pctxt, &pos);
   if (0 != ret2) return LOG_RTERR (pctxt, ret2);

   ret = decJsonBoolValue (pctxt, pvalue, FALSE);

   if (0 != ret) {
      ret2 = rtxResetToPos (pctxt, pos);
      if (0 != ret2) return LOG_RTERR (pctxt, ret2);
   }

   return ret;
}
