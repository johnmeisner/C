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

#include "saxParser.h"
#include "ASN1XERString.h"

// OSXMLBase

OSXMLBase::~OSXMLBase ()
{
}

// StrX
#if 0
StrX::StrX (const XMLCHAR* const toTranscode)
{
   fLocalForm = ASN1XERString::transcode (toTranscode);
}

StrX::~StrX()
{
   delete [] fLocalForm;
}
#endif

// Exception implementation

OSRTSAXExceptionImpl::OSRTSAXExceptionImpl() :
   fMsg(ASN1XERString::replicate(ASN1XERString::zeroLenString ())),
   stat (0), file (0), line (0)
{
}

#ifdef XML_UNICODE
/**
* Create a new OSRTSAXExceptionImpl.
*
* @param msg The error or warning message.
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(const XMLCHAR* const msg) :
   fMsg(ASN1XERString::replicate(msg)), stat (0), file (0), line (0)
{
}
#endif

/**
* Create a new OSRTSAXExceptionImpl.
*
* @param msg The error or warning message.
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(const char* const msg) :
   fMsg(ASN1XERString::transcode(msg)), stat (0), file (0), line (0)
{
}

#ifdef XML_UNICODE
/**
* Create a new OSRTSAXExceptionImpl.
*
* @param msg The error or warning message.
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(const XMLCHAR* const msg,
   int stat_, const XMLCHAR* const file_, int line_) :
   fMsg(ASN1XERString::replicate(msg)), stat (stat_),
   file (ASN1XERString::replicate(file_)), line (line_)
{
}
#endif

/**
* Create a new OSRTSAXExceptionImpl.
*
* @param msg The error or warning message.
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(const char* const msg,
   int stat_, const char* const file_, int line_) :
   fMsg(ASN1XERString::transcode(msg)), stat (stat_),
   file (ASN1XERString::transcode(file_)), line (line_)
{
}

/**
* Create a new OSRTSAXExceptionImpl.
*
* @param msg The error or warning message.
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(int stat_, const char* file_, int line_) :
   fMsg(0), stat (stat_), file (ASN1XERString::transcode(file_)), line (line_)
{
}

/**
* Copy constructor
*
* @param toCopy The exception to be copy constructed
*/
OSRTSAXExceptionImpl::OSRTSAXExceptionImpl(const OSRTSAXExceptionImpl& toCopy) :
   fMsg(ASN1XERString::replicate(toCopy.fMsg)),
   stat (toCopy.stat), file (ASN1XERString::replicate(toCopy.file)),
   line (toCopy.line)
{
}

/** Destructor */
OSRTSAXExceptionImpl::~OSRTSAXExceptionImpl()
{
   delete [] fMsg;
   delete [] file;
}

/** @name Public Operators */
/**
  * Assignment operator
  *
  * @param toCopy The object to be copied
  */
OSRTSAXExceptionImpl&
OSRTSAXExceptionImpl::operator=(const OSRTSAXExceptionImpl& toCopy)
{
   if (this == &toCopy)
      return *this;

   delete [] fMsg;
   delete [] file;
   fMsg = ASN1XERString::replicate(toCopy.fMsg);
   file = ASN1XERString::replicate(toCopy.file);
   stat = toCopy.stat;
   line = toCopy.line;
   return *this;
}

const XMLCHAR* OSRTSAXExceptionImpl::getMessage()
{
   return fMsg;
}

OSRTSAXException* createSAXException ()
{
   return new OSRTSAXExceptionImpl ();
}

#ifdef XML_UNICODE
OSRTSAXException* createSAXException (const XMLCHAR* const msg, int stat,
   const XMLCHAR* const fileName, int line)
{
   return new OSRTSAXExceptionImpl (msg, stat, fileName, line);
}
#endif

OSRTSAXException* createSAXException (const char* const msg, int stat,
   const char* const fileName, int line)
{
   return new OSRTSAXExceptionImpl (msg, stat, fileName, line);
}
