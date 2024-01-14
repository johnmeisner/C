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
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/osMacros.h"

EXTRTMETHOD void rtxPrintBoolean (const char* name, OSBOOL value)
{
   printf ("%s = %s\n", name, value ? "true" : "false");
}

EXTRTMETHOD void rtxPrintInteger (const char* name, OSINT32 value)
{
   printf ("%s = %d\n", name, value);
}

EXTRTMETHOD void rtxPrintInt64 (const char* name, OSINT64 value)
{
   printf ("%s = "OSINT64FMT"\n", name, value);
}

EXTRTMETHOD void rtxPrintIpv4Addr(const char* name, size_t numocts,
                                  const OSOCTET* data)
{
   OSSIZE i;

   printf("%s = ", name);

   for (i = 0; i < numocts; i++)
   {
      if (i != 0)
      {
         printf(".");
      }
      printf("%d", data[i]);
   }

   printf("\n");
}

EXTRTMETHOD void rtxPrintIpv6Addr(const char* name, size_t numocts,
                                  const OSOCTET* data)
{
   OSSIZE i;

   printf("%s = ", name);

   for (i = 0; i < numocts; i++)
   {
      if ((i != 0) && (i % 2 == 0))
      {
         printf(":");
      }
      printf("%x", data[i]);
   }

   printf("\n");
}

EXTRTMETHOD void rtxPrintTBCDStr(const char* name, size_t numocts,
                                 const OSOCTET* data)
{
   OSUINT8 b;
   OSSIZE i, idx = 0;

   printf("%s = ", name);

   for (i = 0; i < numocts * 2; i++)
   {
      if (i % 2 == 0)
      {
         b = data[idx] & 0xF;
      }
      else
      {
         b = data[idx++] >> 4;
      }

      if (b < 0xA)
      {
         putchar((char)(b + '0'));
      }
      else if (b == 0xA)
      {
         putchar('*');
      }
      else if (b == 0xB)
      {
         putchar('#');
      }
      else if (b == 0xC)
      {
         putchar('a');
      }
      else if (b == 0xD)
      {
         putchar('b');
      }
      else if (b == 0xE)
      {
         putchar('c');
      }
   }
}

EXTRTMETHOD void rtxPrintText(const char* name, size_t numocts,
                              const OSOCTET* data)
{
   OSSIZE i;

   printf("%s = ", name);

   for (i = 0; i < numocts; i++)
   {
      if (OS_ISPRINTABLE(data[i]))
      {
         printf("%c", (char)data[i]);
      }
   }

   printf("\n");
}

EXTRTMETHOD void rtxPrintUnsigned (const char* name, OSUINT32 value)
{
   printf ("%s = %u\n", name, value);
}

EXTRTMETHOD void rtxPrintUInt64 (const char* name, OSUINT64 value)
{
   printf ("%s = "OSUINT64FMT"\n", name, value);
}

EXTRTMETHOD void rtxPrintCharStr (const char* name, const char* cstring)
{
   printf ("%s = '%s'\n", name,
      (cstring == 0) ? "(null)" : cstring);
}

EXTRTMETHOD void rtxPrintUTF8CharStr
(const char* name, const OSUTF8CHAR* cstring)
{
   printf ("%s = '%s'\n", name,
      (cstring == 0) ? "(null)" : (const char*)cstring);
}

EXTRTMETHOD void rtxPrintUnicodeCharStr64
(const char* name, const OSUNICHAR* str, OSSIZE nchars)
{
   OSSIZE i;
   if (0 == str) return;
   printf ("%s = '", name);
   for (i = 0; i < nchars; i++) {
      if (OS_ISPRINT (str[i]))
         putc (str[i], stdout);
      else
         printf ("0x%04x", str[i]);
   }
   printf ("'\n");
}

EXTRTMETHOD void rtxPrintUnicodeCharStr
(const char* name, const OSUNICHAR* str, int nchars)
{
   if (0 == str) return;
   if (nchars < 0) {
      nchars = 0;
      while (str[nchars] != 0) nchars++;
   }
   rtxPrintUnicodeCharStr64 (name, str, nchars);
}

EXTRTMETHOD void rtxPrintHexStr
(const char* name, size_t numocts, const OSOCTET* data)
{
   if (numocts <= 32) {
      OSUINT32 i;
      printf ("%s = 0x", name);
      for (i = 0; i < numocts; i++) {
         printf ("%02x", data[i]);
      }
      printf ("\n");
   }
   else {
      printf ("%s =\n", name);
      rtxHexDump (data, numocts);
   }
}

EXTRTMETHOD void rtxPrintHexStrPlain
(const char* name, size_t numocts, const OSOCTET* data)
{
   OSUINT32 i;
   printf ("%s = 0x", name);
   for (i = 0; i < numocts; i++) {
      printf ("%02x", data[i]);
   }
   printf ("\n");
}

EXTRTMETHOD void rtxPrintNVP (const char* name, const OSUTF8NVP* pnvp)
{
   printf ("%s.name  = '%s'\n", name,
      (pnvp->name == 0) ? "(null)" : (const char*)pnvp->name);
   printf ("%s.value = '%s'\n", name,
      (pnvp->value == 0) ? "(null)" : (const char*)pnvp->value);
}

EXTRTMETHOD void rtxPrintArrayNVP
(const char* name, OSSIZE subscript, const OSUTF8NVP* pnvp)
{
   printf ("%s[" OSSIZEFMT "].name  = '%s'\n", name, subscript,
      (pnvp->name == 0) ? "(null)" : (const char*)pnvp->name);
   printf ("%s[" OSSIZEFMT "].value = '%s'\n", name, subscript,
      (pnvp->value == 0) ? "(null)" : (const char*)pnvp->value);
}

EXTRTMETHOD void rtxPrintHexBinary
(const char* name, size_t numocts, const OSOCTET* data)
{
   printf ("%s.numocts = " OSSIZEFMT "\n", name, numocts);
   printf ("%s.data = \n", name);
   rtxHexDump (data, numocts);
}

EXTRTMETHOD void rtxPrintReal (const char* name, OSREAL value)
{
   printf ("%s = %.12g\n", name, value);
}

EXTRTMETHOD void rtxPrintNull (const char* name)
{
   printf ("%s = <null>\n", name);
}

/* Indentation for brace text printing */

static OSUINT32 g_indent = 0;

EXTRTMETHOD void rtxPrintIndent ()
{
   OSUINT32 i;
   for (i = 0; i < g_indent; i++) printf (" ");
}

EXTRTMETHOD void rtxPrintIncrIndent ()
{
   g_indent += OSRTINDENTSPACES;
}

EXTRTMETHOD void rtxPrintDecrIndent ()
{
   if (g_indent > 0)
      g_indent -= OSRTINDENTSPACES;
}

EXTRTMETHOD void rtxPrintCloseBrace ()
{
   rtxPrintDecrIndent ();
   rtxPrintIndent ();
   printf ("}\n");
}

EXTRTMETHOD void rtxPrintOpenBrace (const char* name)
{
   rtxPrintIndent ();
   printf ("%s {\n", name);
   rtxPrintIncrIndent ();
}

EXTRTMETHOD const char* rtxGetArrayElemName
(char* buffer, OSSIZE bufsize, const char* name, OSSIZE subscript)
{
   char numbuf[32];
   return (rtxSizeToCharStr(subscript, numbuf, sizeof(numbuf), 0) < 0) ?
      NULL : rtxStrJoin(buffer, bufsize, name, "[", numbuf, "]", 0);
}
