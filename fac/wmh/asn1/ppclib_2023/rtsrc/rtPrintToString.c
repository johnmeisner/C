/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

/* Run-time print to string utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/rtPrintToString.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxDateTime.h"

EXTRTMETHOD int rtPrintToStringBoolean
(const char* name, OSBOOL value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE endidx = OSCRTLSTRLEN (buffer);
   if (endidx < bufferSize) {
      OSSIZE rembytes = bufferSize - endidx;
      const char* strval = value ? "TRUE" : "FALSE";
      if (rtxStrJoin (&buffer[endidx], rembytes,
                      name, " = ", strval, "\n", 0)) {
         return 0;
      }
   }
   return -1;
}

EXTRTMETHOD int rtPrintToStringInteger
(const char* name, OSINT32 value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE endidx = OSCRTLSTRLEN (buffer);
   if (endidx < bufferSize) {
      OSSIZE rembytes = bufferSize - endidx;
      char numbuf[80];
      if (rtxIntToCharStr (value, numbuf, sizeof(numbuf), 0) >= 0 &&
          rtxStrJoin (&buffer[endidx], rembytes,
                      name, " = ", numbuf, "\n", 0)) {
         return 0;
      }
   }
   return -1;
}

EXTRTMETHOD int rtPrintToStringInt64
(const char* name, OSINT64 value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE endidx = OSCRTLSTRLEN (buffer);
   if (endidx < bufferSize) {
      OSSIZE rembytes = bufferSize - endidx;
      char numbuf[80];
      if (rtxInt64ToCharStr (value, numbuf, sizeof(numbuf), 0) >= 0 &&
          rtxStrJoin (&buffer[endidx], rembytes,
                      name, " = ", numbuf, "\n", 0)) {
         return 0;
      }
   }
   return -1;
}

EXTRTMETHOD int rtPrintToStringIpv4Addr(const char* name, size_t numocts,
                               const OSOCTET* data,
                               char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, i, rembytes;
   char*  bufptr;

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, " = ", 0, 0, 0) == NULL)
   {
      return -1;
   }

   for (i = 0; i < numocts; i++)
   {
      bufferIndex = OSCRTLSTRLEN(buffer);
      if (bufferIndex >= bufferSize)
      {
         return -1;
      }

      bufptr = buffer + bufferIndex;
      if (i != 0)
      {
         if (NULL == rtxStrcat(bufptr++, bufferSize, "."))
         {
            return -1;
         }
      }
      os_snprintf(bufptr, bufferSize, "%d", data[i]);
   }

   if (NULL == rtxStrcat(bufptr, bufferSize, "\n"))
   {
      return -1;
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStringIpv6Addr(const char* name, size_t numocts,
                               const OSOCTET* data,
                               char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, i, rembytes;
   char*  bufptr;

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, " = ", 0, 0, 0) == NULL)
   {
      return -1;
   }

   for (i = 0; i < numocts; i++)
   {
      bufferIndex = OSCRTLSTRLEN(buffer);
      if (bufferIndex >= bufferSize)
      {
         return -1;
      }

      bufptr = buffer + bufferIndex;
      if ((i != 0) && (i % 2 == 0))
      {
         if (NULL == rtxStrcat(bufptr++, bufferSize, ":"))
         {
            return -1;
         }
      }
      os_snprintf(bufptr, bufferSize, "%x", data[i]);
   }

   if (NULL == rtxStrcat(bufptr, bufferSize, "\n"))
   {
      return -1;
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStringTBCDStr(const char* name, size_t numocts,
                               const OSOCTET* data,
                               char* buffer, OSSIZE bufferSize)
{
   OSUINT8 b;
   OSSIZE bufferIndex, i, idx = 0, rembytes;
   char*  bufptr;

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, " = ", 0, 0, 0) == NULL)
   {
      return -1;
   }

   for (i = 0; i < numocts * 2; i++)
   {
      bufferIndex = OSCRTLSTRLEN(buffer);
      if (bufferIndex >= bufferSize)
      {
         return -1;
      }
      bufptr = buffer + bufferIndex;

      if ((i % 2) == 0)
      {
         b = data[idx] & 0xF;
      }
      else
      {
         b = data[idx++] >> 4;
      }

      if (b < 0xA)
      {
         os_snprintf(bufptr, bufferSize, "%c", (char)(b + '0'));
      }
      else if (b == 0xA)
      {
         if (NULL == rtxStrcat(bufptr, bufferSize, "*"))
         {
            return -1;
         }
      }
      else if (b == 0xB)
      {
         if (NULL == rtxStrcat(bufptr, bufferSize, "#"))
         {
            return -1;
         }
      }
      else if (b == 0xC)
      {
         if (NULL == rtxStrcat(bufptr, bufferSize, "a"))
         {
            return -1;
         }
      }
      else if (b == 0xD)
      {
         if (NULL == rtxStrcat(bufptr, bufferSize, "b"))
         {
            return -1;
         }
      }
      else if (b == 0xE)
      {
         if (NULL == rtxStrcat(bufptr, bufferSize, "c"))
         {
            return -1;
         }
      }
   }

   if (NULL == rtxStrcat(bufptr, bufferSize, "\n"))
   {
      return -1;
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStringDateTime(const char* name,
                               OSNumDateTime *pDateTime,
                               char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, " = ", 0, 0, 0) == NULL)
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;
   if (rtxDateTimeToString(pDateTime, (OSUTF8CHAR *)bufptr, rembytes) < 0)
   {
      return -1;
   }

   if (NULL == rtxStrcat(bufptr, bufferSize, "\n"))
   {
      return -1;
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStringText(const char* name, size_t numocts,
                               const OSOCTET* data,
                               char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, i, rembytes;
   char*  bufptr;

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, " = ", 0, 0, 0) == NULL)
   {
      return -1;
   }

   for (i = 0; i < numocts; i++)
   {
      bufferIndex = OSCRTLSTRLEN(buffer);
      if (bufferIndex >= bufferSize)
      {
         return -1;
      }

      bufptr = buffer + bufferIndex;
      if (OS_ISPRINTABLE(data[i]))
      {
         os_snprintf(bufptr, bufferSize, "%c", (char)data[i]);
      }
   }

   if (NULL == rtxStrcat(bufptr, bufferSize, "\n"))
   {
      return -1;
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStringUnsigned
(const char* name, OSUINT32 value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE endidx = OSCRTLSTRLEN (buffer);
   if (endidx < bufferSize) {
      OSSIZE rembytes = bufferSize - endidx;
      char numbuf[80];
      if (rtxUIntToCharStr (value, numbuf, sizeof(numbuf), 0) >= 0 &&
          rtxStrJoin (&buffer[endidx], rembytes,
                      name, " = ", numbuf, "\n", 0)) {
         return 0;
      }
   }
   return -1;
}

EXTRTMETHOD int rtPrintToStringUInt64
(const char* name, OSUINT64 value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE endidx = OSCRTLSTRLEN (buffer);
   if (endidx < bufferSize) {
      OSSIZE rembytes = bufferSize - endidx;
      char numbuf[80];
      if (rtxUInt64ToCharStr (value, numbuf, sizeof(numbuf), 0) >= 0 &&
          rtxStrJoin (&buffer[endidx], rembytes,
                      name, " = ", numbuf, "\n", 0)) {
         return 0;
      }
   }
   return -1;
}

#ifndef BITSTR_BYTES_IN_LINE
#define BITSTR_BYTES_IN_LINE 16
#endif

static int rtBitStringDumpToString (OSSIZE numbits, const OSOCTET* data,
                                    char* buffer, OSSIZE bufferSize)
{
   OSSIZE i, numocts = numbits / 8;
   char buff[9];

   if (numocts > 8) {
      if (rtPrintToString("\n", buffer, bufferSize) < 0 ) {
         return -1;
      }
   }

   for (i = 0; i < numocts; i++) {
      if ((i != 0) && (i % BITSTR_BYTES_IN_LINE == 0)) {
         if (rtPrintToString("\n", buffer, bufferSize) < 0 ) {
            return -1;
         }
      }

      if (i % BITSTR_BYTES_IN_LINE == 0)
         rtxByteToHexCharWithPrefix(data[i], buff, sizeof(buff), "0x");
      else
         rtxByteToHexCharWithPrefix(data[i], buff, sizeof(buff), " 0x");

      if (rtPrintToString(buff, buffer, bufferSize) < 0 ) {
         return -1;
      }
   }

   if (i * 8 != numbits) {
      OSOCTET tm = data[i];
      OSSIZE nmBits = numbits % 8;
      OSSIZE j;

      if ((i % BITSTR_BYTES_IN_LINE) == (BITSTR_BYTES_IN_LINE - 1)) {
         if (rtPrintToString("\n", buffer, bufferSize) < 0 ) {
            return -1;
         }
      }
      else if (i > 0) {
         if (rtPrintToString(" ", buffer, bufferSize) < 0 ) {
            return -1;
         }
      }

      for (j = 0; j < nmBits; j++, tm<<=1)
         buff[j] = ((tm >> 7) & 1) + '0';
      for (; j < 8; j++)
         buff[j] = 'x';
      buff[8] = 0;

      if (rtPrintToString(buff, buffer, bufferSize) < 0 ) {
         return -1;
      }
   }

   return 0;
}

static int rtBitStringDumpBinToString(OSSIZE numbits, const OSOCTET *data,
                                      char *buffer, OSSIZE bufferSize)
{
   int mask = 128, pos = 0;
   OSUINT32 i;

   for (i = 0; i < numbits; i++)
   {
      if (pos == 8)
      {
         pos = 0;
      }

      if (data[i/8] & (mask >> pos))
      {
         if (rtPrintToString("1", buffer, bufferSize) < 0)
         {
            return -1;
         }
      }
      else
      {
         if (rtPrintToString("0", buffer, bufferSize) < 0)
         {
            return -1;
         }
      }
      pos++;
   }

   return 0;
}

static int rtBitStringDumpNamedToString(OSSIZE numbits, const OSOCTET* data,
   OSSIZE namedbits, const OSUINT32 positions[], const char* bitnames[],
   char* buffer, OSSIZE bufferSize)
{
   OSSIZE stringSize = 0;
   OSSIZE index = 0;
   OSSIZE remaining;
   OSUINT32 i, j;
   char* outputString;
   OSSIZE* nameLengths;
   int mask = 128, pos = 0;
   int status;

   /*
   The output string will include the names of all named bits, so calculate
   that size first.  Also allocate memory for an array to hold the lengths
   of the names so we minimize how many times we call strlen().
   */
   nameLengths = (OSSIZE*)OSCRTLMALLOC (sizeof (OSSIZE) * namedbits);
   if (NULL == nameLengths) return -1;
   for (i = 0; i < namedbits; i++)
   {
      size_t length = strlen (bitnames[i]);
      stringSize += length;
      nameLengths[i] = length;
   }

   /*
   Each bit will have a left parenthesis, a 1 or a 0, and a right parenthesis.
   So figure that in.
   */
   stringSize += (numbits * 3);

   /*
   Each bit except the last one will be followed by a comma and a space.
   */
   stringSize += ((numbits - 1) * 2);

   /*
   And finally there will be a left parenthesis at the beginning, a right
   parenthesis at the end, and a terminating null.
   */
   stringSize += 3;

   outputString = (char*)OSCRTLMALLOC (stringSize);
   if (NULL == outputString)
   {
      OSCRTLFREE (nameLengths);
      return -1;
   }
   remaining = stringSize;

   /* Start with the beginning left parenthesis.  */
   outputString[index++] = '(';
   remaining--;

   /*
   The positions array has the positions that have corresponding names.  So
   for each bit position in the bit string, we need to search that array to
   see if that position has a name.
   */
   for (i = 0; i < numbits; i++)
   {
      OSBOOL bHasName = FALSE;
      int nameIndex = 0;
      for (j = 0; j < namedbits; j++)
      {
         if (positions[j] == i)
         {
            bHasName = TRUE;
            nameIndex = j;
            break;
         }
      }
      if (bHasName)
      {
         OSCRTLSAFEMEMCPY (&outputString[index], remaining,
            bitnames[nameIndex], nameLengths[nameIndex]);
         index += nameLengths[nameIndex];
         remaining -= nameLengths[nameIndex];
      }

      /* Now a left parenthesis.  */
      outputString[index++] = '(';
      remaining--;

      /* Now 1 or 0. */
      if (pos == 8) pos = 0;
      if (data[i / 8] & (mask >> pos))
         outputString[index++] = '1';
      else
         outputString[index++] = '0';
      remaining--;
      pos++;

      /* Now a right parenthesis.  */
      outputString[index++] = ')';
      remaining--;

      /* Now a comma and a space, unless we just did the final bit.  */
      if (i != (numbits - 1))
      {
         outputString[index++] = ',';
         outputString[index++] = ' ';
         remaining -= 2;
      }
   }

   /* Now the terminating right parenthesis and the trailing null.  */
   outputString[index++] = ')';
   outputString[index] = '\0';
   status = rtPrintToString (outputString, buffer, bufferSize);
   OSCRTLFREE (outputString);
   OSCRTLFREE (nameLengths);
   return (0 == status ? 0 : -1);
}


