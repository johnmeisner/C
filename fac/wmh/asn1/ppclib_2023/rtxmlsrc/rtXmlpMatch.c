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

EXTXMLMETHOD int rtXmlpMatchStartTag
   (OSCTXT* pctxt, const OSUTF8CHAR* elemLocalName, OSINT16 nsidx)
{
   OSXMLStrFragment localName;
   OSXMLEvent event;
   struct OSXMLReader* pXmlReader;
   OSINT16 nsIndex;

   pXmlReader = rtXmlpGetReader (pctxt);
   if (0 == pXmlReader) return LOG_RTERR (pctxt, RTERR_NOTINIT);

   event = rtXmlRdGetLastEvent (pXmlReader);

   if (event.mId != OSXMLEVT_START_TAG) {
      int stat = rtXmlRdNextEvent
         (pXmlReader, OSXMLEVT_START_TAG, event.mLevel + 1, 0);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   rtXmlRdGetTagName (pXmlReader, &localName, &nsIndex);

   /*  nsidx == -1 - check element name only */
   /*  localName.length == 0 - check namespace index only */
   /*  for anyElement with namespace */
   /* ??? relax namespace index check for local elements (nsidx == 0) */
   /* if ((nsidx == -1 || nsidx == nsIndex) && */
   if ((nsIndex == -2 ||
          nsidx == -1 ||
          nsidx == nsIndex ||
          nsidx == 0)
       &&
       (localName.length == 0 ||
        rtxUTF8StrnEqual (localName.value, elemLocalName, localName.length)))
   {
      rtXmlRdMarkLastEventDone (pXmlReader);
#ifdef XMLPTRACE
      RTDIAG4 (pctxt, "rtXmlpMatchStartTag: localName = '%-*.*s'\n",
               localName.length, localName.length, localName.value);
#endif
#ifndef _COMPACT
      /* Add name to element name stack in context */
      rtxDListAppend (pctxt, &pctxt->elemNameStack, (void*)elemLocalName);
#endif
      return 0;
   }

   if (nsidx < 0 || nsidx > pXmlReader->mNamespacesNumber)
      rtxErrAddStrParm (pctxt, "?");
   else if (nsidx == 0)
      rtxErrAddStrParm (pctxt, "");
   else
      rtxErrAddStrParm (pctxt,
         (const char*)pXmlReader->mNamespaceTable[nsidx - 1]);

   rtxErrAddStrParm (pctxt, (const char*)elemLocalName);

   if (nsIndex < 0  || nsIndex > pXmlReader->mNamespacesNumber)
      rtxErrAddStrParm (pctxt, "?");
   else if (nsIndex == 0)
      rtxErrAddStrParm (pctxt, "");
   else
      rtxErrAddStrParm (pctxt,
         (const char*)pXmlReader->mNamespaceTable[nsIndex - 1]);

   rtxErrAddStrnParm (pctxt, (const char*)localName.value, localName.length);
   rtXmlRdErrAddSrcPos (pXmlReader, FALSE);

   return LOG_RTERR (pctxt, XML_E_IDNOTFOU);
}

EXTXMLMETHOD int rtXmlpMatchEndTag (OSCTXT* pctxt, OSINT32 level)
{
   OSXMLEvent event;
   struct OSXMLReader* pXmlReader;
   int stat = 0;
   OSINT16 nsIndex;
   OSUINT32 textFlag = 0;


   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (level == -1)
      level = pXmlReader->mDecodeLevel;

   event = rtXmlRdGetLastEvent (pXmlReader);

   /*  skip unnecessary error message when content decode function */
   /*  return error and content size is above parser buffer size */
   if (event.mId == OSXMLEVT_TEXT)
      textFlag = OSXMLEVT_TEXT;

   while (event.mId != OSXMLEVT_END_TAG || event.mLevel > level)
   {
      int evt;

      /* check for extra elements */
      if (event.mId == OSXMLEVT_START_TAG) {
         OSXMLStrFragment localName;
         rtXmlRdGetTagName (pXmlReader, &localName, &nsIndex);

         rtxErrAddStrnParm
            (pctxt, (const char*)localName.value, localName.length);

         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         LOG_RTERRNEW (pctxt, RTERR_UNEXPELEM);

         stat = rtXmlRdSkipCurrentLevel (pXmlReader);

         if (stat < 0)
            return stat;

         event = rtXmlRdGetLastEvent (pXmlReader);
      }
      else {
         evt = rtXmlRdNextEvent
            (pXmlReader, OSXMLEVT_START_TAG | OSXMLEVT_END_TAG | textFlag,
             level, &event);

         if (evt < 0)
            return evt;
      }

   }

   #ifdef XMLPTRACE
   {
      OSXMLStrFragment localName;
      rtXmlRdGetTagName (pXmlReader, &localName, &nsIndex);

      RTDIAG4 (pctxt, "rtXmlpMatchEndTag: localName = '%-*.*s'\n",
               localName.length, localName.length, localName.value);
   }
   #endif

   rtXmlRdMarkLastEventDone (pXmlReader);
#ifndef _COMPACT
   rtxDListFreeNode (pctxt, &pctxt->elemNameStack, pctxt->elemNameStack.tail);
#endif

#ifdef _PULL_FROM_BUFFER
   if (pXmlReader->mDecodeLevel == 0 && !OSRTISSTREAM (pctxt)) {
      /* free inner buffer */
      rtxMemFreePtr (pctxt, pXmlReader->mpBuffer);
      pXmlReader->mpBuffer = 0;
   }
#endif /* _PULL_FROM_BUFFER */

   return stat;
}

EXTXMLMETHOD OSBOOL rtXmlpMatchElemId (OSCTXT* pctxt, int elemID, int matchingID)
{
   if (elemID == matchingID) {
      rtXmlRdMarkLastEventDone (XMLPREADER(pctxt));
      return TRUE;
   }
   return FALSE;
}


EXTXMLMETHOD OSINT32 rtXmlpGetCurrentLevel (OSCTXT* pctxt)
{
   return XMLPREADER(pctxt)->mDecodeLevel;
}

EXTXMLMETHOD void rtXmlpSetWhiteSpaceMode
   (OSCTXT* pctxt, OSXMLWhiteSpaceMode whiteSpaceMode)
{
   XMLPREADER(pctxt)->mStringWhiteSpaceMode = whiteSpaceMode;
}

EXTXMLMETHOD OSBOOL rtXmlpSetMixedContentMode (OSCTXT* pctxt, OSBOOL mixedContentMode)
{
   struct OSXMLReader* pXmlReader = XMLPREADER(pctxt);
   OSBOOL result;

   result = pXmlReader->mbMixedContext;
   pXmlReader->mbMixedContext = mixedContentMode;
   return result;
}

EXTXMLMETHOD OSBOOL rtXmlpIsContentMode (OSCTXT* pctxt)
{
   return (OSBOOL) (XMLPREADER(pctxt)->mDataMode != OSXMLDM_SIMULATED);
}

EXTXMLMETHOD void rtXmlpSetListMode (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pReader;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pReader);

   /* need to decode nested lists */
   pReader->mbSkipPullListElem = pReader->mbListMode;

   if (pReader->mDataMode == OSXMLDM_SIMULATED) {
      /* skip ws */
      while (pReader->mData.length) {
         if (OS_ISSPACE (*pReader->mData.value)) {
            pReader->mData.value++;
            pReader->mData.length--;
         }
         else if (*pReader->mData.value == '&') {
            /* get cur pos */
            OSBOOL hexForm = FALSE;
            OSXMLStrFragment tm;

            tm.value = pReader->mData.value;
            tm.length = pReader->mData.length;

            tm.value++;
            tm.length--;
            if (tm.length == 0 || *tm.value != '#')
               break;

            tm.value++;
            tm.length--;
            if (tm.length == 0)
               break;

            if (*tm.value == 'x') {
               hexForm = TRUE;
               tm.value++;
               tm.length--;
               if (tm.length == 0)
                  break;
            }

            /* skip 0s */
            while (tm.length > 0 && *tm.value == '0') {
               tm.value++;
               tm.length--;
            }

            if (hexForm) {
               if ((tm.length >= 2 && tm.value[1] == ';' &&
                      (tm.value[0] == '9' || tm.value[0] == 'A' ||
                       tm.value[0] == 'a' || tm.value[0] == 'D' ||
                       tm.value[0] == 'd')
                      )
                   ||
                   (tm.length >= 3 && tm.value[0] == '2' &&
                    tm.value[1] == '0' && tm.value[2] == ';')
                  )
               {
                  pReader->mData.value = tm.value + 1;
                  pReader->mData.length = tm.length - 1;
                  continue;
               }
            }
            else {
               if ((tm.length >= 2 && tm.value[0] == '9' &&
                    tm.value[1] == ';')
                   ||
                   (tm.length >= 3 && tm.value[2] == ';' &&
                      ((tm.value[0] == '1' &&
                         (tm.value[1] == '0' || tm.value[1] == '3')
                       )
                       ||
                       (tm.value[0] == '3' && tm.value[1] == '2')
                      )
                   ))
               {
                  pReader->mData.value = tm.value + 1;
                  pReader->mData.length = tm.length - 1;
                  continue;
               }
            }
         }

         break;
      }

      if (pReader->mData.length)
         pReader->mbListMode = TRUE;
   }

   else if (
       (pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_START_TAG ||
       (pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_END_TAG)
   {
      pReader->mbListMode = TRUE;
      pReader->mWhiteSpaceMode = OSXMLWSM_COLLAPSE;
   }
}

EXTXMLMETHOD OSBOOL rtXmlpListHasItem (OSCTXT* pctxt)
{
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pReader;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pReader);

   if (pReader->mbListMode) {
      if (pReader->mDataMode == OSXMLDM_SIMULATED) {
         if (!pReader->mData.length) {
            pReader->mDataMode = OSXMLDM_NONE;
            pReader->mbListMode = FALSE;
         }
      }
      else if (pReader->mbSkipPullListElem) {
         pReader->mbSkipPullListElem = FALSE;
      }
      else {
         int stat;

         pReader->mbLastChunk = FALSE;
         /* pReader->mbWSOnlyContext = FALSE; */

         while (1) {
            stat = rtXmlRdNext (pReader);
            if (stat < 0)
               return FALSE;

            if (pReader->mLastEvent.mId == OSXMLEVT_COMMENT)
               ;
            else if (pReader->mLastEvent.mId == OSXMLEVT_TEXT) {
               if (pReader->mData.length != 0)
                  break;
            }
            else if (pReader->mLastEvent.mId == OSXMLEVT_START_TAG) {
               /* error processed by rtXmlpMatchEndTag */
               pReader->mbListMode = FALSE;
               return FALSE;
            }
            else if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT) {
               pReader->mbListMode = FALSE;
               pReader->mError=XML_E_UNEXPEOF;
               LOG_RTERRNEW (pctxt, XML_E_UNEXPEOF);
               return FALSE;
            }
            else {
               pReader->mbListMode = FALSE;
               break;
            }
         }
      }
   }

   return pReader->mbListMode;
}

