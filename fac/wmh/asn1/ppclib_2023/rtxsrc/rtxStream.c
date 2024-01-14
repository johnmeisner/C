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
#include <string.h>
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxDiag.h"

/* Uncomment to enable diagnostics or add -DRTXDEBUG to gcc command line.
#define RTXDEBUG
*/
#ifdef RTXDEBUG
#define RTLDIAG1(pctxt,msg)     rtxDiagStream(pctxt,msg)
#define RTLDIAG2(pctxt,msg,a)   rtxDiagStream(pctxt,msg,a)
#define RTLHEXDUMP(pctxt,buffer,numocts) \
rtxDiagStreamHexDump(pctxt,buffer,numocts)
#else
#define RTLDIAG1(pctxt,msg)
#define RTLDIAG2(pctxt,msg,a)
#define RTLHEXDUMP(pctxt,buffer,numocts)
#endif

#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif /* RTEVAL */

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test refilling */
#endif

EXTRTMETHOD int rtxStreamInit (OSCTXT* pctxt)
{
   /* If another stream is open, close it first */
   if (0 != pctxt->pStream) {
      rtxStreamClose (pctxt);
   }
   pctxt->pStream = (OSRTSTREAM*) rtxMemSysAlloc (pctxt, sizeof(OSRTSTREAM));
   if (pctxt->pStream == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

   OSCRTLMEMSET (pctxt->pStream, 0, sizeof(OSRTSTREAM));

   rtxCtxtClearFlag (pctxt, OSDISSTRM);

   /* LCHECKX (pctxt); */
   return 0;
}

EXTRTMETHOD int rtxStreamInitCtxtBuf (OSCTXT* pctxt)
{
   if (pctxt->buffer.data == 0) {
      OSRTSTREAM *pStream = pctxt->pStream;
      int ret;

      /* Make sure this isn't a buffered stream type that is already
         using the context buffer */
      if (pStream->id == OSRTSTRMID_CTXTBUF ||
          pStream->id == OSRTSTRMID_DIRECTBUF) {
         rtxErrAddStrParm
            (pctxt, "init context buffer on already buffered stream");
         return LOG_RTERR (pctxt, RTERR_NOTSUPP);
      }

      /* allocate */
      pStream->flags |= OSRTSTRMF_BUFFERED;

      ret = rtxInitContextBuffer (pctxt, 0, OSRTSTRM_K_BUFSIZE);
      if (ret != 0) return LOG_RTERR (pctxt, ret);

      if (0 != (pStream->flags & OSRTSTRMF_INPUT)) {
         pctxt->buffer.size = 0;
         pStream->segsize = OSUINT32_MAX;
      }
      pStream->bufsize = OSRTSTRM_K_BUFSIZE;
   }
   else {
      rtxErrAddStrParm (pctxt, "context buffer pointer already set");
      return LOG_RTERR (pctxt, RTERR_FAILED);
   }

   return 0;
}

EXTRTMETHOD int rtxStreamRemoveCtxtBuf (OSCTXT* pctxt)
{
   if (0 != pctxt->pStream &&
       (0 != (pctxt->pStream->flags & OSRTSTRMF_BUFFERED)))
   {
      pctxt->pStream->flags &= ~OSRTSTRMF_BUFFERED;
      if (0 != pctxt->buffer.data && pctxt->buffer.dynamic) {
         if (0 != (pctxt->flags & OSBUFSYSALLOC)) {
            rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
            pctxt->flags &= ~OSBUFSYSALLOC;
         }
         pctxt->buffer.data = 0;
         pctxt->buffer.size = pctxt->buffer.byteIndex = 0;
         pctxt->buffer.bitOffset = 8;
         pctxt->buffer.dynamic = FALSE;
         pctxt->pStream->bufsize = 0;
      }
   }

   return 0;
}

EXTRTMETHOD int rtxStreamFlush (OSCTXT* pctxt)
{
   int stat = 0;
   OSRTSTREAM *pStream = pctxt->pStream;

   if (pStream == 0 || !(pStream->flags & OSRTSTRMF_OUTPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      /* write data reminder  from context buffer */
      size_t len = pctxt->buffer.byteIndex;
      /*
      if (pctxt->buffer.bitOffset != 8)
         len++;
      */
      if (len > 0) {
         stat = rtxStreamWrite (pctxt, pctxt->buffer.data, len);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         pctxt->buffer.byteIndex = 0;
         if (pctxt->buffer.bitOffset != 8) {
            /* move partial byte to start of buffer */
            pctxt->buffer.data[0] = pctxt->buffer.data[len];
         }
      }
   }

   if (pStream->flush != 0) {
      stat = pStream->flush (pStream);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }

   LCHECKX (pctxt);
   return stat;
}

EXTRTMETHOD int rtxStreamClose (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   int stat = 0, stat2 = 0;

   if (pStream == 0 || pStream->close == 0) {
      /* we should not mess with the buffer if the stream does not exist
         because we don't know if it is buffered or not (ED, 2/1/2013)
      if (pctxt->buffer.dynamic && pctxt->buffer.data) {
         if (OSRTSTRMF_BUFFERED)
            rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
         pctxt->buffer.data = 0;
         pctxt->buffer.byteIndex = 0;
         pctxt->buffer.bitOffset = 8;
         pctxt->buffer.size = 0;
      }
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
      */
      if (pStream != 0) {
         rtxMemSysFreePtr (pctxt, pctxt->pStream);
         pctxt->pStream = 0;
      }
      return 0; /* already closed, not an error */
   }

   if (pStream->flags & OSRTSTRMF_OUTPUT) {
      if (0 != pctxt->buffer.data && pctxt->buffer.bitOffset != 8) {
         /* byte align to ensure last octet gets written out */
         pctxt->buffer.byteIndex++;
         pctxt->buffer.bitOffset = 8;
      }
      stat = rtxStreamFlush (pctxt);
   }
   else if (!(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   stat2 = pStream->close (pStream);

   if (pctxt->buffer.data) {
      if (pctxt->buffer.dynamic && pStream->flags & OSRTSTRMF_BUFFERED) {
         /* Free the memory.  The stream buffer is always allocated from
            the system, so memory is freed using rtxMemSysFreePtr. */
         rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
      }

      pctxt->buffer.data = 0;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.size = 0;
   }

   pStream->flags = 0;

   rtxMemSysFreePtr (pctxt, pctxt->pStream);
   pctxt->pStream = 0;

   if ( stat != 0 ) return LOG_RTERR (pctxt, stat);
   else if ( stat2 != 0 ) return LOG_RTERR (pctxt, stat2);
   else return 0;
}

EXTRTMETHOD long rtxStreamWrite
(OSCTXT* pctxt, const OSOCTET* data, size_t numocts)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len;

   if (pStream == 0 || pStream->write == 0 ||
       !(pStream->flags & OSRTSTRMF_OUTPUT))
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   OSRT_CHECK_EVAL_DATE0 (pctxt);

   len = pStream->write (pStream, data, numocts);
   if (len >= 0) {
      pStream->ioBytes += numocts;
      pStream->bytesProcessed += len;

      /* If capture enabled, append data to capture buffer */
      if (0 != pStream->pCaptureBuf) {
         int stat;
         if (0 == data) {
            /* assume context buffer was used */
            data = OSRTBUFPTR (pctxt);
         }
         stat = rtxMemBufAppend (pStream->pCaptureBuf, data, numocts);
         if (0 != stat) LOG_RTERR (pctxt, stat);
      }
   }
   else
      return LOG_RTERR (pctxt, (int)len);

   LCHECKX (pctxt);
   return len;
}

/* If there's a capture buffer, append the data we read to it. */
static void procInputData (OSCTXT* pctxt, OSOCTET* pbuffer, long len)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   if (pStream->id != OSRTSTRMID_CTXTBUF) {
      pStream->ioBytes += len;
   }
   /* If capture enabled, append data to capture buffer */
   if (0 != pStream->pCaptureBuf) {
      int stat;
      if (0 == pbuffer) {
         /* assume context buffer was used */
         pbuffer = OSRTBUFPTR (pctxt);
      }
      stat = rtxMemBufAppend (pStream->pCaptureBuf, pbuffer, len);
      if (0 != stat) LOG_RTERR (pctxt, stat);
   }
}


/**
 * Read directly from the attached stream into the given buffer.
 * This will read nbytes bytes, or as many bytes as can be read before EOF, up
 * to bufSize bytes.  EOF is not considered an error.
 * procInputData is called to send all read bytes to the capture buffer.
 * @param pbuffer Buffer to read into.
 * @param nbytes The minimum number of bytes to attempt to read.
 * @param bufSize The size of the buffer.  The function may read as many as
 *    bufSize bytes.
 * @return # of bytes read or a negative error code.  An error is returned if
 *    nbytes > bufSize.
 */
static long readDirect(OSCTXT *pctxt, OSOCTET* pbuffer, OSSIZE nbytes,
   OSSIZE bufSize)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len = 0;     /* Total bytes read thus far. */

   if (nbytes > bufSize) return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);

   while ((OSSIZE)len < nbytes) {
      OSOCTET* pchunk = pbuffer + len; /* Location to read into. */
      long nread = pStream->read(pStream, pchunk, bufSize - len);
      if (nread < 0) {
         /* error */
         return LOG_RTERR(pctxt, (int)nread);
      }

      if (nread == 0) break; /* EOF */
      else {
         len += nread;
         procInputData(pctxt, pchunk, nread);
      }
   }

   return len;
}


