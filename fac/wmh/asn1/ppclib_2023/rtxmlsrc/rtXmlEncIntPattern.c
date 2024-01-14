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
/* These defs allow this function to be used as a template for other
   XML integer with pattern encode functions (int64) */

#ifndef RTXMLENCINTPATFUNC
#define RTXMLENCINTPATFUNC rtXmlEncIntPattern
#define RTXMLENCINTPATVALUEFUNC rtXmlEncIntPatternValue
#define RTXINTTOCHARSTR rtxIntToCharStr
#define OSINTPATTYPE OSINT32
#endif

/*
For some reason, at least with Visual C++ 6, we sometimes come to this spot
in the compile with RTXMLENCINTPATFUNC defined, but with some or all of the
pieces that should be defined with it being undefined.  Simply adding a few
#pragma message statements to the #ifndef block above prevents this from
happening, so it appears we have stumbled over a quirk of the Visual C++ 6
compiler.  We are adding the block below to ensure that everything that's
needed is defined.
*/

#ifdef RTXMLENCINTPATFUNC

#ifndef RTXMLENCINTPATVALUEFUNC
#define RTXMLENCINTPATVALUEFUNC rtXmlEncIntPatternValue
#endif

#ifndef RTXINTTOCHARSTR
#define RTXINTTOCHARSTR rtxIntToCharStr
#endif

#ifndef OSINTPATTYPE
#define OSINTPATTYPE OSINT32
#endif

#endif

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxRegExp.hh"
#include "rtxsrc/rtxMemBuf.h"

enum {
   OSRTIPS_EMPTY = 0,
   OSRTIPS_DIGITS = 1,
   OSRTIPS_LEADING_SIGN = 2,
   OSRTIPS_LEADING_ZEROS = 4
};

static int formatRange
(OSRTMEMBUF* pMemBuf, OSINTPATTYPE value, int* pState,
 int num, int type, int start, int end, OSBOOL neg)
{
   int j = 0;

   switch (type) {
     case XML_REGEXP_CHARVAL:
     {
         OSUTF8CHAR stCh = (OSUTF8CHAR)((start <= 0) ? 'A' : start);
         OSUTF8CHAR endCh = (OSUTF8CHAR)((end <= 0) ? 'z' : end);

         if (*pState != OSRTIPS_DIGITS && (stCh == '0' && endCh == '0'))
         {
            rtxMemBufSet (pMemBuf, '0', sizeof (OSUTF8CHAR) * num);
            j = num;

            *pState = OSRTIPS_LEADING_ZEROS;
         }
         else if (stCh >= '0' && endCh <= '9') {
            j = num;
            if (*pState != OSRTIPS_DIGITS) {
               char lbuf[40];
               int len = RTXINTTOCHARSTR
                  ((value > 0) ? value : -value, lbuf, sizeof(lbuf), 0);

               if (len > 0) {
                  if (len < num) {
                     rtxMemBufSet (pMemBuf, '0', (num-len) * sizeof (OSUTF8CHAR));
                  }
                  rtxMemBufAppend (pMemBuf, (OSUTF8CHAR*)&lbuf, len);
               } else
                  return len;
            }
            *pState = OSRTIPS_DIGITS;
         }
         else if (*pState == OSRTIPS_EMPTY) {
            j = 1;
            if (stCh == '+' || endCh == '+' || (neg && (stCh == '-' || endCh == '-'))) {
               if (value >= 0)
                  rtxMemBufSet (pMemBuf, '+', sizeof (OSUTF8CHAR));
               else
                  j = 0;
            }
            if (stCh == '-' || endCh == '-' || (neg && (stCh == '+' || endCh == '+'))) {
               if (value <= 0)
                  rtxMemBufSet (pMemBuf, '-', sizeof (OSUTF8CHAR));
               else
                  j = 0;
            }
            if ((stCh != '+' && endCh != '+' &&
                stCh != '-' && endCh != '-'))
               return -1;
            if (j > 0) *pState = OSRTIPS_LEADING_SIGN;
         }
         else
            return -1;
         break;
     }
     case XML_REGEXP_DECIMAL:
     case XML_REGEXP_NUMBER:
     {
         if (neg) return -1;
         if (*pState != OSRTIPS_DIGITS) {
            char lbuf[40];
            int len = RTXINTTOCHARSTR
               ((value > 0) ? value : -value, lbuf, sizeof(lbuf), 0);

            if (len > 0) {
               if (len < num) {
                  rtxMemBufSet (pMemBuf, '0', (num-len) * sizeof (OSUTF8CHAR));
               }
               rtxMemBufAppend (pMemBuf, (OSUTF8CHAR*)&lbuf, len);
            } else
               return len;
         }
         *pState = OSRTIPS_DIGITS;
         break;
     }
     default:
        return -1;
   }
   return j;
}

