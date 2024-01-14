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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/OSRTStream.h"

EXTRTMETHOD OSRTStream::OSRTStream () : mbAttached (FALSE)
{
   mStatus = mInitStatus = mCtxtHolder.getContext()->getStatus ();
   if (mInitStatus != 0) return;

   mStatus = mInitStatus = rtxStreamInit (mCtxtHolder.getContext()->getPtr ());
   if (mInitStatus < 0) return;
}

EXTRTMETHOD OSRTStream::OSRTStream
(OSRTContext* pContext, OSBOOL attachStream) :
   mCtxtHolder (pContext), mbAttached (attachStream)
{
   if (!mCtxtHolder.getContext()->isInitialized ()) {
      mStatus = mInitStatus = RTERR_NOTINIT;
      return;
   }

   if (!mbAttached) {
      mStatus = mInitStatus = rtxStreamInit (getCtxtPtr());
      if (mInitStatus < 0) return;
   }
}

EXTRTMETHOD OSRTStream::OSRTStream (OSRTStream& original) :
   OSRTStreamIF (original),
   mCtxtHolder (original.mCtxtHolder),
   mbAttached (original.mbAttached),
   mStatus (original.mStatus),
   mInitStatus (original.mInitStatus)
{
}

/*
 * Virtual destructor. Closes the stream if it was opened.
 */
EXTRTMETHOD OSRTStream::~OSRTStream ()
{
   if (!mbAttached && isOpened ())
      close ();
}

/*
 * Closes the input or output stream and releases any system resources
 * associated with the stream. For output streams this function also flushes
 * all internal buffers to the stream.
 *
 * @see       ::rtxStreamClose, ::rtxStreamBufClose
 */
EXTRTMETHOD int OSRTStream::close ()
{
   if (!isInitialized ()) return mInitStatus;

   mStatus = rtxStreamClose (mCtxtHolder.getCtxtPtr());
   if (mStatus < 0) return LOG_RTERR (mCtxtHolder.getCtxtPtr(), mStatus);

   return 0;
}

/*
 * Flushes the buffered data to the stream.
 *
 * @see       ::rtxStreamFlush, ::rtxStreamBufFlush
 */
EXTRTMETHOD int OSRTStream::flush ()
{
   if (!isInitialized ()) return mInitStatus;

   mStatus = rtxStreamFlush (mCtxtHolder.getCtxtPtr());
   if (mStatus < 0) return LOG_RTERR (mCtxtHolder.getCtxtPtr(), mStatus);
   return 0;
}

EXTRTMETHOD OSBOOL OSRTStream::isOpened ()
{
   return rtxStreamIsOpened (mCtxtHolder.getCtxtPtr());
}
