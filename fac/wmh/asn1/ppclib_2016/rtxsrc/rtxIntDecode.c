/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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

#include "rtxsrc/rtxBitDecode.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxIntDecode.h"

/* Decode an 16-bit signed integer */

EXTRTMETHOD int rtxDecInt16 (OSCTXT* pctxt, OSINT16* pvalue, OSSIZE nbytes)
{
   int stat;
   OSOCTET tmpbuf[sizeof(OSINT32)];
   OSSIZE  bufidx = 0;
   OSOCTET ub;     /* unsigned */

   /* Make sure integer will fit in target variable */

   if (nbytes > sizeof(tmpbuf)) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      stat = rtxReadBytes (pctxt, tmpbuf, nbytes);
   }
   else {
      stat = rtxDecBitsToByteArray (pctxt, tmpbuf, sizeof(tmpbuf), nbytes*8);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* If target variable not provided, skip contents */

   if (0 == pvalue) { return 0; }

   /* Copy first byte into a signed char variable and assign it to */
   /* object.  This should handle sign extension in the case of a  */
   /* negative number..                                            */

   *pvalue = (signed char) tmpbuf[bufidx++];

   /* Now use unsigned bytes to add in the rest of the integer */

   for ( ; bufidx < nbytes; bufidx++) {
      ub = tmpbuf[bufidx];
      *pvalue = (*pvalue * 256) + ub;
   }

   return 0;
}

/* Decode an 32-bit signed integer */

EXTRTMETHOD int rtxDecInt32 (OSCTXT* pctxt, OSINT32* pvalue, OSSIZE nbytes)
{
   int stat;
   OSOCTET tmpbuf[sizeof(OSINT32)];
   OSSIZE  bufidx = 0;
   OSOCTET ub;     /* unsigned */

   /* Make sure integer will fit in target variable */

   if (nbytes > sizeof(tmpbuf)) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      stat = rtxReadBytes (pctxt, tmpbuf, nbytes);
   }
   else {
      stat = rtxDecBitsToByteArray (pctxt, tmpbuf, sizeof(tmpbuf), nbytes*8);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* If target variable not provided, skip contents */

   if (0 == pvalue) { return 0; }

   /* Copy first byte into a signed char variable and assign it to */
   /* object.  This should handle sign extension in the case of a  */
   /* negative number..                                            */

   *pvalue = (signed char) tmpbuf[bufidx++];

   /* Now use unsigned bytes to add in the rest of the integer */

   for ( ; bufidx < nbytes; bufidx++) {
      ub = tmpbuf[bufidx];
      *pvalue = (*pvalue * 256) + ub;
   }

   return 0;
}

/* Decode an 16-bit unsigned integer */

EXTRTMETHOD int rtxDecUInt16 (OSCTXT* pctxt, OSUINT16* pvalue, OSSIZE nbytes)
{
   int stat;
   OSOCTET tmpbuf[sizeof(OSUINT16)+1];
   OSSIZE bufidx = 0;

   /* Make sure integer will fit in target variable */

   if (nbytes > (sizeof(tmpbuf)))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Read encoded integer contents into memory */

   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      stat = rtxReadBytes (pctxt, tmpbuf, nbytes);
   }
   else {
      stat = rtxDecBitsToByteArray (pctxt, tmpbuf, sizeof(tmpbuf), nbytes*8);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      if (nbytes == (sizeof(tmpbuf))) {
         /* first byte must be zero */
         if (0 != tmpbuf[0])
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         else
            bufidx++;
      }

      for (*pvalue = 0; bufidx < nbytes; bufidx++) {
         *pvalue = (*pvalue * 256) + tmpbuf[bufidx];
      }
   }

   return 0;
}

/* Decode an 32-bit unsigned integer */

EXTRTMETHOD int rtxDecUInt32 (OSCTXT* pctxt, OSUINT32* pvalue, OSSIZE nbytes)
{
   int stat;
   OSOCTET tmpbuf[sizeof(OSUINT32)+1];
   OSSIZE bufidx = 0;

   /* Make sure integer will fit in target variable */

   if (nbytes > (sizeof(tmpbuf)))
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   /* Read encoded integer contents into memory */

   if (pctxt->buffer.bitOffset == 8 || pctxt->buffer.bitOffset == 0) {
      stat = rtxReadBytes (pctxt, tmpbuf, nbytes);
   }
   else {
      stat = rtxDecBitsToByteArray (pctxt, tmpbuf, sizeof(tmpbuf), nbytes*8);
   }
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (0 != pvalue) {
      if (nbytes == (sizeof(tmpbuf))) {
         /* first byte must be zero */
         if (0 != tmpbuf[0])
            return LOG_RTERR (pctxt, RTERR_TOOBIG);
         else
            bufidx++;
      }

      for (*pvalue = 0; bufidx < nbytes; bufidx++) {
         *pvalue = (*pvalue * 256) + tmpbuf[bufidx];
      }
   }

   return 0;
}

