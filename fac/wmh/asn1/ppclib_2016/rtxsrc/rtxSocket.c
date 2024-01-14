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
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxSocket.h"
#include "rtxsrc/rtxError.h"

#include <stdio.h>
/* for mem* functions */

#if !defined(_WIN32) && !defined(_WIN64) && !defined(_WIN32_WCE)
/* I hope this is supported on all *NIX systems.  It is needed for the fcntl
   call in the rtxSocketSetBlocking function. */
#include <fcntl.h>
#endif

/* Cross-platform error definitions */
#if defined(_WIN32) || defined(_WIN64)
#define OSYS_SE_ADDRINUSE     WSAEADDRINUSE
#define OSYS_SE_HOSTDOWN      WSAEHOSTDOWN
#define OSYS_SE_HUNREACH      WSAEHOSTUNREACH
#define OSYS_SE_NUNREACH      WSAENETUNREACH
#define OSYS_SE_NETDOWN       WSAENETDOWN
#define OSYS_SE_NRESET        WSAENETRESET
#define OSYS_SE_CRESET        WSAECONNRESET
#define OSYS_SE_CABORT        WSAECONNABORTED
#define OSYS_SE_ISCONN        WSAEISCONN
#define OSYS_SE_NOTCONN       WSAENOTCONN
#define OSYS_SE_ADDRREQ       WSAEDESTADDRREQ
#define OSYS_SE_CREFUSED      WSAECONNREFUSED
#define OSYS_SE_INPROGRESS    WSAEINPROGRESS
#define OSYS_SE_WOULDBLOCK    WSAEWOULDBLOCK
#elif !defined(_WIN32_WCE)
#define OSYS_SE_ADDRINUSE     EADDRINUSE
#define OSYS_SE_HOSTDOWN      EHOSTDOWN
#define OSYS_SE_HUNREACH      EHOSTUNREACH
#define OSYS_SE_NUNREACH      ENETUNREACH
#define OSYS_SE_NETDOWN       ENETDOWN
#define OSYS_SE_NRESET        ENETRESET
#define OSYS_SE_CRESET        ECONNRESET
#define OSYS_SE_CABORT        ECONNABORTED
#define OSYS_SE_ISCONN        EISCONN
#define OSYS_SE_NOTCONN       ENOTCONN
#define OSYS_SE_ADDRREQ       EDESTADDRREQ
#define OSYS_SE_CREFUSED      ECONNREFUSED
#define OSYS_SE_INPROGRESS    EINPROGRESS
#define OSYS_SE_WOULDBLOCK    EWOULDBLOCK
#else
#define OSYS_SE_ADDRINUSE     0
#define OSYS_SE_HOSTDOWN      0
#define OSYS_SE_HUNREACH      0
#define OSYS_SE_NUNREACH      0
#define OSYS_SE_NETDOWN       0
#define OSYS_SE_NRESET        0
#define OSYS_SE_CRESET        0
#define OSYS_SE_CABORT        0
#define OSYS_SE_ISCONN        0
#define OSYS_SE_NOTCONN       0
#define OSYS_SE_ADDRREQ       0
#define OSYS_SE_CREFUSED      0
#define OSYS_SE_INPROGRESS    0
#define OSYS_SE_WOULDBLOCK    0
#endif

#if defined(_WIN32_WCE)
static int inited = 0;
#define SEND_FLAGS     0
#define SHUTDOWN_FLAGS 0
typedef int os_socklen_t;

#elif defined(__SYMBIAN32__)  /* Symbian OS */
typedef size_t os_socklen_t;
#define SEND_FLAGS     0
#define SHUTDOWN_FLAGS 1
#define closesocket close

