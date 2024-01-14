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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxRegExp.hh"
#include "rtxsrc/rtxMemBuf.h"

enum {
   OSRTFPS_EMPTY = 0,
   OSRTFPS_LEADING_SIGN = 1,
   OSRTFPS_LEADING_ZEROS = 2,
   OSRTFPS_INT_DIGITS = 3,
   OSRTFPS_POINT = 4,
   OSRTFPS_FRACTION_DIGITS = 5,
   OSRTFPS_EXPONENT = 6,
   OSRTFPS_EXP_SIGN = 7,
   OSRTFPS_EXP_LEADING_ZEROS = 8,
   OSRTFPS_EXP_DIGITS = 9
};

static const int automateArray[5][10] = {
   {  1, -1, -1, -1, -1, -1, -1,  8, -1, -1 }, /* +- */
   {  2,  2,  2,  3,  5,  5,  7,  8,  9,  9 }, /* 0 */
   {  3,  3,  3,  3,  5,  5,  7,  9,  9,  9 }, /* d */
   {  4,  4,  4,  4,  5, -1, -1, -1, -1, -1 }, /* . */
   { -1, -1, -1,  6,  6,  6,  7, -1, -1, -1 }  /* E/e */
};

static int getNextState (OSUTF8CHAR charVal, int curState) {
   int row = -1;
   if (charVal == '+' || charVal == '-')
      row = 0;
   else if (charVal == '0')
      row = 1;
   else if ((charVal > '0' && charVal <= '9') || charVal == 'd')
      row = 2;
   else if (charVal == '.')
      row = 3;
   else if (charVal == 'E' || charVal == 'e')
      row = 4;
   else
      return -1;
   if (curState < 0 || curState >= (int)(sizeof (automateArray[0])/sizeof (int)))
      return -1;
   return automateArray[row][curState];
}

typedef struct AutoState {
   int state;
   int intDigits;
   int fracDigits;
   int expDigits;
   OSBOOL exponent;
} AutoState;

