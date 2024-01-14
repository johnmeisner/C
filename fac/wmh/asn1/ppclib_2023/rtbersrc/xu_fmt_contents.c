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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.h"
#include "rtxsrc/rtxHexDump.h"
#include "rtxsrc/rtxMemory.h"

/***********************************************************************
 *
 *  Routine name: xu_fmt_contents
 *
 *  Description:  This routine formats the contents field at the current
 *  ASN.1 decode pointer for display.  Data is returned in a dynamic
 *  buffer with display segments delimitted with null terminators.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  len         int     length of contents
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  count       int*    count of display segments
 *  bufp        char*   pointer to dynamic display buffer (returned as
 *                      function result)
 *
 **********************************************************************/

#define NUM_SEGMENT_BYTES  12   /* # of bytes to display in a segment */

char *xu_fmt_contents (OSCTXT* pctxt, int len, int *count)
{
   int       segmentLen  = (NUM_SEGMENT_BYTES * 4) + 1;
   int       numSegments = ((len - 1))/NUM_SEGMENT_BYTES + 1;
   register int i, j, offset, num_bytes;
   char      *bufp, *hexp, *ascp, buf[3];
   OSOCTET   b = 0;

   *count = 0;

   bufp = (char *) rtxMemAlloc (pctxt, numSegments * (segmentLen+1));
   if (0 == bufp) return ((char*)0);

   for (i = 0, offset = 0; i < numSegments;
        i++, offset += (segmentLen+1), len -= NUM_SEGMENT_BYTES)
   {
      memset (&bufp[offset], ' ', segmentLen);
      bufp[offset+segmentLen] = '\0';

      hexp = &bufp[offset];
      ascp = &bufp[offset+(NUM_SEGMENT_BYTES*3+1)];

      num_bytes = OSRTMIN (len, NUM_SEGMENT_BYTES);

      for (j = 0; j < num_bytes; j++) {
         if (XD_MEMCPY1 (pctxt, &b) == 0) {
            rtxByteToHexChar (b, buf, sizeof(buf));
            *hexp++ = buf[0];
            *hexp++ = buf[1];
            *hexp++ = ' ';
            *ascp++ = (b > 31 && b < 128) ? b : '.';
         }
         else break;
      }
   }

   *count = i;

   return (bufp);
}
