/*
 * regexp.c: generic and extensible Regular Expression engine
 *
 * Basically designed with the purpose of compiling regexps for
 * the variety of validation/shemas mechanisms now available in
 * XML related specifications thise includes:
 *    - XML-1.0 DTD validation
 *    - XML Schemas structure part 1
 *    - XML Schemas Datatypes part 2 especially Appendix F
 *    - RELAX-NG/TREX i.e. the counter proposal
 *
 * See Copyright for the status of this software.
 *
 * Daniel Veillard <veillard@redhat.com>
 */
/*
NOTE: the copyright notice below applies only to source and header files
that include this notice.  It does not apply to other Objective Systems
software with different attached notices.

Except where otherwise noted in the source code (e.g. the files hash.c,
list.c and the trio files, which are covered by a similar licence but
with different Copyright notices) all the files are:

 Copyright (C) 1998-2023 Daniel Veillard.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is fur-
nished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
DANIEL VEILLARD BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Daniel Veillard shall not
be used in advertising or otherwise to promote the sale, use or other deal-
ings in this Software without prior written authorization from him.
*/
/**
 * Changes made to original libxml source file (xmlregexp.c) by
 * Objective Systems, Inc are as follows:
 *
 * 1. Changed to fit Objective Systems run-time environment including
 * common type name changes and use of OSys mem mgmt.
 *
 * 2. Name change from xmlregexp to rtxRegExp to reflect fact that the
 * code will be used in other non-XML environments.
 */
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxRegExp.hh"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"

#define MAX_MEMBUF_SIZE 512

