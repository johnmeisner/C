/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

/**
 * @file xee_common.hh
 */

/* Internal definitions for XER encoding */
#ifndef _XEE_COMMON_HH_
#define _XEE_COMMON_HH_

#include "rtxersrc/asn1xer.h"
#include "rtsrc/asn1intl.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxCtype.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern short ASN1_daysInMonth[];

int xerCopyText2 (OSCTXT* pctxt,
                  const char* text1,
                  const char* text2);

int xerPutCharStr (OSCTXT* pctxt, const char* value, size_t len);
int xerEncChar (OSCTXT* pctxt, OS32BITCHAR wc);

int xerEncBinStrValue (OSCTXT* pctxt,
                       OSSIZE nbits,
                       const OSOCTET* data);

int xerEncHexStrValue (OSCTXT* pctxt,
                       OSSIZE nocts,
                       const OSOCTET* data);

#ifdef __cplusplus
}
#endif

#endif /* _XEE_COMMON_HH_ */
