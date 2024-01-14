/*
 * Copyright (C) 2018-2018 Objective Systems, Inc.
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


int rtPrtToStrmTS32297FileHdr(OSCTXT *pctxt,
                              TS32297CDRFileHeader *pCdrFileHeader)
{
   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.fileLength",
                           pCdrFileHeader->fileLength);
   rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.headerLength",
                           pCdrFileHeader->headerLength);

   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.highReleaseVersionID",
                          pCdrFileHeader->highReleaseVersionID);
   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.lowReleaseVersionID",
                          pCdrFileHeader->lowReleaseVersionID);
   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.openTimestamp",
                          pCdrFileHeader->openTimestamp);
   rtPrintToStreamCharStr(pctxt, "CDR_File_Header.lastAppendTimestamp",
                          pCdrFileHeader->lastAppendTimestamp);

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

   if (pCdrFileHeader->highExtentionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.highReleaseExtension",
                              pCdrFileHeader->highReleaseExtension);
   }

   if (pCdrFileHeader->lowExtentionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_File_Header.lowReleaseExtension",
                              pCdrFileHeader->lowReleaseExtension);
   }

   return 0;
}

int rtPrtToStrmTS32297Hdr(OSCTXT *pctxt, TS32297CDRHeader *pCdrHeader)
{
   rtPrintToStreamUnsigned(pctxt, "CDR_Header.cdrLength",
                           pCdrHeader->cdrLength);
   rtPrintToStreamCharStr(pctxt, "CDR_Header.releaseVersionId",
                          pCdrHeader->releaseVersionID);
   rtPrintToStreamUnsigned(pctxt, "CDR_Header.dataRecordFormat",
                           pCdrHeader->dataRecordFormat);
   rtPrintToStreamCharStr(pctxt, "CDR_Header.tsNumber", pCdrHeader->tsNumber);

   if (pCdrHeader->releaseExtensionPresent)
   {
      rtPrintToStreamUnsigned(pctxt, "CDR_Header.releaseExtension",
                              pCdrHeader->releaseExtension);
   }

   return 0;
}
