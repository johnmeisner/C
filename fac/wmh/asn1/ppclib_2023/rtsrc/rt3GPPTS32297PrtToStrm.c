/*
 * Copyright (C) 2017-2023 Objective Systems, Inc.
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

#include "rt3GPPTS32297.h"
#include "rtsrc/rtPrintToStream.h"

int rtPrtToStrmTS32297FileHdr
(OSCTXT *pctxt, TS32297CDRFileHeader *pCdrFileHeader)
{
   char str[20];

   rtPrintToStreamUnsigned
      (pctxt, "CDR_File_Header.fileLength", pCdrFileHeader->fileLength);

   rtPrintToStreamUnsigned
      (pctxt, "CDR_File_Header.headerLength", pCdrFileHeader->headerLength);

   rtTS32297RelVersIdToText
      (&pCdrFileHeader->highReleaseVersionID,
       pCdrFileHeader->highReleaseExtension, str, sizeof(str));

   rtPrintToStreamCharStr (pctxt, "CDR_File_Header.highReleaseVersionID", str);

   rtTS32297RelVersIdToText
      (&pCdrFileHeader->lowReleaseVersionID,
       pCdrFileHeader->lowReleaseExtension, str, sizeof(str));

   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.lowReleaseVersionID", str);

   rtTS32297TimestampToText (&pCdrFileHeader->openTimestamp, str, sizeof(str));
   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.openTimestamp", str);

   rtTS32297TimestampToText
      (&pCdrFileHeader->lastAppendTimestamp, str, sizeof(str));

   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.lastAppendTimestamp", str);

   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.cdrCount",
                           pCdrFileHeader->cdrCount);
   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.fileSequenceNumber",
                           pCdrFileHeader->fileSequenceNumber);
   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.fileClosureReason",
                           pCdrFileHeader->fileClosureReason);

   rtPrintToStreamHexStr(pctxt, "CDR_File_Header.ipAddress",
                         sizeof(pCdrFileHeader->ipAddress),
                         pCdrFileHeader->ipAddress);
   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.lostCDRIndicator",
                           pCdrFileHeader->lostCDRIndicator);

   if (pCdrFileHeader->cdrRoutingFilterLength > 0)
   {
      rtPrintToStreamHexStr(pctxt, "CDR_File_Header.cdrRoutingFilter",
                            pCdrFileHeader->cdrRoutingFilterLength,
                            pCdrFileHeader->cdrRoutingFilter);
   }

   if (pCdrFileHeader->privateExtensionLength > 0)
   {
      rtPrintToStreamHexStr(pctxt, "CDR_File_Header.privateExtension",
                            pCdrFileHeader->privateExtensionLength,
                            pCdrFileHeader->privateExtension);
   }

   if (pCdrFileHeader->highExtensionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.highReleaseExtension",
                              pCdrFileHeader->highReleaseExtension);
   }

   if (pCdrFileHeader->lowExtensionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.lowReleaseExtension",
                              pCdrFileHeader->lowReleaseExtension);
   }

   return 0;
}

int rtPrtToStrmTS32297Hdr(OSCTXT *pctxt, TS32297CDRHeader *pCdrHeader)
{
   char str[15];

   rtPrintToStreamUnsigned
      (pctxt, "CDR_Header.cdrLength", pCdrHeader->cdrLength);

   rtTS32297RelVersIdToText
      (&pCdrHeader->releaseVersionID,
       pCdrHeader->releaseExtension, str, sizeof(str));

   rtPrintToStreamCharStr (pctxt, "CDR_Header.releaseVersionId", str);
   rtPrintToStreamUnsigned
      (pctxt, "CDR_Header.dataRecordFormat", pCdrHeader->dataRecordFormat);

   rtTS32297TSNumberToText (pCdrHeader->tsNumber, str, sizeof(str));
   rtPrintToStreamCharStr (pctxt, "CDR_Header.tsNumber", str);

   if (pCdrHeader->releaseExtensionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_Header.releaseExtension",
                              pCdrHeader->releaseExtension);
   }

   return 0;
}