static int formatRange
(OSRTMEMBUF* pMemBuf, OSREAL value, AutoState* pState, int minNum,
 int maxNum, int type, int start, int end, OSBOOL neg)
{
   int j = 0, nextState;

   switch (type) {
     case XML_REGEXP_CHARVAL:
     {
         OSUTF8CHAR stCh = (OSUTF8CHAR)((start <= 0) ? 'A' : start);
         OSUTF8CHAR endCh = (OSUTF8CHAR)((end <= 0) ? 'z' : end);
         OSUTF8CHAR curCh = stCh;

         nextState = getNextState (curCh, pState->state);
         switch (nextState) {
            case OSRTFPS_LEADING_SIGN:
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
               if (minNum != 1 || maxNum != 1 ||
                   (stCh != '+' && endCh != '+' &&
                    stCh != '-' && endCh != '-'))
                  return -1;
               break;
            case OSRTFPS_LEADING_ZEROS:
               rtxMemBufSet (pMemBuf, '0', sizeof (OSUTF8CHAR) * minNum);
               j = minNum;
               break;
            case OSRTFPS_INT_DIGITS:
               if (pState->intDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%I", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->intDigits += minNum;
               break;
            case OSRTFPS_FRACTION_DIGITS:
               if (pState->fracDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%F", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->fracDigits += minNum;
               break;
            case OSRTFPS_EXP_DIGITS:
               if (pState->expDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%X", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->expDigits += minNum;
               break;
            case OSRTFPS_POINT:
               if (minNum != 1 || maxNum != 1) return -1;

               rtxMemBufSet (pMemBuf, '.', sizeof (OSUTF8CHAR));
               j = 1;
               break;
            case OSRTFPS_EXPONENT:
               if (minNum != 1 || maxNum != 1 || pState->exponent) return -1;

               rtxMemBufSet (pMemBuf, curCh, sizeof (OSUTF8CHAR));
               pState->exponent = TRUE;
               j = 1;
               break;
         }

         break;
     }
     case XML_REGEXP_DECIMAL:
     {
         if (neg) return -1;

         nextState = getNextState ('d', pState->state);
         switch (nextState) {
            case OSRTFPS_INT_DIGITS:
               if (pState->intDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%I", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->intDigits += minNum;
               break;
            case OSRTFPS_FRACTION_DIGITS:
               if (pState->fracDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%F", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->fracDigits += minNum;
               break;
            case OSRTFPS_EXP_DIGITS:
               if (pState->expDigits == 0) {
                  rtxMemBufAppend (pMemBuf, (const OSUTF8CHAR*)"%X", sizeof (OSUTF8CHAR) * 2);
               }
               j = minNum;
               pState->expDigits += minNum;
               break;
         }
         break;
     }
     default:
        return -1;
   }
   if (j > 0)
      pState->state = nextState;
   return j;
}

static int makeIntFormatStr
(OSCTXT* pOSCTXT, rtxRegexpPtr regexp, OSREAL value, OSUTF8CHAR** ppDest)
{
   int i, minNum, maxNum, j;
   OSRTMEMBUF membuf;
   AutoState state;

   rtxMemBufInit (pOSCTXT, &membuf, 100);
   OSCRTLMEMSET (&state, 0, sizeof (state));

   if (regexp == NULL) {
       return RTERR_REGEXP;
   }
   for (i = 0;i < regexp->nbAtoms; i++) {

       minNum = maxNum = 0;
       switch (regexp->atoms[i]->quant) {
           case XML_REGEXP_QUANT_EPSILON:
           case XML_REGEXP_QUANT_ONCEONLY:
           case XML_REGEXP_QUANT_ONCE:
           case XML_REGEXP_QUANT_OPT:
               minNum = maxNum = 1;
               break;

           case XML_REGEXP_QUANT_MULT:
           case XML_REGEXP_QUANT_PLUS:
               minNum = 1;
               maxNum = INT_MAX;
               break;

           case XML_REGEXP_QUANT_RANGE:
               minNum = regexp->atoms[i]->min;
               maxNum = regexp->atoms[i]->max;
               break;

           case XML_REGEXP_QUANT_ALL:
               break;
       }
       if (regexp->atoms[i]->codepoint != 0) {
           OSUTF8CHAR curCh = (OSUTF8CHAR)regexp->atoms[i]->codepoint;

           if (formatRange (&membuf, value, &state,
                            minNum, maxNum, XML_REGEXP_CHARVAL,
                            curCh, curCh, FALSE) < 0)
              return RTERR_REGEXP;
       }
       else {
          switch (regexp->atoms[i]->type) {
              case XML_REGEXP_RANGES:
              {
                 for (j = 0; maxNum > 0 && j < regexp->atoms[i]->nbRanges; j++) {
                    int dnum = formatRange
                       (&membuf, value, &state, minNum, maxNum,
                        regexp->atoms[i]->ranges[j]->type,
                        regexp->atoms[i]->ranges[j]->start,
                        regexp->atoms[i]->ranges[j]->end,
                        (OSBOOL)regexp->atoms[i]->ranges[j]->neg);

                    if (dnum < 0)
                       return RTERR_REGEXP;
                    maxNum -= dnum;
                 }
                 break;
              }

              default:
              {
                 int dnum = formatRange
                    (&membuf, value, &state, minNum, maxNum,
                     regexp->atoms[i]->type, 0, 0,
                     (OSBOOL)regexp->atoms[i]->neg);

                 if (dnum < 0)
                    return RTERR_REGEXP;
              }
          }
      }
   }

   rtxMemBufSet (&membuf, 0, sizeof (OSUTF8CHAR));

   *ppDest = OSMEMBUFPTR (&membuf);
   return 0;
}

EXTXMLMETHOD int rtXmlEncDoublePatternValue (OSCTXT* pctxt, OSREAL value, const OSUTF8CHAR* pattern)
{
   int stat = 0;
   OSUTF8CHAR* pFormatStr = 0, *pPatternStr, *curp;
   rtxRegexpPtr pRegExp;
   OSUTF8CHAR* endp;

   pctxt->state = OSXMLDATA;

   pPatternStr = curp = rtxUTF8Strdup (pctxt, pattern);

   do {
      endp = rtxUTF8StrChr (curp, '|');

      if (endp != 0)
         *endp = 0;

      pRegExp = rtxRegexpCompile (pctxt, curp);

#ifdef _TRACE
      if (rtxDiagEnabled (pctxt)) {
         rtxRegexpPrint(stdout, pRegExp);
      }
#endif

      if (0 != pRegExp) {

         stat = makeIntFormatStr (pctxt, pRegExp, value, &pFormatStr);

         if (stat == 0) {
            stat = rtxRegexpExec (pctxt, pRegExp, pFormatStr);
            if (stat == 0)
               stat = RTERR_PATMATCH;
            else
               stat = 0;
         }

         rtxRegFreeRegexp (pctxt, pRegExp);
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

EXTXMLMETHOD int rtXmlEncDoublePattern (OSCTXT* pctxt, OSREAL value,
                           const OSUTF8CHAR* elemName,
                           OSXMLNamespace* pNS,
                           /*int totalDigits,*/ int fractionDigits,
                           const OSUTF8CHAR* pattern)
{
   char lbuf[256], lbuf2[256], *psrc = (char*)lbuf, *psrc2;
   int  stat, len;
   OSRTDList namespaces;

   rtxDListInit(&namespaces);

   stat = rtXmlEncStartElement (pctxt, elemName, pNS, &namespaces, FALSE);

   /* if rtXmlEncStartElement added any namespaces, encode them */
   if ( stat == 0 )
      stat = rtXmlEncNSAttrs(pctxt, &namespaces);

   /* encode close of start tag */
   if ( stat == 0 )
      stat = rtXmlEncTermStartElement(pctxt);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   rtXmlEncDoublePatternValue (pctxt, value, pattern);

   if (rtxIsMinusZero (value)) {
      stat = rtXmlWriteChars (pctxt, OSUTF8("-0"), 2);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsPlusInfinity (value)) {
      stat = rtXmlWriteChars (pctxt, OSUTF8("INF"), 3);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsMinusInfinity (value)) {
      stat = rtXmlWriteChars (pctxt, OSUTF8("-INF"), 4);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (rtxIsNaN (value)) {
      stat = rtXmlWriteChars (pctxt, OSUTF8("NaN"), 3);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else if (value == 0.0) {
      stat = rtXmlWriteChars (pctxt, OSUTF8("0"), 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      char formatStr[10];

      if (fractionDigits == -1)
         fractionDigits = 100;

      os_snprintf (formatStr, sizeof(formatStr), "%%1.%iE", fractionDigits);

      /* specify maximum precision after point */
      os_snprintf (lbuf, sizeof(lbuf), formatStr, value);

      psrc = strchr (lbuf, ','); /* look for decimal point, if it is comma */
      if (psrc != NULL)
         *psrc = '.';            /* replace by period */
      psrc2 = psrc = strchr (lbuf, 'E');

      /* skip all trailing zeros in fractional part */
      while (*(psrc2 - 1) == '0')
         psrc2--;

      /* ...if fractional part is zero, leave one zero in it */
      if (*(psrc2 - 1) == '.') psrc2++;

      /* copy the first part before 'E' and exp sign */
      rtxStrncpy (lbuf2, sizeof(lbuf2), lbuf, psrc2 - (char*)lbuf);

      psrc++; /* skip 'E' */
      psrc2 = psrc + 1; /* move src ptr behind 'E' and sign */
      /* skip all leading zeros in exponent */
      while (*psrc2 == '0') psrc2++;

      if (*psrc2 != '\0') {
         rtxStrcat (lbuf2, sizeof(lbuf2), "E");

         if (*psrc == '-') {
            /* if exp is negative, put '-' sign */
            rtxStrcat (lbuf2, sizeof(lbuf2), "-");
         }

         len = (int)OSCRTLSTRLEN (lbuf);

         /* copy exponent part */
         rtxStrncat (lbuf2, sizeof(lbuf2),
                     psrc2, len - (psrc2 - (char*)lbuf));
      }
      else {
         rtxStrcat (lbuf2, sizeof(lbuf2), "E0");
      }
      stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)lbuf2, OSCRTLSTRLEN(lbuf2));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   if (0 != elemName) {
      stat = rtXmlEncEndElement (pctxt, elemName, pNS);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return (0);
}

