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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxSList.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxCharStr.h"

typedef struct {
   OSUINT8 lb, lbm;
   char fmtBitBuffer[40], fmtHexBuffer[10], fmtAscBuffer[10];
   int  fmtBitCharIdx, fmtHexCharIdx, fmtAscCharIdx;
} BinDumpBuffer;

static const char* const htmlHeader =
"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
"<html lang=\"en-US\">\n"
"<head>\n"
"<META http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
"<title>EXI 1.0 Encoding</title>\n"
"<style type=\"text/css\">\n"
"code           { font-family: monospace; }\n"
"\n"
"div.exampleInner pre { margin-left: 1em;\n"
"                       margin-top: 0em; margin-bottom: 0em}\n"
"div.exampleOuter {border: 4px double gray;\n"
"                  margin: 0em; padding: 0em}\n"
"div.exampleInner { background-color: #d5dee3;\n"
"                   border-top-width: 4px;\n"
"                   border-top-style: double;\n"
"                   border-top-color: #d3d3d3;\n"
"                   border-bottom-width: 4px;\n"
"                   border-bottom-style: double;\n"
"                   border-bottom-color: #d3d3d3;\n"
"                   padding: 4px; margin: 0em }\n"
"div.exampleWrapper { margin: 4px }\n"
"div.exampleHeader { font-weight: bold;\n"
"                    margin: 4px}\n"
"\n"
"tr.silver td { color: silver }\n"
"\n"
"tr.bold td { font-weight: bold }\n"
"\n"
"td.xml { \n"
"  background-color: gray; color: white; font-weight: bold; font-size: 100%\n"
"}\n"
"</style>\n"
"</head>\n"
;

static void fmtAndPrintBit (int value, BinDumpBuffer* pbuf, OSRTSList* plist)
{
   if (value > 0) {
      pbuf->fmtBitBuffer[pbuf->fmtBitCharIdx++] = '1';
      pbuf->lb |= pbuf->lbm;
   }
   else if (value < 0) {
      pbuf->fmtBitBuffer[pbuf->fmtBitCharIdx++] = 'x';
   }
   else
      pbuf->fmtBitBuffer[pbuf->fmtBitCharIdx++] = '0';

   pbuf->lbm >>= 1;

   if (pbuf->lbm == 0) {
      if (pbuf->fmtAscCharIdx < 4)
         pbuf->fmtBitBuffer[pbuf->fmtBitCharIdx++] = ' ';

      os_snprintf (&pbuf->fmtHexBuffer[pbuf->fmtHexCharIdx], 10, "%02x", pbuf->lb);

      os_snprintf (&pbuf->fmtAscBuffer[pbuf->fmtAscCharIdx], 10, "%c",
               OS_ISPRINT(pbuf->lb) ? pbuf->lb : '.');

      pbuf->fmtHexCharIdx += 2; pbuf->fmtAscCharIdx++; pbuf->lb = 0;

      /* If buffer is full, print to stdout */

      if (pbuf->fmtAscCharIdx >= 4) {
         BinDumpBuffer* plistbuf =
            (BinDumpBuffer*) malloc (sizeof(BinDumpBuffer));

         if (0 != plistbuf) {
            OSCRTLSAFEMEMCPY (plistbuf, sizeof(BinDumpBuffer), pbuf,
                  sizeof(BinDumpBuffer));
            rtxSListAppend (plist, plistbuf);
            OSCRTLMEMSET (pbuf, 0, sizeof(BinDumpBuffer));
         }
      }

      pbuf->lbm = 0x80;
   }
}

static size_t getMaskAndIndex (size_t bitOffset, OSUINT8* pMask)
{
   int relBitOffset = 7 - (int)(bitOffset % 8);
   *pMask = (OSUINT8)(OSUINTCONST(1) << relBitOffset);
   return (bitOffset / 8);  /* byte index */
}

