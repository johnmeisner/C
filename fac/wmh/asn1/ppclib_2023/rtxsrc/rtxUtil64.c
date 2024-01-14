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

#include <stdlib.h>
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxUtil.h"
#include "rtxsrc/rtxError.h"

#ifndef _NO_INT64_SUPPORT

/*
 * This method returns the number of bytes required to encode an
 * identifier value.
 */
EXTRTMETHOD OSUINT32 rtxGetIdent64ByteCount (OSUINT64 ident)
{
   if (ident < (((OSUINT64)1) << 7)) {         /* 7 */
      return 1;
   }
   else if (ident < (((OSUINT64)1) << 14)) {   /* 14 */
      return 2;
   }
   else if (ident < (((OSUINT64)1) << 21)) {   /* 21 */
      return 3;
   }
   else if (ident < (((OSUINT64)1) << 28)) {   /* 28 */
      return 4;
   }
   else if (ident < (((OSUINT64)1) << 35)) {   /* 35 */
      return 5;
   }
   else if (ident < (((OSUINT64)1) << 42)) {   /* 42 */
      return 6;
   }
   else if (ident < (((OSUINT64)1) << 49)) {	 /* 49 */
      return 7;
   }
   else if (ident < (((OSUINT64)1) << 56)) {   /* 56 */
      return 8;
   }
   else if (ident <= ((((OSUINT64)1) << 63) - 1)) { /* 63 */
      return 9;
   }
   else {
      return 10;
   }
}

EXTRTMETHOD OSUINT32 rtxUInt64BitLen (OSUINT64 value)
{
   OSUINT32 bits = rtxUInt32BitLen
      ((OSUINT32)(value >> (sizeof (OSUINT32) * 8)));

   if (bits == 0)
      bits = rtxUInt32BitLen ((OSUINT32)(value & OSUINT32_MAX));
   else
      bits += sizeof (OSUINT32) * 8;

   return bits;
}

EXTRTMETHOD int rtxEncIdent64(OSUINT64 ident, OSOCTET* buffer, OSSIZE bufsize)
{
   OSUINT32 lv;
   OSSIZE   idx = 0;

   OSUINT64 mask = 0x7f;
   int nshifts, retval;

   /* Find starting point in identifier value.
      rtxGetIdent64ByteCount never returns 0.
   */
   nshifts = retval = (int)rtxGetIdent64ByteCount (ident);
   mask <<= (7 * nshifts);

   /* Encode bytes */

   while (nshifts > 0) {
      mask = ((OSUINT64)0x7f) << (7 * (nshifts - 1));
      nshifts--;
      lv = (OSUINT32)((ident & mask) >> (nshifts * 7));
      if (nshifts != 0) { lv |= 0x80; }

      if (idx < bufsize) {
         buffer[idx++] = (OSOCTET) lv;
      }
      else return RTERR_BUFOVFLW;
   }

   return retval;
}

#endif /* _NO_INT64_SUPPORT */


