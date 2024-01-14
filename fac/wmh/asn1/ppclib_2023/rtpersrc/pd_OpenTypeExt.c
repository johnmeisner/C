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

/***********************************************************************
 *
 *  Routine name: pd_OpenTypeExt
 *
 *  Description:  The following function decodes an ASN.1 open type
 *                extension.  This function will capture the entire
 *                encoded extension block including the optional bit
 *                mask and all encoded fields in an open type.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----------  --------------  --------------------------------------
 *  pctxt      OSCTXT*       pointer to context block structure
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----------  --------------  --------------------------------------
 *  stat        int             completion status of operation
 *  pOpenType   ASN1OpenType*   pointer to open type structure to
 *                              receive decoded data
 *
 *
 **********************************************************************/

EXTPERMETHOD int pd_OpenTypeExt
(OSCTXT* pctxt, const OSOCTET** object_p2, OSSIZE* pnumocts)
{
   OSBOOL   optbit;
   OSUINT32 bitcnt = 0, ui;
   int i, elemcnt = 0, stat;

   if (object_p2) *object_p2 = OSRTBUFPTR (pctxt);

   /* decode extension optional bits length */

   PU_NEWFIELD (pctxt, "ext opt bits len");

   stat = pd_SmallLength (pctxt, &bitcnt);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   /* Decode optional element bits.  Get count of actual number of      */
   /* elements encoded in message..                                     */

   PU_NEWFIELD (pctxt, "ext opt bits");

   for (ui = 0; ui < bitcnt; ui++) {
      stat = PD_BIT (pctxt, &optbit);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (optbit) elemcnt++;
   }

   PU_SETBITCOUNT (pctxt);

   /* Decode elements */

   PU_NEWFIELD (pctxt, "ext elements");

   for (i = 0; i < elemcnt; i++) {
      stat = pd_OpenType (pctxt, 0, 0);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   /* Calculate byte count */

   if (pnumocts)
      *pnumocts = (OSSIZE)(OSRTBUFPTR(pctxt) - *object_p2);

   return 0;
}
