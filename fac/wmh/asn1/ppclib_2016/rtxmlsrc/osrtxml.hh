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

#ifndef _OSRTXML_HH_
#define _OSRTXML_HH_

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxErrCodes.h"

extern const signed char decodeTable[];

#define BASE64TOINT(c) ((((unsigned)(c)) < 128) ? decodeTable [(c) - 40] : -1)

#define DEFAULT_FLOAT_PRECISION  6
#define DEFAULT_DOUBLE_PRECISION 11

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions internal to XML run-time */

int rtXmlFreeCtxtAppInfo (OSCTXT* pctxt);
int rtXmlResetCtxtAppInfo (OSCTXT* pctxt);
OSXMLCtxtInfo* rtXmlCtxtAppInfoDup (OSCTXT* pctxt, OSCTXT* pDstCtxt);
void rtXmlSetCtxtAppInfo (OSCTXT* pctxt, OSXMLCtxtInfo* pXMLInfo);
int rtXmlUpdateBOM (OSXMLCtxtInfo* pXMLInfo, OSXMLEncoding encoding);
int rtXmlSetEncoding (OSCTXT* pctxt, OSXMLEncoding encoding);

int rtXmlEncStartAttrC14N (OSCTXT* pctxt);
int rtXmlEncEndAttrC14N (OSCTXT* pctxt);

int rtXmlEncParseAny (OSCTXT* pctxt, OSCTXT* parseCtxt,
                      const OSUTF8CHAR* pvalue, size_t valueLen,
                      OSBOOL anyType);

int rtXmlSetSchemaLocationByStrFrag
(OSCTXT* pctxt, const OSUTF8CHAR* schemaLocation, size_t nbytes);

int rtXmlSetNoNSSchemaLocationByStrFrag
(OSCTXT* pctxt, const OSUTF8CHAR* schemaLocation, size_t nbytes);

void rtXmlpFreeReader (OSCTXT* pctxt, OSXMLCtxtInfo* pXMLInfo);

/* not used functions */
EXTERNXML OSBOOL rtXmlpIsInGroup
   (int elemID, int grpId, const OSBOOL* grpTab, int nElems);

EXTERNXML void rtXmlpGetContent (OSCTXT* pctxt, int level);

EXTERNXML OSBOOL rtXmlpMatchElemId (OSCTXT* pctxt, int elemID, int matchingID);

EXTERNXML OSBOOL rtXmlpIsContentMode (OSCTXT* pctxt);

#ifdef __cplusplus
}
#endif

#ifdef RTEVAL
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif

#endif
