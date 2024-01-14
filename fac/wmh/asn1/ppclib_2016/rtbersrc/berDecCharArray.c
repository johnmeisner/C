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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.hh"

/* Decode string into a fixed-size character array */

int berDecCharArray (OSCTXT* pctxt, char* charArray, OSSIZE arraySize,
                     ASN1TagType tagging, ASN1TAG tag, int length)
{
   int stat = 0;
   OSSIZE size, ll = 0;
   OSBOOL isConstructedTag;

   if (tagging == ASN1EXPL) {
      if ((stat = xd_match (pctxt, tag, &length, XM_ADVANCE)) < 0)
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }
   isConstructedTag = (OSBOOL)((pctxt->flags & ASN1CONSTAG) != 0);

   /* For an indefinite length message, need to get the actual 	*/
   /* length by parsing tags until the end of the message is 	*/
   /* reached..							*/
#ifndef _BUILD_LICOBJ
   if (length == ASN_K_INDEFLEN) {  /* indefinite length message */
      if ((stat = xd_consStrIndefLenAndSize (pctxt, TM_UNIV|ASN_ID_OCTSTR,
         &ll, &size)) != 0)
         return LOG_RTERR (pctxt, stat);
   }
   else
#endif
      ll = size = (OSSIZE) length;

   if (ll >= arraySize) {
      return LOG_RTERR (pctxt, RTERR_TOOBIG);
   }
   else {
      ll = size;

      stat = xd_octstr64_s
         (pctxt, (OSOCTET*)charArray, &ll, ASN1IMPL, size, FALSE);

      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   charArray[ll] = '\0';

   if (isConstructedTag && length == ASN_K_INDEFLEN) {
      if (XD_MATCHEOC (pctxt)) XD_BUMPIDX (pctxt, 2);
      else return LOG_RTERR (pctxt, ASN_E_INVLEN);
   }

   LCHECKBER (pctxt);

   return 0;
}
