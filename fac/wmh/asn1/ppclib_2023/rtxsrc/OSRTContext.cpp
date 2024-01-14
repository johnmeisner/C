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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/OSRTContext.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDiag.h"

// OSRTContext methods

EXTRTMETHOD OSRTContext::OSRTContext () : mCount(0), mStatus (0)
{
   int stat = rtxInitContext (&mCtxt);
   if (stat != 0)
      mbInitialized = FALSE;
   else
      mbInitialized = TRUE;
}

EXTRTMETHOD OSRTContext::OSRTContext (OSCTXT* pctxt)
   : mCount(0), mbInitialized(TRUE), mStatus (0)
{
   OSCRTLSAFEMEMCPY (&mCtxt, sizeof(OSCTXT), pctxt, sizeof (OSCTXT));
}

EXTRTMETHOD OSRTContext::~OSRTContext ()
{
   if (mbInitialized)
      rtxFreeContext (&mCtxt);
}

EXTRTMETHOD void OSRTContext::_ref ()
{
   mCount++;
//   RTDIAG3 (&mCtxt, "context %x : ref count incremented, count = %d\n",
//            &mCtxt, mCount);
}

EXTRTMETHOD void OSRTContext::_unref()
{
   mCount--;

//   RTDIAG3 (&mCtxt, "context %x : ref count decremented, count = %d\n",
//            &mCtxt, mCount);

   if (mCount <= 0)
      delete this;
}

EXTRTMETHOD OSUINT32 OSRTContext::getRefCount()
{
   return mCount;
}

EXTRTMETHOD char* OSRTContext::getErrorInfo (size_t* pBufSize)
{
   size_t bufsiz;
   char* pBuf = rtxErrGetText (&mCtxt, 0, &bufsiz);
   char* newBuf = new char [bufsiz + 1];

   if (0 == newBuf) {
      if (0 != pBufSize)
         *pBufSize = (size_t)-1;
      return newBuf;
   }

   rtxStrncpy (newBuf, bufsiz + 1, pBuf, bufsiz);
   rtxMemFreePtr (&mCtxt, pBuf);
   if (pBufSize != 0) *pBufSize = bufsiz;
   return newBuf;
}

EXTRTMETHOD char* OSRTContext::getErrorInfo ()
{
   return getErrorInfo (0);
}

EXTRTMETHOD char* OSRTContext::getErrorInfo (char* pBuf, size_t& bufSize)
{
   if (pBuf == 0)
      return getErrorInfo (&bufSize);

   return rtxErrGetText (&mCtxt, pBuf, &bufSize);
}

EXTRTMETHOD int OSRTContext::setStatus (int stat)
{
   mStatus = stat;
   return stat;
}

EXTRTMETHOD int OSRTContext::setRunTimeKey (const OSOCTET* key, size_t keylen)
{
   if (!mbInitialized) return RTERR_NOTINIT;

   mCtxt.key = key;
   mCtxt.keylen = keylen;

   LCHECKX (&mCtxt);

   return 0;
}

void* operator new (size_t nbytes, OSCTXT* pctxt)
{
   return rtxMemAlloc (pctxt, nbytes);
}

void operator delete (void* pmem, OSCTXT* pctxt)
{
   rtxMemFreePtr (pctxt, pmem);
}

