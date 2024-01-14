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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxPattern.h"
#include "rtxsrc/rtxError.h"
#ifndef _OS_NOPATTERN
#include "rtxsrc/rtxRegExp.h"
#endif

EXTRTMETHOD void rtxFreeRegexpCache(OSCTXT* pctxt) {
#ifndef _OS_NOPATTERN
   rtxRegexpPtr pRegExp = 0;
   OSRTDListNode* curNode = pctxt->regExpCache.head;
   OSRTDListNode* removedNode;

   /* find compiled regexp */
   while (curNode) {
      pRegExp = ((rtxRegexpPtr*)curNode->data)[1];

      /* Free the memory associated with pRegExp and pRegExp itself */
      rtxRegFreeRegexp(pctxt, pRegExp);

      /* Remove the node from the list */
      rtxDListRemove (&(pctxt->regExpCache), curNode);
      removedNode = curNode;
      curNode = curNode->next;

      /* Now free the removed node */
      rtxMemSysFreePtr(pctxt, removedNode);
   }
#else
   LOG_RTERR (pctxt, RTERR_NOTSUPP);
#endif
}

EXTRTMETHOD OSBOOL rtxMatchPattern (OSCTXT* pctxt,
                        const OSUTF8CHAR* text,
                        const OSUTF8CHAR* pattern)
{
#ifndef _OS_NOPATTERN
   rtxRegexpPtr pRegExp = 0;
   OSRTDListNode* curNode = pctxt->regExpCache.head;

   /* find compiled regexp */
   while (curNode) {
      /* compare pointers, will be work only with constant strings */
      if (*(const OSUTF8CHAR**)curNode->data == pattern) {
         pRegExp = ((rtxRegexpPtr*)curNode->data)[1];
         /* move regexp to start of list */
         rtxDListRemove (&(pctxt->regExpCache), curNode);
         rtxDListInsertNode (&(pctxt->regExpCache), 0, curNode);
         break;
      }
      curNode = curNode->next;
   }

   if (pRegExp == 0) {
      pRegExp = rtxRegexpCompile (pctxt, pattern);

      curNode = (OSRTDListNode*) rtxMemSysAllocZ
         (pctxt, sizeof (OSRTDListNode) + 2 * sizeof (void*));

      if (0 != curNode) {
         curNode->data = curNode + 1;
         *(const OSUTF8CHAR**)curNode->data = pattern;
         ((rtxRegexpPtr*)curNode->data)[1] = pRegExp;
         rtxDListInsertNode (&(pctxt->regExpCache), 0, curNode);
      }
   }

   if (0 != pRegExp) {
      int stat = rtxRegexpExec (pctxt, pRegExp, text);

      RTDIAG4 (pctxt, "rtxMatchPattern: text='%s' pat='%s' stat=%d\n",
               text, pattern, stat);
#ifdef _TRACE
      if (rtxDiagEnabled (pctxt)) {
         rtxRegexpPrint (stdout, pRegExp);
      }
#endif

      return (OSBOOL)((1 == stat) ? TRUE : FALSE);
   }
#else
   LOG_RTERR (pctxt, RTERR_NOTSUPP);
#endif
   return FALSE;
}

EXTRTMETHOD OSBOOL rtxMatchPattern2 (OSCTXT* pctxt,
                         const OSUTF8CHAR* pattern)
{
#ifndef _OS_NOPATTERN
   OSUTF8CHAR buf[128], *text;
   OSBOOL rv;

   if (OSRTBUFSIZE (pctxt) < sizeof (buf)) {
      text = buf;
      OSCRTLSAFEMEMCPY (text, sizeof(buf), OSRTBUFPTR(pctxt), OSRTBUFSIZE (pctxt));
      text [OSRTBUFSIZE (pctxt)] = 0;
   }
   else {
      text = rtxRegexpStrndup
         (pctxt, (const OSUTF8CHAR*)OSRTBUFPTR(pctxt), OSRTBUFSIZE (pctxt));
   }

   rv = rtxMatchPattern (pctxt, text, pattern);

   if (text != buf)
      rtxMemSysFreePtr (pctxt, text);

   return rv;
#else
   LOG_RTERR (pctxt, RTERR_NOTSUPP);
   return FALSE;
#endif
}
