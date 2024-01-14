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

#include "rtpersrc/pe_common.hh"

/***********************************************************************
 *
 *  Routine name: pe_ChoiceTypeExt
 *
 *  Description:  The following function encodes an ASN.1 open type
 *                extension in a choice construct.  The given type is
 *                assumed to contain a complete extension encoding
 *                including the optional bit mask and all fields.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----------  --------------  --------------------------------------
 *  pctxt      OSCTXT*       pointer to context block structure
 *  pOpenType   ASN1OpenType*   pointer to open type structure to
 *                              encode
 *
 *  Outputs:
 *
 *  Name        Type            Description
 *  ----------  --------------  --------------------------------------
 *  stat        int             completion status of encode operation
 *
 *
 **********************************************************************/

EXTPERMETHOD int pe_ChoiceTypeExt (OSCTXT* pctxt,
                      OSUINT32 numocts,
                      const OSOCTET* data)
{
   int stat;

   PU_NEWFIELD (pctxt, "OpenTypeExt");

   if (numocts > 0) {
      if (pctxt->buffer.aligned) {
         stat = pe_byte_align (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      stat = pe_octets (pctxt, data, numocts * 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   PU_SETBITCOUNT (pctxt);

   return 0;
}

