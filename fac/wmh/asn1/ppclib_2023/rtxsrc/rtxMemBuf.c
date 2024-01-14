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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemHeap.hh"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"

EXTRTMETHOD void rtxMemBufInit
(OSCTXT* pCtxt, OSRTMEMBUF* pMemBuf, OSSIZE segsize)
{
   pMemBuf->pctxt = pCtxt;
   pMemBuf->segsize = (0 == segsize) ? OSMBDFLTSEGSIZE : segsize;
   pMemBuf->startidx = 0;
   pMemBuf->usedcnt = 0;
   pMemBuf->bufsize = 0;
   pMemBuf->buffer = 0;
   pMemBuf->bitOffset = 0;
   pMemBuf->userState = 0;
   pMemBuf->isDynamic = TRUE;
   pMemBuf->isExpandable = TRUE;
   pMemBuf->useSysMem = FALSE;
}

EXTRTMETHOD void rtxMemBufInitBuffer
(OSCTXT* pCtxt, OSRTMEMBUF* pMemBuf,
 OSOCTET* buf, OSSIZE bufsize, OSSIZE segsize)
{
   pMemBuf->pctxt = pCtxt;
   pMemBuf->segsize = segsize;
   pMemBuf->startidx = 0;
   pMemBuf->usedcnt = 0;
   pMemBuf->bufsize = bufsize;
   pMemBuf->buffer = buf;
   pMemBuf->bitOffset = 0;
   pMemBuf->userState = 0;
   pMemBuf->isDynamic = FALSE;
   pMemBuf->isExpandable = TRUE;
   pMemBuf->useSysMem = FALSE;
}

EXTRTMETHOD void rtxMemBufFree (OSRTMEMBUF* pMemBuf)
{
   if (pMemBuf->isDynamic) {
      if (pMemBuf->useSysMem) {
         rtxMemSysFreePtr (pMemBuf->pctxt, pMemBuf->buffer);
      }
      else {
         rtxMemFreePtr (pMemBuf->pctxt, pMemBuf->buffer);
      }
      pMemBuf->buffer = 0;
      pMemBuf->bufsize = 0;
   }

   rtxMemBufReset (pMemBuf);
}

EXTRTMETHOD void rtxMemBufReset (OSRTMEMBUF* pMemBuf)
{
   pMemBuf->usedcnt = 0;
   pMemBuf->startidx = 0;
   pMemBuf->bitOffset = 0;
}

EXTRTMETHOD int rtxMemBufAppend
(OSRTMEMBUF* pMemBuf, const OSOCTET* pdata, OSSIZE nbytes)
{
   int stat = 0;

   if (0 == nbytes) return 0;

   /* Ensure capacity for data */
   stat = rtxMemBufPreAllocate (pMemBuf, nbytes);

   /* Copy data */
   if (stat == 0) {
      if (pdata) {
         OSCRTLSAFEMEMCPY (&pMemBuf->buffer[pMemBuf->usedcnt],
               pMemBuf->bufsize-pMemBuf->usedcnt, pdata, nbytes);
         pMemBuf->usedcnt += nbytes;
      }
   }
   else return LOG_RTERR (pMemBuf->pctxt, stat);
   return 0;
}

EXTRTMETHOD int rtxMemBufPreAllocate (OSRTMEMBUF* pMemBuf, OSSIZE nbytes)
{
   if (0 == nbytes) return 0;

   /* Ensure capacity for data */
   if ((pMemBuf->usedcnt + nbytes) > pMemBuf->bufsize) {
      OSSIZE reqbytes = pMemBuf->usedcnt + nbytes;
      OSSIZE nsegs;
      OSSIZE newsize;

      if ( pMemBuf->segsize == 0 )
         return LOG_RTERRNEW(pMemBuf->pctxt, RTERR_NOMEM);

      nsegs = ((reqbytes - 1) / pMemBuf->segsize) + 1;
      newsize = pMemBuf->segsize * nsegs;

      if (pMemBuf->isDynamic && 0 != pMemBuf->buffer) {
         OSOCTET* newbuf;

         if (pMemBuf->useSysMem) {
            newbuf = (OSOCTET*) rtxMemSysRealloc
               (pMemBuf->pctxt, (void*)pMemBuf->buffer, newsize);
         }
#ifndef _MEMCOMPACT
         else if (((OSMemHeap*)(pMemBuf->pctxt->pMemHeap))->flags &
                                                               RT_MH_STATIC)
         {
            /* Static memory. Call special internal function to
            "reallocate" last chunk.  We'll get null if our memory is not
            at the end of the heap. Static memory cannot be set when _MEMCOMPACT
            is defined.
            */
            newbuf = rtxMemHeapReallocStatic(&pMemBuf->pctxt->pMemHeap,
                                             pMemBuf->buffer, pMemBuf->bufsize,
                                             reqbytes);
            newsize = reqbytes;
         }
#endif
         else {
            newbuf = rtxMemReallocArray
               (pMemBuf->pctxt, pMemBuf->buffer, newsize, OSOCTET);
         }

         if (newbuf == 0)
            return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_NOMEM);

         pMemBuf->buffer = newbuf;
      }
      else { /* Do initial mem alloc */
         OSOCTET *oldbuf = pMemBuf->buffer;

         if (pMemBuf->useSysMem) {
            pMemBuf->buffer = (OSOCTET*) rtxMemSysAlloc
               (pMemBuf->pctxt, newsize);
         }
         else {
            pMemBuf->buffer = rtxMemAllocArray
               (pMemBuf->pctxt, newsize, OSOCTET);
         }

         if (pMemBuf->buffer == 0)
            return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_NOMEM);

         if (oldbuf && pMemBuf->bufsize)
            OSCRTLSAFEMEMCPY (pMemBuf->buffer, newsize,
                  oldbuf, pMemBuf->bufsize);

         pMemBuf->isDynamic = TRUE;
      }
      if (0 == pMemBuf->buffer)
         return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_NOMEM);

      pMemBuf->bufsize = newsize;
   }
   return 0;
}

