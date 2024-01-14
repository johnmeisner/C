/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxTokenConst.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.h"

/**************************************************************/
/*                                                            */
/*  IDREFS, NMTOKENS, NMENTITIES                              */
/*                                                            */
/**************************************************************/
EXTXMLMETHOD int rtSaxStrListParse (OSCTXT* pctxt, OSRTMEMBUF *pMemBuf, OSRTDList* pvalue)
{
   int stat = 0;
   OSCONSTTOKENCTXT tokCtxt;
   const OSUTF8CHAR* pToken;
   OSUTF8CHAR* pData;
   size_t dataSize;
   OSBOOL bFinal;

   if (pMemBuf != 0) {
      pctxt = pMemBuf->pctxt;
      pData = (OSUTF8CHAR*)OSMEMBUFPTR (pMemBuf);
      dataSize = OSMEMBUFUSEDSIZE (pMemBuf);
      bFinal = FALSE;
   }
   else {
      pData = (OSUTF8CHAR*)OSRTBUFPTR (pctxt);
      dataSize = OSRTBUFSIZE (pctxt);
      bFinal = TRUE;
   }

   pToken = rtxTokGetFirstConst
      (&tokCtxt, pData, dataSize, " \r\n\t\0", 5, &dataSize, bFinal);

   while (pToken != 0 && stat == 0) {
      OSXMLSTRING* value;

      rtxInitContextBuffer (pctxt, OSRTSAFECONSTCAST (OSOCTET*, pToken),
         dataSize);

      value = OSRTALLOCTYPE (pctxt, OSXMLSTRING);
      if (value == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

      stat = rtXmlDecXmlStr (pctxt, value);
      if (stat == 0)
         rtxDListAppend (pctxt, pvalue, value);

      pToken = rtxTokGetNextConst (&tokCtxt, &dataSize);
   }
   if (pMemBuf != 0) {
      rtxMemBufCut (pMemBuf, 0, tokCtxt.pStr - OSMEMBUFPTR (pMemBuf));
   }
   if (stat != 0) return LOG_RTERRNEW (pctxt, stat);
   return 0;
}

EXTXMLMETHOD int rtSaxStrListMatch (OSCTXT* pctxt)
{
   OS_UNUSED_ARG(pctxt);
   pctxt = 0; /* to keep VC++ -W4 happy */
   return 0;
}

