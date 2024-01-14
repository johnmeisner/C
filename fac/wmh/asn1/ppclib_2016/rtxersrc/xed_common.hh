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
 * @file xed_common.hh
 */

/* Internal definitions for XER decoding */
#ifndef _XED_COMMON_HH_
#define _XED_COMMON_HH_

#include "rtxersrc/asn1xer.h"
#include "rtsrc/asn1intl.h"
#include "rtsrc/rt_common.hh"
#include "rtxsrc/rtxCtype.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASN1ENDBUFPTR(cp)          &((cp)->buffer.data[(cp)->buffer.size])
#define ASN1SIZEINCHARS(cp,tp)     ((cp)->buffer.size/sizeof(tp))

//extern short ASN1_daysInMonth[];

int xerDecBase64StrValue (OSCTXT* pctxt,
                          OSOCTET* pvalue, OSUINT32* pnocts,
                          size_t bufSize, size_t srcDataSize);
int xerDecBase64StrValue64 (OSCTXT* pctxt,
                            OSOCTET* pvalue, OSSIZE* pnocts,
                            OSSIZE bufSize, OSSIZE srcDataSize);
int xerDecBinStrValue (OSCTXT* pctxt, OSOCTET* pvalue,
                       OSUINT32* pnbits, size_t bufsize,
                       OSBOOL skipWhitespaces);
int xerDecBinStrValue64 (OSCTXT* pctxt, OSOCTET* pvalue,
                         OSSIZE* pnbits, OSSIZE bufsize,
                         OSBOOL skipWhitespaces);
int xerDecHexStrValue (OSCTXT* pctxt, OSOCTET* pvalue,
                       OSUINT32* pnbits, size_t bufsize,
                       OSUINT32 nsemiocts, OSBOOL skipWhitespaces);
int xerDecHexStrValue64 (OSCTXT* pctxt, OSOCTET* pvalue,
                         OSSIZE* pnbits, OSSIZE bufsize,
                         OSSIZE nsemiocts, OSBOOL skipWhitespaces);
long xerGetBase64StrDecodedLen (OSCTXT* pctxt, size_t srcDataSize);
int xerParseToken (OSCTXT* pCtxt, char** ppToken);
int xerReadChar (OSCTXT* pCtxt, OSBOOL skipWhiteSpace);

char* xmlTimeSkipComponents (char* p, int num);

int xerInitCtxtAppInfo (OSCTXT* pctxt);
int xerFreeCtxtAppInfo (OSCTXT* pctxt);
int xerResetCtxtAppInfo (OSCTXT* pctxt);

#ifdef __cplusplus
}
#endif

#endif /* _XED_COMMON_HH_ */
