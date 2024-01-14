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
/**
 * @file OSRTStringConst.h
 * C++ string constant class.
 */
#ifndef _OSRTSTRINGCONST_H_
#define _OSRTSTRINGCONST_H_

#include <stddef.h>
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxExternDefs.h"

class EXTRTCLASS OSRTStringConst {
 protected:
   const char* mpString;

 public:
   OSRTStringConst (const char* str) : mpString(str) {}
   OSRTStringConst (const OSUTF8CHAR* str) : mpString((const char*)str) {}

   bool equals (const char* value, size_t nchars = 0,
                bool caseSensitive = true);
} ;

#endif
