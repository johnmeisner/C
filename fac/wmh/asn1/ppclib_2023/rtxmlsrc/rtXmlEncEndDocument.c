/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxContext.hh"

EXTXMLMETHOD int rtXmlEncEndDocument (OSCTXT* pctxt)
{
   int stat = 0;

   /* If state is init, either nothing has been done yet, or end document
      was called previously.  Do nothing and return. */
   if (pctxt->state == OSXMLINIT) return stat;

   /* Make sure previous start element is terminated */
   stat = rtXmlEncTermStartElement (pctxt);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Check state */
   if (pctxt->state == OSXMLEND && pctxt->level == 0) {
      pctxt->state = OSXMLINIT;
   }
   else if (!rtxCtxtTestFlag (pctxt, OSXMLFRAG)) {
      RTDIAG3 (pctxt, "state = %d, level = %d\n",
               pctxt->state, pctxt->level);
      return LOG_RTERRNEW (pctxt, RTERR_XMLSTATE);
   }

   if (!OSRTISSTREAM (pctxt)) {
      if (pctxt->buffer.byteIndex < pctxt->buffer.size)
         OSRTZTERM (pctxt);
   }
   else {
#ifndef _NO_STREAM
      stat = rtxStreamFlush (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
#else
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);
#endif
   }
   LCHECKX (pctxt);
   return 0;
}


