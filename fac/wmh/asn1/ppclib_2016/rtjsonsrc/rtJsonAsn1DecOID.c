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

#include "asn1json.h"
#include "rtxsrc/rtxCtype.h"

int rtJsonAsn1DecOIDValue (OSCTXT *pctxt, ASN1OBJID *pOID)
{
   int stat;
   OSUINT32 curId = 0;
   OSOCTET ub;
   OSUTF8CHAR pCh;

   /* the format of the OID is (\d+)((.\d+)*); these need to be parsed for
      their groups and then dropped into the appropriate array */

   /* decode the first " */
   stat = rtJsonDecMatchChar (pctxt, '"');
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pOID->numids = 0;

   if ('"' != rtJsonDecPeekChar2 (pctxt)) {
      for ( ;; ) {
         stat = rtxReadBytes (pctxt, &ub, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else if (! OS_ISDIGIT(ub) && ub != '.') {
            return LOG_RTERR (pctxt, RTERR_INVCHAR);
         }

         if (ub == '.') {
            /* reset */
            pOID->subid[pOID->numids] = curId;
            curId = 0;
            pOID->numids ++;
         }
         else {
            curId *= 10;
            curId += ub - '0';
         }

         stat = rtJsonDecPeekChar (pctxt, &pCh);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
         else if (pCh == '"') {
            pOID->subid[pOID->numids] = curId;
            pOID->numids ++;
            break;
         }
      }
   }

   stat = rtJsonDecMatchChar (pctxt, '"');
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}

