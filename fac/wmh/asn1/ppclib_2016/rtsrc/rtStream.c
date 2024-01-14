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
/*
 * This file is deprecated and maintained for backward compatibility only.
 * Definitions in rtxsrc/rtxStream.h should be used for any new
 * development.
 */
#ifndef _NO_STREAM
#include "rtStream.h"

EXTRTMETHOD int rtStreamBufRead (ASN1CTXT* pctxt, ASN1OCTET* pdata, size_t size)
{
   int ll = rtxStreamRead (pctxt, pdata, size);
   return (ll < 0) ? ll : 0;
}
#endif
