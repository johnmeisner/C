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

#include "rtxsrc/OSRTStringConst.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"

bool OSRTStringConst::equals
(const char* value, size_t nchars, bool caseSensitive)
{
   if (0 == mpString || 0 == value) return false;

   if (0 == nchars) {
      int ret = (caseSensitive) ?
         strcmp (value, mpString) : rtxStricmp (value, mpString);
      return (bool)(0 == ret);
   }
   else if (nchars <= OSCRTLSTRLEN(mpString)) {
      for (size_t i = 0; i < nchars; i++) {
         if (caseSensitive) {
            if (value[i] != mpString[i]) return false;
         }
         else {
            if (OS_TOLOWER(value[i]) != OS_TOLOWER(mpString[i])) return false;
         }
      }
      return true;
   }

   return false;
}

