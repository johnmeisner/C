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

#include "rtpersrc/asn1per.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: pd_DynOctetString
 *
 *  Description:  The following function decodes an octet string value
 *                into a dynamic buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  octstr      struct* pointer to structure to receive decoded value
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

EXTPERMETHOD int pd_DynOctetString64 (OSCTXT* pctxt, OSDynOctStr64* pOctStr)
{
   OSSIZE octcnt;
   int stat = 0;
   /* Save size constraint for alignment test.  Version in context will be
      consumed by pd_Length. */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      OSOCTET* p = 0;
      OSSIZE sz = 0;
      OSBOOL fragment = TRUE;

      do {
         stat = pd_Length64 (pctxt, &octcnt);

         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
         else if (stat != RT_OK_FRAG)
            fragment = FALSE;

         PU_NEWFIELD (pctxt, "octets");

         if (octcnt > 0) {
            if (pctxt->buffer.aligned) {
               if (pu_BitAndOctetStringAlignmentTest
                   (&sizeCnst, octcnt, FALSE)) {
                  stat = PD_BYTE_ALIGN (pctxt);
                  if (stat != 0) return LOG_RTERR (pctxt, stat);
               }
            }

            if (0 != pOctStr) {
               OSOCTET* ptm;
               OSSIZE newSize = sz + octcnt;
               if (octcnt > newSize) return LOG_RTERR (pctxt, RTERR_TOOBIG);

               ptm = (OSOCTET*) rtxMemRealloc (pctxt, p, newSize);
               if (0 == ptm)
                  return LOG_RTERR_AND_FREE_MEM (pctxt, RTERR_NOMEM, p);

               p = ptm;

               if (octcnt < OSSIZE_MAX/8) {
                  stat = rtxDecBitsToByteArray
                     (pctxt, p + sz, octcnt, octcnt * 8);
               }
               else stat = RTERR_TOOBIG;
            }
            else {
               stat = rtxSkipBytes (pctxt, octcnt);
            }

            if (stat != 0)
               return LOG_RTERR_AND_FREE_MEM (pctxt, stat, p);

            sz += octcnt;
         }

         PU_SETBITCOUNT (pctxt);
      }
      while (fragment);

      if (pOctStr) {
         pOctStr->numocts = sz;
         pOctStr->data = p;
      }
   }
   else
#endif /* _NO_STREAM */
   if (0 != pOctStr) {
      stat = pd_Length64 (pctxt, &octcnt);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      if (stat == RT_OK_FRAG) {
         /* fragment length always placed in 8 bits and
            there are no padding bits between length and data */

         stat = pd_moveBitCursor (pctxt, -8);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         stat = pd_GetComponentLength (pctxt, 8);

         if (stat < 0) return LOG_RTERR (pctxt, stat);
         else {
            /* Allocate memory for the target string */
            OSOCTET* ptmp = (OSOCTET*) rtxMemAlloc (pctxt, stat);
            if (0 == ptmp) return LOG_RTERR (pctxt, RTERR_NOMEM);

            /* Call static octet string decode function */
            stat = pd_OctetString64
               (pctxt, &pOctStr->numocts, ptmp, (OSSIZE)stat);
            if (stat != 0) return LOG_RTERR_AND_FREE_MEM (pctxt, stat, ptmp);

            pOctStr->data = ptmp;
         }
      }
      else {
         PU_NEWFIELD (pctxt, "octets");

         pOctStr->numocts = octcnt;

         if (octcnt > 0) {
            if (pctxt->buffer.aligned) {
               if (pu_BitAndOctetStringAlignmentTest
                   (&sizeCnst, octcnt, FALSE)) {
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
               pOctStr->data = ASN1BUFPTR (pctxt);

               stat = rtxSkipBytes (pctxt, octcnt);

               if (stat != 0) return LOG_RTERR (pctxt, stat);
            }
            else {
               if (octcnt < OSSIZE_MAX/8) {
                  OSOCTET* ptmp = (OSOCTET*) rtxMemAlloc (pctxt, octcnt);
                  if (0 == ptmp) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  stat = rtxDecBitsToByteArray
                     (pctxt, ptmp, octcnt, octcnt * 8);

                  if (stat != 0)
                     return LOG_RTERR_AND_FREE_MEM (pctxt, stat, ptmp);

                  pOctStr->data = ptmp;
               }
               else return LOG_RTERR (pctxt, RTERR_TOOBIG);
            }
         }
         else
            pOctStr->data = 0;

         PU_SETBITCOUNT (pctxt);
      }
   }
   else {
      /* Call pd_OctetString with no buffer to skip field */
      stat = pd_OctetString64 (pctxt, 0, 0, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   LCHECKPER (pctxt);

   return stat;
}

EXTPERMETHOD int pd_DynOctetString (OSCTXT* pctxt, ASN1DynOctStr* pOctStr)
{
   if (0 != pOctStr) {
      OSDynOctStr64 octstr;
      int ret = pd_DynOctetString64 (pctxt, &octstr);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
      else if (sizeof(OSSIZE) > 4 && octstr.numocts > OSUINT32_MAX) {
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      }
      pOctStr->numocts = (OSUINT32) octstr.numocts;
      pOctStr->data = octstr.data;
      return 0;
   }
   else return pd_OctetString64 (pctxt, 0, 0, 0);
}
