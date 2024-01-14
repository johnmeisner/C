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

/* maximum level of nested parents */
#define MAX_NESTING 16

/* initial size of state array */
#define STATE_ARRAY_SZ 32

/* maximum size of result string */
#define STR_SZ 32

enum Phase {
   Sign, IntPart, FracPart
};

typedef struct StateEnc {
   const char* ePos;
   unsigned sPos;
   const char* eBeg[MAX_NESTING];
   unsigned sBeg[MAX_NESTING];
   int cnt[MAX_NESTING];
   char match[MAX_NESTING];
   int phase[MAX_NESTING];
   int nesting;
   char buf[STR_SZ];
} StateEnc;

static StateEnc* expandStateBuffer
(OSCTXT* pctxt, StateEnc* state, unsigned stateArraySz, unsigned curState)
{
   /* expand state array */
   StateEnc* newState =
      rtxMemAllocArray (pctxt, stateArraySz * 2, StateEnc);

   if (newState == 0)
      return 0;

   OSCRTLSAFEMEMCPY (newState, stateArraySz*2, state + curState,
                 sizeof (StateEnc) * (stateArraySz - curState));

   if (curState > 0)
      OSCRTLSAFEMEMCPY (newState + stateArraySz - curState,
            stateArraySz*2 - (stateArraySz-curState), state,
                    sizeof (StateEnc) * curState);

   if (stateArraySz != STATE_ARRAY_SZ)
      rtxMemFreePtr (pctxt, state);

   return newState;
}

