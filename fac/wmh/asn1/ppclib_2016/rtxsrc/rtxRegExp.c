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
 *
 * 3. Moved print utility functions in separate files as fprintf is not
 * always available on embedded systems.
 *
 * This is revision 1.3 (04/12/2004). It reflects all changes till 
 * LIBXML2 version 2.6.8-CVS by 04/12/2004.
 *
 * History:
 * Revision 1.3.e (03/21/2011). Changed to use system memory allocations,
 *    rather than OSys memory managed allocations, the reason being that
 *    the regular expression cache should survive calls to rtxMemFree and
 *    rtxMemReset.
 * Revision 1.3.d (06/19/2006). Moved print utility functions in dedicated file.
 * Revision 1.3b (11/04/2005). Added fix for regexp "(\d{1,3}\.){3}\d{1,3}"
 * (from libxml2 2.6.23). All remaining logic
 * is still from libxml2 2.6.11. 
 * Revision 1.3a (09/16/2005). Added fix for complicated regexps
 * which caused hang (from libxml2 2.6.23). All remaining logic
 * is still from libxml2 2.6.11. 
 * Revision 1.3 (07/20/2004). It reflects all changes till 
 * LIBXML2 version 2.6.11.
 */
#include "rtxsrc/rtxRegExp.hh"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

static void xmlFAParseRegExp (rtxRegParserCtxtPtr ctxt, int top);
static void rtxRegFreeState (OSCTXT* pOSCTXT, rtxRegStatePtr state);
static void rtxRegFreeAtom (OSCTXT* pOSCTXT, rtxRegAtomPtr atom);

EXTRTMETHOD OSUTF8CHAR* rtxRegexpStrdup
(OSCTXT* pctxt, const OSUTF8CHAR* str)
{
   if (0 != str) {
      size_t nbytes = OSUTF8LEN(str) + 1;
      OSUTF8CHAR* pbuf = rtxMemSysAllocArray (pctxt, nbytes, OSUTF8CHAR);
      if (0 != pbuf) {
         OSCRTLSAFEMEMCPY (pbuf, nbytes, str, nbytes);
      }
      return pbuf;
   }
   else return 0;
}

EXTRTMETHOD OSUTF8CHAR* rtxRegexpStrndup
(OSCTXT* pctxt, const OSUTF8CHAR* str, size_t nbytes)
{
   if (str != 0) {
      OSUTF8CHAR* pbuf = rtxMemSysAllocArray (pctxt, nbytes+1, OSUTF8CHAR);
      if (0 != pbuf) {
         OSCRTLSAFEMEMCPY (pbuf, nbytes, str, nbytes);
         pbuf[nbytes] = 0;
      }
      return pbuf;
   }
   else return 0;
} 


/************************************************************************
 * 									*
 * 			Allocation/Deallocation				*
 * 									*
 ************************************************************************/
/*
 * rtxRegEpxFromParse:
 * @ctxt:  the parser context used to build it
 *
 * Allocate a new regexp and fill it with the reult from the parser
 *
 * Returns the new regexp or NULL in case of error
 */
static rtxRegexpPtr
rtxRegEpxFromParse(rtxRegParserCtxtPtr ctxt) {
    rtxRegexpPtr ret;

    ret = rtxMemSysAllocTypeZ (ctxt->pOSCTXT, rtxRegexp);
    if (ret == 0)
       return 0;
    OSCRTLMEMSET(ret, 0, sizeof(rtxRegexp));
    ret->string = ctxt->string;
    ret->nbStates = ctxt->nbStates;
    ret->states = ctxt->states;
    ret->nbAtoms = ctxt->nbAtoms;
    ret->atoms = ctxt->atoms;
    ret->nbCounters = ctxt->nbCounters;
    ret->counters = ctxt->counters;
    ret->determinist = ctxt->determinist;

    if ((ret->determinist != 0) &&
	(ret->nbCounters == 0) &&
	(ret->atoms != NULL) &&
	(ret->atoms[0] != NULL) &&
	(ret->atoms[0]->type == XML_REGEXP_STRING)) {
	int i, j, nbstates = 0, nbatoms = 0;
	int *stateRemap;
	int *stringRemap;
	int *transitions;
	void **transdata;
	OSUTF8CHAR **stringMap;
        OSUTF8CHAR *value;

	/*
	 * Switch to a compact representation
	 * 1/ counting the effective number of states left
	 * 2/ conting the unique number of atoms, and check that
	 *    they are all of the string type
	 * 3/ build a table state x atom for the transitions
	 */
	stateRemap = 
           rtxMemSysAllocArray (ctxt->pOSCTXT, ret->nbStates, int);
        if (stateRemap == 0)
           return NULL;
	for (i = 0;i < ret->nbStates;i++) {
	    if (ret->states[i] != NULL) {
		stateRemap[i] = nbstates;
		nbstates++;
	    } else {
		stateRemap[i] = -1;
	    }
	}
#ifdef DEBUG_COMPACTION
	printf("Final: %d states\n", nbstates);
#endif
	stringMap = 
           rtxMemSysAllocArray (ctxt->pOSCTXT, ret->nbAtoms, OSUTF8CHAR*);
        if (stringMap == 0)
           return NULL;

	stringRemap =
           rtxMemSysAllocArray (ctxt->pOSCTXT, ret->nbAtoms, int);
        if (stringRemap == 0)
           return NULL;

	for (i = 0;i < ret->nbAtoms;i++) {
	    if ((ret->atoms[i]->type == XML_REGEXP_STRING) &&
		(ret->atoms[i]->quant == XML_REGEXP_QUANT_ONCE)) {
		value = (OSUTF8CHAR*)ret->atoms[i]->valuep;
                for (j = 0;j < nbatoms;j++) {
		    if (rtxUTF8StrEqual(stringMap[j], value)) {
			stringRemap[i] = j;
			break;
		    }
		}
		if (j >= nbatoms) {
		    stringRemap[i] = nbatoms;
		    stringMap[nbatoms] = rtxRegexpStrdup (ctxt->pOSCTXT, value);
		    if (stringMap[nbatoms] == NULL) {
        		rtxMemSysFreeArray (ctxt->pOSCTXT, stateRemap);
        		rtxMemSysFreeArray (ctxt->pOSCTXT, stringRemap);
        		for (i = 0;i < nbatoms;i++)
        		    rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap[i]);
        		rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap);
        		rtxMemSysFreeArray (ctxt->pOSCTXT, ret);
        		return NULL;
		    }
		    nbatoms++;
		}
	    } else {
      		rtxMemSysFreeArray (ctxt->pOSCTXT, stateRemap);
      		rtxMemSysFreeArray (ctxt->pOSCTXT, stringRemap);
		for (i = 0;i < nbatoms;i++)
      		    rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap[i]);
      		rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap);
      		rtxMemSysFreeArray (ctxt->pOSCTXT, ret);
		return NULL;
	    }
	}
#ifdef DEBUG_COMPACTION
	printf("Final: %d atoms\n", nbatoms);
#endif

	/*
	 * Allocate the transition table. The first entry for each
	 * state correspond to the state type.
	 */
	transitions = 
           rtxMemSysAllocArray (ctxt->pOSCTXT, (nbstates + 1) * 
                                      (nbatoms + 1), int);
	if (transitions == NULL) {
           rtxMemSysFreeArray (ctxt->pOSCTXT, stateRemap);
           rtxMemSysFreeArray (ctxt->pOSCTXT, stringRemap);
           rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap);
           rtxMemSysFreeType (ctxt->pOSCTXT, ret);
	   return NULL;
	}
	transdata = NULL;
	OSCRTLMEMSET(transitions, 0, (nbstates + 1) * (nbatoms + 1) * sizeof(int));

	for (i = 0;i < ret->nbStates;i++) {
	    int stateno, atomno, targetno, prev;
	    rtxRegStatePtr state;
	    rtxRegTransPtr trans;

	    stateno = stateRemap[i];
	    if (stateno == -1)
		continue;
	    state = ret->states[i];

	    transitions[stateno * (nbatoms + 1)] = state->type;

	    for (j = 0;j < state->nbTrans;j++) {
		trans = &(state->trans[j]);
		if ((trans->to == -1) || (trans->atom == NULL))
		    continue;
                atomno = stringRemap[trans->atom->no];
		if ((trans->atom->data != NULL) && (transdata == NULL)) {
		    transdata = 
                       rtxMemSysAllocArray (ctxt->pOSCTXT, nbstates * nbatoms,
                                             void*);
		    if (transdata != NULL)
			OSCRTLMEMSET(transdata, 0,
			       nbstates * nbatoms * sizeof(void *));
		    else
		       break;
		}
		targetno = stateRemap[trans->to];
		/*
		 * if the same atome can generate transition to 2 different
		 * states then it means the automata is not determinist and
		 * the compact form can't be used !
		 */
		prev = transitions[stateno * (nbatoms + 1) + atomno + 1];
		if (prev != 0) {
		    if (prev != targetno + 1) {
			ret->determinist = 0;
#ifdef DEBUG_COMPACTION
			printf("Indet: state %d trans %d, atom %d to %d : %d to %d\n",
			       i, j, trans->atom->no, trans->to, atomno, targetno);
			printf("       previous to is %d\n", prev);
#endif
			ret->determinist = 0;
			if (transdata != NULL)
			    rtxMemSysFreeArray (ctxt->pOSCTXT, transdata);
			rtxMemSysFreeArray (ctxt->pOSCTXT, transitions);
			rtxMemSysFreeArray (ctxt->pOSCTXT, stateRemap);
			rtxMemSysFreeArray (ctxt->pOSCTXT, stringRemap);
			for (i = 0;i < nbatoms;i++)
			    rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap[i]);
			rtxMemSysFreeArray (ctxt->pOSCTXT, stringMap);
			goto not_determ;
		    }
		} else {
#if 0
		    printf("State %d trans %d: atom %d to %d : %d to %d\n",
			   i, j, trans->atom->no, trans->to, atomno, targetno);
#endif
		    transitions[stateno * (nbatoms + 1) + atomno + 1] =
			targetno + 1; /* to avoid 0 */
		    if (transdata != NULL)
			transdata[stateno * nbatoms + atomno] =
			    trans->atom->data;
		}
	    }
	}
	ret->determinist = 1;
#ifdef DEBUG_COMPACTION
	/*
	 * Debug
	 */
	for (i = 0;i < nbstates;i++) {
	    for (j = 0;j < nbatoms + 1;j++) {
                printf("%02d ", transitions[i * (nbatoms + 1) + j]);
	    }
	    printf("\n");
	}
	printf("\n");
#endif
	/*
	 * Cleanup of the old data
	 */
	if (ret->states != NULL) {
	    for (i = 0;i < ret->nbStates;i++)
		rtxRegFreeState (ctxt->pOSCTXT, ret->states[i]);

	    rtxMemSysFreeArray (ctxt->pOSCTXT, ret->states);
	}
	ret->states = NULL;
	ret->nbStates = 0;

	if (ret->atoms != NULL) {
	    for (i = 0;i < ret->nbAtoms;i++)
		rtxRegFreeAtom (ctxt->pOSCTXT, ret->atoms[i]);

	    rtxMemSysFreeArray (ctxt->pOSCTXT, ret->atoms);
	}
	ret->atoms = NULL;
	ret->nbAtoms = 0;

	ret->compact = transitions;
	ret->transdata = transdata;
	ret->stringMap = stringMap;
	ret->nbstrings = nbatoms;
	ret->nbstates = nbstates;
	rtxMemSysFreeArray (ctxt->pOSCTXT, stateRemap);
	rtxMemSysFreeArray (ctxt->pOSCTXT, stringRemap);
    }
not_determ:
    ctxt->string = NULL;
    ctxt->nbStates = 0;
    ctxt->states = NULL;
    ctxt->nbAtoms = 0;
    ctxt->atoms = NULL;
    ctxt->nbCounters = 0;
    ctxt->counters = NULL;
    return(ret);
}

/*
 * rtxRegNewParserCtxt:
 * @string:  the string to parse
 *
 * Allocate a new regexp parser context
 *
 * Returns the new context or NULL in case of error
 */
static rtxRegParserCtxtPtr
rtxRegNewParserCtxt (OSCTXT* pOSCTXT, const OSUTF8CHAR *string) 
{
    rtxRegParserCtxtPtr ret;

    ret = rtxMemSysAllocTypeZ (pOSCTXT, rtxRegParserCtxt);
    if (string != NULL)
       ret->string = rtxRegexpStrdup (pOSCTXT, string);
    ret->cur = ret->string;
    ret->neg = 0;
    ret->error = 0;
    ret->determinist = -1;
    ret->pOSCTXT = pOSCTXT;

    return(ret);
}

/*
 * rtxRegNewRange:
 * @ctxt:  the regexp parser context
 * @neg:  is that negative
 * @type:  the type of range
 * @start:  the start codepoint
 * @end:  the end codepoint
 *
 * Allocate a new regexp range
 *
 * Returns the new range or NULL in case of error
 */
static rtxRegRangePtr
rtxRegNewRange(rtxRegParserCtxtPtr ctxt,
	       int neg, rtxRegAtomType type, int start, int end) {
    rtxRegRangePtr ret;

    ret = rtxMemSysAllocType (ctxt->pOSCTXT, rtxRegRange);
    if (ret == NULL) {
	REGEXPERROR("failed to allocate regexp range");
	return(NULL);
    }
    ret->neg = neg;
    ret->type = type;
    ret->start = start;
    ret->end = end;
    return(ret);
}

/*
 * rtxRegFreeRange:
 * @range:  the regexp range
 *
 * Free a regexp range
 */
static void rtxRegFreeRange (OSCTXT* pOSCTXT, rtxRegRangePtr range) 
{
   if (range == NULL)
      return;

   if (range->blockName != NULL)
      rtxMemSysFreeArray (pOSCTXT, range->blockName);

   rtxMemSysFreeArray (pOSCTXT, range);
}

