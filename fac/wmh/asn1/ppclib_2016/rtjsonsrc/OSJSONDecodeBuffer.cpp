/*
 * Copyright (c) 2018-2018 Objective Systems, Inc.
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
#include "rtxsrc/OSRTFileInputStream.h"
#include "rtxsrc/OSRTMemoryInputStream.h"
#include "rtjsonsrc/OSJSONDecodeBuffer.h"

//////////////////////////////////////////
//                                      //
// OSJSONDecodeBuffer methods            //
//                                      //
//////////////////////////////////////////

EXTJSONMETHOD OSJSONDecodeBuffer::OSJSONDecodeBuffer (const char* jsonFile) :
   OSJSONMessageBuffer (JSONDecode), mpInputStream(0), mbOwnStream(FALSE)
{
   if (0 != jsonFile) {
      mpInputStream = new OSRTFileInputStream (getContext(), jsonFile);
      mbOwnStream = TRUE;
   }
}

EXTJSONMETHOD OSJSONDecodeBuffer::OSJSONDecodeBuffer (const OSOCTET* msgbuf, size_t bufsiz) :
   OSJSONMessageBuffer (JSONDecode), mpInputStream(0), mbOwnStream(FALSE)
{
   if (0 != msgbuf) {
      mpInputStream = new OSRTMemoryInputStream (getContext(), msgbuf, bufsiz);
      mbOwnStream = TRUE;
   }
}

EXTJSONMETHOD OSJSONDecodeBuffer::OSJSONDecodeBuffer (OSRTInputStream& inputStream) :
   OSJSONMessageBuffer (JSONDecode, inputStream.getContext())
{
   mpInputStream = &inputStream;
   mbOwnStream = FALSE;
}

EXTJSONMETHOD OSJSONDecodeBuffer::~OSJSONDecodeBuffer ()
{
   // Delete input stream object if this object created it.
   if (mbOwnStream)
      delete mpInputStream;
}

EXTJSONMETHOD int OSJSONDecodeBuffer::init ()
{
   return 0;
}


