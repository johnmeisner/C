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

#include "xee_common.hh"
#if OS_ASN1RT_VERSION >= 600
#include "rtxsrc/rtxBuffer.h"
#endif

int xerCopyText (OSCTXT* pctxt, const char* text)
{
   size_t nchars;
   if (0 != text) {
      nchars = strlen (text);
      if (nchars > 0) {
         return rtxWriteBytes (pctxt, (const OSOCTET*)text, nchars);
      }
   }
   return 0;
}

int xerCopyText2 (OSCTXT* pctxt,
                  const char* text1,
                  const char* text2)
{
   size_t nchars;
   int stat = 0;

   if (0 != text1) {
      nchars = strlen(text1);
      if (nchars > 0) {
         stat = rtxWriteBytes (pctxt, (const OSOCTET*)text1, nchars);
      }
   }

   if (0 != text1 && 0 == stat) {
      nchars = strlen(text2);
      if (nchars > 0) {
         stat = rtxWriteBytes (pctxt, (const OSOCTET*)text2, nchars);
      }
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}
