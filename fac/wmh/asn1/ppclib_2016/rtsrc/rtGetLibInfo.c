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

#include "rtsrc/asn1version.h"
#include "rtsrc/rtGetLibInfo.h"
#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/osSysTypes.h"

EXTRTMETHOD int rtxGetLibVersion (void)
{
   return OS_ASN1RT_VERSION;
}

EXTRTMETHOD const char* rtxGetLibInfo (void)
{
   return "ASN1RT, v."OS_ASN1RT_VERSION_STR
#ifdef _DEBUG
   ", debug"
#endif
#ifdef _COMPACT
   ", opt, compact"
#elif defined (_OPTIMIZED)
   ", opt"
#endif
#ifndef _NO_LICENSE_CHECK
   ", limited"
#endif
#ifdef _TRACE
   ", trace"
#endif
#ifdef _NO_STREAM
   ", no stream"
#endif
#ifdef _16BIT
   ", 16 bit"
#endif
#ifdef _NO_INT64_SUPPORT
   ", no int64 support"
#endif
   ".";
}

EXTRTMETHOD OSBOOL rtxIsLimited (void)
{
#ifndef _NO_LICENSE_CHECK
   return TRUE;
#else
   return FALSE;
#endif
}
