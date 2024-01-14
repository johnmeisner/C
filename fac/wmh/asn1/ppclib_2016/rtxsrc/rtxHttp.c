/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxHttp.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxStream.h"
#include "rtxsrc/rtxUTF8.h"

int rtxHttpGet (OSCTXT* pctxt, const char* url, OSRTHttpContent* pContent)
{
   OSRTNETCONN netConn;
   OSRTNETCONN* pNetConn = &netConn;
   OSRTHttpHeader header;

   int ret = rtxNetInitConn (pctxt, pNetConn, url);
   if (0 != ret) return LOG_RTERR (pctxt, ret);

   /* Connect to server.  If proxy is set, will connect to that. */

   ret = rtxNetConnect(pNetConn);
   if (0 != ret) return LOG_RTERR(pctxt, ret);

   /* If connected to proxy, send HTTP CONNECT */

   if (0 != pNetConn->proxy.domain) {
      ret = rtxHttpConnect(pNetConn);
      if (0 != ret) return LOG_RTERR(pctxt, ret);
   }

   /* Format and send an HTTP GET request */

   ret = rtxHttpSendGetRequest(pNetConn, 0);
   if (0 != ret) return LOG_RTERR(pctxt, ret);

   /* Receive response header */

   ret = rtxHttpRecvRespHdr(pNetConn, &header);
   if (0 != ret) return LOG_RTERR(pctxt, ret);

#ifdef _TRACE
   rtxDiagPrintHttpHeader (pctxt, &header);
#endif
   /* Read response contents */

   ret = rtxHttpRecvContent(pNetConn, &header, pContent);

   rtxNetCloseConn (pNetConn);

   return (0 == ret) ? 0 : LOG_RTERR (pctxt, ret);
}

