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

/* Run-time print utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxPrintToStream.h"
#include "rtxsrc/rtxError.h"

EXTRTMETHOD void rtxPrintToStreamBoolean
(OSCTXT* pctxt, const char* name, OSBOOL value)
{
   rtxPrintToStream (pctxt, "%s = %s\n", name, value ? "true" : "false");
}

EXTRTMETHOD void rtxPrintToStreamInteger
(OSCTXT* pctxt, const char* name, OSINT32 value)
{
   rtxPrintToStream (pctxt, "%s = %d\n", name, value);
}

EXTRTMETHOD void rtxPrintToStreamInt64
(OSCTXT* pctxt, const char* name, OSINT64 value)
{
   rtxPrintToStream (pctxt, "%s = "OSINT64FMT"\n", name, value);
}

EXTRTMETHOD void rtxPrintToStreamUnsigned
(OSCTXT* pctxt, const char* name, OSUINT32 value)
{
   rtxPrintToStream (pctxt, "%s = %u\n", name, value);
}

EXTRTMETHOD void rtxPrintToStreamUInt64
(OSCTXT* pctxt, const char* name, OSUINT64 value)
{
   rtxPrintToStream (pctxt, "%s = "OSUINT64FMT"\n", name, value);
}

EXTRTMETHOD void rtxPrintToStreamCharStr
(OSCTXT* pctxt, const char* name, const char* cstring)
{
   rtxPrintToStream (pctxt, "%s = '%s'\n", name,
      (cstring == 0) ? "(null)" : (const char*)cstring);
}

EXTRTMETHOD void rtxPrintToStreamUTF8CharStr
(OSCTXT* pctxt, const char* name, const OSUTF8CHAR* cstring)
{
   rtxPrintToStream (pctxt, "%s = '%s'\n", name,
      (cstring == 0) ? "(null)" : (const char*)cstring);
}

EXTRTMETHOD void rtxPrintToStreamUnicodeCharStr
(OSCTXT* pctxt, const char* name, const OSUNICHAR* str, int nchars)
{
   int i;
   if (0 == str) return;
   if (nchars < 0) {
      nchars = 0;
      while (str[nchars] != 0) nchars++;
   }
   rtxPrintToStream (pctxt, "%s = '", name);
   for (i = 0; i < nchars; i++) {
      if (OS_ISPRINT (str[i]))
         rtxPrintToStream (pctxt, "%c", str[i]);
      else
         rtxPrintToStream (pctxt, "0x%04x", str[i]);
   }
   rtxPrintToStream (pctxt, "'\n");
}

EXTRTMETHOD void rtxPrintToStreamHexStr
(OSCTXT* pctxt, const char* name, OSSIZE numocts, const OSOCTET* data)
{
   if (numocts <= 32) {
      OSSIZE i;
      rtxPrintToStream (pctxt, "%s = 0x", name);
      for (i = 0; i < numocts; i++) {
         rtxPrintToStream (pctxt, "%02x", data[i]);
      }
      rtxPrintToStream (pctxt, "\n");
   }
   else {
      rtxPrintToStream (pctxt, "%s =\n", name);
      rtxHexDumpToStream (pctxt, data, numocts);
   }
}

EXTRTMETHOD void rtxPrintToStreamHexStrPlain
(OSCTXT* pctxt, const char* name, OSSIZE numocts, const OSOCTET* data)
{
   OSSIZE i;
   rtxPrintToStream (pctxt, "%s = 0x", name);
   for (i = 0; i < numocts; i++) {
      rtxPrintToStream (pctxt, "%02x", data[i]);
   }
   rtxPrintToStream (pctxt, "\n");
}

EXTRTMETHOD void rtxPrintToStreamHexStrNoAscii
(OSCTXT* pctxt, const char* name, OSSIZE numocts, const OSOCTET* data)
{
   rtxPrintToStream (pctxt, "%s =\n", name);
   rtxHexDumpToStreamExNoAscii (pctxt, data, numocts, 1);
}

EXTRTMETHOD void rtxPrintToStreamNVP
(OSCTXT* pctxt, const char* name, const OSUTF8NVP* pnvp)
{
   rtxPrintToStream (pctxt, "%s.name  = '%s'\n", name,
      (pnvp->name == 0) ? "(null)" : (const char*)pnvp->name);
   rtxPrintToStream (pctxt, "%s.value = '%s'\n", name,
      (pnvp->value == 0) ? "(null)" : (const char*)pnvp->value);
}

EXTRTMETHOD void rtxPrintToStreamHexBinary
(OSCTXT* pctxt, const char* name, OSSIZE numocts, const OSOCTET* data)
{
   rtxPrintToStream (pctxt, "%s.numocts = " OSSIZEFMT "\n", name, numocts);
   rtxPrintToStream (pctxt, "%s.data = \n", name);
   rtxHexDumpToStream (pctxt, data, numocts);
}

EXTRTMETHOD void rtxPrintToStreamReal
(OSCTXT* pctxt, const char* name, OSREAL value)
{
   rtxPrintToStream (pctxt, "%s = %g\n", name, value);
}

EXTRTMETHOD void rtxPrintToStreamNull (OSCTXT* pctxt, const char* name)
{
   rtxPrintToStream (pctxt, "%s = <null>\n", name);
}

EXTRTMETHOD int rtxPrintToStreamFile (OSCTXT* pctxt, const char* filename)
{
   char lbuf[1024];
   FILE *fp = 0;

   if (0 != rtxFileOpen (&fp, filename, "r")) {
      return (RTERR_FILNOTFOU);
   }

   while (fgets (lbuf, sizeof(lbuf), fp)) {
      rtxPrintToStream (pctxt, lbuf);
   }

   fclose (fp);

   return 0;
}

#ifndef __SYMBIAN32__
/* Indentation for brace text printing */
static OSUINT32 g_indent = 0;
#endif