EXTXMLMETHOD int rtXmlEncDecimalPatternValue
(OSCTXT* pctxt, OSREAL value, const OSUTF8CHAR* pattern)
{
   OSREAL diff;
   int stat = 0;
   StateEnc stateArray[STATE_ARRAY_SZ];
   unsigned stateArraySz = STATE_ARRAY_SZ;
   StateEnc* state = stateArray;
   unsigned curState = 0;
   unsigned nmStates = 1;

   int lastState;
   StateEnc* lst;

   StateEnc* st = state;
   const char* pe = st->ePos = st->eBeg[0] = (const char*) pattern;
   unsigned ps = st->sPos = st->sBeg[0] = 0;
   char* pstr = st->buf;

   OSBOOL skipAtoms;

   OSCRTLMEMSET (st->buf, 0, sizeof (st->buf));

   if (value < 0.) {
      value = -value;
      *pstr++ = '-';
   }

   diff = value * 2E-15;
   value += value * 1E-15;

   pstr +=
      rtxInt64ToCharStr ((OSINT64) value, pstr, sizeof(st->buf) - 2, 0);
   value -= (OSINT64) value;

   if (value > diff) {
      *pstr++ = '.';

      do {
         value *= 10.;
         diff *= 10.;
         *pstr++ = (char) ('0' + (OSUINT32) value);
         value -= (OSUINT32) value;
      } while (value > diff);
   }

   *pstr = 0;

   st->cnt[0] = 0;
   st->nesting = 0;
   st->phase[0] = Sign;

   while (1) {
      char ce = *pe++;
      char cs = st->buf[ps];
      int match = -1;
      int charMatch = -1;

      if (ce == 0 && cs == 0) {
         stat = rtXmlWriteChars (pctxt, (OSUTF8CHAR*)st->buf,
                                 (int) OSCRTLSTRLEN(st->buf));

         if (state != stateArray)
            rtxMemFreePtr (pctxt, state);

         if (stat != 0) LOG_RTERR (pctxt, stat);
         return stat;
      }

      if (ce == '\\') {
         ce = *pe++;

         if (cs == 0) {
            if (st->phase[st->nesting] == FracPart)
               st->buf[ps] = cs = '0';
            else if (st->phase[st->nesting] == IntPart) {
               st->buf[ps] = cs = '.';
               st->phase[st->nesting] = FracPart;
            }
         }

         switch (ce) {
            case 'd':
               match = (cs >= '0' && cs <= '9');
               break;
            case 'w':
               break;
            case 's':
               match = (cs == ' ');
               break;
         }

         charMatch = match;
      }
      else if (ce == '|') {
         int nmPar = 0;
         unsigned sBeg = st->sBeg[st->nesting];

         if (nmStates == stateArraySz) {
            st = expandStateBuffer (pctxt, state, stateArraySz, curState);
            if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

            curState = 0;
            state = st;
            stateArraySz *= 2;
         }

         lastState = (curState + nmStates) % stateArraySz;
         lst = state + lastState;

         OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
         lst->ePos = pe;
         lst->sPos = sBeg;
         nmStates++;

         /*  skip atoms */
         while (1) {
            ce = *pe++;

            if (ce == 0) {
               pe--;
               break;
            }
            else if (ce == '\\')
               pe++;
            else if (ce == '(')
               nmPar++;
            else if (ce == '[')
               nmPar++;
            else if (ce == ']')
               nmPar--;
            else if (ce == ')') {
               if (nmPar == 0) {
                  pe--;
                  break;
               }
               else
                  nmPar--;
            }
            else if (ce == '|') {
               if (nmStates == stateArraySz) {
                  st = expandStateBuffer (pctxt, state, stateArraySz,
                                          curState);
                  if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  curState = 0;
                  state = st;
                  stateArraySz *= 2;
               }

               lastState = (curState + nmStates) % stateArraySz;
               lst = state + lastState;
               OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));

               lst->ePos = pe;
               lst->sPos = sBeg;
               nmStates++;
            }
         }

         st->ePos = pe;
         st->sPos = ps;
         continue;
      }
      else if (ce == '.') {
         match = (cs != '\n' && cs != '\r');
         charMatch = match;
      }
      else if (ce == '(') {
         st->ePos = pe;
         st->sPos = ps;
         st->nesting++;

         if (st->nesting >= MAX_NESTING)
            return LOG_RTERR (pctxt, RTERR_TOODEEP);

         st->eBeg[st->nesting] = pe - 1;
         st->sBeg[st->nesting] = ps;
         st->cnt[st->nesting] = 0;
         st->match[st->nesting] = 1;
         st->phase[st->nesting] = st->phase[st->nesting - 1];
         continue;
      }
      else if (ce == ')') {
         st->phase[st->nesting - 1] = st->phase[st->nesting];
         match = st->match[st->nesting];
         if (match)
            ps--;
         else
            st->sPos = st->sBeg[st->nesting];

         st->ePos = st->eBeg[st->nesting];
         st->nesting--;
      }
      else if (ce == '[') {
         /*  check range */
         int inv = 0;
         match = 0;

         if (cs == 0) {
            if (st->phase[st->nesting] == FracPart)
               st->buf[ps] = cs = '0';
            else if (st->phase[st->nesting] == IntPart) {
               st->buf[ps] = cs = '.';
               st->phase[st->nesting] = FracPart;
            }
         }

         ce = *pe++;
         if (ce == '^') {
            inv = 1;
            ce = *pe++;
         }

         while (ce && ce != ']') {
            if (*pe == '-') {
               char ce1;

               pe++;
               ce1 = *pe++; /* ??? esc */

               if (cs >= ce && cs <= ce1)
                  match = 1;
            }
            else if (ce == cs)
               match = 1;

            if (match) {
               while (*pe && *pe++ != ']');
               break;
            }

            ce = *pe++;
         }

         match ^= inv;
         charMatch = match;
      }

      if (match == -1) { /*  not checked */
         if (cs == 0) {
            if (st->phase[st->nesting] == FracPart)
               st->buf[ps] = cs = '0';
            else if (st->phase[st->nesting] == IntPart) {
               st->buf[ps] = cs = '.';
               st->phase[st->nesting] = FracPart;
            }
         }

         match = (ce == cs);
         charMatch = match;
      }

      if (charMatch == 1) {
         if (st->phase[st->nesting] == Sign) {
            if (cs == '.')
               st->phase[st->nesting] = FracPart;
            else
               st->phase[st->nesting] = IntPart;
         }
         else if (st->phase[st->nesting] == IntPart) {
            if (cs == '.')
               st->phase[st->nesting] = FracPart;
         }
      }
      else if (charMatch == 0) {
         if (st->phase[st->nesting] == Sign && ce == '+' && cs != '-') {
            if (nmStates == stateArraySz) {
               st = expandStateBuffer (pctxt, state, stateArraySz, curState);
               if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

               curState = 0;
               state = st;
               stateArraySz *= 2;
            }

            lastState = (curState + nmStates) % stateArraySz;
            lst = state + lastState;
            OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
            lst->ePos = pe - 1;
            nmStates++;

            OSCRTLSAFEMEMCPY (lst->buf + 1, sizeof(lst->buf) - 1,
                  lst->buf, sizeof (lst->buf) - 1);
            lst->buf[0] = '+';
            lst->phase[lst->nesting] = IntPart;
            lst->sPos = 0;
         }
         else if (st->phase[st->nesting] != FracPart &&
                  (cs == '.' || ce == '0'))
         {
            unsigned off = 0;

            /*  add lead zero and restart */
            pe = st->ePos = st->eBeg[0] = (const char*) pattern;
            ps = st->sPos = st->sBeg[0] = 0;

            st->cnt[0] = 0;
            st->nesting = 0;
            st->phase[0] = Sign;

            if (st->buf[0] == '-' || st->buf[0] == '+')
               off = 1;

            OSCRTLMEMMOVE (st->buf + off + 1, st->buf + off,
                           sizeof (st->buf) - off - 1);

            st->buf[off] = '0';
            continue;
         }
      }

      if (match)
         ps++;

      /*  * + ? {m.n} */
      skipAtoms = FALSE;
      ce = *pe++;
      switch (ce) {
         case '?':
            if (match) {
               if (nmStates == stateArraySz) {
                  st = expandStateBuffer (pctxt, state, stateArraySz,
                                          curState);
                  if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  curState = 0;
                  state = st;
                  stateArraySz *= 2;
               }

               lastState = (curState + nmStates) % stateArraySz;
               lst = state + lastState;
               OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
               lst->ePos = pe;
               nmStates++;

               st->ePos = pe;
               st->sPos = ps;
            }
            else {
               /*  skip optional */
               st->ePos = pe;
               ps = st->sPos;
            }

            break;
         case '+':
            if (match) {
               if (nmStates == stateArraySz) {
                  st = expandStateBuffer (pctxt, state, stateArraySz,
                                          curState);
                  if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  curState = 0;
                  state = st;
                  stateArraySz *= 2;
               }

               lastState = (curState + nmStates) % stateArraySz;
               lst = state + lastState;

               st->cnt[st->nesting]++;
               OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
               lst->sPos = ps;
               nmStates++;

               st->ePos = pe;
               st->sPos = ps;
               st->cnt[st->nesting] = 0;
            }
            else if (st->cnt[st->nesting] > 0) {
               st->ePos = pe;
               ps = st->sPos;
               st->cnt[st->nesting] = 0;
            }
            else
               skipAtoms = TRUE;

            break;
         case '*':
            if (match) {
               if (nmStates == stateArraySz) {
                  st = expandStateBuffer (pctxt, state, stateArraySz,
                                          curState);
                  if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  curState = 0;
                  state = st;
                  stateArraySz *= 2;
               }

               lastState = (curState + nmStates) % stateArraySz;
               lst = state + lastState;
               OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
               lst->sPos = ps;
               nmStates++;

               st->ePos = pe;
               st->sPos = ps;
            }
            else {
               /*  skip optional */
               st->ePos = pe;
               ps = st->sPos;
            }

            break;
         case '{':
            {
            int n = 0;
            int m;

            /*  get n */
            ce = *pe++;
            while (ce >= '0' && ce <= '9') {
               n *= 10;
               n += ce - '0';
               ce = *pe++;
            }

            m = n;

            if (ce == ',') {
               /*  get m */
               ce = *pe++;
               if (ce != '}')
                  m = 0;
               else
                  m = -1;

               while (ce >= '0' && ce <= '9') {
                  m *= 10;
                  m += ce - '0';
                  ce = *pe++;
               }
            }

            if (match) {
               st->cnt[st->nesting]++;

               if (st->cnt[st->nesting] < n) {
                  pe = st->ePos;
                  st->sPos = ps;
               }
               else {
                  if (m == -1 || st->cnt[st->nesting] < m) {
                     if (nmStates == stateArraySz) {
                        st = expandStateBuffer (pctxt, state, stateArraySz,
                                                curState);
                        if (st == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                        curState = 0;
                        state = st;
                        stateArraySz *= 2;
                     }

                     lastState = (curState + nmStates) % stateArraySz;
                     lst = state + lastState;
                     OSCRTLSAFEMEMCPY (lst, sizeof(StateEnc), st, sizeof (StateEnc));
                     lst->sPos = ps;
                     nmStates++;
                  }

                  st->ePos = pe;
                  st->sPos = ps;
                  st->cnt[st->nesting] = 0;
               }
            }
            else if (st->cnt[st->nesting] >= n) {
               /*  skip last pass */
               st->ePos = pe;
               ps = st->sPos;
               st->cnt[st->nesting] = 0;
            }
            else
               skipAtoms = TRUE;

            }
            break;
         default:
            pe--;

            if (match) {
               st->ePos = pe;
               st->sPos = ps;
            }
            else
               skipAtoms = TRUE;

            break;
      }

      if (skipAtoms) {
         int nmPar = 0;

         while (1) {
            ce = *pe++;

            if (ce == 0) {
               pe--;
               break;
            }
            else if (ce == '\\')
               pe++;
            else if (ce == '(' || ce == '[')
               nmPar++;
            else if (ce == ']')
               nmPar--;
            else if (ce == ')') {
               if (nmPar == 0) {
                  pe--;
                  break;
               }
               else
                  nmPar--;
            }
            else if (ce == '|' && nmPar == 0)
               break;
         }

         if (ce == '|') {
            st->ePos = pe;
            ps = st->sBeg[st->nesting];
         }
         else if (ce == ')') {
            st->match[st->nesting] = 0;
         }
         else {
            /*  get next path */
            if (nmStates == 1) {
               if (state != stateArray)
                  rtxMemFreePtr (pctxt, state);

               return LOG_RTERR (pctxt, RTERR_BADVALUE); /*  match failed */
            }

            curState = (curState + 1) % stateArraySz;
            nmStates--;

            st = state + curState;
            pe = st->ePos;
            ps = st->sPos;
         }
      }
   }
}

