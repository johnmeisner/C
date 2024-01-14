/*
 * Copyright (C) 1997-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxIntDecode.h"
#include "rtxsrc/rtxIntEncode.h"

int rtParseTS32297RelVersText (const char* text, TS32297RelVersId* pRelVersId)
{
   if (0 == strncmp (text, "Beyond", 6)) {
      pRelVersId->relId = 10;
      pRelVersId->versId = 31;
   }
   else { /* format is rel-%d v.%d */
      const char* pstr = text;

      pRelVersId->relId = 0;
      pRelVersId->versId = 0;

      if (0 != strncmp(pstr, "rel-", 4)) {
         return RTERR_BADVALUE;
      }
      pstr += 4;
      while (OS_ISDIGIT(*pstr)) {
         pRelVersId->relId = pRelVersId->relId * 10 + (*pstr - '0');
         pstr++;
      }
      while (OS_ISSPACE(*pstr)) pstr++;
      if (0 != strncmp(pstr, "v.", 2)) {
         return RTERR_BADVALUE;
      }
      pstr += 2; // Skip over "v."...
      while (OS_ISDIGIT(*pstr)) {
         pRelVersId->versId = pRelVersId->versId * 10 + (*pstr - '0');
         pstr++;
      }
   }

   return 0;
}

/* get 3GPP release number */
int rtTS32297GetReleaseNumber
(const TS32297RelVersId* pId, OSUINT8 relIdExt, OSUINT8* pRelNum)
{
   OSUINT8 relnum = 99; /* if relid == 0 */

   if ((pId->relId > 0) && (pId->relId < 7))
   {
      relnum = pId->relId + 3;
   }
   else if (pId->relId == 7)
   {
      relnum = 10 + relIdExt;
   }
   else if (pId->relId > 7)
   {
      return RTERR_BADVALUE;
   }

   if (pRelNum) *pRelNum = relnum;

   return 0;
}

/* relIdExt only used if relVerId is 7 (all 3 bits on)... */
int rtTS32297RelVersIdToText (const TS32297RelVersId* pId, OSUINT8 relIdExt,
                              char *pStr, size_t strSize)
{
   OSUINT8 relnum = 99; /* if relid == 0 */

   int ret = rtTS32297GetReleaseNumber(pId, relIdExt, &relnum);
   if (0 != ret) return ret;

   os_snprintf(pStr, strSize, "rel-%d v.%d", relnum, pId->versId);

   return 0;
}

static int encodeRelVersId (OSCTXT* pctxt, const TS32297RelVersId* pId)
{
   OSOCTET rvbyte;
   OSUINT8 versId = (pId->versId > 31) ? 31 : pId->versId;

   if (pId->relId >= 10)
      rvbyte = (7 << 5) | versId;
   else if (pId->relId >= 3) {
      rvbyte = ((pId->relId - 3) << 5) | versId;
   }
   else return RTERR_BADVALUE;

   return rtxWriteBytes (pctxt, &rvbyte, 1);
}

static int decodeTimestamp (OSCTXT* pctxt, TS32297Timestamp* pTimestamp)
{
   OSUINT32 timestamp;
   OSOCTET buffer[4];

   int stat = rtxDecUInt32(pctxt, &timestamp, 4);
   if (stat < 0) return LOG_RTERR(pctxt, stat);

   buffer[0] = timestamp >> 24;
   timestamp = timestamp << 8;
   buffer[1] = timestamp >> 24;
   timestamp = timestamp << 8;
   buffer[2] = timestamp >> 24;
   timestamp = timestamp << 8;
   buffer[3] = timestamp >> 24;
   pTimestamp->month = (OSUINT8)(buffer[0] >> 4);
   pTimestamp->day = (OSUINT8)(((buffer[0] & 0x0F) << 1) + (buffer[1] >> 7));
   pTimestamp->hour = (OSUINT8)((buffer[1] >> 2) & 0x1F);
   pTimestamp->minute = (OSUINT8)(((buffer[1] << 4) & 0x30) + (buffer[2] >> 4));
   pTimestamp->zhour = (OSUINT8)(((buffer[2] << 2) & 0x1C) + (buffer[3] >> 6));
   pTimestamp->zminute = (OSUINT8)(buffer[3] & 0x3F);
   pTimestamp->zdiff_plus = (buffer[2] & 0x8);

   return 0;
}

