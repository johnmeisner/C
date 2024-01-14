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

#ifndef _OS_NOSOCKET

#ifndef _NO_STREAM

#include <stdio.h>
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxHexDump.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxSocket.h"
#include "rtxsrc/rtxStreamSocket.h"

#ifndef _WIN32
#include <unistd.h> /* for sleep */
#endif

#define MAX_CONNECT_ATTEMPTS    3

typedef struct SocketDesc {
   OSCTXT *pctxt;
   OSRTSOCKET socket;
   OSBOOL ownSocket;
   OSUINT32 readTimeoutSecs;
} SocketDesc;

static long socketWrite
(OSRTSTREAM* pStream, const OSOCTET* data, size_t numocts)
{
   SocketDesc* sdesc;
   int stat;

   if (0 == pStream || pStream->id != OSRTSTRMID_SOCKET ||
       !(pStream->flags & OSRTSTRMF_OUTPUT))
      return RTERR_INVPARAM;

   sdesc = (SocketDesc*) pStream->extra;

   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      size_t i, idx = 0, bufsize = (numocts*2)+1;
      char* ptmpbuf = (char*) rtxMemAlloc (sdesc->pctxt, bufsize);
      if (0 == ptmpbuf) return RTERR_NOMEM;

      for (i = 0; i < numocts; i++) {
         rtxByteToHexChar (data[i], &ptmpbuf[idx], bufsize - idx);
         idx += 2;
      }

      stat = rtxSocketSend
         (sdesc->socket, (const OSOCTET*)ptmpbuf, bufsize-1);

      rtxMemFreePtr (sdesc->pctxt, ptmpbuf);
   }
   else {
      stat = rtxSocketSend (sdesc->socket, data, (int)numocts);
   }

   return stat;
}

