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
 *  Routine name: xd_oid64
 *
 *  Description:  The following function decodes an object identifier
 *                value.  This object identifier uses the C ASN1OID64
 *                type that can hold 64-bit arc values.
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

int xd_oid64 (OSCTXT *pctxt, ASN1OID64 *object_p, ASN1TagType tagging,
              int length)
{
   register int	cnt, j, stat;
   OSUINT64	subid;
   OSOCTET	b;

   if (tagging == ASN1EXPL) {
      if (!XD_PEEKTAG (pctxt, ASN_ID_OBJID))
         return LOG_RTERR(pctxt, RTERR_IDNOTFOU);

      stat = xd_Tag1AndLen (pctxt, &length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Check length */

   if (length > 0) {
      stat = XD_CHKDEFLEN (pctxt, length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   j = 0;
   while (length > 0)
   {
      if (j < ASN_K_MAXSUBIDS) {

         /* Parse a subidentifier out of the contents field */

         object_p->subid[j] = 0;
         cnt = 0;
         do {
            b = XD_FETCH1 (pctxt);
            object_p->subid[j] = (object_p->subid[j] * 128) + (b & 0x7F);
            length--; cnt++;
         } while (b & 0x80 && cnt < 16 && length > 0);

         /* If extension bit is set in last parsed octet, it is an error */

         if ((b & 0x80) != 0) return LOG_RTERR (pctxt, ASN_E_INVOBJID);

         /* If number of bytes parsed is greater than 16, then something   */
         /* is wrong with the arc value..                                  */

         if (cnt >= 16) return LOG_RTERR (pctxt, ASN_E_INVOBJID);

         /* Handle the first subidentifier special case: the first two 	   */
         /* sub-id's are encoded into one using the formula (x * 40) + y   */

         if (j == 0) {
            subid = object_p->subid[0];
            object_p->subid[0] = ((subid / 40) >= 2) ? 2 : subid / 40;
            object_p->subid[1] = (object_p->subid[0] == 2) ?
               subid - 80 : subid % 40;
            j = 2;
         }
         else j++;
      }
      else
         return LOG_RTERR (pctxt, ASN_E_INVOBJID);
   }

   if (length != 0)
      return LOG_RTERR (pctxt, ASN_E_INVLEN);

   object_p->numids = j;
   return 0;
}