static int encodeTimestamp (OSCTXT* pctxt, const TS32297Timestamp* pTimestamp)
{
   int ret = rtxEncBits (pctxt, pTimestamp->month, 4);
   if (0 == ret) {
      ret = rtxEncBits (pctxt, pTimestamp->day, 5);
   }
   if (0 == ret) {
      ret = rtxEncBits (pctxt, pTimestamp->hour, 5);
   }
   if (0 == ret) {
      ret = rtxEncBits (pctxt, pTimestamp->minute, 6);
   }
   if (0 == ret) {
      ret = rtxEncBit (pctxt, pTimestamp->zdiff_plus);
   }
   if (0 == ret) {
      ret = rtxEncBits (pctxt, pTimestamp->zhour, 5);
   }
   if (0 == ret) {
      ret = rtxEncBits (pctxt, pTimestamp->zminute, 6);
   }
   return ret;
}

static int parse2digits (const char* text, OSUINT8* pvalue)
{
   if (OS_ISDIGIT(*text)) {
      *pvalue = (*text++ - '0');
   }
   else return RTERR_BADVALUE;

   if (OS_ISDIGIT(*text)) {
      *pvalue = *pvalue * 10 + (*text - '0');
   }
   else return RTERR_BADVALUE;

   return 0;
}

int rtTS32297TimestampToText
(const TS32297Timestamp* pTimestamp, char* textbuf, OSSIZE textbufsz)
{
   char tmpstr1[3], tmpstr2[3], ts[10];
   rtxUIntToCharStr(pTimestamp->month, tmpstr1, 3, '0');
   rtxUIntToCharStr(pTimestamp->day, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts), tmpstr1, "/", tmpstr2, " ", NULL);
   rtxStrcpy(textbuf, textbufsz, ts);
   rtxUIntToCharStr(pTimestamp->hour, tmpstr1, 3, '0');
   rtxUIntToCharStr(pTimestamp->minute, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts), tmpstr1, ":", tmpstr2, NULL, NULL);
   rtxStrcat(textbuf, textbufsz, ts);
   rtxUIntToCharStr(pTimestamp->zhour, tmpstr1, 3, '0');
   rtxUIntToCharStr(pTimestamp->zminute, tmpstr2, 3, '0');
   rtxStrJoin(ts, sizeof(ts),
      pTimestamp->zdiff_plus ? "+" : "-", tmpstr1, ":", tmpstr2, NULL);
   rtxStrcat(textbuf, textbufsz, ts);
   return 0;
}

int rtParseTS32297TimestampText (const char* text, TS32297Timestamp* pTimestamp)
{
   int ret = parse2digits (text, &pTimestamp->month);
   if (0 == ret && (pTimestamp->month < 1 || pTimestamp->month > 12)) {
      ret = RTERR_BADVALUE;
   }
   if (0 != ret) return ret;
   text += 2;

   if (*text == '/') text++;
   else return RTERR_BADVALUE;

   ret = parse2digits (text, &pTimestamp->day);
   /* TODO: should check for number days within correct range for month */
   if (0 == ret && (pTimestamp->day < 1 || pTimestamp->day > 31)) {
      ret = RTERR_BADVALUE;
   }
   if (0 != ret) return ret;
   text += 2;

   if (OS_ISSPACE(*text)) text++;
   else return RTERR_BADVALUE;

   ret = parse2digits (text, &pTimestamp->hour);
   if (0 == ret && pTimestamp->hour > 24) {
      ret = RTERR_BADVALUE;
   }
   if (0 != ret) return ret;
   text += 2;

   if (*text == ':') text++;
   else return RTERR_BADVALUE;

   ret = parse2digits (text, &pTimestamp->minute);
   if (0 == ret && pTimestamp->minute > 59) {
      ret = RTERR_BADVALUE;
   }
   if (0 != ret) return ret;
   text += 2;

   if ((*text != '+') && (*text != '-'))
   {
      return RTERR_BADVALUE;
   }
   pTimestamp->zdiff_plus = (*text++ == '+') ? TRUE : FALSE;

   ret = parse2digits (text, &pTimestamp->zhour);
   if (0 == ret && pTimestamp->zhour > 12) {
      ret = RTERR_BADVALUE;
   }
   if (0 != ret) return ret;
   text += 2;

   if (*text == ':') text++;
   else return RTERR_BADVALUE;

   ret = parse2digits (text, &pTimestamp->zminute);
   if (0 == ret && pTimestamp->zminute > 59) {
      ret = RTERR_BADVALUE;
   }

   return ret;
}

