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

#ifndef _OS_NOSOCKET

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxSOAP.h"
#include "rtxsrc/rtxSocket.h"
#include "rtxsrc/rtxStreamSocket.h"
#include "rtxsrc/rtxCtype.h"

#define MAX_CONNECT_ATTEMPTS    3

#ifndef OSRTSOAP_K_BUFSIZE
#define OSRTSOAP_K_BUFSIZE 1500 /* common MTU */
#endif

#ifdef _DEBUG
#undef OSRTSOAP_K_BUFSIZE
#define OSRTSOAP_K_BUFSIZE 43 /* small enough to test flushing */
#endif /* _DEBUG */

/* Function to convert chunked block size to size_t variable */
static int parseChunkSize (char* hexstr, size_t* psize)
{
   size_t lsize = 0, i = 0;
   char*  bufp;

   if (0 == psize) return RTERR_INVPARAM;

   /* ignore chunk-extension */
   bufp = strchr (hexstr, ';');
   if (0 != bufp) *bufp = '\0';

   while (hexstr[i] != 0) {
      char c = hexstr[i++];
      if (c >= '0' && c <= '9')
         lsize = (lsize * 16) + (size_t)(c - '0');
      else if (c >= 'A' && c <= 'F')
         lsize = (lsize * 16) + (size_t)(c - 'A' + 10);
      else if (c >= 'a' && c <= 'f')
         lsize = (lsize * 16) + (size_t)(c - 'a' + 10);
      else if (!OS_ISSPACE(c)) {
         *psize = 0;
         return RTERR_INVHEXS;
      }
   }

   *psize = lsize;

   return 0;
}


/**
 * This function writes an HTTP header field using the given key and value
 * to the HTTP message being currently written on the given SOAP connection.
 * This includes writing the \r\n to end the field.
 */
static int rtxHttpWriteHdrField (OSSOAPCONN* pSoapConn, const char* key,
                          const char* value);


typedef enum HttpMessageType
   {HttpMessageTypeRequest, HttpMessageTypeResponse} HttpMessageType;

/**
 * Write an HTPP response or request (POST only) header.
 * @param pSoapConn
 * @param msgType indicates whether to write a response or request header
 * @param count The number of octets to encode as the Content-Length (the size
 *    of the HTPP message body)
 */
static int rtxHttpWriteHeader (OSSOAPCONN* pSoapConn, HttpMessageType msgType,
                        size_t count);

/**
 * Send the given SOAP message (possibly living in the context buffer)
 * via an HTTP request or response, depending on the value of msgType.
 */
int rtxSoapSendHttp2 (OSSOAPCONN* pSoapConn, HttpMessageType msgType,
                  const OSUTF8CHAR* soapMsg);

