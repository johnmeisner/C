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

/* Run-time print to string utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/rtPrintToString.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxPrint.h"

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

   return rtPrintToStringOIDValue (pOID, bufptr, rembytes);
}

EXTRTMETHOD int rtPrintToStringOIDValue
(const ASN1OBJID* pOID, char* buffer, OSSIZE bufferSize)
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

      if (rtxUIntToCharStr (pOID->subid[ui], bufptr, rembytes, 0) < 0) {
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
