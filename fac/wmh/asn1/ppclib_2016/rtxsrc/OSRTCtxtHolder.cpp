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
/**
 * @file OSRTCtxtHolder.h
 * C++ run-time message buffer interface class definition.
 */
#include "rtxsrc/OSRTCtxtHolder.h"

EXTRTMETHOD OSRTCtxtHolder::OSRTCtxtHolder (OSRTContext* pContext) :
   mpContext (pContext)
{
   if (pContext == 0)
      mpContext = new OSRTContext;
}

EXTRTMETHOD OSRTCtxtPtr OSRTCtxtHolder::getContext ()
{
   return mpContext;
}

EXTRTMETHOD OSCTXT* OSRTCtxtHolder::getCtxtPtr ()
{
   return mpContext->getPtr();
}

EXTRTMETHOD char* OSRTCtxtHolder::getErrorInfo ()
{
   return mpContext->getErrorInfo ();
}

EXTRTMETHOD char* OSRTCtxtHolder::getErrorInfo (char* pBuf, size_t& bufSize)
{
   return mpContext->getErrorInfo (pBuf, bufSize);
}

EXTRTMETHOD int OSRTCtxtHolder::getStatus () const
{
   return mpContext->getStatus ();
}

EXTRTMETHOD void OSRTCtxtHolder::printErrorInfo ()
{
   rtxErrPrint (getCtxtPtr());
}

EXTRTMETHOD void OSRTCtxtHolder::resetErrorInfo ()
{
   rtxErrReset (getCtxtPtr());
}

