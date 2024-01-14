/*
 * Copyright (C) 1997-2018 Objective Systems, Inc.
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
#include "rtxsrc/rtxCommon.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxIntDecode.h"
#include "rtxsrc/rtxIntEncode.h"

int decodeReleaseVersionIdentifier(OSCTXT* pctxt, char *pStr, size_t strSize)
{
   OSOCTET field;
   int relid, stat, verid;
   char verid_str[5];

   OSCRTLMEMSET(&verid_str, 0, 5);

   stat = rtxDecUInt8(pctxt, &field);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   relid = field >> 5;
   verid = field & 0x1F;
   switch (relid)
   {
      case 0: rtxStrcpy(pStr, strSize, "rel-99 v."); break;
      case 1: rtxStrcpy(pStr, strSize, "rel-4 v."); break;
      case 2: rtxStrcpy(pStr, strSize, "rel-5 v."); break;
      case 3: rtxStrcpy(pStr, strSize, "rel-6 v."); break;
      case 4: rtxStrcpy(pStr, strSize, "rel-7 v."); break;
      case 5: rtxStrcpy(pStr, strSize, "rel-8 v."); break;
      case 6: rtxStrcpy(pStr, strSize, "rel-9 v."); break;
      case 7: rtxStrcpy(pStr, strSize, "Beyond rel-9 v."); break;
      default: rtxStrcpy(pStr, strSize, "rel-99 v."); break;
   }
   rtxUIntToCharStr(verid, verid_str, 5, 0);
   rtxStrcat(pStr, strSize, verid_str);

   return stat;
}

int encodeReleaseVersionIdentifier(OSCTXT* pctxt, char *pStr)
{
   char *p;
   int relid = 0, verid = 0, stat = 0;

   if (strstr(pStr, "Beyond") != NULL)
   {
      relid = 7 << 5;
   }
   else if (strstr(pStr, "rel-9") != NULL)
   {
      relid = 6 << 5;
   }
   else if (strstr(pStr, "rel-8") != NULL)
   {
      relid = 5 << 5;
   }
   else if (strstr(pStr, "rel-7") != NULL)
   {
      relid = 4 << 5;
   }
   else if (strstr(pStr, "rel-6") != NULL)
   {
      relid = 3 << 5;
   }
   else if (strstr(pStr, "rel-5") != NULL)
   {
      relid = 2 << 5;
   }
   else if (strstr(pStr, "rel-4") != NULL)
   {
      relid = 1 << 5;
   }

   p = strchr(pStr, '.');
   if (p == NULL)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   stat = rtxCharStrToInt(p+1, &verid);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   relid += verid;

   stat = rtxWriteBytes(pctxt, (OSOCTET *)&relid, 1);
   return stat;
}

int decodeTimestamp(OSCTXT* pctxt, char *pTimestamp, size_t tsSize)
{
   OSOCTET buffer[4];
   OSUINT8 month, date, hour, minute, zhour, zminute;
   int stat;
   char tmpstr1[3], tmpstr2[3], ts[10];
   OSBOOL zdiff_plus;

   OSCRTLMEMSET(&tmpstr1, 0, sizeof(tmpstr1));
   OSCRTLMEMSET(&tmpstr2, 0, sizeof(tmpstr2));
   OSCRTLMEMSET(&ts, 0, sizeof(ts));

   stat = rtxReadBytes(pctxt, buffer, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   month = (OSUINT8)(buffer[0] >> 4);
   date = (OSUINT8)(((buffer[0] & 0x0F) << 1) + (buffer[1] >> 7));
   hour = (OSUINT8)((buffer[1] >> 2) & 0x1F);
   minute = (OSUINT8)(((buffer[1] << 4) & 0x30) + (buffer[2] >> 4));
   zhour = (OSUINT8)(((buffer[2] << 2) & 0x1C) + (buffer[3] >> 6));
   zminute = (OSUINT8)(buffer[3] & 0x3F);
   zdiff_plus = (buffer[2] & 0x8);

   rtxUIntToCharStr(month, tmpstr1, 3, '0');
   rtxUIntToCharStr(date, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts), tmpstr1, "/", tmpstr2, " ", NULL);
   rtxStrcpy(pTimestamp, tsSize, ts);
   rtxUIntToCharStr(hour, tmpstr1, 3, '0');
   rtxUIntToCharStr(minute, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts), tmpstr1, ":", tmpstr2, NULL, NULL);
   rtxStrcat(pTimestamp, tsSize, ts);
   rtxUIntToCharStr(zhour, tmpstr1, 3, '0');
   rtxUIntToCharStr(zminute, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts), zdiff_plus ? "+" : "-", tmpstr1, ":", tmpstr2, NULL);
   rtxStrcat(pTimestamp, tsSize, ts);

   return 0;
}

int encodeTimestamp(OSCTXT* pctxt, char *pTimestamp)
{
   OSOCTET buffer[4];
   OSUINT8 month, date, hour, minute, zhour, zminute;
   char *p = pTimestamp;
   int stat;
   char zdifferential[2];

   stat = rtxCharStrToUInt8(p, &month);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   p += 3;
   stat = rtxCharStrToUInt8(p, &date);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   p += 3;
   stat = rtxCharStrToUInt8(p, &hour);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   p += 3;
   stat = rtxCharStrToUInt8(p, &minute);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   p += 2;
   if (NULL == rtxStrncpy(zdifferential, sizeof(zdifferential), p, 1))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   p += 1;
   stat = rtxCharStrToUInt8(p, &zhour);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   p += 3;
   stat = rtxCharStrToUInt8(p, &zminute);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   OSCRTLMEMSET(buffer, 0, sizeof(buffer));
   buffer[0] = (month << 4) + (date >> 1);
   buffer[1] = (date << 7) + (hour << 2) + (minute >> 4);
   buffer[2] = (minute << 4) + (zdifferential[0] == '+' ? 0x8 : 0) + (zhour >> 2);
   buffer[3] = (zhour << 6) + zminute;

   stat = rtxWriteBytes(pctxt, buffer, 4);
   return stat;
}


void rtInitTS32297FileHdr(TS32297CDRFileHeader *pCdrFileHeader)
{
   OSCRTLMEMSET(pCdrFileHeader, 0, sizeof(TS32297CDRFileHeader));

   rtxStrcpy(pCdrFileHeader->highReleaseVersionID,
             sizeof(pCdrFileHeader->highReleaseVersionID), "rel-99 v.0");
   rtxStrcpy(pCdrFileHeader->lowReleaseVersionID,
             sizeof(pCdrFileHeader->lowReleaseVersionID), "rel-99 v.0");
   rtxStrcpy(pCdrFileHeader->openTimestamp,
             sizeof(pCdrFileHeader->openTimestamp), "01/01 09:00+00:00");
   rtxStrcpy(pCdrFileHeader->lastAppendTimestamp,
             sizeof(pCdrFileHeader->lastAppendTimestamp), "01/01 09:00+00:00");
}

void rtInitTS32297Hdr(TS32297CDRHeader *pCdrHeader)
{
   OSCRTLMEMSET(pCdrHeader, 0, sizeof(TS32297CDRHeader));

   rtxStrcpy(pCdrHeader->releaseVersionID,
             sizeof(pCdrHeader->releaseVersionID), "rel-99 v.0");
   rtxStrcpy(pCdrHeader->tsNumber, sizeof(pCdrHeader->tsNumber), "32.005");
   pCdrHeader->dataRecordFormat = BER;
}

int rtDecTS32297FileHdr(OSCTXT* pctxt, TS32297CDRFileHeader *pCdrFileHeader)
{
   size_t count = 0;
   OSUINT32 hdrLen, len;
   int stat;

   stat = rtxDecUInt32(pctxt, &pCdrFileHeader->fileLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 4;

   stat = rtxDecUInt32(pctxt, &pCdrFileHeader->headerLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 4;
   hdrLen = pCdrFileHeader->headerLength;

   if (count < hdrLen)
   {
      stat = decodeReleaseVersionIdentifier(pctxt,
                                 pCdrFileHeader->highReleaseVersionID,
                                 sizeof(pCdrFileHeader->highReleaseVersionID));
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 1;
   }

   if (count < hdrLen)
   {
      stat = decodeReleaseVersionIdentifier(pctxt,
                                 pCdrFileHeader->lowReleaseVersionID,
                                 sizeof(pCdrFileHeader->lowReleaseVersionID));
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 1;
   }

   if (count < hdrLen)
   {
      stat = decodeTimestamp(pctxt, pCdrFileHeader->openTimestamp,
                             sizeof(pCdrFileHeader->openTimestamp));
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 4;
   }

   if (count < hdrLen)
   {
      stat = decodeTimestamp(pctxt, pCdrFileHeader->lastAppendTimestamp,
                             sizeof(pCdrFileHeader->lastAppendTimestamp));
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 4;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt32(pctxt, &pCdrFileHeader->cdrCount, 4);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 4;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt32(pctxt, &pCdrFileHeader->fileSequenceNumber, 4);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 4;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt8(pctxt, &pCdrFileHeader->fileClosureReason);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 1;
   }

   if (count < hdrLen)
   {
      stat = rtxReadBytes(pctxt, pCdrFileHeader->ipAddress, STRING_SIZE);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 20;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt8(pctxt, &pCdrFileHeader->lostCDRIndicator);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 1;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt16(pctxt, &pCdrFileHeader->cdrRoutingFilterLength, 2);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 2;
      len = pCdrFileHeader->cdrRoutingFilterLength;

      if (len > pCdrFileHeader->fileLength)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
      else if (len > 0)
      {
         pCdrFileHeader->cdrRoutingFilter = (OSOCTET *)malloc(len);
         stat = rtxReadBytes(pctxt, pCdrFileHeader->cdrRoutingFilter, len);
         if (stat < 0)
         {
            return LOG_RTERR(pctxt, stat);
         }

         count += len;
      }
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt16(pctxt, &pCdrFileHeader->privateExtensionLength, 2);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 2;
      len = pCdrFileHeader->privateExtensionLength;

      if (len > pCdrFileHeader->fileLength)
      {
         return LOG_RTERR(pctxt, RTERR_BADVALUE);
      }
      else if (len > 0)
      {
         pCdrFileHeader->privateExtension = (OSOCTET *)malloc(len);
         stat = rtxReadBytes(pctxt, pCdrFileHeader->privateExtension, len);
         if (stat < 0)
         {
            return LOG_RTERR(pctxt, stat);
         }

         count += len;
      }
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt8(pctxt, &pCdrFileHeader->highReleaseExtension);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      pCdrFileHeader->highExtentionPresent = TRUE;
      count += 1;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt8(pctxt, &pCdrFileHeader->lowReleaseExtension);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      pCdrFileHeader->lowExtentionPresent = TRUE;
      count += 1;
   }

   return (int)count;
}

int rtDecTS32297Hdr(OSCTXT* pctxt, TS32297CDRHeader *pCdrHeader)
{
   int count = 0, stat = 0;
   size_t size = sizeof(pCdrHeader->tsNumber);
   OSUINT8 temp = 0;

   stat = rtxDecUInt16(pctxt, &pCdrHeader->cdrLength, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 2;

   stat = decodeReleaseVersionIdentifier(pctxt, pCdrHeader->releaseVersionID,
                                         sizeof(pCdrHeader->releaseVersionID));
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 1;

   pCdrHeader->releaseExtensionPresent = (strncmp(pCdrHeader->releaseVersionID,
                                          "Beyond", 6) == 0) ? TRUE : FALSE;

   stat = rtxDecUInt8(pctxt, &temp);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 1;

   switch ((temp >> 5))
   {
      case 1: pCdrHeader->dataRecordFormat = BER; break;
      case 2: pCdrHeader->dataRecordFormat = UPER; break;
      case 3: pCdrHeader->dataRecordFormat = PER; break;
      case 4: pCdrHeader->dataRecordFormat = XER; break;
      default: pCdrHeader->dataRecordFormat = BER; break;
   }

   switch (temp & 0x1F)
   {
      case 0: rtxStrcpy(pCdrHeader->tsNumber, size, "32.005"); break;
      case 1: rtxStrcpy(pCdrHeader->tsNumber, size, "32.015"); break;
      case 2: rtxStrcpy(pCdrHeader->tsNumber, size, "32.205"); break;
      case 3: rtxStrcpy(pCdrHeader->tsNumber, size, "32.215"); break;
      case 4: rtxStrcpy(pCdrHeader->tsNumber, size, "32.225"); break;
      case 5: rtxStrcpy(pCdrHeader->tsNumber, size, "32.235"); break;
      case 6: rtxStrcpy(pCdrHeader->tsNumber, size, "32.250"); break;
      case 7: rtxStrcpy(pCdrHeader->tsNumber, size, "32.251"); break;
      case 8: rtxStrcpy(pCdrHeader->tsNumber, size, "32.252"); break;
      case 9: rtxStrcpy(pCdrHeader->tsNumber, size, "32.260"); break;
      case 10: rtxStrcpy(pCdrHeader->tsNumber, size, "32.270"); break;
      case 11: rtxStrcpy(pCdrHeader->tsNumber, size, "32.271"); break;
      case 12: rtxStrcpy(pCdrHeader->tsNumber, size, "32.272"); break;
      case 13: rtxStrcpy(pCdrHeader->tsNumber, size, "32.273"); break;
      case 14: rtxStrcpy(pCdrHeader->tsNumber, size, "32.275"); break;
      case 15: rtxStrcpy(pCdrHeader->tsNumber, size, "32.274"); break;
      default: rtxStrcpy(pCdrHeader->tsNumber, size, "Unknown"); break;
   }

   if (pCdrHeader->releaseExtensionPresent)
   {
      stat = rtxDecUInt8(pctxt, &pCdrHeader->releaseExtension);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
      count += 1;
   }

   return count;
}

int rtEncTS32297FileHdr(OSCTXT* pctxt, TS32297CDRFileHeader *pCdrFileHeader)
{
   int stat;

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->fileLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->headerLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeReleaseVersionIdentifier(pctxt,
                                         pCdrFileHeader->highReleaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeReleaseVersionIdentifier(pctxt,
                                         pCdrFileHeader->lowReleaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeTimestamp(pctxt, pCdrFileHeader->openTimestamp);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeTimestamp(pctxt, pCdrFileHeader->lastAppendTimestamp);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->cdrCount, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->fileSequenceNumber, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->fileClosureReason, 1);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxWriteBytes(pctxt, pCdrFileHeader->ipAddress, STRING_SIZE);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->lostCDRIndicator, 1);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->cdrRoutingFilterLength, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (pCdrFileHeader->cdrRoutingFilterLength > 0)
   {
      stat = rtxWriteBytes(pctxt, pCdrFileHeader->cdrRoutingFilter,
                           pCdrFileHeader->cdrRoutingFilterLength);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (pCdrFileHeader->privateExtensionLength > 0)
   {
      stat = rtxEncUInt32(pctxt, pCdrFileHeader->privateExtensionLength, 2);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      stat = rtxWriteBytes(pctxt, pCdrFileHeader->privateExtension,
                           pCdrFileHeader->privateExtensionLength);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (pCdrFileHeader->highExtentionPresent)
   {
      stat = rtxEncUInt32(pctxt, pCdrFileHeader->highReleaseExtension, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (pCdrFileHeader->lowExtentionPresent)
   {
      stat = rtxEncUInt32(pctxt, pCdrFileHeader->lowReleaseExtension, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   return pCdrFileHeader->headerLength;
}

int rtEncTS32297Hdr(OSCTXT* pctxt, TS32297CDRHeader *pCdrHeader)
{
   OSUINT32 temp;
   int stat;

   stat = rtxEncUInt32(pctxt, pCdrHeader->cdrLength, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeReleaseVersionIdentifier(pctxt, pCdrHeader->releaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   switch (pCdrHeader->dataRecordFormat)
   {
      case BER: temp = 1 << 5; break;
      case UPER: temp = 2 << 5; break;
      case PER: temp = 3 << 5; break;
      case XER: temp = 4 << 5; break;
      default: temp = 1 << 5; break;
   }

   if (NULL == pCdrHeader->tsNumber)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.005"))
   {
      temp += 0;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.015"))
   {
      temp += 1;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.205"))
   {
      temp += 2;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.215"))
   {
      temp += 3;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.225"))
   {
      temp += 4;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.235"))
   {
      temp += 5;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.250"))
   {
      temp += 6;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.251"))
   {
      temp += 7;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.252"))
   {
      temp += 8;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.260"))
   {
      temp += 9;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.270"))
   {
      temp += 10;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.271"))
   {
      temp += 11;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.272"))
   {
      temp += 12;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.273"))
   {
      temp += 13;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.275"))
   {
      temp += 14;
   }
   else if (0 == rtxStricmp(pCdrHeader->tsNumber, "32.274"))
   {
      temp += 15;
   }
   else
   {
      temp += 1;
   }

   stat = rtxEncUInt32(pctxt, temp, 1);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (pCdrHeader->releaseExtensionPresent)
   {
      stat = rtxEncUInt32(pctxt, pCdrHeader->releaseExtension, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   return (pCdrHeader->releaseExtensionPresent ? 5 : 4);
}
