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

#include "osrtxml.h"

EXTXMLMETHOD int rtXmlEncComment (OSCTXT* pctxt, const OSUTF8CHAR* comment)
{
   static const OSUTF8CHAR startComment[] = "<!-- ";
   static const size_t startCommentSize = sizeof(startComment) - 1;
   static const OSUTF8CHAR endComment[] = " -->";
   static const size_t endCommentSize = sizeof(endComment) - 1;
   int stat;

   stat = rtXmlEncIndent (pctxt);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, startComment, startCommentSize);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, comment, OSUTF8LEN(comment));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   stat = rtXmlWriteChars (pctxt, endComment, endCommentSize);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = OSXMLCOMMENT;

   return stat;
}
