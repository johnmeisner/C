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

#include "xee_common.hh"
#include "rtxsrc/rtxStream.h"
#if OS_ASN1RT_VERSION >= 600
#include "rtxsrc/rtxBuffer.h"
#endif

int xerEncEndDocument (OSCTXT* pctxt)
{
   if (!OSRTISSTREAM (pctxt)) {
      if (pctxt->state == XEREND && pctxt->level == 0) {
         int stat;
         /*!AB: should not add the end of line after the last end element
         stat = xerEncNewLine (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat); */

         stat = rtxWriteBytes (pctxt, (const OSOCTET*)"\0", 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         /* decrement byte index so nul char is not counted (ED, 7/1/2002) */
         pctxt->buffer.byteIndex--;
      }
      else
         return LOG_RTERR (pctxt, RTERR_XMLSTATE);
   }
   else {
      int stat = rtxStreamFlush (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   return 0;
}

