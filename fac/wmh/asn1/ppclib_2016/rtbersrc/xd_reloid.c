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

/***********************************************************************
 *
 *  Routine name: xd_reloid
 *
 *  Description:  This routine decodes the object at the current
 *                message pointer location and returns its value.  The
 *                value is returned in the "ASNOBJID" structure.  This
 *                structure contains a value for the number of subident-
 *                and a pointer to an array of integers which contain the
 *                subidentifer values.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, the object identifer contents are decoded
 *                and stored on the "ASNOBJID" structure.
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
 *  status      int	Status of operation.  Returned as function result.
 *  object      struct  Decoded object identifier value.
 *  length      int     Actual length of message component.
 *
 **********************************************************************/

int xd_reloid (OSCTXT *pctxt, ASN1OBJID *object_p, ASN1TagType tagging,
              int length)
{
   int status, j;
   OSOCTET b;

   if (tagging == ASN1EXPL) {
      if (!XD_MATCH1 (pctxt, ASN_ID_RELOID)) {
         return berErrUnexpTag (pctxt, ASN_ID_RELOID);
      }
      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   /* Check length */

   if (length > 0) {
      status = XD_CHKDEFLEN (pctxt, length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   j = 0;
   while (length > 0)
   {
      if (j < ASN_K_MAXSUBIDS) {

         /* Parse a subidentifier out of the contents field */

         object_p->subid[j] = 0;
         do {
            b = XD_FETCH1 (pctxt);
            object_p->subid[j] = (object_p->subid[j] * 128) + (b & 0x7F);
            length--;
         } while (b & 0x80);

         j++;
      }
      else
         return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   }

   if (length != 0) return LOG_RTERR (pctxt, ASN_E_INVLEN);

   object_p->numids = j;
   return 0;
}