EXTRTMETHOD void rtxPrintToStreamIndent (OSCTXT* pctxt)
{
   OSUINT32 i;

   if (0 == pctxt) {
#ifndef __SYMBIAN32__
      for (i = 0; i < g_indent; i ++) rtxPrintToStream (pctxt, " ");
#endif
   }
   else {
      for (i = 0; i < pctxt->indent; i++) rtxPrintToStream (pctxt, " ");
   }
}

EXTRTMETHOD void rtxPrintToStreamResetIndent (OSCTXT* pctxt)
{
   if (0 == pctxt) {
#ifndef __SYMBIAN32__
      g_indent = 0;
#endif
   }
   else {
      pctxt->indent = 0;
   }
}

EXTRTMETHOD void rtxPrintToStreamIncrIndent (OSCTXT* pctxt)
{
   if (0 == pctxt) {
#ifndef __SYMBIAN32__
      g_indent += OSRTINDENTSPACES;
#endif
   }
   else {
      pctxt->indent += OSRTINDENTSPACES;
   }
}

EXTRTMETHOD void rtxPrintToStreamDecrIndent (OSCTXT* pctxt)
{
   if (0 == pctxt && g_indent > 0) {
#ifndef __SYMBIAN32__
      g_indent -= OSRTINDENTSPACES;
#endif
   }
   else if (pctxt != 0 && pctxt->indent > 0) {
      pctxt->indent -= OSRTINDENTSPACES;
   }
}

EXTRTMETHOD void rtxPrintToStreamCloseBrace (OSCTXT* pctxt)
{
   rtxPrintToStreamDecrIndent (pctxt);
   rtxPrintToStreamIndent (pctxt);
   rtxPrintToStream (pctxt, "}\n");
}

EXTRTMETHOD void rtxPrintToStreamOpenBrace (OSCTXT* pctxt, const char* name)
{
   rtxPrintToStreamIndent (pctxt);
   rtxPrintToStream (pctxt, "%s {\n", name);
   rtxPrintToStreamIncrIndent (pctxt);
}

/* Hex dump functions */

static char* strTrim (char *s)
{
   int i = (int)OSCRTLSTRLEN(s) - 1;
   while (i >= 0 && s[i] == ' ') i--;
   s[i+1] = '\0';
   return (s);
}

EXTRTMETHOD void rtxHexDumpToStream
(OSCTXT* pctxt, const OSOCTET* data, OSSIZE numocts)
{
   rtxHexDumpToStreamEx (pctxt, data, numocts, 1);
}

