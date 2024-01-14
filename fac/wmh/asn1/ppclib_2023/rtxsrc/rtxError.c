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
/*  CHANGE LOG */
/*  Date         Init    Description */
/* ////////////////////////////////////////////////////////////////////////// */
/* Run-time error utility functions */
#include <stdarg.h>
#include <stdio.h>

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

#ifndef _NO_LICENSE_CHECK
#include "rtxsrc/rtxContext.hh"
#endif

/* Error status text
   The way the formatting presently works is as follows:
      - only %s will be replaced by params added to the error.  The first param
         is substituted for the first %s and so on. The param type needn't be
         a string).
      - any remaining params are appended to the formatted text, each one
         separated by a space.
*/
#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
static const char* const g_status_text[] = {
    "Encode buffer overflow",
    "Unexpected end of buffer on decode",
    "Unexpected identifier encountered: expected = %s, parsed = %s",
    "Enumerated value %s not in defined set",
    "Duplicate element in container object (SEQUENCE, SET, etc.)",
    "Missing required element %s in container object (SEQUENCE, SET, etc.)",
    "Element index %s not within bounds of ASN.1 SET or XSD all type",
    "Max elements defined for repeating field exceeded",
    "Element with identifier %s is an invalid option in choice",
    "No dynamic memory available",
    "Invalid hex string",
    "Invalid real value",
    "Max items in sized character or binary string field exceeded",
    "Invalid value specification",
    "Nesting level too deep",
    "Value constraint violation: field %s, value %s",
    "Unexpected end of file detected",
    "Invalid UTF-8 character at index %s",
    "Array index out of bounds",
    "Invalid parameter passed to function or method",
    "Invalid value format",
    "Context is not initialized",
    "Value will not fit in target variable: element '%s'",
    "Character is not within the defined character set",
    "Invalid XML state for attempted operation",
    "Error condition returned from XML parser:\n%s",
    "Sequence elements not in correct order",
    "File not found or can't be opened",
    "Read error",
    "Write error",
    "Invalid Base64 string",
    "Socket error",
    "Attribute \"%s.%s\" not defined",
    "Regexp error: %s: %s",
    "Pattern match violation: field %s, value %s",
    "Missing required attribute \"%s\"",
    "Host name could not be resolved",
    "HTTP error: %s",
    "SOAP error. Code: %s, Reason:\n%s",
#ifdef RTEVAL
    "Evaluation license is expired",
#else
    "", /* placeholder */
#endif
    "Unexpected element \"%s\" found",
    "Invalid number of occurences of element %s (%s)",
    "Invalid message buffer",
    "Decode \"%s\" for element \"%s.%s\" failed",
    "Decode of attribute \"%s.%s\" failed",
    "Current stream must be closed before opening a new stream",
    "Unexpected null pointer value: %s",
    "General failure: %s",
    "Attribute '%s' value '%s' does not match defined fixed value",
    "Multiple errors; see context error list for details on each error",
    "Derived type could not be decoded due to missing type info",
    "Address is already in use",
    "Remote connection was reset",
    "Host was unreachable due to network or host failure",
    "Socket is not connected",
    "Connection refused",
    "Invalid socket option",
    "SOAP fault message",
    "Mark is not supported on this type of stream",
    "Feature not supported: %s",
    "Unbalanced structure detected",
    "Expected name in name/value pair but encountered value instead",
    "Invalid Unicode character encountered",
    "Invalid Boolean value keyword encountered",
    "Invalid null or nil value keyword encountered",
    "Invalid field length detected",
    "Unknown information element received, ID = %d",
    "Element expected to start on byte boundary",
    "Extraneous data exists in buffer after decode complete",
    "Invalid Message Authentication Code; expected = %s; computed = %s",
    "No security parameters were provided to decrypt/validate message",
    "Copy operation failed",
    "Unable to parse message '%s'",
    "Values not equal: element '%s', value1 '%s', value2 '%s'",
    "Buffers not equal: element '%s', diffs = '%s'",
    "Bit string value contains unused bits that have not been set to zero",
    "RLM error: %s",
    "No decoder generated for type found in encoding",
    "Socket would have blocked",
    "CDR or message file contains no data records",
    "Error returned from MBEDTLS function %d, status = %d",
    "Arithmetic operation caused integer value to overflow"
} ;

struct OSRTErrorTable {
   const char* const* ppStatusText;
   OSINT32      minErrCode;
   OSINT32      maxErrCode;
} ;

#define OSRTMAXERRTABENTRIES 5