static int rtxRegexpFillRange
   (OSRTMEMBUF* pMemBuf, int num, int type,
    int start, int end, OSBOOL neg)
{
   int j = 0;

   /* correct type if neg is TRUE */
   if (neg) {
      neg = FALSE;
      switch (type) {
         case XML_REGEXP_LETTER_UPPERCASE:
            type = XML_REGEXP_LETTER_LOWERCASE;
            break;
         case XML_REGEXP_LETTER_LOWERCASE:
            type = XML_REGEXP_LETTER_UPPERCASE;
            break;
         default:
            neg = TRUE;
      }
   }
   switch (type) {
      case XML_REGEXP_EPSILON:
          OSRTASSERT (type != XML_REGEXP_EPSILON);
          break;
      case XML_REGEXP_CHARVAL:
      {

          OSUTF8CHAR stCh = (OSUTF8CHAR)((start <= 0) ? 'A' : start);
          OSUTF8CHAR endCh = (OSUTF8CHAR)((end <= 0) ? 'z' : end);
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             if (neg && curCh >= stCh && curCh <= endCh) {
                int icurCh1 = endCh + (curCh - stCh) + 1;
                OSUTF8CHAR curCh1;

                if (icurCh1 > 127) icurCh1 = ' ' + (icurCh1 - 127);
                curCh1 = (OSUTF8CHAR)icurCh1;
                rtxMemBufAppend (pMemBuf, &curCh1, sizeof (OSUTF8CHAR));
             }
             else
                rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }

          break;
      }
      case XML_REGEXP_RANGES:
      {
          OSRTASSERT (type != XML_REGEXP_RANGES);
          /*int i;

          for (i = 0; i <
          rtxRegexpFillRange (regexp, pMemBuf,
          fprintf(output, "ranges ");*/ break;
      }
      case XML_REGEXP_SUBREG:
          OSRTASSERT (type != XML_REGEXP_SUBREG);
          break;
      case XML_REGEXP_STRING:
          OSRTASSERT (type != XML_REGEXP_STRING);
          break;
      case XML_REGEXP_ANYCHAR:  /* . */
      {
          OSUTF8CHAR curCh = (OSUTF8CHAR)'a';

          rtxMemBufSet (pMemBuf, curCh, sizeof (OSUTF8CHAR) * num);
          break;
      }
      case XML_REGEXP_ANYSPACE: /* \s */
          rtxMemBufSet (pMemBuf, ' ', num);
          j = num;
          break;
      case XML_REGEXP_NOTSPACE: /* \S */
          rtxMemBufSet (pMemBuf, '_', num);
          j = num;
          break;
      case XML_REGEXP_INITNAME:
      case XML_REGEXP_NOTINITNAME:
          break;
      case XML_REGEXP_NAMECHAR: /* \c */
      {
          OSUTF8CHAR stCh = 'a';
          OSUTF8CHAR endCh = 'z';
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_NOTNAMECHAR: /* \C */
      {
          OSUTF8CHAR stCh = '!';
          OSUTF8CHAR endCh = '0' - 1;
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_DECIMAL: /* \d */
      {
          OSUTF8CHAR stCh = (OSUTF8CHAR)((start <= 0) ? '0' : start);
          OSUTF8CHAR endCh = (OSUTF8CHAR)((end <= 0) ? '9' : end);
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_NOTDECIMAL: /* \D */
      {
          OSUTF8CHAR stCh = 'A';
          OSUTF8CHAR endCh = 'z';
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_REALCHAR:    /* \w */
      {
          OSUTF8CHAR stCh = 'a';
          OSUTF8CHAR endCh = 'z';
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_NOTREALCHAR: /* \W */
      {
          OSUTF8CHAR stCh = ' ' + 1;
          OSUTF8CHAR endCh = '0' - 1;
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_LETTER:
      case XML_REGEXP_LETTER_UPPERCASE: /* \p{Lu} */
      case XML_REGEXP_LETTER_TITLECASE:
      {
          OSUTF8CHAR stCh = 'A';
          OSUTF8CHAR endCh = 'Z';
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh++;
             if ((int)curCh > endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_LETTER_LOWERCASE: /* \p{Ll} */
      case XML_REGEXP_LETTER_OTHERS:
      {
          OSUTF8CHAR stCh = 'z';
          OSUTF8CHAR endCh = 'a';
          OSUTF8CHAR curCh = stCh;

          for (j = 0; j < num; j++) {
             rtxMemBufAppend (pMemBuf, &curCh, sizeof (OSUTF8CHAR));
             curCh--;
             if ((int)curCh < endCh) curCh = stCh;
          }
          break;
      }
      case XML_REGEXP_NUMBER:
      case XML_REGEXP_NUMBER_DECIMAL:
      {
         rtxMemBufSet (pMemBuf, '0', num);
         j = num;
         break;
      }
      case XML_REGEXP_LETTER_MODIFIER:
      case XML_REGEXP_MARK:
      case XML_REGEXP_MARK_NONSPACING:
      case XML_REGEXP_MARK_SPACECOMBINING:
      case XML_REGEXP_MARK_ENCLOSING:
      case XML_REGEXP_NUMBER_LETTER:
      case XML_REGEXP_NUMBER_OTHERS:
      case XML_REGEXP_PUNCT:
      case XML_REGEXP_PUNCT_CONNECTOR:
      case XML_REGEXP_PUNCT_DASH:
      case XML_REGEXP_PUNCT_OPEN:
      case XML_REGEXP_PUNCT_CLOSE:
      case XML_REGEXP_PUNCT_INITQUOTE:
      case XML_REGEXP_PUNCT_FINQUOTE:
      case XML_REGEXP_PUNCT_OTHERS:
      case XML_REGEXP_SEPAR:
      case XML_REGEXP_SEPAR_SPACE:
      case XML_REGEXP_SEPAR_LINE:
      case XML_REGEXP_SEPAR_PARA:
      case XML_REGEXP_SYMBOL:
      case XML_REGEXP_SYMBOL_MATH:
      case XML_REGEXP_SYMBOL_CURRENCY:
      case XML_REGEXP_SYMBOL_MODIFIER:
      case XML_REGEXP_SYMBOL_OTHERS:
      case XML_REGEXP_OTHER:
      case XML_REGEXP_OTHER_CONTROL:
      case XML_REGEXP_OTHER_FORMAT:
      case XML_REGEXP_OTHER_PRIVATE:
      case XML_REGEXP_OTHER_NA:
      case XML_REGEXP_BLOCK_NAME:
          OSRTASSERT (type != XML_REGEXP_LETTER_MODIFIER &&
            type != XML_REGEXP_MARK &&
            type != XML_REGEXP_MARK_NONSPACING &&
            type != XML_REGEXP_MARK_SPACECOMBINING &&
            type != XML_REGEXP_MARK_ENCLOSING &&
            type != XML_REGEXP_NUMBER_LETTER &&
            type != XML_REGEXP_NUMBER_OTHERS &&
            type != XML_REGEXP_PUNCT &&
            type != XML_REGEXP_PUNCT_CONNECTOR &&
            type != XML_REGEXP_PUNCT_DASH &&
            type != XML_REGEXP_PUNCT_OPEN &&
            type != XML_REGEXP_PUNCT_CLOSE &&
            type != XML_REGEXP_PUNCT_INITQUOTE);

          /* AT 06/11/2007 -- split this into two asserts to keep within
             HP aCC's limitations */

          OSRTASSERT (type != XML_REGEXP_PUNCT_FINQUOTE &&
            type != XML_REGEXP_PUNCT_OTHERS &&
            type != XML_REGEXP_SEPAR &&
            type != XML_REGEXP_SEPAR_SPACE &&
            type != XML_REGEXP_SEPAR_LINE &&
            type != XML_REGEXP_SEPAR_PARA &&
            type != XML_REGEXP_SYMBOL &&
            type != XML_REGEXP_SYMBOL_MATH &&
            type != XML_REGEXP_SYMBOL_CURRENCY &&
            type != XML_REGEXP_SYMBOL_MODIFIER &&
            type != XML_REGEXP_SYMBOL_OTHERS &&
            type != XML_REGEXP_OTHER &&
            type != XML_REGEXP_OTHER_CONTROL &&
            type != XML_REGEXP_OTHER_FORMAT &&
            type != XML_REGEXP_OTHER_PRIVATE &&
            type != XML_REGEXP_OTHER_NA &&
            type != XML_REGEXP_BLOCK_NAME);
          break;
   }
   return j;
}

typedef struct _trans_traces {
   int min;
   int max;
   int cur;
} _trans_traces;

typedef struct _state_traces {
   int nbTrans;
   _trans_traces* trans;
} _state_traces;

int rtxRegexpFill
   (OSCTXT* pctxt, rtxRegexpPtr regexp, const OSUTF8CHAR** ppDest)
{
   int curStateIdx;
   int curTransIdx, newStateIdx = 0;
   int num = 1, j;
   OSRTMEMBUF membuf;
   _state_traces* traces;
   int *counts;

   rtxMemBufInit (pctxt, &membuf, 100);

   if (regexp == NULL) {
       return -1;
   }

   traces = rtxMemAllocArrayZ (pctxt, regexp->nbStates, _state_traces);
   counts = rtxMemAllocArrayZ (pctxt, regexp->nbCounters, int);
#ifdef _MEMCOMPACT
   if (counts != 0)
      OSCRTLMEMSET (counts, 0, regexp->nbCounters * sizeof (int));
#else
   if (traces == 0 || counts == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   OSCRTLMEMSET (counts, 0, regexp->nbCounters * sizeof (int));
#endif

   for (curStateIdx = 0; curStateIdx < regexp->nbStates; ) {
      rtxRegStatePtr pCurState = regexp->states[curStateIdx];
      rtxRegCounterPtr pCurCounter = 0;
      int count = -1;
      int run;

      num = 1;

      /*!AB this 'if' is necessary only to prevent infinite loop */
      if (pCurState == NULL) {
         curStateIdx++; /*?????*/
         newStateIdx = curStateIdx + 1;
         continue;
      }

      /* if it is a FINAL state, and newStateIdx is less or equal than
         current one - quit. */
      if (pCurState->type == XML_REGEXP_FINAL_STATE &&
          curStateIdx >= newStateIdx)
         break;


      for (run = 1; run; run = 0) {
         for (curTransIdx = 0; curTransIdx < pCurState->nbTrans; curTransIdx++) {
            rtxRegTransPtr pCurTrans = &pCurState->trans[curTransIdx];
            rtxRegAtomPtr pCurAtom = pCurTrans->atom;

            num = 1;

            if (pCurTrans->to < 0 && pCurAtom == 0) /* removed */
               continue;

            /* YG: prefer forward jumps */
            if (pCurTrans->to < curStateIdx) {
               int curTransIdxTm = curTransIdx + 1;

               for (;curTransIdxTm < pCurState->nbTrans; curTransIdxTm++) {
                  rtxRegTransPtr pCurTransTm = &pCurState->trans[curTransIdxTm];
                  rtxRegAtomPtr pCurAtomTm = pCurTransTm->atom;

                  if (pCurTransTm->to < 0 && pCurAtomTm == 0) /* removed */
                     continue;

                  if (pCurTransTm->to > curStateIdx) {
                     pCurTrans = pCurTransTm;
                     pCurAtom = pCurAtomTm;
                     curTransIdx = curTransIdxTm;
                     break;
                  }
               }
            }

            if (pCurAtom == 0 || pCurAtom->type == XML_REGEXP_EPSILON) {
               if (pCurTrans->count != -1) {
                  if (pCurTrans->count != REGEXP_ALL_COUNTER &&
                      pCurTrans->count != REGEXP_ALL_LAX_COUNTER)
                  {
                     int max;
                     pCurCounter = &regexp->counters[pCurTrans->count];
                     /* YG
                     if (pCurCounter->max - pCurCounter->min > 10)
                        max = pCurCounter->min + 10;
                     else
                        max = pCurCounter->max;
                     */
                     if (pCurCounter->min > 0)
                        max = pCurCounter->min;
                     else if (pCurCounter->max > 0)
                        max = 1;
                     else
                        max = 0;

                     if (counts[pCurTrans->count] >= max) {
                        newStateIdx = pCurTrans->to;
                        counts[pCurTrans->count] = 0;   /*YG - restart count*/
                        break;
                     }
                  }
                  else {
                     /* ??? AB: don't know what to do here. Need more invetigations. */
                     newStateIdx = pCurTrans->to;
                     break;
                  }
               }
               else
                  newStateIdx = pCurTrans->to;
            }

            if (pCurAtom == 0)
               continue;

            /*!AB this 'if' is necessary only to prevent infinite loop */
            if (OSMEMBUFUSEDSIZE(&membuf) > MAX_MEMBUF_SIZE) {
               newStateIdx++;
               break;
            }
/*            if (pCurTrans->counter < 0) {
               _trans_traces* pCurTrace;

               if (traces [curStateIdx].trans == 0) {
                   traces [curStateIdx].nbTrans = pCurState->nbTrans;
                   traces [curStateIdx].trans =
                      OSRTALLOCTYPEARRAYZ (pctxt, _trans_traces, pCurState->nbTrans);
                   if (traces [curStateIdx].trans == NULL)
                      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
               }
               pCurTrace = &traces [curStateIdx].trans[curTransIdx];
               if (pCurTrace->max == 0) {
                  pCurTrace->max = 1;
               }
               if (pCurTrace->cur++ > pCurTrace->max) {
                  newStateIdx++;
                  break;
               }
            }
*/
            if (pCurTrans->counter >= 0) {
               int max;
               pCurCounter = &regexp->counters[pCurTrans->counter];
               count = counts[pCurTrans->counter];
               /* YG
               if (pCurCounter->max - pCurCounter->min > 10)
                  max = pCurCounter->min + 10;
               else
                  max = pCurCounter->max;
               */
               if (pCurCounter->min > 0)
                  max = pCurCounter->min;
               else if (pCurCounter->max > 0)
                  max = 1;
               else
                  max = 0;

               if (count >= max)
                  continue;
               counts[pCurTrans->counter]++;
            }

            switch (pCurAtom->quant) {
                case XML_REGEXP_QUANT_EPSILON:
                    break;

                case XML_REGEXP_QUANT_ONCE:
                    num = 1;
                    break;
                case XML_REGEXP_QUANT_OPT:
                    num = 1;
                    break;

                case XML_REGEXP_QUANT_MULT:
                    num = 10;/* !rand */
                    break;
                case XML_REGEXP_QUANT_PLUS:
                    num = 10;/* !rand */
                    break;
                case XML_REGEXP_QUANT_RANGE:
                    num = pCurAtom->min;
                    if (pCurAtom->max > num + 10) {
                       num += 10;
                    }
                    else
                       num = pCurAtom->max;
                    break;
                case XML_REGEXP_QUANT_ONCEONLY:
                    OSRTASSERT
                       (pCurAtom->quant != XML_REGEXP_QUANT_ONCEONLY);
                    break;
                case XML_REGEXP_QUANT_ALL:
                    OSRTASSERT
                       (pCurAtom->quant != XML_REGEXP_QUANT_ALL);
                    break;
            }
            if (pCurAtom->codepoint != 0) {
                OSUTF8CHAR curCh = (OSUTF8CHAR)pCurAtom->codepoint;

                rtxMemBufSet (&membuf, curCh, sizeof (OSUTF8CHAR) * num);
            }
            else {
               switch (pCurAtom->type) {
                   case XML_REGEXP_RANGES:
                   {
                      for (j = 0; num > 0 && j < pCurAtom->nbRanges; j++) {
                         num -= rtxRegexpFillRange (&membuf, num,
                            pCurAtom->ranges[j]->type,
                            pCurAtom->ranges[j]->start,
                            pCurAtom->ranges[j]->end,
                            (OSBOOL)pCurAtom->ranges[j]->neg);
                      }
                      break;
                   }

                   default:
                         rtxRegexpFillRange (&membuf, num,
                            pCurAtom->type, 0, 0,
                            (OSBOOL)pCurAtom->neg);
               }
            }

            if (pCurTrans->to != curStateIdx) {
               newStateIdx = pCurTrans->to;
               break;
            }
         }

         /* check the counter - if exists and min value is not reached - repeat
            the state */
         /*if (pCurCounter != 0 && count < pCurCounter->min)
            continue;
         else */
         /*break;*/ /* else - go to next state */
      } /* for (run = 1 ... */

      /* if it is a FINAL state, and newStateIdx is less or equal than
         current one - quit. */
      /*if (pCurState->type == XML_REGEXP_FINAL_STATE &&
           // curStateIdx >= newStateIdx) */
      /*    break; */
      /* !AB: moved to the beginning of the loop (pattern #29: "([0-2](\.[0-9]+)+)") */

      curStateIdx = newStateIdx;
   }

   if (traces != 0) {
      for (j = 0; j < regexp->nbStates; j++) {
         rtxMemFreePtr (pctxt, traces[j].trans);
      }
      rtxMemFreePtr (pctxt, traces);
   }
   rtxMemFreePtr (pctxt, counts);

   rtxMemBufSet (&membuf, 0, sizeof (OSUTF8CHAR));

   RTDIAG2 (pctxt, "Generated: %s\n", OSMEMBUFPTR (&membuf));

   *ppDest = OSMEMBUFPTR (&membuf);
   return 0;
}

