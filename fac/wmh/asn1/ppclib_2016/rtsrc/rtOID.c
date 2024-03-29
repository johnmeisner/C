/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

/* Run-time utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxCtype.h"

/* Object identifier helper functions */

void rtSetOID (ASN1OBJID* ptarget, ASN1OBJID* psource)
{
   OSUINT32 ui;
   ptarget->numids = psource->numids;
   for (ui = 0; ui < psource->numids && ui < ASN_K_MAXSUBIDS; ui++) {
      ptarget->subid[ui] = psource->subid[ui];
   }
}

/* Adds one OID to another */

void rtAddOID (ASN1OBJID* ptarget, ASN1OBJID* psource)
{
   OSUINT32 ui, ti = ptarget->numids;
   for (ui = 0; ui < psource->numids && ti < ASN_K_MAXSUBIDS; ui++, ti++) {
      ptarget->subid[ti] = psource->subid[ui];
   }
   ptarget->numids = ti;
}

/* Compare two OID's for equality */

OSBOOL rtOIDsEqual (const ASN1OBJID* pOID1, const ASN1OBJID* pOID2)
{
   if (pOID1->numids == pOID2->numids) {
      OSUINT32 ui;
      for (ui = 0; ui < pOID1->numids; ui++) {
         if (pOID1->subid[ui] != pOID2->subid[ui]) {
            return FALSE;
         }
      }
      return TRUE;
   }

   return FALSE;
}

/**
 * Return length of identifier that begins at str.
 * @param str String that begins with identifier.
 * @param len Length of string (must be greater than zero)
 * @return length of identifier (<= len)
 */
static OSSIZE readIdentifier(const char* str, OSSIZE len)
{
   OSSIZE idx = 1;   /* all chars before idx are part of identifier */
   for(; idx < len; idx++) {
      char c = str[idx];
      if ( !( OS_ISALNUM(c) || c == '-' ) ) break;
   }
   return idx;
}


/**
 * Read integer (digits only) from given string, which must begin with
 * a digit.
 * @param str String to read from; begins with a digit
 * @param len Length of string
 * @param pval Receives the value.
 * @param pvalLen Receives the length of the integer read.
 * @return RTERR_BADVALUE if there is overflow.
 */
static int readInteger(const char* str, OSSIZE len, OSUINT32* pval,
                        OSSIZE* pvalLen)
{
   OSSIZE idx;      /* all chars before idx are digits */
   OSUINT32 intVal = 0; /* integer value read */

   for(idx = 0; idx < len; idx++)
   {
      char c = str[idx];
      if ( OS_ISDIGIT(c) )
      {
         OSUINT32 sum = intVal * 10 + (c - '0');
         if ( sum >= intVal ) intVal = sum;
         else return RTERR_BADVALUE;   /* overflow */
      }
      else break;
   }

   *pvalLen = idx;
   *pval = intVal;

   return 0;
}

typedef enum {
   START,   /* start of OID parse */
	ITU_T,   /* parent is oid { 0 } */
	ISO,     /* parent is oid { 1 } */
	ITU_T_REC, /* parent is oid { 0 0 } */
	OTHER    /* none of the above */
} OidParseState;


struct OidParseStateEntry {
   const char* identifier;    /* identifier to match */
   OidParseState newState;    /* state to transition to */
   OSUINT32 intVal;           /* integer value for this identifier */
};

struct OidParseStateEntries {
   struct OidParseStateEntry* entries;
   OSINT32 numEntries;
};

static struct OidParseStateEntry opse_start[] =
      {  { "itu-t", ITU_T, 0 },
         { "ccitt", ITU_T, 0 },
         { "iso", ISO, 1},
         { "joint-iso-itu-t", OTHER, 2 },
         { "joint-iso-ccitt", OTHER, 2 }
      };

static struct OidParseStateEntry opse_itu_t[] =
      {  { "recommendation", ITU_T_REC, 0 },
         { "question", OTHER, 1 },
         { "administration", OTHER, 2 },
         { "network-operator", OTHER, 3 },
         { "identified-organization", OTHER, 4}
      };

static struct OidParseStateEntry opse_iso[] =
      {  {"standard", OTHER, 0 },
         {"registration-authority", OTHER, 1 },
         {"member-body", OTHER, 2 },
         {"identified-organization", OTHER, 3 }
      };


/* The OID parse table. The table is indexed by the parse state. */
static struct OidParseStateEntries oidParseTable[] =
{
   /* START */
   { opse_start, sizeof(opse_start)/sizeof(struct OidParseStateEntry) },

   /* ITU_T */
   { opse_itu_t, sizeof(opse_itu_t)/sizeof(struct OidParseStateEntry) },

   /* ISO */
   { opse_iso, sizeof(opse_iso)/sizeof(struct OidParseStateEntry) },

   /* ITU_T_REC */
   { 0, 0 },