/*
 * rtxRegNewAtom:
 * @ctxt:  the regexp parser context
 * @type:  the type of atom
 *
 * Allocate a new regexp range
 *
 * Returns the new atom or NULL in case of error
 */
static rtxRegAtomPtr
rtxRegNewAtom(rtxRegParserCtxtPtr ctxt, rtxRegAtomType type) {
    rtxRegAtomPtr ret;

    ret = rtxMemSysAllocType (ctxt->pOSCTXT, rtxRegAtom);
    if (ret == NULL) {
	REGEXPERROR("failed to allocate regexp atom");
	return(NULL);
    }
    OSCRTLMEMSET(ret, 0, sizeof(rtxRegAtom));
    ret->type = type;
    ret->quant = XML_REGEXP_QUANT_ONCE;
    ret->origQuant = ret->quant; /*!AB, 01/05/05 */
    ret->min = 0;
    ret->max = 0;
    return(ret);
}

/*
 * rtxRegFreeAtom:
 * @atom:  the regexp atom
 *
 * Free a regexp atom
 */
static void rtxRegFreeAtom (OSCTXT* pOSCTXT, rtxRegAtomPtr atom) 
{
    int i;

    if (atom == NULL)
	return;

    for (i = 0;i < atom->nbRanges;i++)
	rtxRegFreeRange (pOSCTXT, atom->ranges[i]);

    if (atom->ranges != NULL)
	rtxMemSysFreeArray (pOSCTXT, atom->ranges);

    if (atom->type == XML_REGEXP_STRING)
	rtxMemSysFreeArray (pOSCTXT, atom->valuep);

    rtxMemSysFreeType (pOSCTXT, atom);
}

static rtxRegStatePtr
rtxRegNewState(rtxRegParserCtxtPtr ctxt) {
    rtxRegStatePtr ret;

    ret = rtxMemSysAllocType (ctxt->pOSCTXT, rtxRegState);
    if (ret == NULL) {
	REGEXPERROR("failed to allocate regexp state");
	return(NULL);
    }
    OSCRTLMEMSET(ret, 0, sizeof(rtxRegState));
    ret->type = XML_REGEXP_TRANS_STATE;
    ret->mark = XML_REGEXP_MARK_NORMAL;
    return(ret);
}

/*
 * rtxRegFreeState:
 * @state:  the regexp state
 *
 * Free a regexp state
 */
static void rtxRegFreeState (OSCTXT* pOSCTXT, rtxRegStatePtr state) 
{
   if (state == NULL)
      return;

   if (state->trans != NULL)
      rtxMemSysFreeArray (pOSCTXT, state->trans);

   rtxMemSysFreeType (pOSCTXT, state);
}

/*
 * rtxRegFreeParserCtxt:
 * @ctxt:  the regexp parser context
 *
 * Free a regexp parser context
 */
static void rtxRegFreeParserCtxt (rtxRegParserCtxtPtr ctxt) 
{
    int i;
    if (ctxt == NULL)
	return;

    if (ctxt->string != NULL)
	rtxMemSysFreeArray (ctxt->pOSCTXT, ctxt->string);

    if (ctxt->states != NULL) {
	for (i = 0;i < ctxt->nbStates;i++)
	    rtxRegFreeState (ctxt->pOSCTXT, ctxt->states[i]);

	rtxMemSysFreeArray (ctxt->pOSCTXT, ctxt->states);
    }

    if (ctxt->atoms != NULL) {
	for (i = 0;i < ctxt->nbAtoms;i++)
	    rtxRegFreeAtom (ctxt->pOSCTXT, ctxt->atoms[i]);

	rtxMemSysFreeArray (ctxt->pOSCTXT, ctxt->atoms);
    }

    if (ctxt->counters != NULL)
	rtxMemSysFreeArray (ctxt->pOSCTXT, ctxt->counters);

    rtxMemSysFreeType (ctxt->pOSCTXT, ctxt);
}

/************************************************************************
 *                                                                      *
 *	     Display of Data structures                                      *
 *                                                                      *
 ************************************************************************/

/* Moved to rtxRegExpPrint.c */

/************************************************************************
 *                                                                      *
 *      Finite Automata structures manipulations                        *
 *                                                                      *
 ************************************************************************/

static void 
rtxRegAtomAddRange(rtxRegParserCtxtPtr ctxt, rtxRegAtomPtr atom,
	           int neg, rtxRegAtomType type, int start, int end,
		   OSUTF8CHAR *blockName) {
    rtxRegRangePtr range;

    if (atom == NULL) {
	REGEXPERROR("add range: atom is NULL");
	return;
    }
    if (atom->type != XML_REGEXP_RANGES) {
	REGEXPERROR("add range: atom is not ranges");
	return;
    }
    if (atom->maxRanges == 0) {
	atom->maxRanges = 4;

	atom->ranges = rtxMemSysAllocArray
           (ctxt->pOSCTXT, atom->maxRanges, rtxRegRangePtr);

	if (atom->ranges == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    atom->maxRanges = 0;
	    return;
	}
    } 
    else if (atom->nbRanges >= atom->maxRanges) {
	rtxRegRangePtr *tmp;
	atom->maxRanges *= 2;

	tmp = (rtxRegRangePtr*) rtxMemSysRealloc 
           (ctxt->pOSCTXT, atom->ranges, 
            atom->maxRanges * sizeof(rtxRegRangePtr));

	if (tmp == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    atom->maxRanges /= 2;
	    return;
	}
	atom->ranges = tmp;
    }
    range = rtxRegNewRange(ctxt, neg, type, start, end);
    if (range == NULL)
	return;
    range->blockName = blockName;
    atom->ranges[atom->nbRanges++] = range;
    
}

static int
rtxRegGetCounter(rtxRegParserCtxtPtr ctxt) {
    if (ctxt->maxCounters == 0) {
	ctxt->maxCounters = 4;

	ctxt->counters = rtxMemSysAllocArray
           (ctxt->pOSCTXT, ctxt->maxCounters, rtxRegCounter);

	if (ctxt->counters == NULL) {
	    REGEXPERROR("reg counter: allocation failed");
	    ctxt->maxCounters = 0;
	    return(-1);
	}
    } else if (ctxt->nbCounters >= ctxt->maxCounters) {
	rtxRegCounter *tmp;
	ctxt->maxCounters *= 2;

	tmp = (rtxRegCounter*) rtxMemSysRealloc
           (ctxt->pOSCTXT, ctxt->counters, 
            ctxt->maxCounters * sizeof(rtxRegCounter));

	if (tmp == NULL) {
	    REGEXPERROR("reg counter: allocation failed");
	    ctxt->maxCounters /= 2;
	    return(-1);
	}
	ctxt->counters = tmp;
    }
    ctxt->counters[ctxt->nbCounters].min = -1;
    ctxt->counters[ctxt->nbCounters].max = -1;
    return(ctxt->nbCounters++);
}

static int 
rtxRegAtomPush(rtxRegParserCtxtPtr ctxt, rtxRegAtomPtr atom) {
    if (atom == NULL) {
	REGEXPERROR("atom push: atom is NULL");
	return -1;
    }
    if (ctxt->maxAtoms == 0) {
	ctxt->maxAtoms = 4;
	ctxt->atoms = (rtxRegAtomPtr *) 
           rtxMemSysAllocArray (ctxt->pOSCTXT, ctxt->maxAtoms,
                                 rtxRegAtomPtr);
	if (ctxt->atoms == NULL) {
	    REGEXPERROR("atom push: allocation failed");
	    ctxt->maxAtoms = 0;
	    return -1;
	}
    } else if (ctxt->nbAtoms >= ctxt->maxAtoms) {
	rtxRegAtomPtr *tmp;
	ctxt->maxAtoms *= 2;

	tmp = (rtxRegAtomPtr*) rtxMemSysRealloc
           (ctxt->pOSCTXT, ctxt->atoms, 
            ctxt->maxAtoms *sizeof(rtxRegAtomPtr));

	if (tmp == NULL) {
	    REGEXPERROR("atom push: allocation failed");
	    ctxt->maxAtoms /= 2;
	    return -1;
	}
	ctxt->atoms = tmp;
    }
    atom->no = ctxt->nbAtoms;
    ctxt->atoms[ctxt->nbAtoms++] = atom;
    return 0;
}

static void 
rtxRegStateAddTrans(rtxRegParserCtxtPtr ctxt, rtxRegStatePtr state,
	            rtxRegAtomPtr atom, rtxRegStatePtr target,
		    int counter, int count) {
    int nrtrans;

    if (state == NULL) {
	REGEXPERROR("add state: state is NULL");
	return;
    }
    if (target == NULL) {
	REGEXPERROR("add state: target is NULL");
	return;
    }
    /*
     * Other routines follow the philosophy 'When in doubt, add a transition'
     * so we check here whether such a transition is already present and, if
     * so, silently ignore this request.
     */

    for (nrtrans=0; nrtrans<state->nbTrans; nrtrans++) {
        if ((state->trans[nrtrans].atom == atom) &&
            (state->trans[nrtrans].to == target->no) &&
            (state->trans[nrtrans].counter == counter) &&
            (state->trans[nrtrans].count == count)) {
#ifdef DEBUG_REGEXP_GRAPH
            printf("Ignoring duplicate transition from %d to %d\n",
                    state->no, target->no);
#endif
            return;
        }
    }

    if (state->maxTrans == 0) {
	state->maxTrans = 4;
	state->trans = (rtxRegTrans *) 
           rtxMemSysAllocArray (ctxt->pOSCTXT, state->maxTrans,
                                 rtxRegTrans);
	if (state->trans == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    state->maxTrans = 0;
	    return;
	}
    } else if (state->nbTrans >= state->maxTrans) {
	rtxRegTrans *tmp;
	state->maxTrans *= 2;

	tmp = (rtxRegTrans*) rtxMemSysRealloc
           (ctxt->pOSCTXT, state->trans, 
            state->maxTrans * sizeof(rtxRegTrans));

	if (tmp == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    state->maxTrans /= 2;
	    return;
	}
	state->trans = tmp;
    }
#ifdef DEBUG_REGEXP_GRAPH
    printf("Add trans from %d to %d ", state->no, target->no);
    if (count == REGEXP_ALL_COUNTER)
	printf("all transition\n");
    else if (count >= 0)
	printf("count based %d\n", count);
    else if (counter >= 0)
	printf("counted %d\n", counter);
    else if (atom == NULL)
	printf("epsilon transition\n");
    else if (atom != NULL) 
        xmlRegPrintAtom(stdout, atom);
#endif

    state->trans[state->nbTrans].atom = atom;
    state->trans[state->nbTrans].to = target->no;
    state->trans[state->nbTrans].counter = counter;
    state->trans[state->nbTrans].count = count;
    state->nbTrans++;
}

static int 
rtxRegStatePush(rtxRegParserCtxtPtr ctxt, rtxRegStatePtr state) {
    if (ctxt->maxStates == 0) {
	ctxt->maxStates = 4;
	ctxt->states =
           rtxMemSysAllocArray (ctxt->pOSCTXT, ctxt->maxStates,
                                 rtxRegStatePtr);
	if (ctxt->states == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    ctxt->maxStates = 0;
	    return -1;
	}
    } else if (ctxt->nbStates >= ctxt->maxStates) {
	rtxRegStatePtr *tmp;
	ctxt->maxStates *= 2;

	tmp = (rtxRegStatePtr*) rtxMemSysRealloc
           (ctxt->pOSCTXT, ctxt->states, 
            ctxt->maxStates * sizeof(rtxRegStatePtr));

	if (tmp == NULL) {
	    REGEXPERROR("add range: allocation failed");
	    ctxt->maxStates /= 2;
	    return -1;
	}
	ctxt->states = tmp;
    }
    state->no = ctxt->nbStates;
    ctxt->states[ctxt->nbStates++] = state;
    return 0;
}

/*
 * xmlFAGenerateEpsilonTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 *
 */
static void
xmlFAGenerateEpsilonTransition(rtxRegParserCtxtPtr ctxt,
			       rtxRegStatePtr from, rtxRegStatePtr to) {
    if (to == NULL) {
	to = rtxRegNewState(ctxt);
	rtxRegStatePush(ctxt, to);
	ctxt->state = to;
    }
    rtxRegStateAddTrans(ctxt, from, NULL, to, -1, -1);
}

/*
 * xmlFAGenerateCountedEpsilonTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * counter:  the counter for that transition
 *
 */
static void
xmlFAGenerateCountedEpsilonTransition(rtxRegParserCtxtPtr ctxt,
	    rtxRegStatePtr from, rtxRegStatePtr to, int counter) {
    if (to == NULL) {
	to = rtxRegNewState(ctxt);
	rtxRegStatePush(ctxt, to);
	ctxt->state = to;
    }
    rtxRegStateAddTrans(ctxt, from, NULL, to, counter, -1);
}

/*
 * xmlFAGenerateCountedTransition:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * counter:  the counter for that transition
 *
 */
static void
xmlFAGenerateCountedTransition(rtxRegParserCtxtPtr ctxt,
	    rtxRegStatePtr from, rtxRegStatePtr to, int counter) {
    if (to == NULL) {
	to = rtxRegNewState(ctxt);
	rtxRegStatePush(ctxt, to);
	ctxt->state = to;
    }
    rtxRegStateAddTrans(ctxt, from, NULL, to, -1, counter);
}

/*
 * xmlFAGenerateTransitions:
 * @ctxt:  a regexp parser context
 * @from:  the from state
 * @to:  the target state or NULL for building a new one
 * @atom:  the atom generating the transition
 *
 * Returns 0 if succes and -1 in case of error.
 */
