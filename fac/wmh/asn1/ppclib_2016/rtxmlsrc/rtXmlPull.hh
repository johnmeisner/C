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

#ifndef __RTXMLPULL_HH
#define __RTXMLPULL_HH

#include "rtxmlsrc/rtXmlPull.h"
#include "rtxmlsrc/rtXmlErrCodes.h"

/* Uncomment to turn on extended tracing in the pull parser functions */
#define XMLPTRACE

#define MAX_MEMBLOCK_SIZE 2048

typedef struct {
   OSXMLStrFragment     mQName;
   OSXMLStrFragment     mLocalName;
   OSXMLStrFragment     mPrefix;
   OSXMLStrFragment     mValue;
} OSXMLAttribute;

typedef struct {
   OSXMLStrFragOffset   mQName;
   OSXMLStrFragOffset   mLocalName;
   OSXMLStrFragOffset   mPrefix;
   OSXMLStrFragOffset   mValue;
   OSINT16              index;
} OSXMLAttrOffset;

typedef struct OSXMLStrFragStack {
   OSXMLStrFragment*    mpStackArray;
   size_t               mSize;
   size_t               mCount;
} OSXMLStrFragStack;

typedef struct OSXMLNamespace_ {
   OSXMLStrFragment     mPrefix;
   OSUTF8CHAR           prefixBuff[8];
   /*OSXMLStrFragment     mURI;*/
   OSINT16              index;
   int                  mLevel;
#ifdef OSXMLNS12
   const OSUTF8CHAR*    urn;
#else
   const char*          urn;
#endif
} OSXMLNamespace_;

typedef struct OSXMLNamespacesStack {
   OSXMLNamespace_*     mpStackArray;
   size_t               mSize;
   size_t               mCount;
} OSXMLNamespacesStack;

/* These are all defined locally in rtXmlPull.c ..
#define rtXmlRdGetOffset(pReader) \
((pReader->mMarkedPos != (size_t)-1)? \
pReader->mByteIndex - pReader->mMarkedPos : (size_t)-1)

#define rtXmlRdGetOffsetPtr(pReader,offset) \
((pReader->mMarkedPos != (size_t)-1)? \
pReader->mpBuffer + pReader->mMarkedPos + offset : 0)

#define rtXmlRdCancelMark(pReader) pReader->mMarkedPos = (size_t)-1;
#define rtXmlRdIsMarked(pReader) (pReader->mMarkedPos != (size_t)-1 && pReader->mMarkedPos < pReader->mReadSize)

#define READ_SYMBOL(pReader) (pReader->mByteIndex < pReader->mReadSize) ? \
pReader->mpBuffer [pReader->mByteIndex++] : \
((rtXmlRdPreReadFromStream (pReader) == 0) ? pReader->mpBuffer [pReader->mByteIndex++] : -1)

#define PEEK_SYMBOL(pReader) (pReader->mByteIndex < pReader->mReadSize) ? \
pReader->mpBuffer [pReader->mByteIndex] : \
((rtXmlRdPreReadFromStream (pReader) == 0) ? pReader->mpBuffer [pReader->mByteIndex] : -1)

#define ADVANCE_PTR(pReader) (pReader->mByteIndex++)
#define RETROGRESS_PTR(pReader) (pReader->mByteIndex--)

#define NEED_PREREAD(pReader) (pReader->mByteIndex >= pReader->mReadSize)
*/

const OSXMLAttrOffset* rtXmlRdGetAttribute
   (struct OSXMLReader* pReader, size_t index);

#endif /* __RTXMLPULL_HH */
