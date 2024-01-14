/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtjsonsrc/OSJSONEncodeStream.h"

//////////////////////////////////////////
//                                      //
// OSJSONEncodeStream methods            //
//                                      //
//////////////////////////////////////////

EXTJSONMETHOD
OSJSONEncodeStream::OSJSONEncodeStream (OSRTOutputStream& outputStream) :
   OSJSONMessageBuffer (JSONEncode, outputStream.getContext()),
   mpStream (&outputStream), mbOwnStream (FALSE)
{
   OSRTASSERT (0 != mpStream);
   mpCtxt = getCtxtPtr();
}

EXTJSONMETHOD OSJSONEncodeStream::OSJSONEncodeStream
(OSRTOutputStream* pOutputStream, OSBOOL ownStream) :
   OSJSONMessageBuffer (JSONEncode, pOutputStream->getContext ()),
   mpStream (pOutputStream), mbOwnStream (ownStream)
{
   OSRTASSERT (0 != pOutputStream);
   mpCtxt = getCtxtPtr();
}

EXTJSONMETHOD OSJSONEncodeStream::~OSJSONEncodeStream ()
{
   if (mbOwnStream) {
      mpStream->close ();
      delete mpStream;
   }
}

// This method resets the encode stream to allow another message to be
// encoded..

EXTJSONMETHOD int OSJSONEncodeStream::init ()
{
   mpCtxt->buffer.byteIndex = 0;
   //mpCtxt->state = OSXMLINIT;
   return 0;
}

// Encode JSON textual content

EXTJSONMETHOD int OSJSONEncodeStream::encodeText (const OSUTF8CHAR* value)
{
   return rtJsonEncStringValue (mpCtxt, value);
}


