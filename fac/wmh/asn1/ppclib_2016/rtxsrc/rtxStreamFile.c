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
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/rtxHexDump.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStreamFile.h"

typedef struct FileDesc {
   OSCTXT* pctxt;
   FILE* fp;
   OSBOOL attached;
   long savedIndex;
} FileDesc;

static long fileWrite (OSRTSTREAM* pStream, const OSOCTET* data, size_t numocts)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_FILE || !(pStream->flags & OSRTSTRMF_OUTPUT))
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      size_t i;
      char tmpbuf[4];

      for (i = 0; i < numocts; i++) {
         rtxByteToHexChar (data[i], tmpbuf, sizeof(tmpbuf));
         fputs (tmpbuf, fdesc->fp);
      }
   }
   else {
      if (fwrite (data, 1, numocts, fdesc->fp) < numocts)
         return RTERR_WRITEERR;
   }

   return 0;
}

static long fileRead (OSRTSTREAM* pStream, OSOCTET* pbuffer, size_t bufSize)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;
   long readBytes = 0;

   if (pStream->id != OSRTSTRMID_FILE ||
       !(pStream->flags & OSRTSTRMF_INPUT) || 0 == bufSize)
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      int c;
      size_t nbits = 0;
      OSOCTET hbyte;

      /* Assumes even number of hex chars.  Whitespace between chars is OK */
      for (;;) {
         /* Read a character from the stream */
         c = fgetc (fdesc->fp);
         if (c == EOF) break;

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
      readBytes = (long)fread (pbuffer, 1, bufSize, fdesc->fp);
   }

   if (ferror (fdesc->fp) != 0)
      return RTERR_READERR;

   if (!(pStream->flags & OSRTSTRMF_BUFFERED)) {
      pStream->bytesProcessed += readBytes;
   }

   RTHEXDUMP (fdesc->pctxt, pbuffer, readBytes);

   return readBytes;
}

static int fileSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_FILE || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (fseek (fdesc->fp, (long)skipBytes, SEEK_CUR))
      return RTERR_READERR;

   pStream->bytesProcessed += skipBytes;

   return 0;
}

static int fileFlush (OSRTSTREAM* pStream)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_FILE) return RTERR_INVPARAM;

   fflush (fdesc->fp);
   return 0;
}

static int fileClose (OSRTSTREAM* pStream)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;

   if (pStream->id != OSRTSTRMID_FILE)
      return RTERR_INVPARAM;

   if (!fdesc->attached)
      fclose (fdesc->fp);

   rtxMemSysFreePtr (fdesc->pctxt, pStream->extra);

   return 0;
}

static int fileMark (OSRTSTREAM* pStream, size_t readAheadLimit)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;
   size_t offset = 0;
   long lstat = 0;

   if (pStream->id != OSRTSTRMID_FILE || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_BUFFERED)
      offset = fdesc->pctxt->buffer.size - fdesc->pctxt->buffer.byteIndex;

   lstat = ftell (fdesc->fp);
   if (lstat < 0) return RTERR_READERR;

   fdesc->savedIndex =  lstat - (long)offset;

   pStream->readAheadLimit = readAheadLimit;
   pStream->markedBytesProcessed = pStream->bytesProcessed;

   if (pStream->flags & OSRTSTRMF_BUFFERED)
      pStream->markedBytesProcessed += fdesc->pctxt->buffer.byteIndex;

   return 0;
}

static int fileReset (OSRTSTREAM* pStream)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;

   if (fseek (fdesc->fp, fdesc->savedIndex, SEEK_SET) != 0)
      return RTERR_READERR;

   pStream->bytesProcessed = pStream->markedBytesProcessed;
   fdesc->savedIndex = 0;
   pStream->readAheadLimit = INT_MAX;
   pStream->flags |= OSRTSTRMF_POSMARKED;
   return 0;
}

