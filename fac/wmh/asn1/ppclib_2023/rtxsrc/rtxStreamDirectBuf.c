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

#ifndef _NO_STREAM

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStreamMemory.h"
#include "rtxsrc/rtxStreamDirectBuf.hh"

#if 0 // #ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough, to test how flushing works */
#endif

/* Internal functions implemented in rtxStream.c */
int rtxiStreamGetPos (OSCTXT* pctxt, OSRTSTREAM *pStream, size_t* ppos);
int rtxiStreamSetPos (OSCTXT* pctxt, OSRTSTREAM *pStream, size_t pos);

static long dirbufWrite
(OSRTSTREAM* pStream, const OSOCTET* pdata, size_t numocts)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;

   if (pStream->id != OSRTSTRMID_DIRECTBUF ||
       !(pStream->flags & OSRTSTRMF_OUTPUT))
      return RTERR_INVPARAM;

   if (pctxt->buffer.byteIndex + numocts > pctxt->buffer.size) {
      int stat;

      /* Flush the buffer here */
      stat = pUnderStream->write
         (pUnderStream, pctxt->buffer.data, pctxt->buffer.byteIndex);
      if (stat != 0) return stat;

      /* reset the buffer pointer */
      pctxt->buffer.byteIndex = 0;
   }
   /* if number of octets is less then buffer size - put them into the buffer.
      Otherwise, write them into the stream directly. */
   if (pctxt->buffer.size > numocts) {
      OSCRTLSAFEMEMCPY ((void*)&pctxt->buffer.data [pctxt->buffer.byteIndex],
              pctxt->buffer.size - pctxt->buffer.byteIndex, pdata, numocts);
      pctxt->buffer.byteIndex += numocts;
   }
   else {
      /* write directly to the stream. Buffer should be flushed */
      int stat = pUnderStream->write (pUnderStream, pdata, numocts);
      if (stat != 0)
         return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

static long dirbufRead (OSRTSTREAM* pStream, OSOCTET* pdata, size_t size)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTBuffer *pbuf = &pctxt->buffer;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   int stat;
   size_t readySize;

   if (pStream->id != OSRTSTRMID_DIRECTBUF ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (pctxt->savedInfo.byteIndex != OSRTSTRM_K_INVALIDMARK) {
      if (pctxt->savedInfo.byteIndex + pStream->readAheadLimit <
          pbuf->byteIndex + size)
      {
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
         pStream->readAheadLimit = 0;
      }
   }

   stat = rtxStreamDirectBufPreRead (pctxt, size);
   if (stat < 0) return stat;

   readySize = pbuf->size - pbuf->byteIndex;

   if (0 != pdata) {
      if (size <= readySize) {
         OSCRTLSAFEMEMCPY (pdata, size, pbuf->data + pbuf->byteIndex, size);
         /*
         RTDIAG3 (pctxt, "dirbufRead: copied %d bytes from index %d\n",
                  size, pbuf->byteIndex);
         */
         pbuf->byteIndex += size;
      }
      else {
         size_t nbytes = 0;
         OSCRTLSAFEMEMCPY (pdata, size, pbuf->data + pbuf->byteIndex, readySize);
         pbuf->byteIndex += readySize;

         /* read directly from stream */
         do {
            long len = pUnderStream->read
               (pUnderStream, pdata + readySize, size - readySize);
            if (len <= 0) return RTERR_ENDOFBUF;
            nbytes += (size_t) len;
            readySize += len;
         } while (readySize < size);

         pStream->bytesProcessed += nbytes;
         /*
         RTDIAG2 (pctxt, "dirbufRead: direct read of %d bytes\n", nbytes);
         */
      }
      /*
      RTDIAG1 (pctxt, "dirbufRead - data read:\n");
      RTHEXDUMP (pctxt, pdata, size);
      */
   }

   return (long)size; /* was readySize */
}

static int dirbufClose (OSRTSTREAM* pStream)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   OSCTXT* pctxt = mdesc->pctxt;
   int stat = 0, stat2 = 0;

   if (pStream->id != OSRTSTRMID_DIRECTBUF)
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_OUTPUT) {
      stat = pStream->flush (pStream);
   }
   else if (!(pStream->flags & OSRTSTRMF_INPUT))
      stat = RTERR_NOTINIT;

   if (pctxt->buffer.dynamic && pctxt->buffer.data) {
      rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
      pctxt->buffer.data = 0;
   }

   stat2 = pUnderStream->close (pUnderStream);

   rtxMemSysFreePtr (pctxt, pUnderStream);
   rtxMemSysFreePtr (pctxt, pStream->extra);

   if (stat != 0 || stat2 != 0)
      return LOG_RTERR (pctxt, (stat != 0) ? stat : stat2);

   return 0;
}

