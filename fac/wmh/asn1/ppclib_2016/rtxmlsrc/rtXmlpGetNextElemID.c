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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxmlsrc/rtXmlPull.hh"
#include "rtxmlsrc/rtXmlErrCodes.h"

/*
 * This function should work only inside the current level.
 * Once the start or end tag parsed is out of scope of current level
 * this function returns.
 */
EXTXMLMETHOD int rtXmlpGetNextElemID
(OSCTXT* pctxt, const OSXMLElemIDRec* tab, size_t nrows, OSINT32 level,
 OSBOOL continueParse)
{
   struct OSXMLReader* pXmlReader;
   OSXMLStrFragment localName;
   OSINT16 nsIndex;
   OSINT32 curLevel;
   int stat = 0;
   size_t i;
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

   do {
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

      if (event.mId == OSXMLEVT_START_TAG) {
         if (event.mLevel != level + 1) {
            rtXmlRdGetTagName (pXmlReader, &localName, 0);

            rtxErrAddStrnParm
               (pctxt, (const char*)localName.value, localName.length);

            rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
            return LOG_RTERRNEW (pctxt, XML_E_UNEXPSTARTTAG);
         }
      }
      else {
         rtXmlRdGetTagName (pXmlReader, &localName, 0);

         rtxErrAddStrnParm
            (pctxt, (const char*)localName.value, localName.length);

         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         return LOG_RTERRNEW (pctxt, XML_E_UNEXPENDTAG);
      }

      rtXmlRdGetTagName (pXmlReader, &localName, &nsIndex);
#ifdef XMLPTRACE
      RTDIAG4 (pctxt, "rtXmlpGetNextElemID: localName = '%-*.*s'\n",
               localName.length, localName.length, localName.value);
#endif
      /* first pass: check named elements */
      for (i = 0; i < nrows; i++) {
         if (tab[i].descr.localName.length == 0) continue;
         /* nsidx == -1 - check element name only */
         /*??? relax namespace index check for local elements */
         /*if ((tab[i].descr.nsidx == -1 || nsIndex == tab[i].descr.nsidx) && */
         if ((nsIndex == -2 ||
                tab[i].descr.nsidx == -1 ||
                nsIndex == tab[i].descr.nsidx ||
                tab[i].descr.nsidx == 0)
             &&
             (tab[i].descr.localName.length == 0 ||
                 OSXMLFRAGSEQUAL (localName, tab[i].descr.localName)))
         {
            rtXmlRdMarkLastEventDone (pXmlReader);
#ifndef _COMPACT
            /* Add name to element name stack in context */
            rtxDListAppend (pctxt, &pctxt->elemNameStack,
                            (void*)tab[i].descr.localName.value);
#endif
            return tab[i].id;
         }
      }

      /* second pass: check any elements by namespace */
      for (i = 0; i < nrows; i++) {
         OSBOOL found;
         if (tab[i].descr.localName.length != 0) continue;

         /* ##other - length == 0 and localName is not empty */
         found = FALSE;

         if (tab[i].descr.localName.value &&
             tab[i].descr.localName.value[0] != 0)
         {
            /* ##other */
            if (nsIndex != 0 && nsIndex != tab[i].descr.nsidx)
               found = TRUE;
         }
         else if (nsIndex == tab[i].descr.nsidx) {
            found = TRUE;
         }

         if (found) {
            rtXmlRdMarkLastEventDone (pXmlReader);
#ifndef _COMPACT
            /* Add name to element name stack in context */
            rtxDListAppend (pctxt, &pctxt->elemNameStack, "*");
#endif
            return tab[i].id;
         }
      }

      /* not found! skip whole level and log warning here */

      if (continueParse) {
         rtxErrAddStrnParm
            (pctxt, (const char*)localName.value, localName.length);

         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         LOG_RTERRNEW (pctxt, RTERR_UNEXPELEM);

         stat = rtXmlRdSkipCurrentLevel (pXmlReader);
         event = rtXmlRdGetLastEvent (pXmlReader);
      }
      else {
         stat = RTERR_UNEXPELEM;
#ifndef _COMPACT
         /* Add dummy name to element name stack in context */
         rtxDListAppend (pctxt, &pctxt->elemNameStack, (void*)"<unknown>");
#endif
      }

   } while (stat == 0);

   return stat;
}

