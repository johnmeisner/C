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

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxNetUtil.h"
#include "rtxsrc/rtxStreamSocket.h"
#include "rtxsrc/rtxSysInfo.h"

#define MAX_CONNECT_ATTEMPTS    3

int rtxNetParseURL2
(OSRTNETCONN* pNetConn, OSRTNETURL* pNetURL, const char* url)
{
   OSCTXT* pctxt = pNetConn->pctxt;
   char* s;

   if (OS_ISEMPTY (url)) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   if (0 != pNetURL->buffer) {
      rtxMemFreePtr (pctxt, pNetURL->buffer);
   }
   pNetURL->buffer = rtxStrdup (pctxt, url);
   if (0 == pNetURL->buffer) {
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   s = strchr (pNetURL->buffer, ':');
   if (s && s[1] == '/' && s[2] == '/') {
      if (url[0] == 'h' && url[1] == 't' && url[2] == 't' && url[3] == 'p') {
         pNetConn->protocol = OSRTNetHttp;
      }
      else if (url[0] == 'f' && url[1] == 't' && url[2] == 'p') {
         pNetConn->protocol = OSRTNetFtp;
      }
      else {
         pNetConn->protocol = OSRTNetOtherP;
      }
      s += 3;
   }
   else {
      s = pNetURL->buffer;
      /* Default protocol is HTTP if not specified */
      pNetConn->protocol = OSRTNetHttp;
   }

   /* Parse domain name */
   pNetURL->domain = s;
   while (*s != '\0' && *s != '/' && *s != ':') s++;
   if (*s == ':') {
      /* Parse port number */
      *s++ = '\0';  /* null terminate domain name and advance pointer */
      pNetURL->port = 0;
      while (OS_ISDIGIT(*s)) {
         pNetURL->port = pNetURL->port * 10 + (*s - '0');
         s++;
      }
      if (*s != '/' && *s != '\0') {
         /* Malformed URL */
         rtxMemFreePtr (pctxt, pNetURL->buffer);
         pNetURL->buffer = 0;
         return LOG_RTERR (pctxt, RTERR_INVFORMAT);
      }
   }

   /* Parse path */
   if (*s == '/') {
      *s++ = '\0';  /* null terminate domain name and advance pointer */
      pNetURL->path = s;
   }

   return 0;
}

int rtxNetParseURL (OSRTNETCONN* pNetConn, const char* url)
{
   return rtxNetParseURL2 (pNetConn, &pNetConn->url, url);
}

int rtxNetInitConn (OSCTXT* pctxt, OSRTNETCONN* pNetConn, const char* url)
{
   /* Initialize connection member variables */
   pNetConn->pctxt = pctxt;
   pNetConn->socket = OSRTSOCKET_INVALID;
   pNetConn->protocol = OSRTNetNoProto;
   pNetConn->transport = OSRTNetTCP;
   pNetConn->dynamic = FALSE;
   pNetConn->url.buffer = 0;
   pNetConn->url.domain = 0;
   pNetConn->url.path = 0;
   pNetConn->url.port = 80;
   pNetConn->proxy.buffer = 0;
   pNetConn->proxy.domain = 0;
   pNetConn->proxy.path = 0;
   pNetConn->proxy.port = 80;

   /* Parse URL */
   if (!OS_ISEMPTY (url)) {
      int ret = rtxNetParseURL2 (pNetConn, &pNetConn->url, url);
      if (0 != ret) return LOG_RTERR (pctxt, ret);
   }

   /* If HTTP, check proxy environment variables */
#ifndef _WIN32_WCE
   if (pNetConn->protocol == OSRTNetHttp) {
      char* env;

      /* Check 'no_proxy' environment variable.  If set to '*', do not
         do further checks. */
      env = rtxEnvVarDup ("no_proxy");
      if (0 != env && ((env[0] == '*') && (env[1] == 0))) {
         OSCRTLFREE (env);
         return 0;
      }

      /* Check 'http_proxy' environment variable.  If set, parse URI and
         set proxy variables */
      env = rtxEnvVarDup ("http_proxy");
      if (0 == env) {
         env = rtxEnvVarDup ("HTTP_PROXY");
      }
      if (0 != env) {
         int ret = rtxNetParseURL2 (pNetConn, &pNetConn->proxy, env);
         if (0 != ret) return LOG_RTERR (pctxt, ret);

         OSCRTLFREE (env);
      }
   }
#endif

   return 0;
}

OSRTNETCONN* rtxNetCreateConn (OSCTXT* pctxt, const char* url)
{
   OSRTNETCONN* pNetConn;

   if (0 == pctxt) {
      return 0;
   }

   if (0 == url || 0 == *url) {
      LOG_RTERR (pctxt, RTERR_INVPARAM);
      return 0;
   }

   pNetConn = (OSRTNETCONN*)
      rtxMemSysAlloc (pctxt, sizeof(OSRTNETCONN));

   if (0 == pNetConn) {
      LOG_RTERR (pctxt, RTERR_NOMEM);
      return 0;
   }

   if (rtxNetInitConn (pctxt, pNetConn, url) < 0) {
      rtxMemSysFreePtr (pctxt, pNetConn);
      return 0;
   }

   pNetConn->dynamic = TRUE;

   return pNetConn;
}

int rtxNetConnect (OSRTNETCONN* pNetConn)
{
   int i, stat, port;
   const char* domain;
   OSCTXT* pctxt = pNetConn->pctxt;

   if (OSRTSOCKET_INVALID == pNetConn->socket) {
      stat = rtxSocketCreate (&pNetConn->socket);
      if (0 != stat) return LOG_RTERR (pctxt, stat);
   }

   if (0 == pNetConn->proxy.domain) {
      port = pNetConn->url.port;
      domain = pNetConn->url.domain;
   }
   else {
      port = pNetConn->proxy.port;
      domain = pNetConn->proxy.domain;
   }

   RTDIAG3 (pctxt, "Connecting to %s:%i..",
            domain, port);

   for (i = 0; i < MAX_CONNECT_ATTEMPTS; i++) {
      stat = rtxSocketConnect
         (pNetConn->socket, domain, port);
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
      (pctxt, pNetConn->socket,
       OSRTSTRMF_OUTPUT|OSRTSTRMF_INPUT|OSRTSTRMF_UNBUFFERED);

   if (0 != stat) return LOG_RTERR (pctxt, stat);
#endif /* _NO_STREAM */

   return 0;
}

int rtxNetCloseConn (OSRTNETCONN* pNetConn)
{
   if (OSRTSOCKET_INVALID != pNetConn->socket) {
      rtxStreamSocketClose (pNetConn->pctxt);
      pNetConn->socket = OSRTSOCKET_INVALID;
   }

   if (0 != pNetConn->url.buffer) {
      rtxMemFreePtr (pNetConn->pctxt, pNetConn->url.buffer);
      pNetConn->url.buffer = 0;
   }

   if (0 != pNetConn->proxy.buffer) {
      rtxMemFreePtr (pNetConn->pctxt, pNetConn->proxy.buffer);
      pNetConn->proxy.buffer = 0;
   }

   if (pNetConn->dynamic) {
      rtxMemSysFreePtr (pNetConn->pctxt, pNetConn);
   }

   return 0;
}

#endif /* _OS_NOSOCKET */