static struct OSRTErrorTable g_error_table[OSRTMAXERRTABENTRIES];
static OSINT16 g_numErrorTableEntries = 0;

static const char g_normal_msg[] = "normal completion status";
static const OSSIZE g_normal_msglen = sizeof(g_normal_msg)-1;

static const char g_unrec_msg[] = "unrecognized completion status";
static const OSSIZE g_unrec_msglen = sizeof(g_unrec_msg)-1;
#endif /* COMPACT, ARMTCC, SYMBIAN */

/* Add error table entry to error table array */

EXTRTMETHOD OSBOOL rtxErrAddErrorTableEntry
(const char* const* ppStatusText, OSINT32 minErrCode, OSINT32 maxErrCode)
{
#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
   if (g_numErrorTableEntries < OSRTMAXERRTABENTRIES) {

      /* Make sure table entry has not already been added */
      OSINT16 i;
      for (i = 0; i < g_numErrorTableEntries; i++) {
         if (g_error_table[i].ppStatusText == ppStatusText) return FALSE;
      }

      /* Add entry */

      g_error_table[g_numErrorTableEntries].ppStatusText = ppStatusText;
      g_error_table[g_numErrorTableEntries].minErrCode = minErrCode;
      g_error_table[g_numErrorTableEntries++].maxErrCode = maxErrCode;

      return TRUE;
   }
#endif
   return FALSE;
}

EXTRTMETHOD void rtxErrInit ()
{
#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
   rtxErrAddErrorTableEntry (g_status_text, -1, -99);
#endif
}

EXTRTMETHOD OSBOOL rtxErrAddCtxtBufParm (OSCTXT* pctxt)
{
   return rtxErrAddStrnParm
      (pctxt, (const char*)OSRTBUFPTR (pctxt), OSRTBUFSIZE (pctxt));
}

EXTRTMETHOD OSBOOL rtxErrAddElemNameParm (OSCTXT* pctxt)
{
   OSBOOL res;

   if (pctxt->elemNameStack.count > 0) {
      OSUTF8CHAR* elemName = 0;
      rtxDListToUTF8Str (pctxt, &pctxt->elemNameStack, &elemName, '.');

      if (0 != elemName) {
         /* Add string parameter */
         res = rtxErrAddStrParm (pctxt, (const char*)elemName);

         /* Free temp memory */
         rtxMemFreePtr (pctxt, elemName);
      }
      else res = FALSE;
   }
   else res = rtxErrAddStrParm (pctxt, "?");

   return res;
}

/* Add an integer parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddIntParm (OSCTXT* pctxt, int errParm)
{
   char lbuf[16];
   rtxIntToCharStr (errParm, lbuf, sizeof(lbuf), 0);
   return rtxErrAddStrParm (pctxt, lbuf);
}

#if !defined(_NO_INT64_SUPPORT)
/* Add an 64-bit integer parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddInt64Parm (OSCTXT* pctxt, OSINT64 errParm)
{
   char lbuf[32];
   rtxInt64ToCharStr (errParm, lbuf, sizeof(lbuf), 0);
   return rtxErrAddStrParm (pctxt, lbuf);
}
#endif /* !defined(_NO_INT64_SUPPORT) */

/* Add a character string parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddStrParm (OSCTXT* pctxt, const char* pErrParm)
{
   OSRTErrInfo* pErrInfo = &pctxt->errInfo.reserved;
   if (pErrParm == 0) pErrParm = "(null)";
   if (pErrInfo != 0 && pErrInfo->parmcnt < OSRTMAXERRPRM) {
      OSSIZE bufsiz = OSCRTLSTRLEN(pErrParm) + 1;
      OSUTF8CHAR* tmpstr =
         rtxMemAllocArray (pctxt, bufsiz, OSUTF8CHAR);
      if (tmpstr == NULL) return FALSE;
      rtxStrcpy ((char*)tmpstr, bufsiz, pErrParm);
      pErrInfo->parms[pErrInfo->parmcnt] = tmpstr;
      pErrInfo->parmcnt++;
      return TRUE;
   }
   else
      return FALSE;
}

/* Add a character string parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddStrnParm
(OSCTXT* pctxt, const char* pErrParm, OSSIZE nchars)
{
   OSRTErrInfo* pErrInfo = &pctxt->errInfo.reserved;
   if (pErrParm == 0) pErrParm = "(null)";
   if (pErrInfo != 0 && pErrInfo->parmcnt < OSRTMAXERRPRM) {
      OSUTF8CHAR* tmpstr =
         rtxMemAllocArray (pctxt, nchars + 1, OSUTF8CHAR);
      if (tmpstr == NULL) return FALSE;
      rtxStrncpy ((char*)tmpstr, nchars + 1, pErrParm, nchars);
      pErrInfo->parms[pErrInfo->parmcnt] = tmpstr;
      pErrInfo->parmcnt++;
      return TRUE;
   }
   else
      return FALSE;
}

/* Add an unsigned integer parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddUIntParm (OSCTXT* pctxt, unsigned int errParm)
{
   char lbuf[16];
   rtxUIntToCharStr (errParm, lbuf, sizeof(lbuf), 0);
   return rtxErrAddStrParm (pctxt, lbuf);
}

#if !defined(_NO_INT64_SUPPORT)
/* Add an unsigned 64-bit integer parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddUInt64Parm (OSCTXT* pctxt, OSUINT64 errParm)
{
   char lbuf[32];
   rtxUInt64ToCharStr (errParm, lbuf, sizeof(lbuf), 0);
   return rtxErrAddStrParm (pctxt, lbuf);
}
#endif /* !defined(_NO_INT64_SUPPORT) */

