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

#include "rtxmlsrc/rtSaxCXmlHandler.h"
#include "rtxsrc/rtxFile.h"

static void indent (FILE* fptr, OSUINT32 nspaces)
{
   OSUINT32 i;
   for (i = 0; i < nspaces; i++) fprintf (fptr, " ");
}

EXTXMLMETHOD int SAX2XMLInitUserData
(OSCTXT* pctxt, SAX2XMLUserData* pUserData, const char* outFileName)
{
   pUserData->pctxt = pctxt;
   if (0 != outFileName) {
      int stat = rtxFileOpen (&pUserData->outfile, outFileName, "w");
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   pUserData->xmlState = OSXMLINIT;
   pUserData->indentSpaces = 0;
   return 0;
}

EXTXMLMETHOD int SAX2XMLStartElement
(void *userData, const OSUTF8CHAR* localname,
 const OSUTF8CHAR* qname, const OSUTF8CHAR* const* attrs)
{
   SAX2XMLUserData* pSAX2XML = (SAX2XMLUserData*)userData;

   if (pSAX2XML->xmlState == OSXMLINIT) {
      fprintf (pSAX2XML->outfile,
               "<?xml version=\"1.0\" encoding=\"%s\"?>\n", OSXMLHDRUTF8);
   }
   else if (pSAX2XML->xmlState == OSXMLSTART) {
      fprintf (pSAX2XML->outfile, ">\n");
   }
   if (pSAX2XML->xmlState != OSXMLDATA) {
      indent (pSAX2XML->outfile, pSAX2XML->indentSpaces);
   }
   fprintf (pSAX2XML->outfile, "<%s", qname);
   if (0 != attrs && 0 != attrs[0]) {
      OSUINT32 i = 0;
      while (0 != attrs[i]) {
         fprintf (pSAX2XML->outfile, " %s=\"%s\"", attrs[i], attrs[i+1]);
         i += 2;
      }
   }

   pSAX2XML->indentSpaces += 3;
   pSAX2XML->xmlState = OSXMLSTART;
   if (0 != localname) {} /* to suppress level 4 warning */

   return 0;
}

EXTXMLMETHOD int SAX2XMLCharacters (void *userData, const OSUTF8CHAR* chars, int length)
{
   int i;
   SAX2XMLUserData* pSAX2XML = (SAX2XMLUserData*)userData;
   if (pSAX2XML->xmlState == OSXMLSTART) {
      fprintf (pSAX2XML->outfile, ">");
   }
   for (i = 0; i < length; i++) fputc (chars[i], pSAX2XML->outfile);
   pSAX2XML->xmlState = OSXMLDATA;
   return 0;
}

EXTXMLMETHOD int SAX2XMLEndElement
(void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   SAX2XMLUserData* pSAX2XML = (SAX2XMLUserData*)userData;
   pSAX2XML->indentSpaces -= 3;
   if (pSAX2XML->xmlState == OSXMLSTART) {
      fprintf (pSAX2XML->outfile, "/>\n");
   }
   else {
      if (pSAX2XML->xmlState == OSXMLEND) {
         indent (pSAX2XML->outfile, pSAX2XML->indentSpaces);
      }
      fprintf (pSAX2XML->outfile, "</%s>\n", qname);
   }
   pSAX2XML->xmlState = OSXMLEND;
   if (0 != localname) {} /* to suppress level 4 warning */
   return 0;
}