EXTRTMETHOD int rtxStreamLoadInputBuffer(OSCTXT* pctxt, OSSIZE nbytes)
{
   int stat;
   OSRTSTREAM* pStream;

   if (0 == pctxt) {
      return RTERR_INVPARAM;
   }

   pStream = pctxt->pStream;

   if (0 == pStream) {
      return LOG_RTERR(pctxt, RTERR_NOTINIT);
   }

   if (pctxt->buffer.data == 0) {
      stat = rtxStreamInitCtxtBuf(pctxt);
      if (0 != stat) return LOG_RTERR(pctxt, stat);
   }

   /* Not sure what this does.. */
   if (pctxt->buffer.size + pStream->bytesProcessed == pStream->segsize) {
      /* unexpected end of segment */
      return LOG_RTERR(pctxt, RTERR_ENDOFBUF);
   }

   if (pctxt->buffer.byteIndex + nbytes > pStream->bufsize) {
      /* The buffer cannot hold the requested number of bytes, at least not
         without flushing out some previously read data and perhaps not even
         then.
      */
      if (pStream->flags & OSRTSTRMF_FIXINMEM) {
         /* We cannot flush used data out of the buffer. */
         if (pStream->markedBytesProcessed > 0) {
            /* skip unmarked space */
            /* We flush out markedBytesProcessed bytes.  I'm not sure why
               that's okay. It seems contrary to OSRTSTRMF_FIXINMEM. */
            OSSIZE tail; /* bytes to keep */
            tail = pctxt->buffer.size - pStream->markedBytesProcessed;
            if (tail > 0)
               OSCRTLSAFEMEMMOVE(pctxt->buffer.data, pctxt->buffer.size,
                  pctxt->buffer.data + pStream->markedBytesProcessed, tail);

            pctxt->buffer.byteIndex -= pctxt->pStream->markedBytesProcessed;
            pStream->markedBytesProcessed = 0;
            pctxt->buffer.size = tail;
         }
         /* else: we really can't flush anything; the buffer will be resized. */
      }
      else {
         /* Flush used data out of the buffer. */
         /* Note: The version of readInputStream that was in rtxCheckBuffer.c
         was fixed to preserve MIN_STREAM_BACKOFF bytes of already read data
         in the buffer.  Currently, MIN_STREAM_BACKOFF is defined as 0, so this
         is irrelevant, but I'm carrying over the fix to this code just in case
         we end up defining MIN_STREAM_BACKOFF > 0 again. */
         OSSIZE tail; /* bytes to keep */
         tail = pctxt->buffer.size - pctxt->buffer.byteIndex +
            MIN_STREAM_BACKOFF;

         if (tail > 0) {
            OSCRTLSAFEMEMMOVE(pctxt->buffer.data, pctxt->buffer.size,
               pctxt->buffer.data + pctxt->buffer.byteIndex - MIN_STREAM_BACKOFF,
               tail);
         }

         pStream->bytesProcessed +=
            pctxt->buffer.byteIndex - MIN_STREAM_BACKOFF;

         pctxt->buffer.size = tail;
         pctxt->buffer.byteIndex = MIN_STREAM_BACKOFF;
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
      }

      if (pctxt->buffer.byteIndex + nbytes <= pStream->bufsize)
         ; /* Buffer has enough space now. */
      else if (pctxt->buffer.dynamic) {
         /* Resize the buffer. */
         OSOCTET* newbuf;
         OSSIZE extent = OSRTMAX(pctxt->pStream->bufsize, nbytes);

         pStream->bufsize += extent;

         newbuf = (OSOCTET*)rtxMemSysRealloc
         (pctxt, pctxt->buffer.data, pStream->bufsize);

         if (newbuf == NULL) return LOG_RTERR(pctxt, RTERR_NOMEM);

         pctxt->buffer.data = newbuf;
      }
      else {
         return LOG_RTERRNEW(pctxt, RTERR_BUFOVFLW);
      }
   }

   /* Read bytes into the buffer.
      We have ensured that:
         pctxt->buffer.byteIndex + nbytes <= pStream->bufsize
      If we fill the buffer (which is optimal), we will satisfy the post
      condition:
         pctxt->buffer.byteIndex + nbytes <= pctxt->buffer.size.

      Loop until we reach EOF or make nbytes available.
   */
   if (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size)
   {
      OSSIZE bytesToFillRequest =
         nbytes - (pctxt->buffer.size - pctxt->buffer.byteIndex);
      OSSIZE bytesToFillBuffer = pStream->bufsize - pctxt->buffer.size;
      OSOCTET* pdata = pctxt->buffer.data + pctxt->buffer.size;

      RTLDIAG2 (pctxt, "~D reading " OSSIZEFMT " bytes to fill context buffer"
                "\n", bytesToFillBuffer);

      stat = (int)readDirect(pctxt, pdata,
                              bytesToFillRequest, bytesToFillBuffer);
      if (stat < 0)
         return LOG_RTERR(pctxt, stat);
      else {
         RTLDIAG2(pctxt, "~D %d bytes read\n", stat);
         /* Extend buffer size to reflect available data. */
         pctxt->buffer.size += stat;
      }
   }

   if (pctxt->buffer.size + pctxt->pStream->bytesProcessed >
      pctxt->pStream->segsize)
   {
      /* set segment border */
      pctxt->buffer.size =
         pctxt->pStream->segsize - pctxt->pStream->bytesProcessed;
   }

   return 0;
}


