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

#include "rtxsrc/rtxCommon.hh"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxToken.h"

#include "rtxmlsrc/osrtxml.hh"
#include "rtxmlsrc/rtSaxCppAny.h"

/**************************************************************/
/*                                                            */
/*  any                                                       */
/*                                                            */
/**************************************************************/
#ifdef ASN1RT
EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(const OSUTF8CHAR** ppMsgData, OSRTContext* pContext, int level) :
   OSXMLDefaultHandler (pContext), mppMsgData (ppMsgData)
{
   mLevel = 0;
   mStartLevel = level;
   localInit (pContext);
}

EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(const OSUTF8CHAR** ppMsgData, OSRTContext* pContext,
 const OSUTF8CHAR* elemName) :
   OSXMLDefaultHandler (pContext), mppMsgData (ppMsgData)
{
   mLevel = 0;
   mStartLevel = 1;
   localInit (pContext);
   mpElemName = elemName;
}
#else // XBinder
EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(OSXSDAnyTypeClass& msgData, OSRTContext* pContext, int level):
   OSXMLDefaultHandler (pContext),
   mpAnyMsgData (0), mpAnyTypeMsgData (&msgData)
{
   mLevel = 0;
   mStartLevel = (OSINT16)level;
   localInit (pContext);
}

EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(OSXSDAnyTypeClass& msgData, OSRTContext* pContext, const OSUTF8CHAR* elemName)
   :   OSXMLDefaultHandler (pContext),
       mpAnyTypeMsgData (&msgData), mpAnyMsgData (0)
{
   mLevel = 0;
   mStartLevel = 1;
   localInit (pContext);
   mpElemName = elemName;
}

// xsd:any

EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(OSRTXMLString& msgData, OSRTContext* pContext, int level):
   OSXMLDefaultHandler (pContext),
   mpAnyMsgData (&msgData), mpAnyTypeMsgData (0)
{
   mLevel = 0;
   mStartLevel = (OSINT16)level;
   localInit (pContext);
}

EXTXMLMETHOD OSXMLAnyHandler::OSXMLAnyHandler
(OSRTXMLString& msgData, OSRTContext* pContext, const OSUTF8CHAR* elemName)
   :   OSXMLDefaultHandler (pContext),
       mpAnyMsgData (&msgData), mpAnyTypeMsgData (0)
{
   mLevel = 0;
   mStartLevel = 1;
   localInit (pContext);
   mpElemName = elemName;
}
#endif

EXTXMLMETHOD void OSXMLAnyHandler::localInit (OSRTContext* pContext)
{
   rtxCtxtSetMemHeap (mEncCtxt.getPtr(), pContext->getPtr());
   rtXmlSetEncBufPtr (mEncCtxt.getPtr(), 0, 1024);
   if (mStartLevel >= mLevel)
      mEncCtxt.getPtr()->state = OSXMLINIT;
   else
      mEncCtxt.getPtr()->state = OSXMLSTART;
}

EXTXMLMETHOD OSXMLAnyHandler::~OSXMLAnyHandler ()
{
}

EXTXMLMETHOD int OSXMLAnyHandler::startElement
   (const OSUTF8CHAR* const /*uri*/,
    const OSUTF8CHAR* const localname,
    const OSUTF8CHAR* const qname,
    const OSUTF8CHAR* const* attrs)
{
   OSCTXT* pctxt = mpContext->getPtr ();
   OSCTXT* pEncCtxt = mEncCtxt.getPtr ();
   int stat = 0;

   OSCPPSAXDIAGSTART ("OSXMLAnyHandler::startElement", localname);

   if (mLevel == 0 && mpElemName != 0) {
      int stat = rtXmlDecXSIAttrs (pctxt, attrs, "any");
      if (stat != 0) LOG_SAXERR (pctxt, stat);

      pEncCtxt->state = OSXMLINIT;

      if (!rtxUTF8StrEqual ((const OSUTF8CHAR*)localname, mpElemName)) {
         return LOG_SAXERRNEW2_AND_SKIP (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm
               (pctxt, (const char*)mpElemName),
            rtxErrAddStrParm (pctxt, (const char*)qname));
      }
   }

   if (mLevel == mStartLevel) {
      pEncCtxt->state = OSXMLSTART;
   }

   if (pEncCtxt->state != OSXMLINIT) {

      /* Encode start element */

      stat = rtXmlPutChar (pEncCtxt, '<');
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      stat = rtXmlWriteChars (pEncCtxt, localname,
                       rtxUTF8LenBytes (localname));
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      /* add attributes if present */
      if (attrs != 0) {
         for (int i = 0; attrs[i] != 0; i += 2) {
            stat = rtXmlEncUTF8Attr (pEncCtxt, attrs[i], attrs[i + 1]);
            if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
         }
      }
      stat = rtXmlPutChar (pEncCtxt, '>');
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
   }
   /* Bump level */

   mLevel++;

   OSCPPSAXDIAGEND ("OSXMLAnyHandler::startElement", localname);
   return 0;
}