/* Add a size-typed parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddSizeParm (OSCTXT* pctxt, OSSIZE errParm)
{
   char lbuf[32];
   rtxSizeToCharStr (errParm, lbuf, sizeof(lbuf), 0);
   return rtxErrAddStrParm (pctxt, lbuf);
}

/* Add a double parameter to an error message */

EXTRTMETHOD OSBOOL rtxErrAddDoubleParm (OSCTXT* pctxt, double errParm)
{
   char lbuf[200];
#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
   os_snprintf (lbuf, 200, "%E", errParm);
#else
   rtxStrncpy (lbuf, 200, "?", 1);
#endif
   return rtxErrAddStrParm (pctxt, lbuf);
}

void rtxErrFreeNodeParms (OSCTXT* pctxt, OSRTErrInfo* pErrInfo)
{
   OSUINT8 i, parmcnt;

   if (pErrInfo == 0) return;

   parmcnt = OS_MIN (pErrInfo->parmcnt, OSRTMAXERRPRM);

   for (i = 0; i < parmcnt; i++) {
      rtxMemFreeArray (pctxt, (char*)pErrInfo->parms[i]);
   }
   pErrInfo->parmcnt = 0;
   pErrInfo->status = 0;
}

/* Free error parameter memory */

EXTRTMETHOD void rtxErrFreeParms (OSCTXT* pctxt)
{
   OSRTDListNode* pNode = pctxt->errInfo.list.head;

   for ( ; pNode != 0; pNode = pNode->next) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;
      rtxMemFreePtr (pctxt, pErrInfo->elemName);
      rtxErrFreeNodeParms (pctxt, pErrInfo);
   }

   /* Free params in reserved node */
   rtxErrFreeNodeParms (pctxt, &pctxt->errInfo.reserved);
}

/* Reset error */

EXTRTMETHOD int rtxErrResetErrInfo (OSCTXT* pctxt)
{
   OSRTDListNode* pNode;
   OSRTErrInfo* pErrInfo;

   rtxErrFreeParms (pctxt);

   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      pErrInfo = (OSRTErrInfo*) pNode->data;
      if (pErrInfo != &pctxt->errInfo.reserved) {
         rtxMemFreePtr (pctxt, (void*)pErrInfo);
      }
   }
   rtxDListFreeNodes (pctxt, &pctxt->errInfo.list);

   return 0;
}

EXTRTMETHOD int rtxErrReset (OSCTXT* pctxt)
{
   /* Clear off the element name stack. It may have had things lingering
      due to an error having occured.  The names should have been literals,
      so they aren't freed. */
   rtxDListFreeNodes( pctxt, &pctxt->elemNameStack );

   rtxErrResetErrInfo (pctxt);

   /* START NOOSS */
   rtxCtxtPopAllContainers(pctxt);

   /* END NOOSS */
   return 0;
}

#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)

/* Format error message text */

