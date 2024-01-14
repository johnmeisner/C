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


int xd_timestr(OSCTXT* pctxt, const char** object_p,
               ASN1TagType tagging, ASN1TAG tag, int length)
{
   const char *pdata;
   int stat = 0;
   ASN1Time tval;

   stat = xd_charstr(pctxt, &pdata, tagging, tag, (OSSIZE)length);
   if (0 != stat)
   {
      return stat;
   }

   /* Now validate string. */
   stat = rtParseTime(pctxt, pdata, &tval, TRUE);
   if ( stat == ASN_E_NOTCANON ) stat = 0;
   else if ( stat != 0 ) LOG_RTERR(pctxt, stat);

   if ( stat != 0 )
   {
      rtxMemFreePtr(pctxt, pdata);
   }

   *object_p = pdata;
   return stat;
}
