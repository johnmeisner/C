/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxsrc/rtxStreamMemory.h"
#include "rtxsrc/OSRTMemoryInputStream.h"

EXTRTMETHOD OSRTMemoryInputStream::OSRTMemoryInputStream
(const OSOCTET* pMemBuf, size_t bufSize)
{
   if (!isInitialized ()) return;

   mStatus = mInitStatus = rtxStreamMemoryAttach
      (getCtxtPtr(), (OSOCTET*)pMemBuf, bufSize, OSRTSTRMF_INPUT);

   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr(), mInitStatus);
}

EXTRTMETHOD OSRTMemoryInputStream::OSRTMemoryInputStream
(OSRTContext* pContext, const OSOCTET* pMemBuf, size_t bufSize) :
   OSRTInputStream (pContext)
{
   if (!isInitialized ()) return;

   mStatus = mInitStatus = rtxStreamMemoryAttach
      (getCtxtPtr(), (OSOCTET*)pMemBuf, bufSize, OSRTSTRMF_INPUT);

   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr(), mInitStatus);
}