static int rtXmlMakeIntFormatStr
(OSCTXT* pOSCTXT, rtxRegexpPtr regexp, OSINTPATTYPE value, OSUTF8CHAR** ppDest)
{
   int i, num, j, state = OSRTIPS_EMPTY;
   OSRTMEMBUF membuf;
   int dnum = 0;

   rtxMemBufInit (pOSCTXT, &membuf, 100);

   if (regexp == NULL) {
       return RTERR_REGEXP;
   }
   for (i = 0;i < regexp->nbAtoms; i++) {

       num = 0;
       switch (regexp->atoms[i]->quant) {
           case XML_REGEXP_QUANT_EPSILON:
           case XML_REGEXP_QUANT_ONCEONLY:
           case XML_REGEXP_QUANT_ONCE:
           case XML_REGEXP_QUANT_OPT:
               num = 1;
               break;

           case XML_REGEXP_QUANT_MULT:
           case XML_REGEXP_QUANT_PLUS:
               num = INT_MAX;
               break;
           case XML_REGEXP_QUANT_RANGE:
               num = regexp->atoms[i]->max;
               break;

           case XML_REGEXP_QUANT_ALL:
               break;
       }
       if (regexp->atoms[i]->codepoint != 0) {
           OSUTF8CHAR curCh = (OSUTF8CHAR)regexp->atoms[i]->codepoint;

           dnum = formatRange (&membuf, value, &state,
                            num, XML_REGEXP_CHARVAL, curCh, curCh, FALSE);
       }
       else {
          switch (regexp->atoms[i]->type) {
              case XML_REGEXP_RANGES:
              {
                 for (j = 0; num > 0 && j < regexp->atoms[i]->nbRanges; j++) {
                    dnum = formatRange
                       (&membuf, value, &state, num,
                        regexp->atoms[i]->ranges[j]->type,
                        regexp->atoms[i]->ranges[j]->start,
                        regexp->atoms[i]->ranges[j]->end,
                        (OSBOOL)regexp->atoms[i]->ranges[j]->neg);
                    if (dnum < 0) break;
                    num -= dnum;
                 }
                 break;
              }

              default:
              {
                 dnum = formatRange
                    (&membuf, value, &state, num,
                     regexp->atoms[i]->type, 0, 0,
                     (OSBOOL)regexp->atoms[i]->neg);
              }
          }
       }
       if (dnum >= 0) break;
   }

   if (dnum < 0)
      return RTERR_REGEXP;

   rtxMemBufSet (&membuf, 0, sizeof (OSUTF8CHAR));

   *ppDest = OSMEMBUFPTR (&membuf);
   return 0;
}

EXTXMLMETHOD int RTXMLENCINTPATVALUEFUNC
(OSCTXT* pctxt, OSINTPATTYPE value, const OSUTF8CHAR* pattern)
{
   int stat = 0;
   OSUTF8CHAR *pPatternStr, *curp;
   OSUTF8CHAR* pFormatStr = 0;
   rtxRegexpPtr pRegExp;
   OSUTF8CHAR* endp;

   pctxt->state = OSXMLDATA;

   pPatternStr = curp = rtxUTF8Strdup (pctxt, pattern);

   do {
      endp = rtxUTF8StrChr (curp, '|');

      if (endp != 0)
         *endp = 0;

      pRegExp = rtxRegexpCompile (pctxt, curp);

      if (0 != pRegExp) {

         stat = rtXmlMakeIntFormatStr (pctxt, pRegExp, value, &pFormatStr);

         if (stat == 0) {
            stat = rtxRegexpExec (pctxt, pRegExp, pFormatStr);
            if (stat == 0)
               stat = RTERR_PATMATCH;
            else
               stat = 0;
         }

         rtxRegFreeRegexp (pctxt, pRegExp);
         if (stat < 0) return LOG_RTERRNEW (pctxt, stat);
      }
      else
         return LOG_RTERRNEW (pctxt, RTERR_REGEXP);
      curp = endp + 1;
   } while (stat != 0 && endp != 0);

   if (stat == 0)
      stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)pFormatStr,
                              OSCRTLSTRLEN((const char*)pFormatStr));

   if (pFormatStr != 0) {
      rtxMemFreePtr (pctxt, pFormatStr);
   }
   rtxMemFreePtr (pctxt, pPatternStr);

   if (stat < 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

EXTXMLMETHOD int RTXMLENCINTPATFUNC (OSCTXT* pctxt, OSINTPATTYPE value,
                        const OSUTF8CHAR* elemName,
                        OSXMLNamespace* pNS,
                        const OSUTF8CHAR* pattern)
{
   int stat;
   OSRTDList namespaces;

   rtxDListInit(&namespaces);

   stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

   /* if rtXmlEncStartElement added any namespaces, encode them */
   if ( stat == 0 )
      stat = rtXmlEncNSAttrs(pctxt, &namespaces);

   /* encode close of start tag */
   if ( stat == 0 )
      stat = rtXmlEncTermStartElement(pctxt);

   if (0 == stat)
      stat = RTXMLENCINTPATVALUEFUNC (pctxt, value, pattern);

   if (0 != elemName && 0 == stat) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
   }

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}