static char* rtxErrFmtMsgText
(OSRTErrInfo* pErrInfo, const char* errmsg, char* bufp, OSSIZE bufsiz)
{
   const char* tp;
   OSSIZE j = 0;
   OSUINT8 pcnt = 0;

   if (pErrInfo->parmcnt > OSRTMAXERRPRM) {
      pErrInfo->parmcnt = OSRTMAXERRPRM;
   }

   /* Substitute error parameters into error message */

   tp = errmsg;

   while (*tp) {
      if (*tp == '%' && *(tp+1) == 's') {

         /* Plug in error parameter */

         if (pcnt < pErrInfo->parmcnt && pErrInfo->parms[pcnt]) {
            OSSIZE k = j + OSCRTLSTRLEN ((const char*)pErrInfo->parms[pcnt]);
            if (k < (bufsiz - 1)) {
               rtxStrcpy (&bufp[j], bufsiz-j,
                          (const char*)pErrInfo->parms[pcnt++]);
               j = k;
            }
            else break;
         }
         else
            bufp[j++] = '?';

         tp += 2;
      }
      else if (j < (bufsiz - 1)) {
         bufp[j++] = *tp++;
      }
      else break;
   }

   /* append additional params */
   while (pcnt < (OSSIZE)pErrInfo->parmcnt && pErrInfo->parms[pcnt]) {
      OSSIZE k = 1 + j + OSCRTLSTRLEN ((const char*)pErrInfo->parms[pcnt]);
      if (k < (bufsiz - 1)) {
         bufp[j++] = ' ';
         rtxStrcpy (&bufp[j], bufsiz-j,
                    (const char*)pErrInfo->parms[pcnt++]);
         j = k;
      }
      else break;
   }

   bufp[j] = '\0';        /* null terminate string */

   return bufp;
}

static const char* getStatusTextTmpl (OSRTErrInfo* pErrInfo)
{
   const char* const* ppStatusText = 0;
   OSINT16 i;

   /* Find error table */

   for (i = 0; i < g_numErrorTableEntries; i++) {
      /* Note: test is reversed because status codes are negative */
      if (pErrInfo->status <= g_error_table[i].minErrCode &&
          pErrInfo->status > g_error_table[i].maxErrCode) {
         ppStatusText = g_error_table[i].ppStatusText;
         break;
      }
   }

   /* Return error text template */

   if (0 != ppStatusText) {
      OSINT32 erridx = abs (pErrInfo->status - g_error_table[i].minErrCode);
      return ppStatusText[erridx];
   }

   return 0;
}
#endif

EXTRTMETHOD const char* rtxErrFmtMsg
(OSRTErrInfo* pErrInfo, char* bufp, OSSIZE bufsiz)
{
   if (0 == pErrInfo || 0 == bufp || 0 == bufsiz) return 0;

#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
   if (pErrInfo->status == RTERR_MULTIPLE) {
      /* no status text should be returned for multiple error indicator */
   }
   else if (pErrInfo->status < 0)
   {
      const char* pErrMsgText = getStatusTextTmpl (pErrInfo);

      if (0 != pErrMsgText) {
         rtxErrFmtMsgText (pErrInfo, pErrMsgText, bufp, bufsiz);
      }
      else
         rtxStrcpy (bufp, bufsiz, g_unrec_msg);
   }
   else rtxStrcpy (bufp, bufsiz, g_normal_msg);
#endif

   return (bufp);
}

static OSSIZE calcErrMsgLen (OSRTErrInfo* pErrInfo)
{
#if !defined(_ARMTCC) && !defined(_COMPACT) && !defined(__SYMBIAN32__)
   const char* pErrMsgText = getStatusTextTmpl (pErrInfo);
   OSSIZE i, len = 0;

   if (0 == pErrMsgText)
      return (pErrInfo->status < 0) ? g_unrec_msglen : g_normal_msglen;

   if (pErrInfo->parmcnt > OSRTMAXERRPRM) {
      pErrInfo->parmcnt = OSRTMAXERRPRM;
   }

   /* Buffer size is approximately length of message + length of parameters
      (this is a little over because it does not account for format specifiers
      being replaced with actual text, but for a buffer size calculation, a
      little over is OK).. */

   for (i = 0; i < pErrInfo->parmcnt; i++) {
      if (0 != pErrInfo->parms[i]) {
         len += OSCRTLSTRLEN ((const char*)pErrInfo->parms[i]);
      }
   }

   return (OSCRTLSTRLEN(pErrMsgText) + len + 1);
#else
   return 0;
#endif
}