EXTRTMETHOD void rtxHexDumpToStreamEx
(OSCTXT* pctxt, const OSOCTET* data, OSSIZE numocts, OSSIZE bytesPerUnit)
{
   OSSIZE   i;
   OSUINT32 val;
   OSOCTET  b, dumpstr = 1;
   char     hexstr[49], ascstr[17], buf[21], *pbuf;
   OSSIZE   k, unitsPerLine = 16 / bytesPerUnit, ai, shift, avail_buf;

   if (numocts == 0 || data == 0) return;

   OSCRTLMEMSET (hexstr, ' ', 48); hexstr[48] = '\0';
   OSCRTLMEMSET (ascstr, ' ', 16); ascstr[16] = '\0';
   ai = 0;

   if (bytesPerUnit == 0 || bytesPerUnit > 4) bytesPerUnit = 4;

   for (i = 0; i < numocts / bytesPerUnit; i++)
   {
      pbuf = (char*)buf;
      avail_buf = (sizeof(buf) - 1); /* 20 characters + NUL. */
      buf [bytesPerUnit * 2] = 0;
      if (bytesPerUnit == 1) {
         val = *data++;
         shift = 0;
      }
      else if (bytesPerUnit == 2) {
         val = *data++;
         val = (val * 256) + *data++;
         shift = 8;
      }
      else { /* greater than 2 */
         OSSIZE ii;
         for (ii = 0, val = 0; ii < sizeof (OSUINT32); ii++) {
            val = (val * 256) + *data++;
         }
         shift = (sizeof (OSUINT32) - 1) * 8;
      }
      for (k = 0; k < bytesPerUnit;
           k++, pbuf += 2, avail_buf -= 2, shift -= 8)
      {
         b = (OSOCTET)((val >> shift) & 0xFF);
         /* Maximum bytesPerUnit = 4;
            4 * 2 bytes per hex representation of byte = 8;
            Will never overflow 20 character buffer. */
         if (0 != rtxByteToHexChar (b, pbuf, avail_buf)) break;
         ascstr[ai++] = (char) (OS_ISPRINT(b) ? b : '.');
      }
      *pbuf = 0;

      k = i % unitsPerLine * (bytesPerUnit * 2 + 1);
      OSCRTLSAFEMEMCPY (&hexstr[k], sizeof(hexstr) - k, buf, bytesPerUnit * 2);

      if ((dumpstr = (OSOCTET) ((i + 1) % unitsPerLine == 0)) != 0)
      {
         rtxPrintToStream (pctxt, "%48s %16s\n", hexstr, ascstr);
         if (i < (numocts - 1)) {
            OSCRTLMEMSET (hexstr, ' ', 48);
            OSCRTLMEMSET (ascstr, ' ', 16);
            ai = 0;
         }
      }
   }

   if (!dumpstr)
      rtxPrintToStream (pctxt, "%48s %s\n", hexstr, strTrim(ascstr));
}

EXTRTMETHOD void rtxHexDumpToStreamExNoAscii
(OSCTXT* pctxt, const OSOCTET* data, OSSIZE numocts, OSSIZE bytesPerUnit)
{
   OSSIZE   i;
   OSUINT32 val;
   OSOCTET  b, dumpstr = 1;
   char     hexstr[49], buf[21], *pbuf;
   OSSIZE   k, unitsPerLine = 16 / bytesPerUnit, shift, avail_buf;

   if (numocts == 0 || data == 0) return;

   OSCRTLMEMSET (hexstr, ' ', 48); hexstr[48] = '\0';

   if (bytesPerUnit == 0 || bytesPerUnit > 4) bytesPerUnit = 4;

   for (i = 0; i < numocts / bytesPerUnit; i++)
   {
      pbuf = (char*)buf;
      avail_buf = (sizeof(buf) - 1); /* 20 characters + NUL. */
      buf [bytesPerUnit * 2] = 0;
      if (bytesPerUnit == 1) {
         val = *data++;
         shift = 0;
      }
      else if (bytesPerUnit == 2) {
         val = *data++;
         val = (val * 256) + *data++;
         shift = 8;
      }
      else { /* greater than 2 */
         OSSIZE ii;
         for (ii = 0, val = 0; ii < sizeof (OSUINT32); ii++) {
            val = (val * 256) + *data++;
         }
         shift = (sizeof (OSUINT32) - 1) * 8;
      }
      for (k = 0; k < bytesPerUnit;
           k++, pbuf += 2, avail_buf -= 2, shift -= 8)
      {
         b = (OSOCTET)((val >> shift) & 0xFF);
         /* Maximum bytesPerUnit = 4;
            4 * 2 bytes per hex representation of byte = 8;
            Will never overflow 20 character buffer. */
         if (0 != rtxByteToHexChar (b, pbuf, avail_buf)) break;
      }
      *pbuf = 0;

      k = i % unitsPerLine * (bytesPerUnit * 2 + 1);
      OSCRTLSAFEMEMCPY (&hexstr[k], sizeof(hexstr), buf, bytesPerUnit * 2);

      if ((dumpstr = (OSOCTET) ((i + 1) % unitsPerLine == 0)) != 0)
      {
         rtxPrintToStream (pctxt, "%48s\n", hexstr);
         if (i < (numocts - 1)) {
            OSCRTLMEMSET (hexstr, ' ', 48);
         }
      }
   }

   if (!dumpstr)
      rtxPrintToStream (pctxt, "%48s\n", hexstr);
}
