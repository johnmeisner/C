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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxmlsrc/rtXmlPull.hh"
#include "rtxmlsrc/rtXmlErrCodes.h"

EXTXMLMETHOD int rtXmlpGetNextElem (OSCTXT* pctxt, OSXMLElemDescr* pElem, OSINT32 level)
{
   struct OSXMLReader* pXmlReader;
   OSINT32 curLevel;
   int stat = 0;
   OSXMLEvent event;

   pXmlReader = rtXmlpGetReader (pctxt);
   if (0 == pXmlReader) return LOG_RTERR (pctxt, RTERR_NOTINIT);

   if (level == -1)
      level = pXmlReader->mDecodeLevel;

   curLevel = rtXmlpGetCurrentLevel (pctxt);
   if (curLevel < level) {
      return XML_OK_EOB;
   }

   event = rtXmlRdGetLastEvent (pXmlReader);

   if (event.mId == OSXMLEVT_END_TAG) {
      if (event.mLevel == level) {
         return XML_OK_EOB;
      }
   }

   if (event.mId != OSXMLEVT_START_TAG) {
      stat = rtXmlRdNextEvent
         (pXmlReader, OSXMLEVT_START_TAG | OSXMLEVT_END_TAG, level, &event);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }

   if (event.mId == OSXMLEVT_END_TAG) {
      if (event.mLevel == level) {
         return XML_OK_EOB;
      }
   }

   rtXmlRdMarkLastEventDone (pXmlReader);

   rtXmlRdGetTagName (pXmlReader, &pElem->localName, &pElem->nsidx);

#ifdef XMLPTRACE
   RTDIAG3 (pctxt, "rtXmlpGetNextElem: localName = '%.*s'\n",
            pElem->localName.length, pElem->localName.value);
#endif
#ifndef _COMPACT
   /*
    * Allocate node and data for elemNameStack list element.  In this
    * case, both must be allocated as one unit because the free logic
    * only frees the node..
    */
   { OSRTDListUTF8StrNode* pStrNode =
        (OSRTDListUTF8StrNode*) rtxMemAlloc
        (pctxt, sizeof(OSRTDListUTF8StrNode) + pElem->localName.length);

   if (0 == pStrNode) return RTERR_NOMEM;

   pStrNode->node.data = pStrNode->utf8chars;

   OSCRTLSAFEMEMCPY ((void*)pStrNode->utf8chars, pElem->localName.length,
           pElem->localName.value, pElem->localName.length);

   pStrNode->utf8chars[pElem->localName.length] = '\0';

   rtxDListAppendNode (&pctxt->elemNameStack, &pStrNode->node);
   }
#endif

   return 0;
}