static int
xmlFAGenerateTransitions(rtxRegParserCtxtPtr ctxt, rtxRegStatePtr from,
	                 rtxRegStatePtr to, rtxRegAtomPtr atom) {
    if (atom == NULL) {
	REGEXPERROR("genrate transition: atom == NULL");
	return -1;
    }
    if (atom->type == XML_REGEXP_SUBREG) {
	/*
	 * this is a subexpression handling one should not need to
	 * create a new node excep for XML_REGEXP_QUANT_RANGE.
	 */
	if (rtxRegAtomPush(ctxt, atom) < 0)
	   return -1;
	if ((to != NULL) && (atom->stop != to) &&
	    (atom->quant != XML_REGEXP_QUANT_RANGE)) {
	    /*
	     * Generate an epsilon transition to link to the target
	     */
	    xmlFAGenerateEpsilonTransition(ctxt, atom->stop, to);
	}
	switch (atom->quant) {
	    case XML_REGEXP_QUANT_OPT:
		atom->quant = XML_REGEXP_QUANT_ONCE;
		/* YG - transition must be to the state after end of atom.
		        1. set transition from atom start to new state
		        2. set transition from atom end to this state. 
		    xmlFAGenerateEpsilonTransition(ctxt, atom->start, atom->stop);
		*/
		xmlFAGenerateEpsilonTransition(ctxt, atom->start, 0);
		xmlFAGenerateEpsilonTransition(ctxt, atom->stop, ctxt->state);
		break;
	    case XML_REGEXP_QUANT_MULT:
		atom->quant = XML_REGEXP_QUANT_ONCE;
		xmlFAGenerateEpsilonTransition(ctxt, atom->start, atom->stop);
		xmlFAGenerateEpsilonTransition(ctxt, atom->stop, atom->start);
		break;
	    case XML_REGEXP_QUANT_PLUS:
		atom->quant = XML_REGEXP_QUANT_ONCE;
		xmlFAGenerateEpsilonTransition(ctxt, atom->stop, atom->start);
		break;
	    case XML_REGEXP_QUANT_RANGE: {
		int counter;
		rtxRegStatePtr newstate;

		/*
		 * This one is nasty:
		 *   1/ if range has minOccurs == 0, create a new state
		 *	and create epsilon transitions from atom->start
		 *	to atom->stop, as well as atom->start to the new
		 *	state
		 *   2/ register a new counter
		 *   3/ register an epsilon transition associated to
		 *      this counter going from atom->stop to atom->start
		 *   4/ create a new state
		 *   5/ generate a counted transition from atom->stop to
		 *      that state
		 */
		if (atom->min == 0) {
		    xmlFAGenerateEpsilonTransition(ctxt, atom->start,
			atom->stop);
		    newstate = rtxRegNewState(ctxt);
		    rtxRegStatePush(ctxt, newstate);
		    ctxt->state = newstate;
		    xmlFAGenerateEpsilonTransition(ctxt, atom->start,
			newstate);
		}
		counter = rtxRegGetCounter(ctxt);
		ctxt->counters[counter].min = atom->min - 1;
		ctxt->counters[counter].max = atom->max - 1;
		atom->min = 0;
		atom->max = 0;
		atom->quant = XML_REGEXP_QUANT_ONCE;
		/* YG - move this function call down
		xmlFAGenerateCountedEpsilonTransition(ctxt, atom->stop,
			                              atom->start, counter);
		*/
		if (to != NULL) {
		    newstate = to;
		} else {
		    newstate = rtxRegNewState(ctxt);
		    rtxRegStatePush(ctxt, newstate);
		    ctxt->state = newstate;
		}
		xmlFAGenerateCountedTransition(ctxt, atom->stop,
			                       newstate, counter);

                /* YG - first check count and if OK jump forward; 
                   if checking fail increment count and jump back */
		xmlFAGenerateCountedEpsilonTransition(ctxt, atom->stop,
			                              atom->start, counter);
	    }
	    default:
		break;
	}
	return 0;
    } else {
	if (to == NULL) {
	    to = rtxRegNewState(ctxt);
	    if (to != NULL)
               rtxRegStatePush(ctxt, to);
            else {
               return -1;
            }
	}
	if (rtxRegAtomPush(ctxt, atom) < 0) {
	   return -1;
	}
	rtxRegStateAddTrans(ctxt, from, atom, to, -1, -1);
	ctxt->state = to;
    }
    switch (atom->quant) {
	case XML_REGEXP_QUANT_OPT:
	    atom->quant = XML_REGEXP_QUANT_ONCE;
	    xmlFAGenerateEpsilonTransition(ctxt, from, to);
	    break;
	case XML_REGEXP_QUANT_MULT:
	    atom->quant = XML_REGEXP_QUANT_ONCE;
	    xmlFAGenerateEpsilonTransition(ctxt, from, to);
	    rtxRegStateAddTrans(ctxt, to, atom, to, -1, -1);
	    break;
	case XML_REGEXP_QUANT_PLUS:
	    atom->quant = XML_REGEXP_QUANT_ONCE;
	    rtxRegStateAddTrans(ctxt, to, atom, to, -1, -1);
	    break;
	default:
	    break;
    }
    return 0;
}

/*
 * xmlFAReduceEpsilonTransitions:
 * @ctxt:  a regexp parser context
 * @fromnr:  the from state
 * @tonr:  the to state 
 * @cpunter:  should that transition be associted to a counted
 *
 */
static void
xmlFAReduceEpsilonTransitions(rtxRegParserCtxtPtr ctxt, int fromnr,
	                      int tonr, int counter) {
    int transnr;
    rtxRegStatePtr from;
    rtxRegStatePtr to;

#ifdef DEBUG_REGEXP_GRAPH
    printf("xmlFAReduceEpsilonTransitions(%d, %d)\n", fromnr, tonr);
#endif
    from = ctxt->states[fromnr];
    if (from == NULL)
	return;
    to = ctxt->states[tonr];
    if (to == NULL)
	return;
    if ((to->mark == XML_REGEXP_MARK_START) ||
	(to->mark == XML_REGEXP_MARK_VISITED))
	return;

    to->mark = XML_REGEXP_MARK_VISITED;
    if (to->type == XML_REGEXP_FINAL_STATE) {
#ifdef DEBUG_REGEXP_GRAPH
	printf("State %d is final, so %d becomes final\n", tonr, fromnr);
#endif
	from->type = XML_REGEXP_FINAL_STATE;
    }
    for (transnr = 0;transnr < to->nbTrans;transnr++) {
	if (to->trans[transnr].atom == NULL) {
	    /*
	     * Don't remove counted transitions
	     * Don't loop either
	     */
	    if (to->trans[transnr].to != fromnr) {
		if (to->trans[transnr].count >= 0) {
		    int newto = to->trans[transnr].to;

		    rtxRegStateAddTrans(ctxt, from, NULL,
					ctxt->states[newto], 
					-1, to->trans[transnr].count);
		} else {
#ifdef DEBUG_REGEXP_GRAPH
		    printf("Found epsilon trans %d from %d to %d\n",
			   transnr, tonr, to->trans[transnr].to);
#endif
		    if (to->trans[transnr].counter >= 0) {
			xmlFAReduceEpsilonTransitions(ctxt, fromnr,
					      to->trans[transnr].to,
					      to->trans[transnr].counter);
		    } else {
			xmlFAReduceEpsilonTransitions(ctxt, fromnr,
					      to->trans[transnr].to,
					      counter);
		    }
		}
	    }
	} else {
	    int newto = to->trans[transnr].to;

	    if (to->trans[transnr].counter >= 0) {
		rtxRegStateAddTrans(ctxt, from, to->trans[transnr].atom, 
				    ctxt->states[newto], 
				    to->trans[transnr].counter, -1);
	    } else {
		rtxRegStateAddTrans(ctxt, from, to->trans[transnr].atom, 
				    ctxt->states[newto], counter, -1);
	    }
	}
    }
    to->mark = XML_REGEXP_MARK_NORMAL;
}

/*
 * xmlFAEliminateEpsilonTransitions:
 * @ctxt:  a regexp parser context
 *
 */
static void
xmlFAEliminateEpsilonTransitions(rtxRegParserCtxtPtr ctxt) {
    int statenr, transnr;
    rtxRegStatePtr state;

    if (ctxt->states == NULL) return;

    /*
     * build the completed transitions bypassing the epsilons
     * Use a marking algorithm to avoid loops
     */
    for (statenr = 0;statenr < ctxt->nbStates;statenr++) {
	state = ctxt->states[statenr];
	if (state == NULL)
	    continue;
	for (transnr = 0;transnr < state->nbTrans;transnr++) {
	    if ((state->trans[transnr].atom == NULL) &&
		(state->trans[transnr].to >= 0)) {
		if (state->trans[transnr].to == statenr) {
		    state->trans[transnr].to = -1;
#ifdef DEBUG_REGEXP_GRAPH
		    printf("Removed loopback epsilon trans %d on %d\n",
			   transnr, statenr);
#endif
		} else if (state->trans[transnr].count < 0) {
		    int newto = state->trans[transnr].to;

#ifdef DEBUG_REGEXP_GRAPH
		    printf("Found epsilon trans %d from %d to %d\n",
			   transnr, statenr, newto);
#endif
		    state->mark = XML_REGEXP_MARK_START;
		    xmlFAReduceEpsilonTransitions(ctxt, statenr,
				      newto, state->trans[transnr].counter);
		    state->mark = XML_REGEXP_MARK_NORMAL;
#ifdef DEBUG_REGEXP_GRAPH
		} else {
		    printf("Found counted transition %d on %d\n",
			   transnr, statenr);
#endif
	        }
	    }
	}
    }
    /*
     * Eliminate the epsilon transitions
     */
    for (statenr = 0;statenr < ctxt->nbStates;statenr++) {
	state = ctxt->states[statenr];
	if (state == NULL)
	    continue;
	for (transnr = 0;transnr < state->nbTrans;transnr++) {
	    if ((state->trans[transnr].atom == NULL) &&
		(state->trans[transnr].count < 0) &&
		(state->trans[transnr].to >= 0)) {
		state->trans[transnr].to = -1;
	    }
	}
    }

    /*
     * Use this pass to detect unreachable states too
     */
    for (statenr = 0;statenr < ctxt->nbStates;statenr++) {
	state = ctxt->states[statenr];
	if (state != NULL)
	    state->reached = XML_REGEXP_MARK_NORMAL;
    }
    state = ctxt->states[0];
    if (state != NULL)
	state->reached = XML_REGEXP_MARK_START;
    while (state != NULL) {
	rtxRegStatePtr target = NULL;
	state->reached = XML_REGEXP_MARK_VISITED;
	/*
	 * Mark all state reachable from the current reachable state
	 */
	for (transnr = 0;transnr < state->nbTrans;transnr++) {
	    if ((state->trans[transnr].to >= 0) &&
		((state->trans[transnr].atom != NULL) ||
		 (state->trans[transnr].count >= 0))) {
		int newto = state->trans[transnr].to;

		if (ctxt->states[newto] == NULL)
		    continue;
		if (ctxt->states[newto]->reached == XML_REGEXP_MARK_NORMAL) {
		    ctxt->states[newto]->reached = XML_REGEXP_MARK_START;
		    target = ctxt->states[newto];
		}
	    }
	}
	/*
	 * find the next accessible state not explored
	 */
	if (target == NULL) {
	    for (statenr = 1;statenr < ctxt->nbStates;statenr++) {
		state = ctxt->states[statenr];
		if ((state != NULL) && (state->reached == XML_REGEXP_MARK_START)) {
		    target = state;
		    break;
		}
	    }
	}
	state = target;
    }
    for (statenr = 0;statenr < ctxt->nbStates;statenr++) {
	state = ctxt->states[statenr];
	if ((state != NULL) && (state->reached == XML_REGEXP_MARK_NORMAL)) {
#ifdef DEBUG_REGEXP_GRAPH
	    printf("Removed unreachable state %d\n", statenr);
#endif
	    rtxRegFreeState (ctxt->pOSCTXT, state);
	    ctxt->states[statenr] = NULL;
	}
    }

}

/************************************************************************
 * 									*
 *	Routines to check input against transition atoms		*
 * 									*
 ************************************************************************/