EXTRTMETHOD char* rtxErrGetMsgText (OSCTXT* pctxt)
{
   static const char elemNameLabel[] = "Element: ";
   static const OSSIZE elemNameLabelLen = sizeof(elemNameLabel) - 1;
   OSSIZE newLineLen = OSCRTLSTRLEN("\n");
   OSSIZE bufsiz = 0, idx = 0;
   OSRTDListNode* pNode;
   OSRTErrInfo* pErrInfo;
   char* pstrbuf;

   /* Calculate buffer size */
   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      pErrInfo = (OSRTErrInfo*)pNode->data;
      if (0 != pErrInfo->elemName) {
         bufsiz += OSCRTLSTRLEN((const char*)pErrInfo->elemName) +
            elemNameLabelLen + newLineLen;
      }
      bufsiz += calcErrMsgLen (pErrInfo);
      if (0 != pNode->next) bufsiz += 2; /* padding for newline */
   }

   /* Allocate memory */
   pstrbuf = (char*) rtxMemAlloc (pctxt, bufsiz);
   if (0 == pstrbuf) return 0;

   /* Format error message(s).  If multiple, message are separated by
      newline character. */
   *pstrbuf = '\0';
   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      pErrInfo = (OSRTErrInfo*)pNode->data;

      /* Add element name if exists */
      if (0 != pErrInfo->elemName) {
         rtxStrncat (pstrbuf, bufsiz, elemNameLabel, elemNameLabelLen);
         rtxStrcat (pstrbuf, bufsiz, (const char*)pErrInfo->elemName);
         if (!rtxStrncat (pstrbuf, bufsiz, "\n", newLineLen)) break;
      }

      idx = OSCRTLSTRLEN(pstrbuf);
      rtxErrFmtMsg (pErrInfo, &pstrbuf[idx], bufsiz - idx);
      if (0 != pNode->next) {
         rtxStrncat (pstrbuf, bufsiz, "\n", newLineLen);
      }
   }

   return pstrbuf;
}

/* Get error text in a dynamic or static memory buffer.  If pBuf and    */
/* pBufSize arguments are non-null, it is assumed a static buffer is    */
/* provided and this will be used.  Otherwise, this allocates memory    */
/* using the 'rtxMemAlloc' function.  This memory is automatically      */
/* freed at the time the 'rtxMemFree' function is called.               */
/* The calling function should free the memory.                         */

EXTRTMETHOD char* rtxErrGetText (OSCTXT* pctxt, char* pBuf, OSSIZE* pBufSize)
{
   OSSIZE bufsiz;
   OSRTDListNode* pNode;
#ifndef _NO_LICENSE_CHECK
   char* lstatmsg = LGETSTATMSG (pctxt);
#endif
   if (pBuf == 0 || pBufSize == 0) {
      /* calculate buffer size */
      bufsiz = 200;
      for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
         OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;
         bufsiz += 100 * (2 + pErrInfo->stkx) * sizeof(char);
         if (0 != pErrInfo->elemName) {
            bufsiz += rtxUTF8LenBytes (pErrInfo->elemName) + 20;
         }
      }
#ifndef _NO_LICENSE_CHECK
      if (0 != lstatmsg) {
         bufsiz += OSCRTLSTRLEN(lstatmsg) + 2;
      }
#endif
      pBuf = rtxMemAllocArray (pctxt, bufsiz, char);
      if (pBuf == NULL) return NULL;
      if (pBufSize != 0) *pBufSize = bufsiz;
   }
   else
      bufsiz = *pBufSize;

   rtxErrGetTextBuf (pctxt, pBuf, bufsiz);

#ifndef _NO_LICENSE_CHECK
   if (0 != lstatmsg) {
      rtxStrcat (pBuf, bufsiz, lstatmsg);
      rtxStrcat (pBuf, bufsiz, "\n");
      rtxMemFreePtr (pctxt, lstatmsg);
   }
#endif
   return pBuf;
}