int rtxHttpSendRequest (OSRTNETCONN* pNetConn,
   const char* method, const char* content, const char* contentType)
{
   OSRTMEMBUF httpBuffer;
   OSOCTET* bufptr;
   OSCTXT* pctxt;
   size_t contentLen = (0 == content) ? 0 : OSCRTLSTRLEN (content);
   int buflen, stat;

   pctxt = (0 == pNetConn) ? 0 : pNetConn->pctxt;

   if (0 == pctxt) {
      return RTERR_INVPARAM;
   }

   if (0 == pNetConn || pNetConn->protocol != OSRTNetHttp) {
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
   }

   rtxMemBufInit (pctxt, &httpBuffer, 512);

   /* Create header */

   OSMBAPPENDSTR (&httpBuffer, method);
   OSMBAPPENDSTRL (&httpBuffer, " ");
   if (!OS_ISEMPTY (pNetConn->url.path)) {
      if (*pNetConn->url.path != '/') {
         OSMBAPPENDSTRL (&httpBuffer, "/");
      }
      OSMBAPPENDSTR (&httpBuffer, pNetConn->url.path);
   }
   OSMBAPPENDSTRL (&httpBuffer, " HTTP/1.1\r\nHost: ");
   OSMBAPPENDSTR (&httpBuffer, pNetConn->url.domain);

   if (pNetConn->url.port != 80) {
      char buffer[32];
      rtxIntToCharStr (pNetConn->url.port, buffer, sizeof(buffer), 0);
      OSMBAPPENDSTRL (&httpBuffer, ":");
      OSMBAPPENDSTRL (&httpBuffer, buffer);
   }

   OSMBAPPENDSTRL (&httpBuffer, "\r\n");

   if (!OS_ISEMPTY (contentType)) {
      OSMBAPPENDSTRL (&httpBuffer, "Content-Type: ");
      OSMBAPPENDSTR (&httpBuffer, contentType);
      OSMBAPPENDSTRL (&httpBuffer, "\r\n");
   }

   if (contentLen > 0) {
      char buffer[32];
      rtxSizeToCharStr (contentLen, buffer, sizeof(buffer), 0);
      OSMBAPPENDSTRL (&httpBuffer, "Content-Length: ");
      OSMBAPPENDSTRL (&httpBuffer, buffer);
      OSMBAPPENDSTRL (&httpBuffer, "\r\n");
   }

   OSMBAPPENDSTRL (&httpBuffer, "\r\n");

   /* Write to socket stream */

   bufptr = rtxMemBufGetData (&httpBuffer, &buflen);

   RTDIAG1 (pctxt, "HTTP header:\n");
   RTDIAGCHARS (pctxt, (const char*)bufptr, buflen);

   stat = rtxStreamWrite (pctxt, bufptr, buflen);
   if (0 != stat) return LOG_RTERR (pctxt, stat);

   if (contentLen > 0) {
      stat = rtxStreamWrite (pctxt, (const OSOCTET*)content, contentLen);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   stat = rtxStreamFlush (pctxt);

   return (0 == stat) ? 0 : LOG_RTERR (pctxt, stat);
}

int rtxHttpSendGetRequest (OSRTNETCONN* pNetConn, const char* url)
{
   if (!OS_ISEMPTY(url)) {
      int ret = rtxNetParseURL (pNetConn, url);
      if (0 != ret) return LOG_RTERR (pNetConn->pctxt, ret);
   }
   return rtxHttpSendRequest
      (pNetConn, "GET", 0, 0);
}

static int readHttpLine (OSCTXT* pctxt, char* lbuf, size_t lbufsiz)
{
   size_t i;
   int stat = 0;
   OSOCTET c = (OSOCTET)EOF;

   for (i = 0; i < lbufsiz; i++) {
      stat = rtxStreamBlockingRead (pctxt, (OSOCTET*)&lbuf[i], 1);
      if (1 != stat) return LOG_RTERR (pctxt, stat);

      if (lbuf[i] == '\r' || lbuf[i] == '\n' || (signed int)lbuf[i] == EOF) {
         c = (OSOCTET) lbuf[i];
         lbuf[i] = '\0';
         break;
      }
   }

   if (i >= lbufsiz) return LOG_RTERR (pctxt, RTERR_BUFOVFLW);

   while (c != '\n' && c != (OSOCTET)EOF) {
      stat = rtxStreamBlockingRead (pctxt, &c, 1);
      if (1 != stat) return LOG_RTERR (pctxt, stat);
   }

   /* TODO: handle HTTP line continuations */

   RTDIAG2 (pctxt, "%s\n", lbuf);

   return (c == (OSOCTET)EOF) ? RTERR_ENDOFFILE : 0;
}

int rtxHttpRecvRespHdr (OSRTNETCONN* pNetConn, OSRTHttpHeader* pHeader)
{
   OSCTXT* pctxt;
   OSUTF8NVP* pnvp;
   char lbuf[1024], *s;
   int httpStat, stat = 0;

   if (0 == pNetConn || 0 == pHeader) return RTERR_INVPARAM;

   pctxt = pNetConn->pctxt;

   if (0 == pctxt) return RTERR_INVPARAM;

   /* Init header structure */
   OSCRTLMEMSET (pHeader, 0, sizeof(OSRTHttpHeader));
   rtxArrayListInit (&pHeader->fields, 100);

   do {
      stat = readHttpLine (pctxt, lbuf, sizeof(lbuf));
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      if ((s = strchr (lbuf, '/')) != 0) {
         s++;
         while (OS_ISDIGIT(*s)) {
            pHeader->majorVersion =
               (OSUINT8)(pHeader->majorVersion * 10 + (*s - '0'));
            s++;
         }
         if (*s == '.') s++;
         while (OS_ISDIGIT(*s)) {
            pHeader->minorVersion =
               (OSUINT8)(pHeader->minorVersion * 10 + (*s - '0'));
            s++;
         }
      }
      RTDIAG2 (pctxt, "HTTP major version = %d\n", pHeader->majorVersion);
      RTDIAG2 (pctxt, "HTTP minor version = %d\n", pHeader->minorVersion);

      if ((s = strchr (lbuf, ' ')) != 0) {
         char* nextSpace = strchr(s + 1, ' ');
         if ( nextSpace != 0 ) *nextSpace = '\0';
         stat = rtxCharStrToInt (s, &httpStat);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
      }
      else httpStat = 0;

      RTDIAG2 (pctxt, "HTTP status = %d\n", httpStat);

      for (;;) {
         stat = readHttpLine (pctxt, lbuf, sizeof(lbuf));
         if (0 != stat) return LOG_RTERR (pctxt, stat);

         if (lbuf[0] == '\0') break;

         s = strchr (lbuf, ':');
         if (0 != s) {
            *s = '\0';
            do s++; while (*s && *s <= 32);
         } else
            return LOG_RTERR (pctxt, RTERR_HTTPERR);

         pnvp = rtxMemAllocType (pctxt, OSUTF8NVP);
         if (0 == pnvp) return LOG_RTERR (pctxt, RTERR_NOMEM);

         pnvp->name = rtxUTF8Strdup (pctxt, OSUTF8(lbuf));
         pnvp->value = rtxUTF8Strdup (pctxt, OSUTF8(s));

         if (0 == pnvp->name || 0 == pnvp->value) {
            rtxMemFreePtr (pctxt, pnvp);
            return LOG_RTERR (pctxt, RTERR_NOMEM);
         }

         stat = rtxArrayListAdd (pctxt, &pHeader->fields, (void*)pnvp, 0);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
      }
   }
   while (httpStat == 100);

   pHeader->status = httpStat;

   if (httpStat == 0 || httpStat == 200 || httpStat == 400 || httpStat == 500)
      return 0;
   else {
      rtxErrNewNode (pctxt);
      rtxErrAddIntParm (pctxt, httpStat);
      return LOG_RTERR (pctxt, RTERR_HTTPERR);
   }
}

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

int rtxHttpRecvContent
(OSRTNETCONN* pNetConn, OSRTHttpHeader* pHeader, OSRTHttpContent* pContent)
{
   OSCTXT* pctxt;
   OSRTArrayListIter iter;
   OSUTF8NVP* pnvp;
   OSBOOL chunked = FALSE;
   int stat = 0;

   if (0 == pNetConn || 0 == pHeader || 0 == pContent)
      return RTERR_INVPARAM;

   /* Parse content length from header */

   pContent->length = 0;

   pctxt = pNetConn->pctxt;

   if (0 == pctxt)
      return RTERR_INVPARAM;

   rtxArrayListInitIter (&iter, &pHeader->fields, 0);
   while (rtxArrayListHasNextItem (&iter)) {
      pnvp = (OSUTF8NVP*) rtxArrayListNextItem (&iter);

      /* This should never happen, but we include it to avoid lint-checking
       * issues. */
      if (0 == pnvp) {
         return LOG_RTERR (pctxt, RTERR_HTTPERR);
      }

      if (rtxUTF8StrEqual (pnvp->name, OSUTF8("Content-Length"))) {
         stat = rtxUTF8StrToSize (pnvp->value, &pContent->length);
         if (0 != stat) return LOG_RTERR (pctxt, stat);
      }
      else if (rtxUTF8StrEqual (pnvp->name, OSUTF8("Transfer-Encoding"))) {
         if (rtxUTF8StrEqual (pnvp->value, OSUTF8("chunked"))) {
            chunked = TRUE;
         }
      }
   }

   if (pContent->length > 0) {
      long lstat;

      pContent->data = (OSOCTET*) rtxMemAlloc (pctxt, pContent->length + 1);
      if (0 == pContent->data) return LOG_RTERR (pctxt, RTERR_NOMEM);

      lstat = rtxStreamBlockingRead (pctxt, pContent->data, pContent->length);
      if (lstat < 0) return LOG_RTERR (pctxt, (int)lstat);

      /* Null terminate */

      pContent->data[pContent->length] = (OSOCTET)'\0';
   }
   else {
      /* If chunked, read body chunk by chunk.  Otherwise, read in up to
         1024 bytes at a time, until the connection is closed.
      */
      char lbuf[80];
      size_t chunkSize;
      OSRTMEMBUF membuf;
      int nbytes;

      pContent->length = 0;

      if (chunked) {
         stat = readHttpLine(pctxt, lbuf, sizeof(lbuf));
         if (0 != stat) return LOG_RTERR(pctxt, RTERR_HTTPERR);

         stat = parseChunkSize(lbuf, &chunkSize);
         if (stat < 0) return LOG_RTERR(pctxt, stat);
      }
      else chunkSize = 1024;

      rtxMemBufInit (pctxt, &membuf, chunkSize + 1);

      /* read chunks */
      while (chunkSize > 0) {
         /* expand dynamic buffer */
         stat = rtxMemBufPreAllocate (&membuf, chunkSize+1);
         if (0 != stat) return LOG_RTERR (pctxt, stat);

         /* read data */
         nbytes = rtxStreamBlockingRead
            (pctxt, &membuf.buffer[membuf.usedcnt], chunkSize);

         if (nbytes < 0) return LOG_RTERR (pctxt, nbytes);

         /*TODO if not chunked, some error must indicate eof, in which case
            we can break the loop*/

         membuf.usedcnt += nbytes;

         if (chunked) {
            /* read blank line */
            stat = readHttpLine(pctxt, lbuf, sizeof(lbuf));
            if (0 != stat) return LOG_RTERR(pctxt, RTERR_HTTPERR);

            /* read next chunk size */
            stat = readHttpLine(pctxt, lbuf, sizeof(lbuf));
            if (0 != stat) return LOG_RTERR(pctxt, RTERR_HTTPERR);

            stat = parseChunkSize(lbuf, &chunkSize);
            if (stat < 0) return LOG_RTERR(pctxt, stat);
         }
         else if (nbytes == 0) break;
      }

      /* ignore trailor */

      /* add null terminator */
      if (membuf.usedcnt > 0) {
         membuf.buffer[membuf.usedcnt] = (OSOCTET)'\0';
      }

      pContent->data = membuf.buffer;
      pContent->length = membuf.usedcnt;
   }

   return 0;
}

int rtxHttpRecvRespContent
(OSRTNETCONN* pNetConn, OSRTHttpHeader* pHeader, OSRTHttpContent* pContent)
{
   OSCTXT* pctxt;
   OSRTArrayListIter iter;
   OSUTF8NVP* pnvp;
   int stat = 0;

   if (0 == pNetConn || 0 == pHeader || 0 == pContent)
      return RTERR_INVPARAM;

   /* Parse content length from header */

   pContent->length = 0;

   pctxt = pNetConn->pctxt;

   if (0 == pctxt)
      return RTERR_INVPARAM;

   rtxArrayListInitIter(&iter, &pHeader->fields, 0);
   while (rtxArrayListHasNextItem(&iter)) {
      pnvp = (OSUTF8NVP*)rtxArrayListNextItem(&iter);

      /* This should never happen, but we include it to avoid lint-checking
      * issues. */
      if (0 == pnvp) {
         return LOG_RTERR(pctxt, RTERR_HTTPERR);
      }

      if (rtxUTF8StrEqual(pnvp->name, OSUTF8("Content-Length"))) {
         stat = rtxUTF8StrToSize(pnvp->value, &pContent->length);
         if (0 != stat) return LOG_RTERR(pctxt, stat);
      }
   }

   if (pContent->length > 0) {
      long lstat;

      pContent->data = (OSOCTET*)rtxMemAlloc(pctxt, pContent->length + 1);
      if (0 == pContent->data) return LOG_RTERR(pctxt, RTERR_NOMEM);

      lstat = rtxStreamBlockingRead(pctxt, pContent->data, pContent->length);
      if (lstat < 0) return LOG_RTERR(pctxt, (int)lstat);

      /* Null terminate */

      pContent->data[pContent->length] = (OSOCTET)'\0';
   }
   return 0;
}

int rtxHttpConnect (OSRTNETCONN* pNetConn)
{
   OSRTHttpHeader header;
   OSRTHttpContent content;
   OSCTXT* pctxt = pNetConn->pctxt;
   char  numbuf[32];
   char* url;
   int   ret;

   /* Construct URL for connect request */

   if (pNetConn->url.port != 0) {
      numbuf[0] = ':';
      ret = rtxIntToCharStr
         (pNetConn->url.port, &numbuf[1], sizeof(numbuf)-1, 0);
      if (ret < 0) return LOG_RTERR (pctxt, ret);
   }
   else numbuf[0] = '\0';

   url = rtxStrDynJoin (pctxt, "http://", pNetConn->url.domain, numbuf, 0, 0);
   if (0 == url) return LOG_RTERR (pctxt, RTERR_NOMEM);

   ret = rtxHttpSendRequest (pNetConn, "CONNECT", url, "text/plain");

   rtxMemFreePtr (pctxt, url);

   if (0 == ret) {
      ret = rtxHttpRecvRespHdr (pNetConn, &header);
   }
   if (0 != ret) return LOG_RTERR (pNetConn->pctxt, ret);

#ifdef _TRACE
   rtxDiagPrintHttpHeader (pNetConn->pctxt, &header);
#endif
   /* Receive content */

   ret = rtxHttpRecvRespContent (pNetConn, &header, &content);

   return (0 != ret) ? LOG_RTERR (pNetConn->pctxt, ret) : 0;
}

#ifdef _TRACE
void rtxDiagPrintHttpHeader (OSCTXT* pctxt, const OSRTHttpHeader* pHeader)
{
   OSRTArrayListIter iter;
   OSUTF8NVP* pnvp;

   if (!rtxDiagTraceLevelEnabled (pctxt, OSRTDiagDebug)) return;

   rtxDiagStream (pctxt, "HTTP version = %d.%d\n",
                  pHeader->majorVersion, pHeader->minorVersion);

   rtxDiagStream (pctxt, "HTTP status = %d\n", pHeader->status);
   rtxDiagStream (pctxt, "HTTP fields:\n");

   rtxArrayListInitIter (&iter, &pHeader->fields, 0);
   while (rtxArrayListHasNextItem (&iter)) {
      pnvp = (OSUTF8NVP*) rtxArrayListNextItem (&iter);

      /* This should never happen, but we include it to avoid lint-checking
       * issues. */
      if (0 == pnvp) {
         continue;
      }

      rtxDiagStream (pctxt, "   Name = '%s', Value = '%s'\n",
                     pnvp->name, pnvp->value);
   }
}
#endif

#ifdef UNITTEST
#include "rtxsrc/rtxPrint.h"

/* usage: rtxHttp <url> */

static void usage()
{
   printf ("usage: rtxHttp <url>\n");
}

int main (int argc, char* argv[])
{
   OSRTHttpContent content;
   OSCTXT ctxt;
   int ret;

   if (2 != argc) { usage(); return 0; }

   rtxSocketsInit();

   ret = rtxInitContext (&ctxt);
   if (0 != ret) {
      printf ("rtxInitContext, status = %d\n", ret);
      return ret;
   }

   /* Execute GET request */

   ret = rtxHttpGet (&ctxt, argv[1], &content);
   if (0 != ret) {
      printf ("rtxHttpGet failed:\n");
      rtxErrPrint (&ctxt);
      rtxFreeContext (&ctxt);
      return ret;
   }

   rtxHexDump (content.data, content.length);

}
#endif /* UNITTEST */

#endif /* _OS_NOSOCKET */