static void getBitTextList
(const OSOCTET* dataBuffer, size_t currBitOffset, size_t numbits,
 size_t nextBitOffset, BinDumpBuffer* pbuf, OSRTSList* plist)
{
   size_t  byteIndex, i, j, bitidx;
   BinDumpBuffer* plistbuf;
   OSUINT8 bitMask;
   OSBOOL  bitValue;

   /* Set start of dump buffer to hyphens */
   for (i = 0, j = 0; i < (size_t)pbuf->fmtBitCharIdx; i++) {
      if (j == 8) {
         pbuf->fmtBitBuffer[i] = ' ';
         j = 0;
      }
      else {
         pbuf->fmtBitBuffer[i] = '-';
         j++;
      }
   }
   for (i = 0; i < (size_t)pbuf->fmtHexCharIdx; i++) {
      pbuf->fmtHexBuffer[i] = '-';
   }
   for (i = 0; i < (size_t)pbuf->fmtAscCharIdx; i++) {
      pbuf->fmtAscBuffer[i] = '-';
   }

   /* Get byte index and mask from field relative bit offset */

   byteIndex = getMaskAndIndex (currBitOffset, &bitMask);

   /* Loop to dump all bits in the field */

   for (i = 0; i < numbits; i++) {
      bitValue = (OSBOOL)((dataBuffer[byteIndex] & bitMask) != 0);

      fmtAndPrintBit (bitValue, pbuf, plist);

      if ((bitMask >>= 1) == 0) {
         byteIndex++;
         bitMask = 0x80;
      }

      currBitOffset++;
   }

   /* Print unused bits between this and the next field */

   while (currBitOffset < nextBitOffset) {
      fmtAndPrintBit (-1, pbuf, plist);
      currBitOffset++;
   }

   /* If anything remains in the format buffer, add entry to list */

   if (pbuf->fmtBitCharIdx > 0) {

      /* Calculate bit index */

      if (pbuf->fmtBitCharIdx < 8) bitidx = pbuf->fmtBitCharIdx;
      else if (pbuf->fmtBitCharIdx < 17) bitidx = pbuf->fmtBitCharIdx - 1;
      else if (pbuf->fmtBitCharIdx < 26) bitidx = pbuf->fmtBitCharIdx - 2;
      else bitidx = pbuf->fmtBitCharIdx - 3;

      /* Fill to end of bit dump buffer with hyphens */

      for (i = pbuf->fmtBitCharIdx, j = bitidx % 8; i < 35; i++) {
         if (j == 8) {
            pbuf->fmtBitBuffer[i] = ' ';
            j = 0;
         }
         else {
            pbuf->fmtBitBuffer[i] = '-';
            j++;
         }
      }
      for (i = pbuf->fmtHexCharIdx; i < 8; i++) {
         pbuf->fmtHexBuffer[i] = '-';
      }
      for (i = pbuf->fmtAscCharIdx; i < 4; i++) {
         pbuf->fmtAscBuffer[i] = '-';
      }

      plistbuf = (BinDumpBuffer*) malloc (sizeof(BinDumpBuffer));
      if (0 != plistbuf) {
         OSCRTLSAFEMEMCPY (plistbuf, sizeof(BinDumpBuffer), pbuf,
               sizeof(BinDumpBuffer));
         rtxSListAppend (plist, plistbuf);
      }
   }
}

