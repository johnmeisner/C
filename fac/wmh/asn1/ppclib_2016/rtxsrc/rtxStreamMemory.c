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
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxHexDump.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStreamMemory.h"

typedef struct MemDesc {
   OSCTXT*  pctxt;
   OSBOOL   attached;
   OSSIZE   savedIndex;
} MemDesc;

#define MEMBUF_GROWTH 512

static long memWrite (OSRTSTREAM* pStream, const OSOCTET* data, size_t numocts)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   size_t bufsize = numocts;

   if (pStream->id != OSRTSTRMID_MEMORY || !(pStream->flags & OSRTSTRMF_OUTPUT))
      return RTERR_INVPARAM;

   /* If hex text output, required buffer size is doubled */
   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      bufsize *= 2;
   }

   if (mdesc->attached) { /* fixed memory buffer is used */
      if (bufsize > pctxt->buffer.size - pctxt->buffer.byteIndex)
         return RTERR_BUFOVFLW;
   }
   else { /* dynamic buffer */
      if (pctxt->buffer.byteIndex + numocts > pctxt->buffer.size) {
         size_t newSize = (pctxt->buffer.byteIndex + bufsize + MEMBUF_GROWTH);
         OSOCTET* newBuf = (OSOCTET*) rtxMemSysRealloc
            (pctxt, pctxt->buffer.data, newSize);
         if (newBuf == NULL) return RTERR_NOMEM;
         pctxt->buffer.data = newBuf;
         pctxt->buffer.size = newSize;
      }
   }

   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      size_t i;
      for (i = 0; i < numocts; i++) {
         rtxByteToHexChar
            (data[i], (char*)&pctxt->buffer.data[pctxt->buffer.byteIndex],
             pctxt->buffer.size - pctxt->buffer.byteIndex);

         pctxt->buffer.byteIndex += 2;
      }
   }
   else {
      OSCRTLMEMCPY (&pctxt->buffer.data[pctxt->buffer.byteIndex],
                    data, numocts);

      pctxt->buffer.byteIndex += numocts;
   }

   return 0;
}

static long memRead (OSRTSTREAM* pStream, OSOCTET* pbuffer, size_t bufSize)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;
   OSSIZE readBytes;

   if (pStream->id != OSRTSTRMID_MEMORY || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (pctxt->buffer.byteIndex >= pctxt->buffer.size)
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

   /* Read data directly from context buffer */
   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      int c;
      size_t nbits = 0;
      OSOCTET hbyte;

      if (bufSize > pctxt->buffer.size - pctxt->buffer.byteIndex) {
         bufSize = pctxt->buffer.size - pctxt->buffer.byteIndex;
      }

      /* Assumes even number of hex chars.  Whitespace between chars is OK */
      for (readBytes = 0;;) {
         /* Read a character from the stream */
         c = pctxt->buffer.data[pctxt->buffer.byteIndex++];

         /* Character successfully read */
         if (!OS_ISSPACE (c)) {
            if (c >= '0' && c <= '9')
               hbyte = (OSOCTET)(c - '0');
            else if (c >= 'A' && c <= 'F')
               hbyte = (OSOCTET)(c - 'A' + 10);
            else if (c >= 'a' && c <= 'f')
               hbyte = (OSOCTET)(c - 'a' + 10);
            else
               return RTERR_INVHEXS;

            if (nbits % 8 == 0)
               pbuffer[readBytes] = (OSOCTET)(hbyte << 4);
            else {
               pbuffer[readBytes++] |= (hbyte & 0xF);
               if ((size_t)readBytes >= bufSize) break;
            }

            nbits += 4;
         }
      }
   }
   else {
      readBytes = (bufSize < (pctxt->buffer.size - pctxt->buffer.byteIndex)) ?
         bufSize : (pctxt->buffer.size - pctxt->buffer.byteIndex);

      OSCRTLSAFEMEMCPY (pbuffer, bufSize, OSRTBUFPTR(pctxt), readBytes);

      pctxt->buffer.byteIndex += readBytes;
   }

   if (!(pStream->flags & OSRTSTRMF_BUFFERED)) {
      pStream->bytesProcessed += readBytes;
   }

   return (long)readBytes;
}

static int memSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;

   if (0 == skipBytes) return 0;

   if (pStream->id != OSRTSTRMID_MEMORY || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (pctxt->buffer.byteIndex >= pctxt->buffer.size)
      return LOG_RTERR (pctxt, RTERR_ENDOFBUF);

   if (skipBytes > (pctxt->buffer.size - pctxt->buffer.byteIndex))
      return LOG_RTERR (pctxt, RTERR_BUFOVFLW);

   pctxt->buffer.byteIndex += skipBytes;
   if (!(pStream->flags & OSRTSTRMF_BUFFERED)) {
      pStream->bytesProcessed += skipBytes;
   }

   return 0;
}