#elif defined (_WIN32)
#if !defined (_MINGW_SOURCE) && !defined(__MINGW32__)
static LPFN_ACCEPT accept;
static LPFN_BIND bind;
static LPFN_CLOSESOCKET closesocket;
static LPFN_CONNECT connect;
static LPFN_GETHOSTBYNAME gethostbyname;
static LPFN_GETSOCKOPT getsockopt;
static LPFN_HTONL htonl;
static LPFN_HTONS htons;
static LPFN_INET_ADDR inet_addr;
static LPFN_IOCTLSOCKET ioctlsocket;
static LPFN_LISTEN listen;
static LPFN_NTOHL ntohl;
static LPFN_NTOHS ntohs;
static LPFN_RECV recv;
static LPFN_SEND send;
static LPFN_SELECT select;
static LPFN_SHUTDOWN shutdown;
static LPFN_SETSOCKOPT setsockopt;
static LPFN_SOCKET socket;
static LPFN_WSAGETLASTERROR WSAGetLastError;
#endif
static HMODULE ws32 = 0;
#define SEND_FLAGS     0
#define SHUTDOWN_FLAGS SD_SEND
typedef int os_socklen_t;
#else
#define SEND_FLAGS     0
#if defined(__UNIXWARE__) || defined(__USLC__) || defined(__svr4__)
/* UnixWare does not define SHUT_WR macro without _XOPEN_SOURCE.
   But usage of _XOPEN_SOURCE produces more problem. Thus, I use
   explicit value 1 instead of SHUT_WR for UnixWare */
#define SHUTDOWN_FLAGS 1

#elif defined(__vxworks)  /* VxWorks Tornado PPC */
#define SHUTDOWN_FLAGS 2  /* send and receive disallowed */
#else
#define SHUTDOWN_FLAGS SHUT_WR
#endif
#define closesocket close

#if (defined(_HP_UX) || defined(__hpux) || \
(defined(__alpha) && defined(__osf__))) && !defined (_XOPEN_SOURCE_EXTENDED)
typedef int os_socklen_t;
#elif defined(__UNIXWARE__) || defined(__USLC__) || defined(__svr4__)
typedef size_t os_socklen_t;
#elif defined(__vxworks)  /* VxWorks Tornado PPC */
typedef int os_socklen_t;
#elif defined(VMS) || defined(__VMS)
typedef size_t os_socklen_t;
#else
typedef socklen_t os_socklen_t;
#endif
#endif

/* Maps error codes to their corresponding runtime equivalents. */
static int errToRTERR (int errnum)
{
   switch (errnum) {
      case OSYS_SE_ADDRINUSE: return RTERR_ADDRINUSE;

      case OSYS_SE_HOSTDOWN:
      case OSYS_SE_HUNREACH:
      case OSYS_SE_NUNREACH:
      case OSYS_SE_NETDOWN:
         return RTERR_UNREACHABLE;

      case OSYS_SE_NRESET:
      case OSYS_SE_CRESET:
      case OSYS_SE_CABORT:
         return RTERR_CONNRESET;

      case OSYS_SE_NOTCONN:
      case OSYS_SE_ADDRREQ:
         return RTERR_NOCONN;

      case OSYS_SE_CREFUSED: return RTERR_CONNREFUSED;

      case OSYS_SE_ISCONN: return RTERR_INVSOCKOPT;

      default: return RTERR_INVSOCKET;
   }
}


#include "rtxsrc/rtxCommonDefs.h"