/* Get error text in static buffer */
EXTRTMETHOD char* rtxErrGetTextBuf (OSCTXT* pctxt, char* pbuf, OSSIZE bufsiz)
{
   char lbuf[200] = {'\0'}, numbuf[32];
   OSRTDListNode* pNode;

   if (pbuf == 0 || bufsiz == 0) return 0;

   *pbuf = '\0';

   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;

      rtxStrcat (pbuf, bufsiz, "ERROR: Status ");
      rtxIntToCharStr (pErrInfo->status, numbuf, sizeof(numbuf), 0);
      rtxStrcat (pbuf, bufsiz, numbuf);
      if (!rtxStrcat (pbuf, bufsiz, "\n")) break;

      rtxStrcat (pbuf, bufsiz, rtxErrFmtMsg (pErrInfo, lbuf, sizeof(lbuf)));
      if (!rtxStrcat (pbuf, bufsiz, "\n")) break;
#ifndef _COMPACT
      {
      OSUINT8 stkx = pErrInfo->stkx;

      if (0 != pErrInfo->elemName) {
         rtxStrcat (pbuf, bufsiz, "Element name: ");
         rtxStrcat (pbuf, bufsiz, (const char*)pErrInfo->elemName);
         if (!rtxStrcat (pbuf, bufsiz, "\n")) break;
      }
      if (stkx > 0) {
         if (!rtxStrcat (pbuf, bufsiz, "Stack trace:\n")) break;
      }

      while (stkx > 0) {
         stkx--;
         if (0 != pErrInfo->stack[stkx].module) {
            rtxStrcat (pbuf, bufsiz, "  Module: ");
            rtxStrcat (pbuf, bufsiz, (const char*)pErrInfo->stack[stkx].module);
            rtxStrcat (pbuf, bufsiz, " Line ");
            rtxIntToCharStr
               (pErrInfo->stack[stkx].lineno, numbuf, sizeof(numbuf), 0);
            rtxStrcat (pbuf, bufsiz, numbuf);
            if (!rtxStrcat (pbuf, bufsiz, "\n")) break;
         }
      }}
#endif
   }

   return pbuf;
}

/* Get error text in static buffer.  This version get the message text only.  */
EXTRTMETHOD char* rtxErrGetMsgTextBuf (OSCTXT* pctxt, char* pbuf, OSSIZE bufsiz)
{
   char lbuf[200];
   OSRTDListNode* pNode;

   if (pbuf == 0 || bufsiz == 0) return 0;

   *pbuf = '\0';

   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;

      rtxStrcat (pbuf, bufsiz, rtxErrFmtMsg (pErrInfo, lbuf, sizeof(lbuf)));
      if (!rtxStrcat (pbuf, bufsiz, "\n")) break;
   }

   return pbuf;
}

/* Log error using given callback function */

EXTRTMETHOD void rtxErrLogUsingCB (OSCTXT* pctxt, OSErrCbFunc cb, void* cbArg_p)
{
   const char* pBuf = rtxErrGetText (pctxt, 0, 0);

   if (pBuf == NULL) {
      pBuf = "ERROR : no memory available";
   }
   (*cb) (pBuf, cbArg_p);

   rtxErrFreeParms (pctxt);
}

/* Print error information to the standard output */

EXTRTMETHOD void rtxErrPrintElement (OSRTErrInfo* pErrInfo)
{
   char lbuf[400] = { '\0' };

   printf ("ERROR: Status %d\n", pErrInfo->status);
   printf ("%s\n", rtxErrFmtMsg (pErrInfo, lbuf, sizeof(lbuf)));
#ifndef _COMPACT
   {
   OSUINT8 stkx = pErrInfo->stkx;
   if (0 != pErrInfo->elemName) {
      printf ("Element name: %s\n", pErrInfo->elemName);
   }
   if (stkx > 0) printf ("Stack trace:");
   while (stkx > 0) {
      stkx--;

      /* we may have some errors pushed onto the stack that are null if debug
       * output is preserved in some parts of the code and not others */
      if (0 != pErrInfo->stack[stkx].module) {
         printf ("  Module: %s, Line %d\n",
                 pErrInfo->stack[stkx].module,
                 pErrInfo->stack[stkx].lineno);
      }
   }}
#endif
}

EXTRTMETHOD void rtxErrPrint (OSCTXT* pctxt)
{
   OSRTDListNode* pNode;

   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;
      rtxErrPrintElement (pErrInfo);
   }
#ifndef _NO_LICENSE_CHECK
   LPRINTSTAT (pctxt);
#endif
}

/* Set error data */