int rtTS32297TSNumberToText (OSOCTET tsNumber, char *pStr, size_t strSize)
{
   int ret = 0;
   switch (tsNumber & 0x1F)
   {
   case 0: rtxStrcpy(pStr, strSize, "32.005"); break;
   case 1: rtxStrcpy(pStr, strSize, "32.015"); break;
   case 2: rtxStrcpy(pStr, strSize, "32.205"); break;
   case 3: rtxStrcpy(pStr, strSize, "32.215"); break;
   case 4: rtxStrcpy(pStr, strSize, "32.225"); break;
   case 5: rtxStrcpy(pStr, strSize, "32.235"); break;
   case 6: rtxStrcpy(pStr, strSize, "32.250"); break;
   case 7: rtxStrcpy(pStr, strSize, "32.251"); break;
   case 8: rtxStrcpy(pStr, strSize, "32.252"); break;
   case 9: rtxStrcpy(pStr, strSize, "32.260"); break;
   case 10: rtxStrcpy(pStr, strSize, "32.270"); break;
   case 11: rtxStrcpy(pStr, strSize, "32.271"); break;
   case 12: rtxStrcpy(pStr, strSize, "32.272"); break;
   case 13: rtxStrcpy(pStr, strSize, "32.273"); break;
   case 14: rtxStrcpy(pStr, strSize, "32.275"); break;
   case 15: rtxStrcpy(pStr, strSize, "32.274"); break;
   default:
      rtxStrcpy(pStr, strSize, "Unknown");
      ret = RTERR_BADVALUE;
   }
   return ret;
}

int rtParseTS32297TSNumberText (const char* text, OSUINT8* pNumber)
{
   if (text && *text == '3' && *(text+1) == '2' && *(text+2) == '.') {
      OSUINT32 number;
      int ret = rtxCharStrToUInt (text+3, &number);
      if (0 != ret) return ret;

      switch (number) {
      case 5: *pNumber = 0; break;
      case 15: *pNumber = 1; break;
      case 205: *pNumber = 2; break;
      case 215: *pNumber = 3; break;
      case 225: *pNumber = 4; break;
      case 235: *pNumber = 5; break;
      case 250: *pNumber = 6; break;
      case 251: *pNumber = 7; break;
      case 252: *pNumber = 8; break;
      case 260: *pNumber = 9; break;
      case 270: *pNumber = 10; break;
      case 271: *pNumber = 11; break;
      case 272: *pNumber = 12; break;
      case 273: *pNumber = 13; break;
      case 275: *pNumber = 14; break;
      case 274: *pNumber = 15; break;
      default: return RTERR_BADVALUE;
      }
   }
   else return RTERR_BADVALUE;

   return 0;
}

static void initTimestamp (TS32297Timestamp* pTimestamp)
{
   // "01/01 01:00+00:00"
   pTimestamp->month = 1;
   pTimestamp->day = 1;
   pTimestamp->hour = 1;
   pTimestamp->minute = 0;
   pTimestamp->zhour = 0;
   pTimestamp->zminute = 0;
   pTimestamp->zdiff_plus = TRUE;
}

void rtInitTS32297FileHdr(TS32297CDRFileHeader *pCdrFileHeader)
{
   OSCRTLMEMSET(pCdrFileHeader, 0, sizeof(TS32297CDRFileHeader));
   initTimestamp (&pCdrFileHeader->openTimestamp);
   initTimestamp (&pCdrFileHeader->lastAppendTimestamp);
}

void rtInitTS32297Hdr(TS32297CDRHeader *pCdrHeader)
{
   OSCRTLMEMSET(pCdrHeader, 0, sizeof(TS32297CDRHeader));
   pCdrHeader->dataRecordFormat = BER;
}

static int decRelVersId (OSCTXT* pctxt, TS32297RelVersId* pRelVersId)
{
   OSOCTET ub;
   int stat = rtxDecUInt8(pctxt, &ub);
   if (stat < 0) return LOG_RTERR(pctxt, stat);

   pRelVersId->relId = ub >> 5;
   pRelVersId->versId = ub & 0x1F;

   return 0;
}

