/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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

#include "rtAvn.h"
//#include "rtxsrc/rtxCharStr.h"
//#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxText.h"


static int rtAvnReadObjId_internal(OSCTXT* pctxt, ASN1OBJID* pvalue,
                                    OSBOOL bRelative)
{
   int ret = 0;
   OSRTMEMBUF membuf;
   OSOCTET ub;

   /* Match opening { */
   ret = rtxTxtMatchChar(pctxt, '{', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   rtxMemBufInit(pctxt, &membuf, 80);

   /* Capture everything until closing }.
      An improvement would be to discard any comments embedded in the vaue. */
   for(;;) {
      ret = rtxReadBytes(pctxt, &ub, 1);
      if (ret != 0) { LOG_RTERR(pctxt, ret); break; }

      if ( ub == '}' ) break;
      else {
         ret = rtxMemBufAppend(&membuf, &ub, 1);
         if ( ret != 0 ) { LOG_RTERR(pctxt, ret); break; }
      }
   }

   if (ret == 0) {
      const char* oidstr;
      int oidstrlen;

      oidstr = (char*) rtxMemBufGetData(&membuf, &oidstrlen);
      if (bRelative) ret = rtRelOIDParseCompList(oidstr, oidstrlen, pvalue);
      else ret = rtOIDParseCompList(oidstr, oidstrlen, pvalue);

      if (ret != 0) LOG_RTERR(pctxt, ret);
   }

   rtxMemBufFree(&membuf);

   return ret;
}


EXTRTMETHOD  int rtAvnReadObjId(OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   return rtAvnReadObjId_internal(pctxt, pvalue, FALSE);
}


EXTRTMETHOD int rtAvnReadRelOID(OSCTXT* pctxt, ASN1OBJID* pvalue)
{
   return rtAvnReadObjId_internal(pctxt, pvalue, TRUE);
}
