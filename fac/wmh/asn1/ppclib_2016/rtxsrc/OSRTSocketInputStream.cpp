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

#ifndef _OS_NOSOCKET

#include "rtxsrc/rtxStreamSocket.h"
#include "rtxsrc/OSRTSocketInputStream.h"

EXTRTMETHOD OSRTSocketInputStream::OSRTSocketInputStream (OSRTSocket& socket):
   mSocket (socket)
{
   if (!isInitialized ()) return;

   mStatus = mInitStatus =
      rtxStreamSocketAttach (getCtxtPtr(), mSocket.getSocket(),
                             OSRTSTRMF_INPUT);
   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr (), mInitStatus);
}

EXTRTMETHOD OSRTSocketInputStream::OSRTSocketInputStream
   (OSRTContext* pContext, OSRTSocket& socket) :
   OSRTInputStream (pContext), mSocket (socket)
{
   if (!isInitialized()) return;

   mStatus = mInitStatus =
      rtxStreamSocketAttach (getCtxtPtr(), mSocket.getSocket (),
                             OSRTSTRMF_INPUT);
   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr(), mInitStatus);
}

EXTRTMETHOD OSRTSocketInputStream::OSRTSocketInputStream
(OSRTSOCKET socket, OSBOOL ownership): mSocket (socket, ownership)
{
   if (!isInitialized ()) return;

   mStatus = mInitStatus = rtxStreamSocketAttach
      (getCtxtPtr(), mSocket.getSocket (), OSRTSTRMF_INPUT);
   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr(), mInitStatus);
}

OSRTSocketInputStream::OSRTSocketInputStream
   (OSRTContext* pContext, OSRTSOCKET socket, OSBOOL ownership) :
   OSRTInputStream (pContext), mSocket (socket, ownership)
{
   if (!isInitialized ()) return;

   mStatus = mInitStatus = rtxStreamSocketAttach
      (getCtxtPtr(), mSocket.getSocket (), OSRTSTRMF_INPUT);
   if (mInitStatus != 0) LOG_RTERR (getCtxtPtr(), mInitStatus);
}

#endif /* _OS_NOSOCKET */