static int
rtxRegCheckCharacterRange(rtxRegAtomType type, int codepoint, int neg,
	                  int start, int end, const OSUTF8CHAR *blockName) {
    int ret = 0;

    switch (type) {
        case XML_REGEXP_STRING:
        case XML_REGEXP_SUBREG:
        case XML_REGEXP_RANGES:
        case XML_REGEXP_EPSILON:
	    return(-1);
        case XML_REGEXP_ANYCHAR:
	    ret = ((codepoint != '\n') && (codepoint != '\r'));
	    break;
        case XML_REGEXP_CHARVAL:
	    ret = ((codepoint >= start) && (codepoint <= end));
	    break;
        case XML_REGEXP_NOTSPACE:
	    neg = !neg;
        case XML_REGEXP_ANYSPACE:
	    ret = ((codepoint == '\n') || (codepoint == '\r') ||
		   (codepoint == '\t') || (codepoint == ' '));
	    break;
        case XML_REGEXP_NOTINITNAME:
	    neg = !neg;
        case XML_REGEXP_INITNAME:
	    ret = (rtxUCSIsLetter(codepoint) || 
		   (codepoint == '_') || (codepoint == ':'));
	    break;
        case XML_REGEXP_NOTNAMECHAR:
	    neg = !neg;
        case XML_REGEXP_NAMECHAR:
	    ret = (rtxUCSIsLetter(codepoint) || 
                   rtxUCSIsDigit(codepoint) ||
		   (codepoint == '.') || (codepoint == '-') ||
		   (codepoint == '_') || (codepoint == ':') ||
		   rtxUCSIsCombining(codepoint) || 
                   rtxUCSIsExtender(codepoint));
	    break;
        case XML_REGEXP_NOTDECIMAL:
	    neg = !neg;
        case XML_REGEXP_DECIMAL:
	    ret = rtxUCSIsCatNd(codepoint);
	    break;
        case XML_REGEXP_REALCHAR:
	    neg = !neg;
        case XML_REGEXP_NOTREALCHAR:
	    ret = rtxUCSIsCatP(codepoint);
	    if (ret == 0)
		ret = rtxUCSIsCatZ(codepoint);
	    if (ret == 0)
		ret = rtxUCSIsCatC(codepoint);
	    break;
        case XML_REGEXP_LETTER:
	    ret = rtxUCSIsCatL(codepoint);
	    break;
        case XML_REGEXP_LETTER_UPPERCASE:
	    ret = rtxUCSIsCatLu(codepoint);
	    break;
        case XML_REGEXP_LETTER_LOWERCASE:
	    ret = rtxUCSIsCatLl(codepoint);
	    break;
        case XML_REGEXP_LETTER_TITLECASE:
	    ret = rtxUCSIsCatLt(codepoint);
	    break;
        case XML_REGEXP_LETTER_MODIFIER:
	    ret = rtxUCSIsCatLm(codepoint);
	    break;
        case XML_REGEXP_LETTER_OTHERS:
	    ret = rtxUCSIsCatLo(codepoint);
	    break;
        case XML_REGEXP_MARK:
	    ret = rtxUCSIsCatM(codepoint);
	    break;
        case XML_REGEXP_MARK_NONSPACING:
	    ret = rtxUCSIsCatMn(codepoint);
	    break;
        case XML_REGEXP_MARK_SPACECOMBINING:
	    ret = rtxUCSIsCatMc(codepoint);
	    break;
        case XML_REGEXP_MARK_ENCLOSING:
	    ret = rtxUCSIsCatMe(codepoint);
	    break;
        case XML_REGEXP_NUMBER:
	    ret = rtxUCSIsCatN(codepoint);
	    break;
        case XML_REGEXP_NUMBER_DECIMAL:
	    ret = rtxUCSIsCatNd(codepoint);
	    break;
        case XML_REGEXP_NUMBER_LETTER:
	    ret = rtxUCSIsCatNl(codepoint);
	    break;
        case XML_REGEXP_NUMBER_OTHERS:
	    ret = rtxUCSIsCatNo(codepoint);
	    break;
        case XML_REGEXP_PUNCT:
	    ret = rtxUCSIsCatP(codepoint);
	    break;
        case XML_REGEXP_PUNCT_CONNECTOR:
	    ret = rtxUCSIsCatPc(codepoint);
	    break;
        case XML_REGEXP_PUNCT_DASH:
	    ret = rtxUCSIsCatPd(codepoint);
	    break;
        case XML_REGEXP_PUNCT_OPEN:
	    ret = rtxUCSIsCatPs(codepoint);
	    break;
        case XML_REGEXP_PUNCT_CLOSE:
	    ret = rtxUCSIsCatPe(codepoint);
	    break;
        case XML_REGEXP_PUNCT_INITQUOTE:
	    ret = rtxUCSIsCatPi(codepoint);
	    break;
        case XML_REGEXP_PUNCT_FINQUOTE:
	    ret = rtxUCSIsCatPf(codepoint);
	    break;
        case XML_REGEXP_PUNCT_OTHERS:
	    ret = rtxUCSIsCatPo(codepoint);
	    break;
        case XML_REGEXP_SEPAR:
	    ret = rtxUCSIsCatZ(codepoint);
	    break;
        case XML_REGEXP_SEPAR_SPACE:
	    ret = rtxUCSIsCatZs(codepoint);
	    break;
        case XML_REGEXP_SEPAR_LINE:
	    ret = rtxUCSIsCatZl(codepoint);
	    break;
        case XML_REGEXP_SEPAR_PARA:
	    ret = rtxUCSIsCatZp(codepoint);
	    break;
        case XML_REGEXP_SYMBOL:
	    ret = rtxUCSIsCatS(codepoint);
	    break;
        case XML_REGEXP_SYMBOL_MATH:
	    ret = rtxUCSIsCatSm(codepoint);
	    break;
        case XML_REGEXP_SYMBOL_CURRENCY:
	    ret = rtxUCSIsCatSc(codepoint);
	    break;
        case XML_REGEXP_SYMBOL_MODIFIER:
	    ret = rtxUCSIsCatSk(codepoint);
	    break;
        case XML_REGEXP_SYMBOL_OTHERS:
	    ret = rtxUCSIsCatSo(codepoint);
	    break;
        case XML_REGEXP_OTHER:
	    ret = rtxUCSIsCatC(codepoint);
	    break;
        case XML_REGEXP_OTHER_CONTROL:
	    ret = rtxUCSIsCatCc(codepoint);
	    break;
        case XML_REGEXP_OTHER_FORMAT:
	    ret = rtxUCSIsCatCf(codepoint);
	    break;
        case XML_REGEXP_OTHER_PRIVATE:
	    ret = rtxUCSIsCatCo(codepoint);
	    break;
        case XML_REGEXP_OTHER_NA:
	    /* ret = rtxUCSIsCatCn(codepoint); */
	    /* Seems it doesn't exist anymore in recent Unicode releases */
	    ret = 0;
	    break;
        case XML_REGEXP_BLOCK_NAME:
	    ret = rtxUCSIsBlock(codepoint, (const char *) blockName);
	    break;

        default: break;
    }
    if (neg)
	return(!ret);
    return(ret);
}

static int
rtxRegCheckCharacter(rtxRegAtomPtr atom, int codepoint) {
    int i, ret = 0;
    rtxRegRangePtr range;

    if ((atom == NULL) || (!rtxUCSIsChar(codepoint)))
	return(-1);

    switch (atom->type) {
        case XML_REGEXP_SUBREG:
        case XML_REGEXP_EPSILON:
	    return(-1);
        case XML_REGEXP_CHARVAL:
            return(codepoint == atom->codepoint);
        case XML_REGEXP_RANGES: {
	    int accept = 0;
	    for (i = 0;i < atom->nbRanges;i++) {
		range = atom->ranges[i];
		if (range->neg == 2) {
		    ret = rtxRegCheckCharacterRange(range->type, codepoint,
						0, range->start, range->end,
						range->blockName);
		    if (ret != 0)
			return(0); /* excluded char */
		} else if (range->neg) {
		    ret = rtxRegCheckCharacterRange(range->type, codepoint,
						0, range->start, range->end,
						range->blockName);
		    if (ret == 0)
			accept = 1;
		    else
		        return 0;	
		} else {
		    ret = rtxRegCheckCharacterRange(range->type, codepoint,
						0, range->start, range->end,
						range->blockName);
		    if (ret != 0)
			accept = 1; /* might still be excluded */
		}
	    }
	    return(accept);
	}
        case XML_REGEXP_STRING:
	    printf("TODO: XML_REGEXP_STRING\n");
	    return(-1);
        case XML_REGEXP_ANYCHAR:
        case XML_REGEXP_ANYSPACE:
        case XML_REGEXP_NOTSPACE:
        case XML_REGEXP_INITNAME:
        case XML_REGEXP_NOTINITNAME:
        case XML_REGEXP_NAMECHAR:
        case XML_REGEXP_NOTNAMECHAR:
        case XML_REGEXP_DECIMAL:
        case XML_REGEXP_NOTDECIMAL:
        case XML_REGEXP_REALCHAR:
        case XML_REGEXP_NOTREALCHAR:
        case XML_REGEXP_LETTER:
        case XML_REGEXP_LETTER_UPPERCASE:
        case XML_REGEXP_LETTER_LOWERCASE:
        case XML_REGEXP_LETTER_TITLECASE:
        case XML_REGEXP_LETTER_MODIFIER:
        case XML_REGEXP_LETTER_OTHERS:
        case XML_REGEXP_MARK:
        case XML_REGEXP_MARK_NONSPACING:
        case XML_REGEXP_MARK_SPACECOMBINING:
        case XML_REGEXP_MARK_ENCLOSING:
        case XML_REGEXP_NUMBER:
        case XML_REGEXP_NUMBER_DECIMAL:
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
	    ret = rtxRegCheckCharacterRange(atom->type, codepoint, 0, 0, 0,
		                            (const OSUTF8CHAR *)atom->valuep);
	    if (atom->neg)
		ret = !ret;
	    break;

        default: break;
    }
    return(ret);
}

/************************************************************************
 * 									*
 *	Saving an restoring state of an execution context		*
 * 									*
 ************************************************************************/

#ifdef DEBUG_REGEXP_EXEC
static void
xmlFARegDebugExec(rtxRegExecCtxtPtr exec) {
    printf("state: %d:%d:idx %d", exec->state->no, exec->transno, exec->index);
    if (exec->inputStack != NULL) {
	int i;
	printf(": ");
	for (i = 0;(i < 3) && (i < exec->inputStackNr);i++)
	    printf("%s ", exec->inputStack[exec->inputStackNr - (i + 1)]);
    } else {
	printf(": %s", &(exec->inputString[exec->index]));
    }
    printf("\n");
}
#endif

static void
xmlFARegExecSave(rtxRegExecCtxtPtr exec) {
#ifdef DEBUG_REGEXP_EXEC
    printf("saving ");
    exec->transno++;
    xmlFARegDebugExec(exec);
    exec->transno--;
#endif
#ifdef MAX_PUSH
    if (exec->nbPush > MAX_PUSH) {
        return;
    }
    exec->nbPush++;
#endif

    if (exec->maxRollbacks == 0) {
	exec->maxRollbacks = 4;
	exec->rollbacks = 
           rtxMemSysAllocArray (exec->pOSCTXT, exec->maxRollbacks,
                                 rtxRegExecRollback);
	if (exec->rollbacks == NULL) {
#ifdef DEBUG_REGEXP_EXEC
	    printf ("exec save: allocation failed");
#endif
	    exec->maxRollbacks = 0;
	    return;
	}
	OSCRTLMEMSET(exec->rollbacks, 0,
	       exec->maxRollbacks * sizeof(rtxRegExecRollback));
    } 
    else if (exec->nbRollbacks >= exec->maxRollbacks) {
	rtxRegExecRollback *tmp;
	int len = exec->maxRollbacks;

	exec->maxRollbacks *= 2;

	tmp = (rtxRegExecRollback *) rtxMemSysRealloc
           (exec->pOSCTXT, exec->rollbacks,
            exec->maxRollbacks * sizeof(rtxRegExecRollback));

	if (tmp == NULL) {
#ifdef DEBUG_REGEXP_EXEC
	    printf ("exec save: allocation failed");
#endif
	    exec->maxRollbacks /= 2;
	    return;
	}
	exec->rollbacks = tmp;
	tmp = &exec->rollbacks[len];
	OSCRTLMEMSET(tmp, 0, (exec->maxRollbacks - len) * sizeof(rtxRegExecRollback));
    }
    exec->rollbacks[exec->nbRollbacks].state = exec->state;
    exec->rollbacks[exec->nbRollbacks].index = exec->index;
    exec->rollbacks[exec->nbRollbacks].nextbranch = exec->transno + 1;
    if (exec->comp->nbCounters > 0) {
	if (exec->rollbacks[exec->nbRollbacks].counts == NULL) {
	    exec->rollbacks[exec->nbRollbacks].counts = 
		rtxMemSysAllocArray (exec->pOSCTXT, 
                                      exec->comp->nbCounters, int);
	    if (exec->rollbacks[exec->nbRollbacks].counts == NULL) {
#ifdef DEBUG_REGEXP_EXEC
		printf ("exec save: allocation failed");
#endif
		exec->status = -5;
		return;
	    }
	}
	OSCRTLSAFEMEMCPY(exec->rollbacks[exec->nbRollbacks].counts, 
         exec->comp->nbCounters*sizeof(int), exec->counts,
         exec->comp->nbCounters * sizeof(int));
   }
    exec->nbRollbacks++;
}

static void
xmlFARegExecRollBack(rtxRegExecCtxtPtr exec) {
    if (exec->nbRollbacks <= 0) {
	exec->status = -1;
#ifdef DEBUG_REGEXP_EXEC
	printf("rollback failed on empty stack\n");
#endif
	return;
    }
    exec->nbRollbacks--;
    exec->state = exec->rollbacks[exec->nbRollbacks].state;
    exec->index = exec->rollbacks[exec->nbRollbacks].index;
    exec->transno = exec->rollbacks[exec->nbRollbacks].nextbranch;
    if (exec->comp->nbCounters > 0) {
	if (exec->rollbacks[exec->nbRollbacks].counts == NULL) {
	    fprintf(stderr, "exec save: allocation failed");
	    exec->status = -6;
	    return;
	}
	OSCRTLSAFEMEMCPY(exec->counts, exec->comp->nbCounters*sizeof(int),
         exec->rollbacks[exec->nbRollbacks].counts,
	      exec->comp->nbCounters * sizeof(int));
    }

#ifdef DEBUG_REGEXP_EXEC
    printf("restored ");
    xmlFARegDebugExec(exec);
#endif
}

/************************************************************************
 * 									*
 *	Verifyer, running an input against a compiled regexp		*
 * 									*
 ************************************************************************/

