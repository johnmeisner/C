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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxDiagBitTrace.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxCharStr.h"

static void fmtAndPrintBit
(OSCTXT* pctxt, int value, OSRTDiagBinDumpBuffer* pbuf)
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
      char hexCharStr[4];

      if (pbuf->fmtAscCharIdx < 4)
         pbuf->fmtBitBuffer[pbuf->fmtBitCharIdx++] = ' ';

      rtxByteToHexChar (pbuf->lb, hexCharStr, sizeof(hexCharStr));
      pbuf->fmtHexBuffer[pbuf->fmtHexCharIdx++] = hexCharStr[0];
      pbuf->fmtHexBuffer[pbuf->fmtHexCharIdx++] = hexCharStr[1];

      pbuf->fmtAscBuffer[pbuf->fmtAscCharIdx++] =
         OS_ISPRINT(pbuf->lb) ? pbuf->lb : '.';

      pbuf->lb = 0;

      /* If buffer is full, print to stdout */

      if (pbuf->fmtAscCharIdx >= 4) {
         rtxPrintToStream
            (pctxt, "%-35.35s    %-8.8s    %-4.4s\n",
             pbuf->fmtBitBuffer, pbuf->fmtHexBuffer, pbuf->fmtAscBuffer);

         OSCRTLMEMSET (pbuf, 0, sizeof(OSRTDiagBinDumpBuffer));
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

static void dumpBits (OSCTXT* pctxt, const OSOCTET* dataBuffer,
                      size_t currBitOffset, size_t numbits,
                      size_t nextBitOffset, OSRTDiagBinDumpBuffer* pbuf)
{
   size_t  byteIndex, i, j, bitidx;
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

      fmtAndPrintBit (pctxt, bitValue, pbuf);

      if ((bitMask >>= 1) == 0) {
         byteIndex++;
         bitMask = 0x80;
      }

      currBitOffset++;
   }

   /* Print unused bits between this and the next field */

   while (currBitOffset < nextBitOffset) {
      fmtAndPrintBit (pctxt, -1, pbuf);
      currBitOffset++;
   }

   /* If anything remains in the format buffer, print to stdout */

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

      rtxPrintToStream
         (pctxt, "%-35.35s    %-8.8s    %-4.4s\n",
          pbuf->fmtBitBuffer, pbuf->fmtHexBuffer, pbuf->fmtAscBuffer);
   }

}

static void dumpField
(OSCTXT* pctxt, const OSOCTET* dataBuffer, OSRTDiagBitField* pField,
 const char* varname, size_t nextBitOffset, OSRTDiagBinDumpBuffer* pbuf)
{
   char lbuf[400];

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
   rtxPrintToStream (pctxt, "%s\n", lbuf);

   /* Print bit offsets and counts */

   rtxPrintToStream
      (pctxt, "bit offset: %d\tbit count: %d\tnext bit offset: %d\n",
       pField->bitOffset, pField->numbits, nextBitOffset);

   /* Print bits */

   dumpBits (pctxt, dataBuffer, pField->bitOffset,
             pField->numbits, nextBitOffset, pbuf);

   rtxPrintToStream (pctxt, "\n");
}

static void dumpFields (OSRTDiagBitFieldList* pBFList,
   const char* varname, int level, const OSOCTET* dataBuffer)
{
   size_t bitAdjustment = 0, nextBitOffset;
   OSRTSListNode* pNode;
   OSRTDiagBitField* pField;
   OSCTXT* pctxt;

   if (0 == pBFList) return;

   pctxt = pBFList->fieldList.pctxt;

   if (0 == dataBuffer) {
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
            return;
         }
      }
      else
#endif /* _NO_STREAM */
         dataBuffer = pctxt->buffer.data;
   }

   pNode = (0 == pBFList->pLastPrinted) ?
      pBFList->fieldList.head : pBFList->pLastPrinted->next;

   /* If first field is a bit adjustment record, set value to use to
      adjust bit offsets in all subsequent records */

   if (0 != pNode) {
      pField = (OSRTDiagBitField*) pNode->data;
      if (0 == pField->elemName && 0 == pField->numbits) {
         bitAdjustment = pField->bitOffset;
         pNode = pNode->next;
      }
   }

   /* Loop through the field list and dump fields */

   while (pNode) {
      nextBitOffset = (pNode->next) ?
         ((OSRTDiagBitField*)pNode->next->data)->bitOffset + bitAdjustment : 0;

      pField = (OSRTDiagBitField*)pNode->data;
      pField->bitOffset += bitAdjustment;

      dumpField (pctxt, dataBuffer, pField, varname,
                 nextBitOffset, &pBFList->printBuffer);

      if (0 != pField->pOpenTypeFieldList) {
         size_t byteIndex = pField->bitOffset/8;
         rtxPrintToStream (pctxt, "*** Open Type Data ***\n");

         /* Make recursive call */
         dumpFields (pField->pOpenTypeFieldList, varname,
                     level+1, &dataBuffer[byteIndex]);

         rtxPrintToStream (pctxt, "***\n");
      }

      pBFList->pLastPrinted = pNode;
      pNode = pNode->next;
   }
}

EXTRTMETHOD void rtxDiagBitTracePrint
(OSRTDiagBitFieldList* pBFList, const char* varname)
{
   dumpFields (pBFList, varname, 0, 0);
}
