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

#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxTokenConst.h"

OSBOOL rtxTokIsWhiteSpaceConst (OSCONSTTOKENCTXT* pTokCtxt, OSUTF8CHAR ch)
{
   size_t i, delimSize = pTokCtxt->delimSize;

   for (i = 0; i < delimSize; i++) {
      if (ch == pTokCtxt->delimiters [i])
         return TRUE;
   }
   return FALSE;
}

const OSUTF8CHAR* rtxTokSkipWhiteSpacesConst (OSCONSTTOKENCTXT* pTokCtxt)
{
   size_t i, delimSize = pTokCtxt->delimSize;
   const OSUTF8CHAR* pStr = pTokCtxt->pStr;
   const OSUTF8CHAR* pEndStr = pTokCtxt->pEndStr;

   while (pStr < pEndStr) {
      for (i = 0; i < delimSize; i++) {
         if (*pStr == pTokCtxt->delimiters [i])
            break;
      }
      if (i >= delimSize)
         break;
      pStr++;
   }
   return (pTokCtxt->pStr = pStr);
}

const OSUTF8CHAR* rtxTokGetNextConst
(OSCONSTTOKENCTXT* pTokCtxt, size_t* pTokenLength)
{
   const OSUTF8CHAR* pStr, *pToken;
   const OSUTF8CHAR* pEndStr;

   OSRTASSERT (pTokCtxt != 0);

   pEndStr = pTokCtxt->pEndStr;

   pToken = pStr = rtxTokSkipWhiteSpacesConst (pTokCtxt);

   while (pStr < pEndStr) {
      if (rtxTokIsWhiteSpaceConst (pTokCtxt, *pStr))
         break;
      pStr++;
   }
   if (pToken != pStr && (pStr < pEndStr || (pStr == pEndStr && pTokCtxt->bFinal))) {
      pTokCtxt->lastTokenSize = (size_t)(pStr - pToken);
      if (pTokenLength != 0)
         *pTokenLength = pTokCtxt->lastTokenSize;
      pTokCtxt->pStr = pStr;
      pTokCtxt->pLastToken = pToken;
      return pToken;
   }
   pTokCtxt->lastTokenSize = 0;
   pTokCtxt->pLastToken = 0;
   if (pTokenLength != 0)
      *pTokenLength = pTokCtxt->lastTokenSize;
   return 0;
}

const OSUTF8CHAR* rtxTokGetFirstConst (OSCONSTTOKENCTXT* pTokCtxt,
                                       const OSUTF8CHAR* pStr, size_t strSize,
                                       const char* delimiters, size_t delimSize,
                                       size_t* pTokenLength, OSBOOL isFinal)
{
   OSRTASSERT(pTokCtxt != 0 && delimiters != 0 && delimSize != 0);

   pTokCtxt->pStr = pStr;
   pTokCtxt->delimiters = delimiters;
   pTokCtxt->delimSize = delimSize;
   pTokCtxt->bFinal = isFinal;
   if (pStr != 0) {
      pTokCtxt->pEndStr = pStr + strSize;
      return rtxTokGetNextConst (pTokCtxt, pTokenLength);
   }
   else
      pTokCtxt->pEndStr = 0;
   return 0;
}