static int
xmlFARegExec (OSCTXT* pOSCTXT, rtxRegexpPtr comp, const OSUTF8CHAR *content) 
{
    rtxRegExecCtxt execval;
    rtxRegExecCtxtPtr exec = &execval;
    int ret, codepoint, len = 0;

    exec->pOSCTXT = pOSCTXT;
    exec->inputString = content;
    exec->index = 0;
    exec->nbPush = 0;
    exec->determinist = 1;
    exec->maxRollbacks = 0;
    exec->nbRollbacks = 0;
    exec->rollbacks = NULL;
    exec->status = 0;
    exec->comp = comp;
    exec->state = comp->states[0];
    exec->transno = 0;
    exec->transcount = 0;
    exec->inputStack = NULL;
    exec->inputStackMax = 0;
    if (comp->nbCounters > 0) {
	exec->counts = 
           rtxMemSysAllocArray (pOSCTXT, comp->nbCounters, int);
	if (exec->counts == NULL)
	    return(-1);
        OSCRTLMEMSET(exec->counts, 0, comp->nbCounters * sizeof(int));
    } else
	exec->counts = NULL;
    while ((exec->status == 0) &&
	   ((exec->inputString[exec->index] != 0) ||
	    (exec->state->type != XML_REGEXP_FINAL_STATE))) {
	rtxRegTransPtr trans;
	rtxRegAtomPtr atom;

	/*
	 * If end of input on non-terminal state, rollback, however we may
	 * still have epsilon like transition for counted transitions
	 * on counters, in that case don't break too early.  Additionally,
	 * if we are working on a range like "AB{0,2}", where B is not present,
	 * we don't want to break.
	 */
	if ((exec->inputString[exec->index] == 0) && (exec->counts == NULL)) {
	    /*
	     * if there is a transition, we must check if
	     *  atom allows minOccurs of 0
	     */
	    if (exec->transno < exec->state->nbTrans) {
	        trans = &exec->state->trans[exec->transno];
		if (trans->to >=0) {
		    atom = trans->atom;
		    if (!((atom->min == 0) && (atom->max > 0)))
		        goto rollback;
		}
	    } else
	        goto rollback;
	}

	exec->transcount = 0;
	for (;exec->transno < exec->state->nbTrans;exec->transno++) {
	    trans = &exec->state->trans[exec->transno];
	    if (trans->to < 0)
		continue;
	    atom = trans->atom;
	    ret = 0;
	    if (trans->count >= 0) {
		int count;
		rtxRegCounterPtr counter;

      if (exec->counts == NULL) {
         exec->status = -1;
         goto error;
      }
		/*
		 * A counted transition.
		 */

		count = exec->counts[trans->count];
		counter = &exec->comp->counters[trans->count];
#ifdef DEBUG_REGEXP_EXEC
		printf("testing count %d: val %d, min %d, max %d\n",
		       trans->count, count, counter->min,  counter->max);
#endif
		ret = ((count >= counter->min) && (count <= counter->max));
	    } else if (atom == NULL) {
		fprintf(stderr, "epsilon transition left at runtime\n");
		exec->status = -2;
		break;
	    } else if (exec->inputString[exec->index] != 0) {
                codepoint = CUR_SCHAR(&(exec->inputString[exec->index]), len);
		ret = rtxRegCheckCharacter(atom, codepoint);
		if ((ret == 1) && (atom->min >= 0) && (atom->max > 0)) {
		    rtxRegStatePtr to = comp->states[trans->to];

		    /*
		     * this is a multiple input sequence
		     * If there is a counter associated increment it now.
		     * before potentially saving and rollback
		     */
		    if (trans->counter >= 0) {
#ifdef DEBUG_REGEXP_EXEC
			printf("Increasing count %d\n", trans->counter);
#endif
			exec->counts[trans->counter]++;
		    }
		    /*
		     * this is a multiple input sequence
		     */
		    if (exec->state->nbTrans > exec->transno + 1) {
			xmlFARegExecSave(exec);
		    }
		    exec->transcount = 1;
		    do {
			/*
			 * Try to progress as much as possible on the input
			 */
			if (exec->transcount == atom->max) {
			    break;
			}
			exec->index += len;
			/*
			 * End of input: stop here
			 */
			if (exec->inputString[exec->index] == 0) {
			    exec->index -= len;
			    break;
			}
			if (exec->transcount >= atom->min) {
			    int transno = exec->transno;
			    rtxRegStatePtr state = exec->state;

			    /*
			     * The transition is acceptable save it
			     */
			    exec->transno = -1; /* trick */
			    exec->state = to;
			    xmlFARegExecSave(exec);
			    exec->transno = transno;
			    exec->state = state;
			}
			codepoint = 
                           CUR_SCHAR(&(exec->inputString[exec->index]), len);
			ret = rtxRegCheckCharacter(atom, codepoint);
			exec->transcount++;
		    } while (ret == 1);
		    if (exec->transcount < atom->min)
			ret = 0;

		    /*
		     * If the last check failed but one transition was found
		     * possible, rollback
		     */
		    if (ret < 0)
			ret = 0;
		    if (ret == 0) {
			goto rollback;
		    }
		    if (trans->counter >= 0) {
#ifdef DEBUG_REGEXP_EXEC
			printf("Decreasing count %d\n", trans->counter);
#endif
			exec->counts[trans->counter]--;
		    }
		} else if ((ret == 0) && (atom->min == 0) && (atom->max > 0)) {
		    /*
		     * we don't match on the codepoint, but minOccurs of 0
		     * says that's ok.  Setting len to 0 inhibits stepping
		     * over the codepoint.
		     */
		    exec->transcount = 1;
		    len = 0;
		    ret = 1;
		}
	    } else if ((atom->min == 0) && (atom->max > 0)) {
	        /* another spot to match when minOccurs is 0 */
		exec->transcount = 1;
		len = 0;
		ret = 1;
	    }
	    if (ret == 1) {
		if (exec->state->nbTrans > exec->transno + 1) {
		    xmlFARegExecSave(exec);
		}
		/* YG - restart count for expressions like this ((abc){2})* */
		if (trans->count >= 0) {
#ifdef DEBUG_REGEXP_EXEC
		    printf("Reset count %d\n", trans->count);
#endif
		    exec->counts[trans->count] = 0;
		}
		if (trans->counter >= 0) {
#ifdef DEBUG_REGEXP_EXEC
		    printf("Increasing count %d\n", trans->counter);
#endif
		    exec->counts[trans->counter]++;
		}
#ifdef DEBUG_REGEXP_EXEC
		printf("entering state %d\n", trans->to);
#endif
		exec->state = comp->states[trans->to];
		exec->transno = 0;
		if (trans->atom != NULL) {
		    exec->index += len;
		}
		goto progress;
	    } else if (ret < 0) {
		exec->status = -4;
		break;
	    }
	}
	if ((exec->transno != 0) || (exec->state->nbTrans == 0)) {
rollback:
	    /*
	     * Failed to find a way out
	     */
	    exec->determinist = 0;
	    xmlFARegExecRollBack(exec);
	}
progress:
	continue;
    }
error:
    if (exec->rollbacks != NULL) {
	if (exec->counts != NULL) {
	    int i;

	    for (i = 0;i < exec->maxRollbacks;i++)
		if (exec->rollbacks[i].counts != NULL)
		    rtxMemSysFreeArray (exec->pOSCTXT, exec->rollbacks[i].counts);
	}
	rtxMemSysFreeArray (exec->pOSCTXT, exec->rollbacks);
    }
    if (exec->counts != NULL)
	rtxMemSysFreeArray (exec->pOSCTXT, exec->counts);
    if (exec->status == 0)
	return(1);
    if (exec->status == -1) {
	if (exec->nbPush > MAX_PUSH)
	    return(-1);
	return(0);
    }
    return(exec->status);
}

/************************************************************************
 * 									*
 *	Progressive interface to the verifyer one atom at a time	*
 * 									*
 ************************************************************************/

/*
 * rtxRegNewExecCtxt:
 * @comp: a precompiled regular expression
 * @callback: a callback function used for handling progresses in the
 *            automata matching phase
 * @data: the context data associated to the callback in this context
 *
 * Build a context used for progressive evaluation of a regexp.
 *
 * Returns the new context
 */
rtxRegExecCtxtPtr rtxRegNewExecCtxt 
(OSCTXT* pOSCTXT, rtxRegexpPtr comp, rtxRegExecCallbacks callback, void *data)
{
    rtxRegExecCtxtPtr exec;

    if (comp == NULL)
	return(NULL);
    if ((comp->compact == NULL) && (comp->states == NULL))
        return(NULL);

    exec = rtxMemSysAllocTypeZ (pOSCTXT, rtxRegExecCtxt);
    if (exec == NULL) {
	return(NULL);
    }
    exec->pOSCTXT = pOSCTXT;
    exec->inputString = NULL;
    exec->index = 0;
    exec->determinist = 1;
    exec->maxRollbacks = 0;
    exec->nbRollbacks = 0;
    exec->rollbacks = NULL;
    exec->status = 0;
    exec->comp = comp;
    if (comp->compact == NULL)
	exec->state = comp->states[0];
    exec->transno = 0;
    exec->transcount = 0;
    exec->callback = callback;
    exec->data = data;
    if (comp->nbCounters > 0) {
	exec->counts = rtxMemSysAllocArray 
           (pOSCTXT, comp->nbCounters, int);
	if (exec->counts == NULL) {
	    rtxMemSysFreeType (exec->pOSCTXT, exec);
	    return(NULL);
	}
        OSCRTLMEMSET(exec->counts, 0, comp->nbCounters * sizeof(int));
    } else
	exec->counts = NULL;
    exec->inputStackMax = 0;
    exec->inputStackNr = 0;
    exec->inputStack = NULL;
    exec->nbPush = 0;
    return(exec);
}

/*
 * rtxRegFreeExecCtxt:
 * @exec: a regular expression evaulation context
 *
 * Free the structures associated to a regular expression evaulation context.
 */
void
rtxRegFreeExecCtxt(rtxRegExecCtxtPtr exec) {
    if (exec == NULL)
	return;

    if (exec->rollbacks != NULL) {
	if (exec->counts != NULL) {
	    int i;

	    for (i = 0;i < exec->maxRollbacks;i++)
		if (exec->rollbacks[i].counts != NULL)
		    rtxMemSysFreeArray (exec->pOSCTXT, exec->rollbacks[i].counts);
	}
	rtxMemSysFreeArray (exec->pOSCTXT, exec->rollbacks);
    }
    if (exec->counts != NULL)
	rtxMemSysFreeArray (exec->pOSCTXT, exec->counts);
    if (exec->inputStack != NULL) {
	int i;

	for (i = 0;i < exec->inputStackNr;i++) {
	    if (exec->inputStack[i].value != NULL)
		rtxMemSysFreeArray (exec->pOSCTXT, exec->inputStack[i].value);
	}
	rtxMemSysFreeArray (exec->pOSCTXT, exec->inputStack);
    }
    rtxMemSysFreeType (exec->pOSCTXT, exec);
}

static void
xmlFARegExecSaveInputString(rtxRegExecCtxtPtr exec, const OSUTF8CHAR *value,
	                    void *data) {
#ifdef DEBUG_PUSH
    printf("saving value: %d:%s\n", exec->inputStackNr, value);
#endif
    if (exec->inputStackMax == 0) {
	exec->inputStackMax = 4;
	exec->inputStack =
	    rtxMemSysAllocArray (exec->pOSCTXT, 
                                  exec->inputStackMax, rtxRegInputToken);
	if (exec->inputStack == NULL) {
#ifdef DEBUG_PUSH
	    printf("push input: allocation failed");
#endif
	    exec->inputStackMax = 0;
	    return;
	}
    } else if (exec->inputStackNr + 1 >= exec->inputStackMax) {
	rtxRegInputTokenPtr tmp;

	exec->inputStackMax *= 2;

	tmp = (rtxRegInputTokenPtr) rtxMemSysRealloc
           (exec->pOSCTXT, exec->inputStack,
            exec->inputStackMax * sizeof(rtxRegInputToken));

	if (tmp == NULL) {
#ifdef DEBUG_PUSH
	    printf("push input: allocation failed");
#endif
	    exec->inputStackMax /= 2;
	    return;
	}
	exec->inputStack = tmp;
    }
    exec->inputStack[exec->inputStackNr].value = 
       rtxRegexpStrdup (exec->pOSCTXT, value);
    exec->inputStack[exec->inputStackNr].data = data;
    exec->inputStackNr++;
    exec->inputStack[exec->inputStackNr].value = NULL;
    exec->inputStack[exec->inputStackNr].data = NULL;
}


/*
 * rtxRegCompactPushString:
 * @exec: a regexp execution context
 * @comp:  the precompiled exec with a compact table
 * @value: a string token input
 * @data: data associated to the token to reuse in callbacks
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *     a negative value in case of error.
 */
static int
rtxRegCompactPushString(rtxRegExecCtxtPtr exec,
	                rtxRegexpPtr comp,
	                const OSUTF8CHAR *value,
	                void *data) {
    int state = exec->index;
    int i, target;

    if ((comp == NULL) || (comp->compact == NULL) || (comp->stringMap == NULL))
	return(-1);
    
    if (value == NULL) {
	/*
	 * are we at a final state ?
	 */
	if (comp->compact[state * (comp->nbstrings + 1)] ==
            XML_REGEXP_FINAL_STATE)
	    return(1);
	return(0);
    }

#ifdef DEBUG_PUSH
    printf("value pushed: %s\n", value);
#endif

    /*
     * Examine all outside transition from current state
     */
    for (i = 0;i < comp->nbstrings;i++) {
	target = comp->compact[state * (comp->nbstrings + 1) + i + 1];
	if ((target > 0) && (target <= comp->nbstates)) {
	    target--; /* to avoid 0 */
	    if (rtxUTF8StrEqual(comp->stringMap[i], value)) {
		exec->index = target;
		if ((exec->callback != NULL) && (comp->transdata != NULL)) {
		    exec->callback((struct _rtxRegExecCtxt*)exec->data, value,
			  comp->transdata[state * comp->nbstrings + i], data);
		}
#ifdef DEBUG_PUSH
		printf("entering state %d\n", target);
#endif
		if (comp->compact[target * (comp->nbstrings + 1)] ==
		    XML_REGEXP_FINAL_STATE)
		    return(1);
		return(0);
	    }
	}
    }
    /*
     * Failed to find an exit transition out from current state for the
     * current token
     */
#ifdef DEBUG_PUSH
    printf("failed to find a transition for %s on state %d\n", value, state);
#endif
    exec->status = -1;
    return(-1);
}

/*
 * rtxRegExecPushString:
 * @exec: a regexp execution context
 * @value: a string token input
 * @data: data associated to the token to reuse in callbacks
 *
 * Push one input token in the execution context
 *
 * Returns: 1 if the regexp reached a final state, 0 if non-final, and
 *     a negative value in case of error.
 */
