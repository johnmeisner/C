/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

/* Utility function to convert hex characters to binary form.  The function
   prototype is in rtxCharStr.h */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"

/* This function returns the number of bytes that a conversion of the
   given hex string to binary will generate */

EXTRTMETHOD int rtxHexCharsToBinCount (const char* hexstr, size_t nchars)
{
   int nbytes = 0;
   size_t i;

   if (0 == nchars) nchars = OSCRTLSTRLEN (hexstr);

   for (i = 0; i < nchars; i++) {
      if (!OS_ISSPACE (hexstr[i]) && hexstr[i] != ',') {
         /* skip 0x prefix */
         if (hexstr[i] == '0' && OS_TOLOWER(hexstr[i+1]) == 'x') {
            i += 2;
         }
         if (!OS_ISXDIGIT (hexstr[i])) {
            return RTERR_INVHEXS;
         }

         /* There may be 1 or 2 hex chars that follow */
         if (i < nchars && OS_ISXDIGIT (hexstr[i+1])) i++;

         nbytes++;
      }
   }

   return nbytes;
}

/* This function converts the given hex string to binary.  The result is
   stored in the given binary buffer */

static int hexCharToNibble (char c, OSOCTET* hbyte)
{
   if (c >= '0' && c <= '9')
      *hbyte = (OSOCTET)(c - '0');
   else if (c >= 'A' && c <= 'F')
      *hbyte = (OSOCTET)(c - 'A' + 10);
   else if (c >= 'a' && c <= 'f')
      *hbyte = (OSOCTET)(c - 'a' + 10);
   else
      return RTERR_INVHEXS;

   return 0;
}

EXTRTMETHOD int rtxHexCharsToBin
(const char* hexstr, size_t nchars, OSOCTET* binbuf, size_t bufsize)
{
   size_t i, di = 0, start_i;
   size_t numHexChars = 0;
   OSOCTET hbyte;
   OSBOOL delimitted = FALSE;

   if (OS_ISEMPTY (hexstr)) return 0;
   if (0 == nchars) nchars = OSCRTLSTRLEN (hexstr);

   /* Count all non-whitespace chars */

   for (i = 0; i < nchars; i++) {
      if (!OS_ISSPACE (hexstr[i]) && hexstr[i] != ',') {
         /* skip 0x prefix */
         if (hexstr[i] == '0' && OS_TOLOWER(hexstr[i+1]) == 'x') {
            i++;
         }
         else if (!OS_ISXDIGIT (hexstr[i])) {
            return RTERR_INVHEXS;
         }
         else numHexChars++;
      }
   }

   /* Skip leading whitespace */

   for (i = 0; i < nchars; i++) {
      if (!OS_ISSPACE (hexstr[i])) break;
   }

   start_i = i;

   /* Determine if string is delimited */

   for ( ; i < nchars; i++) {
      if (OS_ISSPACE (hexstr[i]) || hexstr[i] == ',') {
         delimitted = TRUE;
         break;
      }
   }

   /* Handle case of string having odd number of bytes */

   i = start_i;

   if ((numHexChars % 2) != 0 && !delimitted) {
      /* skip 0x prefix */
      if (hexstr[i] == '0' && OS_TOLOWER(hexstr[i+1]) == 'x') {
         i += 2;
      }
      OS_HEXCHARTONIBBLE (hexstr[i], binbuf[di]);
      i++; di++;
   }

   /* Convert characters to binary */

   for ( ; i < nchars; i++) {
      if (!OS_ISSPACE (hexstr[i]) && hexstr[i] != ',') {
         int ret;

         /* skip 0x prefix */
         if (hexstr[i] == '0' && OS_TOLOWER(hexstr[i+1]) == 'x') {
            i += 2;
         }
         if (di >= bufsize) {
            return RTERR_STROVFLW;
         }

         /* Convert first hex character */
         ret = hexCharToNibble (hexstr[i], &hbyte);
         if (0 != ret) return ret;

         binbuf [di] = hbyte;

         /* Check for 2nd character; if delimitted, there may only be 1 */
         if (i < nchars && OS_ISXDIGIT(hexstr[i+1])) {
            ret = hexCharToNibble (hexstr[++i], &hbyte);
            if (0 != ret) return ret;

            binbuf[di] <<= 4;
            binbuf[di] |= (hbyte & 0xF);
         }

         di++;
      }
   }

   return (int) di;
}