EXTRTMETHOD int rtxSocketsInit ()
{
#if defined(_WIN32_WCE)
   WORD wVersionRequested;
   WSADATA wsaData;
   int err;

   if (inited) return 0;

   wVersionRequested = MAKEWORD( 1, 1 );

   err = WSAStartup (wVersionRequested, &wsaData);
   if ( err != 0 ) {
      /* Tell the user that we could not find a usable */
      /* WinSock DLL.   */
      return RTERR_NOTINIT;
   }
   inited = 1;

#elif defined (_WIN32) && !defined(__SYMBIAN32__)
   LPFN_WSASTARTUP wsaStartup = NULL;
   WSADATA wsaData;

   if (ws32 != 0) return 0;

   ws32 = LoadLibrary (_T("WSOCK32.DLL"));
   if (ws32 == NULL) {
#ifdef _DEBUG
      LPTSTR lpMsgBuf;
      DWORD dw = GetLastError();

      if (FormatMessage(
             FORMAT_MESSAGE_ALLOCATE_BUFFER |
             FORMAT_MESSAGE_FROM_SYSTEM,
             NULL,
             dw,
             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
             (LPTSTR) &lpMsgBuf,
             0,
             NULL ))
      {
         _tprintf (TEXT("LoadLibrary failed for wsock32.dll ---\n"));
         _tprintf (TEXT("error code %ld: %s\n"), dw, (LPTSTR)lpMsgBuf);
         LocalFree (lpMsgBuf);
      }
#endif
      return RTERR_NOTINIT;
   }

   wsaStartup = (LPFN_WSASTARTUP) GetProcAddress (ws32, "WSAStartup");
   if (wsaStartup == NULL) return RTERR_NOTINIT;

#if !defined (_MINGW_SOURCE) && !defined(__MINGW32__)
   accept = (LPFN_ACCEPT) GetProcAddress (ws32, "accept");
   if (accept == NULL) return RTERR_NOTINIT;

   bind = (LPFN_BIND) GetProcAddress (ws32, "bind");
   if (bind == NULL) return RTERR_NOTINIT;

   closesocket = (LPFN_CLOSESOCKET) GetProcAddress (ws32, "closesocket");
   if (closesocket == NULL) return RTERR_NOTINIT;

   connect = (LPFN_CONNECT) GetProcAddress (ws32, "connect");
   if (connect == NULL) return RTERR_NOTINIT;

   gethostbyname = (LPFN_GETHOSTBYNAME) GetProcAddress (ws32, "gethostbyname");
   if (gethostbyname == NULL) return RTERR_NOTINIT;

   getsockopt = (LPFN_GETSOCKOPT) GetProcAddress (ws32, "getsockopt");
   if (getsockopt == NULL) return RTERR_NOTINIT;

   htonl = (LPFN_HTONL) GetProcAddress (ws32, "htonl");
   if (htonl == NULL) return RTERR_NOTINIT;

   htons = (LPFN_HTONS) GetProcAddress (ws32, "htons");
   if (htons == NULL) return RTERR_NOTINIT;

   inet_addr = (LPFN_INET_ADDR) GetProcAddress (ws32, "inet_addr");
   if (inet_addr == NULL) return RTERR_NOTINIT;

   ioctlsocket = (LPFN_IOCTLSOCKET) GetProcAddress (ws32, "ioctlsocket");
   if (ioctlsocket == NULL) return RTERR_NOTINIT;

   listen = (LPFN_LISTEN) GetProcAddress (ws32, "listen");
   if (listen == NULL) return RTERR_NOTINIT;

   ntohl = (LPFN_NTOHL) GetProcAddress (ws32, "ntohl");
   if (ntohl == NULL) return RTERR_NOTINIT;

   ntohs = (LPFN_NTOHS) GetProcAddress (ws32, "ntohs");
   if (ntohs == NULL) return RTERR_NOTINIT;

   recv = (LPFN_RECV) GetProcAddress (ws32, "recv");
   if (recv == NULL) return RTERR_NOTINIT;

   send = (LPFN_SEND) GetProcAddress (ws32, "send");
   if (send == NULL) return RTERR_NOTINIT;

   select = (LPFN_SELECT) GetProcAddress (ws32, "select");
   if (select == NULL) return RTERR_NOTINIT;

   setsockopt = (LPFN_SETSOCKOPT) GetProcAddress (ws32, "setsockopt");
   if (setsockopt == NULL) return RTERR_NOTINIT;

   shutdown = (LPFN_SHUTDOWN) GetProcAddress (ws32, "shutdown");
   if (shutdown == NULL) return RTERR_NOTINIT;

   socket = (LPFN_SOCKET) GetProcAddress (ws32, "socket");
   if (socket == NULL) return RTERR_NOTINIT;

   WSAGetLastError = (LPFN_WSAGETLASTERROR)
      GetProcAddress (ws32, "WSAGetLastError");
   if (WSAGetLastError == NULL) return RTERR_NOTINIT;
#endif
   if (wsaStartup (MAKEWORD(1, 1), &wsaData) == -1) return RTERR_NOTINIT;
#endif
   return 0;
}