int rtDecTS32297FileHdr(OSCTXT* pctxt, TS32297CDRFileHeader *pCdrFileHeader)
{
   OSSIZE count = 0;
   OSUINT32 hdrLen, len;
   int stat;

   /* Decode file length */
   stat = rtxDecUInt32(pctxt, &len, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 4;

   /* Decode header length */
   stat = rtxDecUInt32(pctxt, &hdrLen, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 4;

   if (0 == pCdrFileHeader) {
      /* skip header */
      if (hdrLen > count) {
         return rtxReadBytes (pctxt, 0, (hdrLen - count));
      }
      else if (hdrLen < count) {
         return LOG_RTERR (pctxt, RTERR_INVLEN);
      }
      else return 0;
   }

   pCdrFileHeader->fileLength = len;
   pCdrFileHeader->headerLength = hdrLen;

   if (count < hdrLen)
   {
      stat = decRelVersId (pctxt, &pCdrFileHeader->highReleaseVersionID);
      if (stat < 0) return LOG_RTERR(pctxt, stat);
      count += 1;
   }

   if (count < hdrLen)
   {
      stat = decRelVersId (pctxt, &pCdrFileHeader->lowReleaseVersionID);
      if (stat < 0) return LOG_RTERR(pctxt, stat);
      count += 1;
   }

   if (count < hdrLen)
   {
      stat = decodeTimestamp(pctxt, &pCdrFileHeader->openTimestamp);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      count += 4;
   }

   if (count < hdrLen)
   {
      stat = decodeTimestamp(pctxt, &pCdrFileHeader->lastAppendTimestamp);
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
      stat = rtxReadBytes(pctxt, pCdrFileHeader->ipAddress, 20);
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

      pCdrFileHeader->highExtensionPresent = TRUE;
      count += 1;
   }

   if (count < hdrLen)
   {
      stat = rtxDecUInt8(pctxt, &pCdrFileHeader->lowReleaseExtension);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }

      pCdrFileHeader->lowExtensionPresent = TRUE;
      count += 1;
   }

   return (int)count;
}

int rtDecTS32297Hdr(OSCTXT* pctxt, TS32297CDRHeader *pCdrHeader)
{
   int count = 0, stat = 0;
   OSUINT16 len;

   /* Decode CDR length */
   stat = rtxDecUInt16(pctxt, &len, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 2;

   if (0 == pCdrHeader) {
      /* skip header */
      OSOCTET relId;
      stat = rtxReadBytes (pctxt, &relId, 1);
      return (stat < 0) ? LOG_RTERR (pctxt, stat) :
         rtxReadBytes (pctxt, 0, (7 == (relId >> 5)) ? 2 : 1);
   }
   pCdrHeader->cdrLength = len;

   /* Decode release/version identifiers octet */
   stat = decRelVersId (pctxt, &pCdrHeader->releaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 1;

   /* Decode data format/TS number octet */
   stat = rtxDecUInt8(pctxt, &pCdrHeader->tsNumber);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }
   count += 1;

   switch ((pCdrHeader->tsNumber >> 5))
   {
      case 1: pCdrHeader->dataRecordFormat = BER; break;
      case 2: pCdrHeader->dataRecordFormat = UPER; break;
      case 3: pCdrHeader->dataRecordFormat = PER; break;
      case 4: pCdrHeader->dataRecordFormat = XER; break;
      default: pCdrHeader->dataRecordFormat = BER; break;
   }

   /* Decode release extension id octet, if present */
   pCdrHeader->releaseExtensionPresent =
      (pCdrHeader->releaseVersionID.relId == 7) ? TRUE : FALSE;

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

OSUINT32 rtTS32297CalcFileHdrLen
(const TS32297CDRFileHeader *pCdrFileHeader)
{
   OSUINT32 len = 52; /* length of all fixed fields */
   len += pCdrFileHeader->cdrRoutingFilterLength;
   len += pCdrFileHeader->privateExtensionLength;
   if (pCdrFileHeader->highExtensionPresent) len++;
   if (pCdrFileHeader->lowExtensionPresent) len++;
   return len;
}

int rtEncTS32297FileHdr(OSCTXT* pctxt, TS32297CDRFileHeader *pCdrFileHeader)
{
   int stat;

   /* The encoder should overwrite this at the end when the full length of
      the file is known.. */
   stat = rtxEncUInt32(pctxt, pCdrFileHeader->fileLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   /* Header length should be calculated, not provided by user */
   pCdrFileHeader->headerLength = rtTS32297CalcFileHdrLen (pCdrFileHeader);

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->headerLength, 4);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeRelVersId(pctxt, &pCdrFileHeader->highReleaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeRelVersId(pctxt, &pCdrFileHeader->lowReleaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeTimestamp(pctxt, &pCdrFileHeader->openTimestamp);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeTimestamp(pctxt, &pCdrFileHeader->lastAppendTimestamp);
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

   stat = rtxWriteBytes(pctxt, pCdrFileHeader->ipAddress, 20);
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

   stat = rtxEncUInt32(pctxt, pCdrFileHeader->privateExtensionLength, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   if (pCdrFileHeader->privateExtensionLength > 0)
   {
      stat = rtxWriteBytes(pctxt, pCdrFileHeader->privateExtension,
                           pCdrFileHeader->privateExtensionLength);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (pCdrFileHeader->highExtensionPresent)
   {
      stat = rtxEncUInt32(pctxt, pCdrFileHeader->highReleaseExtension, 1);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, stat);
      }
   }

   if (pCdrFileHeader->lowExtensionPresent)
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
   OSOCTET dataFmtTsNumVal;
   int stat;

   stat = rtxEncUInt32(pctxt, pCdrHeader->cdrLength, 2);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   stat = encodeRelVersId (pctxt, &pCdrHeader->releaseVersionID);
   if (stat < 0)
   {
      return LOG_RTERR(pctxt, stat);
   }

   dataFmtTsNumVal = (pCdrHeader->dataRecordFormat << 5) | pCdrHeader->tsNumber;
   stat = rtxEncUInt32(pctxt, dataFmtTsNumVal, 1);
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
