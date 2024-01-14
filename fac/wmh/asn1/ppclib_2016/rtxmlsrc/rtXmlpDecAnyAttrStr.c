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

#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlPull.hh"

EXTXMLMETHOD int rtXmlpDecAnyAttrStr (OSCTXT* pctxt, const OSUTF8CHAR** ppAttrStr,
   size_t idx)
{
   struct OSXMLReader* pXmlReader;
   const OSXMLAttrOffset* pAttrOff;
   size_t newsize = 0;
   size_t markedPos;
   OSUTF8CHAR* pNewData;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   pAttrOff = rtXmlRdGetAttribute (pXmlReader, idx);
   OSRTASSERT (pAttrOff != 0);

   /* any attribute always OK as anyAttribute */
   if (!ppAttrStr)
      return 0;

   *ppAttrStr = 0;

   newsize = pAttrOff->mValue.offset - pAttrOff->mQName.offset +
      pAttrOff->mValue.length + 1;

   pNewData = (OSUTF8CHAR*) rtxMemAlloc (pctxt, newsize+1);

   if (pNewData == 0) {
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   }

   if (pXmlReader->mMarkedPos != (size_t)-1)
      markedPos = pXmlReader->mMarkedPos;
   else
      markedPos = 0;

   OSCRTLSAFEMEMCPY (pNewData, newsize+1,
      pXmlReader->mpBuffer + markedPos + pAttrOff->mQName.offset, newsize);

   pNewData[newsize] = 0;

   *ppAttrStr = pNewData;

   return 0;
}