static int dirbufFlush (OSRTSTREAM* pStream)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   OSCTXT* pctxt = mdesc->pctxt;
   int stat = 0;
   OSRTBuffer* buffer;

   if (!(pStream->flags & OSRTSTRMF_OUTPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   buffer = &pctxt->buffer;
   if (buffer->byteIndex > 0) {

      /* Flush the buffer here */
      if (pStream->write == 0)
         return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

      stat = pUnderStream->write
         (pUnderStream, buffer->data, buffer->byteIndex);

      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* reset the buffer pointer */
      buffer->byteIndex = 0;
   }
   return (pUnderStream->flush != 0) ? pUnderStream->flush (pUnderStream) : 0;
}

static int dirbufMark (OSRTSTREAM* pStream, size_t readAheadLimit)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;

   if (!(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
   pctxt->savedInfo.byteIndex = pctxt->buffer.byteIndex;
   /*??pctxt->savedInfo.bitOffset = pctxt->buffer.bitOffset;*/
   pctxt->savedInfo.flags = pctxt->flags;
   pStream->readAheadLimit = readAheadLimit;
   pStream->markedBytesProcessed = pStream->bytesProcessed;
   return 0;
}

static int dirbufReset (OSRTSTREAM* pStream)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;

   if (!(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (pctxt->savedInfo.byteIndex == OSRTSTRM_K_INVALIDMARK ||
       pctxt->buffer.byteIndex - pctxt->savedInfo.byteIndex >
       pStream->readAheadLimit)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pctxt->buffer.byteIndex = pctxt->savedInfo.byteIndex;
   /*??pctxt->buffer.bitOffset = pctxt->savedInfo.bitOffset;*/
   pctxt->flags = pctxt->savedInfo.flags;
   pStream->bytesProcessed = pStream->markedBytesProcessed;
   /*!AB: do we need to reset the mark?
   pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
   pStream->readAheadLimit = 0;*/
   return 0;
}

static int dirbufGetPos (OSRTSTREAM* pStream, size_t* ppos)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   /* return rtxiStreamGetPos (mdesc->pctxt, mdesc->pUnderStream, ppos); */
   *ppos = pStream->bytesProcessed + pctxt->buffer.byteIndex;
   return 0;
}

static int dirbufSetPos (OSRTSTREAM* pStream, size_t pos)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   int stat;

   if (pos < pStream->bytesProcessed) {
      pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.size = 0;

      stat = rtxiStreamSetPos (mdesc->pctxt, mdesc->pUnderStream, pos);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      pStream->bytesProcessed = pUnderStream->bytesProcessed;
   }
   else { /* position is within buffer */
      pctxt->buffer.byteIndex = pos - pStream->bytesProcessed;
   }

   return 0;
}

static int dirbufSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   DirBufDesc* mdesc = (DirBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTBuffer *pbuf = &pctxt->buffer;
   int stat;
   size_t readySize;

   if (pctxt->savedInfo.byteIndex != OSRTSTRM_K_INVALIDMARK) {
      if (pctxt->savedInfo.byteIndex + pStream->readAheadLimit <
          pbuf->byteIndex + skipBytes)
      {
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
         pStream->readAheadLimit = 0;
      }
   }

   stat = rtxStreamDirectBufPreRead (pctxt, skipBytes);
   if (stat < 0) return stat;

   readySize = pbuf->size - pbuf->byteIndex;
   if (skipBytes <= readySize) {
      pbuf->byteIndex += skipBytes;
   }
   else {
      pbuf->byteIndex += readySize;

      stat = pUnderStream->skip (pUnderStream, skipBytes - readySize);
      if (stat != 0) return stat;

      pStream->bytesProcessed += (skipBytes - readySize);
   }
   return 0;
}

int rtxStreamDirectBufCreate (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream;
   OSRTSTREAM *pUnderStream;
   DirBufDesc* mdesc;
   int stat;

   if (0 == pctxt->pStream) {
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
   }
   pUnderStream = pctxt->pStream;

   pctxt->pStream = 0;
   /* re-init stream */
   stat = rtxStreamInit (pctxt);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   pStream = pctxt->pStream;

   pctxt->pStream->flags |= OSRTSTRMF_BUFFERED;

   /* create buffer, if it is not created yet */
   if (pctxt->buffer.data == 0) {
      if ((stat = rtxInitContextBuffer (pctxt, 0, OSRTSTRM_K_BUFSIZE)) != 0)
         return LOG_RTERR (pctxt, stat);
   }
   else pctxt->buffer.byteIndex = 0; /* Reset to beginning */

   pctxt->pStream->bufsize = pctxt->buffer.size;
   pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;

   if (pUnderStream->flags & OSRTSTRMF_OUTPUT) {
      pStream->write = dirbufWrite;
      pStream->flush = dirbufFlush;
   }
   else if (pUnderStream->flags & OSRTSTRMF_INPUT) {
      pStream->read  = dirbufRead;
      pStream->skip  = dirbufSkip;
      pStream->mark = dirbufMark;
      pStream->reset = dirbufReset;
      pStream->getPos = dirbufGetPos;
      pStream->setPos = dirbufSetPos;
      pctxt->buffer.size = 0;
   }
   pStream->close = dirbufClose;

   mdesc = rtxMemSysAlloc (pctxt, sizeof(DirBufDesc));
   if (mdesc == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   mdesc->pctxt = pctxt;
   mdesc->pUnderStream = pUnderStream;
   mdesc->savedIndex = OSRTSTRM_K_INVALIDMARK;

   pStream->flags = pUnderStream->flags;
   pStream->flags |= OSRTSTRMF_BUFFERED;
   pStream->id    = OSRTSTRMID_DIRECTBUF;
   pStream->extra = mdesc;

   return 0;
}

int rtxStreamDirectBufCheck (OSCTXT* pctxt)
{
   if (OSRTISSTREAM (pctxt)) {
      int stat;

      if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_DIRECTBUF &&
          OSRTSTREAM_ID (pctxt) != OSRTSTRMID_MEMORY &&
          OSRTSTREAM_ID (pctxt) != OSRTSTRMID_ZLIB) {
         stat = rtxStreamDirectBufCreate (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      return 0;
   }
   return RTERR_NOTINIT;
}

int rtxStreamDirectBufPreRead (OSCTXT* pctxt, size_t size)
{
   OSRTSTREAM* pStream;
   DirBufDesc* mdesc;
   OSRTBuffer *pbuf;
   size_t newIndex, lowestIndex;
   int stat;

   stat = rtxStreamDirectBufCheck (pctxt);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   pStream = pctxt->pStream;
   pbuf = &pctxt->buffer;
   newIndex = pbuf->byteIndex;
   lowestIndex = pbuf->byteIndex;
   mdesc = (DirBufDesc*)pStream->extra;
   newIndex += size;

   /* reset mark pos if it is out of range */
   if (pctxt->savedInfo.byteIndex != OSRTSTRM_K_INVALIDMARK &&
       newIndex - pctxt->savedInfo.byteIndex > pStream->readAheadLimit)
   {
      pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
      pStream->readAheadLimit = 0;
   }
   else {
      if (pctxt->savedInfo.byteIndex < pbuf->byteIndex)
         lowestIndex = pctxt->savedInfo.byteIndex;
   }

   /* new index exceed the current size (pbuf->size)? */
   if (newIndex > pbuf->size) {
      long exceedDelta = (long)(newIndex - pbuf->size);

      if (newIndex >= pStream->bufsize) {
         if (newIndex - lowestIndex > pStream->bufsize) {
            size_t newSize = newIndex - lowestIndex;
            OSOCTET* newdata = (OSOCTET*)
               rtxMemSysRealloc (pctxt, pbuf->data, newSize);
            if (newdata == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
            pbuf->data = newdata;
            pStream->bufsize = newSize;
         }

         if (pctxt->savedInfo.byteIndex != OSRTSTRM_K_INVALIDMARK)
         {
            RTDIAG1 (pctxt, "move marked data to beginning of buffer\n");
            pbuf->size = pbuf->size - pctxt->savedInfo.byteIndex;
            OSCRTLSAFEMEMCPY (pbuf->data, pbuf->size, pbuf->data + pctxt->savedInfo.byteIndex,
                    pbuf->size);
            pStream->bytesProcessed += pctxt->savedInfo.byteIndex;
            pStream->markedBytesProcessed = pStream->bytesProcessed;
            pbuf->byteIndex -= pctxt->savedInfo.byteIndex;
            pctxt->savedInfo.byteIndex = 0;
         }
         else {
            RTDIAG1 (pctxt, "reset buffer position to beginning\n");
            pbuf->size = pbuf->size - pbuf->byteIndex;
            OSCRTLSAFEMEMMOVE
               (pbuf->data, pbuf->size, pbuf->data + pbuf->byteIndex, pbuf->size);
            pStream->bytesProcessed += pbuf->byteIndex;
            pbuf->byteIndex = 0;
         }
         RTDIAG2 (pctxt, "pStream->bytesProcessed = %d\n",
                  pStream->bytesProcessed);
         RTDIAG2 (pctxt, "pbuf->byteIndex = %d\n", pbuf->byteIndex);
      }

      /* read pre-determined mandatory amount of data from stream */
      do {
         size_t bytesToRead = pStream->bufsize - pbuf->size;
         long len = mdesc->pUnderStream->read
            (mdesc->pUnderStream, pbuf->data + pbuf->size, bytesToRead);

         if (len <= 0)
            return LOG_RTERRNEW (pctxt, RTERR_ENDOFBUF);

         exceedDelta -= len;
         pbuf->size += len;
      } while (exceedDelta > 0 && pStream->bufsize - pbuf->size > 0);

      RTDIAG1 (pctxt, "rtxStreamDirectBufPreRead > context buffer:\n");
      RTHEXDUMP (pctxt, pbuf->data, pbuf->size);
   }

   return 0;
}

#endif /* _NO_STREAM */