EXTRTMETHOD int rtPrintToStringBitStr
(const char* name, OSSIZE numbits, const OSOCTET* data,
 const char* conn, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr (numbits, numbuf, sizeof(numbuf), 0) < 0) {
      return -1;
   }
   if (rtxStrJoin (bufptr, rembytes,
                   name, conn, "numbits = ", numbuf, "\n") == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, conn, "data = ", 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtBitStringDumpToString (numbits, data, bufptr, rembytes) < 0) {
      return -1;
   }

   if (rtPrintToString("\n", bufptr, rembytes) < 0 ) {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrBin(const char *name, OSSIZE numbits,
                                         const OSOCTET *data, const char *conn,
                                         char *buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr(numbits, numbuf, sizeof(numbuf), 0) < 0)
   {
      return -1;
   }

   if (rtxStrJoin(bufptr, rembytes, name, conn,
                  "numbits = ", numbuf, "\n") == 0)
   {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN(bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, conn, "data = ", 0, 0) == 0)
   {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN(bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtBitStringDumpBinToString(numbits, data, bufptr, rembytes) < 0)
   {
      return -1;
   }

   if (rtPrintToString("\n", bufptr, rembytes) < 0 )
   {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrNamed(const char* name, OSSIZE numbits,
   const OSOCTET* data, const char* conn, OSSIZE namedBits,
   const OSUINT32 positions[], const char* bitnames[], char* buffer,
   OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char* bufptr;
   char   numbuf[50];

   if ((buffer == NULL) || (bufferSize == 0))
   {
      return -1;
   }

   bufferIndex = OSCRTLSTRLEN(buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr(numbits, numbuf, sizeof(numbuf), 0) < 0)
   {
      return -1;
   }

   if (rtxStrJoin(bufptr, rembytes, name, conn,
      "numbits = ", numbuf, "\n") == 0)
   {
      return -1;
   }

   if (rtPrintToStringIndent(buffer, bufferSize) < 0) {
      return -1;
   }
   bufferIndex += OSCRTLSTRLEN(bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin(bufptr, rembytes, name, conn, "data = ", 0, 0) == 0)
   {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN(bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtBitStringDumpNamedToString(numbits, data, namedBits, positions, bitnames, bufptr, rembytes) < 0)
   {
      return -1;
   }

   if (rtPrintToString("\n", bufptr, rembytes) < 0)
   {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrNamedExt(const char* name, OSSIZE numbits,
   const OSOCTET* data, OSSIZE dataSize, const OSOCTET* extdata,
   const char* conn, OSSIZE namedBits, const OSUINT32 positions[],
   const char* bitnames[], char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char* bufptr;
   OSSIZE dataSizeBits = dataSize * 8;
   char   numbuf[50];

   if (numbits > dataSizeBits)
   {
      if ((buffer == NULL) || (bufferSize == 0))
      {
         return -1;
      }

      bufferIndex = OSCRTLSTRLEN(buffer);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxSizeToCharStr(numbits, numbuf, sizeof(numbuf), 0) < 0)
      {
         return -1;
      }

      if (rtxStrJoin(bufptr, rembytes, name, conn,
         "numbits = ", numbuf, "\n") == 0)
      {
         return -1;
      }

      if (rtPrintToStringIndent(buffer, bufferSize) < 0) {
         return -1;
      }
      bufferIndex += OSCRTLSTRLEN(bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxStrJoin(bufptr, rembytes, name, conn, "data = ", 0, 0) == 0)
      {
         return -1;
      }

      bufferIndex += OSCRTLSTRLEN(bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtBitStringDumpNamedToString(numbits, data, namedBits, positions, bitnames, bufptr, rembytes) < 0)
      {
         return -1;
      }

      if (NULL != extdata)
      {
         if (rtPrintToString(" ", bufptr, rembytes) < 0)
         {
            return -1;
         }
         if (rtBitStringDumpToString(numbits - dataSizeBits, extdata, bufptr,
            rembytes) < 0)
         {
            return -1;
         }
      }

      if (rtPrintToString("\n", bufptr, rembytes) < 0)
      {
         return -1;
      }
   }
   else
   {
      if (0 != rtPrintToStringBitStrNamed(name, numbits, data, conn,
         namedBits, positions, bitnames, buffer, bufferSize))
      {
         return -1;
      }
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrExt(const char* name, OSSIZE numbits,
   const OSOCTET* data, OSSIZE dataSize, const OSOCTET* extdata,
   const char* conn, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   OSSIZE dataSizeBits = dataSize * 8;
   char   numbuf[50];

   if (numbits > dataSizeBits)
   {
      if (buffer == 0 || bufferSize == 0) { return -1; }

      bufferIndex = OSCRTLSTRLEN (buffer);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxSizeToCharStr (numbits, numbuf, sizeof(numbuf), 0) < 0) {
         return -1;
      }
      if (rtxStrJoin (bufptr, rembytes,
                   name, conn, "numbits = ", numbuf, "\n") == 0) {
         return -1;
      }

      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxStrJoin (bufptr, rembytes, name, conn, "data = ", 0, 0) == 0) {
         return -1;
      }

      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtBitStringDumpToString(dataSizeBits, data, bufptr, rembytes) < 0)
      {
         return -1;
      }

      if (NULL != extdata)
      {
         if (rtPrintToString(" ", bufptr, rembytes) < 0)
         {
            return -1;
         }

         if (rtBitStringDumpToString(numbits - dataSizeBits, extdata, bufptr,
                                     rembytes) < 0)
         {
            return -1;
         }
      }

      if (rtPrintToString("\n", bufptr, rembytes) < 0 )
      {
         return -1;
      }
   }
   else
   {
      if (0 != rtPrintToStringBitStr(name, numbits, data, conn,
                                     buffer, bufferSize))
      {
         return -1;
      }
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrBraceText
(const char* name, OSSIZE numbits, const OSOCTET* data,
 char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr (numbits, numbuf, sizeof(numbuf), 0) < 0) {
      return -1;
   }
   if (rtxStrJoin (bufptr, rembytes, name, " = { ", numbuf, ", ", 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtBitStringDumpToString (numbits, data, bufptr, rembytes) < 0) {
      return -1;
   }
   if (rtPrintToString(" }\n", bufptr, rembytes) < 0 ) {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringBitStrBraceTextExt(const char* name,
   OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   OSSIZE dataSizeBits = dataSize * 8;
   char   numbuf[50];

   if (numbits > dataSizeBits)
   {
      bufferIndex = OSCRTLSTRLEN (buffer);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxSizeToCharStr (numbits, numbuf, sizeof(numbuf), 0) < 0) {
         return -1;
      }
      if (rtxStrJoin (bufptr, rembytes, name, " = { ", numbuf, ", ", 0) == 0) {
         return -1;
      }

      bufferIndex = OSCRTLSTRLEN (buffer);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtBitStringDumpToString(dataSizeBits, data, bufptr, rembytes) < 0)
      {
         return -1;
      }

      if (NULL != extdata)
      {
         if (rtPrintToString(" ", bufptr, rembytes) < 0)
         {
            return -1;
         }

         if (rtBitStringDumpToString(numbits - dataSizeBits, extdata, bufptr,
                                     rembytes) < 0)
         {
            return -1;
         }
      }

      if (rtPrintToString(" }\n", bufptr, rembytes) < 0 )
      {
         return -1;
      }
   }
   else
   {
      if (0 != rtPrintToStringBitStrBraceText(name, numbits, data,
                                              buffer, bufferSize))
      {
         return -1;
      }
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringOctStr
(const char* name, OSSIZE numocts, const OSOCTET* data, const char* conn,
 char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr (numocts, numbuf, sizeof(numbuf), 0) < 0) {
      return -1;
   }
   if (rtxStrJoin (bufptr, rembytes,
                   name, conn, "numocts = ", numbuf, "\n") == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, conn, "data = ", 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxHexDumpToString
      (data, numocts, buffer, bufferIndex, bufferSize) < 0) {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringCharStr
(const char* name, const char* cstring, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = '",
                     cstring == 0 ? "(null)" : cstring, "'\n", 0) == 0)
   {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToStringUTF8Str
(const char *name, const OSUTF8CHAR *ustring, char *buffer, OSSIZE bufferSize)
{
   return rtPrintToStringCharStr
      (name, (const char *)ustring, buffer, bufferSize);
}

EXTRTMETHOD int rtPrintToString16BitCharStr (const char* name,
   const Asn116BitCharString* bstring, const char* conn,
   char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr (bstring->nchars, numbuf, sizeof(numbuf), 0) < 0) {
      return -1;
   }
   if (rtxStrJoin (bufptr, rembytes, name, conn,
                   "nchars = ", numbuf, "\n") == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, conn,
                   "data =\n", 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);

   if( rtxHexDumpToStringEx ((OSOCTET*)bstring->data,
      bstring->nchars * 2, buffer, bufferIndex, bufferSize, 2) < 0) {
      return -1;
   }

   return 0;
}

EXTRTMETHOD int rtPrintToString32BitCharStr (const char* name,
   const Asn132BitCharString* bstring, const char* conn,
   char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[50];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxSizeToCharStr (bstring->nchars, numbuf, sizeof(numbuf), 0) < 0) {
      return -1;
   }
   if (rtxStrJoin (bufptr, rembytes, name, conn,
                   "nchars = ", numbuf, "\n") == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, conn,
                   "data =\n", 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);

   if( rtxHexDumpToStringEx ((OSOCTET*)bstring->data,
      bstring->nchars * sizeof (OS32BITCHAR), buffer,
      bufferIndex, bufferSize, 4) < 0 ) {
      return -1;
   }

   return 0;
}

#ifndef _NO_ASN1REAL
EXTRTMETHOD int rtPrintToStringReal
(const char* name, OSREAL value, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;
   char   numbuf[100];

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   os_snprintf (numbuf, sizeof(numbuf), " = %g\n", value);

   rtxStrJoin (bufptr, rembytes, name, " = ", numbuf, "\n", 0);

   return 0;
}
#endif

EXTRTMETHOD int rtPrintToStringOID
(const char* name, const ASN1OBJID* pOID, char* buffer, OSSIZE bufferSize)
{
   return rtPrintToStringOID2
      (name, pOID->numids, pOID->subid, buffer, bufferSize);
}

EXTRTMETHOD int rtPrintToStringOIDValue
(const ASN1OBJID* pOID, char* buffer, OSSIZE bufferSize)
{
   return rtPrintToStringOIDValue2
      (pOID->numids, pOID->subid, buffer, bufferSize);
}

EXTRTMETHOD int rtPrintToStringOID2
(const char* name, OSSIZE numids, const OSUINT32* subidArray,
 char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = ", 0, 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   return rtPrintToStringOIDValue2 (numids, subidArray, bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringOIDValue2
(OSSIZE numids, const OSUINT32* subidArray, char* buffer, OSSIZE bufferSize)
{
   OSSIZE ui, bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtPrintToString ("{ ", bufptr, rembytes) < 0 ) {
      return -1;
   }

   for (ui = 0; ui < numids; ui++) {
      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxUIntToCharStr (subidArray[ui], bufptr, rembytes, 0) < 0) {
         return -1;
      }

      if (rtPrintToString (" ", bufptr, rembytes) < 0) {
         return -1;
      }
   }

   return rtPrintToString ("}\n", bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringOID64
(const char* name, const ASN1OID64* pOID, char* buffer, OSSIZE bufferSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = ", 0, 0, 0) == 0) {
      return -1;
   }

   bufferIndex += OSCRTLSTRLEN (bufptr);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   return rtPrintToStringOID64Value (pOID, bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringOID64Value
(const ASN1OID64* pOID, char* buffer, OSSIZE bufferSize)
{
   OSUINT32 ui;
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufferSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

   if (rtPrintToString ("{ ", bufptr, rembytes) < 0 ) {
      return -1;
   }

   for (ui = 0; ui < pOID->numids; ui++) {
      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufferSize) ? bufferSize - bufferIndex : 0;

      if (rtxUInt64ToCharStr (pOID->subid[ui], bufptr, rembytes, 0) < 0) {
         return -1;
      }

      if (rtPrintToString (" ", bufptr, rembytes) < 0) {
         return -1;
      }
   }

   return rtPrintToString ("}\n", bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringOpenType
(const char* name, OSSIZE numocts, const OSOCTET* data, const char* conn,
 char* buffer, OSSIZE bufferSize)
{
   return rtPrintToStringOctStr (name, numocts, data, conn, buffer, bufferSize);
}

EXTRTMETHOD int rtPrintToStringOpenTypeExt
(const char* name, const OSRTDList* pElemList, char* buffer, OSSIZE bufferSize)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            if( rtPrintToStringOpenType (name, pOpenType->numocts,
               pOpenType->data, ".", buffer, bufferSize) < 0) {
               return -1;
            }
         }
         pnode = pnode->next;
      }
   }
   return 0;
}

EXTRTMETHOD int rtPrintToString
(const char* namebuf, char* buffer, OSSIZE bufSize)
{
   return (0 == rtxStrcat (buffer, bufSize, namebuf)) ? -1 : 0;
}

EXTRTMETHOD int rtPrintToStringUnicodeCharStr
(const char* name, const Asn116BitCharString* bstring,
 char* buffer, OSSIZE bufSize)
{
   OSUINT32 i;
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = '", 0, 0, 0) == 0) {
      return -1;
   }

   for (i = 0; i < bstring->nchars; i++) {
      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

      if (OS_ISPRINT (bstring->data[i])){
         char ch = (char) bstring->data[i];
         if (0 == rtxStrncat (bufptr, rembytes, &ch, 1)) {
            return -1;
         }
      }
      else {
         OSOCTET msb = (OSOCTET)(bstring->data[i]/256);
         OSOCTET lsb = (OSOCTET)(bstring->data[i]%256);
         if (0 == rtxStrncat (bufptr, rembytes, "0x", 2)) {
            return -1;
         }
         bufferIndex += 2;
         bufptr = buffer + bufferIndex;
         rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

         if (rtxByteToHexChar (msb, bufptr, rembytes) < 0) {
            return -1;
         }
         bufferIndex += 2;
         bufptr = buffer + bufferIndex;
         rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

         if (rtxByteToHexChar (lsb, bufptr, rembytes) < 0) {
            return -1;
         }
      }
   }

   return rtPrintToString ("'\n", bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringUnivCharStr
(const char* name, const Asn132BitCharString* bstring,
 char* buffer, OSSIZE bufSize)
{
   OSUINT32 i;
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = '", 0, 0, 0) == 0) {
      return -1;
   }

   for (i = 0; i < bstring->nchars; i++) {
      bufferIndex += OSCRTLSTRLEN (bufptr);
      bufptr = buffer + bufferIndex;
      rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

      if (OS_ISPRINT (bstring->data[i])){
         char ch = (char) bstring->data[i];
         if (0 == rtxStrncat (bufptr, rembytes, &ch, 1)) {
            return -1;
         }
      }
      else {
         OSOCTET bdata[4];
         OSINT32 j;
         OSUINT32 tmpval = bstring->data[i];

         if (0 == rtxStrncat (bufptr, rembytes, "0x", 2)) {
            return -1;
         }
         bufferIndex += 2;
         bufptr = buffer + bufferIndex;
         rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

         for (j = 3; j >= 0; j--) {
            bdata[j] = tmpval % 256;
            tmpval /= 256;
         }
         for (j = 0; j < 4; j++) {
            if (rtxByteToHexChar (bdata[j], bufptr, rembytes) < 0) {
               return -1;
            }
            bufferIndex += 2;
            bufptr = buffer + bufferIndex;
            rembytes = (bufferIndex < bufSize) ?
               bufSize - bufferIndex : 0;
         }
      }
   }

   return rtPrintToString ("'\n", bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringHexStr
(const char* name, OSSIZE numocts, const OSOCTET* data,
 char* buffer, OSSIZE bufSize)
{
   OSSIZE bufferIndex, rembytes;
   char*  bufptr;

   if (buffer == 0 || bufSize == 0) { return -1; }

   bufferIndex = OSCRTLSTRLEN (buffer);
   bufptr = buffer + bufferIndex;
   rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

   if (rtxStrJoin (bufptr, rembytes, name, " = ", 0, 0, 0) == 0) {
      return -1;
   }

   if (numocts <= 32) {
      OSSIZE i;
      if (rtPrintToString ("0x", bufptr, rembytes) < 0)
         return -1;

      for (i = 0; i < numocts; i++) {
         bufferIndex += OSCRTLSTRLEN (bufptr);
         bufptr = buffer + bufferIndex;
         rembytes = (bufferIndex < bufSize) ? bufSize - bufferIndex : 0;

         if (rtxByteToHexChar (data[i], bufptr, rembytes) < 0) {
            return -1;
         }
      }

      return rtPrintToString ("\n", bufptr, rembytes);
   }
   else {
      if (rtPrintToString ("\n", bufptr, rembytes) < 0)
         return -1;

      bufferIndex++;

      return rtxHexDumpToString
         (data, numocts, bufptr, bufferIndex, rembytes);
   }
}

#ifndef __SYMBIAN32__

EXTRTMETHOD int rtPrintToStringOpenTypeExtBraceText
(const char* name, const OSRTDList* pElemList, char* buffer, OSSIZE bufSize)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            if (rtPrintToStringIndent (buffer, bufSize) < 0)
               return -1;
	    if (rtPrintToStringHexStr (name, pOpenType->numocts,
                                       pOpenType->data, buffer, bufSize) < 0)
               return -1;

         }
         pnode = pnode->next;
      }
   }
   return 0;
}
#endif

#ifndef __SYMBIAN32__

/* Indentation for brace text printing */

static OSUINT32 g_indent = 0;

EXTRTMETHOD int rtPrintToStringIndent (char* buffer, OSSIZE bufSize)
{
   OSUINT32 i;
   for (i = 0; i < g_indent; i++){
      if (rtPrintToString (" ", buffer, bufSize) < 0)
         return -1;
   }
   return 0;

}

EXTRTMETHOD void rtPrintToStringResetIndent (void)
{
   g_indent = 0;
}

EXTRTMETHOD void rtPrintToStringIncrIndent (void)
{
   g_indent += OSRTINDENTSPACES;
}

EXTRTMETHOD void rtPrintToStringDecrIndent (void)
{
   if (g_indent > 0)
      g_indent -= OSRTINDENTSPACES;
}

EXTRTMETHOD int rtPrintToStringCloseBrace (char* buffer, OSSIZE bufSize)
{
   rtPrintToStringDecrIndent ();
   if(rtPrintToStringIndent (buffer, bufSize) < 0)
      return -1;

   if(rtPrintToString ("}\n", buffer, bufSize) < 0)
      return -1;

   return 0;
}

EXTRTMETHOD int rtPrintToStringOpenBrace
(const char* name, char* buffer, OSSIZE bufSize)
{
   if(rtPrintToStringIndent (buffer, bufSize) < 0)
      return -1;

   if(rtPrintToString(name, buffer, bufSize) < 0)
      return -1;

   if(rtPrintToString (" {\n", buffer, bufSize) < 0)
      return -1;

   rtPrintToStringIncrIndent ();
   return 0;
}
#endif
