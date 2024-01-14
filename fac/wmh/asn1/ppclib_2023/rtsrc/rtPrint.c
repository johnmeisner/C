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

/* Run-time print utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/rtPrint.h"
#include "rtxsrc/rtxCtype.h"

#ifndef BITSTR_BYTES_IN_LINE
#define BITSTR_BYTES_IN_LINE 16
#endif

static void rtBitStringDump (OSSIZE numbits, const OSOCTET* data)
{
   OSSIZE i, numocts = numbits / 8;
   char buff[9];

   if (numbits == 0 || 0 == data) return;

   if (numocts > 8)
      printf ("\n");

   for (i = 0; i < numocts; i++) {
      if ((i != 0) && (i % BITSTR_BYTES_IN_LINE == 0)) {
         printf ("\n");
      }
      else if (i % BITSTR_BYTES_IN_LINE != 0) {
         printf (" ");
      }

      printf ("0x%02X", data[i]);
   }

   if (i * 8 != numbits) {
      OSOCTET tm = data[i];
      OSSIZE nmBits = numbits % 8;
      OSSIZE j;

      for (j = 0; j < nmBits; j++, tm<<=1)
         buff[j] = ((tm >> 7) & 1) + '0';
      for (; j < 8; j++)
         buff[j] = 'x';

      if ((i % BITSTR_BYTES_IN_LINE) == (BITSTR_BYTES_IN_LINE - 1))
         printf ("\n%.8s", buff);
      else if (i > 0)
         printf (" %.8s", buff);
      else
         printf ("%.8s", buff);
   }
}

static void rtBitStringDumpBin(OSSIZE numbits, const OSOCTET *data)
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
         printf("1");
      }
      else
      {
         printf("0");
      }
      pos++;
   }
}

static void rtBitStringDumpNamed(OSSIZE numbits, const OSOCTET* data,
   OSSIZE namedbits, const OSUINT32 positions[], const char* bitnames[])
{
   OSSIZE stringSize = 0;
   OSSIZE index = 0;
   OSSIZE remaining;
   OSUINT32 i, j;
   char* outputString;
   OSSIZE* nameLengths;
   int mask = 128, pos = 0;

   /*
   The output string will include the names of all named bits, so calculate
   that size first.  Also allocate memory for an array to hold the lenghs
   of the names so we minimize how many times we call strlen().
   */
   nameLengths = (OSSIZE*)OSCRTLMALLOC (sizeof (OSSIZE) * namedbits);
   if (NULL == nameLengths) return;
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
      return;
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
   printf ("%s", outputString);
   OSCRTLFREE (outputString);
   OSCRTLFREE (nameLengths);
}