/**
  Removes leading and trailing white spaces. Buffer should contain
  16-bit character string without trailing zero.
  Returns the length of resulting buffer.
*/
EXTRTMETHOD OSSIZE rtxMemBufTrimW (OSRTMEMBUF* pMemBuf)
{
   OSSIZE i;

   /* skip leading white space */

   for (i = pMemBuf->startidx; i < pMemBuf->usedcnt; i += 2) {
      if (0 == pMemBuf->buffer[i] && OS_ISSPACE(pMemBuf->buffer[i+1])) {
         pMemBuf->startidx += sizeof (OSUNICHAR);
      }
      else break;
   }

   /* skip trailing white space */

   if (pMemBuf->usedcnt >= 2) {
      for (i = pMemBuf->usedcnt - 2; i >= pMemBuf->startidx; i -= 2) {
         if (0 == pMemBuf->buffer[i] && OS_ISSPACE(pMemBuf->buffer[i+1])) {
            pMemBuf->usedcnt -= sizeof (OSUNICHAR);
         }
         else break;
      }
   }

   return (pMemBuf->usedcnt >= pMemBuf->startidx) ?
      pMemBuf->usedcnt - pMemBuf->startidx : 0;
}

EXTRTMETHOD int rtxMemBufSet
(OSRTMEMBUF* pMemBuf, OSOCTET value, OSSIZE nbytes)
{
   int stat;

   if (0 == nbytes) return 0;

   /* Ensure capacity for data */
   stat = rtxMemBufPreAllocate (pMemBuf, nbytes);
   if (stat != 0) return LOG_RTERR (pMemBuf->pctxt, stat);

   /* Set data */
   OSCRTLMEMSET (&pMemBuf->buffer[pMemBuf->usedcnt], value, nbytes);
   pMemBuf->usedcnt += nbytes;

   return 0;
}

EXTRTMETHOD OSOCTET* rtxMemBufGetData (const OSRTMEMBUF* pMemBuf, int* length)
{
   if (length != 0) {
      OSSIZE sz = pMemBuf->usedcnt - pMemBuf->startidx;
      *length = (sz <= OSINT32_MAX) ? (int)sz : -1;
   }
   return pMemBuf->buffer + pMemBuf->startidx;
}

EXTRTMETHOD OSOCTET* rtxMemBufGetDataExt
(const OSRTMEMBUF* pMemBuf, OSSIZE* length)
{
   if (length != 0) {
      *length = rtxMemBufGetDataLen (pMemBuf);
   }
   return pMemBuf->buffer + pMemBuf->startidx;
}

EXTRTMETHOD OSSIZE rtxMemBufGetDataLen (const OSRTMEMBUF* pMemBuf)
{
   return (pMemBuf->usedcnt >= pMemBuf->startidx) ?
      (OSSIZE)(pMemBuf->usedcnt - pMemBuf->startidx) : 0;
}

EXTRTMETHOD int rtxMemBufCut
(OSRTMEMBUF* pMemBuf, OSSIZE fromOffset, OSSIZE nbytes)
{
   if (fromOffset + nbytes > pMemBuf->usedcnt)
      return LOG_RTERRNEW (pMemBuf->pctxt, RTERR_INVPARAM);

   OSCRTLSAFEMEMMOVE (pMemBuf->buffer + pMemBuf->startidx + fromOffset,
         pMemBuf->usedcnt-nbytes,
         pMemBuf->buffer + pMemBuf->startidx + fromOffset + nbytes,
         pMemBuf->usedcnt - nbytes);

   pMemBuf->usedcnt -= nbytes;
   return 0;
}

EXTRTMETHOD OSBOOL rtxMemBufSetExpandable
(OSRTMEMBUF* pMemBuf, OSBOOL isExpandable)
{
   OSBOOL wasExpandable = pMemBuf->isExpandable;
   pMemBuf->isExpandable = isExpandable;
   return wasExpandable;
}

EXTRTMETHOD OSBOOL rtxMemBufSetUseSysMem
(OSRTMEMBUF* pMemBuf, OSBOOL useSysMem)
{
   OSBOOL prevSetting = pMemBuf->useSysMem;
   pMemBuf->useSysMem = useSysMem;
   return prevSetting;
}
