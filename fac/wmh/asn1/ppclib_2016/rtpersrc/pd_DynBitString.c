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

/***********************************************************************
 *
 *  Routine name: pd_DynBitString
 *
 *  Description:  The following function decodes a bit string value
 *                into a dynamic buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  bitstr      struct* pointer to structure to receive decoded value
 *  lbound      int     lower bound value of string size (0 if unconstrained)
 *  ubound      int     upper bound value of string size (MAX if unconstrained)
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_DynBitString64 (OSCTXT* pctxt, ASN1DynBitStr64* pBitStr)
{
   OSOCTET* ptmp;
   OSSIZE bitcnt;
   OSSIZE octcnt = 0;
   int stat = 0;
   OSBOOL fragment = TRUE;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pd_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      OSOCTET* p = 0;
      OSSIZE sz = 0;
      OSSIZE bits = 0;

      do {
         stat = pd_Length64 (pctxt, &bitcnt);

         if (stat != 0)
            return LOG_RTERR (pctxt, stat);
         else if (stat != RT_OK_FRAG)
            fragment = FALSE;

         PU_NEWFIELD (pctxt, "bits");

         if (bitcnt > 0) {
            octcnt = bitcnt / 8;
            if (bitcnt % 8 != 0) octcnt++;

            if (pctxt->buffer.aligned) {
               if (pu_BitAndOctetStringAlignmentTest
                   (&sizeCnst, bitcnt, TRUE)) {
                  stat = PD_BYTE_ALIGN (pctxt);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }
            if (octcnt < (OSSIZE_MAX - sz)) {
               ptmp = (OSOCTET*) rtxMemRealloc (pctxt, p, sz + octcnt);
               if (0 == ptmp)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, RTERR_NOMEM, p);

               p = ptmp;

               stat = rtxDecBitsToByteArray (pctxt, p + sz, octcnt, bitcnt);
            }
            else stat = RTERR_TOOBIG;

            if (stat != 0)
               return LOG_RTERR_AND_FREE_MEM (pctxt, stat, p);
         }

         PU_SETBITCOUNT (pctxt);

         sz += octcnt;
         bits += bitcnt;
      }
      while (fragment);

      pBitStr->numbits = bits;
      pBitStr->data = p;
   }
   else
#endif /* _NO_STREAM */
   {
      stat = pd_Length64 (pctxt, &bitcnt);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      octcnt = bitcnt / 8;
      if (bitcnt % 8 != 0) octcnt++;

      if (stat == RT_OK_FRAG) {
         /* fragment length always placed in 8 bits and
            it is no padding bits between length and data */

         stat = pd_moveBitCursor (pctxt, -8);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);

         stat = pd_GetComponentLength (pctxt, 1);

         if (stat < 0) return LOG_RTERR (pctxt, stat);
         else {
            /* Allocate memory for the target string */
            ptmp = (OSOCTET*) rtxMemAlloc (pctxt, (OSSIZE)stat);
            if (0 == ptmp) return LOG_RTERR (pctxt, RTERR_NOMEM);

            /* Call static octet string decode function */
            stat = pd_BitString64 (pctxt, &pBitStr->numbits, ptmp, stat);
            if (stat != 0) return LOG_RTERR_AND_FREE_MEM (pctxt, stat, ptmp);

            pBitStr->data = ptmp;
         }
      }
      else {
         PU_NEWFIELD (pctxt, "bits");

         pBitStr->numbits = bitcnt;

         if (bitcnt > 0) {
            if (pctxt->buffer.aligned) {
               if (pu_BitAndOctetStringAlignmentTest
                   (&sizeCnst, bitcnt, TRUE)) {
                  stat = PD_BYTE_ALIGN (pctxt);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }

#ifndef _NO_STREAM
            if (!pctxt->pStream && (pctxt->flags & ASN1FASTCOPY) &&
                pctxt->buffer.bitOffset == 8)
#else /* _NO_STREAM */
            if ((pctxt->flags & ASN1FASTCOPY) &&
                pctxt->buffer.bitOffset == 8)
#endif /* _NO_STREAM */
            {
               pBitStr->data = ASN1BUFPTR (pctxt);

               stat = pd_moveBitCursor (pctxt, bitcnt);
               if (stat != 0)
                  return LOG_RTERR (pctxt, stat);
            }
            else {
               ptmp = (OSOCTET*) rtxMemAlloc (pctxt, octcnt);
               if (0 == ptmp) return LOG_RTERR (pctxt, RTERR_NOMEM);

               stat = rtxDecBitsToByteArray (pctxt, ptmp, octcnt, bitcnt);
               if (stat != 0)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, stat, ptmp);

               pBitStr->numbits = bitcnt;
               pBitStr->data = ptmp;
            }
         }
         else
            pBitStr->data = 0;

         PU_SETBITCOUNT (pctxt);
      }
   }

   return stat;
}

EXTPERMETHOD int pd_DynBitString (OSCTXT* pctxt, ASN1DynBitStr* pBitStr)
{
   ASN1DynBitStr64 bitstr;
   int ret = pd_DynBitString64 (pctxt, &bitstr);
   if (0 != ret) return LOG_RTERR (pctxt, ret);
   else if (sizeof(OSSIZE) > 4 && bitstr.numbits > OSUINT32_MAX) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   pBitStr->numbits = (OSUINT32)bitstr.numbits;
   pBitStr->data = bitstr.data;

   return 0;
}
