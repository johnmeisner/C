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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxToken.h"

#include "rtxmlsrc/rtSaxCppSimpleType.h"

/**************************************************************/
/*                                                            */
/*  simpleType                                                */
/*                                                            */
/**************************************************************/
EXTXMLMETHOD int OSXMLSimpleTypeHandler::startElement
   (const OSUTF8CHAR* const /*uri*/,
    const OSUTF8CHAR* const localname,
    const OSUTF8CHAR* const qname,
    const OSUTF8CHAR* const* attrs)
{
   OSCTXT* pctxt = mpContext->getPtr ();

   OSCPPSAXDIAGSTART ("OSXMLSimpleTypeHandler::startElement", localname);

   if (mLevel == 0) {
      int stat = rtXmlDecXSIAttrs (pctxt, attrs, (const char*)mpElemName);
      if (stat != 0) LOG_SAXERR (pctxt, stat);

      pctxt->state = OSXMLSTART;

      if (!rtxUTF8StrEqual ((const OSUTF8CHAR*)localname,
           mpElemName))
      {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, (const char*)mpElemName),
            rtxErrAddStrParm (pctxt, (const char*)qname));
      }
   }
   mLevel++;

   OSCPPSAXDIAGEND ("OSXMLSimpleTypeHandler::startElement", localname);
   return 0;
}

EXTXMLMETHOD int OSXMLSimpleTypeHandler::characters
   (const OSUTF8CHAR* const chars, unsigned int length)
{
   OSCTXT* pctxt = mpContext->getPtr ();

   RTDIAG1 (pctxt, "OSXMLSimpleTypeHandler::characters: start\n");

   if ((pctxt->state == OSXMLSTART || pctxt->state == OSXMLDATA)
      && (mLevel >= 1))
   {
      int stat;
      pctxt->state = OSXMLDATA;
      stat = rtxMemBufAppend (&mCurrElemValue, (const OSOCTET*) chars,
                              length * sizeof (OSUTF8CHAR));
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
   }
   RTDIAG1 (pctxt, "OSXMLSimpleTypeHandler::characters: end\n");
   return 0;
}

EXTXMLMETHOD int OSXMLSimpleTypeHandler::endElement
   (const OSUTF8CHAR* const /*uri*/,
    const OSUTF8CHAR* const localname,
    const OSUTF8CHAR* const /*qname*/)
{
   OSCTXT* pctxt = mpContext->getPtr ();

   OSCPPSAXDIAGSTART ("OSXMLSimpleTypeHandler::endElement", localname);

   if (mLevel == 1 &&
      (pctxt->state == OSXMLSTART || pctxt->state == OSXMLDATA))
   {
      rtXmlFinalizeMemBuf (&mCurrElemValue);

      finalize ();
   }

   OSCPPSAXDIAGEND ("OSXMLSimpleTypeHandler::endElement", localname);
   return 0;
}

EXTXMLMETHOD OSXMLSimpleTypeHandler::OSXMLSimpleTypeHandler
   (OSRTContext* pContext, const OSUTF8CHAR* elemName):
    OSXMLDefaultHandler (pContext)
{
   rtxMemBufInit (mpContext->getPtr (), &mCurrElemValue, OSRTMEMBUFSEG);
   mpElemName = elemName;
}

EXTXMLMETHOD OSXMLSimpleTypeHandler::~OSXMLSimpleTypeHandler ()
{
   rtxMemBufFree (&mCurrElemValue);
}


