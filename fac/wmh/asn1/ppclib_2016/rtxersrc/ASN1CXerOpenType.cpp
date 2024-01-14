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

#include "rtxersrc/ASN1CXerOpenType.h"

ASN1CXerOpenType::ASN1CXerOpenType (ASN1TOpenType& data) :
   mEncodeBuffer(TRUE), ASN1CType(), msgData(data)
{
   mpEncCtxt = mEncodeBuffer.getCtxtPtr();
}

ASN1CXerOpenType::ASN1CXerOpenType
(OSRTMessageBufferIF& msgBuf, ASN1TOpenType& data) :
   mEncodeBuffer (TRUE, msgBuf.getContext(), 1024),
   ASN1CType (msgBuf), msgData (data)
{
   mpEncCtxt = mEncodeBuffer.getCtxtPtr();
}

ASN1CXerOpenType::ASN1CXerOpenType (ASN1TObject& data) :
   mEncodeBuffer(TRUE), ASN1CType(), msgData((ASN1TOpenType&)data.encoded)
{
   mpEncCtxt = mEncodeBuffer.getCtxtPtr();
}

ASN1CXerOpenType::ASN1CXerOpenType
(OSRTMessageBufferIF& msgBuf, ASN1TObject& data) :
   mEncodeBuffer (TRUE, msgBuf.getContext(), 1024),
   ASN1CType(msgBuf), msgData((ASN1TOpenType&)data.encoded)
{
   mpEncCtxt = mEncodeBuffer.getCtxtPtr();
}

ASN1CXerOpenType::~ASN1CXerOpenType ()
{
   mpEncCtxt->buffer.data = 0; // to prevent from freeing
   mpEncCtxt->buffer.size = 0;
}

int ASN1CXerOpenType::startElement
(const OSUTF8CHAR* const uri, const OSUTF8CHAR* const localname,
 const OSUTF8CHAR* const qname, const OSUTF8CHAR* const* attrs)
{
   int stat = 0;
   mCurrState = XERSTART;
   mpEncCtxt->state = (OSOCTET)mCurrState;

   // Encode start element

   stat = xerCopyText (mpEncCtxt, "<");
   if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

   stat = xerEncXmlCharData (mpEncCtxt, qname, xerTextLength (qname));

   if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

   // TODO: need to add attributes if present

   stat = xerCopyText (mpEncCtxt, ">");
   if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

   // Bump level

   mLevel++;

   return 0;
}

int ASN1CXerOpenType::characters
(const OSUTF8CHAR* const chars, const unsigned int length)
{
   RTDIAGSTRM2 (NULL,"ASN1CXerOpenType::characters: chars =\n");
   RTHEXDUMPSTRM (NULL,(OSOCTET*)chars, (length*2));

   int stat = xerEncXmlCharData (mpEncCtxt, chars, length);
   if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

   mCurrState = XERDATA;

   return 0;
}

int ASN1CXerOpenType::endElement
(const OSUTF8CHAR* const uri, const OSUTF8CHAR* const localname,
 const OSUTF8CHAR* const qname)
{
   int stat = 0;
   mpEncCtxt->state = XERSTART;

   if (!isComplete ()) {
      // Encode end element

      if (!isEmptyElement (qname)) {
         stat = xerCopyText (mpEncCtxt, "</");
         if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

         stat = xerEncXmlCharData (mpEncCtxt, qname, xerTextLength(qname));

         if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }

         stat = xerCopyText (mpEncCtxt, ">");
         if (stat != 0) { return LOG_RTERR (mpEncCtxt, stat); }
      }

      // Update mem link and open type data variable

      const OSOCTET* data = mEncodeBuffer.getMsgPtr();

      // Move ownership of memory from mEncodeBuffer to decoding context,
      // so that encode buffer is not deleted when mEncodeBuffer is deleted.
      // This will contain the open type data, therefore, it will be owned
      // by the decoding context
      /*
      rtxMemMoveBlock (&mEncodeBuffer.getCtxtPtr()->pMemLink, data,
         &getCtxtPtr()->pMemLink, FALSE);
      */
      msgData.data = data;
      msgData.numocts = (OSUINT32)mEncodeBuffer.getMsgLen();

      // If last element, set state to XEREND

      if (--mLevel == 0) mCurrState = XEREND;
   }

   return 0;
}

// Check encode buffer for empty element.  This method will modify the
// buffer contents if element is found to be empty by replacing last
// '>' character with '/>'.

OSBOOL ASN1CXerOpenType::isEmptyElement (const OSUTF8CHAR* const qname)
{
   mpEncCtxt->buffer.byteIndex--;
   char* bufp = (char*) ASN1BUFPTR (mpEncCtxt);
   if (*bufp == '>') { // no character data
      *bufp-- = '\0';  // null out last '>' character
      while (*bufp != '<') {
         bufp--;
      }
      bufp++;
      if (xerCmpText (qname, bufp)) {
         // empty element
         xerCopyText (mpEncCtxt, "/>"); // append null element indicator
         return TRUE;
      }
      else
         xerCopyText (mpEncCtxt, ">");  // put back original '>'
   }
   else
      mpEncCtxt->buffer.byteIndex++;    // restore byte index

   return FALSE;
}