static int memClose (OSRTSTREAM* pStream)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;
   OSCTXT* pctxt = mdesc->pctxt;

   if (pStream->id != OSRTSTRMID_MEMORY)
      return RTERR_INVPARAM;

   rtxMemSysFreePtr (pctxt, pStream->extra);

   return 0;
}

static int memMark (OSRTSTREAM* pStream, size_t readAheadLimit)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_MEMORY || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   mdesc->savedIndex = mdesc->pctxt->buffer.byteIndex;
   pStream->readAheadLimit = readAheadLimit;
   pStream->markedBytesProcessed = pStream->bytesProcessed;

   if (pStream->flags & OSRTSTRMF_BUFFERED)
      pStream->markedBytesProcessed += mdesc->pctxt->buffer.byteIndex;

   return 0;
}

static int memReset (OSRTSTREAM* pStream)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;

   if (mdesc->savedIndex == OSRTSTRM_K_INVALIDMARK)
      return RTERR_INVPARAM;

   mdesc->pctxt->buffer.byteIndex = mdesc->savedIndex;
   pStream->bytesProcessed = pStream->markedBytesProcessed;
   mdesc->savedIndex = 0;
   pStream->readAheadLimit = INT_MAX;
   pStream->flags |= OSRTSTRMF_POSMARKED;
   return 0;
}

static int memGetPos (OSRTSTREAM* pStream, OSSIZE* ppos)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_MEMORY || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   *ppos = mdesc->pctxt->buffer.byteIndex;

   return 0;
}

static int memSetPos (OSRTSTREAM* pStream, OSSIZE pos)
{
   MemDesc* mdesc = (MemDesc*)pStream->extra;
   mdesc->pctxt->buffer.byteIndex = pos;
   pStream->bytesProcessed = pos;

   return 0;
}

