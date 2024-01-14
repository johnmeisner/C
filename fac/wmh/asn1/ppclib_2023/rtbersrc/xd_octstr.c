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
#include "rtsrc/asn1intl.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xd_octstr64
 *
 *  Description:  This routine decodes the octet string at the current
 *                message pointer location and returns its value.  This
 *                routine is identical to the xd_octstr_s above
 *                except for the fact that dynamic memory is allocated
 *                for the decoded string.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  tagging     bool    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object      char**	Pointer to decoded octet string value.
 *  numocts	int	Number of octets in string.
 *
 **********************************************************************/

int xd_octstr64 (OSCTXT* pctxt,
                 OSOCTET** object_p2, OSSIZE* pnumocts,
                 ASN1TagType tagging, OSSIZE length, OSBOOL indefLen)
{
   OSOCTET* poctstr = 0;
   int stat = 0;
   int isConstructedTag;

   if (tagging == ASN1EXPL) {
      stat = xd_match1_64 (pctxt, ASN_ID_OCTSTR, &length, &indefLen);
      if (stat < 0) {
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
      }
   }
   isConstructedTag = (int)((pctxt->flags & ASN1CONSTAG) != 0);

   if (isConstructedTag && rtxCtxtTestFlag(pctxt, ASN1DER))
   {
      /* DER requires primitive encoding form */
      LOG_RTERRNEW(pctxt, ASN_E_PRIM_REQ);
      rtxErrSetNonFatal(pctxt);
   }

   if (stat == 0) {
      OSSIZE size, ll = 0;

      /* For an indefinite length message, need to get the actual 	*/
      /* length by parsing tags until the end of the message is 	*/
      /* reached..							*/
#ifndef _BUILD_LICOBJ
      if (indefLen) {  /* indefinite length message */
         stat = xd_consStrIndefLenAndSize
            (pctxt, TM_UNIV|ASN_ID_OCTSTR, &ll, &size);

         if (0 != stat) return LOG_RTERR (pctxt, stat);
      }
      else
#endif
         ll = size = length;

      if (rtxCtxtTestFlag(pctxt, ASN1CANON))
      {
         if (ll <= 1000 && isConstructedTag)
         {
            /* CER requires strings of <= 1000 octets to use primitive form. */
            LOG_RTERRNEW(pctxt, ASN_E_PRIM_REQ);
            rtxErrSetNonFatal(pctxt);
         }
         else if (ll > 1000 && !isConstructedTag)
         {
            /* CER requires constructed form (fragmentation) when contents
            octets > 1000 */
            LOG_RTERRNEW(pctxt, ASN_E_FRAGMENTS);
            rtxErrSetNonFatal(pctxt);
         }
      }

      /* If "fast copy" option is not set (ASN1FATSCOPY) or if constructed,
       * copy the octet string value into a dynamic memory buffer;
       * otherwise, store the pointer to the value in the decode
       * buffer in the data pointer argument. */

      if (0 == ll) {
         *pnumocts = 0;
      }
      else if ((pctxt->flags & ASN1FASTCOPY) == 0 || isConstructedTag) {
         poctstr = (OSOCTET*) rtxMemAlloc (pctxt, ll);
         if (0 != poctstr) {
            *pnumocts = (OSSIZE) size;

            stat = xd_octstr64_s (pctxt, poctstr, pnumocts,
                                  ASN1IMPL, size, FALSE);

            if (stat != 0) {
               rtxMemFreePtr (pctxt, poctstr);
               return LOG_RTERR (pctxt, stat);
            }
         }
         else
            return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
      else {
         *pnumocts = size;
         poctstr = ASN1BUFPTR (pctxt);
         XD_BUMPIDX (pctxt, size);
      }
   }

   *object_p2 = poctstr;

   if (isConstructedTag && indefLen) {
      if (XD_MATCHEOC (pctxt)) XD_BUMPIDX (pctxt, 2);
      else return LOG_RTERR (pctxt, ASN_E_INVLEN);
   }

   LCHECKBER (pctxt);

   return 0;
}

int xd_octstr (OSCTXT* pctxt,
               const OSOCTET** object_p2, OSUINT32* pnumocts,
               ASN1TagType tagging, int length)
{
   OSOCTET* pdata;
   OSSIZE numocts = *pnumocts;
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   int ret = xd_octstr64
      (pctxt, &pdata, &numocts, tagging, (OSSIZE)length, indefLen);

   if (0 == ret) {
      if (sizeof(numocts) > 4 && numocts > OSUINT32_MAX)
         return (LOG_RTERR (pctxt, RTERR_TOOBIG));

      *pnumocts = (OSUINT32)numocts;
      *object_p2 = pdata;
   }

   return ret;
}
