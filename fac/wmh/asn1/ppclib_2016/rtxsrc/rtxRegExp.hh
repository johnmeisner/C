/**
 * @file rtxRegExp.hh
 */

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

 Copyright (C) 1998-2003 Daniel Veillard.  All Rights Reserved.

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
/*
 * Changes made to original libxml source file (xmlregexp.c) by 
 * Objective Systems, Inc are as follows:
 *
 * 1. Changed to fit Objective Systems run-time environment including 
 * common type name changes and use of OSys mem mgmt.
 *
 * 2. Name change from xmlregexp to rtxRegExp to reflect fact that the 
 * code will be used in other non-XML environments.
 */
#ifndef __RTXREGEXP_HH__
#define __RTXREGEXP_HH__

#include <stdio.h>
#include <string.h>
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxRegExp.h"
#include "rtxsrc/rtxUnicode.h"

/* #define DEBUG_REGEXP_GRAPH  */
/* #define DEBUG_REGEXP_EXEC */
/* #define DEBUG_PUSH */
/* #define DEBUG_COMPACTION */

#define MAX_PUSH 100000

#define REGEXPERROR(str) ctxt->error = 1;				\
LOG_RTERRNEW (ctxt->pOSCTXT, RTERR_REGEXP); \
rtxErrAddStrParm (ctxt->pOSCTXT, (const char*)str);                     \
rtxErrAddStrParm (ctxt->pOSCTXT, (const char*)ctxt->cur);

#define NEXT ctxt->cur++
#define CUR (*(ctxt->cur))
#define NXT(index) (ctxt->cur[index])

#define CUR_SCHAR(s, l) rtxUTF8CharToWC (s, (OSUINT32*)&l)
#define NEXTL(l) ctxt->cur += l;

/*
 * TODO:
 *
 * macro to flag unimplemented blocks
 */
#define TODO \
fprintf (stderr, "Unimplemented block at %s:%d\n", __FILE__, __LINE__)

/************************************************************************
 * 									*
 * 			Datatypes and structures			*
 * 									*
 ************************************************************************/

typedef enum {
    XML_REGEXP_NONE = 0,
    XML_REGEXP_EPSILON = 1,
    XML_REGEXP_CHARVAL,
    XML_REGEXP_RANGES,
    XML_REGEXP_SUBREG,
    XML_REGEXP_STRING,
    XML_REGEXP_ANYCHAR, /* . */
    XML_REGEXP_ANYSPACE, /* \s */
    XML_REGEXP_NOTSPACE, /* \S */
    XML_REGEXP_INITNAME, /* \l */
    XML_REGEXP_NOTINITNAME, /* \l */
    XML_REGEXP_NAMECHAR, /* \c */
    XML_REGEXP_NOTNAMECHAR, /* \C */
    XML_REGEXP_DECIMAL, /* \d */
    XML_REGEXP_NOTDECIMAL, /* \d */
    XML_REGEXP_REALCHAR, /* \w */
    XML_REGEXP_NOTREALCHAR, /* \w */
    XML_REGEXP_LETTER,
    XML_REGEXP_LETTER_UPPERCASE,
    XML_REGEXP_LETTER_LOWERCASE,
    XML_REGEXP_LETTER_TITLECASE,
    XML_REGEXP_LETTER_MODIFIER,
    XML_REGEXP_LETTER_OTHERS,
    XML_REGEXP_MARK,
    XML_REGEXP_MARK_NONSPACING,
    XML_REGEXP_MARK_SPACECOMBINING,
    XML_REGEXP_MARK_ENCLOSING,
    XML_REGEXP_NUMBER,
    XML_REGEXP_NUMBER_DECIMAL,
    XML_REGEXP_NUMBER_LETTER,
    XML_REGEXP_NUMBER_OTHERS,
    XML_REGEXP_PUNCT,
    XML_REGEXP_PUNCT_CONNECTOR,
    XML_REGEXP_PUNCT_DASH,
    XML_REGEXP_PUNCT_OPEN,
    XML_REGEXP_PUNCT_CLOSE,
    XML_REGEXP_PUNCT_INITQUOTE,
    XML_REGEXP_PUNCT_FINQUOTE,
    XML_REGEXP_PUNCT_OTHERS,
    XML_REGEXP_SEPAR,
    XML_REGEXP_SEPAR_SPACE,
    XML_REGEXP_SEPAR_LINE,
    XML_REGEXP_SEPAR_PARA,
    XML_REGEXP_SYMBOL,
    XML_REGEXP_SYMBOL_MATH,
    XML_REGEXP_SYMBOL_CURRENCY,
    XML_REGEXP_SYMBOL_MODIFIER,
    XML_REGEXP_SYMBOL_OTHERS,
    XML_REGEXP_OTHER,
    XML_REGEXP_OTHER_CONTROL,
    XML_REGEXP_OTHER_FORMAT,
    XML_REGEXP_OTHER_PRIVATE,
    XML_REGEXP_OTHER_NA,
    XML_REGEXP_BLOCK_NAME
} rtxRegAtomType;

typedef enum {
    XML_REGEXP_QUANT_EPSILON = 1,
    XML_REGEXP_QUANT_ONCE,
    XML_REGEXP_QUANT_OPT,
    XML_REGEXP_QUANT_MULT,
    XML_REGEXP_QUANT_PLUS,
    XML_REGEXP_QUANT_ONCEONLY,
    XML_REGEXP_QUANT_ALL,
    XML_REGEXP_QUANT_RANGE
} rtxRegQuantType;