int
rtxRegExecPushString(rtxRegExecCtxtPtr exec, const OSUTF8CHAR *value,
	             void *data) {
    rtxRegTransPtr trans;
    rtxRegAtomPtr atom;
    int ret;
    int final = 0;

    if (exec == NULL)
	return(-1);
    if (exec->comp == NULL)
	return(-1);
    if (exec->status != 0)
	return(exec->status);

    if (exec->comp->compact != NULL)
	return(rtxRegCompactPushString(exec, exec->comp, value, data));

    if (value == NULL) {
        if (exec->state->type == XML_REGEXP_FINAL_STATE)
	    return(1);
	final = 1;
    }

#ifdef DEBUG_PUSH
    printf("value pushed: %s\n", value);
#endif
    /*
     * If we have an active rollback stack push the new value there
     * and get back to where we were left
     */
    if ((value != NULL) && (exec->inputStackNr > 0)) {
	xmlFARegExecSaveInputString(exec, value, data);
	value = exec->inputStack[exec->index].value;
	data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
	printf("value loaded: %s\n", value);
#endif
    }

    while ((exec->status == 0) &&
	   ((value != NULL) ||
	    ((final == 1) &&
	     (exec->state->type != XML_REGEXP_FINAL_STATE)))) {

	/*
	 * End of input on non-terminal state, rollback, however we may
	 * still have epsilon like transition for counted transitions
	 * on counters, in that case don't break too early.
	 */
	if ((value == NULL) && (exec->counts == NULL))
	    goto rollback;

	exec->transcount = 0;
	for (;exec->transno < exec->state->nbTrans;exec->transno++) {
	    trans = &exec->state->trans[exec->transno];
	    if (trans->to < 0)
		continue;
	    atom = trans->atom;
	    ret = 0;
	    if (trans->count == REGEXP_ALL_LAX_COUNTER) {
		int i;
		int count;
		rtxRegTransPtr t;
		rtxRegCounterPtr counter;

		ret = 0;

#ifdef DEBUG_PUSH
		printf("testing all lax %d\n", trans->count);
#endif
		/*
		 * Check all counted transitions from the current state
		 */
		if ((value == NULL) && (final)) {
		    ret = 1;
		} else if (value != NULL) {
		    for (i = 0;i < exec->state->nbTrans;i++) {
			t = &exec->state->trans[i];
			if ((t->counter < 0) || (t == trans))
			    continue;
			counter = &exec->comp->counters[t->counter];
			count = exec->counts[t->counter];
			if ((count < counter->max) && 
		            (t->atom != NULL) &&
			    (rtxUTF8StrEqual(value, (const OSUTF8CHAR*)t->atom->valuep))) {
			    ret = 0;
			    break;
			}
			if ((count >= counter->min) &&
			    (count < counter->max) &&
			    (rtxUTF8StrEqual(value, (const OSUTF8CHAR*)t->atom->valuep))) {
			    ret = 1;
			    break;
			}
		    }
		}
	    } else if (trans->count == REGEXP_ALL_COUNTER) {
		int i;
		int count;
		rtxRegTransPtr t;
		rtxRegCounterPtr counter;

		ret = 1;

#ifdef DEBUG_PUSH
		printf("testing all %d\n", trans->count);
#endif
		/*
		 * Check all counted transitions from the current state
		 */
		for (i = 0;i < exec->state->nbTrans;i++) {
                    t = &exec->state->trans[i];
		    if ((t->counter < 0) || (t == trans))
			continue;
                    counter = &exec->comp->counters[t->counter];
		    count = exec->counts[t->counter];
		    if ((count < counter->min) || (count > counter->max)) {
			ret = 0;
			break;
		    }
		}
	    } else if (trans->count >= 0) {
		int count;
		rtxRegCounterPtr counter;

		/*
		 * A counted transition.
		 */

		count = exec->counts[trans->count];
		counter = &exec->comp->counters[trans->count];
#ifdef DEBUG_PUSH
		printf("testing count %d: val %d, min %d, max %d\n",
		       trans->count, count, counter->min,  counter->max);
#endif
		ret = ((count >= counter->min) && (count <= counter->max));
	    } else if (atom == NULL) {
		fprintf(stderr, "epsilon transition left at runtime\n");
		exec->status = -2;
		break;
	    } else if (value != NULL) {
		ret = rtxUTF8StrEqual(value, (const OSUTF8CHAR*)atom->valuep);
		if ((ret == 1) && (trans->counter >= 0)) {
		    rtxRegCounterPtr counter;
		    int count;

		    count = exec->counts[trans->counter];
		    counter = &exec->comp->counters[trans->counter];
		    if (count >= counter->max)
			ret = 0;
		}

		if ((ret == 1) && (atom->min > 0) && (atom->max > 0)) {
		    rtxRegStatePtr to = exec->comp->states[trans->to];

		    /*
		     * this is a multiple input sequence
		     */
		    if (exec->state->nbTrans > exec->transno + 1) {
			if (exec->inputStackNr <= 0) {
			    xmlFARegExecSaveInputString(exec, value, data);
			}
			xmlFARegExecSave(exec);
		    }
		    exec->transcount = 1;
		    do {
			/*
			 * Try to progress as much as possible on the input
			 */
			if (exec->transcount == atom->max) {
			    break;
			}
			exec->index++;
			value = exec->inputStack[exec->index].value;
			data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
			printf("value loaded: %s\n", value);
#endif

			/*
			 * End of input: stop here
			 */
			if (value == NULL) {
			    exec->index --;
			    break;
			}
			if (exec->transcount >= atom->min) {
			    int transno = exec->transno;
			    rtxRegStatePtr state = exec->state;

			    /*
			     * The transition is acceptable save it
			     */
			    exec->transno = -1; /* trick */
			    exec->state = to;
			    if (exec->inputStackNr <= 0) {
				xmlFARegExecSaveInputString(exec, value, data);
			    }
			    xmlFARegExecSave(exec);
			    exec->transno = transno;
			    exec->state = state;
			}
			ret = rtxUTF8StrEqual(value, (const OSUTF8CHAR*)atom->valuep);
			exec->transcount++;
		    } while (ret == 1);
		    if (exec->transcount < atom->min)
			ret = 0;

		    /*
		     * If the last check failed but one transition was found
		     * possible, rollback
		     */
		    if (ret < 0)
			ret = 0;
		    if (ret == 0) {
			goto rollback;
		    }
		}
	    }
	    if (ret == 1) {
		if ((exec->callback != NULL) && (atom != NULL) &&
			(data != NULL)) {
		    exec->callback((struct _rtxRegExecCtxt*)exec->data, 
		                   (const OSUTF8CHAR*)atom->valuep,
			           atom->data, data);
		}
		if (exec->state->nbTrans > exec->transno + 1) {
		    if (exec->inputStackNr <= 0) {
			xmlFARegExecSaveInputString(exec, value, data);
		    }
		    xmlFARegExecSave(exec);
		}
		if (trans->counter >= 0) {
#ifdef DEBUG_PUSH
		    printf("Increasing count %d\n", trans->counter);
#endif
		    exec->counts[trans->counter]++;
		}
#ifdef DEBUG_PUSH
		printf("entering state %d\n", trans->to);
#endif
		exec->state = exec->comp->states[trans->to];
		exec->transno = 0;
		if (trans->atom != NULL) {
		    if (exec->inputStack != NULL) {
			exec->index++;
			if (exec->index < exec->inputStackNr) {
			    value = exec->inputStack[exec->index].value;
			    data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
			    printf("value loaded: %s\n", value);
#endif
			} else {
			    value = NULL;
			    data = NULL;
#ifdef DEBUG_PUSH
			    printf("end of input\n");
#endif
			}
		    } else {
			value = NULL;
			data = NULL;
#ifdef DEBUG_PUSH
			printf("end of input\n");
#endif
		    }
		}
		goto progress;
	    } else if (ret < 0) {
		exec->status = -4;
		break;
	    }
	}
	if ((exec->transno != 0) || (exec->state->nbTrans == 0)) {
rollback:
	    /*
	     * Failed to find a way out
	     */
	    exec->determinist = 0;
	    xmlFARegExecRollBack(exec);
	    if (exec->status == 0) {
		value = exec->inputStack[exec->index].value;
		data = exec->inputStack[exec->index].data;
#ifdef DEBUG_PUSH
		printf("value loaded: %s\n", value);
#endif
	    }
	}
progress:
	continue;
    }
    if (exec->status == 0) {
        return(exec->state->type == XML_REGEXP_FINAL_STATE);
    }
    return(exec->status);
}

/************************************************************************
 * 									*
 *	Parser for the Shemas Datatype Regular Expressions		*
 *	http://www.w3.org/TR/2001/REC-xmlschema-2-20010502/#regexs	* 
 * 									*
 ************************************************************************/

/*
 * xmlFAIsChar:
 * @ctxt:  a regexp parser context
 *
 * [10]   Char   ::=   [^.\?*+()|#x5B#x5D]
 */
static int
xmlFAIsChar(rtxRegParserCtxtPtr ctxt) {
    int cur;
    int len;

    cur = CUR_SCHAR(ctxt->cur, len);
    if ((cur == '.') || (cur == '\\') || (cur == '?') ||
	(cur == '*') || (cur == '+') || (cur == '(') ||
	(cur == ')') || (cur == '|') || (cur == 0x5B) ||
	(cur == 0x5D) || (cur == 0))
	return(-1);
    return(cur);
}

/*
 * xmlFAParseCharProp:
 * @ctxt:  a regexp parser context
 *
 * [27]   charProp   ::=   IsCategory | IsBlock
 * [28]   IsCategory ::= Letters | Marks | Numbers | Punctuation |
 *                       Separators | Symbols | Others 
 * [29]   Letters   ::=   'L' [ultmo]?
 * [30]   Marks   ::=   'M' [nce]?
 * [31]   Numbers   ::=   'N' [dlo]?
 * [32]   Punctuation   ::=   'P' [cdseifo]?
 * [33]   Separators   ::=   'Z' [slp]?
 * [34]   Symbols   ::=   'S' [mcko]?
 * [35]   Others   ::=   'C' [cfon]?
 * [36]   IsBlock   ::=   'Is' [a-zA-Z0-9#x2D]+
 */
static void
xmlFAParseCharProp(rtxRegParserCtxtPtr ctxt) {
    int cur;
    rtxRegAtomType type = (rtxRegAtomType) 0;
    OSUTF8CHAR *blockName = NULL;
    
    cur = CUR;
    if (cur == 'L') {
	NEXT;
	cur = CUR;
	if (cur == 'u') {
	    NEXT;
	    type = XML_REGEXP_LETTER_UPPERCASE;
	} else if (cur == 'l') {
	    NEXT;
	    type = XML_REGEXP_LETTER_LOWERCASE;
	} else if (cur == 't') {
	    NEXT;
	    type = XML_REGEXP_LETTER_TITLECASE;
	} else if (cur == 'm') {
	    NEXT;
	    type = XML_REGEXP_LETTER_MODIFIER;
	} else if (cur == 'o') {
	    NEXT;
	    type = XML_REGEXP_LETTER_OTHERS;
	} else {
	    type = XML_REGEXP_LETTER;
	}
    } else if (cur == 'M') {
	NEXT;
	cur = CUR;
	if (cur == 'n') {
	    NEXT;
	    /* nonspacing */
	    type = XML_REGEXP_MARK_NONSPACING;
	} else if (cur == 'c') {
	    NEXT;
	    /* spacing combining */
	    type = XML_REGEXP_MARK_SPACECOMBINING;
	} else if (cur == 'e') {
	    NEXT;
	    /* enclosing */
	    type = XML_REGEXP_MARK_ENCLOSING;
	} else {
	    /* all marks */
	    type = XML_REGEXP_MARK;
	}
    } else if (cur == 'N') {
	NEXT;
	cur = CUR;
	if (cur == 'd') {
	    NEXT;
	    /* digital */
	    type = XML_REGEXP_NUMBER_DECIMAL;
	} else if (cur == 'l') {
	    NEXT;
	    /* letter */
	    type = XML_REGEXP_NUMBER_LETTER;
	} else if (cur == 'o') {
	    NEXT;
	    /* other */
	    type = XML_REGEXP_NUMBER_OTHERS;
	} else {
	    /* all numbers */
	    type = XML_REGEXP_NUMBER;
	}
    } else if (cur == 'P') {
	NEXT;
	cur = CUR;
	if (cur == 'c') {
	    NEXT;
	    /* connector */
	    type = XML_REGEXP_PUNCT_CONNECTOR;
	} else if (cur == 'd') {
	    NEXT;
	    /* dash */
	    type = XML_REGEXP_PUNCT_DASH;
	} else if (cur == 's') {
	    NEXT;
	    /* open */
	    type = XML_REGEXP_PUNCT_OPEN;
	} else if (cur == 'e') {
	    NEXT;
	    /* close */
	    type = XML_REGEXP_PUNCT_CLOSE;
	} else if (cur == 'i') {
	    NEXT;
	    /* initial quote */
	    type = XML_REGEXP_PUNCT_INITQUOTE;
	} else if (cur == 'f') {
	    NEXT;
	    /* final quote */
	    type = XML_REGEXP_PUNCT_FINQUOTE;
	} else if (cur == 'o') {
	    NEXT;
	    /* other */
	    type = XML_REGEXP_PUNCT_OTHERS;
	} else {
	    /* all punctuation */
	    type = XML_REGEXP_PUNCT;
	}
    } else if (cur == 'Z') {
	NEXT;
	cur = CUR;
	if (cur == 's') {
	    NEXT;
	    /* space */
	    type = XML_REGEXP_SEPAR_SPACE;
	} else if (cur == 'l') {
	    NEXT;
	    /* line */
	    type = XML_REGEXP_SEPAR_LINE;
	} else if (cur == 'p') {
	    NEXT;
	    /* paragraph */
	    type = XML_REGEXP_SEPAR_PARA;
	} else {
	    /* all separators */
	    type = XML_REGEXP_SEPAR;
	}
    } else if (cur == 'S') {
	NEXT;
	cur = CUR;
	if (cur == 'm') {
	    NEXT;
	    type = XML_REGEXP_SYMBOL_MATH;
	    /* math */
	} else if (cur == 'c') {
	    NEXT;
	    type = XML_REGEXP_SYMBOL_CURRENCY;
	    /* currency */
	} else if (cur == 'k') {
	    NEXT;
	    type = XML_REGEXP_SYMBOL_MODIFIER;
	    /* modifiers */
	} else if (cur == 'o') {
	    NEXT;
	    type = XML_REGEXP_SYMBOL_OTHERS;
	    /* other */
	} else {
	    /* all symbols */
	    type = XML_REGEXP_SYMBOL;
	}
    } else if (cur == 'C') {
	NEXT;
	cur = CUR;
	if (cur == 'c') {
	    NEXT;
	    /* control */
	    type = XML_REGEXP_OTHER_CONTROL;
	} else if (cur == 'f') {
	    NEXT;
	    /* format */
	    type = XML_REGEXP_OTHER_FORMAT;
	} else if (cur == 'o') {
	    NEXT;
	    /* private use */
	    type = XML_REGEXP_OTHER_PRIVATE;
	} else if (cur == 'n') {
	    NEXT;
	    /* not assigned */
	    type = XML_REGEXP_OTHER_NA;
	} else {
	    /* all others */
	    type = XML_REGEXP_OTHER;
	}
    } else if (cur == 'I') {
	const OSUTF8CHAR *start;
	NEXT;
	cur = CUR;
	if (cur != 's') {
	    REGEXPERROR("IsXXXX expected");
	    return;
	}
	NEXT;
	start = ctxt->cur;
	cur = CUR;
	if (((cur >= 'a') && (cur <= 'z')) || 
	    ((cur >= 'A') && (cur <= 'Z')) || 
	    ((cur >= '0') && (cur <= '9')) || 
	    (cur == 0x2D)) {
	    NEXT;
	    cur = CUR;
	    while (((cur >= 'a') && (cur <= 'z')) || 
		((cur >= 'A') && (cur <= 'Z')) || 
		((cur >= '0') && (cur <= '9')) || 
		(cur == 0x2D)) {
		NEXT;
		cur = CUR;
	    }
	}
	type = XML_REGEXP_BLOCK_NAME;
	blockName = rtxRegexpStrndup (ctxt->pOSCTXT, start, ctxt->cur - start);
    } else {
	REGEXPERROR("Unknown char property");
	return;
    }
    if (ctxt->atom == NULL) {
	ctxt->atom = rtxRegNewAtom(ctxt, type);
	if (ctxt->atom != NULL)
	    ctxt->atom->valuep = blockName;
    } else if (ctxt->atom->type == XML_REGEXP_RANGES) {
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
		           type, 0, 0, blockName);
    }
}