static long socketRead (OSRTSTREAM* pStream, OSOCTET* pbuffer, size_t bufSize)
{
   SocketDesc* sdesc;

   if (0 == pStream || pStream->id != OSRTSTRMID_SOCKET ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   sdesc = (SocketDesc*) pStream->extra;

   if (pStream->flags & OSRTSTRMF_HEXTEXT) {
      int ret;
      size_t tmpbufSize = (bufSize*2)+1;
      char* ptmpbuf = (char*) rtxMemAlloc (sdesc->pctxt, tmpbufSize);
      if (0 == ptmpbuf) return RTERR_NOMEM;

      if (sdesc->readTimeoutSecs > 0) {
         ret = rtxSocketRecvTimed
            (sdesc->socket, (OSOCTET*)ptmpbuf, tmpbufSize, 
             sdesc->readTimeoutSecs);
      }
      else {
         ret = rtxSocketRecv (sdesc->socket, (OSOCTET*)ptmpbuf, tmpbufSize);
      }

      if (ret >= 0) {
         ret = rtxHexCharsToBin (ptmpbuf, (size_t)ret, pbuffer, bufSize);
      }

      rtxMemFreePtr (sdesc->pctxt, ptmpbuf);
      return ret;
   }
   else {
      if (sdesc->readTimeoutSecs > 0) {
         return rtxSocketRecvTimed
            (sdesc->socket, pbuffer, bufSize, sdesc->readTimeoutSecs);
      }
      else {
         return rtxSocketRecv (sdesc->socket, pbuffer, bufSize);
      }
   }
}

static long socketBlockingRead
(OSRTSTREAM* pStream, OSOCTET* pbuffer, size_t readBytes)
{
   SocketDesc* sdesc;
   size_t curlen = 0;
   long len;

   if (pStream->id != OSRTSTRMID_SOCKET ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   sdesc = (SocketDesc*) pStream->extra;

   while (curlen < readBytes) {
      if (sdesc->readTimeoutSecs > 0) {
         len = rtxSocketRecvTimed
            (sdesc->socket, pbuffer + curlen, (int)(readBytes - curlen),
             sdesc->readTimeoutSecs);
      }
      else {
         len = rtxSocketRecv
            (sdesc->socket, pbuffer + curlen, (int)(readBytes - curlen));
      }
      if (len < 0) return len; /* < 0 - error  */
      if (len == 0) {          /* ==0 - closed */
         RTDIAG1 (sdesc->pctxt, "socketBlockingRead: received zero bytes\n");
         break;
      }
      curlen += (size_t)len;
   }

   return (long)curlen;
}

static int socketSkip (OSRTSTREAM* pStream, size_t skipBytes)
{
   SocketDesc* sdesc;
   OSOCTET buf[128];
   long len;

   if (pStream->id != OSRTSTRMID_SOCKET ||
       !(pStream->flags & OSRTSTRMF_INPUT))
      return RTERR_INVPARAM;

   sdesc = (SocketDesc*) pStream->extra;

   while (skipBytes > 0) {
      if (sdesc->readTimeoutSecs > 0) {
         len = rtxSocketRecvTimed
            (sdesc->socket, (OSOCTET*)buf, OSRTMIN (sizeof(buf), skipBytes),
             sdesc->readTimeoutSecs);
      }
      else {
         len = rtxSocketRecv
            (sdesc->socket, (OSOCTET*)buf, OSRTMIN (sizeof(buf), skipBytes));
      }
      if (len < 0) return len;
      if (len == 0) break;     /* ==0 - closed */
      skipBytes -= len;
   }

   return 0;
}

static int socketFlush (OSRTSTREAM* pStream)
{
   OS_UNUSED_ARG (pStream); /* workaround for VC++ level 4 warning */
   return 0;
}

static int socketClose (OSRTSTREAM* pStream)
{
   SocketDesc* sdesc;
   int stat = 0;

   if (pStream->id != OSRTSTRMID_SOCKET)
      return RTERR_INVPARAM;

   sdesc = (SocketDesc*) pStream->extra;
   if (0 == sdesc) return 0;

   if (sdesc->ownSocket) {
      stat = rtxSocketClose (sdesc->socket);
   }
   rtxMemSysFreePtr (sdesc->pctxt, pStream->extra);
   pStream->extra = 0;

   return stat;
}

static int socketMark (OSRTSTREAM* pStream, size_t readAheadLimit)
{
   SocketDesc* sdesc = (SocketDesc*)pStream->extra;
   OSCTXT* pctxt = sdesc->pctxt;

   if (!(pStream->flags & OSRTSTRMF_BUFFERED)) {
      rtxErrAddStrParm (pctxt, "stream mark/reset");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   if (!(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
   pctxt->savedInfo.byteIndex = pctxt->buffer.byteIndex;
   pctxt->savedInfo.bitOffset = pctxt->buffer.bitOffset;
   pctxt->savedInfo.flags = pctxt->flags;
   pStream->readAheadLimit = readAheadLimit;
   pStream->markedBytesProcessed = pStream->bytesProcessed;
   return 0;
}

static int socketReset (OSRTSTREAM* pStream)
{
   SocketDesc* sdesc = (SocketDesc*)pStream->extra;
   OSCTXT* pctxt = sdesc->pctxt;

   if (!(pStream->flags & OSRTSTRMF_BUFFERED)) {
      rtxErrAddStrParm (pctxt, "stream mark/reset");
      return LOG_RTERRNEW (pctxt, RTERR_NOTSUPP);
   }
   if (!(pStream->flags & OSRTSTRMF_INPUT))
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   if (pctxt->savedInfo.byteIndex == OSRTSTRM_K_INVALIDMARK ||
       pctxt->buffer.byteIndex - pctxt->savedInfo.byteIndex >
       pStream->readAheadLimit)
      return LOG_RTERRNEW (pctxt, RTERR_INVPARAM);

   pctxt->buffer.byteIndex = pctxt->savedInfo.byteIndex;
   pctxt->buffer.bitOffset = pctxt->savedInfo.bitOffset;
   pctxt->flags = pctxt->savedInfo.flags;
   pStream->bytesProcessed = pStream->markedBytesProcessed;

   pctxt->savedInfo.byteIndex = OSRTSTRM_K_INVALIDMARK;
   pStream->readAheadLimit = 0;
   return 0;
}

EXTRTMETHOD int rtxStreamSocketAttach
(OSCTXT* pctxt, OSRTSOCKET sock, OSUINT16 flags)
{
   OSRTSTREAM *pStream;
   SocketDesc* sdesc;

   if (0 == pctxt->pStream) {
      int stat = rtxStreamInit (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }
   pStream = pctxt->pStream;

   if (sock == OSRTSOCKET_INVALID)
      return LOG_RTERRNEW (pctxt, RTERR_INVSOCKET);

   sdesc = (SocketDesc*) rtxMemSysAlloc (pctxt, sizeof(SocketDesc));
   if (sdesc == NULL) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   sdesc->pctxt = pctxt;
   sdesc->socket = sock;
   sdesc->ownSocket = TRUE;
   sdesc->readTimeoutSecs = 0;

   if (flags & OSRTSTRMF_INPUT) {
      pStream->read  = socketRead;
      pStream->blockingRead = socketBlockingRead;
      pStream->skip  = socketSkip;
      pStream->mark = socketMark;
      pStream->reset = socketReset;
   }
   if (flags & OSRTSTRMF_OUTPUT) {
      pStream->write = socketWrite;
      pStream->flush = socketFlush;
   }

   pStream->flags = flags;
   pStream->id    = OSRTSTRMID_SOCKET;
   pStream->extra = sdesc;
   pStream->close = socketClose;

   if (pctxt->buffer.data == 0 && 0 == (flags & OSRTSTRMF_UNBUFFERED)) {
      int stat = rtxStreamInitCtxtBuf (pctxt);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

EXTRTMETHOD int rtxStreamSocketClose (OSCTXT* pctxt)
{
   int stat;

#ifndef _WIN32
   /* We need to add the delay to avoid a bug in some Unix systems when
    * the socket is closed quickly before connection is actually accepted.
    * In this case 'accept()' returns the error and errno == ECONNABORTED.
    * Client (writer) program does not receive any errors from sockets in
    * this case. */
   sleep (2);
#endif
   /* this is called from inside rtxStreamClose (ED, 1/4/11)
   stat = socketClose (pctxt->pStream);
   if (0 != stat) return LOG_RTERR (pctxt, stat);
   */
   stat = rtxStreamClose (pctxt);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTRTMETHOD int rtxStreamSocketCreateWriter
(OSCTXT* pctxt, const char* host, int port)
{
   int i, stat;
   OSRTSOCKET sock;

   if (0 != pctxt->pStream) {
      return LOG_RTERR (pctxt, RTERR_STRMINUSE);
   }

   stat = rtxSocketCreate (&sock);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   RTDIAG3 (pctxt, "Connecting to %s:%i..", host, port);
   for (i = 0; i < MAX_CONNECT_ATTEMPTS; i++) {
      stat = rtxSocketConnect (sock, host, port);
      if (0 != stat) {
         if (i + 1 == MAX_CONNECT_ATTEMPTS) {
            RTDIAG2 (pctxt, "\nConnection failed, error code is %i\n", stat);
            return LOG_RTERR (pctxt, stat);
         }
      }
      else break;
   }
   RTDIAG1 (pctxt, "\nConnection established.\n");

   rtxStreamInit (pctxt);

   stat = rtxStreamSocketAttach (pctxt, sock, OSRTSTRMF_OUTPUT);

   if (0 != stat) return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTRTMETHOD int rtxStreamSocketSetOwnership (OSCTXT* pctxt, OSBOOL ownSocket)
{
   SocketDesc* sdesc;

   if (0 == pctxt->pStream || 0 == pctxt->pStream->extra) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }
   sdesc = (SocketDesc*)pctxt->pStream->extra;
   sdesc->ownSocket = ownSocket;
   return 0;
}

EXTRTMETHOD int rtxStreamSocketSetReadTimeout (OSCTXT* pctxt, OSUINT32 nsecs)
{
   SocketDesc* sdesc;

   if (0 == pctxt->pStream || 0 == pctxt->pStream->extra) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }
   sdesc = (SocketDesc*)pctxt->pStream->extra;
   sdesc->readTimeoutSecs = nsecs;
   return 0;
}
#endif /* _OS_NOSOCKET */

#endif /* _NO_STREAM */