EXTXMLMETHOD int OSXMLAnyHandler::characters
   (const OSUTF8CHAR* const chars, unsigned int length)
{
   OSCTXT* pctxt = mpContext->getPtr ();
   OSCTXT* pEncCtxt = mEncCtxt.getPtr ();
   int stat;

   RTDIAG1 (pctxt, "OSXMLAnyHandler::characters: start\n");
   if (pEncCtxt->state == OSXMLSTART || pEncCtxt->state == OSXMLDATA) {
      stat = rtXmlEncStringValue2 (pEncCtxt, chars, length);
      if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

      pEncCtxt->state = OSXMLDATA;
   }
   RTDIAG1 (pctxt, "OSXMLAnyHandler::characters: end\n");
   return 0;
}

EXTXMLMETHOD int OSXMLAnyHandler::endElement
   (const OSUTF8CHAR* const /*uri*/,
    const OSUTF8CHAR* const localname,
    const OSUTF8CHAR* const /*qname*/)
{
   OSCTXT* pctxt = mpContext->getPtr ();
   OSCTXT* pEncCtxt = mEncCtxt.getPtr ();

   OSCPPSAXDIAGSTART ("OSXMLAnyHandler::endElement", localname);

   --mLevel;

   if (pEncCtxt->state == OSXMLSTART || pEncCtxt->state == OSXMLDATA) {
      /* Encode end element */

      if (mLevel >= 0 && !isEmptyElement (localname)) {
         int stat;

         stat = rtXmlWriteChars (pEncCtxt, OSUTF8("</"), 2);
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         stat = rtXmlWriteChars (pEncCtxt,
                          localname,
                          rtxUTF8LenBytes ((const OSUTF8CHAR*)localname));
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         stat = rtXmlPutChar (pEncCtxt, '>');
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);
      }

      if (isComplete ()) {
         int stat;

         /* finalize value by trailing zero */

         stat = rtXmlPutChar (pEncCtxt, '\0');
         if (stat != 0) return LOG_SAXERR_AND_SKIP (pctxt, stat);

         /* Update mem link and open type data variable */
#ifdef ASN1RT
         *mppMsgData =
            rtxUTF8Strdup (pctxt, (const OSUTF8CHAR*)pEncCtxt->buffer.data);
#else // XBinder
         if (0 != mpAnyTypeMsgData)
            mpAnyTypeMsgData->setValue (pEncCtxt->buffer.data);
         else
            mpAnyMsgData->setValue (pEncCtxt->buffer.data);
#endif
         rtxMemFree (pEncCtxt);
         pEncCtxt->buffer.data = 0;
         pEncCtxt->buffer.size = 0;
         pEncCtxt->state = OSXMLEND;
      }
   }

   OSCPPSAXDIAGEND ("OSXMLAnyHandler::endElement", localname);
   return 0;
}


/* Check encode buffer for empty element.  This method will modify the
 * buffer contents if element is found to be empty by replacing last
 * '>' character with '/>'.
 */
EXTXMLMETHOD OSBOOL OSXMLAnyHandler::isEmptyElement (const OSUTF8CHAR* qname)
{
   OSCTXT* pEncCtxt = mEncCtxt.getPtr ();
   char* bufp;
   int stat;

   pEncCtxt->buffer.byteIndex--;
   bufp = (char*) OSRTBUFPTR (pEncCtxt);
   if (*bufp == '>') { /* no character data */
      *bufp-- = '\0';  /* null out last '>' character */
      while (*bufp != '<') {
         bufp--;
      }
      bufp++;
      if (rtXmlStrCmpAsc (qname, bufp)) {
         /* empty element */
         stat = rtXmlWriteChars (pEncCtxt, OSUTF8("/>"), 2);
         return TRUE;
      }
      else
         stat = rtXmlPutChar (pEncCtxt, '>');  /* put back original '>' */
   }
   else
      pEncCtxt->buffer.byteIndex++;    /* restore byte index */

   return FALSE;
}