EXTRTMETHOD int rtxSocketCreate (OSRTSOCKET* psocket)
{
   int on;
#ifndef __SYMBIAN32__
   struct linger linger;
#endif
   OSRTSOCKET sock;

   rtxSocketsInit();

   sock = socket (AF_INET, SOCK_STREAM, 0);
   if (sock == OSRTSOCKET_INVALID) {
      return errToRTERR (ERRNO);
   }
   on = 1;
#ifndef __SYMBIAN32__
   if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
                   (const char* ) &on, sizeof (on)) == -1) {
      closesocket (sock);
      return errToRTERR (ERRNO);
   }

   /* To perform graceful disconnect it is necessary to set SO_DONTLINGER
      option rather than SO_LINGER.
      setting linger.l_onoff to zero with SO_LINGER is equivalent
      to SO_DONTLINGER. */
   linger.l_onoff = 0;
   linger.l_linger = 0;

   if (setsockopt (sock, SOL_SOCKET, SO_LINGER,
                   (const char* ) &linger, sizeof (linger)) == -1) {
      closesocket (sock);
      return errToRTERR (ERRNO);
   }

#else
   if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
                   (void* ) &on, sizeof (on)) == -1) {
      closesocket (sock);
      return errToRTERR (ERRNO);
   }

#endif
   *psocket = sock;
   return 0;
}

EXTRTMETHOD int rtxSocketCreateUDP (OSRTSOCKET *psocket)
{
   int on;

   OSRTSOCKET sock = socket (AF_INET, SOCK_DGRAM, 0);

   if (sock == OSRTSOCKET_INVALID) {
      return errToRTERR (ERRNO);
   }
   on = 1;

   if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
            (const char* ) &on, sizeof (on)) == -1) {
      closesocket (sock);
      return errToRTERR (ERRNO);
   }

   *psocket = sock;

   return 0;
}

EXTRTMETHOD int rtxSocketClose (OSRTSOCKET sock)
{
   shutdown (sock, SHUTDOWN_FLAGS);
   if (closesocket (sock) == -1)
      return errToRTERR (ERRNO);
   return 0;
}

EXTRTMETHOD int rtxSocketBind (OSRTSOCKET sock, OSIPADDR addr, int port)
{
   struct sockaddr_in m_addr;

   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
   OSCRTLMEMSET (&m_addr, 0, sizeof (m_addr));
   m_addr.sin_family = AF_INET;
   m_addr.sin_addr.s_addr = (addr == 0) ? INADDR_ANY : htonl (addr);
   m_addr.sin_port = htons ((unsigned short)port);

   if (bind (sock, (struct sockaddr *) (void*) &m_addr,
                     sizeof (m_addr)) == -1)
      return errToRTERR (ERRNO);

   return 0;
}

EXTRTMETHOD int rtxSocketListen (OSRTSOCKET sock, int maxConnection)
{
   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
   if (listen (sock, maxConnection) == -1)
      return errToRTERR (ERRNO);

   return 0;
}

EXTRTMETHOD int rtxSocketAccept (OSRTSOCKET sock, OSRTSOCKET *pNewSocket,
                                 OSIPADDR* destAddr, int* destPort)
{
   struct sockaddr_in m_addr;
   os_socklen_t addr_length = sizeof (m_addr);

   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
   if (pNewSocket == 0) return RTERR_INVPARAM;

   *pNewSocket = accept (sock, (struct sockaddr *) (void*) &m_addr,
                         &addr_length);
   if (*pNewSocket <= 0) {
      return errToRTERR (ERRNO);
   }
   if (destAddr != 0)
      *destAddr = ntohl (m_addr.sin_addr.s_addr);

   if (destPort != 0)
      *destPort = ntohs (m_addr.sin_port);

   return 0;
}

EXTRTMETHOD int rtxSocketConnect (OSRTSOCKET sock, const char* host, int port)
{
   struct sockaddr_in m_addr;
   int ret;

   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
   OSCRTLMEMSET (&m_addr, 0, sizeof (m_addr));

   m_addr.sin_family = AF_INET;
   m_addr.sin_port = htons ((unsigned short)port);

   ret = rtxSocketGetHost (host, &m_addr.sin_addr);
   if (0 != ret) return ret;

   if (connect (sock, (struct sockaddr *) (void*) &m_addr,
                sizeof (m_addr)) == -1)
      return errToRTERR (ERRNO);

   return 0;
}

