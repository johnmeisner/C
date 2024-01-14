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

#ifndef RT3GPPTS32297_H
#define RT3GPPTS32297_H

#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/rtxContext.h"

/**
 * 3GPP TS 32.297 release/version identifier structure
 */
typedef struct {
   OSUINT8 relId;
   OSUINT8 versId;
} TS32297RelVersId;

/**
 * 3GPP TS 32.297 timestamp structure
 */
typedef struct {
   OSUINT8 month;
   OSUINT8 day;
   OSUINT8 hour;
   OSUINT8 minute;
   OSUINT8 zhour;
   OSUINT8 zminute;
   OSBOOL zdiff_plus;
} TS32297Timestamp;

/**
 * Structure containing 3GPP TS 32.297 File Header format
 */
typedef struct _TS32297CDRFileHeader
{
   OSUINT32 fileLength;
   OSUINT32 headerLength;

   TS32297RelVersId highReleaseVersionID;
   TS32297RelVersId lowReleaseVersionID;

   TS32297Timestamp openTimestamp;
   TS32297Timestamp lastAppendTimestamp;

   OSUINT32 cdrCount;

   OSUINT32 fileSequenceNumber;
   OSUINT8 fileClosureReason;

   OSOCTET ipAddress[20];

   OSUINT8 lostCDRIndicator;

   OSUINT16 cdrRoutingFilterLength;
   OSOCTET * cdrRoutingFilter;

   OSUINT16 privateExtensionLength;
   OSOCTET * privateExtension;

   OSBOOL highExtensionPresent;
   OSUINT8 highReleaseExtension;

   OSBOOL lowExtensionPresent;
   OSUINT8 lowReleaseExtension;
} TS32297CDRFileHeader;


/**
 * Structure containing 3GPP TS 32.297 Message Header format
 */
typedef struct _TS32297CDRHeader
{
   OSUINT16 cdrLength;
   TS32297RelVersId releaseVersionID;
   enum {Unk, BER, UPER, PER, XER} dataRecordFormat;
   OSOCTET tsNumber;
   OSBOOL releaseExtensionPresent;
   OSUINT8 releaseExtension;
} TS32297CDRHeader;


#ifdef __cplusplus
extern "C" {
#endif


/**
 * These functions initialize header structures with default values.
 * They should be called before first use of any encode/decode functions
 */
EXTERNRT void rtInitTS32297FileHdr(TS32297CDRFileHeader *pCdrFileHeader);
EXTERNRT void rtInitTS32297Hdr(TS32297CDRHeader *pCdrHeader);

/**
 * Decode/encode functions for the 3GPP TS 32.297 message and file Headers
 */

EXTERNRT int rtDecTS32297FileHdr(OSCTXT* pctxt,
                                 TS32297CDRFileHeader *pCdrFileHeader);
EXTERNRT int rtDecTS32297Hdr(OSCTXT* pctxt, TS32297CDRHeader *pCdrHeader);

EXTERNRT int rtEncTS32297FileHdr(OSCTXT* pctxt,
                                 TS32297CDRFileHeader *pCdrFileHeader);
EXTERNRT int rtEncTS32297Hdr(OSCTXT* pctxt, TS32297CDRHeader *pCdrHeader);

EXTERNRT int rtParseTS32297RelVersText
(const char* text, TS32297RelVersId* pRelVersId);

/**
 * This function parses timestamp text and converts it into numeric components
 * within the given timestamp structure.
 *
 * @param text Textual timestamp. Format is mm/dd hh:mm+hh:mm.
 * @param pTimestamp Pointer to timestamp structure.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtParseTS32297TimestampText
(const char* text, TS32297Timestamp* pTimestamp);

/**
 * This function converts a textual TS number into the binary identifier to
 * be used in the header.
 *
 * @param text Textual TS number (for example, "32.205")
 * @param pNumber Number to be used in header.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtParseTS32297TSNumberText (const char* text, OSUINT8* pNumber);

/**
 * This function calculates the length of a TS 32.297 file header.
 *
 * @param pCdrFileHeader Pointer to file header structure.
 * @return Calculated length.
 */
EXTERNRT OSUINT32 rtTS32297CalcFileHdrLen
(const TS32297CDRFileHeader *pCdrFileHeader);

/**
 * This function converts a TS 32.297 release/version ID structure into
 * a release number.
 *
 * @param pId Pointer to release/version ID structure.
 * @param relIdExt Release ID extension.
 * @param pRelNum Pointer to variable to receive release number.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtTS32297GetReleaseNumber
(const TS32297RelVersId* pId, OSUINT8 relIdExt, OSUINT8* pRelNum);

/**
 * This function converts a TS 32.297 release/version ID structure into text.
 *
 * @param pId Pointer to release/version ID structure.
 * @param relIdExt Release ID extension.
 * @param pStr Text buffer to receive converted text.
 * @param strSize Size of the text buffer.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtTS32297RelVersIdToText
(const TS32297RelVersId* pId, OSUINT8 relIdExt, char *pStr, size_t strSize);

/**
 * This function converts a TS 32.297 timestamp structure into text.
 *
 * @param pTimestamp Pointer to timestamp structure.
 * @param textbuf Text buffer to receive converted text.
 * @param textbufsz Size of the text buffer.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtTS32297TimestampToText
(const TS32297Timestamp* pTimestamp, char* textbuf, OSSIZE textbufsz);

/**
 * This function converts a TS 32.297 TS number octet into text.
 *
 * @param tsNumber Octet containing TS number as described in TS 32.297.
 * @param textbuf Text buffer to receive converted text.
 * @param textbufsz Size of the text buffer.
 * @return Zero for success or a negative error code.
 */
EXTERNRT int rtTS32297TSNumberToText
(OSOCTET tsNumber, char *pStr, size_t strSize);

/**
 * Print-to-stream functions for the 3GPP TS 32.297 message and file Headers
 */
EXTERNRT int rtPrtToStrmTS32297FileHdr(OSCTXT *pctxt,
                                       TS32297CDRFileHeader *pCdrFileHeader);
EXTERNRT int rtPrtToStrmTS32297Hdr(OSCTXT *pctxt, TS32297CDRHeader *pCdrHeader);

#ifdef __cplusplus
}
#endif

#endif
