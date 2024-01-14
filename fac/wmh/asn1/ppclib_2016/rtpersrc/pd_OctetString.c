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

#include "rtpersrc/pu_common.hh"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_OctetString
 *
 *  Description:  The following function decodes an octet string value
 *                into a static buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  numocts     int*    pointer to variable to receive decoded number of octets
 *  buffer      octet*  pointer to buffer to receive decoded octet string data
 *  bufsiz      int     size of the data buffer
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_OctetString64
(OSCTXT* pctxt, OSSIZE* pnumocts, OSOCTET* buffer, OSSIZE bufsiz)
{
   OSSIZE octcnt, octidx = 0, numocts = 0;
   int lstat, stat;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pd_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   for (;;) {
      lstat = pd_Length64 (pctxt, &octcnt);
      if (lstat < 0) return LOG_RTERR (pctxt, lstat);

      PU_NEWFIELD (pctxt, "octets");

      if (octcnt > 0) {
         OSSIZE bitcnt = octcnt * 8;
         if (octcnt > bitcnt) return LOG_RTERR (pctxt, RTERR_TOOBIG);

         numocts += octcnt;

         if (pctxt->buffer.aligned) {
            if (pu_BitAndOctetStringAlignmentTest (&sizeCnst, octcnt, FALSE)) {
               stat = PD_BYTE_ALIGN (pctxt);
               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
         }

         if (0 != buffer) {
            stat = pd_octets (pctxt, &buffer[octidx],
                              bufsiz - octidx, bitcnt);
         }
         else {
            if (bitcnt <= OSINT32_MAX)
               stat = pd_moveBitCursor (pctxt, (int)bitcnt);
            else
               return LOG_RTERR (pctxt, RTERR_TOOBIG);
         }
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      PU_SETBITCOUNT (pctxt);

      if (lstat == RT_OK_FRAG) {
         octidx += octcnt;
      }
      else break;
   }

   if (0 != pnumocts) *pnumocts = numocts;

   LCHECKPER (pctxt);

   return 0;
}

EXTPERMETHOD int pd_OctetString
(OSCTXT* pctxt, OSUINT32* pnumocts, OSOCTET* buffer, OSUINT32 bufsiz)
{
   OSSIZE len;
   int ret = pd_OctetString64 (pctxt, &len, buffer, bufsiz);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   /* note: it is not necessary to check for buffer overflow because
      bufsiz can never be larger than OSUINT32_MAX due to type */

   if (0 != pnumocts) *pnumocts = (OSUINT32) len;

   return 0;
}
