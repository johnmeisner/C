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
 *  32-bit character (UCS-4) utility routines
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: rtUCSToCString, rtUCSToNewCString
 *
 *  Description:  These routines convert UCS-4 strings (strings made up
 *                of 32-bit characters) into standard, null-terminated
 *                C character strings.  These functions assume the
 *                string does not contain any extended characters (i.e.
 *                characters that will not fit in an 8-bit char value).
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ----            -----------
 *  pUCSString  ASN1UniversalString*  Pointer to structure containing UCS-4
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

EXTRTMETHOD const char* rtUCSToCString
(ASN1UniversalString* pUCSString, char* cstring, OSSIZE cstrsize)
{
   OSSIZE i, j;
   for (i = 0, j = 0; i < pUCSString->nchars; i++) {
      if (pUCSString->data[i] < 256) {
         cstring[j++] = (char)pUCSString->data[i];
         if (j >= cstrsize) return cstring;
      }
   }
   cstring[j] = '\0';
   return cstring;
}

/* This version of the function allocates memory using malloc for       */
/* the converted string value..                                         */

EXTRTMETHOD const char* rtUCSToNewCString (ASN1UniversalString* pUCSString)
{
#if defined(_NO_THREADS) || !defined(_NO_MALLOC)
   char* cstring = (char*) OSCRTMALLOC0 (pUCSString->nchars + 1);
   if (cstring == NULL) return 0;
   return rtUCSToCString (pUCSString, cstring, pUCSString->nchars + 1);
#else
   return 0;
#endif
}

/* This version of the function allocates memory using rtxMemAlloc for   */
/* the converted string value..                                         */

EXTRTMETHOD const char* rtUCSToNewCStringEx (OSCTXT* pctxt, ASN1UniversalString* pUCSString)
{
   char* cstring = (char*) rtxMemAlloc (pctxt, pUCSString->nchars + 1);
   if (cstring == NULL) return 0;
   return rtUCSToCString (pUCSString, cstring, pUCSString->nchars + 1);
}

/***********************************************************************
 *
 *  Routine name: rtCToUCSString
 *
 *  Description:  This routine converts a standard, null-terminated
 *                C character string into a UCS-4 string.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pctxt      OSCTXT*               Pointer to ASN.1 context structure
 *                                      (used for mem alloc function calls)
 *  cstring     const char*        C string to be converted
 *  pCharSet    Asn132BitCharSet*       Pointer to constraining character
 *                                      set (optional).  If present,
 *                                      function will drop all characters
 *                                      not in set.
 *
 *  Outputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pUCSString  ASN1UniversalString*    Target UCS-4 string.  This pointer
 *                                      value is also returned as the
 *                                      function result.
 *
 *
 **********************************************************************/

EXTRTMETHOD ASN1UniversalString* rtCToUCSString (OSCTXT* pctxt,
                                     const char* cstring,
                                     ASN1UniversalString* pUCSString,
                                     Asn132BitCharSet* pCharSet)
{
   OSUINT32 i;
   OS32BITCHAR tmpchr;
   size_t len = strlen(cstring);

   pUCSString->nchars = 0;
   pUCSString->data = (OS32BITCHAR*)
      rtxMemAlloc (pctxt, len * sizeof(OS32BITCHAR));
   if (pUCSString->data == NULL) return 0;

   for (i = 0; i < len; i++) {
      tmpchr = (OSOCTET) cstring[i];
      if (pCharSet) {
         if (!rtIsIn32BitCharSet (tmpchr, pCharSet))
            continue;
      }
      pUCSString->data[pUCSString->nchars++] = tmpchr;
   }

   return pUCSString;
}

/***********************************************************************
 *
 *  Routine name: rtIsIn32BitCharSet
 *
 *  Description:  This routine checks the given 32-bit character
 *                against the given character set and returns true
 *                if it is within it.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  ch          OS32BITCHAR           32-bit character
 *  pCharSet    Asn132BitCharSet*       Character set to compare
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

EXTRTMETHOD OSBOOL rtIsIn32BitCharSet (OS32BITCHAR ch, Asn132BitCharSet* pCharSet)
{
   OSUINT32 i;
   if (ch < pCharSet->firstChar || ch >  pCharSet->lastChar) return FALSE;
   if (pCharSet->charSet.nchars == 0) return TRUE;
   for (i = 0; i < pCharSet->charSet.nchars; i++) {
      if (ch == pCharSet->charSet.data[i]) return TRUE;
   }
   return FALSE;
}

/***********************************************************************
 *
 *  Routine name: rtUCSToWCSString
 *
 *  Description:  These routine converts UCS-4 string (string made up
 *                of 32-bit characters) into null-terminated WCS (wide-char
 *                string, made up 16-bit characters).
 *                This function assumes the string does not contain any
 *                extended characters (i.e. characters that will not fit in
 *                a 16-bit wchar_t value).
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ----            -----------
 *  pUCSString  ASN1UniversalString*  Pointer to structure containing UCS-4
 *                              string to be converted.
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----        ----            -----------
 *  result      wchar_t*        Pointer to converted string (returned
 *                              as function result)
 *  wcstring    wchar_t*        Buffer to receive converted string
 *  wcstrsize   OSUINT32        String buffer size
 *
 *
 **********************************************************************/