EXTRTMETHOD void rtPrintBitStr (const char* name, OSSIZE numbits,
                                const OSOCTET* data, const char* conn)
{
   printf ("%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   printf ("%s%sdata = ", name, conn);
   rtBitStringDump (numbits, data);
   printf ("\n");
}

EXTRTMETHOD void rtPrintBitStrBin(const char *name, OSSIZE numbits,
                                  const OSOCTET *data, const char *conn)
{
   rtxPrintIndent();
   printf("%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintIndent();
   printf("%s%sdata = ", name, conn);
   rtBitStringDumpBin(numbits, data);
   printf("\n");
}

EXTRTMETHOD void rtPrintBitStrNamed(const char *name, OSSIZE numbits,
   const OSOCTET *data, const char *conn, OSSIZE namedbits,
   const OSUINT32 positions[], const char* bitnames[])
{
   rtxPrintIndent();
   printf("%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintIndent();
   printf("%s%sdata = ", name, conn);
   rtBitStringDumpNamed(numbits, data, namedbits, positions, bitnames);
   printf("\n");
}

EXTRTMETHOD void rtPrintBitStrNamedExt(const char *name, OSSIZE numbits,
   const OSOCTET *data, OSSIZE dataSize, const OSOCTET* extdata,
   const char *conn, OSSIZE namedbits, const OSUINT32 positions[],
   const char* bitnames[])
{
   OSSIZE dataSizeBits = dataSize * 8;
   rtxPrintIndent();
   printf("%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintIndent();
   printf("%s%sdata = ", name, conn);
   if (numbits > dataSizeBits) {
      rtBitStringDumpNamed(dataSizeBits, data, namedbits, positions, bitnames);
      if (0 != extdata) {
         printf(" ");
         /* Just a regular dump without bit names of the extension data */
         rtBitStringDump (numbits - dataSizeBits, extdata);
      }
   }
   else {
      rtBitStringDumpNamed(numbits, data, namedbits, positions, bitnames);
   }
   printf("\n");
}


EXTRTMETHOD void rtPrintBitStrExt (const char* name, OSSIZE numbits,
                                   const OSOCTET* data, OSSIZE dataSize,
                                   const OSOCTET* extdata, const char* conn)
{
   OSSIZE dataSizeBits = dataSize * 8;
   rtxPrintIndent();
   printf ("%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintIndent();
   printf ("%s%sdata = ", name, conn);
   if (numbits > dataSizeBits) {
      rtBitStringDump (dataSizeBits, data);
      if (0 != extdata) {
         printf (" ");
         rtBitStringDump (numbits - dataSizeBits, extdata);
      }
   }
   else {
      rtBitStringDump (numbits, data);
   }
   printf ("\n");
}

EXTRTMETHOD void rtPrintBitStrBraceText
(const char* name, OSSIZE numbits, const OSOCTET* data)
{
   printf ("%s = { %lu, ", name, (unsigned long)numbits);
   rtBitStringDump (numbits, data);
   printf (" }\n");
}

EXTRTMETHOD void rtPrintBitStrBraceTextExt
(const char* name, OSSIZE numbits, const OSOCTET* data,
 OSSIZE dataSize, const OSOCTET* extdata)
{
   OSSIZE dataSizeBits = dataSize * 8;
   printf ("%s = { %lu, ", name, (unsigned long)numbits);
   if (numbits > dataSizeBits) {
      rtBitStringDump (dataSizeBits, data);
      if (0 != extdata) {
         printf (" ");
         rtBitStringDump (numbits - dataSizeBits, extdata);
      }
   }
   else {
      rtBitStringDump (numbits, data);
   }
   printf (" }\n");
}

EXTRTMETHOD void rtPrintOctStr (const char* name, OSSIZE numocts,
                                const OSOCTET* data, const char* conn)
{
   printf ("%s%snumocts = " OSSIZEFMT "\n", name, conn, numocts);
   printf ("%s%sdata = \n", name, conn);
   rtxHexDump (data, numocts);
}

EXTRTMETHOD void rtPrint16BitCharStr
(const char* name, const Asn116BitCharString* bstring)
{
   rtxPrintUnicodeCharStr64 (name, bstring->data, bstring->nchars);
}

EXTRTMETHOD void rtPrint32BitCharStr
(const char* name, const Asn132BitCharString* bstring, const char* conn)
{
   printf ("%s%snchars = " OSSIZEFMT "\n", name, conn, bstring->nchars);
   printf ("%s%sdata = \n", name, conn);
   rtxHexDumpEx ((OSOCTET*)bstring->data,
                 bstring->nchars * sizeof (OSUINT32), 4);
}

EXTRTMETHOD void rtPrintOID (const char* name, const ASN1OBJID* pOID)
{
   rtPrintOID2(name, pOID->numids, pOID->subid);
}

EXTRTMETHOD void rtPrintOIDValue (const ASN1OBJID* pOID)
{
   rtPrintOIDValue2(pOID->numids, pOID->subid);
}

EXTRTMETHOD void rtPrintOID2
(const char* name, OSSIZE numids, const OSUINT32* subidArray)
{
   printf ("%s = ", name);
   rtPrintOIDValue2 (numids, subidArray);
}

EXTRTMETHOD void rtPrintOIDValue2 (OSSIZE numids, const OSUINT32* subidArray)
{
   OSSIZE ui;
   printf ("{ ");
   for (ui = 0; ui < numids; ui++) {
      printf ("%d ", subidArray[ui]);
   }
   printf ("}\n");
}

/* OID with subid of 64 bit */
EXTRTMETHOD void rtPrintOID64 (const char* name, const ASN1OID64* pOID)
{
   printf ("%s = ", name);
   rtPrintOID64Value (pOID);
}

EXTRTMETHOD void rtPrintOID64Value (const ASN1OID64* pOID)
{
   OSINT64 ui;
   printf ("{ ");
   for (ui = 0; ui < pOID->numids; ui++) {
      printf (OSINT64FMT" " , pOID->subid[(unsigned int)ui]);
   }
   printf ("}\n");
}

EXTRTMETHOD void rtPrintOpenType (const char* name, OSSIZE numocts,
                                  const OSOCTET* data, const char*  conn)
{
   printf ("%s%snumocts = %lu\n", name, conn, (unsigned long)numocts);
   printf ("%s%sdata = \n", name, conn);
   rtxHexDump (data, numocts);
}

EXTRTMETHOD void rtPrintOpenTypeExt
(const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            rtPrintOpenType (name, pOpenType->numocts, pOpenType->data, ".");
         }
         pnode = pnode->next;
      }
   }
}

EXTRTMETHOD void rtPrintUnivCharStr
(const char* name, const Asn132BitCharString* bstring)
{
   OSUINT32 i;
   printf ("%s = '", name);
   for (i = 0; i < bstring->nchars; i++) {
      if (OS_ISPRINT (bstring->data[i]))
         putc (bstring->data[i], stdout);
      else
         printf ("0x%08x", bstring->data[i]);
   }
   printf ("'\n");
}

#ifndef __SYMBIAN32__
EXTRTMETHOD void rtPrintOpenTypeExtBraceText
(const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            rtxPrintIndent ();
            rtxPrintHexStr (name, pOpenType->numocts, pOpenType->data);
         }
         pnode = pnode->next;
      }
   }
}
#endif

