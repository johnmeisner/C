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

#ifndef RT3GPPTS32297_H
#define RT3GPPTS32297_H

#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/rtxContext.h"

#define STRING_SIZE 20
/**
 * Structure containing 3GPP TS 32.297 File Header format
 */
typedef struct _TS32297CDRFileHeader
{
      OSUINT32 fileLength;
      OSUINT32 headerLength;

      char highReleaseVersionID[STRING_SIZE];
      char lowReleaseVersionID[STRING_SIZE];

      char openTimestamp[STRING_SIZE];
      char lastAppendTimestamp[STRING_SIZE];

      OSUINT32 cdrCount;

      OSUINT32 fileSequenceNumber;
      OSUINT8 fileClosureReason;

      OSOCTET ipAddress[STRING_SIZE];

      OSUINT8 lostCDRIndicator;

      OSUINT16 cdrRoutingFilterLength;
      OSOCTET * cdrRoutingFilter;

      OSUINT16 privateExtensionLength;
      OSOCTET * privateExtension;

      OSBOOL highExtentionPresent;
      OSUINT8 highReleaseExtension;

      OSBOOL lowExtentionPresent;
      OSUINT8 lowReleaseExtension;
} TS32297CDRFileHeader;


/**
 * Structure containing 3GPP TS 32.297 Message Header format
 */
typedef struct _TS32297CDRHeader
{
      OSUINT16 cdrLength;
      char releaseVersionID[STRING_SIZE];
      enum {BER, UPER, PER, XER} dataRecordFormat;
      char tsNumber[STRING_SIZE];
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