EXTXMLMETHOD int rtXmlpGetNextSeqElemID
(OSCTXT* pctxt, const OSXMLElemIDRec* tab, const OSXMLGroupDesc* ppGroup,
 int curID, int lastMandatoryID, OSBOOL groupMode)
{
   return rtXmlpGetNextSeqElemID2(pctxt, tab, ppGroup,
      0 /*don't know # of groups */, curID, lastMandatoryID, groupMode, FALSE);
}

EXTXMLMETHOD int rtXmlpGetNextSeqElemID2
(OSCTXT* pctxt, const OSXMLElemIDRec* tab, const OSXMLGroupDesc* ppGroup,
 int groups, int curID, int lastMandatoryID, OSBOOL groupMode,
 OSBOOL checkRepeat)
{
   int fromrow = ppGroup[curID].row;
   int rows = ppGroup[curID].num;
   int anyCase = ppGroup[curID].anyCase;
   int elemID;
   OSBOOL skipUnknown = TRUE;

   if ( checkRepeat ) {
      /*
      When checkRepeat is true, in case there isn't a repetition, we want to be
      able to recognize elements for the id that follows curID, if there is one.
      Adjust rows if necessary.
      */
      if ( curID < groups - 1 &&
         fromrow + rows < ppGroup[curID+1].row + ppGroup[curID+1].num ) {
         /* Assign rows so that it includes the rows for curID + 1 */
         rows = ppGroup[curID+1].row + ppGroup[curID+1].num - fromrow;
      }
      /* else: there is no curID + 1 or the rows for curID + 1 are already
               included */
   }

   /* When decoding a sequence that is not the complete content model for a
      type, we must break on the first unknown element type where only optional
      elements remain, as the unknown element may be recognized outside of
      the sequence.  Otherwise, we can skip the unknown element and try to
      locate elements belonging to the sequence.

      When groupMode and checkRepeat are TRUE, we are decoding a repeating
      particle, inside a group, which particle has already had one occurrence.
      If there are no subsequent required particles, we should not skip an
      unexpected element; it may be recognized by the enclosing group. This
      risks not skipping an element when it is actually followed by another
      repetition of the current particle, but we cannot be sure whether another
      repetition is required or even allowed, and this risk only applies to
      invalid documents anyway. Since some valid documents would require not
      skipping the unexpected element, and no valid documents would require
      skipping, the safe thing is to not skip.
      */
   if (anyCase >= 0 || (groupMode && (curID > lastMandatoryID ||
         (checkRepeat && curID + 1 > lastMandatoryID) ) ) )
      skipUnknown = FALSE;

   elemID = rtXmlpGetNextElemID (pctxt, tab + fromrow, rows, -1, skipUnknown);

   /* if group has any element use it */
   if (elemID == RTERR_UNEXPELEM && anyCase >= 0) {
      elemID = anyCase;
   }
   else if (elemID == RTERR_UNEXPELEM) {
      /* elemID == RTERR_UNEXPELEM implies skipUnknown == FALSE; together
         with anyCase < 0, this implies groupMode == TRUE.
         No error will have been logged.
      */
      OSXMLStrFragment localName;
      struct OSXMLReader* pXmlReader;

      OSRTASSERT (0 != pctxt->pXMLInfo);
      pXmlReader = rtXmlpGetReader (pctxt);

      rtXmlRdGetTagName (pXmlReader, &localName, 0);

      rtxErrAddStrnParm
            (pctxt, (const char*)localName.value, localName.length);
      rtXmlRdErrAddSrcPos (pXmlReader, FALSE);

      if (curID > lastMandatoryID || (checkRepeat && curID == lastMandatoryID))
         /*
         We hit an unexpected element.  There are no more required elements
         so return XML_OK_EOB to signal the caller to break the group decoding.
         */
         elemID = XML_OK_EOB;
      else
         /* Some mandatory item has not been found yet so the group cannot
            possibly be complete.  Report RTERR_UNEXPELEM. */
         return LOG_RTERR (pctxt, elemID);
   }
   else if (elemID < 0) {
      return LOG_RTERR (pctxt, elemID);
   }
   else if (elemID == XML_OK_EOB) {
      /* If not all mandatory elements decoded, report XML_E_ELEMSMISRQ.
         If curID is the last mandatory item and we're checking for a
         repeat of it, don't report it as missing.
      */
      if (curID <= lastMandatoryID &&
         !(checkRepeat && curID == lastMandatoryID))
         return LOG_RTERRNEW (pctxt, XML_E_ELEMSMISRQ);
   }

   return elemID;
}


