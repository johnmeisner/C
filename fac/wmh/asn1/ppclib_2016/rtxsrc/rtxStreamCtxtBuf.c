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
#include <stdio.h>
#include <string.h>
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStreamCtxtBuf.h"

#ifdef _DEBUG
#undef OSRTSTRM_K_BUFSIZE
#define OSRTSTRM_K_BUFSIZE 43 /* small enough to test flushing */
#endif

typedef struct CtxtBufDesc {
   OSCTXT* pctxt;
   OSRTSTREAM* pUnderStream; /* underlying stream object */
} CtxtBufDesc;

/* Read data from underlying stream into given buffer.  This function
   will read up to nbytes of data from the given stream.  It may read
   less if end-of-file is reached. */
static int readData (OSCTXT* pctxt, OSOCTET* pbuffer, size_t nbytes)
{
   OSRTSTREAM* pStream = pctxt->pStream;
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;

   size_t idx = 0;
   int len;

   OSRTASSERT (0 != pbuffer);
   OSRTASSERT (nbytes > 0);
   do {
      len = pUnderStream->read (pUnderStream, pbuffer + idx, nbytes - idx);
      if (len < 0) return LOG_RTERR (pctxt, len);
      else if (len > 0) {
         idx += len;
         pStream->ioBytes += len;
      }
   } while (len > 0 && idx < nbytes);

   RTDIAG2 (pctxt, "readData: read %d bytes to context buffer:\n", idx);
   RTHEXDUMP (pctxt, pbuffer, (OSUINT32)idx);

   /* If capture enabled, append data to capture buffer */
   if (0 != pStream->pCaptureBuf) {
      int stat = rtxMemBufAppend (pStream->pCaptureBuf, pbuffer, idx);
      if (0 != stat) LOG_RTERRNEW (pctxt, stat);
   }

   return (int)idx;
}