int rtxSoapInitConn (OSSOAPCONN* pSoapConn, OSCTXT* pctxt,
                     OSSoapVersion soapv, const char* url)
{
   char lbuf[200];
   const char* s;
   char* pbuf;
   int i, n;

#ifndef _NO_STREAM
   int stat;
#endif /* _NO_STREAM */

   OSRTASSERT (0 != pSoapConn);
   pSoapConn->pctxt = pctxt;
   pSoapConn->socket = OSRTSOCKET_INVALID;
   pSoapConn->soapVersion = soapv;
   pSoapConn->endpoint = url;
   pSoapConn->soapAction = 0;
   pSoapConn->host = 0;
   pSoapConn->port = 80;
   pSoapConn->path = 0;
   pSoapConn->keepAlive = FALSE;
   pSoapConn->chunked = FALSE;
   pSoapConn->contentLength = 0;
   pSoapConn->recvTimeoutSecs = 0;

#ifndef _NO_STREAM
   /* init stream block within context */
   if ((stat = rtxStreamInit (pctxt)) != 0) {
      return LOG_RTERR (pctxt, stat);
   }
#endif /* _NO_STREAM */

   /* parse URL */
   if (0 == url || 0 == *url)
      return 0;

   s = strchr (url, ':');
   if (s && s[1] == '/' && s[2] == '/')
      s += 3;
   else
      s = url;

   /* parse host name from URL */
   n = (int)strlen (s);
   if (n >= (int)sizeof(lbuf))
      n = sizeof(lbuf) - 1;

   for (i = 0; i < n; i++) {
      lbuf[i] = s[i];
      if (s[i] == '/' || s[i] == ':')
         break;
   }
   lbuf[i] = '\0';

   pbuf = (char*)rtxMemAlloc (pctxt, i+1);
   if (pbuf == 0)
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
   rtxStrcpy (pbuf, i+1, lbuf);
   pSoapConn->host = pbuf;

   /* parse port number */
   if (s[i] == ':') {
      pSoapConn->port = (int) atol (s + i + 1);
      for (i++; i < n; i++)
         if (s[i] == '/')
            break;
   }

   /* parse path */
   if (s[i]) {
      size_t bufsize = strlen(s+i+1) + 1;
      pbuf = (char*)rtxMemAlloc (pctxt, bufsize);
      if (pbuf == 0) {
         rtxMemFreePtr (pctxt, pSoapConn->host);
         return LOG_RTERRNEW (pctxt, RTERR_NOMEM);
      }
      rtxStrcpy (pbuf, bufsize, s+i+1);
      pSoapConn->path = pbuf;
   }

   return 0;
}

int rtxSoapAcceptConn (OSRTSOCKET listenSocket, OSSOAPCONN* pSoapConn)
{
   OSIPADDR destIP;
   int      destPort, stat;
   OSCTXT*  pctxt = pSoapConn->pctxt;

   stat = rtxSocketAccept
      (listenSocket, &pSoapConn->socket, &destIP, &destPort);

   if (0 != stat) {
      return LOG_RTERR (pctxt, stat);
   }

   if (rtxDiagEnabled (pctxt)) {
      char addr[100];

      if (rtxSocketAddrToStr (destIP, addr, sizeof (addr)) != 0)
         rtxStrcpy (addr, sizeof(addr), "<unknown>");

      RTDIAG3 (pctxt, "Connection established to %s:%d\n", addr, destPort);
   }

#ifndef _NO_STREAM
   stat = rtxStreamSocketAttach
      (pctxt, pSoapConn->socket,
       OSRTSTRMF_OUTPUT|OSRTSTRMF_INPUT|OSRTSTRMF_UNBUFFERED);

   if (0 == stat && pSoapConn->recvTimeoutSecs > 0) {
      stat = rtxStreamSocketSetReadTimeout (pctxt, pSoapConn->recvTimeoutSecs);
   }

   rtxStreamRemoveCtxtBuf (pctxt);

   if (0 != stat) return LOG_RTERR (pctxt, stat);
#endif /* _NO_STREAM */

   return 0;
}