EXTRTMETHOD wchar_t* rtUCSToWCSString (ASN1UniversalString* pUCSString,
                           wchar_t* wcstring, OSUINT32 wcstrsize)
{
   OSUINT32 i, j;
   for (i = 0, j = 0; i < pUCSString->nchars; i++) {
      if (pUCSString->data[i] < 65536) {
         wcstring[j++] = (wchar_t)pUCSString->data[i];
         if (j >= wcstrsize) return wcstring;
      }
   }
   wcstring[j] = '\0';
   return wcstring;
}

/***********************************************************************
 *
 *  Routine name: rtWCSToUCSString
 *
 *  Description:  This routine converts a wide-char, null-terminated
 *                16-bit character string into a UCS-4 string.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pctxt      OSCTXT*               Pointer to ASN.1 context structure
 *                                      (used for mem alloc function calls)
 *  cstring     wchar_t*                Wide-char string to be converted
 *  pCharSet    Asn132BitCharSet*       Pointer to constraining character
 *                                      set (optional).  If present,
 *                                      function will drop all characters
 *                                      not in set.
 *
 *  Outputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pUCSString  ASN1UniversalString*    Target UCS-4 string.  This pointer
 *                                      value is also returned as the
 *                                      function result.
 *
 *
 **********************************************************************/

EXTRTMETHOD ASN1UniversalString* rtWCSToUCSString
(OSCTXT* pctxt, wchar_t* wcstring, ASN1UniversalString* pUCSString,
 Asn132BitCharSet* pCharSet)
{
   OSUINT32 i;
   OS32BITCHAR tmpchr;
   size_t len = 0;

   while (wcstring[len] != 0)
      len++;

   pUCSString->nchars = 0;
   pUCSString->data = (OS32BITCHAR*)
      rtxMemAlloc (pctxt, len * sizeof(OS32BITCHAR));
   if (pUCSString->data == NULL) return 0;

   for (i = 0; i < len; i++) {
      tmpchr = (OSUINT16) wcstring[i];
      if (pCharSet) {
         if (!rtIsIn32BitCharSet (tmpchr, pCharSet))
            continue;
      }
      pUCSString->data[pUCSString->nchars++] = tmpchr;
   }

   return pUCSString;
}

EXTRTMETHOD int rtUnivStrToUTF8
(OSCTXT* pctxt, const ASN1UniversalString* pUnivStr, OSOCTET* outbuf,
 size_t outbufsiz)
{
   const OSUINT32 encoding_mask[] = {
      0xfffff800, 0xffff0000, 0xffe00000, 0xfc000000
   };
   const unsigned char encoding_byte[] = {
      0xc0, 0xe0, 0xf0, 0xf8, 0xfc
   };
   size_t inbufx, outbufx = 0;
   size_t step, start;
   OS32BITCHAR wc;

   if (0 == pUnivStr) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   for (inbufx = 0; inbufx < pUnivStr->nchars; inbufx++) {
      wc = pUnivStr->data[inbufx];

      if (wc < 0x80) {
         /* One byte sequence */
         if (outbufx >= outbufsiz)
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
         else
            outbuf[outbufx++] = (char) wc;
      }
      else {
         for (step = 2; step < 6; ++step)
            if ((wc & encoding_mask[step - 2]) == 0)
               break;

         if (outbufx + step >= outbufsiz)
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);

         /* Copy multi-byte sequence to output buffer */

         start = outbufx;
         outbufx += step;
         outbuf[start] = encoding_byte[step - 2];
         --step;
         do {
            outbuf[start + step] = (OSOCTET) (0x80 | (wc & 0x3f));
            wc >>= 6;
         }
         while (--step > 0);
         outbuf[start] |= wc;
      }
   }

   if (outbufx < outbufsiz)
      outbuf[outbufx] = 0;
   else
      return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);

   return 0;
}