static long ctxtbufWrite
(OSRTSTREAM* pStream, const OSOCTET* pdata, size_t numocts)
{
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;

   if (pStream->id != OSRTSTRMID_CTXTBUF ||
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
   /* If number of octets is less then buffer size - put them into the buffer.
      Otherwise, write them into the stream directly. */
   if (pctxt->buffer.size > numocts) {
      OSCRTLSAFEMEMCPY ((void*)OSRTBUFPTR(pctxt), pctxt->buffer.size, pdata, numocts);
      pctxt->buffer.byteIndex += numocts;
   }
   else {
      /* Write directly to the stream. Buffer should be flushed */
      int stat = pUnderStream->write (pUnderStream, pdata, numocts);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

static long ctxtbufRead (OSRTSTREAM* pStream, OSOCTET* pdata, size_t size)
{
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSRTBuffer *pbuf = &pctxt->buffer;
   size_t nbytesToRead, startIndex;
   int len;

   if (pStream->id != OSRTSTRMID_CTXTBUF ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   RTDIAG2 (pctxt, "ctxtbufRead: start, size = %d\n", size);

   /* If current byte is partially consumed, adjust size by adding one */
   if (pbuf->bitOffset == 0) {
      pbuf->byteIndex++;
      pbuf->bitOffset = 8;
   }
   else if (pbuf->bitOffset != 8 && pdata != 0) {
      RTDIAG1 (pctxt, "Reading unaligned bytes\n");
      /* YG: in this case caller must pass increased size */
#if 0
      size++;
      RTDIAG2 (pctxt, "Partial byte, size increased to %d\n", size);
#endif
   }

   /* Determine if read is necessary.  If there are enough bytes remaining
      in the context buffer to satisfy the request, data does not need to
      be read from the underlying stream. */
   if (pbuf->byteIndex + size <= pbuf->size) {
      if (0 != pdata) {
         OSCRTLSAFEMEMCPY (pdata, size, pbuf->data + pbuf->byteIndex, size);
         pbuf->byteIndex += size;
         RTDIAG3 (pctxt, "Moved %d bytes to buffer %x\n", size, pdata);
         RTHEXDUMP (pctxt, pdata, (OSUINT32)size);
         RTDIAG2 (pctxt, "buffer.byteIndex = %d\n", pbuf->byteIndex);
      }
      RTDIAG2 (pctxt, "ctxtbufRead: end, returning size %d\n", size);
      return (int)size;
   }
   pStream->bytesProcessed += pbuf->byteIndex;

   /* Move bytes at end of buffer forward to beginning */
   if (pbuf->byteIndex > 0) {
      size_t nbytesToMove = (pbuf->size >= pbuf->byteIndex) ?
         pbuf->size - pbuf->byteIndex : 0;

      if (nbytesToMove > 0) {
         if (0 != pdata) {
            /* Copy data to given buffer, not context buffer */
            OSCRTLSAFEMEMCPY (pdata, OSRTMIN(nbytesToMove,size),
                  pbuf->data + pbuf->byteIndex, nbytesToMove);
            pbuf->size = 0; /* context buffer is now empty */
            RTDIAG2 (pctxt, "Moved %d bytes to given buffer\n", nbytesToMove);
            RTDIAG1 (pctxt, "Context buffer is now empty\n");
            RTDIAG1 (pctxt, "Output buffer contents:\n");
            RTHEXDUMP (pctxt, pdata, (OSUINT32)nbytesToMove);
         }
         else {
            OSCRTLSAFEMEMCPY
               (pbuf->data, pbuf->size, pbuf->data + pbuf->byteIndex,
                nbytesToMove);
            pbuf->size = pbuf->byteIndex;
            RTDIAG2 (pctxt, "Moved %d bytes up in context buffer\n",
                     nbytesToMove);
            RTDIAG1 (pctxt, "Context buffer contents:\n");
            RTHEXDUMP (pctxt, pbuf->data, (OSUINT32)pbuf->size);
         }
      }
      else {
         pbuf->size = 0;
         RTDIAG1 (pctxt, "Context buffer is now empty\n");
      }
      pbuf->byteIndex = 0;
      startIndex = nbytesToMove;
   }
   else startIndex = 0;

   /* Read more data into context buffer */

   /* If user provided buffer (pdata != NULL) :
        If requested number of bytes (size) > context buffer size,
          Read directly into user buffer and leave context buffer empty
        Else
          Fill context buffer and copy number of request bytes to user buffer
    */
   if (0 != pdata) {
      nbytesToRead = size - startIndex;
      if (nbytesToRead >= (pStream->bufsize - startIndex)) {
         /* read directly from underlying stream */
         len = readData (pctxt, pdata + startIndex, nbytesToRead);
         if (len > 0) len += (int)startIndex;
         RTDIAG3 (pctxt, "Read %d bytes into buffer %x:\n", len, pdata);
         RTHEXDUMP (pctxt, pdata, len);
      }
      else {
         size_t bytesAvailable;

         /* fill context buffer */
         nbytesToRead = (pStream->bufsize - startIndex);
         len = readData (pctxt, pbuf->data + startIndex, nbytesToRead);
         if (len > 0) pbuf->size += len;

         RTDIAG3 (pctxt, "Read %d bytes into ctxt buf @ index %d:\n",
                  len, startIndex);

         /* copy data from context buffer into user buffer */
         bytesAvailable = pbuf->size - pbuf->byteIndex;
         nbytesToRead = size - startIndex;
         if (nbytesToRead <= bytesAvailable) {
            len = (int)size;
         }
         else {
            /* There are not enough bytes available to satisfy the read
               request */
            nbytesToRead = bytesAvailable;
            len = RTERR_ENDOFBUF;
            RTDIAG1 (pctxt, "ERROR: not enough bytes available for read\n");
            RTDIAG2 (pctxt, "nbytesToRead adjusted to %d\n", (int)nbytesToRead);
         }
         OSCRTLSAFEMEMCPY
            (pdata + startIndex, size, pbuf->data + pbuf->byteIndex, nbytesToRead);

         pbuf->byteIndex += nbytesToRead;

         RTDIAG3 (pctxt, "Copied %d bytes from context buffer to buffer %x:\n",
                  nbytesToRead, pdata);
         RTHEXDUMP (pctxt, pdata, (OSUINT32)pbuf->byteIndex);
      }
   }

   /* If user did NOT provide buffer (pdata == NULL) :
        Fill context buffer
        Set context buffer size to number bytes read + remainder
    */
   else {
      nbytesToRead = (pStream->bufsize - startIndex);

      RTDIAG3 (pctxt, "Read %d bytes into ctxt buf @ index %d\n",
               nbytesToRead, startIndex);

      len = readData (pctxt, pbuf->data + startIndex, nbytesToRead);

      RTDIAG2 (pctxt, "Number of actual bytes read = %d\n", len);
      if (len > 0) {
         len += (int)startIndex;
      }
      pbuf->size = len;
      len = (int)OSRTMIN (size, pbuf->size);
   }

   RTDIAG1 (pctxt, "Current contents of context buffer:\n");
   RTHEXDUMP (pctxt, pbuf->data, (OSUINT32)pbuf->size);

   RTDIAG2 (pctxt, "ctxtbufRead: end, returning len %d\n", len);

   return (len >= 0) ? len : LOG_RTERR (pctxt, len);
}

static int ctxtbufClose (OSRTSTREAM* pStream)
{
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   OSCTXT* pctxt = mdesc->pctxt;
   int stat = 0, stat2 = 0;

   if (pStream->id != OSRTSTRMID_CTXTBUF)
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_OUTPUT) {
      stat = pStream->flush (pStream);
   }
   else if (!(pStream->flags & OSRTSTRMF_INPUT))
      stat = RTERR_NOTINIT;

   stat2 = pUnderStream->close (pUnderStream);

   rtxMemFreeType (mdesc->pctxt, pStream->extra);

   if (0 != stat) return LOG_RTERR (pctxt, stat);
   else if (0 != stat2) return LOG_RTERR (pctxt, stat2);
   else return 0;
}

static int ctxtbufFlush (OSRTSTREAM* pStream)
{
   CtxtBufDesc* mdesc = 0;
   OSRTSTREAM* pUnderStream = 0;
   OSCTXT* pctxt = 0;
   int stat = 0;
   OSRTBuffer* buffer;

   if (pStream == 0 || !(pStream->flags & OSRTSTRMF_OUTPUT))
      return RTERR_INVPARAM;

   mdesc = (CtxtBufDesc*)pStream->extra;
   pUnderStream = mdesc->pUnderStream;
   pctxt = mdesc->pctxt;

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

static int ctxtbufMark (OSRTSTREAM* pStream, size_t readAheadLimit)
{
   OSRTCHECKPARAM (0 != pStream);
   OSRTCHECKPARAM (0 != readAheadLimit);
   return RTERR_MARKNOTSUP;
}

static int ctxtbufReset (OSRTSTREAM* pStream)
{
   OSRTCHECKPARAM (0 != pStream);
   return RTERR_MARKNOTSUP;
}

static int ctxtbufSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
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
      else {
         /*
         stat = rtxStreamCtxtBufPreRead (pctxt, skipBytes);
         if (stat < 0) return stat;
         */
      }
   }

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

int rtxStreamCtxtBufCreate (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream;
   OSRTSTREAM *pUnderStream;
   CtxtBufDesc* mdesc;
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
   pStream->flags |= OSRTSTRMF_BUFFERED;

   /* create buffer, if it is not created yet */
   if (pctxt->buffer.data == 0) {
      if ((stat = rtxInitContextBuffer (pctxt, 0, OSRTSTRM_K_BUFSIZE)) != 0)
         return LOG_RTERR (pctxt, stat);
   }
   else pctxt->buffer.byteIndex = 0; /* Reset to beginning */

   pctxt->pStream->bufsize = OSRTSTRM_K_BUFSIZE;
   pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
   pctxt->buffer.size = 0;

   if (pUnderStream->flags & OSRTSTRMF_OUTPUT) {
      pStream->write = ctxtbufWrite;
      pStream->flush = ctxtbufFlush;
   }
   else if (pUnderStream->flags & OSRTSTRMF_INPUT) {
      pStream->read  = ctxtbufRead;
      pStream->blockingRead  = ctxtbufRead;
      pStream->skip  = ctxtbufSkip;
      pStream->mark = ctxtbufMark;
      pStream->reset = ctxtbufReset;
   }
   pStream->close = ctxtbufClose;

   mdesc = rtxMemAllocType (pctxt, CtxtBufDesc);
   if (mdesc == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   mdesc->pctxt = pctxt;
   mdesc->pUnderStream = pUnderStream;

   pStream->flags = pUnderStream->flags;
   pStream->flags |= OSRTSTRMF_BUFFERED;
   pStream->id    = OSRTSTRMID_CTXTBUF;
   pStream->extra = mdesc;
   pStream->pCaptureBuf = pUnderStream->pCaptureBuf;

   if (pStream->flags & OSRTSTRMF_INPUT) {
      stat = readData (pctxt, pctxt->buffer.data, pStream->bufsize);
      if (stat < 0) return LOG_RTERR (pctxt, stat);

      pctxt->buffer.size = stat;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.bitOffset = 8;
   }

   return 0;
}

int rtxStreamCtxtBufCheck (OSCTXT* pctxt)
{
   if (OSRTISSTREAM (pctxt)) {
      int stat;

      if (OSRTSTREAM_ID (pctxt) != OSRTSTRMID_CTXTBUF &&
          OSRTSTREAM_ID (pctxt) != OSRTSTRMID_ZLIB) {
         stat = rtxStreamCtxtBufCreate (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      return 0;
   }
   return RTERR_NOTINIT;
}

int rtxStreamCtxtBufFree (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   CtxtBufDesc* mdesc = (CtxtBufDesc*)pStream->extra;
   OSRTSTREAM* pUnderStream = mdesc->pUnderStream;
   int stat = 0;

   if (pStream->id != OSRTSTRMID_CTXTBUF)
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_OUTPUT) {
      stat = pStream->flush (pStream);
   }
   else if (!(pStream->flags & OSRTSTRMF_INPUT))
      stat = RTERR_NOTINIT;

   mdesc->pctxt->pStream = pUnderStream;

   rtxMemFreeType (mdesc->pctxt, pStream);
   pStream = 0;

   rtxMemFreeType (mdesc->pctxt, mdesc);
   mdesc = 0;

   if (pctxt->buffer.dynamic && pctxt->buffer.data) {
      rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
      pctxt->buffer.data = 0;
   }

   if (0 != stat) return LOG_RTERR (pctxt, stat);
   else return 0;
}

#ifdef TEST
#include "rtxsrc/rtxStreamFile.h"

int main (int argc, const char** argv)
{
   OSCTXT ctxt;
   OSOCTET savedByte;
   int    len, stat;
   const char* filename = 0;

   if (argc > 1) {
      if (argv[1][0] != '-') {
         filename = argv[1];
      }
   }
   if (0 == filename) {
      printf ("usage: streamtest <filename>\n");
      return 0;
   }

   /* Create file stream reader */

   /* Init context */

   stat = rtxInitContext (&ctxt);
   if (0 != stat) {
      printf ("Context initialization failed.\n");
      rtxErrPrint (&ctxt);
      return stat;
   }

   /* Create input source object */

   stat = rtxStreamFileCreateReader (&ctxt, filename);
   if (stat != 0) {
      printf ("Error creating file input stream.\n");
      rtxErrPrint (&ctxt);
      return stat;
   }

   /* Create context-buffer stream on top */

   stat = rtxStreamCtxtBufCheck (&ctxt);
   if (0 != stat) {
      printf ("Error creating context-buffer stream.\n");
      rtxErrPrint (&ctxt);
      return stat;
   }

   /* Read and print contents of file */

   while ((len = rtxStreamRead (&ctxt, 0, ctxt.pStream->bufsize - 1)) > 0) {
      printf ("%d bytes read from stream\n", len);
      savedByte = ctxt.buffer.data[len];
      ctxt.buffer.data[len] = '\0';
      printf ((const char*)OSRTBUFPTR(&ctxt));
      printf ("\n");
      ctxt.buffer.data[len] = savedByte;
      ctxt.buffer.byteIndex += len;
   }

   if (len < 0) {
      printf ("Error reading data from stream.\n");
      rtxErrPrint (&ctxt);
   }

   return len;
}
#endif

#endif /* _NO_STREAM */