typedef enum {
    XML_REGEXP_START_STATE = 1,
    XML_REGEXP_FINAL_STATE,
    XML_REGEXP_TRANS_STATE
} rtxRegStateType;

typedef enum {
    XML_REGEXP_MARK_NORMAL = 0,
    XML_REGEXP_MARK_START,
    XML_REGEXP_MARK_VISITED
} rtxRegMarkedType;

typedef struct _rtxRegRange rtxRegRange;
typedef rtxRegRange *rtxRegRangePtr;

struct _rtxRegRange {
    int neg; /* 0 normal, 1 not, 2 exclude */
    rtxRegAtomType type;
    int start;
    int end;
    OSUTF8CHAR *blockName;
};

typedef struct _rtxRegAtom rtxRegAtom;
typedef rtxRegAtom *rtxRegAtomPtr;

typedef struct _xmlAutomataState rtxRegState;
typedef rtxRegState *rtxRegStatePtr;

struct _rtxRegAtom {
    int no;
    rtxRegAtomType type;
    rtxRegQuantType quant;
    int min;
    int max;

    void *valuep;
    void *valuep2;
    int neg;
    int codepoint;
    rtxRegStatePtr start;
    rtxRegStatePtr stop;
    int maxRanges;
    int nbRanges;
    rtxRegRangePtr *ranges;
    void *data;

    /* extension of standard Atom */
    rtxRegQuantType origQuant;
    int origMin;
    int origMax;
};

typedef struct _rtxRegCounter rtxRegCounter;
typedef rtxRegCounter *rtxRegCounterPtr;

struct _rtxRegCounter {
    int min;
    int max;
};

typedef struct _rtxRegTrans rtxRegTrans;
typedef rtxRegTrans *rtxRegTransPtr;

struct _rtxRegTrans {
    rtxRegAtomPtr atom;
    int to;
    int counter;
    int count;
};

struct _xmlAutomataState {
    rtxRegStateType type;
    rtxRegMarkedType mark;
    rtxRegMarkedType reached;
    int no;

    int maxTrans;
    int nbTrans;
    rtxRegTrans *trans;
};

typedef struct _xmlAutomata rtxRegParserCtxt;
typedef rtxRegParserCtxt *rtxRegParserCtxtPtr;

struct _xmlAutomata {
    OSUTF8CHAR *string;
    OSUTF8CHAR *cur;

    int error;
    int neg;

    rtxRegStatePtr start;
    rtxRegStatePtr end;
    rtxRegStatePtr state;

    rtxRegAtomPtr atom;

    int maxAtoms;
    int nbAtoms;
    rtxRegAtomPtr *atoms;

    int maxStates;
    int nbStates;
    rtxRegStatePtr *states;

    int maxCounters;
    int nbCounters;
    rtxRegCounter *counters;

    int determinist;

   /* added by Objective Systems (ED, 3/28/2003) */
   OSCTXT* pOSCTXT;
};

struct _rtxRegexp {
    OSUTF8CHAR *string;
    int nbStates;
    rtxRegStatePtr *states;
    int nbAtoms;
    rtxRegAtomPtr *atoms;
    int nbCounters;
    rtxRegCounter *counters;
    int determinist;
    /*
     * That's the compact form for determinists automatas
     */
    int nbstates;
    int *compact;
    void **transdata;
    int nbstrings;
    OSUTF8CHAR **stringMap;
};

typedef struct _rtxRegExecRollback rtxRegExecRollback;
typedef rtxRegExecRollback *rtxRegExecRollbackPtr;

struct _rtxRegExecRollback {
    rtxRegStatePtr state;/* the current state */
    int index;		/* the index in the input stack */
    int nextbranch;	/* the next transition to explore in that state */
    int *counts;	/* save the automate state if it has some */
};

typedef struct _rtxRegInputToken rtxRegInputToken;
typedef rtxRegInputToken *rtxRegInputTokenPtr;

struct _rtxRegInputToken {
    OSUTF8CHAR *value;
    void *data;
};

struct _rtxRegExecCtxt {
    int status;		/* execution status != 0 indicate an error */
    int determinist;	/* did we found an inderterministic behaviour */
    rtxRegexpPtr comp;	/* the compiled regexp */
    rtxRegExecCallbacks callback;
    void *data;

    rtxRegStatePtr state;/* the current state */
    int transno;	/* the current transition on that state */
    int transcount;	/* the number of char in char counted transitions */

    /*
     * A stack of rollback states
     */
    int maxRollbacks;
    int nbRollbacks;
    rtxRegExecRollback *rollbacks;

    /*
     * The state of the automata if any
     */
    int *counts;

    /*
     * The input stack
     */
    int inputStackMax;
    int inputStackNr;
    int index;
    int *charStack;
    const OSUTF8CHAR *inputString; /* when operating on characters */
    rtxRegInputTokenPtr inputStack;/* when operating on strings */

    int nbPush;

   /* added by Objective Systems (ED, 3/28/2003) */
   OSCTXT* pOSCTXT;
};

#define REGEXP_ALL_COUNTER	0x123456
#define REGEXP_ALL_LAX_COUNTER	0x123457

#endif /* __RTXREGEXP_HH__ */