static int fileGetPos (OSRTSTREAM* pStream, size_t* ppos)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;
   size_t offset = 0;
   long savedIndex;

   if (pStream->id != OSRTSTRMID_FILE || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (pStream->flags & OSRTSTRMF_BUFFERED)
      offset = fdesc->pctxt->buffer.size - fdesc->pctxt->buffer.byteIndex;

   savedIndex = ftell (fdesc->fp) - (long)offset;
   if (savedIndex < 0) return RTERR_READERR;

   *ppos = (size_t) savedIndex;

   return 0;
}

static int fileSetPos (OSRTSTREAM* pStream, size_t pos)
{
   FileDesc* fdesc = (FileDesc*)pStream->extra;
   OSCTXT* pctxt = fdesc->pctxt;

   if (pStream->id != OSRTSTRMID_FILE || !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   if (fseek (fdesc->fp, (long) pos, SEEK_SET) != 0)
      return RTERR_READERR;

   pStream->bytesProcessed = pos;

   if (pctxt->pStream->flags & OSRTSTRMF_BUFFERED)
   {
      pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.size = 0;
   }

   return 0;
}

EXTRTMETHOD int rtxStreamFileOpen
(OSCTXT* pctxt, const char* pFilename, OSUINT16 flags)
{
   FILE* fp;
   const char* access;
   int stat;

   if (pFilename == 0)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (flags & OSRTSTRMF_OUTPUT) {
      access = "wb";
   }
   else if (flags & OSRTSTRMF_INPUT) {
      access = "rb";
      pctxt->buffer.size = 0;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.bitOffset = 8;
   }
   else
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   stat = rtxFileOpen (&fp, pFilename, access);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = rtxStreamFileAttach (pctxt, fp, flags);

   if (0 == stat) {
      FileDesc* fdesc = (FileDesc*) pctxt->pStream->extra;
      fdesc->attached = FALSE;
   }
   else {
      fclose (fp);
      return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

EXTRTMETHOD int rtxStreamFileAttach (OSCTXT* pctxt, FILE* pFile, OSUINT16 flags)
{
   OSRTSTREAM* pStream;
   FileDesc* fdesc;

   if (0 == pctxt->pStream) {
      int stat = rtxStreamInit (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   pStream = pctxt->pStream;

   if (pFile == 0 ||
      (!(flags & OSRTSTRMF_OUTPUT) && !(flags & OSRTSTRMF_INPUT)))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   fdesc = (FileDesc*) rtxMemSysAlloc (pctxt, sizeof(FileDesc));
   if (fdesc == NULL)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   fdesc->pctxt = pctxt;
   fdesc->fp = pFile;
   fdesc->attached = TRUE;
   fdesc->savedIndex = 0;

   if (flags & OSRTSTRMF_OUTPUT) {
      pStream->write = fileWrite;
      pStream->flush = fileFlush;
   }
   else if (flags & OSRTSTRMF_INPUT) {
      pStream->read  = fileRead;
      pStream->blockingRead  = fileRead;
      pStream->skip  = fileSkip;
      pStream->mark  = fileMark;
      pStream->reset  = fileReset;
      pStream->getPos = fileGetPos;
      pStream->setPos = fileSetPos;
      pctxt->buffer.size = 0;
   }
   pStream->flags = (OSUINT16) (flags | OSRTSTRMF_POSMARKED);
   pStream->id    = OSRTSTRMID_FILE;
   pStream->extra = fdesc;
   pStream->close = fileClose;

   if (pctxt->buffer.data == 0) {
      int stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

EXTRTMETHOD int rtxStreamFileCreateReader (OSCTXT* pctxt, const char* pFilename)
{
   return rtxStreamFileOpen (pctxt, pFilename, OSRTSTRMF_INPUT);
}

EXTRTMETHOD int rtxStreamFileCreateWriter (OSCTXT* pctxt, const char* pFilename)
{
   return rtxStreamFileOpen (pctxt, pFilename, OSRTSTRMF_OUTPUT);
}

#endif /* _NO_STREAM */
