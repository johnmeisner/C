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
#include "rtsrc/rtPrintToStream.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDateTime.h"

#ifndef BITSTR_BYTES_IN_LINE
#define BITSTR_BYTES_IN_LINE 16
#endif

static void rtBitStringDump (OSCTXT *pctxt, OSSIZE numbits, const OSOCTET* data)
{
   OSSIZE i, numocts = numbits / 8;
   char buff[9];

   if (numbits == 0 || 0 == data) return;

   if (numocts > 8)
      rtxPrintToStream (pctxt, "\n");

   for (i = 0; i < numocts; i++) {
      if ((i != 0) && (i % BITSTR_BYTES_IN_LINE == 0)) {
         rtxPrintToStream (pctxt, "\n");
      }
      else if (i % BITSTR_BYTES_IN_LINE != 0) {
         rtxPrintToStream (pctxt, " ");
      }

      rtxPrintToStream (pctxt, "0x%02X", data[i]);
   }

   if (i * 8 != numbits) {
      OSOCTET tm = data[i];
      OSSIZE nmBits = numbits % 8;
      OSSIZE j;

      for (j = 0; j < nmBits; j++, tm <<= 1)
         buff[j] = ((tm >> 7) & 1) + '0';
      for (; j < 8; j++)
         buff[j] = 'x';

      if ((i % BITSTR_BYTES_IN_LINE) == (BITSTR_BYTES_IN_LINE - 1))
         rtxPrintToStream (pctxt, "\n%.8s", buff);
      else if (i > 0)
         rtxPrintToStream (pctxt, " %.8s", buff);
      else
         rtxPrintToStream (pctxt, "%.8s", buff);
   }
}

static int rtBitStringDumpToList (OSCTXT *pctxt, OSSIZE numbits,
                                  const OSOCTET* data, OSRTSList* pList)
{
   OSSIZE i, numocts = numbits / 8;
   char* pbuf, *ps;

   if (0 == numbits) return 0;

   pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
   if (0 == pbuf) return RTERR_NOMEM;

   for (i = 0; i < numocts; i++) {
      if ((i != 0) && (i % BITSTR_BYTES_IN_LINE == 0)) {
         rtxSListAppend (pList, (void*)pbuf);
         pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
         if (0 == pbuf) return RTERR_NOMEM;
      }

      if (i % BITSTR_BYTES_IN_LINE != 0) {
         *ps++ = ' ';
      }
      rtxByteToHexChar (data[i], ps, 3);
      ps += 2;
   }

   *ps = '\0';

   if (i * 8 != numbits) {
      OSOCTET tm = data[i];
      OSSIZE nmBits = numbits % 8;
      OSSIZE j;
      char buff[9];

      for (j = 0; j < nmBits; j++, tm<<=1)
         buff[j] = ((tm >> 7) & 1) + '0';
      for (; j < 8; j++)
         buff[j] = 'x';

      buff[sizeof(buff)-1] = '\0';

      if ((i % BITSTR_BYTES_IN_LINE) == (BITSTR_BYTES_IN_LINE - 1)) {
         rtxSListAppend (pList, (void*)pbuf);
         pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
         if (0 == pbuf) return RTERR_NOMEM;
      }
      else if (i > 0) {
         *ps++ = ' ';
      }
      *ps = '\0';
      rtxStrcat (pbuf, 81, buff);
   }

   rtxSListAppend (pList, (void*)pbuf);

   return 0;
}

static int rtBitStringDumpBin(OSCTXT *pctxt, OSSIZE numbits,
                              const OSOCTET *data)
{
   int mask = 128, pos = 0, ret = 0;
   OSUINT32 i;

   for (i = 0; i < numbits; i++)
   {
      if (pos == 8)
      {
         pos = 0;
      }

      if (data[i/8] & (mask >> pos))
      {
         ret = rtxPrintToStream(pctxt, "1");
         if (ret != 0)
         {
            return ret;
         }
      }
      else
      {
         ret = rtxPrintToStream(pctxt, "0");
         if (ret != 0)
         {
            return ret;
         }
      }
      pos++;
   }

   ret = rtxPrintToStream(pctxt, "\n");
   return ret;
}

static void rtBitStringDumpNamed(OSCTXT *pctxt, OSSIZE numbits,
   const OSOCTET* data, OSSIZE namedbits, const OSUINT32 positions[],
   const char* bitnames[])
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
   rtxPrintToStream (pctxt, "%s", outputString);
   OSCRTLFREE (outputString);
   OSCRTLFREE (nameLengths);
}