int rtxSoapConnect (OSSOAPCONN* pSoapConn)
{
   int i, stat;
   OSCTXT* pctxt = pSoapConn->pctxt;

   if (OSRTSOCKET_INVALID == pSoapConn->socket) {
      stat = rtxSocketCreate (&pSoapConn->socket);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   RTDIAG3 (pctxt, "Connecting to %s:%i..", pSoapConn->host, pSoapConn->port);
   for (i = 0; i < MAX_CONNECT_ATTEMPTS; i++) {
      stat = rtxSocketConnectTimed
         (pSoapConn->socket, pSoapConn->host, pSoapConn->port, 15);
      if (0 != stat) {
         if (i + 1 == MAX_CONNECT_ATTEMPTS) {
            RTDIAG2 (pctxt, "\nConnection failed, error code is %i\n", stat);
            return LOG_RTERR (pctxt, stat);
         }
      }
      else break;
   }
   RTDIAG1 (pctxt, "\nConnection established.\n");

#ifndef _NO_STREAM
   stat = rtxStreamSocketAttach
      (pctxt, pSoapConn->socket,
       OSRTSTRMF_OUTPUT|OSRTSTRMF_INPUT|OSRTSTRMF_UNBUFFERED);

   if (0 == stat && pSoapConn->recvTimeoutSecs > 0) {
      stat = rtxStreamSocketSetReadTimeout (pctxt, pSoapConn->recvTimeoutSecs);
   }

   rtxStreamRemoveCtxtBuf (pctxt);

   if (0 != stat) return LOG_RTERR (pctxt, stat);
#endif /* _NO_STREAM */

   return 0;
}

int rtxSoapRecvHttp (OSSOAPCONN* pSoapConn)
{
   char lbuf[1024], *s;
   char header[1024];
   int  httpStat, stat;
   OSCTXT* pctxt = pSoapConn->pctxt;

#ifdef _NO_STREAM
   if (pctxt->buffer.size < OSRTSOAP_K_BUFSIZE) {
      /* free context buffer */
      if (pctxt->buffer.dynamic)
         rtxMemFreePtr (pctxt, pctxt->buffer.data);

      pctxt->buffer.data = 0;
   }

   if (pctxt->buffer.data == 0) {
      stat = rtxInitContextBuffer (pctxt, 0, OSRTSOAP_K_BUFSIZE);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.size = 0;
#endif /* _NO_STREAM */

   do {
      stat = rtxSoapRecvHttpLine (pSoapConn, lbuf, sizeof(lbuf));
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

      for (;;) {
         stat = rtxSoapRecvHttpLine (pSoapConn, header, sizeof(header));
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

         if (header[0] == '\0') break;

         s = strchr (header, ':');
         if (0 != s) {
            *s = '\0';
            do s++; while (*s && *s <= 32);
         } else
            return LOG_RTERR (pSoapConn->pctxt, RTERR_HTTPERR);

         stat = rtxHttpParseHdr (pSoapConn, header, s);
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
      }

      if ((s = strchr (lbuf, ' ')) != 0)
         httpStat = strtoul (s, NULL, 10);
      else
         httpStat = 0;
   }
   while (httpStat == 100);

   if (httpStat == 0 || httpStat == 200 || httpStat == 400 || httpStat == 500)
      return 0;
   else {
      rtxErrNewNode (pctxt);
      rtxErrAddIntParm (pctxt, httpStat);
      return LOG_RTERR (pctxt, RTERR_HTTPERR);
   }
}

int rtxSoapRecvHttpContent (OSSOAPCONN* pSoapConn, OSOCTET** ppbuf)
{
   int stat = 0;
   OSCTXT* pctxt = pSoapConn->pctxt;
   /*
   printf("\nHello again from rtxSoapRecvHttpContent\n\n");
   */
   if (pSoapConn->contentLength > 0) {
#ifndef _NO_STREAM
      if (0 == ppbuf)
         return 0;
#else /* _NO_STREAM */
      OSOCTET* ptm = 0;

      if (0 == ppbuf) {
         ppbuf = &ptm;

         if (pSoapConn->contentLength <= OSRTSOAP_K_BUFSIZE) {
            ptm = pctxt->buffer.data;
         }
      }
#endif /* _NO_STREAM */

      if (0 == *ppbuf) {
         *ppbuf = (OSOCTET*) rtxMemAlloc (pctxt, pSoapConn->contentLength + 1);
         if (*ppbuf == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);
      }

      /* Read content */

#ifndef _NO_STREAM
      stat = rtxStreamRead (pSoapConn->pctxt, *ppbuf,
                                    pSoapConn->contentLength);
#else /* _NO_STREAM */
      {
         size_t sz = pSoapConn->contentLength;
         OSOCTET* p = *ppbuf;

         if (pctxt->buffer.byteIndex < pctxt->buffer.size) {
            size_t tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
            if (tail > sz) tail = sz;

            OSCRTLSAFEMEMCPY
               (p, sz, pctxt->buffer.data + pctxt->buffer.byteIndex, tail);

            sz -= tail;
            p += tail;
            pctxt->buffer.byteIndex += tail;
         }

         while (sz) {
            if (pSoapConn->recvTimeoutSecs > 0) {
               stat = rtxSocketRecvTimed
                  (pSoapConn->socket, p, sz, pSoapConn->recvTimeoutSecs);
            }
            else {
               stat = rtxSocketRecv (pSoapConn->socket, p, sz);
            }
            if (stat < 0) return LOG_RTERR (pctxt, stat);
            else if (stat == 0) return LOG_RTERR (pctxt, RTERR_READERR);

            p += stat;
            sz -= stat;
         }

         if (ppbuf == &ptm) {
            if (pctxt->buffer.data != ptm && pctxt->buffer.dynamic)
               rtxMemFreePtr (pctxt, pctxt->buffer.data);

            pctxt->buffer.data = ptm;
            pctxt->buffer.byteIndex = 0;
            pctxt->buffer.size = pSoapConn->contentLength;
            pctxt->buffer.dynamic = TRUE;
         }
      }
#endif /* _NO_STREAM */

      /* Null terminate */

      (*ppbuf)[pSoapConn->contentLength] = (OSOCTET)'\0';
   }
   else {
      /* If chunked, read chunk by chunk.  Otherwise, read in 1024 byte chunks
         until we run out of input (connection closed).
      */
      char lbuf[80];
      size_t chunkSize;
      OSRTMEMBUF membuf;
#ifndef _NO_STREAM
      int  nbytes;
      if (0 == ppbuf)
         return LOG_RTERR (pSoapConn->pctxt, RTERR_NOTSUPP);
#endif /* _NO_STREAM */

      /* read 'chunked' content */
      pSoapConn->chunked = FALSE;
      pSoapConn->contentLength = 0;

      if (pSoapConn->chunked) {
         stat = rtxSoapRecvHttpLine(pSoapConn, lbuf, sizeof(lbuf));
         if (0 != stat) return LOG_RTERR(pSoapConn->pctxt, RTERR_HTTPERR);

         stat = parseChunkSize(lbuf, &chunkSize);
         if (stat < 0) return LOG_RTERR(pSoapConn->pctxt, stat);
      }
      else chunkSize = 1024;

      rtxMemBufInit (pSoapConn->pctxt, &membuf, chunkSize + 1);

      /* read chunks */
      while (chunkSize > 0) {
         /* expand dynamic buffer */
         stat = rtxMemBufPreAllocate (&membuf, chunkSize+1);
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

         /* read data */
#ifndef _NO_STREAM
         nbytes = rtxStreamRead
            (pSoapConn->pctxt, &membuf.buffer[membuf.usedcnt], chunkSize);
         if (nbytes < 0) return LOG_RTERR (pSoapConn->pctxt, nbytes);

         membuf.usedcnt += nbytes;
#else /* _NO_STREAM */
         {
            size_t sz = chunkSize;
            OSOCTET* p = &membuf.buffer[membuf.usedcnt];

            if (pctxt->buffer.byteIndex < pctxt->buffer.size) {
               size_t tail = pctxt->buffer.size - pctxt->buffer.byteIndex;
               if (tail > sz) tail = sz;

               OSCRTLSAFEMEMCPY
                  (p, membuf.bufsize-membuf.usedcnt,
                   pctxt->buffer.data + pctxt->buffer.byteIndex, tail);

               sz -= tail;
               p += tail;
               pctxt->buffer.byteIndex += tail;
            }

            while (sz) {
               stat = rtxSocketRecv (pSoapConn->socket, p, sz);

               if (stat < 0) return LOG_RTERR (pctxt, stat);
               else if (stat == 0) return LOG_RTERR (pctxt, RTERR_READERR);

               p += stat;
               sz -= stat;
            }

            membuf.usedcnt += chunkSize;
         }
#endif /* _NO_STREAM */

         if (pSoapConn->chunked) {
            /* read blank line */
            stat = rtxSoapRecvHttpLine(pSoapConn, lbuf, sizeof(lbuf));
            if (0 != stat) return LOG_RTERR(pSoapConn->pctxt, RTERR_HTTPERR);

            /* read next chunk size */
            stat = rtxSoapRecvHttpLine(pSoapConn, lbuf, sizeof(lbuf));
            if (0 != stat) return LOG_RTERR(pSoapConn->pctxt, RTERR_HTTPERR);

            stat = parseChunkSize(lbuf, &chunkSize);
            if (stat < 0) return LOG_RTERR(pSoapConn->pctxt, stat);
         }
#ifndef _NO_STREAM
         else if (nbytes == 0) break;     /* no more data */
#endif
      }

      /* ignore trailor */

      /* add null terminator */
      if (membuf.usedcnt > 0) {
         membuf.buffer[membuf.usedcnt] = (OSOCTET)'\0';
      }

      /* set length and content buffer */
      if (ppbuf) {
         *ppbuf = membuf.buffer;
      }
#ifdef _NO_STREAM
      else {
         if (pctxt->buffer.dynamic)
            rtxMemFreePtr (pctxt, pctxt->buffer.data);

         pctxt->buffer.data = membuf.buffer;
         pctxt->buffer.byteIndex = 0;
         pctxt->buffer.size = membuf.usedcnt;
         pctxt->buffer.dynamic = TRUE;
      }
#endif /* _NO_STREAM */

      pSoapConn->contentLength = membuf.usedcnt;
   }

   return 0;
}

int rtxSoapRecvHttpLine (OSSOAPCONN* pSoapConn, char* lbuf, size_t lbufsiz)
{
   long i = (long)lbufsiz;
   int stat;
   char* s = lbuf;
   OSCTXT*  pctxt = pSoapConn->pctxt;
   OSOCTET c = (OSOCTET)EOF;

#ifndef _NO_STREAM
   while (--i > 0) {
      stat = rtxStreamRead (pctxt, &c, 1);
      if (1 != stat) return LOG_RTERR (pctxt, stat);

      if (c == '\r' || c == '\n' || c == (OSOCTET)EOF)
         break;

      *s++ = (char) c;
   }

   *s++ = '\0';

   while (c != '\n' && c != (OSOCTET)EOF) {
      stat = rtxStreamRead (pctxt, &c, 1);
      if (1 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* TODO: handle HTTP line continuations */
#else /* _NO_STREAM */
   int phase = 0;

   while (i > 1) {
      if (pctxt->buffer.byteIndex == pctxt->buffer.size) {
         stat = rtxSocketRecv (pSoapConn->socket, pctxt->buffer.data,
                               OSRTSOAP_K_BUFSIZE);

         if (stat < 0) return LOG_RTERR (pctxt, stat);
         else if (stat == 0) {
            c = (OSOCTET)EOF;
            break;
         }

         pctxt->buffer.size = (size_t) stat;
         pctxt->buffer.byteIndex = 0;
      }

      c = pctxt->buffer.data[pctxt->buffer.byteIndex++];

      if (phase == 0 && c == '\r')
         phase = 1;
      else if (c == '\n') {
         if (phase == 1)
            phase = 2;
         /* else invalid message header */
      }
      else if (phase == 2) {
         if (c == ' ' || c == '\t') { /* line continuations */
            *s++ = c;
            i--;
            phase = 0;
         }
         else {
            pctxt->buffer.byteIndex--;
            break;
         }
      }
      else {
         *s++ = c;
         i--;
      }
   }

   *s++ = '\0';
#endif /* _NO_STREAM */

   RTDIAG2 (pSoapConn->pctxt, "%s\n", lbuf);

   return (c == (OSOCTET)EOF) ? RTERR_ENDOFFILE : 0;
}

int rtxSoapSendHttp (OSSOAPCONN* pSoapConn, const OSUTF8CHAR* soapMsg)
{
   return rtxSoapSendHttp2(pSoapConn, HttpMessageTypeRequest, soapMsg);
};

int rtxSoapSendHttpResponse (OSSOAPCONN* pSoapConn, const OSUTF8CHAR* soapMsg)
{
   return rtxSoapSendHttp2(pSoapConn, HttpMessageTypeResponse, soapMsg);
};

int rtxSoapSendHttp2 (OSSOAPCONN* pSoapConn, HttpMessageType msgType,
                      const OSUTF8CHAR* soapMsg)
{
   OSRTBuffer savedBuffer;
   size_t len = rtxUTF8LenBytes (soapMsg);
   OSCTXT* pctxt = pSoapConn->pctxt;
   int stat;
   OSBOOL msgInCtxtBuffer = FALSE;

#ifndef _NO_STREAM
   OSOCTET tmpBuff [OSRTSTRM_K_BUFSIZE];

   /* Make sure stream is enabled, otherwise the data will be written   */
   /* to the encode buffer..                                            */
   rtxCtxtClearFlag (pctxt, OSDISSTRM);

   if (soapMsg >= pctxt->buffer.data &&
       soapMsg < pctxt->buffer.data + pctxt->buffer.size)
   {
      /* soap message placed in context buffer */
      OSCRTLSAFEMEMCPY (&savedBuffer, sizeof(OSRTBuffer), &pctxt->buffer,
            sizeof (OSRTBuffer));

      pctxt->buffer.data = tmpBuff;
      pctxt->buffer.byteIndex = 0;
      pctxt->buffer.size = OSRTSTRM_K_BUFSIZE;
      pctxt->pStream->flags |= OSRTSTRMF_BUFFERED;

      msgInCtxtBuffer = TRUE;
   }

   /* Output HTTP header */
   stat = rtxHttpWriteHeader (pSoapConn, msgType, len);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Output XML content */
   stat = rtxWriteBytes (pctxt, soapMsg, len);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = rtxStreamFlush (pctxt);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (msgInCtxtBuffer) {
      pctxt->pStream->flags &= ~OSRTSTRMF_BUFFERED;
      OSCRTLSAFEMEMCPY (&pctxt->buffer, sizeof(OSRTBuffer), &savedBuffer,
            sizeof (OSRTBuffer));
   }
#else /* _NO_STREAM */
   if ((soapMsg >= pctxt->buffer.data &&
       soapMsg < pctxt->buffer.data + pctxt->buffer.size)
       || pctxt->buffer.dynamic == FALSE)
   {
      /* soap message placed in context buffer */
      OSCRTLSAFEMEMCPY (&savedBuffer, sizeof(OSRTBuffer), &pctxt->buffer,
            sizeof (OSRTBuffer));

      stat = rtxInitContextBuffer (pctxt, 0, OSRTSOAP_K_BUFSIZE);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      msgInCtxtBuffer = TRUE;
   }
   else if (pctxt->buffer.data == 0) {
      stat = rtxInitContextBuffer (pctxt, 0, OSRTSOAP_K_BUFSIZE);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* Output HTTP header */
   stat = rtxHttpWriteHeader (pSoapConn, msgType, len);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   stat = rtxSocketSend (pSoapConn->socket, pctxt->buffer.data,
                         pctxt->buffer.byteIndex);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   /* Output XML content */
   stat = rtxSocketSend (pSoapConn->socket, soapMsg, len);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (msgInCtxtBuffer) {
      rtxMemFreePtr (pctxt, pctxt->buffer.data);
      OSCRTLSAFEMEMCPY (&pctxt->buffer, sizeof(OSRTBuffer), &savedBuffer,
            sizeof (OSRTBuffer));
   }
#endif /* _NO_STREAM */

   RTDIAG1 (pctxt, (const char*)soapMsg);
   RTDIAG1 (pctxt, "\n");

   return 0;
}

int rtxHttpParseHdr (OSSOAPCONN* pSoapConn, const char* key, const char* value)
{
   char* tmpbuf;
   char* s;
   if (!rtxHttpTagCmp (key, "Host")) {
      size_t bufsize = strlen(value) + 8;
      tmpbuf = (char*) rtxMemAlloc (pSoapConn->pctxt, bufsize);
      if (tmpbuf == 0)
         return LOG_RTERR (pSoapConn->pctxt, RTERR_NOMEM);
      rtxStrJoin (tmpbuf, bufsize, "http://", value, 0, 0, 0);
      pSoapConn->endpoint = tmpbuf;
   }
   else if (!rtxHttpTagCmp (key, "Content-Length")) {
      int tmpLen = atoi (value);
      if (tmpLen < 0) {
         return LOG_RTERR (pSoapConn->pctxt, RTERR_HTTPERR);
      }
      pSoapConn->contentLength = tmpLen;
   }
   else if (!rtxHttpTagCmp (key, "Transfer-Encoding")) {
      if (!strcmp (value, "chunked"))
         pSoapConn->chunked = TRUE;
   }
   else if (!rtxHttpTagCmp (key, "SOAPAction")) {
      pSoapConn->soapAction = rtxStrdup (pSoapConn->pctxt, value);
      if (pSoapConn->soapAction == 0)
         return LOG_RTERR (pSoapConn->pctxt, RTERR_NOMEM);
   }
   else if ((pSoapConn->soapVersion == OSSOAPV2) &&
            (!rtxHttpTagCmp (key, "Content-Type"))) {
      s = strstr (value, "action=");
      if (s != NULL) {
         pSoapConn->soapAction = rtxStrdup (pSoapConn->pctxt, s);
         if (pSoapConn->soapAction == 0)
            return LOG_RTERR (pSoapConn->pctxt, RTERR_NOMEM);
      }
   }
   /* TODO: parse other header fields */
   return 0;
}

int rtxHttpPost (OSSOAPCONN* pSoapConn, size_t count)
{
   return rtxHttpWriteHeader(pSoapConn, HttpMessageTypeRequest, count);
}


int rtxHttpWriteHeader (OSSOAPCONN* pSoapConn, HttpMessageType msgType,
                        size_t count)
{
   int stat = 0;
   char lbuf[200];
   const char* s;
   const char* path = (0 == pSoapConn->path) ? "" : pSoapConn->path;
   const char MEDIATYPESOAP12[] = "application/soap+xml; charset=utf-8";
   const char MEDIATYPESOAP11[] = "text/xml; charset=utf-8";
   char numbuf[32];

   if ( msgType == HttpMessageTypeRequest ) {
      /* write request line */
      rtxStrJoin (lbuf, sizeof(lbuf), "POST /", path, " HTTP/", HTTP_VERSION,
                     0);
      stat = rtxHttpWriteHdrField (pSoapConn, lbuf, 0);

      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

      /* write Host */
      if (pSoapConn->port != 80) {
         stat = rtxIntToCharStr (pSoapConn->port, numbuf, sizeof(numbuf), 0);
         if (stat < 0) return LOG_RTERR (pSoapConn->pctxt, stat);

         s = rtxStrJoin (lbuf, sizeof(lbuf), pSoapConn->host, ":", numbuf, 0,
                           0);
      }
      else
         s = pSoapConn->host;

      stat = rtxHttpWriteHdrField (pSoapConn, "Host", s);
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
   }
   else {
      /* write status line.  Use the given response status code and leave the
         reason phrase empty */
      stat = rtxIntToCharStr (pSoapConn->responseStatusCode,
                                 numbuf, sizeof(numbuf), 0);
      if (stat < 0) return LOG_RTERR (pSoapConn->pctxt, stat);

      rtxStrJoin (lbuf, sizeof(lbuf), "HTTP/", HTTP_VERSION, " ", numbuf, " ");
      stat = rtxHttpWriteHdrField (pSoapConn, lbuf, 0);
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
   }

   /* write Content-Type */

   if (pSoapConn->soapVersion == OSSOAPV2) {
      if (0 != pSoapConn->soapAction) {
         s = rtxStrDynJoin
            (pSoapConn->pctxt, MEDIATYPESOAP12, "; action=\"",
             pSoapConn->soapAction, "\"", 0);

         if (s == 0)
            return LOG_RTERR (pSoapConn->pctxt, RTERR_NOMEM);
      } else
         s = MEDIATYPESOAP12;
   } else {
      s = MEDIATYPESOAP11;
   }

   stat = rtxHttpWriteHdrField (pSoapConn, "Content-Type", s);
   if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);


   if (msgType == HttpMessageTypeRequest &&
         pSoapConn->soapVersion != OSSOAPV2)
   {
      if (0 != pSoapConn->soapAction) {
         s = pSoapConn->soapAction;
         stat = rtxHttpWriteHdrField (pSoapConn, "SOAPAction", s);
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
      }
   }

   if (count > 0) {
      stat = rtxUInt64ToCharStr (count, lbuf, sizeof(lbuf), 0);
      if (stat < 0) return LOG_RTERR (pSoapConn->pctxt, stat);

      stat = rtxHttpWriteHdrField (pSoapConn, "Content-Length", lbuf);
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
   }

   if ( msgType == HttpMessageTypeRequest ) {
      s = (pSoapConn->keepAlive) ? "keep-alive" : "close";

      stat = rtxHttpWriteHdrField (pSoapConn, "Connection", s);
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);
   }

   RTDIAG1 (pSoapConn->pctxt, "\n");
   return rtxWriteBytes (pSoapConn->pctxt, (const OSOCTET*)"\r\n", 2);
}


int rtxHttpPostHdr (OSSOAPCONN* pSoapConn, const char* key, const char* value)
{
   return rtxHttpWriteHdrField(pSoapConn, key, value);
}

int rtxHttpWriteHdrField (OSSOAPCONN* pSoapConn, const char* key,
                          const char* value)
{
   if (0 != key) {
      int stat = rtxCopyAsciiText (pSoapConn->pctxt, key);
      if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

      RTDIAG1 (pSoapConn->pctxt, key);

      if (0 != value) {
         stat = rtxWriteBytes (pSoapConn->pctxt, (const OSOCTET*)": ", 2);
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

         stat = rtxCopyAsciiText (pSoapConn->pctxt, value);
         if (0 != stat) return LOG_RTERR (pSoapConn->pctxt, stat);

         RTDIAG2 (pSoapConn->pctxt, ": %s", value);
      }
   }
   RTDIAG1 (pSoapConn->pctxt, "\n");
   return rtxWriteBytes (pSoapConn->pctxt, (const OSOCTET*)"\r\n", 2);
}

int rtxHttpTagCmp (const char *s, const char *t)
{
   for ( ; *s && *s != '"'; s++, t++) {
      if (OS_TOLOWER(*s) != OS_TOLOWER(*t)) {
         if (*t != '-') {
            if (*t != '*') return 1;
            if (*++t) {
               register int c = OS_TOLOWER(*t);
               for ( ; *s && *s != '"'; s++) {
                  if (OS_TOLOWER(*s) == c)
                     if (!rtxHttpTagCmp (s + 1, t + 1))
                        return 0;
               }
               break;
            }
            else
               return 0;
         }
      }
   }
   if (*t == '*' && !t[1])
      return 0;

  return *t;
}

int rtxSoapSetReadTimeout (OSSOAPCONN* pSoapConn, OSUINT32 nsecs)
{
   pSoapConn->recvTimeoutSecs = nsecs;
   return 0;
}

#endif

