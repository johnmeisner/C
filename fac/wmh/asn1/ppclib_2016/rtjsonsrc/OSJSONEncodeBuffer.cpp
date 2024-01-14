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
#include "rtjsonsrc/OSJSONEncodeBuffer.h"
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxFile.h"

//////////////////////////////////////////
//                                      //
// OSJSONEncodeBuffer methods            //
//                                      //
//////////////////////////////////////////

EXTJSONMETHOD OSJSONEncodeBuffer::OSJSONEncodeBuffer () :
   OSJSONMessageBuffer(JSONEncode)
{
   if (getContext()->getStatus () != 0) return;

   int stat = rtxInitContext (getCtxtPtr());
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);
   getCtxtPtr()->indent = 3;

   stat = rtxInitContextBuffer (getCtxtPtr(), 0, 0);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);

   // LCHECKX (getCtxtPtr());
   getCtxtPtr()->buffer.data[0] = '\0';
   //getCtxtPtr()->state = OSXMLDATA; /* we're always in OSXMLDATA */
}

EXTJSONMETHOD OSJSONEncodeBuffer::OSJSONEncodeBuffer
(OSOCTET* pMsgBuf, size_t msgBufLen) :
   OSJSONMessageBuffer (JSONEncode)
{
   int stat = 0;

   if (getContext()->getStatus () != 0) return;

   getCtxtPtr()->indent = 3;

   stat = rtxInitContextBuffer (getCtxtPtr(), pMsgBuf, msgBufLen);
   if (stat != 0) LOG_RTERR (getCtxtPtr(), stat);

   // LCHECKX (getCtxtPtr());
   getCtxtPtr()->buffer.data[0] = '\0';
   //getCtxtPtr()->state = OSXMLDATA; /* we're always in OSXMLDATA */
}

// This method resets the encode buffer to allow another message to be
// encoded..

EXTJSONMETHOD int OSJSONEncodeBuffer::init ()
{
   OSCTXT* pctxt = getCtxtPtr();
   pctxt->buffer.byteIndex = 0;
   //pctxt->state = OSXMLDATA; /* we're always in OSXMLDATA */
   return 0;
}

// Write JSON document out to given filename
// returns: number of bytes written or negative status code if error

EXTJSONMETHOD long OSJSONEncodeBuffer::write (const char* filename)
{
   FILE* fp;
   long nbytes = rtxFileOpen (&fp, filename, "wb");
   if (0 == nbytes) {
      nbytes = write (fp);
      fclose (fp);
   }
   return nbytes;
}

// Write JSON document out to given file
// returns: number of bytes written or negative status code if error

EXTJSONMETHOD long OSJSONEncodeBuffer::write (FILE* fp)
{
   return (long)fwrite (getMsgPtr(), 1, getMsgLen(), fp);
}