EXTRTMETHOD int rtPrintToStreamBitStr
(OSCTXT *pctxt, const char* name, OSSIZE numbits,
 const OSOCTET* data, const char* conn)
{
   OSRTSList bitStrList;
   int ret;

   rtxSListInitEx (pctxt, &bitStrList);

   ret = rtxPrintToStream (pctxt, "%s%snumbits = %u\n", name, conn, numbits);
   if (ret != 0) return ret;

   ret = rtBitStringDumpToList (pctxt, numbits, data, &bitStrList);
   if (ret != 0) return ret;

   if (numbits >= 64) {
      OSRTSListNode* pnode = bitStrList.head;
      ret = rtxPrintToStream (pctxt, "%s%sdata =\n", name, conn);
      if (ret != 0) return ret;

      while (0 != pnode) {
         ret = rtxPrintToStream (pctxt, "%s\n", (char*)pnode->data);
         if (ret != 0) break;
         else pnode = pnode->next;
      }
   }
   else if (numbits > 0) {
      ret = rtxPrintToStream (pctxt, "%s%sdata = %s\n",
                              name, conn, (char*)bitStrList.head->data);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s%sdata = NULL\n", name, conn);
   }

   rtxSListFreeAll (&bitStrList);

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrBin(OSCTXT *pctxt, const char *name,
                         OSSIZE numbits, const OSOCTET *data, const char *conn)
{
   int ret;

   ret = rtxPrintToStream(pctxt, "%s%snumbits = %u\n", name, conn, numbits);
   if (ret != 0)
   {
      return ret;
   }

   ret = rtxPrintToStream(pctxt, "%s%sdata = ", name, conn);
   if (ret != 0)
   {
      return ret;
   }

   ret = rtBitStringDumpBin(pctxt, numbits, data);
   if (ret != 0)
   {
      return ret;
   }

   return ret;
}

EXTRTMETHOD void rtPrintToStreamBitStrNamed(OSCTXT *pctxt, const char* name,
   OSSIZE numbits, const OSOCTET* data, const char* conn, OSSIZE namedbits,
   const OSUINT32 positions[], const char* bitnames[])
{
   rtxPrintToStreamIndent(pctxt);
   rtxPrintToStream(pctxt, "%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintToStreamIndent(pctxt);
   rtxPrintToStream(pctxt, "%s%sdata = ", name, conn);
   rtBitStringDumpNamed(pctxt, numbits, data, namedbits, positions, bitnames);
   rtxPrintToStream(pctxt, "\n");
}

EXTRTMETHOD void rtPrintToStreamBitStrNamedExt(OSCTXT *pctxt, const char* name,
   OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata, const char* conn, OSSIZE namedbits,
   const OSUINT32 positions[], const char* bitnames[])
{
   OSSIZE dataSizeBits = dataSize * 8;
   rtxPrintToStreamIndent(pctxt);
   rtxPrintToStream(pctxt, "%s%snumbits = %lu\n", name, conn, (unsigned long)numbits);
   rtxPrintToStreamIndent(pctxt);
   rtxPrintToStream(pctxt, "%s%sdata = ", name, conn);
   if (numbits > dataSizeBits) {
      rtBitStringDumpNamed(pctxt, dataSizeBits, data, namedbits, positions, bitnames);
      if (0 != extdata) {
         rtxPrintToStream(pctxt, " ");
         /* Just a regular dump without bit names of the extension data */
         rtBitStringDump (pctxt, numbits - dataSizeBits, extdata);
      }
   }
   else {
      rtBitStringDumpNamed(pctxt, numbits, data, namedbits, positions, bitnames);
   }
   rtxPrintToStream(pctxt, "\n");
}

EXTRTMETHOD int rtPrintToStreamBitStrExt(OSCTXT *pctxt, const char* name,
   OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata, const char* conn)
{
   OSRTSList bitStrList;
   OSSIZE dataSizeBits = dataSize * 8;
   int ret;

   rtxSListInitEx (pctxt, &bitStrList);

   ret = rtxPrintToStream (pctxt, "%s%snumbits = %u\n", name, conn, numbits);
   if (ret != 0) return ret;

   ret = rtBitStringDumpToList (pctxt,
              (numbits > dataSizeBits) ? dataSizeBits : numbits, data,
              &bitStrList);
   if (ret != 0) return ret;

   if (numbits >= 64) {
      OSRTSListNode* pnode = bitStrList.head;
      ret = rtxPrintToStream (pctxt, "%s%sdata =\n", name, conn);
      if (ret != 0) return ret;

      while (0 != pnode) {
         ret = rtxPrintToStream (pctxt, "%s\n", (char*)pnode->data);
         if (ret != 0) break;
         else pnode = pnode->next;
      }
   }
   else if (numbits > 0) {
      ret = rtxPrintToStream (pctxt, "%s%sdata = %s\n",
                              name, conn, (char*)bitStrList.head->data);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s%sdata = NULL\n", name, conn);
   }

   if (NULL != extdata)
   {
      ret = rtxPrintToStream (pctxt, "%s%sextdata = 0x%x\n",
                              name, conn, *extdata);
      if (0 != ret)
      {
         return ret;
      }
   }

   rtxSListFreeAll (&bitStrList);

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrBraceText
(OSCTXT *pctxt, const char* name, OSSIZE numbits, const OSOCTET* data)
{
   int ret = 0;
   if (numbits > 0) {
      OSRTSListNode* pnode;
      OSRTSList bitStrList;

      rtxSListInitEx (pctxt, &bitStrList);

      ret = rtBitStringDumpToList (pctxt, numbits, data, &bitStrList);
      if (ret != 0) return ret;

      if (numbits >= 64) {
         ret = rtxPrintToStream (pctxt, "%s = { %u,\n", name, numbits);
         if (ret != 0) return ret;

         pnode = bitStrList.head;
         while (0 != pnode) {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   %s\n", (char*)pnode->data);
            if (ret != 0) break;
            else pnode = pnode->next;
         }

         rtxPrintToStreamIndent (pctxt);
         ret = rtxPrintToStream (pctxt, "}\n");
      }
      else {
         ret = rtxPrintToStream (pctxt, "%s = { %u, %s }\n",
                                 name, numbits, (char*)bitStrList.head->data);
      }

      rtxSListFreeAll (&bitStrList);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s = { 0 }\n", name);
   }

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrBraceTextExt(OSCTXT *pctxt,
   const char* name, OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata)
{
   OSSIZE dataSizeBits = dataSize * 8;
   int ret;

   if (numbits > 0) {
      OSRTSListNode* pnode;
      OSRTSList bitStrList;

      rtxSListInitEx (pctxt, &bitStrList);

      ret = rtBitStringDumpToList (pctxt,
              (numbits > dataSizeBits) ? dataSizeBits : numbits,
              data, &bitStrList);
      if (ret != 0) return ret;

      if (numbits >= 64) {
         ret = rtxPrintToStream (pctxt, "%s = { %u,\n", name, numbits);
         if (ret != 0) return ret;

         pnode = bitStrList.head;
         while (0 != pnode) {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   %s\n", (char*)pnode->data);
            if (ret != 0) break;
            else pnode = pnode->next;
         }

         if (NULL != extdata)
         {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   0x%x\n", *extdata);
            if (ret != 0) return ret;
         }

         rtxPrintToStreamIndent (pctxt);
         ret = rtxPrintToStream (pctxt, "}\n");
      }
      else {
         if (NULL != extdata)
         {
            ret = rtxPrintToStream (pctxt, "%s = { %u, %s, 0x%x }\n",
                                    name, numbits,
                                    (char *)bitStrList.head->data,
                                    *extdata);
         }
         else
         {
            ret = rtxPrintToStream (pctxt, "%s = { %u, %s }\n",
                                    name, numbits,
                                    (char *)bitStrList.head->data);
         }
      }

      rtxSListFreeAll (&bitStrList);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s = { 0 }\n", name);
   }

   return ret;
}
EXTRTMETHOD int rtPrintToStreamIpv4Addr(OSCTXT *pctxt, const char* name,
                               OSSIZE numocts, const OSOCTET* data)
{
   OSSIZE i;
   int stat = 0;

   stat = rtxPrintToStream(pctxt, "%s = ", name);
   if (stat != 0)
   {
      return stat;
   }

   for (i = 0; i < numocts; i++)
   {
      if (i != 0)
      {
         stat = rtxPrintToStream(pctxt, ".");
         if (stat != 0)
         {
            return stat;
         }
      }
      stat = rtxPrintToStream(pctxt, "%d", data[i]);
      if (stat != 0)
      {
         return stat;
      }
   }

   stat = rtxPrintToStream(pctxt, "\n");
   return stat;
}

EXTRTMETHOD int rtPrintToStreamIpv6Addr(OSCTXT *pctxt, const char* name,
                               OSSIZE numocts, const OSOCTET* data)
{
   OSSIZE i;
   int stat = 0;

   stat = rtxPrintToStream(pctxt, "%s = ", name);
   if (stat != 0)
   {
      return stat;
   }

   for (i = 0; i < numocts; i++)
   {
      if ((i != 0) && (i % 2 == 0))
      {
         stat = rtxPrintToStream(pctxt, ":");
         if (stat != 0)
         {
            return stat;
         }
      }
      stat = rtxPrintToStream(pctxt, "%x", data[i]);
      if (stat != 0)
      {
         return stat;
      }
   }

   stat = rtxPrintToStream(pctxt, "\n");
   return stat;
}

EXTRTMETHOD int rtPrintToStreamTBCDStr(OSCTXT *pctxt, const char* name,
                               OSSIZE numocts, const OSOCTET* data)
{
   OSUINT8 b;
   OSSIZE i, idx = 0;
   int stat = 0;

   stat = rtxPrintToStream(pctxt, "%s = ", name);
   if (stat != 0)
   {
      return stat;
   }

   for (i = 0; i < numocts * 2; i++)
   {
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
         stat = rtxPrintToStream(pctxt, "%c", (char)(b + '0'));
         if (stat != 0)
         {
            return stat;
         }
      }
      else if (b == 0xA)
      {
         stat = rtxPrintToStream(pctxt, "*");
         if (stat != 0)
         {
            return stat;
         }
      }
      else if (b == 0xB)
      {
         stat = rtxPrintToStream(pctxt, "#");
         if (stat != 0)
         {
            return stat;
         }
      }
      else if (b == 0xC)
      {
         stat = rtxPrintToStream(pctxt, "a");
         if (stat != 0)
         {
            return stat;
         }
      }
      else if (b == 0xD)
      {
         stat = rtxPrintToStream(pctxt, "b");
         if (stat != 0)
         {
            return stat;
         }
      }
      else if (b == 0xE)
      {
         stat = rtxPrintToStream(pctxt, "c");
         if (stat != 0)
         {
            return stat;
         }
      }
   }

   stat = rtxPrintToStream(pctxt, "\n");
   return stat;
}

EXTRTMETHOD int rtPrintToStreamDateTime(OSCTXT *pctxt, const char* name,
                               OSNumDateTime *pDateTime)
{
   OSUTF8CHAR dtbuf[50];
   int stat = 0;

   stat = rtxPrintToStream(pctxt, "%s = ", name);
   if (stat != 0)
   {
      return stat;
   }

   if (rtxDateTimeToString(pDateTime, dtbuf, 50) < 0)
   {
      return -1;
   }

   stat = rtxPrintToStream(pctxt, "%s", dtbuf);
   if (stat != 0)
   {
      return stat;
   }

   stat = rtxPrintToStream(pctxt, "\n");
   return stat;
}

EXTRTMETHOD int rtPrintToStreamText(OSCTXT *pctxt, const char* name,
                               OSSIZE numocts, const OSOCTET* data)
{
   OSSIZE i;
   int stat = 0;

   stat = rtxPrintToStream(pctxt, "%s = ", name);
   if (stat != 0)
   {
      return stat;
   }

   for (i = 0; i < numocts; i++)
   {
      if (OS_ISPRINTABLE(data[i]))
      {
         stat = rtxPrintToStream(pctxt, "%c", (char)data[i]);
         if (stat != 0)
         {
            return stat;
         }
      }
   }

   stat = rtxPrintToStream(pctxt, "\n");
   return stat;
}

EXTRTMETHOD int rtPrintToStreamOctStr
(OSCTXT *pctxt, const char* name, OSSIZE numocts,
 const OSOCTET* data, const char* conn)
{
   int ret = 0;
   ret = rtxPrintToStream(pctxt, "%s%snumocts = %u\n", name, conn, numocts);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStream (pctxt, data, numocts);
   return ret;
}


EXTRTMETHOD int rtPrintToStream16BitCharStr (OSCTXT *pctxt,
   const char* name, const Asn116BitCharString* bstring, const char* conn)
{
   int ret = 0;

   ret = rtxPrintToStream(pctxt, "%s%snchars = %u\n", name, conn,
                         bstring->nchars);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStreamEx (pctxt, (OSOCTET*)bstring->data,
                        bstring->nchars * 2, 2);
   return ret;
}


EXTRTMETHOD int rtPrintToStream32BitCharStr (OSCTXT *pctxt,
   const char* name, const Asn132BitCharString* bstring, const char* conn)
{
   int ret = 0;

   ret = rtxPrintToStream(pctxt,"%s%snchars = %u\n", name, conn,
                         bstring->nchars);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStreamEx (pctxt, (OSOCTET*)bstring->data,
                        bstring->nchars * sizeof (OS32BITCHAR), 4);
   return ret;
}


EXTRTMETHOD int rtPrintToStreamOIDValue (OSCTXT *pctxt, const ASN1OBJID* pOID)
{
   return rtPrintToStreamOIDValue2(pctxt, pOID->numids, pOID->subid);
}

EXTRTMETHOD int rtPrintToStreamOID
(OSCTXT *pctxt, const char* name, const ASN1OBJID* pOID)
{
   return rtPrintToStreamOID2(pctxt, name, pOID->numids, pOID->subid);
}

EXTRTMETHOD int rtPrintToStreamOIDValue2
(OSCTXT *pctxt, OSSIZE numids, const OSUINT32* subidArray)
{
   OSSIZE ui;
   int ret = rtxPrintToStream(pctxt, "{ ");
   if (0 == ret) {
      for (ui = 0; ui < numids; ui++) {
         ret = rtxPrintToStream(pctxt, "%d ", subidArray[ui]);
      }
   }
   if (0 == ret) {
      ret = rtxPrintToStream(pctxt, "}\n");
   }
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOID2
(OSCTXT *pctxt, const char* name, OSSIZE numids, const OSUINT32* subidArray)
{
   int ret = rtxPrintToStream(pctxt, "%s = ", name);
   if (0 == ret) {
      ret = rtPrintToStreamOIDValue2(pctxt, numids, subidArray);
   }
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOID64Value (OSCTXT *pctxt, const ASN1OID64* pOID)
{
   OSINT64 ui;
   rtxPrintToStream (pctxt, "{ ");
   for (ui = 0; ui < pOID->numids; ui++) {
      rtxPrintToStream (pctxt, OSINT64FMT" " ,
                       pOID->subid[(unsigned int)ui]);
   }
   rtxPrintToStream (pctxt, "}\n");
   return 0;
}

/* OID with subid of 64 bit */
EXTRTMETHOD int rtPrintToStreamOID64
(OSCTXT *pctxt, const char* name, const ASN1OID64* pOID)
{
   int ret = 0;
   ret = rtxPrintToStream(pctxt, "%s = ", name);
   if(ret != 0) return ret;
   ret = rtPrintToStreamOID64Value (pctxt, pOID);
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOpenType
(OSCTXT *pctxt,const char* name, OSSIZE numocts,
 const OSOCTET* data, const char* conn)
{
   int ret = 0;
   ret =  rtxPrintToStream(pctxt, "%s%snumocts = %u\n", name, conn, numocts);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStream (pctxt, data, numocts);
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOpenTypeExt
(OSCTXT *pctxt, const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            rtPrintToStreamOpenType (pctxt, name, pOpenType->numocts,
                                     pOpenType->data, ".");
         }
         pnode = pnode->next;
      }
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStreamUnicodeCharStr
   (OSCTXT *pctxt, const char* name, const Asn116BitCharString* bstring)
{
   OSUINT32 i;
   if(rtxPrintToStream (pctxt, "%s = '", name) < 0)
      return -1;

   for (i = 0; i < bstring->nchars; i++) {
      if (OS_ISPRINT (bstring->data[i])){
         if(rtxPrintToStream (pctxt, "%c", bstring->data[i]) < 0)
            return -1;
      }
      else{
         if(rtxPrintToStream (pctxt, "0x%04x", bstring->data[i]) < 0)
            return -1;
       }
   }
   if(rtxPrintToStream (pctxt, "'\n") < 0)
      return -1;

   return 0;
}

EXTRTMETHOD int rtPrintToStreamUnivCharStr
   (OSCTXT *pctxt, const char* name, const Asn132BitCharString* bstring)
{
   OSUINT32 i;

   if(rtxPrintToStream (pctxt, "%s = '", name) < 0)
      return -1;

   for (i = 0; i < bstring->nchars; i++) {
      if (OS_ISPRINT (bstring->data[i])){
         if(rtxPrintToStream (pctxt, "%c", bstring->data[i]) < 0)
            return -1;
      }
      else{
         if(rtxPrintToStream (pctxt, "0x%08x", bstring->data[i]) < 0)
            return -1;
      }
   }
   if(rtxPrintToStream (pctxt, "'\n") < 0)
      return -1;

   return 0;
}

EXTRTMETHOD int rtPrintToStreamOpenTypeExtBraceText
   (OSCTXT *pctxt, const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;

            rtPrintToStreamIndent (pctxt);

            rtPrintToStreamHexStr
               (pctxt, name, pOpenType->numocts, pOpenType->data);
         }
         pnode = pnode->next;
      }
   }
   return 0;
}




