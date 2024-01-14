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

#include "rtpersrc/pd_common.hh"

/***********************************************************************
 *
 *  Routine name: pd_ChoiceOpenTypeExt
 *
 *  Description:  The following function decodes an ASN.1 open type
 *                extension field in a choice construct.  This function
 *                will capture the entire encoded extension block
 *                including the choice index value and all encoded
 *                fields in an open type.
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

EXTPERMETHOD int pd_ChoiceOpenTypeExt
(OSCTXT* pctxt, const OSOCTET** object_p2, OSSIZE* pnumocts)
{
   ASN1OpenType openType;
   OSUINT32 idx = 0;
   int stat;

   *object_p2 = OSRTBUFPTR (pctxt);

   /* Decode choice index value */

   PU_NEWFIELD (pctxt, "choice index");

   stat = pd_SmallNonNegWholeNumber (pctxt, &idx);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   /* Decode element */

   PU_NEWFIELD (pctxt, "choice element");

   stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   /* Calculate byte count */

   *pnumocts = (OSSIZE)(OSRTBUFPTR(pctxt) - *object_p2);

   return 0;
}