EXTRTMETHOD int rtxSocketConnectTimed
(OSRTSOCKET sock, const char* host, int port, int nsecs)
{
   fd_set fds;
   struct timeval timeout;
   struct sockaddr_in m_addr;
   int res, ret;

   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
   OSCRTLMEMSET (&m_addr, 0, sizeof (m_addr));

   m_addr.sin_family = AF_INET;
   m_addr.sin_port = htons ((unsigned short)port);

   ret = rtxSocketGetHost (host, &m_addr.sin_addr);
   if (0 != ret) return ret;

   /* Set non-blocking */
   ret = rtxSocketSetBlocking (sock, FALSE);
   if (0 != ret) return ret;

   /* Try to connect */
   res = connect (sock, (struct sockaddr*) (void*)&m_addr, sizeof (m_addr));

   if (res < 0) {
      int errnum = ERRNO;
      if (errnum == OSYS_SE_INPROGRESS || errnum == OSYS_SE_WOULDBLOCK) {
         timeout.tv_sec = nsecs;
         timeout.tv_usec = 0;

         FD_ZERO (&fds);
         FD_SET (sock, &fds);

         res = rtxSocketSelect ((int)(sock + 1), 0, &fds, 0, &timeout);

         if (res < 0) {
            ret = res;
         }
         else if (res == 0) {  /* timeout */
            ret = -1;
         }
         else {
#if defined(_WIN32) || defined(_WIN64)
            int optlen = (int) sizeof(int), valopt;
            getsockopt (sock, SOL_SOCKET, SO_ERROR,
                        (char*)(&valopt), &optlen);
#elif defined(VMS) || defined(__VMS)
            size_t optlen = (int) sizeof(int), valopt;
            getsockopt (sock, SOL_SOCKET, SO_ERROR,
                        (char*)(&valopt), &optlen);
#else
            socklen_t optlen = sizeof(int), valopt;
            getsockopt (sock, SOL_SOCKET, SO_ERROR,
                        (void*)(&valopt), &optlen);
#endif
            ret = (0 == valopt) ? 0 : -1;
         }
      }
      else ret = errToRTERR (errnum);
   }

   /* Restore blocking mode */
   res = rtxSocketSetBlocking (sock, TRUE);
   if (0 != res && 0 == ret) return res;

   return ret;
}

EXTRTMETHOD int rtxSocketSelect
(int nfds, fd_set* readfds, fd_set* writefds,
 fd_set* exceptfds, struct timeval* timeout)
{
   int ret;
#if defined (_WIN32) || defined (_WIN64)
   ret = select (nfds, readfds, writefds, exceptfds,
                 (const struct timeval *) timeout);
#else
   ret = select (nfds, readfds, writefds, exceptfds, timeout);
#endif
   return ret;
}

EXTRTMETHOD int rtxSocketSend
(OSRTSOCKET sock, const OSOCTET* pdata, size_t size)
{
   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
#if defined(_WIN32) || defined(_WIN64)
   if (send (sock, (const char*) pdata, (int) size, SEND_FLAGS) == -1)
#else
   if (send (sock, (void*) pdata, size, SEND_FLAGS) == -1)
#endif
   {
#ifdef _TRACE
      perror ("send");
#endif
      return errToRTERR (ERRNO);
   }

   return 0;
}

EXTRTMETHOD int rtxSocketRecv
(OSRTSOCKET sock, OSOCTET* pbuf, size_t bufsize)
{
   int len;
   if (sock == OSRTSOCKET_INVALID) {
      return RTERR_INVSOCKET;
   }
#if defined(_WIN32) || defined(_WIN64)
   if ((len = recv (sock, (char*) pbuf, (int)bufsize, 0)) == -1)
#else
   if ((len = recv (sock, (void*) pbuf, bufsize, 0)) == -1)
#endif
   {
#ifdef _TRACE
      perror ("recv");
#endif
      return errToRTERR (ERRNO);
   }

   return len;
}

EXTRTMETHOD int rtxSocketRecvTimed
(OSRTSOCKET sock, OSOCTET* pbuf, size_t bufsize, OSUINT32 secs)
{
   if (sock != OSRTSOCKET_INVALID)
   {
#if defined(_WIN32) || defined(_WIN64)
      DWORD msecs = secs * 1000;
      setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO,
                  (const char*)&msecs, sizeof(DWORD));
#else
      struct timeval timeout;
      timeout.tv_sec = secs;
      timeout.tv_usec = 0;
      setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO,
                  (char*)&timeout, sizeof(timeout));