EXTXMLMETHOD void rtXmlpCountListItems (OSCTXT* pctxt, OSSIZE* itemCnt) {
   OSXMLCtxtInfo* xmlCtxt;
   struct OSXMLReader* pReader;
   OSSIZE cnt = 0;
   const OSUTF8CHAR* beg;
   const OSUTF8CHAR* end;
   OSBOOL flToken = FALSE;
   OSUTF8CHAR c;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   xmlCtxt = ((OSXMLCtxtInfo*)pctxt->pXMLInfo);
   pReader = xmlCtxt->pXmlPPReader;
   OSRTASSERT (0 != pReader);

   beg = pReader->mpBuffer + pReader->mByteIndex;
   end = pReader->mpBuffer + pReader->mReadSize;

   for (; beg!=end && (c = *beg) != '<'; beg++) {
      OSBOOL fl = (OSBOOL) (c > ' ');
      if (fl && fl != flToken) cnt++;
      flToken = fl;
   }

   if (beg != end)
      *itemCnt = cnt;
}

void rtXmlpSetNamespaceTable
#ifdef OSXMLNS12
(OSCTXT* pctxt, const OSUTF8CHAR* namespaceTable[], size_t nmNamespaces)
#else
(OSCTXT* pctxt, const char** namespaceTable, size_t nmNamespaces)
#endif
{
   struct OSXMLReader* pReader = XMLPREADER(pctxt);

   pReader->mPrevNamespaceTable = pReader->mNamespaceTable;
   pReader->mPrevNamespacesNumber = pReader->mNamespacesNumber;

   pReader->mNamespaceTable = namespaceTable;
   pReader->mNamespacesNumber = (int) nmNamespaces;
}

