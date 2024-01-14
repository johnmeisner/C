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

#include "rtAvn.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxReal.h"


EXTRTMETHOD int rtAvnWriteReal(OSCTXT* pctxt, OSREAL value)
{
   int ret = 0;

   if (rtxIsPlusInfinity(value))
   {
      ret = rtxCopyAsciiText(pctxt, "PLUS-INFINITY");
   }
   else if (rtxIsMinusInfinity(value))
   {
      ret = rtxCopyAsciiText(pctxt, "MINUS-INFINITY");
   }
   else if (rtxIsNaN(value))
   {
      ret = rtxCopyAsciiText(pctxt, "NOT-A-NUMBER");
   }
   else if (rtxIsMinusZero(value))
   {
      ret = rtxCopyAsciiText(pctxt, "-0");
   }
   else {
      char text[80];
      os_snprintf(text, sizeof(text), "%g", value);
      ret = rtxCopyAsciiText(pctxt ,text);
   }

   if ( ret < 0 ) return LOG_RTERR(pctxt, ret);

   return 0;
}