EXTRTMETHOD int rtxErrSetData
(OSCTXT* pctxt, int status, const char* module, int lineno)
{
   OSRTErrInfo* pErrInfo;
   // For TOOBIG error, always add element name to error output...
   if (RTERR_TOOBIG == status)
   {
      rtxErrAddElemNameParm(pctxt);
   }

   pErrInfo = OSRT_GET_LAST_ERROR_INFO (pctxt);
   if (pErrInfo == 0) {
      return rtxErrSetNewData (pctxt, status, module, lineno);
   }
   if (pErrInfo->status == 0) {
      pErrInfo->status = (OSINT16)status;
   }
   else if (pErrInfo->status != status) {
      /* Assume this is a new error.  Note that this test is not sufficient
         to cover all cases.  A new error with the same status value as the
         last error may be being logged, but this function in its current
         form will think information is being added to the last error. */
      pErrInfo = rtxErrNewNode (pctxt);
      if (0 == pErrInfo) return RTERR_NOMEM;
      pErrInfo->status = (OSINT16)status;
   }
#ifndef _COMPACT
   /* copy parameters from reserved */
   if ((pErrInfo != &pctxt->errInfo.reserved)
         && (pctxt->errInfo.reserved.parmcnt != 0)) {
      OSUINT8 i, parmcnt =
         OS_MIN (pctxt->errInfo.reserved.parmcnt, OSRTMAXERRPRM);
      for (i = 0; i < parmcnt; i++) {
         pErrInfo->parms[i] = pctxt->errInfo.reserved.parms[i];
         pctxt->errInfo.reserved.parms[i] = 0;
      }
      pErrInfo->parmcnt = parmcnt;
      pctxt->errInfo.reserved.parmcnt = 0;
   }
   if (0 == (pctxt->flags & OSRESERVED1)) {
      if (pErrInfo->stkx < OSRTERRSTKSIZ) {
         if (! OS_ISEMPTY(module)) {
            pErrInfo->stack[pErrInfo->stkx].module = (const OSUTF8CHAR*)module;
            pErrInfo->stack[pErrInfo->stkx++].lineno = lineno;
         }
      }

      /* If element name is present, copy it to the error structure */
      if (OS_ISEMPTY (pErrInfo->elemName)) {
         rtxDListToUTF8Str
            (pctxt, &pctxt->elemNameStack, &pErrInfo->elemName, '.');
      }
   }
#endif
   return rtxErrGetStatus (pctxt);
}

EXTRTMETHOD int rtxErrSetNewData
(OSCTXT* pctxt, int status, const char* module, int lineno)
{
   OSRTErrInfo* pErrInfo;

   /* create new error info node */

   pErrInfo = rtxErrNewNode (pctxt);
   if (pErrInfo == 0) {

      /* use reseved OSRTErrInfo element */

      if (pctxt->errInfo.reserved.status == 0 && status == RTERR_NOMEM) {
         pErrInfo = &pctxt->errInfo.reserved;
         pctxt->errInfo.reservedNode.data = pErrInfo;
         rtxDListAppendNode (&pctxt->errInfo.list,
            &pctxt->errInfo.reservedNode);
      }
      else
         return RTERR_NOMEM;
   }
   else {
      pctxt->errInfo.reserved.status = 0; /* clear reserved errInfo */
   }
   return rtxErrSetData (pctxt, status, module, lineno);
}

EXTRTMETHOD OSRTErrInfo* rtxErrNewNode (OSCTXT* pctxt)
{
   OSRTErrInfo* pErrInfo = rtxMemAllocTypeZ (pctxt, OSRTErrInfo);
   if (pErrInfo != 0) {
      pErrInfo->fatal = TRUE;
      if (0 == rtxDListAppend (pctxt, &pctxt->errInfo.list, pErrInfo)) {
         rtxMemFreePtr (pctxt, pErrInfo);
         return 0;
      }
   }
   return pErrInfo;
}

EXTRTMETHOD void rtxErrSetNonFatal(OSCTXT* pctxt)
{
   if (pctxt->errInfo.list.tail != 0)
   {
      ((OSRTErrInfo*)(pctxt->errInfo.list.tail->data))->fatal = FALSE;
   }
}

EXTRTMETHOD int rtxErrCheckNonFatal(OSCTXT* pctxt)
{
   int stat = 0;
   if (pctxt->errInfo.list.count != 0) {
      OSRTDListNode* pNode;
      OSRTErrInfo* pErrInfo;
      int firstStatus = 0;
      for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
         pErrInfo = (OSRTErrInfo*)pNode->data;
         stat = pErrInfo->status;
         if (firstStatus == 0) firstStatus = pErrInfo->status;
         if (pErrInfo->status != firstStatus) {
            stat = RTERR_MULTIPLE;
            break;
         }
      }
   }
   return stat;
}

EXTRTMETHOD void rtxErrAssertionFailed
(const char* conditionText, int lineNo, const char* fileName)
{
   printf ("Run-time assertion '%s' failed.  Line %d, File %s\n",
           conditionText, lineNo, fileName);
#ifndef _ARMTCC
   exit (-1);
#endif
}

EXTRTMETHOD int rtxErrGetFirstError (const OSCTXT* pctxt)
{
   OSRTErrInfo* pErrInfo = OSRT_GET_FIRST_ERROR_INFO (pctxt);
   if (pErrInfo != 0)
      return pErrInfo->status;
   return 0;
}

