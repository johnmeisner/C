/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include <stdarg.h>
#include <stdlib.h>

#include "rtsrc/asn1ErrCodes.h"
#include "rtsrc/rtContext.h"
#include "rtxsrc/rtxError.h"

#if !defined(_COMPACT) && !defined(__SYMBIAN32__)

/* Error status text */
static const char* g_status_text[] = {
    "Invalid object identifier",                         /* base */
    "Invalid field length detected",                     /* -1 */
    "Invalid tag value",                                 /* -2 */
    "Invalid binary string value",                       /* -3 */
    "Invalid index for table constraint identifier",     /* -4 */
    "Invalid value for relational table constraint fixed type field",   /* -5 */
    "List error: concurrent modification attempt while iterating",/* -6 */
    "List error: illegal state for attempted operation", /* -7 */
    "Attempt to invoke encode or decode method on a non-PDU type",/* -8 */
    "Element type could not be resolved",                /* -9 */
    "PER encoding for element does not match configured value",   /* -10 */
    "Element with tag %s not defined in SEQUENCE",       /* -11 */
    "Encoding has bad alignment",                        /* -12 */
    "PDU type for message could not be determined",      /* -13 */
    "Encoding is not canonical: %s",                     /* -14 */
    "Value is not valid for the given type",             /* -15 */
    "Length form is invalid for given encoding rules",   /* -16 */
    "Length is not encoded with minimal octets",         /* -17 */
    "Given encoding rules require primitive form",       /* -18 */
    "Value is not correctly fragmented as required by encoding rules" /* -19 */
} ;

void rtErrASN1Init ()
{
   rtxErrAddErrorTableEntry (g_status_text, ASN_E_BASE, (ASN_E_BASE-99));
}
#else
void rtErrASN1Init () {}
#endif
