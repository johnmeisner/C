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

#include "rtxsrc/rtxContext.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStream.h"

EXTRTMETHOD int rtxMarkPos (OSCTXT* pctxt, size_t* ppos)
{
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      OSRTSTREAM *pStream = pctxt->pStream;
      int stat;

      if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
          pStream->id != OSRTSTRMID_CTXTBUF &&
          pStream->id != OSRTSTRMID_DIRECTBUF)
      {
         /* stream attached to context buffer */
         if (pStream->getPos == 0 || pStream->setPos == 0) {
            if ((pStream->flags & OSRTSTRMF_FIXINMEM) == 0) {
               pStream->markedBytesProcessed = pctxt->buffer.byteIndex;
               pStream->flags |= OSRTSTRMF_FIXINMEM;
            }

            *ppos = pctxt->buffer.byteIndex - pStream->markedBytesProcessed;
         }
         else {
            stat = pStream->getPos (pStream, ppos);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);
         }
      }
      else { /* direct access to stream */
         stat = pStream->getPos (pStream, ppos);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }
   }
   else
#endif /* _NO_STREAM */
      *ppos = pctxt->buffer.byteIndex;

   return 0;
}

EXTRTMETHOD int rtxResetToPos (OSCTXT* pctxt, size_t pos)
{
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      OSRTSTREAM *pStream = pctxt->pStream;
      int stat;

      if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
          pStream->id != OSRTSTRMID_CTXTBUF &&
          pStream->id != OSRTSTRMID_DIRECTBUF)
      {
         /* stream attached to context buffer */

         if (pStream->getPos == 0 || pStream->setPos == 0) {
            if ((pStream->flags & OSRTSTRMF_FIXINMEM) == 0)
               return LOG_RTERR (pctxt, RTERR_INVPARAM);

            if (pos == 0)
               pStream->flags &= ~OSRTSTRMF_FIXINMEM;

            pctxt->buffer.byteIndex = pStream->markedBytesProcessed + pos;
         }
         else if (pStream->bytesProcessed < pos) {
            /* position in buffer */
            pctxt->buffer.byteIndex = pos - pStream->bytesProcessed;
         }
         else {
            stat = pStream->setPos (pStream, pos);
            if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            pctxt->buffer.byteIndex = 0;
            pctxt->buffer.size = 0;
         }
      }
      else { /* direct access to stream */
         stat = pStream->setPos (pStream, pos);
         if (stat < 0)
            return LOG_RTERR (pctxt, stat);
      }
   }
   else
#endif /* _NO_STREAM */
      pctxt->buffer.byteIndex = pos;

   return 0;
}