EXTRTMETHOD int rtxErrGetLastError (const OSCTXT* pctxt)
{
   OSRTErrInfo* pErrInfo = OSRT_GET_LAST_ERROR_INFO (pctxt);
   return (pErrInfo != 0) ? pErrInfo->status : 0;
}

EXTRTMETHOD OSSIZE rtxErrGetErrorCnt (const OSCTXT* pctxt)
{
   return pctxt->errInfo.list.count;
}

EXTRTMETHOD int rtxErrGetStatus (const OSCTXT* pctxt)
{

   OSRTDListNode* pNode;
   int retval = 0;

   if ( pctxt->errInfo.list.count == 0 ) return 0;

   for (pNode = pctxt->errInfo.list.head; pNode != 0; pNode = pNode->next) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;
      if (pErrInfo->fatal)
      {
         if (retval == 0 ) retval = pErrInfo->status; /* first fatal found */
         else return RTERR_MULTIPLE;   /* 2nd fatal found */
      }
      /* else: ignore non-fatal error */
   }

   return retval;
}

EXTRTMETHOD int rtxErrResetLastErrors (OSCTXT* pctxt, OSSIZE errorsToReset)
{
   OSRTDListNode* pNode = pctxt->errInfo.list.tail, *pPrevNode;

   for (;errorsToReset > 0 && pNode != 0; pNode = pPrevNode, errorsToReset--) {
      OSRTErrInfo* pErrInfo = (OSRTErrInfo*)pNode->data;
      pPrevNode = pNode->prev;

      rtxErrFreeNodeParms (pctxt, pErrInfo);

      if (pErrInfo != &pctxt->errInfo.reserved) {
         rtxDListFreeNode (pctxt, &pctxt->errInfo.list, pNode);
      }
      else { /* just remove, but not free */
         rtxDListRemove (&pctxt->errInfo.list, pNode);
      }
   }
   return 0;
}

static void copyErrInfo
(OSCTXT* pctxt, OSRTErrInfo* pDest, const OSRTErrInfo* pSrc)
{
   OSUINT8 i, parmcnt = OS_MIN (pSrc->parmcnt, OSRTMAXERRPRM);
   for (i = 0; i < OSRTERRSTKSIZ; i++) {
      pDest->stack[i] = pSrc->stack[i];
   }
   pDest->status = pSrc->status;
   pDest->stkx = pSrc->stkx;
   pDest->parmcnt = parmcnt;
   for (i = 0; i < parmcnt; i++) {
      pDest->parms[i] = rtxUTF8Strdup (pctxt, pSrc->parms[i]);
   }
   pDest->elemName = (0 != pSrc->elemName) ?
      rtxUTF8Strdup (pctxt, pSrc->elemName) : 0;
}

EXTRTMETHOD int rtxErrCopy (OSCTXT* pDestCtxt, const OSCTXT* pSrcCtxt)
{
   int ret = 0;

   if (pDestCtxt != pSrcCtxt) {
      /* Reset any errors that may exist in the destination context */
      rtxErrReset (pDestCtxt);

      /* Copy info from reserved */
      copyErrInfo (pDestCtxt,
                   &pDestCtxt->errInfo.reserved, &pSrcCtxt->errInfo.reserved);

      /* Copy error info from the nodes in the error list */
      ret = rtxErrAppend (pDestCtxt, pSrcCtxt);
      if (0 != ret) {
         rtxErrReset (pDestCtxt);
      }
   }

   return ret;
}

EXTRTMETHOD int rtxErrAppend (OSCTXT* pDestCtxt, const OSCTXT* pSrcCtxt)
{
   OSRTDListNode* pnode;
   OSRTErrInfo*   pErrInfo;

   /* Copy error info from the nodes in the error list */
   pnode = pSrcCtxt->errInfo.list.head;
   while (0 != pnode) {
      pErrInfo = rtxMemAllocType (pDestCtxt, OSRTErrInfo);
      if (0 == pErrInfo) {
         return RTERR_NOMEM;
      }
      copyErrInfo (pDestCtxt, pErrInfo, (OSRTErrInfo*)pnode->data);

      rtxDListAppend (pDestCtxt, &pDestCtxt->errInfo.list, pErrInfo);

      pnode = pnode->next;
   }

   return 0;
}

int rtxErrInvUIntOpt (OSCTXT* pctxt, OSUINT32 ident)
{
   char numbuf[40];
   rtxUIntToCharStr (ident, numbuf, sizeof(numbuf), 0);
   rtxErrAddStrParm (pctxt, numbuf);
   return RTERR_INVOPT;
}
