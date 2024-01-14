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

#include <string.h>
#include "rtxsrc/rtxCharStr.h"
#include "OSRTStringTokenizer.h"

OSRTStringTokenizer::OSRTStringTokenizer (const char* str, const char* delims) :
   mTokenListIter (mTokenList)
{
   init (str, strlen(str), delims);
}

OSRTStringTokenizer::OSRTStringTokenizer
(const char* str, size_t len, const char* delims) :
   mTokenListIter (mTokenList)
{
   init (str, len, delims);
}

void OSRTStringTokenizer::init (const char* str, size_t len, const char* delims)
{
   mpWorkBuffer = new char [len + 1];
   rtxStrncpy (mpWorkBuffer, len + 1, str, len);
   mpWorkBuffer[len] = '\0';
   const char* tok = strtok (mpWorkBuffer, delims);
   while (0 != tok) {
      mTokenList.append ((void*)tok);
      tok = strtok (0, delims);
   }
   mTokenListIter.setFirst();
}

OSRTStringTokenizer::~OSRTStringTokenizer ()
{
   delete [] mpWorkBuffer;
}

OSBOOL OSRTStringTokenizer::containsToken (const char* token)
{
   void* tok;
   ForAllOSRTVoidPtrListIter (tok, mTokenListIter) {
      if (!strcmp ((const char*)tok, token))
         return TRUE;
   }
   return FALSE;
}

size_t OSRTStringTokenizer::countTokens ()
{
   return mTokenList.count();
}

int OSRTStringTokenizer::hasMoreTokens ()
{
   return !mTokenListIter.isDone();
}

const char* OSRTStringTokenizer::nextToken ()
{
   const char* tok = (const char*) mTokenListIter.getCurrentItem();
   mTokenListIter.setNext();
   return tok;
}

