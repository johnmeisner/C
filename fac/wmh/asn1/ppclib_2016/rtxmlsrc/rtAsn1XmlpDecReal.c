/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

/* #include "rtsrc/asn1type.h" */
/* #include "rtxsrc/rtxCtype.h" */
#include "rtxsrc/rtxReal.h"
#include "rtxmlsrc/asn1xml.h"
#include "rtxmlsrc/rtXmlPull.h"
#include "rtxmlsrc/osrtxml.h"

static OSUTF8CHAR REAL_POS_INFINITY[] =
{'P','L','U','S','-','I','N','F','I','N','I','T','Y'};
static OSUTF8CHAR REAL_NEG_INFINITY[] =
{'M','I','N','U','S','-','I','N','F','I','N','I','T','Y'};
static OSUTF8CHAR REAL_NAN[] =
{'N','O','T','-','A','-','N','U','M','B','E','R'};

EXTXMLMETHOD int rtAsn1XmlpDecReal (OSCTXT* pctxt, OSREAL* pvalue)
{
   /* The content model is mixed.  It should be either an empty */
   /* element corresponding to one of the special values or else text */
   /* corresponding to a regular real. */

   OSXMLReader* pXmlReader = rtXmlpGetReader(pctxt);
   int stat = 0;
   OSXMLDataCtxt dataCtxt;
   OSXMLEvent xmlEvent;

   /* Use rtXmlRdFirstData to locate the first chunk of text (with */
   /* collapsing whitespace).  If it returns 0, there is no non-whitespace text */
   /* before the end tag or the next start tag.  This means the value is */
   /* either missing or else we possibly have an element to decode. */

   /* If the encoded value is text, then we should currently be on */
   /* a start element, and rtXmlpDecDoubleExt will expect this, so */
   /* we mark our position first. */
   rtXmlRdMarkPos(pXmlReader);

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_COLLAPSE);

   /* if rtXmlRdFirstData returns 0, then it may have found an empty */
   /* TEXT (whitespace collapsed to empty), or start or end tag. */
   /* If we found empty text, read next data. Keep doing this until we */
   /* end up on a start or end tag. */
   stat = rtXmlRdFirstData (pXmlReader, &dataCtxt);
   xmlEvent = rtXmlRdGetLastEvent(pXmlReader);

   while ( stat == 0 && xmlEvent.mId != OSXMLEVT_START_TAG &&
      xmlEvent.mId != OSXMLEVT_END_TAG)
   {
      stat = rtXmlRdNextData (pXmlReader, &dataCtxt);
      xmlEvent = rtXmlRdGetLastEvent(pXmlReader);
   }

   if (stat == 0) {
      /* no text found.  xmlEvent is either start or end tag. */
      OSXMLStrFragment localName;
      OSINT16 nsIdx;

      if (xmlEvent.mId != OSXMLEVT_START_TAG ) {
         /* value missing */
         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
      }

      /* start element event */
      stat = rtXmlRdGetTagName(pXmlReader, &localName, &nsIdx);
      if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

      if ( nsIdx != 0 ) {
         /* namespace should be empty */
         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         return LOG_RTERRNEW (pctxt, RTERR_INVREAL);
      }
      else if ( localName.length == sizeof(REAL_POS_INFINITY) &&
         rtxUTF8StrnEqual(localName.value, REAL_POS_INFINITY,
           localName.length) )
      {
         *pvalue = rtxGetPlusInfinity();
      }
      else if ( localName.length == sizeof(REAL_NEG_INFINITY) &&
         rtxUTF8StrnEqual(localName.value, REAL_NEG_INFINITY,
         localName.length) )
      {
         *pvalue = rtxGetMinusInfinity();
      }
      else if ( localName.length == sizeof(REAL_NAN) &&
         rtxUTF8StrnEqual(localName.value, REAL_NAN,
         localName.length) )
      {
         *pvalue = rtxGetNaN();
      }
      else {
         /* wrong element */
         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
         return LOG_RTERRNEW (pctxt, RTERR_UNEXPELEM);
      }

      /* if we reach here, we matched one of the special value empty elements */

      rtXmlRdMarkLastEventDone (pXmlReader);

      stat = rtXmlDecEmptyElement(pctxt);
      if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

      stat = rtXmlpMatchEndTag(pctxt, -1);
      if ( stat != 0 ) return LOG_RTERR(pctxt, stat);

      return 0;
   }
   else  { /* TEXT event */
      /* rewind.  rtXmlpDecDoubleExt expects us to be at the start tag */
      /* of the element containing the TEXT */
      rtXmlRdRewindToMarkedPos(pXmlReader);
      return rtXmlpDecDoubleExt(pctxt, OSXMLREALENC_BXER, pvalue);
   }
}