EXTXMLMETHOD int rtXmlpGetNextSeqElemIDExt
(OSCTXT* pctxt, const OSXMLElemIDRec* tab, const OSXMLGroupDesc* ppGroup,
 const OSBOOL* extRequired, int postExtRootID,
 int curID, int lastMandatoryID, OSBOOL groupMode)
{
   /*
   There are three cases where we can just invoke rtXmlpGetNextSeqElemID.
   1) The decode group includes an any case.  This means there cannot be any
   required extension elements ahead (they would intervene and hide the any
   case), so we don't need to worry about skipping over them or not.
   2) The decode group includes a required extension element that really must be
   present.
   3) We have already gotten past the extension elements.
   */
   if ( ppGroup[curID].anyCase >= 0 || extRequired[curID] ||
      (postExtRootID >=0 && curID >= postExtRootID ) )
   {
      return rtXmlpGetNextSeqElemID(pctxt, tab, ppGroup, curID, lastMandatoryID,
               groupMode);
   }
   else {
      /* Since the any case is not visible and we haven't gotten beyond the
         extension elements, there must be some required extension elements
         ahead. However, these may actually be absent in the encoding, so we
         have to be able to look for matches beyond them.  To do that, we pass
         lastMandatoryID = -1 and groupMode = TRUE, which prevents skipping
         elements and avoids getting an XML_E_ELEMSMISRQ if the extensions are
         absent.
      */
      int ret = rtXmlpGetNextSeqElemID(pctxt, tab, ppGroup, curID,
                                    -1 /*lastMandatoryID*/, TRUE /*groupMode*/);

      if ( ret == XML_OK_EOB ) {
         /* Note: XML_OK_EOB is returned for unexpected elements also. */
         /* We know we didn't match any of the extension elements, some of which
            were required.  This means if there was an unexpected element, it
            would have to match against root elements, if there are any.
            We'll effectively jump ahead to the point where any root elements
            are visible and try again.  If there are no root elements,
            we're done.
         */
         if ( postExtRootID >= 0) {
            return rtXmlpGetNextSeqElemID(pctxt, tab, ppGroup, postExtRootID,
                                             lastMandatoryID, groupMode);
         }
         else return ret;
      }
      else {
         //a match or some error
         return ret;
      }
   }
}



/* rtXmlpGetNextAllElemID */
#define _rtXmlpGetNextAllElemID_NAME rtXmlpGetNextAllElemID
#define _rtXmlpGetNextAllElemID_TYPE OSUINT8
#include "rtXmlpGetNextAllElemID.c"
#undef _rtXmlpGetNextAllElemID_NAME
#undef _rtXmlpGetNextAllElemID_TYPE

/* rtXmlpGetNextAllElemID16 */
#define _rtXmlpGetNextAllElemID_NAME rtXmlpGetNextAllElemID16
#define _rtXmlpGetNextAllElemID_TYPE OSUINT16
#include "rtXmlpGetNextAllElemID.c"
#undef _rtXmlpGetNextAllElemID_NAME
#undef _rtXmlpGetNextAllElemID_TYPE

/* rtXmlpGetNextAllElemID32 */
#define _rtXmlpGetNextAllElemID_NAME rtXmlpGetNextAllElemID32
#define _rtXmlpGetNextAllElemID_TYPE OSUINT32
#include "rtXmlpGetNextAllElemID.c"
#undef _rtXmlpGetNextAllElemID_NAME
#undef _rtXmlpGetNextAllElemID_TYPE


EXTXMLMETHOD void rtXmlpForceDecodeAsGroup (OSCTXT* pctxt)
{
   XMLPREADER(pctxt)->mbDecodeAsGroup = TRUE;
}

EXTXMLMETHOD OSBOOL rtXmlpIsDecodeAsGroup (OSCTXT* pctxt)
{
   return XMLPREADER(pctxt)->mbDecodeAsGroup;
}