/**
 * Read nocts bytes into pdata.  The bytes may come from the input stream or
 * from bytes previously buffered in the context buffer (which is being used
 * for buffering), or both.
 *
 * This will block until the given number of bytes are read, EOF is reached,
 * or there is an error.  Any bytes read from the stream will be sent to
 * the capture buffer, if there is one.
 * Returns the number of bytes read into pdata.
 */
static long readFromCtxtBufOrStream
(OSCTXT* pctxt, OSOCTET* pdata, OSSIZE nocts)
{
   OSSIZE nread = 0; /* bytes read into pdata */

   if (pctxt->buffer.byteIndex + nocts > pctxt->buffer.size) {
      /* We need more bytes than are in the buffer. */
      OSRTSTREAM* pStream = pctxt->pStream;
      OSSIZE numToRead;
      int stat;

      if (pctxt->buffer.size > pctxt->buffer.byteIndex) {
         /* Some bytes remain in buffer.  Use them. */
         OSSIZE tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
         if (0 != pdata) {
            OSCRTLSAFEMEMCPY (pdata, tail, OSRTBUFPTR(pctxt), tail);
            pdata += tail;
         }
         pctxt->buffer.byteIndex = pctxt->buffer.size;
         nread += tail;
      }

      numToRead = nocts - nread;

      if (numToRead <= OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF) {
         RTLDIAG2 (pctxt, "~D readFromCtxtBufOrStream: fill context buffer, "
                   "numToRead = " OSSIZEFMT "\n", numToRead);

         /* Try to read more data into the buffer. */
         stat = rtxStreamLoadInputBuffer(pctxt, numToRead);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         /* Adjust numToRead based on actual bytes now available. We could have
            hit EOF and fewer bytes would be available. */
         numToRead =
            OSRTMIN(numToRead, pctxt->buffer.size - pctxt->buffer.byteIndex);
         if (0 != pdata) {
            OSCRTLSAFEMEMCPY (pdata, numToRead, OSRTBUFPTR(pctxt), numToRead);
         }
         pctxt->buffer.byteIndex += numToRead;
         nread += numToRead;
      }
      else {
         RTLDIAG1 (pctxt, "~D readFromCtxtBufOrStream: direct stream read\n");

         /* Requested number of bytes is larger than what context buffer
            can hold; therefore, we read directly from stream and let the
            context buffer be empty. */

         /* The buffer data is entirely processed and is now being discarded. */
         pStream->bytesProcessed += pctxt->buffer.size;
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
         pctxt->buffer.size = 0;
         pctxt->buffer.byteIndex = 0;

         /* Read directly from the stream. */
         stat = readDirect(pctxt, pdata, numToRead, numToRead);
         if (stat < 0) return LOG_RTERR(pctxt, stat);
         else {
            nread += (OSSIZE)stat;
            pStream->bytesProcessed += (OSSIZE)stat;
         }
      }
   }
   else {
      /* Copy directly from context buffer to target buffer */
      if (0 != pdata) {
         OSCRTLSAFEMEMCPY (pdata, nocts, OSRTBUFPTR(pctxt), nocts);
      }
      pctxt->buffer.byteIndex += nocts;
      nread = nocts;
   }

   /* Set last character in context */
   if (pctxt->buffer.byteIndex > 0) {
      pctxt->lastChar = pctxt->buffer.data[pctxt->buffer.byteIndex-1];
   }

   return (long)nread;
}


