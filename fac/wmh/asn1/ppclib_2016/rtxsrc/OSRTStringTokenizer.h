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

#ifndef _OSSTRINGTOKENIZER_H_
#define _OSSTRINGTOKENIZER_H_

#include "OSRTVoidPtrList.h"

// string tokenizer object - this is patterned off of the Java
// StringTokenizer class..

class OSRTStringTokenizer {
 protected:
   char* mpWorkBuffer;
   OSRTVoidPtrList mTokenList;       // list of const char*
   OSRTVoidPtrListIter mTokenListIter;

   void init (const char* str, size_t len, const char* delims);

 public:
   OSRTStringTokenizer (const char* str, const char* delims);
   OSRTStringTokenizer (const char* str, size_t len, const char* delims);
   ~OSRTStringTokenizer ();
   OSBOOL containsToken (const char* token);
   size_t countTokens ();
   int hasMoreTokens ();
   const char* nextToken ();
} ;

#endif