/*
 * xmlFAParseCharClassEsc:
 * @ctxt:  a regexp parser context
 *
 * [23] charClassEsc ::= ( SingleCharEsc | MultiCharEsc | catEsc | complEsc ) 
 * [24] SingleCharEsc ::= '\' [nrt\|.?*+(){}#x2D#x5B#x5D#x5E]
 * [25] catEsc   ::=   '\p{' charProp '}'
 * [26] complEsc ::=   '\P{' charProp '}'
 * [37] MultiCharEsc ::= '.' | ('\' [sSiIcCdDwW])
 */
static void
xmlFAParseCharClassEsc(rtxRegParserCtxtPtr ctxt) {
    int cur;

    if (CUR == '.') {
	if (ctxt->atom == NULL) {
	    ctxt->atom = rtxRegNewAtom(ctxt, XML_REGEXP_ANYCHAR);
	} else if (ctxt->atom->type == XML_REGEXP_RANGES) {
	    rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			       XML_REGEXP_ANYCHAR, 0, 0, NULL);
	}
	NEXT;
	return;
    }
    if (CUR != '\\') {
	REGEXPERROR("Escaped sequence: expecting \\");
	return;
    }
    NEXT;
    cur = CUR;
    if (cur == 'p') {
	NEXT;
	if (CUR != '{') {
	    REGEXPERROR("Expecting '{'");
	    return;
	}
	NEXT;
	xmlFAParseCharProp(ctxt);
	if (CUR != '}') {
	    REGEXPERROR("Expecting '}'");
	    return;
	}
	NEXT;
    } else if (cur == 'P') {
	NEXT;
	if (CUR != '{') {
	    REGEXPERROR("Expecting '{'");
	    return;
	}
	NEXT;
	xmlFAParseCharProp(ctxt);
	ctxt->atom->neg = 1;
	if (CUR != '}') {
	    REGEXPERROR("Expecting '}'");
	    return;
	}
	NEXT;
    } else if ((cur == 'n') || (cur == 'r') || (cur == 't') || (cur == '\\') ||
	(cur == '|') || (cur == '.') || (cur == '?') || (cur == '*') ||
	(cur == '+') || (cur == '(') || (cur == ')') || (cur == '{') ||
	(cur == '}') || (cur == 0x2D) || (cur == 0x5B) || (cur == 0x5D) ||
	(cur == 0x5E)) {
	if (ctxt->atom == NULL) {
	    ctxt->atom = rtxRegNewAtom(ctxt, XML_REGEXP_CHARVAL);
	    if (ctxt->atom != NULL)
		ctxt->atom->codepoint = cur;
	} else if (ctxt->atom->type == XML_REGEXP_RANGES) {
	    rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			       XML_REGEXP_CHARVAL, cur, cur, NULL);
	}
	NEXT;
    } else if ((cur == 's') || (cur == 'S') || (cur == 'i') || (cur == 'I') ||
	(cur == 'c') || (cur == 'C') || (cur == 'd') || (cur == 'D') ||
	(cur == 'w') || (cur == 'W')) {
	rtxRegAtomType type = XML_REGEXP_ANYSPACE;

	switch (cur) {
	    case 's': 
		type = XML_REGEXP_ANYSPACE;
		break;
	    case 'S': 
		type = XML_REGEXP_NOTSPACE;
		break;
	    case 'i': 
		type = XML_REGEXP_INITNAME;
		break;
	    case 'I': 
		type = XML_REGEXP_NOTINITNAME;
		break;
	    case 'c': 
		type = XML_REGEXP_NAMECHAR;
		break;
	    case 'C': 
		type = XML_REGEXP_NOTNAMECHAR;
		break;
	    case 'd': 
		type = XML_REGEXP_DECIMAL;
		break;
	    case 'D': 
		type = XML_REGEXP_NOTDECIMAL;
		break;
	    case 'w': 
		type = XML_REGEXP_REALCHAR;
		break;
	    case 'W': 
		type = XML_REGEXP_NOTREALCHAR;
		break;
	}
	NEXT;
	if (ctxt->atom == NULL) {
	    ctxt->atom = rtxRegNewAtom(ctxt, type);
	} else if (ctxt->atom->type == XML_REGEXP_RANGES) {
	    rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
			       type, 0, 0, NULL);
	}
    }
}

/*
 * xmlFAParseCharRef:
 * @ctxt:  a regexp parser context
 *
 * [19]   XmlCharRef   ::=   ( '&#' [0-9]+ ';' ) | (' &#x' [0-9a-fA-F]+ ';' )
 */
static int
xmlFAParseCharRef(rtxRegParserCtxtPtr ctxt) {
    int ret = 0, cur;

    if ((CUR != '&') || (NXT(1) != '#'))
	return(-1);
    NEXT;
    NEXT;
    cur = CUR;
    if (cur == 'x') {
	NEXT;
	cur = CUR;
	if (((cur >= '0') && (cur <= '9')) ||
	    ((cur >= 'a') && (cur <= 'f')) ||
	    ((cur >= 'A') && (cur <= 'F'))) {
	    while (((cur >= '0') && (cur <= '9')) ||
		   ((cur >= 'A') && (cur <= 'F'))) {
		if ((cur >= '0') && (cur <= '9'))
		    ret = ret * 16 + cur - '0';
		else if ((cur >= 'a') && (cur <= 'f'))
		    ret = ret * 16 + 10 + (cur - 'a');
		else
		    ret = ret * 16 + 10 + (cur - 'A');
		NEXT;
		cur = CUR;
	    }
	} else {
	    REGEXPERROR("Char ref: expecting [0-9A-F]");
	    return(-1);
	}
    } else {
	if ((cur >= '0') && (cur <= '9')) {
	    while ((cur >= '0') && (cur <= '9')) {
		ret = ret * 10 + cur - '0';
		NEXT;
		cur = CUR;
	    }
	} else {
	    REGEXPERROR("Char ref: expecting [0-9]");
	    return(-1);
	}
    }
    if (cur != ';') {
	REGEXPERROR("Char ref: expecting ';'");
	return(-1);
    } else {
	NEXT;
    }
    return(ret);
}

/*
 * xmlFAParseCharRange:
 * @ctxt:  a regexp parser context
 *
 * [17]   charRange   ::=     seRange | XmlCharRef | XmlCharIncDash 
 * [18]   seRange   ::=   charOrEsc '-' charOrEsc
 * [20]   charOrEsc   ::=   XmlChar | SingleCharEsc
 * [21]   XmlChar   ::=   [^\#x2D#x5B#x5D]
 * [22]   XmlCharIncDash   ::=   [^\#x5B#x5D]
 */
static void
xmlFAParseCharRange(rtxRegParserCtxtPtr ctxt) {
    int cur, len;
    int start = -1;
    int end = -1;

    if ((CUR == '&') && (NXT(1) == '#')) {
	end = start = xmlFAParseCharRef(ctxt);
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
	                   XML_REGEXP_CHARVAL, start, end, NULL);
	return;
    }
    cur = CUR;
    if (cur == '\\') {
	NEXT;
	cur = CUR;
	switch (cur) {
	    case 'n': start = 0xA; break;
	    case 'r': start = 0xD; break;
	    case 't': start = 0x9; break;
	    case '\\': case '|': case '.': case '-': case '^': case '?':
	    case '*': case '+': case '{': case '}': case '(': case ')':
	    case '[': case ']':
		start = cur; break;
	    default:
		REGEXPERROR("Invalid escape value");
		return;
	}
	end = start;
        len = 1;
    } else if ((cur != 0x5B) && (cur != 0x5D)) {
        end = start = CUR_SCHAR(ctxt->cur, len);
    } else {
	REGEXPERROR("Expecting a char range");
	return;
    }
    NEXTL(len);
    if (start == '-') {
        if (NXT(1) != '[')  /* YG [-abcd] [a-c-f-z] */      
           rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
                                 XML_REGEXP_CHARVAL, start, end, NULL);
	return;
    }
    cur = CUR;
    if ((cur == '-') && (NXT(1) == ']')) { /* YG [abcd-] */
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
                              XML_REGEXP_CHARVAL, start, end, NULL);
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
                              XML_REGEXP_CHARVAL, cur, cur, NULL);
        NEXT;
        return;
    }
    if ((cur != '-') || (NXT(1) == ']')) {
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
		              XML_REGEXP_CHARVAL, start, end, NULL);
	return;
    }
    NEXT;
    cur = CUR;
    if (cur == '\\') {
	NEXT;
	cur = CUR;
	switch (cur) {
	    case 'n': end = 0xA; break;
	    case 'r': end = 0xD; break;
	    case 't': end = 0x9; break;
	    case '\\': case '|': case '.': case '-': case '^': case '?':
	    case '*': case '+': case '{': case '}': case '(': case ')':
	    case '[': case ']':
		end = cur; break;
	    default:
		REGEXPERROR("Invalid escape value");
		return;
	}
        len = 1;
    } else if ((cur != 0x5B) && (cur != 0x5D)) {
        end = CUR_SCHAR(ctxt->cur, len);
    } else {
	REGEXPERROR("Expecting the end of a char range");
	return;
    }
    NEXTL(len);
    /* TODO check that the values are acceptable character ranges for XML */
    if (end < start) {
	REGEXPERROR("End of range is before start of range");
    } else {
        rtxRegAtomAddRange(ctxt, ctxt->atom, ctxt->neg,
		           XML_REGEXP_CHARVAL, start, end, NULL);
    }
    return;
}

/*
 * xmlFAParsePosCharGroup:
 * @ctxt:  a regexp parser context
 *
 * [14]   posCharGroup ::= ( charRange | charClassEsc  )+
 */
static void
xmlFAParsePosCharGroup(rtxRegParserCtxtPtr ctxt) {
    do {
	if (CUR == '\\') {
	    xmlFAParseCharClassEsc(ctxt);
	} else {
	    xmlFAParseCharRange(ctxt);
	}
    } while ((CUR != ']') && (CUR != '^') && (CUR != '-') &&
	     (ctxt->error == 0));
}

/*
 * xmlFAParseCharGroup:
 * @ctxt:  a regexp parser context
 *
 * [13]   charGroup    ::= posCharGroup | negCharGroup | charClassSub
 * [15]   negCharGroup ::= '^' posCharGroup
 * [16]   charClassSub ::= ( posCharGroup | negCharGroup ) '-' charClassExpr  
 * [12]   charClassExpr ::= '[' charGroup ']'
 */
static void
xmlFAParseCharGroup(rtxRegParserCtxtPtr ctxt) {
    int n = ctxt->neg;
    while ((CUR != ']') && (ctxt->error == 0)) {
	if (CUR == '^') {
	    int neg = ctxt->neg;

	    NEXT;
	    ctxt->neg = !ctxt->neg;
	    xmlFAParsePosCharGroup(ctxt);
	    ctxt->neg = neg;
	} else if ((CUR == '-') && (NXT(1) == '[')) {
	    int neg = ctxt->neg;
	    ctxt->neg = 2;
	    NEXT;	/* eat the '-' */
	    NEXT;	/* eat the '[' */
	    xmlFAParseCharGroup(ctxt);
	    if (CUR == ']') {
		NEXT;
	    } else {
		REGEXPERROR("charClassExpr: ']' expected");
		break;
	    }
	    ctxt->neg = neg;
	    break;
	} else if (CUR != ']') {
	    xmlFAParsePosCharGroup(ctxt);
	}
    }
    ctxt->neg = n;
}