EXTRTMETHOD long rtxStreamRead
(OSCTXT* pctxt, OSOCTET* pbuffer, OSSIZE bufSize)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len;

   if (pStream == 0 || pStream->read == 0 ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   if (bufSize == 0) return 0;

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      /* Context buffer is being used to buffer data read from the stream. */
      len = readFromCtxtBufOrStream (pctxt, pbuffer, bufSize);
   }
   else {
      /* Context buffer is not involved. Read directly from stream. */
      len = readDirect(pctxt, pbuffer, bufSize, bufSize);
   }

   if ( len < 0 ) return LOG_RTERR(pctxt, (int)len);

   OSRT_CHECK_EVAL_DATE1(pctxt);
   /* LCHECKX (pctxt); */

   return len;
}


EXTRTMETHOD long rtxStreamReadDirect
(OSCTXT* pctxt, OSOCTET* pbuffer, OSSIZE bufSize)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len;

   if (pStream == 0 || pStream->read == 0 ||
      !(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW(pctxt, RTERR_NOTINIT);

   len = pStream->read(pStream, pbuffer, bufSize);

   if (len >= 0) {
      procInputData(pctxt, pbuffer, len);
   }
   else
      return LOG_RTERR(pctxt, (int)len);

   OSRT_CHECK_EVAL_DATE1(pctxt);
   /* LCHECKX (pctxt); */

   return len;
}


EXTRTMETHOD int rtxStreamSkip (OSCTXT* pctxt, OSSIZE bytes)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   int stat;

   if (pStream == 0 || !(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (pStream->skip == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      OSSIZE tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

      if (bytes <= tail) {
         /* new position presents in current buffer */
         pctxt->buffer.byteIndex += bytes;
         pStream->ioBytes += bytes;
         return 0;
      }
      else {
         pctxt->buffer.byteIndex = pctxt->buffer.size;
         bytes -= tail;
      }
   }

   stat = pStream->skip (pStream, bytes);
   if (stat >= 0) {
      pStream->ioBytes += bytes;
   }
   else
      return LOG_RTERR (pctxt, stat);

   return stat;
}

EXTRTMETHOD int rtxStreamGetIOBytes (OSCTXT* pctxt, OSSIZE* pPos)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   if (pStream == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   if (pPos != 0)
      *pPos = pStream->ioBytes;

   return 0;
}

EXTRTMETHOD int rtxStreamMark (OSCTXT* pctxt, OSSIZE readAheadLimit)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   int stat;

   if (pStream == 0)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (pStream->mark == 0 || pStream->reset == 0) {
      rtxErrAddStrParm (pctxt, "stream mark/reset");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   if (pStream->nextMarkOffset != 0) {
      /* move stream pos to end of PDU */
      stat = pStream->reset (pStream);
      if (pStream->skip != 0)
         stat = pStream->skip (pStream, pStream->nextMarkOffset);
   }

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      pctxt->savedInfo.byteIndex = pctxt->buffer.byteIndex;
   }

   stat = pStream->mark (pStream, readAheadLimit);
   if (stat < 0) return LOG_RTERR (pctxt, stat);
   pStream->flags |= OSRTSTRMF_POSMARKED;
   pStream->nextMarkOffset = 0;
   return 0;
}

EXTRTMETHOD int rtxStreamReset (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   int stat;

   if (pStream == 0)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (pStream->mark == 0 || pStream->reset == 0) {
      rtxErrAddStrParm (pctxt, "stream mark/reset");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   pStream->flags &= ~OSRTSTRMF_POSMARKED;

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      if (pctxt->savedInfo.byteIndex != OSRTSTRM_K_INVALIDMARK) {
         /* saved position presents in current buffer */
         pctxt->buffer.byteIndex = pctxt->savedInfo.byteIndex;
         return 0;
      }
      else {
         /* clear buffer */
         pctxt->buffer.byteIndex = 0;
         pctxt->buffer.size = 0;
      }
   }

   stat = pStream->reset (pStream);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   pStream->nextMarkOffset = 0;

   return 0;
}

EXTRTMETHOD OSBOOL rtxStreamMarkSupported (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   if (pStream == 0 || pStream->mark == 0 || pStream->reset == 0)
      return FALSE;
   return TRUE;
}

EXTRTMETHOD OSBOOL rtxStreamIsOpened (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   if (pStream != 0 &&
      ((pStream->flags & OSRTSTRMF_INPUT) ||
       (pStream->flags & OSRTSTRMF_OUTPUT)))
      return TRUE;
   return FALSE;
}

EXTRTMETHOD OSBOOL rtxStreamIsReadable (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   return (OSBOOL)(pStream != 0 && (pStream->flags & OSRTSTRMF_INPUT));
}

EXTRTMETHOD OSBOOL rtxStreamIsWritable (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   return (OSBOOL)(pStream != 0 && (pStream->flags & OSRTSTRMF_OUTPUT));
}

EXTRTMETHOD void rtxStreamSetCapture (OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   if (0 != pStream) {
      pStream->pCaptureBuf = pmembuf;

      /* If context-buffer stream and context buffer contains data,
         append data to capture buffer */
      if ((pStream->id == OSRTSTRMID_CTXTBUF ||
           pStream->id == OSRTSTRMID_MEMORY) && pctxt->buffer.size > 0) {
         rtxMemBufAppend
            (pStream->pCaptureBuf, pctxt->buffer.data, pctxt->buffer.size);
      }
   }
}

EXTRTMETHOD OSRTMEMBUF* rtxStreamGetCapture (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   return (0 != pStream) ? pStream->pCaptureBuf : 0;
}

int rtxiStreamGetPos (OSCTXT* pctxt, OSRTSTREAM *pStream, OSSIZE* ppos)
{
   int stat;

   if (pStream->getPos == 0) {
      rtxErrAddStrParm (pctxt, "stream get position");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   if (pStream->nextMarkOffset) {
      RTLDIAG1 (pctxt, "~D rtxStreamGetPos: set pos to nextMarkOffset\n");
      *ppos = pStream->nextMarkOffset;
   }
   else {
      stat = pStream->getPos (pStream, ppos);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }
   RTLDIAG2 (pctxt, "~D rtxStreamGetPos: position set to %ld\n", *ppos);

   return 0;
}

EXTRTMETHOD int rtxStreamGetPos (OSCTXT* pctxt, OSSIZE* ppos)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   return (pStream != 0) ? rtxiStreamGetPos (pctxt, pStream, ppos) :
      LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
}

int rtxiStreamSetPos (OSCTXT* pctxt, OSRTSTREAM *pStream, OSSIZE pos)
{
   int stat;

   if (pStream->setPos == 0) {
      rtxErrAddStrParm (pctxt, "stream set position");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   stat = pStream->setPos (pStream, pos);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (pStream->nextMarkOffset)
      pStream->nextMarkOffset = pos;

   return 0;
}

EXTRTMETHOD int rtxStreamSetPos (OSCTXT* pctxt, OSSIZE pos)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   return (pStream != 0) ? rtxiStreamSetPos (pctxt, pStream, pos) :
      LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
}

#endif /* _NO_STREAM */
