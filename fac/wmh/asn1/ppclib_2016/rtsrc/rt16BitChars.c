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

/***********************************************************************
 *
 *  16-bit character utility routines
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: rtxBMPToCString, rtxBMPToNewCString
 *
 *  Description:  These routines convert BMP strings (strings made up
 *                of 16-bit characters) into standard, null-terminated
 *                C character strings.  These functions assume the
 *                string does not contain any extended characters (i.e.
 *                characters that will not fit in an 8-bit char value).
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ----            -----------
 *  pBMPString  ASN1BMPString*  Pointer to structure containing BMP
 *                              string to be converted.
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ----            -----------
 *  result      char*           Pointer to converted string (returned
 *                              as function result)
 *  cstring     char*           Buffer to receive converted string
 *  cstrsize    OSUINT32        String buffer size
 *
 *
 **********************************************************************/

#include <stdlib.h>
#include "rtsrc/asn1type.h"

EXTRTMETHOD const char* rtBMPToCString
(ASN1BMPString* pBMPString, char* cstring, OSSIZE cstrsize)
{
   OSSIZE i, j;
   for (i = 0, j = 0; i < pBMPString->nchars; i++) {
      if (pBMPString->data[i] < 256) {
         cstring[j++] = (char)pBMPString->data[i];
         if (j >= cstrsize) return cstring;
      }
   }
   cstring[j] = '\0';
   return cstring;
}

/* This version of the function allocates memory using malloc for       */
/* the converted string value..                                         */

EXTRTMETHOD const char* rtBMPToNewCString (ASN1BMPString* pBMPString)
{
#if defined(_NO_THREADS) || !defined(_NO_MALLOC)
   char* cstring = (char*) OSCRTMALLOC0 (pBMPString->nchars + 1);
   if (cstring == NULL) return 0;
   return rtBMPToCString (pBMPString, cstring, pBMPString->nchars + 1);
#else
   return 0;
#endif
}

/* This version of the function allocates memory using rtxMemAalloc for  */
/* the converted string value..                                         */

EXTRTMETHOD const char* rtBMPToNewCStringEx (OSCTXT *pctxt, ASN1BMPString* pBMPString)
{
   char* cstring = (char*) rtxMemAlloc (pctxt, pBMPString->nchars + 1);
   if (cstring == NULL) return 0;
   return rtBMPToCString (pBMPString, cstring, pBMPString->nchars + 1);
}

/***********************************************************************
 *
 *  Routine name: rtCToBMPString
 *
 *  Description:  This routine converts a standard, null-terminated
 *                C character string into a BMP string.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pctxt       OSCTXT*               Pointer to ASN.1 context structure
 *                                      (used for mem alloc function calls)
 *  cstring     const char*           C string to be converted
 *  pCharSet    Asn116BitCharSet*       Pointer to constraining character
 *                                      set (optional).  If present,
 *                                      function will drop all characters
 *                                      not in set.
 *
 *  Outputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pBMPString  ASN1BMPString*          Target BMP string.  This pointer
 *                                      value is also returned as the
 *                                      function result.
 *
 *
 **********************************************************************/

EXTRTMETHOD ASN1BMPString* rtCToBMPString (OSCTXT* pctxt,
                               const char* cstring,
                               ASN1BMPString* pBMPString,
                               Asn116BitCharSet* pCharSet)
{
   OSUINT32 i;
   OSUNICHAR tmpchr;

   pBMPString->nchars = 0;
   pBMPString->data = (OSUNICHAR*)
      rtxMemAlloc (pctxt, strlen(cstring) * sizeof(OSUNICHAR));
   if (pBMPString->data == NULL) return 0;

   for (i = 0; i < strlen(cstring); i++) {
      tmpchr = (OSOCTET) cstring[i];
      if (pCharSet) {
         if (!rtIsIn16BitCharSet (tmpchr, pCharSet))
            continue;
      }
      pBMPString->data[pBMPString->nchars++] = tmpchr;
   }

   return pBMPString;
}

/***********************************************************************
 *
 *  Routine name: rtIsIn16BitCharSet
 *
 *  Description:  This routine checks the given 16-bit character
 *                against the given character set and returns true
 *                if it is within it.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  ch          OSUNICHAR           16-bit character
 *  pCharSet    Asn116BitCharSet*       Character set to compare
 *                                      character against
 *
 *  Outputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  result      OSBOOL                Boolean return value: true if
 *                                      character within set.
 *
 **********************************************************************/

EXTRTMETHOD OSBOOL rtIsIn16BitCharSet (OSUNICHAR ch, Asn116BitCharSet* pCharSet)
{
   OSUINT32 i;
   if (ch < pCharSet->firstChar || ch >  pCharSet->lastChar) return FALSE;
   if (pCharSet->charSet.nchars == 0) return TRUE;
   for (i = 0; i < pCharSet->charSet.nchars; i++) {
      if (ch == pCharSet->charSet.data[i]) return TRUE;
   }
   return FALSE;
}