#endif
      return rtxSocketRecv (sock, pbuf, bufsize);
   }
   else return RTERR_INVSOCKET;
}

EXTRTMETHOD int rtxSocketStrToAddr (const char* pIPAddrStr, OSIPADDR* pIPAddr)
{
   int i = 0, parts[4];
   const char* sp = pIPAddrStr;

   while (*sp != 0 && i < 4) {
      parts[i] = 0;
      while (*sp != 0 && *sp != '.') {
         if (*sp >= '0' && *sp <= '9') {
            parts[i] = (parts[i] * 10) + (*sp - '0');
         }
         else if (*sp != ' ') return RTERR_INVPARAM;
         sp++;
      }
      if (*sp == '.') { sp++; i++; }
   }

   if (i != 3 ||
      (parts[0] < 0 || parts[0] > 256) || (parts[1] < 0 || parts[1] > 256) ||
      (parts[2] < 0 || parts[2] > 256) || (parts[3] < 0 || parts[3] > 256))
      return RTERR_INVPARAM;

   *pIPAddr = ((parts[0] & 0xFF) << 24) | ((parts[1] & 0xFF) << 16) |
              ((parts[2] & 0xFF) << 8) | (parts[3] & 0xFF);

   return 0;
}

EXTRTMETHOD int rtxSocketAddrToStr (OSIPADDR ipAddr, char* pbuf, size_t bufsize)
{
   size_t i, rshift = 24, cnt = 0;
   OSINT32 ret, value;

   if (bufsize < 8)
      return RTERR_BUFOVFLW;

   for (i = 0; i < 4; i++) {
      value = (OSINT32)((ipAddr >> rshift) & 0xFF);
      rshift -= 8;

      ret = rtxIntToCharStr (value, pbuf+cnt, bufsize-cnt, 0);
      if (ret < 0) return ret;
      else cnt = (size_t)ret;

      if (cnt < bufsize && i < 3) pbuf[cnt++] = '.';
   }

   if (cnt < bufsize) pbuf[cnt] = '\0';

   return 0;
}

EXTRTMETHOD int rtxSocketGetHost (const char* host, struct in_addr *inaddr)
{
   OSUINT32 iadd;
   struct hostent* pHostEnt;

   iadd = inet_addr (host);
   if ((int)iadd != -1) {
      OSCRTLMEMCPY (inaddr, &iadd, sizeof(iadd));
      return 0;
   }

   pHostEnt = gethostbyname (host);
   if (0 != pHostEnt) {
      OSCRTLMEMCPY (inaddr, pHostEnt->h_addr, pHostEnt->h_length);
      return 0;
   }
   else return RTERR_HOSTNOTFOU;
}

EXTRTMETHOD int rtxSocketParseURL
(char* url, char** protocol, char** address, int* port)
{
   char* sp;

   if (0 == url || 0 == protocol || 0 == address || 0 == port)
      return RTERR_INVPARAM;

   /* Parse protocol and address */
   sp = strstr (url, "://");
   if (sp != 0) {
      *protocol = url;
      *address = sp + 3;
      *sp = '\0';
   }
   else {
      *protocol = 0;
      *address = url;
   }

   /* Parse port */
   sp = strchr (*address, ':');
   if (sp != 0) {
      *sp++ = '\0';
      *port = atoi (sp);
   }
   else if (*protocol != 0 && !OSCRTLSTRCMP (*protocol, "http"))
      *port = 80;
   else
      *port = 0;

   return 0;
}

EXTRTMETHOD int rtxSocketSetBlocking (OSRTSOCKET sock, OSBOOL value)
{
   int ret = 0;
#if defined(_WIN32)
   unsigned long nonblock = (value) ? 0 : 1;
   ret = ioctlsocket (sock, FIONBIO, &nonblock);
#else
   int flags =  fcntl (sock, F_GETFL, 0);
   if (value)
      flags &= (~O_NONBLOCK);
   else
      flags |= O_NONBLOCK;

   fcntl (sock, F_SETFL, flags);
#endif
   return ret;
}

#endif /* _OS_NOSOCKET */