   /* OTHER */
   { 0, 0 }
};

/**
 * Determine the next OID parser state from either the given identifier or
 * the given integer value, based on the current state.
 *
 * If idenfifier is null, then pvalue must supply the integer value of the
 * next component.  Otherwise, identifier identifies the next component.
 *
 * @param pState On input, the current parse state (which must not be OTHER).
 *       This serves to identify what OID arc we're under. On output, the new
 *       state.
 * @param identifier The identifier used for the current component, or null.
 * @param idLen The length of identifier.
 * @param pNewState Receives the new parser state.
 * @param pvalue If identifier is not null, this receives the integer value
 *       corresponding to identifier, if the function returns TRUE.
 *       If identifier is null, the supplies the integer value for the
 *       next component.
 * @return If identifier is not null, then TRUE if the function could determine
 *       the integer value and FALSE otherwise.  If the identifier is null,
 *       the return is TRUE.
 */
static OSBOOL updateOidParseState( OidParseState* pState,
                        const char* identifier,
                        OSSIZE idLen, OSUINT32* pvalue)
{
   if ( *pState == ITU_T_REC )
   {
      *pState = OTHER;
      if ( idLen == 1 && identifier != NULL && OS_ISLOWER(*identifier) )
      {
         /* Under {itu recommendation}, predefined subarcs include 'a' to 'z'
            and are numbered 1..26.
         */
         *pvalue = *identifier - 'a' + 1;
         return TRUE;
      }
   }
   else {
      int i;
      for( i = 0; i < oidParseTable[*pState].numEntries; i++)
      {
         const char* id = oidParseTable[*pState].entries[i].identifier;

         if ( identifier == NULL &&
               *pvalue == oidParseTable[*pState].entries[i].intVal )
         {
            *pState = oidParseTable[*pState].entries[i].newState;
            return TRUE;
         }
         else if ( identifier != NULL &&
            idLen == strlen( id ) && 0 == strncmp( id, identifier, idLen))
         {
            *pvalue = oidParseTable[*pState].entries[i].intVal;
            *pState = oidParseTable[*pState].entries[i].newState;
            return TRUE;
         }
      }
   }

   /* No entry matches */
   *pState = OTHER;
   return identifier == NULL;
}

/**
 * Shared implementation for parsing XMLObjectIdentifierValue and
 * XMLRelativeOIDValue.
 * @param bRelative If TRUE, the input is a relative OID, which means that no
 *    component can be name only.
 */
static int rtOIDParseStringInternal
(const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue, OSBOOL bRelative)
{
   OidParseState state = START;  /* track what arc we're under */
   OSSIZE i;

   if (0 == oidstr || 0 == pvalue) return RTERR_BADVALUE;

   pvalue->numids = 0;

   /* skip leading whitespace */
   for(; oidstrlen > 0 && OS_ISSPACE(*oidstr); oidstrlen--, oidstr++)

   if ( 0 == oidstrlen ) return RTERR_BADVALUE;

   for(;;) {
      /* PRE: oidstr points to beginning of next unprocessed component.
              oidstrlen is the length of the unprocessed portion of the
               original string.
      */
      const char* identifier = 0;   /* identifier, if we have one */
      OSSIZE idLen = 0;             /* length of identifier, if we have one */
      OSSIZE compLen;               /* length of component */
      OSBOOL bSetInt;               /* true when integer is set */
      OSUINT32 intVal = 0;          /* component integer value */

      char c = *oidstr;

      if ( OS_ISLOWER(c) ) {
         /* component must be identifier or identifier(number) */
         identifier = oidstr;
         idLen = readIdentifier(oidstr, oidstrlen);

         if ( idLen == oidstrlen || oidstr[idLen] == '.' ||
               oidstr[idLen] == ' ')
         {
            /* A truly legal XMLObjectIdentifierValue does not allow spaces.
               I take a space as the end of the components. */
            compLen = idLen;
         }
         else if ( oidstr[idLen] == '(' ) {
            /* read integer, should go to closing ) */
            if ( OS_ISDIGIT( oidstr[idLen+1] ) )
            {
               OSSIZE intValLen;

               int ret = readInteger(oidstr + idLen + 1, oidstrlen - idLen - 1,
                                 &intVal, &intValLen);
               if ( ret < 0 ) return ret;

               compLen = idLen + 1 + intValLen;

               if ( compLen == oidstrlen || oidstr[compLen] != ')')
               {
                  /* string terminated before ')', or something other than ')'
                     followed the integer */
                  return RTERR_BADVALUE;
               }
               else compLen++; /* int followed by ), +1 for the ) */
            }
            else return RTERR_BADVALUE;
         }
         else {
            /* unexpected stuff after the identifier */
            return RTERR_BADVALUE;
         }
      }
      else if ( OS_ISDIGIT(c) )
      {
         /* component is number only */
         OSSIZE intValLen;
         int ret = readInteger(oidstr, oidstrlen, &intVal, &intValLen);
         if ( ret < 0 ) return ret;

         if ( intValLen != oidstrlen && oidstr[intValLen] != ' ' &&
               oidstr[intValLen] != '.')
         {
            /* integer is followed by something unexpected */
            return RTERR_BADVALUE;
         }
         /* else: integer ends with string or is followed by space or '.'.
            If followed by space, the string should end with whitespace.*/

         compLen = intValLen;
      }
      else {
         return RTERR_BADVALUE;
      }

      /* If we got an identifier and an integer value, clear the identifier
         as we don't need it and don't want to pass it to updateOidParseState.
      */
      if ( identifier != 0 && compLen > idLen )
      {
         identifier = 0;
         idLen = 0;
      }

      /* Use state, identifier, and/or number to update state.
         If we have have the number, we do not pass the identifier.
         If bRelative is true, state is irrelevant b/c we cannot derive
         integer value from identifiers.
         If state is OTHER, it remains OTHER forevermore and we can't determine
         the integer value (we must already have it).
      */
      bSetInt = !bRelative && state != OTHER &&
                  updateOidParseState(&state, identifier, idLen, &intVal);

      /* If we needed to set the integer & failed to do so, report an error.
         compLen == idLen implies we had only the identifier which means we
         needed to get the integer value.
      */
      if ( compLen == idLen && !bSetInt ) return RTERR_BADVALUE;

      /* use integer to add to the obj id array */
      if (pvalue->numids >= ASN_K_MAXSUBIDS) {
         return ASN_E_INVOBJID;
      }
      pvalue->subid[pvalue->numids++] = intVal;

      if ( oidstr[compLen] == '.' ) {
         /* move to next component */
         oidstr = oidstr + compLen + 1;
         oidstrlen = oidstrlen - compLen - 1;
      }
      else {
         /* no more components */
         oidstr = oidstr + compLen;
         oidstrlen = oidstrlen - compLen;
         break;
      }
   }

   /* error if anything other than spaces remains */
   for(i = 0; i < oidstrlen; i++)
   {
      if ( !OS_ISSPACE(oidstr[i]) ) return RTERR_BADVALUE;
   }

   return 0;
}