EXTRTMETHOD int rtxStreamMemoryCreate (OSCTXT* pctxt, OSUINT16 flags)
{
   OSRTSTREAM* pStream;
   MemDesc* mdesc;

   if (0 != pctxt->pStream) {
      rtxStreamClose (pctxt);
   }
   if (0 == pctxt->pStream) {
      int stat = rtxStreamInit (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   pStream = pctxt->pStream;

   if (flags & OSRTSTRMF_OUTPUT) {
      pStream->write = memWrite;
      pStream->flush = 0;

      /* data is written directly to dynamic context buffer */
      if (0 == pctxt->buffer.size) {
         pctxt->buffer.size = MEMBUF_GROWTH;
      }
      pctxt->buffer.data = (OSOCTET*)
         rtxMemSysAlloc (pctxt, pctxt->buffer.size);
      if (0 == pctxt->buffer.data) return RTERR_NOMEM;

      pctxt->buffer.bitOffset = 8;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.dynamic = TRUE;
      pctxt->flags |= OSBUFSYSALLOC;
   }
   else if (flags & OSRTSTRMF_INPUT) {
      pStream->read = memRead;
      pStream->blockingRead  = memRead;
      pStream->skip = memSkip;
      pStream->mark = memMark;
      pStream->reset = memReset;
      pStream->getPos = memGetPos;
      pStream->setPos = memSetPos;
      pctxt->buffer.size = 0;
   }
   else
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   mdesc = (MemDesc*) rtxMemSysAlloc (pctxt, sizeof(MemDesc));
   if (mdesc == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   mdesc->pctxt = pctxt;
   mdesc->attached = FALSE;
   mdesc->savedIndex = 0;

   pStream->flags = (OSUINT16) (flags | OSRTSTRMF_POSMARKED);
   pStream->id    = OSRTSTRMID_MEMORY;
   pStream->extra = mdesc;
   pStream->close = memClose;

   return 0;
}

EXTRTMETHOD int rtxStreamMemoryAttach
(OSCTXT* pctxt, OSOCTET* pMemBuf, size_t bufSize, OSUINT16 flags)
{
   OSRTSTREAM* pStream;
   MemDesc* mdesc;

   if (pMemBuf == 0 || bufSize == 0)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (0 != pctxt->pStream) {
      rtxStreamClose (pctxt);
   }
   if (0 == pctxt->pStream) {
      int stat = rtxStreamInit (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   pStream = pctxt->pStream;

   mdesc = (MemDesc*) rtxMemSysAlloc (pctxt, sizeof(MemDesc));
   if (mdesc == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   mdesc->pctxt = pctxt;
   mdesc->attached = TRUE;
   mdesc->savedIndex = 0;

   /* data is read from or written directly to static context buffer */
   pctxt->buffer.data = pMemBuf;
   pctxt->buffer.size = bufSize;
   pctxt->buffer.bitOffset = 8;
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.dynamic = FALSE;

   if (flags & OSRTSTRMF_OUTPUT) {
      pStream->write = memWrite;
      pStream->flush = 0;
   }
   else if (flags & OSRTSTRMF_INPUT) {
      pStream->read = memRead;
      pStream->blockingRead = memRead;
      pStream->skip = memSkip;
      pStream->mark = memMark;
      pStream->reset = memReset;
      pStream->getPos = memGetPos;
      pStream->setPos = memSetPos;
   }
   else
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pStream->flags = (OSUINT16) (flags | OSRTSTRMF_POSMARKED);
   pStream->id    = OSRTSTRMID_MEMORY;
   pStream->extra = mdesc;
   pStream->close = memClose;

   return 0;
}


EXTRTMETHOD OSOCTET* rtxStreamMemoryGetBuffer (OSCTXT* pctxt, size_t* pSize)
{
   OSRTSTREAM *pStream = pctxt->pStream;
   /* MemDesc* mdesc; */

   if (pStream == 0 || 0 != (pStream->flags & OSRTSTRMF_OUTPUT)) {
      /* Assume output stream and return pointer to context buffer */
      if (pSize != 0) {
         OSSIZE len = pctxt->buffer.byteIndex;
         if (pctxt->buffer.bitOffset != 8 &&
             pctxt->buffer.bitOffset != 0) /* partial byte */ len++;

         *pSize = len;
      }
      return pctxt->buffer.data;
   }
   else if (pStream->extra == 0 ||
            (!(pStream->flags & OSRTSTRMF_OUTPUT) &&
             !(pStream->flags & OSRTSTRMF_INPUT)))
      return 0;

   if (OSRTSTREAM_ID (pctxt) == OSRTSTRMID_DIRECTBUF &&
       !(OSRTSTREAM_FLAGS (pctxt) & OSRTSTRMF_UNBUFFERED))
   {
      OSRTSTREAM *pUnderStream = ((DirBufDesc*)pStream->extra)->pUnderStream;
      if (pUnderStream == 0) return 0;
      /* mdesc = (MemDesc*)pUnderStream->extra; */
   }
   /*
   else
      mdesc = (MemDesc*)pStream->extra;
   */
   if (pStream->flags & OSRTSTRMF_INPUT) {
      if (pSize != 0) *pSize = pctxt->buffer.byteIndex;
      return pctxt->buffer.data;
   }
   else {
      /* Assume context buffer was used directly for output */
      /* *pSize = mdesc->curIndex; */
      if (pSize != 0) {
         OSSIZE len = pctxt->buffer.byteIndex;
         if (pctxt->buffer.bitOffset != 8 &&
             pctxt->buffer.bitOffset != 0) /* partial byte */ len++;

         *pSize = len;
      }
      return pctxt->buffer.data;
   }
}

EXTRTMETHOD int rtxStreamMemoryCreateReader
   (OSCTXT* pctxt, OSOCTET* pMemBuf, size_t bufSize)
{
   if (pMemBuf == 0 || bufSize == 0) {
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
   }
   else {
      return rtxStreamMemoryAttach
         (pctxt, pMemBuf, bufSize, OSRTSTRMF_INPUT);
   }
}

EXTRTMETHOD int rtxStreamMemoryCreateWriter
   (OSCTXT* pctxt, OSOCTET* pMemBuf, size_t bufSize)
{
   if (pMemBuf == 0) {
      pctxt->buffer.size = bufSize;
      return rtxStreamMemoryCreate (pctxt, OSRTSTRMF_OUTPUT);
   }
   else {
      return rtxStreamMemoryAttach
         (pctxt, pMemBuf, bufSize, OSRTSTRMF_OUTPUT);
   }
}

EXTRTMETHOD int rtxStreamMemoryResetWriter (OSCTXT* pctxt)
{
   OSRTSTREAM *pStream = pctxt->pStream;

   if (pStream == 0 || pStream->extra == 0 ||
       !(pStream->flags & OSRTSTRMF_OUTPUT))
      return LOG_RTERR (pctxt, RTERR_NOTINIT);

   pctxt->buffer.byteIndex = 0;

   return 0;
}

#endif /* _NO_STREAM */