EXTXMLMETHOD void rtXmlpMarkPos (OSCTXT* pctxt)
{
   rtXmlRdMarkPos (XMLPREADER(pctxt));
}

EXTXMLMETHOD void rtXmlpRewindToMarkedPos (OSCTXT* pctxt)
{
   rtXmlRdRewindToMarkedPos (XMLPREADER(pctxt));
}

EXTXMLMETHOD void rtXmlpResetMarkedPos (OSCTXT* pctxt)
{
   rtXmlRdResetMarkedPos (XMLPREADER(pctxt));
}

EXTXMLMETHOD OSBOOL rtXmlpIsEmptyElement (OSCTXT* pctxt)
{
   struct OSXMLReader* pReader = XMLPREADER(pctxt);

   return (OSBOOL) (pReader->mbEmptyElement || rtXmlRdIsEmpty (pReader));
}

EXTXMLMETHOD int rtXmlDecEmptyElement (OSCTXT* pctxt)
{
   struct OSXMLReader* pReader = XMLPREADER(pctxt);
   OSXMLEvent event;

   event = rtXmlRdGetLastEvent(pReader);

   if ( ( event.mId & OSXMLEVT_ID_MASK ) != OSXMLEVT_START_TAG ) {
      return LOG_RTERR( pctxt, XML_E_INVMODE );
   }

   if ( pReader->mbEmptyElement || rtXmlRdIsEmpty (pReader))
      return 0;   /* content is empty */
   /* else: content may still be empty */

   /* The above test is not conclusive.  It may evaluate to FALSE when the only */
   /* conent is comments, which doesn't count as content.  In the most common */
   /* cases, however, it will quickly evaluate to TRUE when an element */
   /* really is empty.  The next test is longer and more accurate, to avoid */
   /* false negatives. */
   pReader->mWhiteSpaceMode = OSXMLWSM_PRESERVE;

   for(;;) {
      int stat;
      OSUINT32 eventId;

      stat = rtXmlRdNextEvent(pReader,
         OSXMLEVT_START_TAG | OSXMLEVT_END_TAG | OSXMLEVT_TEXT,
         -1 /*maxLevel - irrelevant*/, &event);

      if ( stat != 0 ) return stat;

      eventId = event.mId & OSXMLEVT_ID_MASK;

      /* eventId must be OSXMLEVT_START_TAG, OSXMLEVT_END_TAG, OSXMLEVT_TEXT */
      /* given args to rtXmlRdNextEvent */
      if ( eventId == OSXMLEVT_END_TAG ) return 0;    /* no content was found */
      else if ( eventId == OSXMLEVT_START_TAG ||
         ( eventId == OSXMLEVT_TEXT && pReader->mData.length > 0 ) )
      {
         /* content found */
         rtXmlRdErrAddSrcPos (pReader, FALSE);
         return LOG_RTERR(pctxt, XML_E_NOTEMPTY);
      }
      /* else: must be an empty TEXT event, which will pass over */
   }
}

