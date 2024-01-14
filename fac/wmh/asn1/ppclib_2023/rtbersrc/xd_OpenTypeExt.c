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

/***********************************************************************
 *
 *  Routine name: xd_OpenTypeExt
 *
 *  Description:  This routine decodes an ASN.1 open type
 *                extension field.  This is an implicit field
 *                that is added a to a type when an extension
 *                marker (...) is present at the end of a message
 *                or when a pair of markers are present in the
 *                middle with no defined fields (..., ...).
 *
 *
 *  Inputs:
 *
 *  Name        Type                Description
 *  ----        ---------           -----------
 *  pctxt      OSCTXT*           Context block pointer
 *  ccb_p       ASN1CCB*            Context control block pointer
 *  tags         ASN1TAG*           Array of next expected tag values
 *                                  (null if last field)
 *  tagCount     int                The number of tags in the tags array.
 *
 *  Outputs:
 *
 *  Name        Type                Description
 *  ----        ---------           -----------
 *  status      int	            Status of operation.
 *                                  Returned as function result.
 *  pElemList   list*               Pointer to list to which decoded
 *                                  open type elements will be added.
 *
 **********************************************************************/

int xd_OpenTypeExt64 (OSCTXT* pctxt, const OSOCTET* consptr,
                      OSSIZE conslen, OSBOOL indefLen, ASN1TAG* tags,
                      OSSIZE tagCount, OSRTDList *pElemList)
{
   int stat;

   if (tags == 0) {
      /* Loop through remaining elements in SEQUENCE */
      while (!xd_chkend64 (pctxt, consptr, conslen, indefLen)) {
         stat = xd_OpenTypeAppend (pctxt, pElemList);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }
   else {
      /* Loop through elements until matching tag found or some other   */
      /* error occurs..                                                 */
      while (!xd_chkend64 (pctxt, consptr, conslen, indefLen)) {
         OSSIZE idx;
         for (idx = 0; idx < tagCount; idx++) {
            stat = xd_match64 (pctxt, tags[idx], 0, 0, 0);
            if ( stat == 0 ) {
               /*found expected tag*/
               return 0;
            }
            else if ( stat == RTERR_IDNOTFOU) {
               /*tag not found; reset error and look for next tag*/
               rtxErrReset (pctxt);
            }
            else return LOG_RTERR (pctxt, stat);
         }

         stat = xd_OpenTypeAppend (pctxt, pElemList);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
   }

   return 0;
}

int xd_OpenTypeExt (OSCTXT* pctxt, ASN1CCB* ccb_p, ASN1TAG* tags,
                    int tagCount, OSRTDList *pElemList)
{
   OSSIZE conslen;
   OSBOOL indefLen;
   OSSIZE tagCount2 = (OSSIZE)((tagCount < 0) ? 0 : tagCount);

   if (ccb_p->len == ASN_K_INDEFLEN) {
      conslen = OSSIZE_MAX;
      indefLen = TRUE;
   }
   else {
      conslen = (OSSIZE)ccb_p->len;
      indefLen = FALSE;
   }

   return xd_OpenTypeExt64
      (pctxt, ccb_p->ptr, conslen, indefLen, tags, tagCount2, pElemList);
}