/*
 * xmlFAParseCharClass:
 * @ctxt:  a regexp parser context
 *
 * [11]   charClass   ::=     charClassEsc | charClassExpr
 * [12]   charClassExpr   ::=   '[' charGroup ']'
 */
static void
xmlFAParseCharClass(rtxRegParserCtxtPtr ctxt) {
    if (CUR == '[') {
	NEXT;
	ctxt->atom = rtxRegNewAtom(ctxt, XML_REGEXP_RANGES);
	if (ctxt->atom == NULL)
	    return;
	xmlFAParseCharGroup(ctxt);
	if (CUR == ']') {
	    NEXT;
	} else {
	    REGEXPERROR("xmlFAParseCharClass: ']' expected");
	}
    } else {
	xmlFAParseCharClassEsc(ctxt);
    }
}

/*
 * xmlFAParseQuantExact:
 * @ctxt:  a regexp parser context
 *
 * [8]   QuantExact   ::=   [0-9]+
 *
 * Returns 0 if success or -1 in case of error
 */
static int
xmlFAParseQuantExact(rtxRegParserCtxtPtr ctxt) {
    int ret = 0;
    int ok = 0;

    while ((CUR >= '0') && (CUR <= '9')) {
	ret = ret * 10 + (CUR - '0');
	ok = 1;
	NEXT;
    }
    if (ok != 1) {
	return(-1);
    }
    return(ret);
}

/*
 * xmlFAParseQuantifier:
 * @ctxt:  a regexp parser context
 *
 * [4]   quantifier   ::=   [?*+] | ( '{' quantity '}' )
 * [5]   quantity   ::=   quantRange | quantMin | QuantExact
 * [6]   quantRange   ::=   QuantExact ',' QuantExact
 * [7]   quantMin   ::=   QuantExact ','
 * [8]   QuantExact   ::=   [0-9]+
 */
static int
xmlFAParseQuantifier(rtxRegParserCtxtPtr ctxt) {
    int cur;

    cur = CUR;
    if ((cur == '?') || (cur == '*') || (cur == '+')) {
	if (ctxt->atom != NULL) {
	    if (cur == '?')
		ctxt->atom->quant = XML_REGEXP_QUANT_OPT;
	    else if (cur == '*')
		ctxt->atom->quant = XML_REGEXP_QUANT_MULT;
	    else if (cur == '+')
		ctxt->atom->quant = XML_REGEXP_QUANT_PLUS;

	    ctxt->atom->origQuant = ctxt->atom->quant; /*!AB, 01/05/05 */
	}
	NEXT;
	return(1);
    }
    if (cur == '{') {
	int min = 0, max = 0;

	NEXT;
	cur = xmlFAParseQuantExact(ctxt);
	if (cur >= 0)
	    min = cur;
	if (CUR == ',') {
	    NEXT;
	    if (CUR == '}')
	        max = INT_MAX;
	    else {
	        cur = xmlFAParseQuantExact(ctxt);
	        if (cur >= 0)
		    max = cur;
		else {
		    REGEXPERROR("Improper quantifier");
		}
	    }
	}
	if (CUR == '}') {
	    NEXT;
	} else {
	    REGEXPERROR("Unterminated quantifier");
	}
	if (max == 0)
	    max = min;
	if (ctxt->atom != NULL) {
	    ctxt->atom->quant = XML_REGEXP_QUANT_RANGE;
	    ctxt->atom->min = min;
	    ctxt->atom->max = max;
	    ctxt->atom->origQuant = ctxt->atom->quant; /*!AB, 01/05/05 */
	}
	return(1);
    }
    return(0);
}

/*
 * xmlFAParseAtom:
 * @ctxt:  a regexp parser context
 *
 * [9]   atom   ::=   Char | charClass | ( '(' regExp ')' )
 */
static int
xmlFAParseAtom(rtxRegParserCtxtPtr ctxt) {
    int codepoint, len;

    codepoint = xmlFAIsChar(ctxt);
    if (codepoint > 0) {
	ctxt->atom = rtxRegNewAtom(ctxt, XML_REGEXP_CHARVAL);
	if (ctxt->atom == NULL)
	    return(-1);
	codepoint = CUR_SCHAR(ctxt->cur, len);
	ctxt->atom->codepoint = codepoint;
	NEXTL(len);
	return(1);
    } else if (CUR == '|') {
	return(0);
    } else if (CUR == 0) {
	return(0);
    } else if (CUR == ')') {
	return(0);
    } else if (CUR == '(') {
	rtxRegStatePtr start, oldend;

	NEXT;
	xmlFAGenerateEpsilonTransition(ctxt, ctxt->state, NULL);
	start = ctxt->state;
	oldend = ctxt->end;
	ctxt->end = NULL;
	ctxt->atom = NULL;
	xmlFAParseRegExp(ctxt, 0);
	if (CUR == ')') {
	    NEXT;
	} else {
	    REGEXPERROR("xmlFAParseAtom: expecting ')'");
	}
	ctxt->atom = rtxRegNewAtom(ctxt, XML_REGEXP_SUBREG);
	if (ctxt->atom == NULL)
	    return(-1);
	ctxt->atom->start = start;
	ctxt->atom->stop = ctxt->state;
	ctxt->end = oldend;
	return(1);
    } else if ((CUR == '[') || (CUR == '\\') || (CUR == '.')) {
	xmlFAParseCharClass(ctxt);
	return(1);
    }
    return(0);
}

/*
 * xmlFAParsePiece:
 * @ctxt:  a regexp parser context
 *
 * [3]   piece   ::=   atom quantifier?
 */
static int
xmlFAParsePiece(rtxRegParserCtxtPtr ctxt) {
    int ret;

    ctxt->atom = NULL;
    ret = xmlFAParseAtom(ctxt);
    if (ret == 0)
	return(0);
    if (ctxt->atom == NULL) {
	REGEXPERROR("internal: no atom generated");
    }
    xmlFAParseQuantifier(ctxt);
    return(1);
}

/* YG - some optimization for remove duplicate path in automata 
      in expressions like this (a|b)(c|d) */

/*
 * xmlFAParseBranch:
 * @ctxt:  a regexp parser context
 *
 * [2]   branch   ::=   piece*
 8
 */
static int
xmlFAParseBranch(rtxRegParserCtxtPtr ctxt, rtxRegStatePtr to) {
    /* YG - add parameter "to" */
    rtxRegStatePtr previous;
    int ret;

    previous = ctxt->state;
    ret = xmlFAParsePiece(ctxt);
    if (ret != 0) {
	/* YG - on first branch generate new state and use it for next branches */
	if (xmlFAGenerateTransitions(ctxt, previous, 
	        (CUR=='|' || CUR==')') ? to : NULL, ctxt->atom) < 0)
	    return(-1);
	previous = ctxt->state;
	ctxt->atom = NULL;
    }
    while ((ret != 0) && (ctxt->error == 0)) {
	ret = xmlFAParsePiece(ctxt);
	if (ret != 0) {
	    /* YG - reuse generated state again */
	    if (xmlFAGenerateTransitions(ctxt, previous,  
	            (CUR=='|' || CUR==')') ? to : NULL, ctxt->atom) < 0)
		return(-1);
	    previous = ctxt->state;
	    ctxt->atom = NULL;
	}
    }
    return(0);
}

/*
 * xmlFAParseRegExp:
 * @ctxt:  a regexp parser context
 * @top:  is that the top-level expressions ?
 *
 * [1]   regExp   ::=     branch  ( '|' branch )*
 */
static void
xmlFAParseRegExp(rtxRegParserCtxtPtr ctxt, int top) {
    rtxRegStatePtr start, end /*, oldend, oldstart */;

    /*  oldend = ctxt->end; */

    /*  oldstart = ctxt->state; */
    /* if not top start should have been generated by an epsilon trans */
    start = ctxt->state;
    ctxt->end = NULL;
    xmlFAParseBranch(ctxt, NULL); /* YG */
    if (top) {
#ifdef DEBUG_REGEXP_GRAPH
	printf("State %d is final\n", ctxt->state->no);
#endif
	ctxt->state->type = XML_REGEXP_FINAL_STATE;
    }
    if (CUR != '|') {
	ctxt->end = ctxt->state;
	return;
    }
    end = ctxt->state;
    while ((CUR == '|') && (ctxt->error == 0)) {
	NEXT;
	ctxt->state = start;
	ctxt->end = NULL;
	xmlFAParseBranch(ctxt, end);
#if 0 /* YG */
	if (top) {
	    ctxt->state->type = XML_REGEXP_FINAL_STATE;
#ifdef DEBUG_REGEXP_GRAPH
	    printf("State %d is final\n", ctxt->state->no);
#endif
	} else {
	    xmlFAGenerateEpsilonTransition(ctxt, ctxt->state, end);
	}
#endif  /* YG */
    }
    if (!top) {
	ctxt->state = end;
	ctxt->end = end;
    }
}

/************************************************************************
 *                                                                      *
 *        The basic API                                                 *
 *                                                                      *
 ************************************************************************/

/*
 * rtxRegexpPrint:
 * @output: the file for the output debug
 * @regexp: the compiled regexp
 *
 * Print the content of the compiled regular expression
 */
/* Moved in rtxRegExpPrint.c */

#if 0
/* YG patch */
static void xmlCountedTransitionsSwap(rtxRegParserCtxtPtr ctxt) {
   int i,j;
   rtxRegStatePtr state;
   rtxRegTrans tm;

   for(i=0; i<ctxt->nbStates; i++) {
      state=ctxt->states[i];
      if (state) for(j=0; j<state->nbTrans; j++) {
         if(j>0 && state->trans[j].to>=0 && state->trans[j].atom==0 &&
               state->trans[j-1].counter==state->trans[j].count)
         {
            /* swap */
            OSCRTLMEMCPY(&tm,state->trans+(j-1),sizeof(rtxRegTrans));
            OSCRTLMEMCPY(state->trans+(j-1),state->trans+j,sizeof(rtxRegTrans));
            OSCRTLMEMCPY(state->trans+j,&tm,sizeof(rtxRegTrans));
         }
      }
   }
}
#endif

/*
 * rtxRegexpCompile:
 * @regexp:  a regular expression string
 *
 * Parses a regular expression conforming to XML Schemas Part 2 Datatype
 * Appendix F and build an automata suitable for testing strings against
 * that regular expression
 *
 * Returns the compiled expression or NULL in case of error
 */
EXTRTMETHOD rtxRegexpPtr rtxRegexpCompile (OSCTXT* pOSCTXT, const OSUTF8CHAR *regexp) 
{
    rtxRegexpPtr ret;
    rtxRegParserCtxtPtr ctxt;

    ctxt = rtxRegNewParserCtxt (pOSCTXT, regexp);
    if (ctxt == NULL)
	return(NULL);

    /* initialize the parser */
    ctxt->end = NULL;
    ctxt->start = ctxt->state = rtxRegNewState(ctxt);
    rtxRegStatePush(ctxt, ctxt->start);

    /* parse the expression building an automata */
    xmlFAParseRegExp(ctxt, 1);
    if (CUR != 0) {
	REGEXPERROR("xmlFAParseRegExp: extra characters");
    }
    ctxt->end = ctxt->state;
    ctxt->start->type = XML_REGEXP_START_STATE;
    ctxt->end->type = XML_REGEXP_FINAL_STATE;

    /* remove the Epsilon except for counted transitions */
    xmlFAEliminateEpsilonTransitions(ctxt);

    /* YG */
    /*xmlCountedTransitionsSwap(ctxt);*/

    if (ctxt->error != 0) {
	rtxRegFreeParserCtxt(ctxt);
	return(NULL);
    }
    ret = rtxRegEpxFromParse(ctxt);
    rtxRegFreeParserCtxt(ctxt);
    return(ret);
}

/*
 * rtxRegexpExec:
 * @comp:  the compiled regular expression
 * @content:  the value to check against the regular expression
 *
 * Check if the regular expression generate the value
 *
 * Returns 1 if it matches, 0 if not and a negativa value in case of error
 */
EXTRTMETHOD int rtxRegexpExec 
(OSCTXT* pOSCTXT, rtxRegexpPtr comp, const OSUTF8CHAR *content) 
{
    if ((comp == NULL) || (content == NULL))
	return(-1);

    return (xmlFARegExec (pOSCTXT, comp, content));
}

/*
 * rtxRegFreeRegexp:
 * @regexp:  the regexp
 *
 * Free a regexp
 */
EXTRTMETHOD void rtxRegFreeRegexp (OSCTXT* pOSCTXT, rtxRegexpPtr regexp) 
{
   int i;
   if (regexp == NULL)
      return;

   if (regexp->string != NULL)
      rtxMemSysFreeArray (pOSCTXT, regexp->string);

   if (regexp->states != NULL) {
      for (i = 0;i < regexp->nbStates;i++)
         rtxRegFreeState (pOSCTXT, regexp->states[i]);

      rtxMemSysFreeArray (pOSCTXT, regexp->states);
   }

   if (regexp->atoms != NULL) {
      for (i = 0;i < regexp->nbAtoms;i++)
         rtxRegFreeAtom (pOSCTXT, regexp->atoms[i]);

      rtxMemSysFreeArray (pOSCTXT, regexp->atoms);
   }

   if (regexp->counters != NULL)
      rtxMemSysFreeArray (pOSCTXT, regexp->counters);

   if (regexp->compact != NULL)
      rtxMemSysFreeArray (pOSCTXT, regexp->compact);

   if (regexp->transdata != NULL)
      rtxMemSysFreeArray (pOSCTXT, regexp->transdata);

   if (regexp->stringMap != NULL) {
      for (i = 0; i < regexp->nbstrings;i++)
         rtxMemSysFreeArray (pOSCTXT, regexp->stringMap[i]);

      rtxMemSysFreeArray (pOSCTXT, regexp->stringMap);
   }

   rtxMemSysFreeType (pOSCTXT, regexp);
}

