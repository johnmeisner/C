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

/*  This file to be included by rtXmlpGetNextElemID.c */
/*  Including file should #define the following: */
/*  _rtXmlpGetNextAllElemID_NAME The name of the method to be defined */
/*  _rtXmlpGetNextAllElemID_TYPE The integer type for the pOrder argument. */

EXTXMLMETHOD int _rtXmlpGetNextAllElemID_NAME
(OSCTXT* pctxt, const OSXMLElemIDRec* tab, size_t nrows,
   const _rtXmlpGetNextAllElemID_TYPE* pOrder, OSSIZE nOrder,
   OSSIZE maxOrder, int anyID)
{
   int elemID = rtXmlpGetNextElemID
      (pctxt, tab, nrows, -1, (OSBOOL)(anyID < 0));

   if (nOrder == maxOrder && elemID >= 0 && elemID != XML_OK_EOB)
      elemID = RTERR_UNEXPELEM;

   if (elemID == RTERR_UNEXPELEM) {
      if (anyID >= 0) elemID = anyID;
   }

   if (elemID == XML_OK_EOB) {
      return elemID;
   }

   /* check duplicate elements */
   if (elemID >= 0) {
      const _rtXmlpGetNextAllElemID_TYPE* p = pOrder;
      const _rtXmlpGetNextAllElemID_TYPE* pEnd = pOrder + nOrder;

      while (p != pEnd) {
         if (*p++ == (_rtXmlpGetNextAllElemID_TYPE)elemID)
            return LOG_RTERR (pctxt, RTERR_SETDUPL);
      }
   }

   if (elemID < 0) {
      if (elemID == RTERR_UNEXPELEM) {
         OSXMLStrFragment localName;
         struct OSXMLReader* pXmlReader;

         OSRTASSERT (0 != pctxt->pXMLInfo);
         pXmlReader = rtXmlpGetReader (pctxt);

         rtXmlRdGetTagName (pXmlReader, &localName, 0);

         rtxErrAddStrnParm
            (pctxt, (const char*)localName.value, localName.length);
         rtXmlRdErrAddSrcPos (pXmlReader, FALSE);
      }
      return LOG_RTERR (pctxt, elemID);
   }

   return elemID;
}
