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
#include "rtxmlsrc/osrtxml.hh"

/**
 * This function adds the given amount of indentation to the output
 * stream.
 */
EXTXMLMETHOD int rtXmlEncIndent (OSCTXT* pctxt)
{
   register int i, stat, nspaces;
   OSXMLCtxtInfo* ctxtInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (ctxtInfo == NULL) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }

   /* if canonical XER, do not add whitespace to the encoding */
   if ( rtxCtxtTestFlag(pctxt, OSXMLC14N ) &&
      rtxCtxtTestFlag(pctxt, OSASN1XER ) ) return 0;

   if (ctxtInfo->indent > 0) {
      char indentChar = (char)ctxtInfo->indentChar;

      nspaces = pctxt->level * ctxtInfo->indent;
      /* Verify indentation whitespace will fit in encode buffer */

      stat = rtXmlCheckBuffer (pctxt, nspaces + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Copy data to buffer */

      if (pctxt->state != OSXMLINIT)
         stat = rtXmlPutChar (pctxt, '\n');

      for (i = 0; i < nspaces; i++) {
         stat = rtXmlPutChar (pctxt, indentChar);
      }
   }
   return 0;
}

