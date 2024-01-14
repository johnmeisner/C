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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxStream.h"

EXTXMLMETHOD int rtXmlSetEncBufPtr
(OSCTXT* pctxt, OSOCTET* bufaddr, size_t bufsiz)
{
   int stat;

#ifndef _NO_STREAM
   if (bufaddr == 0 && pctxt->pStream != 0 &&
      0 == (pctxt->pStream->flags & OSRTSTRMF_BUFFERED))
   {
      /* Under the above conditions, rtxInitContextBuffer would not allocate
         a buffer, as requested, leading to a crash when pctxt->buffer is
         written into below.  By calling this method, the assumption is
         that we want a buffer for encoding and not a stream, so close the
         stream so that a buffer will be allocated.
      */
      stat = rtxStreamClose(pctxt);
      if (stat != 0) return LOG_RTERR(pctxt, stat);
   }
#endif

   stat = rtxInitContextBuffer(pctxt, bufaddr, bufsiz);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   OSRT_CHECK_EVAL_DATE1(pctxt);

   pctxt->buffer.data[0] = '\0';
   rtxCtxtSetFlag (pctxt, OSDISSTRM); /* disable stream operations */
   pctxt->state = OSXMLINIT;

   return 0;
}