/******************* test ******************/

#ifdef TEST
#include <stdio.h>

void testEncode (OSCTXT* pctxt, OSREAL value, const char* pattern)
{
   OSOCTET msgbuf[100];
   int stat;

   rtXmlSetEncBufPtr (pctxt, msgbuf, sizeof(msgbuf));
   stat = rtXmlEncDecimalPatternValue (pctxt, value, OSUTF8(pattern));
   if (stat == 0)
      printf ("OK value = %.15f pattern = %s result = %.*s\n",
              value, pattern, pctxt->buffer.byteIndex, pctxt->buffer.data);
   else {
      printf ("Failed value = %.15f pattern = %s\n", value, pattern);
      rtxErrPrint (pctxt);
   }
}

int main ()
{
   OSCTXT ctxt;

   /* Create a new context structure */
   if (rtxInitContext (&ctxt) != 0) return -1;

   testEncode (&ctxt, 12.34567, "(+|-)\\d{3}\\.\\d{6}");
   testEncode (&ctxt, 1.5, "1\\.\\d{6}");
   testEncode (&ctxt, 1.5, "1\\.\\d{3,6}");
   testEncode (&ctxt, 1.567, "1\\.\\d{3,6}");
   testEncode (&ctxt, 1.56789, "1\\.\\d{3,6}");
   testEncode (&ctxt, 1.567891, "1\\.\\d{3,6}");
   testEncode (&ctxt, 2.5, "0+2\\.\\d{5}");
   testEncode (&ctxt, 1.5, "1\\.\\d{6}|0+2\\.\\d{5}");
   testEncode (&ctxt, 2.5, "1\\.\\d{6}|0+2\\.\\d{5}");
   testEncode (&ctxt, 2.5, "1\\.\\d{6}|\\+0+2\\.\\d{5}");
   testEncode (&ctxt, 11, "(1|2)(1|2)\\.(0|2)(0|2)");
   /* test expanding of state array */
   testEncode (&ctxt, 111111211111,
      "(0|2)(0|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)(1|2)\\."
      "(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)(0|2)");

   rtxFreeContext (&ctxt);
   return 0;
}
#endif
