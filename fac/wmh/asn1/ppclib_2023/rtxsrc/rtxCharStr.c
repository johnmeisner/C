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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxErrCodes.h"

EXTRTMETHOD int rtxStricmp (const char* str1, const char* str2)
{
   if (0 != str1 && 0 != str2) {
      size_t len = OSCRTLSTRLEN(str1);
      if (len == OSCRTLSTRLEN(str2)) {
         size_t i;
         for (i = 0; i < len; i++) {
            if (OS_TOLOWER(str1[i]) != OS_TOLOWER(str2[i])) return 1;
         }
         return 0;
      }
      else return 1;
   }
   return -1;
}


EXTRTMETHOD int rtxStrnicmp (const char* str1, const char* str2, size_t count)
{
   if (0 != str1 && 0 != str2) {
      size_t len1 = OSCRTLSTRLEN(str1);
      size_t len2 = OSCRTLSTRLEN(str2);
      if (len1 == len2 || ( count <= len1 && count <= len2 ) )
      {
         size_t i;
         size_t bound = OSRTMIN(count, len1);
         bound = OSRTMIN(bound, len2);

         for (i = 0; i < bound; i++) {
            if (OS_TOLOWER(str1[i]) != OS_TOLOWER(str2[i])) return 1;
         }
         return 0;
      }
      else {
         /* the lengths don't match in length and at least one of the strings
            has fewer than count characters so the difference will appear within
            count characters.
         */
         return 1;
      }
   }
   return -1;
}


EXTRTMETHOD char* rtxStrcat (char* dest, size_t bufsiz, const char* src)
{
   if (0 == dest || 0 == bufsiz || 0 == src)
      return (char*)0;
   else
      return rtxStrncat (dest, bufsiz, src, OSCRTLSTRLEN(src));
}

EXTRTMETHOD char* rtxStrncat
(char* dest, size_t bufsiz, const char* src, size_t nchars)
{
   size_t i;

   if (0 == dest || bufsiz <= nchars || 0 == src || 0 == nchars)
      return (char*)0;

   i = OSCRTLSTRLEN (dest);

   if ((bufsiz - i) <= nchars || i >= bufsiz) /*  check for underflow */
      return (char*)0;

   return rtxStrncpy (&dest[i], bufsiz - i, src, nchars);
}

EXTRTMETHOD char* rtxStrcpy (char* dest, size_t bufsiz, const char* src)
{
   if (0 == dest || 0 == bufsiz || 0 == src)
      return (char*)0;
   else
      return rtxStrncpy (dest, bufsiz, src, OSCRTLSTRLEN(src));
}

EXTRTMETHOD char* rtxStrncpy
(char* dest, size_t bufsiz, const char* src, size_t nchars)
{
   size_t i;

   if (0 == dest || bufsiz < nchars || 0 == src || bufsiz == 0) {
      return (char *)0;
   }

   for (i = 0; i < nchars; i++) {
      if (src[i] == '\0') break;
      else dest[i] = src[i];
   }

   if (i < bufsiz) {
      dest[i] = '\0';
      return dest;
   }
   else
   {
      dest[i-1] = '\0'; /* always terminate */
      return (char *)0;
   }
}

EXTRTMETHOD char* rtxStrdup (OSCTXT* pctxt, const char* src)
{
   OSSIZE bufsize = OSCRTLSTRLEN(src) + 1;
   char* tmpbuf = (char*) rtxMemAlloc (pctxt, bufsize);
   if (tmpbuf != 0) {
      rtxStrncpy (tmpbuf, bufsize, src, bufsize - 1);
   }
   return tmpbuf;
}

EXTRTMETHOD char* rtxStrndup (OSCTXT* pctxt, const char* src, OSSIZE nchars)
{
   OSSIZE bufsize = nchars + 1;
   char* tmpbuf = (char*) rtxMemAlloc (pctxt, bufsize);
   if (tmpbuf != 0) {
      rtxStrncpy (tmpbuf, bufsize, src, nchars);
   }
   return tmpbuf;
}

EXTRTMETHOD const char* rtxStrJoin
(char* dest, size_t bufsiz, const char* str1, const char* str2,
 const char* str3, const char* str4, const char* str5)
{
   size_t partLen, idx = 0;
   const char* parts[5];
   OSUINT32 i;

   parts[0] = str1; parts[1] = str2; parts[2] = str3;
   parts[3] = str4, parts[4] = str5;

   for (i = 0; i < 5; i++) {
      if (0 != parts[i]) {
         partLen = OSCRTLSTRLEN (parts[i]);
         if ((idx + partLen) < bufsiz) {
            rtxStrcpy (&dest[idx], bufsiz - idx, parts[i]);
            idx += partLen;
         }
         else break;
      }
      else break;
   }

   dest[idx] = '\0';

   return dest;
}

EXTRTMETHOD char* rtxStrDynJoin
(OSCTXT* pctxt, const char* str1, const char* str2,
 const char* str3, const char* str4, const char* str5)
{
   char* destbuf = 0;
   size_t bufsize = 0;
   if (0 == str1) return 0;
   else bufsize = OSCRTLSTRLEN (str1);

   if (0 != str2) bufsize += OSCRTLSTRLEN (str2);
   if (0 != str3) bufsize += OSCRTLSTRLEN (str3);
   if (0 != str4) bufsize += OSCRTLSTRLEN (str4);
   if (0 != str5) bufsize += OSCRTLSTRLEN (str5);
   bufsize++; /* for null-terminator */

   destbuf = (char*) rtxMemAlloc (pctxt, bufsize);
   if (0 == destbuf) return 0;

   return (0 != rtxStrJoin (destbuf, bufsize, str1, str2, str3, str4, str5)) ?
      destbuf : 0;
}

EXTRTMETHOD char* rtxStrTrimEnd (char *s)
{
   OSSIZE i = OSCRTLSTRLEN(s);
   if (i > 0 && OS_ISSPACE(s[i-1])) {
      while (i > 0) {
         i--;
         if (!OS_ISSPACE(s[i])) break;
      }
      s[i+1] = '\0';
   }
   return (s);
}

EXTRTMETHOD int rtxValidateConstrainedStr(OSCTXT *pctxt, const char *pvalue,
                                          const char *pCharSet)
{
   char badChar[2];
   size_t i, setLen, strLen;
   char *p;
   const char *pElemName;

   if (!pvalue || !pCharSet)
   {
      return 0;
   }
   strLen = OSCRTLSTRLEN(pvalue);
   setLen = OSCRTLSTRLEN(pCharSet);

   for (i = 0; i < strLen; i++)
   {
      p = (char *)memchr(pCharSet, pvalue[i], setLen);

      /* If character not in the defined set, return a constraint
         violation error...                                        */
      if (NULL == p)
      {
         pElemName = (const char *)rtxCtxtPeekElemName(pctxt);
         if (NULL == pElemName)
         {
            pElemName = "?";
         }
         rtxErrAddStrParm(pctxt, pElemName);

         badChar[0] = pvalue[i];
         badChar[1] = '\0';
         rtxErrAddStrParm(pctxt, badChar);

         return RTERR_CONSVIO;
      }
   }

   return 0;
}

