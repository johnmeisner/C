/*
 * Copyright (c) 2016-2023 by Objective Systems, Inc.
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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxGenValueType.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxHexDump.h"

const char* rtxGenValueGetIdent (OSRTDataType id)
{
   switch (id) {
   case OSRTDataType_UnsignedInt: return "UnsignedInt";
   case OSRTDataType_SignedInt: return "SignedInt";
   case OSRTDataType_ByteArray: return "ByteArray";
   case OSRTDataType_CharStr: return "CharStr";
   case OSRTDataType_Array: return "Array";
   case OSRTDataType_Bool: return "Boolean";
   case OSRTDataType_Float: return "Float";
   default: return "<unknown>";
   }
}

int rtxGenValueCompare
(OSCTXT* pctxt, const OSRTGenValue* pvalue1, const OSRTGenValue* pvalue2)
{
   if (0 == pvalue1 || 0 == pvalue2) return LOG_RTERR (pctxt, RTERR_NULLPTR);
   if (pvalue1->t != pvalue2->t) {
      rtxCtxtPushElemName (pctxt, OSUTF8("t"));
      rtxErrAddElemNameParm (pctxt);
      rtxErrAddStrParm (pctxt, rtxGenValueGetIdent(pvalue1->t));
      rtxErrAddStrParm (pctxt, rtxGenValueGetIdent(pvalue2->t));
      rtxCtxtPopElemName (pctxt);
      return LOG_RTERR (pctxt, RTERR_VALCMPERR);
   }
   switch (pvalue1->t) {
   case OSRTDataType_UnsignedInt:
      if (pvalue1->u.uintval != pvalue2->u.uintval) {
         rtxCtxtPushElemName (pctxt, OSUTF8("uintval"));
         rtxErrAddElemNameParm (pctxt);
#ifndef _NO_INT64_SUPPORT
         rtxErrAddUInt64Parm (pctxt, pvalue1->u.uintval);
         rtxErrAddUInt64Parm (pctxt, pvalue2->u.uintval);
#else
         rtxErrAddUIntParm (pctxt, pvalue1->u.uintval);
         rtxErrAddUIntParm (pctxt, pvalue2->u.uintval);
#endif
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      break;

   case OSRTDataType_SignedInt:
      if (pvalue1->u.intval != pvalue2->u.intval) {
         rtxCtxtPushElemName (pctxt, OSUTF8("intval"));
         rtxErrAddElemNameParm (pctxt);
#ifndef _NO_INT64_SUPPORT
         rtxErrAddInt64Parm (pctxt, pvalue1->u.intval);
         rtxErrAddInt64Parm (pctxt, pvalue2->u.intval);
#else
         rtxErrAddIntParm (pctxt, pvalue1->u.intval);
         rtxErrAddIntParm (pctxt, pvalue2->u.intval);
#endif
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      break;

   case OSRTDataType_ByteArray:
      if (pvalue1->u.bytestr.numocts != pvalue2->u.bytestr.numocts) {
         rtxCtxtPushElemName (pctxt, OSUTF8("bytestr.numocts"));
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddSizeParm (pctxt, pvalue1->u.bytestr.numocts);
         rtxErrAddSizeParm (pctxt, pvalue2->u.bytestr.numocts);
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      else if (0 != OSCRTLMEMCMP (pvalue1->u.bytestr.data,
                                  pvalue2->u.bytestr.data,
                                  pvalue1->u.bytestr.numocts)) {
         char* diffbuf;
         rtxCtxtPushElemName (pctxt, OSUTF8("bytestr.data"));
         rtxErrAddElemNameParm (pctxt);
         diffbuf = rtxHexDiffToDynString
            (pctxt, pvalue1->u.bytestr.data, pvalue2->u.bytestr.data,
             pvalue1->u.bytestr.numocts);

         if (0 != diffbuf) {
            rtxErrAddStrParm (pctxt, diffbuf);
            rtxMemFreePtr (pctxt, diffbuf);
         }
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_BUFCMPERR);
      }
      break;

   case OSRTDataType_CharStr:
      if (0 != OSCRTLSTRCMP (pvalue1->u.utf8str, pvalue2->u.utf8str)) {
         rtxCtxtPushElemName (pctxt, OSUTF8("utf8str.data"));
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddStrParm (pctxt, pvalue1->u.utf8str);
         rtxErrAddStrParm (pctxt, pvalue2->u.utf8str);
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      break;

   case OSRTDataType_Array:
      if (pvalue1->u.array.nitems != pvalue2->u.array.nitems) {
         rtxCtxtPushElemName (pctxt, OSUTF8("array.nitems"));
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddSizeParm (pctxt, pvalue1->u.array.nitems);
         rtxErrAddSizeParm (pctxt, pvalue2->u.array.nitems);
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      else {
         OSSIZE i;
         for (i = 0; i < pvalue1->u.array.nitems; i++) {
            rtxCtxtPushArrayElemName (pctxt, OSUTF8("array.values"), i);
            rtxGenValueCompare
               (pctxt, pvalue1->u.array.values[i], pvalue2->u.array.values[i]);
            rtxCtxtPopArrayElemName (pctxt);
         }
      }
      break;

   case OSRTDataType_Bool:
      if (pvalue1->u.boolval != pvalue2->u.boolval) {
         rtxCtxtPushElemName (pctxt, OSUTF8("boolval"));
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddIntParm (pctxt, pvalue1->u.boolval);
         rtxErrAddIntParm (pctxt, pvalue2->u.boolval);
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      break;

   case OSRTDataType_Float:
      if (pvalue1->u.fltval != pvalue2->u.fltval) {
         rtxCtxtPushElemName (pctxt, OSUTF8("fltval"));
         rtxErrAddElemNameParm (pctxt);
         rtxErrAddDoubleParm (pctxt, pvalue1->u.fltval);
         rtxErrAddDoubleParm (pctxt, pvalue2->u.fltval);
         rtxCtxtPopElemName (pctxt);
         LOG_RTERRNEW (pctxt, RTERR_VALCMPERR);
      }
      break;

   default:
      rtxErrAddIntParm (pctxt, pvalue1->t);
      LOG_RTERR (pctxt, RTERR_NOTSUPP);
   }

   return rtxErrGetStatus (pctxt);
}