/* Parse a dotted number string which is the form of XML content.
   This can be used to parse either an OID or a relative OID value.
   OID-specific rules are not checked.  This can be done by calling
   rtOIDIsValid after parsing. */

int rtOIDParseDottedNumberString
(const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue)
{
   OSUINT32 i, val = 0;
   OSBOOL   digitParsed = FALSE;

   if (0 == oidstr || 0 == oidstrlen || 0 == pvalue) return RTERR_BADVALUE;

   pvalue->numids = 0;

   for (i = 0; i < oidstrlen; i++) {
      if (OS_ISSPACE (oidstr[i])) {
         continue;
      }
      else if (OS_ISDIGIT (oidstr[i])) {
         val = (val * 10) + (oidstr[i] - '0');
         digitParsed = TRUE;
      }
      else if (oidstr[i] == '.') {
         if (pvalue->numids >= ASN_K_MAXSUBIDS) {
            return ASN_E_INVOBJID;
         }
         pvalue->subid[pvalue->numids++] = val;
         digitParsed = FALSE;
         val = 0;
      }
      else if (oidstr[i] == '\0') break;
   }

   if (digitParsed && pvalue->numids < ASN_K_MAXSUBIDS) {
      pvalue->subid[pvalue->numids++] = val;
   }
   else return ASN_E_INVOBJID;

   return 0;
}

/* This function checks to see if an OID value is valid by applying
   ASN.1 rules */

OSBOOL rtOIDIsValid (const ASN1OBJID* pvalue)
{
   if (0 != pvalue && pvalue->numids >= 2 && pvalue->subid[0] <= 2) {
      if (pvalue->subid[0] < 2) return (pvalue->subid[1] <= 39);
      else return TRUE;
   }
   return FALSE;
}


/**
 * This function parses an OID that is an XMLObjectIdentifierValue.
 * Components that use only a name can be converted if that form was used
 * as permitted by X.680 & X.660.  Leading and trailing whitespace is ignored.
 *
 * @param oidstr        OID string containing data to be parsed.
 * @param oidstrlen     Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
int rtOIDParseString (const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue)
{
   return rtOIDParseStringInternal(oidstr, oidstrlen, pvalue, FALSE);
}



/**
 * This function parses an OID that is an XMLRelativeOIDValue.
 * Name-only components are not supported.  Leading and trailing whitespace is
 * ignored.
 *
 * @param oidstr        OID string containing data to be parsed.
 * @param oidstrlen     Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
int rtRelOIDParseString (const char* oidstr, OSSIZE oidstrlen,
                           ASN1OBJID* pvalue)
{
   return rtOIDParseStringInternal(oidstr, oidstrlen, pvalue, TRUE);
}
