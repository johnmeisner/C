/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

   if (pctxt->buffer.dynamic && pctxt->buffer.data) {
      if (pStream->flags & OSRTSTRMF_BUFFERED)
         rtxMemSysFreePtr (pctxt, pctxt->buffer.data);

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

static int readInputStream (OSCTXT* pctxt, OSSIZE nbytes, OSBOOL blocking)
{
   int stat;
   OSSIZE tail = 0;
   OSRTSTREAM* pStream = pctxt->pStream;

   if (pctxt->buffer.data == 0) {
      stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* Not sure what this does.. */
   if (pctxt->buffer.size + pStream->bytesProcessed == pStream->segsize) {
      /* unexpected end of segment */
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
   }

   if (pctxt->buffer.byteIndex + nbytes > pStream->bufsize) {
      if (pStream->flags & OSRTSTRMF_FIXINMEM) {
         if (pStream->markedBytesProcessed > 0) {
            /* skip unmarked space */
            tail = pctxt->buffer.size - pStream->markedBytesProcessed;
            if (tail > 0)
               OSCRTLSAFEMEMMOVE (pctxt->buffer.data, pctxt->buffer.size,
                  pctxt->buffer.data + pStream->markedBytesProcessed, tail);

            pctxt->buffer.byteIndex -= pctxt->pStream->markedBytesProcessed;
            pStream->markedBytesProcessed = 0;
            pctxt->buffer.size = tail;
         }

         if (pctxt->buffer.byteIndex + nbytes <= pStream->bufsize)
            ; /* buffer has enough space now */
         else if (pctxt->buffer.dynamic) {
            OSOCTET* newbuf;
            OSSIZE extent = OSRTMAX (pctxt->pStream->bufsize, nbytes);

            pStream->bufsize += extent;

            newbuf = (OSOCTET*) rtxMemSysRealloc
               (pctxt, pctxt->buffer.data, pStream->bufsize);

            if (newbuf == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

            pctxt->buffer.data = newbuf;
            tail = pctxt->buffer.size;
         }
         else {
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
         }
      }
      else {
         /* move data to start of buffer */
         tail = pctxt->buffer.size - pctxt->buffer.byteIndex;

         OSCRTLSAFEMEMMOVE (pctxt->buffer.data, pctxt->buffer.size,
              pctxt->buffer.data + pctxt->buffer.byteIndex, tail);

         pStream->bytesProcessed += pctxt->buffer.byteIndex;

         pctxt->buffer.size = tail;
         pctxt->buffer.byteIndex = 0;
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
      }
   }
   else
      tail = pctxt->buffer.size;

   /* read data */
   {
      OSOCTET* pdata = pctxt->buffer.data + tail;
      OSSIZE nocts = pStream->bufsize - tail;
      stat = (blocking) ?
         (int) pStream->read (pStream, pdata, nocts) :
         (int) pStream->blockingRead (pStream, pdata, nocts);
   }
   if (stat == 0)
      return RTERR_ENDOFFILE;
   else if (stat < 0)
      return LOG_RTERR (pctxt, stat);

   pctxt->buffer.size += stat;

   if (pctxt->buffer.size + pctxt->pStream->bytesProcessed >
       pctxt->pStream->segsize)
   {
      /* set segment border */
      pctxt->buffer.size =
         pctxt->pStream->segsize - pctxt->pStream->bytesProcessed;
   }

   return 0;
}

static int checkInputBuffer (OSCTXT* pctxt, OSSIZE nbytes, OSBOOL blocking)
{
   while (pctxt->buffer.byteIndex + nbytes > pctxt->buffer.size) {
      int stat = readInputStream (pctxt, nbytes, blocking);
      if (stat < 0) {
         /* If EOF and unprocessed bytes remain in the buffer,
            return success */
         if (RTERR_ENDOFFILE == stat &&
             pctxt->buffer.byteIndex < pctxt->buffer.size)
         {
            stat = 0;
            break;
         }
         return LOG_RTERR (pctxt, stat);
      }
   }

   return 0;
}

static long readFromCtxtBufOrStream
(OSCTXT* pctxt, OSOCTET* pdata, OSSIZE nocts, OSBOOL blocking)
{
   /* Check if buffer contains number of bytes requested */
   if (pctxt->buffer.byteIndex + nocts > pctxt->buffer.size) {
      OSRTSTREAM* pStream = pctxt->pStream;
      OSSIZE numToRead = nocts;
      int stat;

      if (pctxt->buffer.size > pctxt->buffer.byteIndex) {
         OSSIZE tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
         if (0 != pdata) {
            OSCRTLSAFEMEMCPY (pdata, tail, OSRTBUFPTR(pctxt), tail);
            pdata += tail;
         }
         pctxt->buffer.byteIndex += tail;
         numToRead -= tail;
      }

      if (numToRead <= OSRTSTRM_K_BUFSIZE - MIN_STREAM_BACKOFF) {
         RTDIAG1 (pctxt, "~D readFromCtxtBufOrStream: fill context buffer\n");

         /* refill buffer */
         stat = checkInputBuffer (pctxt, numToRead, blocking);
         if (stat < 0) return LOG_RTERR (pctxt, stat);

         if (0 != pdata) {
            OSCRTLSAFEMEMCPY (pdata, numToRead, OSRTBUFPTR(pctxt), numToRead);
         }
         pctxt->buffer.byteIndex += numToRead;
      }
      else {
         RTDIAG1 (pctxt, "~D readFromCtxtBufOrStream: direct stream read\n");

         /* Requested number of bytes is larger than what context buffer
            can hold; therefore, we read directly from stream and then
            fill context buffer after read complete. */
         nocts = 0;
         numToRead -= MIN_STREAM_BACKOFF;
         pStream->bytesProcessed += numToRead + pctxt->buffer.byteIndex;
         pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;

         while (numToRead > 0) {
            /* read data from stream */
            stat = (blocking) ?
               (int) pStream->read (pStream, pdata, numToRead) :
               (int) pStream->blockingRead (pStream, pdata, numToRead);

            if (stat == 0) {
               /*if (nocts == 0) {
                  return LOG_RTERR (pctxt, RTERR_ENDOFFILE);
               }
               else*/ break;
            }
            else if (stat < 0)
               return LOG_RTERR (pctxt, stat);

            numToRead -= (OSSIZE) stat;
            nocts += (OSSIZE) stat;
            if (0 != pdata) pdata += stat;
         }

         /* clear buffer */
         pctxt->buffer.size = 0;
         pctxt->buffer.byteIndex = 0;
      }
   }
   else {
      /* Copy directly from context buffer to target buffer */
      if (0 != pdata) {
         OSCRTLSAFEMEMCPY (pdata, nocts, OSRTBUFPTR(pctxt), nocts);
      }
      pctxt->buffer.byteIndex += nocts;
   }

   /* Set last character in context */
   if (pctxt->buffer.byteIndex > 0) {
      pctxt->lastChar = pctxt->buffer.data[pctxt->buffer.byteIndex-1];
   }

   return (long)nocts;
}

EXTRTMETHOD long rtxStreamRead
(OSCTXT* pctxt, OSOCTET* pbuffer, OSSIZE bufSize)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len;

   if (pStream == 0 || pStream->read == 0 ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      len = readFromCtxtBufOrStream (pctxt, pbuffer, bufSize, FALSE);
   }
   else
      len = pStream->read (pStream, pbuffer, bufSize);

   if (len >= 0) {
      procInputData (pctxt, pbuffer, len);
   }
   else
      return LOG_RTERR (pctxt, (int)len);

   OSRT_CHECK_EVAL_DATE1 (pctxt);
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
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   len = pStream->read (pStream, pbuffer, bufSize);

   if (len >= 0) {
      procInputData (pctxt, pbuffer, len);
   }
   else
      return LOG_RTERR (pctxt, (int)len);

   OSRT_CHECK_EVAL_DATE1 (pctxt);
   /* LCHECKX (pctxt); */

   return len;
}

EXTRTMETHOD long rtxStreamBlockingRead
(OSCTXT* pctxt, OSOCTET* pbuffer, OSSIZE readBytes)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   long len;

   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE0 (pctxt);

   if (pStream == 0 || pStream->blockingRead == 0 ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);

   if ((pStream->flags & OSRTSTRMF_BUFFERED) &&
       pStream->id != OSRTSTRMID_CTXTBUF &&
       pStream->id != OSRTSTRMID_DIRECTBUF)
   {
      len = readFromCtxtBufOrStream (pctxt, pbuffer, readBytes, TRUE);
   }
   else
      len = pStream->blockingRead (pStream, pbuffer, readBytes);

   if (len >= 0) {
      procInputData (pctxt, pbuffer, len);
   }
   else
      return LOG_RTERR (pctxt, (int)len);

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
      RTDIAG1 (pctxt, "~I rtxStreamGetPos: set pos to nextMarkOffset\n");
      *ppos = pStream->nextMarkOffset;
   }
   else {
      stat = pStream->getPos (pStream, ppos);
      if (stat < 0) return LOG_RTERR (pctxt, stat);
   }
   RTDIAG2 (pctxt, "~I rtxStreamGetPos: position set to %ld\n", *ppos);

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
