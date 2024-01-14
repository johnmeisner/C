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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxUTF8.h"

EXTRTMETHOD int rtxUTF8StrnToBool
(const OSUTF8CHAR* utf8str, size_t nbytes, OSBOOL* pvalue)
{
   int stat = rtxUTF8RemoveWhiteSpace (utf8str, nbytes, &utf8str);
   if (stat < 0) return stat; else nbytes = stat;

   if ((nbytes == 4 && rtxUTF8StrnEqual (utf8str, OSUTF8("true"), 4)) ||
       (nbytes == 1 && *utf8str == '1')) {
      *pvalue = TRUE;
   }
   else
   if ((nbytes == 5 && rtxUTF8StrnEqual (utf8str, OSUTF8("false"), 5)) ||
       (nbytes == 1 && *utf8str == '0')) {
      *pvalue = FALSE;
   }
   else return RTERR_BADVALUE;

   return 0;
}

EXTRTMETHOD int rtxUTF8StrToBool (const OSUTF8CHAR* utf8str, OSBOOL* pvalue)
{
   return rtxUTF8StrnToBool (utf8str, rtxUTF8LenBytes(utf8str), pvalue);
}