static void dumpField
(FILE* fp, const OSOCTET* dataBuffer, OSRTDiagBitField* pField,
 const char* varname, size_t nextBitOffset, BinDumpBuffer* pbuf)
{
   char lbuf[400];
   BinDumpBuffer* pListBuf;
   OSRTSListNode* pNode;
   OSRTSList binDumpBufferList;

   rtxSListInit (&binDumpBufferList);

   fprintf (fp, "<tr>\n");
   fprintf (fp, "<td colspan=\"3\" class=\"xml\">\n");
   fprintf (fp, "<code>\n");

   /* Print field name */

   lbuf[0] = '\0';
   if (!OS_ISEMPTY(varname))
      rtxStrcat (lbuf, sizeof(lbuf), varname);

   if (!OS_ISEMPTY(pField->elemName)) {
      if ('\0' != lbuf[0])
         rtxStrncat (lbuf, sizeof(lbuf), ".", 1);

      rtxStrcat (lbuf, sizeof(lbuf), pField->elemName);
   }

   if (!OS_ISEMPTY(pField->nameSuffix)) {
      if ('\0' != lbuf[0])
         rtxStrncat (lbuf, sizeof(lbuf), " ", 1);

      rtxStrncat (lbuf, sizeof(lbuf), pField->nameSuffix, 32);
   }
   fprintf (fp, "%s<br/>\n", lbuf);

   /* Print bit offsets and counts */

#if defined(VMS) || defined(__VMS)
   fprintf (fp, "bit offset: %d bit count: %d next bit offset: %lu",
            pField->bitOffset, pField->numbits, nextBitOffset);
#else
   fprintf (fp, "bit offset: %zd bit count: %zd next bit offset: %zd",
            pField->bitOffset, pField->numbits, nextBitOffset);
#endif

   fprintf (fp, "</td>\n");
   fprintf (fp, "</tr>\n");
   fprintf (fp, "\n");

   /* Get formatted bit fit field text data list */
   getBitTextList (dataBuffer, pField->bitOffset, pField->numbits,
                   nextBitOffset, pbuf, &binDumpBufferList);

   /* Print formatted data rows */
   fprintf (fp, "<tr>\n");
   fprintf (fp, "<td align=\"left\" width=\"60%%\"><code>\n");

   for (pNode = binDumpBufferList.head; pNode != 0; pNode = pNode->next) {
      pListBuf = (BinDumpBuffer*) pNode->data;
      fprintf (fp, "%s", pListBuf->fmtBitBuffer);
      if (0 != pNode->next) {
         fprintf (fp, "<br/>");
      }
      fprintf (fp, "\n");
   }

   fprintf (fp, "</code></td>\n");
   fprintf (fp, "<td align=\"left\" width=\"25%%\"><code>\n");

   if (0 != binDumpBufferList.head) {
      for (pNode = binDumpBufferList.head; pNode != 0; pNode = pNode->next) {
         pListBuf = (BinDumpBuffer*) pNode->data;
         fprintf (fp, "%s", pListBuf->fmtHexBuffer);
         if (0 != pNode->next) {
            fprintf (fp, "<br/>");
         }
         fprintf (fp, "\n");
      }
   }
   else {
      fprintf (fp, "&nbsp;\n");
   }

   fprintf (fp, "</code></td>\n");
   fprintf (fp, "<td align=\"left\" width=\"15%%\"><code>\n");

   for (pNode = binDumpBufferList.head; pNode != 0; pNode = pNode->next) {
      pListBuf = (BinDumpBuffer*) pNode->data;
      fprintf (fp, "%s", pListBuf->fmtAscBuffer);
      if (0 != pNode->next) {
         fprintf (fp, "<br/>");
      }
      fprintf (fp, "\n");
      free (pListBuf);
   }

   rtxSListFree (&binDumpBufferList);
   fprintf (fp, "</code></td>\n");
   fprintf (fp, "</tr>\n\n");
}

EXTRTMETHOD void rtxDiagBitTracePrintHTML
(const char* filename, OSRTDiagBitFieldList* pBFList, const char* varname)
{
   size_t nextBitOffset;
   OSRTSListNode* pNode;
   const OSOCTET* dataBuffer;
   BinDumpBuffer binDumpBuffer;
   OSCTXT* pctxt;
   FILE* fp;

   if (0 == pBFList) return;

   pctxt = pBFList->fieldList.pctxt;

   if (!OS_ISEMPTY(filename) && strcmp(filename, "stdout")) {
      if (rtxFileOpen (&fp, filename, "w") < 0) {
         printf ("could not open %s for write access\n", filename);
         return;
      }
   }
   else fp = stdout;

   OSCRTLMEMSET (&binDumpBuffer, 0, sizeof(binDumpBuffer));
   binDumpBuffer.lbm = 0x80;

#ifndef _NO_STREAM
   /* If stream, set data buffer to point at capture buffer; otherwise
      use context buffer */
   if (OSRTISSTREAM (pctxt)) {
      OSRTMEMBUF* pmembuf = rtxStreamGetCapture (pctxt);
      if (0 != pmembuf) {
         dataBuffer = rtxMemBufGetData (pmembuf, 0);
      }
      else {
         printf ("ERROR: capture buffer was not assigned to stream\n");
         if (fp != stdout) fclose (fp);
		 return;
      }
   }
   else
#endif /* _NO_STREAM */
      dataBuffer = pctxt->buffer.data;

   /* Print start of HTML document */
   fputs (htmlHeader, fp);
   fprintf (fp, "<body>\n");
   fprintf (fp, "\n");
   fprintf
      (fp, "<h1><a name=\"title\" id=\"title\"></a>EXI 1.0 Encoding</h1>\n");
   fprintf (fp, "<table border=\"1\" width=\"800\"><thead>\n");

   /* Loop through the field list and dump fields */
   pNode = pBFList->fieldList.head;
   while (pNode) {
      nextBitOffset = (pNode->next) ?
         ((OSRTDiagBitField*)pNode->next->data)->bitOffset : 0;

      dumpField (fp, dataBuffer, (OSRTDiagBitField*)pNode->data, varname,
                 nextBitOffset, &binDumpBuffer);

      pNode = pNode->next;
   }

   fprintf (fp, "</table>\n");
   fprintf (fp, "</body>\n");
   fprintf (fp, "</html>\n");
   if (fp != stdout) fclose (fp);
}
