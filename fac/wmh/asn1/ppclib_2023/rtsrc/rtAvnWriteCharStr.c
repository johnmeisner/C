/*
* Copyright (c) 1997-2023 Objective Systems, Inc.
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

EXTRTMETHOD int rtAvnWriteCharStr(OSCTXT* pctxt, const char* str)
{
   int ret = 0;
   const char* start = str;   /* start of bytes to write */
   const char* end = str;     /* end of bytes to write */

   OSRTSAFEPUTCHAR(pctxt, '"');

   for (;;)
   {
      /* Advance end up to double quote char or null terminator. */
      while (*end != 0 && *end != '"') end++;

      /* Copy bytes from start to end, including end if not null terminator. */
      if ( end > start )
      {
         OSSIZE bytes = end - start;
         if ( *end != 0 ) bytes += 1;

         ret = rtxWriteBytes(pctxt, (OSOCTET *)start, bytes);
         if ( ret != 0 ) return LOG_RTERR(pctxt, ret);
      }

      /* Copy double quote char; its the escape or the end of string. */
      OSRTSAFEPUTCHAR(pctxt, '"');

      if ( *end == 0 ) break;

      start = end = end + 1;
   }

   return 0;
}
