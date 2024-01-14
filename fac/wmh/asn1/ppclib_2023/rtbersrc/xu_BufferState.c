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

#include "rtbersrc/asn1ber.h"

/* Functions to save and restore buffer state */

void xu_SaveBufferState (OSCTXT* pCtxt, OSRTBufSave* pSavedInfo)
{
   if (!pSavedInfo) pSavedInfo = &pCtxt->savedInfo;
   pSavedInfo->byteIndex = pCtxt->buffer.byteIndex;
   pSavedInfo->bitOffset = pCtxt->buffer.bitOffset;
   pSavedInfo->flags = pCtxt->flags;
}

void xu_RestoreBufferState (OSCTXT* pCtxt, OSRTBufSave* pSavedInfo)
{
   if (!pSavedInfo) pSavedInfo = &pCtxt->savedInfo;
   pCtxt->buffer.byteIndex = pSavedInfo->byteIndex;
   pCtxt->buffer.bitOffset = pSavedInfo->bitOffset;
   pCtxt->flags = (pSavedInfo->flags & (~ASN1LASTEOC));
}
