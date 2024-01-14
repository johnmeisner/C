/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: tlsSocket.c                                                      */
/* Purpose: provide socket and TLS interface  for the TLS lib                 */
/*                                                                            */
/* Copyright (C) 2017 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2017-07-2x  DSJGREEN      Initial Release                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <resolv.h>
#include <pthread.h>
#include "wsu_util.h"
#include "tlsImpl.h"

//************************* Include right libraries **************************//
#ifndef EAI_NODATA
#  define EAI_NODATA	  -5	/* No address associated with NAME.  */
#endif
#ifndef EAI_ADDRFAMILY  
#  define EAI_ADDRFAMILY  -9	/* Address family for NAME not supported.  */
#endif
//************************* Include right libraries **************************//

/* constants */
#define FAILURE        0
#define SUCCESS        1
#define CLEAR_DATA     0
#define SOCKET_ERROR   -1
#define TERMINATOR_LEN 2
#define MAX_TXBUFLEN   512

extern bool TLS_API_LOGGING;
extern bool TLS_ADDITIONAL_LOGGING;
extern bool TLS_CONSOLE_LOGGING;

// Configuration variables
extern uint8_t cfgIpVersion;
extern bool    cfgTlsRootCertificateCheck;

/**
 * This is a routine to initialize SSL
 */
int32_t tlsInitSsl(connUserInfo *uci)
{

    uci->Ssl.initialized = 0;
    uci->Ssl.certbio     = NULL;
    uci->Ssl.outbio      = NULL;
    uci->Ssl.cert        = NULL;
    uci->Ssl.certname    = NULL;
    uci->Ssl.ssl         = NULL;
    uci->Ssl.server      = 0;
    memset(uci->scmsIpAddr, 0, INET6_ADDRSTRLEN);

    /* create input/output BIOs */
    uci->Ssl.certbio = BIO_new(BIO_s_file());
    uci->Ssl.outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

    /* set tlsv1.2 client hello, also announce  */
    uci->Ssl.method = TLSv1_2_client_method();

    /* create a new ssl context structure */
    if ((uci->Ssl.ctx = SSL_CTX_new(uci->Ssl.method)) == NULL) {
        TLS_LOG(1, "%s(): Unable to create a new SSL context structure.\n", __func__);
        printf("LIBTLS %s(): Unable to create a new SSL context structure.\n", __func__);
        return SOCKET_ERROR;
    }

    /* disable all methods up to and including tls 1.1  */
    SSL_CTX_set_options(uci->Ssl.ctx, SSL_OP_NO_TLSv1);

    /* create a new ssl connection state object */
    uci->Ssl.ssl = SSL_new(uci->Ssl.ctx);

    if (cfgTlsRootCertificateCheck) {
        /* set paths and files for verify certs for 5900 */
        if (SSL_CTX_load_verify_locations(uci->Ssl.ctx, "/home/root/aerolink/certificates/root.tls", NULL) != 1) {
            ERR_print_errors_fp(stderr);
        }
    }

    /* end of ssl initialization */
    TLS_LOG(1, "%s(): openssl init complete\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s(): openssl init complete\n", __func__);

    /* set flag */
    uci->Ssl.initialized = SslInitialized;

    return SslInitialized;
}

/**
 * Initializer for openSsl library, should only be done once.
 */
uint32_t tlsLibInit(void)
{
    /* init ssl library and register algos */
    if (SSL_library_init() < 0) {
        TLS_LOG(1, "%s(): Could not initialize the OpenSSL library !\n", __func__);
        printf("LIBTLS %s(): Could not initialize the OpenSSL library !\n", __func__);
        return SOCKET_ERROR;
    }

    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    return SslInitialized;
}

/**
 * This is a routine to check a server address and create a socket to connect
 * to it. This supports the OBS's need to verify the address right away and
 * tell it if it fails to verify correctly.
 */
int32_t libtlsCheckAddress(connUserInfo *ui, char const *address, uint16_t port, int *sockin)
{
    int             gaierr = 0;
    int             sock   = 0;
    struct addrinfo hints, *si, *results;
    char            portbuf[6];
    uint8_t         ipVersion = (ui->Ssl.ipVersion == AF_INET6) ? AF_INET6 : AF_INET;
    int             flags = 0;

    if (ipVersion == AF_INET) {
        TLS_LOG(1, "%s(): ipv4 mode - addr: %s\n", __func__, address);
        TLS_CONSOLE_LOG("LIBTLS %s(): ipv4 mode - addr: %s\n", __func__, address);
        setenv("SOCK", "", 1);
    }
    else {
        TLS_LOG(1, "%s(): ipv6 mode - addr: %s\n", __func__, address);
        TLS_CONSOLE_LOG("LIBTLS %s(): ipv6 mode - addr: %s\n", __func__, address);
        setenv("SOCK", "/llc", 1);
    }

    /* Pass this IP address into getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = ipVersion;
    hints.ai_socktype = SOCK_STREAM;
    TLS_LOG(1, "%s(): using ipv%d for socket/connection\n", __func__, (ipVersion == AF_INET) ? 4 : 6);
    TLS_CONSOLE_LOG("LIBTLS %s(): using ipv%d for socket/connection\n", __func__, (ipVersion == AF_INET) ? 4 : 6);

    /* Get information about this server; getaddinfo() automatically does a DNS lookup */
    snprintf(portbuf, 6, "%u", port);
    gaierr = getaddrinfo(address, portbuf, &hints, &results);
    TLS_LOG(1, "%s(): getaddrinfo(\"%s\", \"%s\", %p, %p) returns: %d\n", __func__, address, portbuf, &hints, &results, gaierr);
    TLS_CONSOLE_LOG("LIBTLS %s(): getaddrinfo(\"%s\", \"%s\", %p, %p) returns: %d\n", __func__, address, portbuf, &hints, &results, gaierr);

    if (gaierr != 0) {
        /* getaddrinfo() failed */
        TLS_LOG(1, "%s(): gai fail code: %d  err defined: %s   -- check resolv.conf -- \n", __func__, gaierr, gai_strerror(gaierr));
        printf("LIBTLS %s(): gai fail code: %d  err defined: %s   -- check resolv.conf -- \n", __func__, gaierr, gai_strerror(gaierr));

        switch (gaierr) {
        case EAI_AGAIN:
        case EAI_BADFLAGS:
        case EAI_FAIL:
        case EAI_NODATA:
        case EAI_NONAME:
        case EAI_SERVICE:
            return AerolinkPKI_ERR_HOST_NOT_RESOLVABLE;
        break;

        case EAI_FAMILY:
        case EAI_ADDRFAMILY:
            return AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED;
        break;

        default:
            return AerolinkPKI_ERR_HOST_NOT_RESOLVABLE;
        }
    }

    /* Loop through the linked list of all getaddinfo() results */
    si = results;

    while (si != NULL) {
        if (si->ai_family == ipVersion) {
            /* We found a suitable entry; use it */
            ui->tlsSockaddrLen = si->ai_addrlen;
            memcpy(&ui->tlsSockaddr, si->ai_addr, si->ai_addrlen) ;
            break;
        }

        si = si->ai_next;
    }

    if (si == NULL) {
        /* No suitable result found in the linked list */
        TLS_LOG(1, "%s(): Reached end of linked list from getaddrinfo\n", __func__);
        printf("LIBTLS %s(): Reached end of linked list from getaddrinfo\n", __func__);
        return AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED;
    }

    /* Convert the IP address to human-readable format */
    if (si->ai_family == AF_INET) {
        inet_ntop(AF_INET, &((struct sockaddr_in  *)si->ai_addr)->sin_addr, ui->scmsIpAddr, si->ai_addrlen);
    }
    else {
        inet_ntop(AF_INET6, &((struct sockaddr_in6 *)si->ai_addr)->sin6_addr, ui->scmsIpAddr, si->ai_addrlen);
    }

    TLS_LOG(1, "%s(): create a socket - family: %d  socktype: %d   proto: %d  addr: %s  addrlen: %d\n", __func__,
            si->ai_family, si->ai_socktype, si->ai_protocol, ui->scmsIpAddr, si->ai_addrlen);
    TLS_CONSOLE_LOG("LIBTLS %s(): create a socket - family: %d  socktype: %d   proto: %d  addr: %s  addrlen: %d\n", __func__,
                    si->ai_family, si->ai_socktype, si->ai_protocol, ui->scmsIpAddr, si->ai_addrlen);

    /* Create the socket */
    if ((sock = socket(si->ai_family, si->ai_socktype, si->ai_protocol)) == SOCKET_ERROR) {
        freeaddrinfo(results);
        TLS_LOG(1, "%s(): socket fail: %s\n", __func__, strerror(errno));
        printf("LIBTLS %s(): socket fail: %s\n", __func__, strerror(errno));
        return AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED;
    }

    TLS_LOG(1, "%s(): socket = %d\n", __func__, sock);
    TLS_CONSOLE_LOG("LIBTLS %s(): socket = %d\n", __func__, sock);

    /* Apparently we will have to make it non-blocking the old fashioned way */
    if ((flags = fcntl(sock, F_GETFL, 0)) < 0) {
        freeaddrinfo(results);
        TLS_LOG(1, "%s(): Close socket: %u\n",__func__, sock);
        TLS_CONSOLE_LOG("LIBTLS %s(): Close socket: %u\n",__func__, sock);
        close(sock);
        TLS_LOG(1, "%s(): flag read fail: %d\n", __func__, errno);
        printf("LIBTLS %s(): flag read fail: %d\n", __func__, errno);
        return -1;
    }
    flags |= O_NONBLOCK;      

    if (fcntl(sock, F_SETFL, flags) < 0) {
        freeaddrinfo(results);
        TLS_LOG(1, "%s(): Close socket: %u\n",__func__, sock);
        TLS_CONSOLE_LOG("LIBTLS %s(): Close socket: %u\n",__func__, sock);
        close(sock);
        TLS_LOG(1, "%s(): flag set fail: %d\n", __func__, errno);
        printf("LIBTLS %s(): flag set fail: %d\n", __func__, errno);
        return -1;
    }

    /* Success */
    *sockin = sock;
    freeaddrinfo(results);
    TLS_LOG(1, "%s(): return sock create success - sock: %d\n", __func__, sock);
    TLS_CONSOLE_LOG("LIBTLS %s(): return sock create success - sock: %d\n", __func__, sock);
    return AerolinkPKI_ERR_NONE;
}

/**
 * This is a routine to get dis-connect()ed from a server. initiatedByServer is
 * set to true if the disconnect was initiated by the server.
 */
void libtlsDisconnect(connUserInfo *ui, bool initiatedByServer)
{
    bool tlsConnected = ui->tlsConnected;
    char tlsLogLine4[MAX_LOG_LINE_LEN]; /* Since this routine is called from
                                         * multiple threads, I'll give it its
                                         * own log line on the local stack */
    bool reallyDidSomething = false;

    /* Only let one instance of libtlsDisconnect() run at a time, since this
     * can be called from both the main thread and the receive threads */
    pthread_mutex_lock(&disconnectMutex);
    ui->tlsConnected = false;

    /* Cancel the receive thread */
    if (ui->recvThr != (pthread_t)NULL) {
       TLS_LOG(4, "%s(): Canceling receive pthread: 0x%x\n", __func__, (unsigned int)ui->recvThr);
       TLS_CONSOLE_LOG("LIBTLS %s(): Canceling receive pthread: 0x%x\n", __func__, (unsigned int)ui->recvThr);
       pthread_cancel(ui->recvThr);
       ui->recvThr =  (pthread_t)NULL;
       reallyDidSomething = true;
    }

    if (ui->secureConnection) {
        /* ssl close */
        if (tlsConnected) {
            TLS_LOG(4, "%s(): Close TLS connection: 0p%p\n",__func__, ui->Ssl.ssl);
            TLS_CONSOLE_LOG("LIBTLS %s(): Close TLS connection: 0p%p\n",__func__, ui->Ssl.ssl);
            
            if (!initiatedByServer) {
                SSL_shutdown(ui->Ssl.ssl);
            }

            SSL_free(ui->Ssl.ssl);
            SSL_CTX_free(ui->Ssl.ctx);
            reallyDidSomething = true;
        }
    }

    // tcp close
    if (ui->tcpConnected) {
        ui->tcpConnected   = false;
        reallyDidSomething = true;
    }

    // close the socket if it was opened
    if (ui->tcpSocket != -1) {
        TLS_LOG(4, "%s(): Close socket: %u\n",__func__, ui->tcpSocket);
        TLS_CONSOLE_LOG("LIBTLS %s(): Close socket: %u\n",__func__, ui->tcpSocket);
        close(ui->tcpSocket);
        ui->tcpSocket      = -1;
        reallyDidSomething = true;
    }

    // clean up my connection info
    ui->connectionHandle = UnusedBlock;
    ui->dataCb           = NULL;
    ui->connectCbData    = NULL;
    ui->scmsPort         = 0;
    ui->recvThr          =  (pthread_t)NULL;
    strcpy(ui->scmsIpAddr, " ");

    if (reallyDidSomething) {
        TLS_LOG(4, "%s: Successful closing of socket/connection related stuff.\n", __func__);
        //TLS_CONSOLE_LOG("LIBTLS %s(): successful closing of socket/connection related stuff\n",__func__);

        if (TLS_API_LOGGING || TLS_CONSOLE_LOGGING) {
            printf("%s: Disconnected\n", __func__);
        }
    }

    /* Allow other instances of libtlsDisconnect() to run */
    pthread_mutex_unlock(&disconnectMutex);
}

/**
 * This is a routine to get connect()ed to a server via TCP.
 */
static int32_t libtlsConnectTCP(connUserInfo *ui)
{
    int            result;
    int            err_no;
    fd_set         fdset;
    struct timeval tv;
    socklen_t      len;

    /* timeout on select() */
    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    /* Attempt a TCP connect */
    result = connect(ui->tcpSocket, (struct sockaddr *)&ui->tlsSockaddr, ui->tlsSockaddrLen);

    if (0 <= result) {
        /* TCP connect success */
        TLS_LOG(1, "%s(): (1) tcp connected to %s\n", __func__, ui->scmsIpAddr);
        TLS_CONSOLE_LOG("LIBTLS %s(): (1) tcp connected to %s\n", __func__, ui->scmsIpAddr);
    }
    else {
        /* No TCP connection (yet) */
        if (EINPROGRESS != errno) {
            /* A real error */
            err_no = errno;
        }
        else {
            /* TCP connection in progress, wait until we are connected,
             * then get the connection success/fail results */
            TLS_LOG(1, "%s(): connect in progress\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): connect in progress\n", __func__);
            FD_ZERO(&fdset);
            FD_SET(ui->tcpSocket, &fdset);
            result = select(ui->tcpSocket+1, &fdset, &fdset, NULL, &tv);

            /* At this point, either the select() failed or timed out, or
             * the TCP connect succeeded or failed */
            TLS_LOG(1, "%s(): tcp connect result from select(): %d\n", __func__, result);
            TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect result from select(): %d\n", __func__, result);

            if (-1 == result) {
                /* select() failed */
                TLS_LOG(1, "%s(): tcp connect select FAIL: %s\n", __func__, strerror(errno));
                TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect select FAIL: %s\n", __func__, strerror(errno));
                return AerolinkPKI_ERR_HOST_UNREACHABLE;
            }
            else if (0 == result) {
                /* select() timed out */
                TLS_LOG(1, "%s(): select TIMEOUT\n", __func__);
                TLS_CONSOLE_LOG("LIBTLS %s(): select TIMEOUT\n", __func__);
                return AerolinkPKI_ERR_HOST_UNREACHABLE;
            }

            /* Initialize len with size of err_no */
            len = sizeof(err_no);

            /* select() succeeded; get the TCP connect success/fail code */
            result = getsockopt(ui->tcpSocket, SOL_SOCKET, SO_ERROR, &err_no, &len);

            if (result == -1) {
                /* getsockopt() failed */
                TLS_LOG(1, "%s(): tcp connect getsockopt FAIL: %s\n", __func__, strerror(errno));
                TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect getsockopt FAIL: %s\n", __func__, strerror(errno));
                return AerolinkPKI_ERR_HOST_UNREACHABLE;
            }
        }

        /* At this point, err_no contains the TCP connect success/fail code */
        if (0 == err_no) {
            /* TCP connect was successful */
            TLS_LOG(1, "%s(): tcp connect success\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect success\n", __func__);
        }
        else if (EISCONN == err_no) {
            /* The socket is already connected */
            TLS_LOG(1, "%s(): tcp connect() errno = already connected\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect() errno = already connected\n", __func__);
        }
        else {
            /* TCP connect error */
            TLS_LOG(1, "%s(): tcp connect fail: %s\n", __func__, strerror(err_no));
            TLS_CONSOLE_LOG("LIBTLS %s(): tcp connect fail: %s\n", __func__, strerror(err_no));
            return (ETIMEDOUT == err_no) ? AerolinkPKI_ERR_HOST_UNREACHABLE :
                                           AerolinkPKI_ERR_BAD_CONNECTION_HANDLE;
        }
    }
    
    return AerolinkPKI_ERR_NONE;
}

/**
 * This routine converts a socket associated with ui to be non-blocking
 */
#ifdef OBU_IPV6
static int32_t libtlsMakeSocketNonBlocking(connUserInfo *ui)
{
    int flags = fcntl(ui->tcpSocket, F_GETFL, 0);

    /* can you make a socket non blocking after being tcp connected? - apparently. */
    if (flags < 0) {
        TLS_LOG(1, "%s(): flag read fail: %d\n", __func__, errno);
        TLS_CONSOLE_LOG("LIBTLS %s(): flag read fail: %d\n", __func__, errno);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    flags |= O_NONBLOCK;   

    if (fcntl(ui->tcpSocket, F_SETFL, flags) < 0) {
        TLS_LOG(1, "%s(): flag set fail: %d\n", __func__, errno);
        TLS_CONSOLE_LOG("LIBTLS %s(): flag set fail: %d\n", __func__, errno);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    return AerolinkPKI_ERR_NONE;
}
#endif
/**
 * This is a routine to get connect()ed to a server via SSL after already
 * connection successfully via TCP.
 */
static int32_t libtlsConnectSSL(connUserInfo *ui)
{
    int  sslFuncReturn;
    long sslVerifyFuncReturn;

    ui->Ssl.server = ui->tcpSocket;
    TLS_LOG(1, "%s(): begin tls connect on socket %d\n", __func__, ui->tcpSocket);
    TLS_CONSOLE_LOG("LIBTLS %s(): begin tls connect on socket %d\n", __func__, ui->tcpSocket);

    if (SUCCESS != SSL_set_fd(ui->Ssl.ssl, ui->tcpSocket)) {
        TLS_LOG(1, "%s(): ssl_set_fd returns Fail, fd: %d\n", __func__, ui->Ssl.server);
        TLS_CONSOLE_LOG("LIBTLS %s(): ssl_set_fd returns Fail, fd: %d\n", __func__, ui->Ssl.server);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    TLS_LOG(1, "%s(): ssl_set_fd returns success, fd: %d\n", __func__, ui->tcpSocket);
    TLS_CONSOLE_LOG("LIBTLS %s(): ssl_set_fd returns success, fd: %d\n", __func__, ui->tcpSocket);

    if (cfgTlsRootCertificateCheck) {
        /* Set verification depth and mode (SSL_VERIFY_NONE still does
         * verification, but requires call to SSL_get_verify_result() to see if
         * verification succeeded, and does not cause SSL_connect() to fail) */
        SSL_set_verify_depth(ui->Ssl.ssl, 4);
        SSL_set_verify(ui->Ssl.ssl, SSL_VERIFY_NONE, NULL);
    }

    /* attempt SSL connection */
    sslFuncReturn = SSL_connect(ui->Ssl.ssl);
    TLS_LOG(1, "%s(): SSL_connect returns: %d \n", __func__, sslFuncReturn);
    TLS_CONSOLE_LOG("LIBTLS %s(): SSL_connect returns: %d \n", __func__, sslFuncReturn);

    if (sslFuncReturn < 0) {
        /* TLS connect failed */
        switch (SSL_get_error(ui->Ssl.ssl, sslFuncReturn)) {
        case SSL_ERROR_NONE:
            TLS_LOG(1, "%s(): ssl_connect returns ERR NONE\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR NONE\n", __func__);
        break;
        case SSL_ERROR_ZERO_RETURN:
            TLS_LOG(1, "%s(): ssl_connect returns ERR ZERORETURN\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR ZERORETURN\n", __func__);
        break;
        case SSL_ERROR_WANT_READ:
            TLS_LOG(1, "%s(): ssl_connect returns ERR WANT READ - try again to ssl_connect()\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR WANT READ - try again to ssl_connect()\n", __func__);
            /* The SSL_connect fails with SSL_ERROR_WANT_READ */
            /* Try it out until the error code is non SSL_ERROR_WANT_READ */
            /* Or for 5 times before giving up */

            unsigned short int sslReadCount = 0;
            do {
                usleep(500000); /* 500ms */
                sslReadCount++;
                sslFuncReturn = SSL_connect(ui->Ssl.ssl);
            } while (((SSL_get_error(ui->Ssl.ssl, sslFuncReturn)) == SSL_ERROR_WANT_READ ) && (sslReadCount < 5));
        break;
        case SSL_ERROR_WANT_WRITE:
            TLS_LOG(1, "%s(): ssl_connect returns ERR WANT WRITE\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR WANT WRITE\n", __func__);
        break;
        case SSL_ERROR_WANT_CONNECT:
        case SSL_ERROR_WANT_ACCEPT:
            TLS_LOG(1, "%s(): ssl_connect returns ERR WANT CONNECTorACCEPT\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR WANT CONNECTorACCEPT\n", __func__);
        break;
        case SSL_ERROR_WANT_X509_LOOKUP:
            TLS_LOG(1, "%s(): ssl_connect returns ERR WANT X509\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR WANT X509\n", __func__);
        break;
        case SSL_ERROR_SYSCALL:
            TLS_LOG(1, "%s(): ssl_connect returns ERR SYSCALL, errno: %d\n", __func__, errno);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns ERR SYSCALL, errno: %d\n", __func__, errno);
        break;
        default:
            TLS_LOG(1, "%s(): ssl_connect returns UNKNOWN ERR, errno: %d\n", __func__, errno);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect returns UNKNOWN ERR, errno: %d\n", __func__, errno);
        }

        /* When SSL_ERROR_WANT_READ error is occurred, we try the SSL_connect again. */
        /* If the any of those attempts are successful, don't return here */
        if (sslFuncReturn < 0) {
            TLS_LOG(1, "%s(): ssl_connect fail, errno:%d\n", __func__, errno);
            TLS_CONSOLE_LOG("LIBTLS %s(): ssl_connect fail, errno:%d\n", __func__, errno);
            return AerolinkPKI_ERR_HOST_UNREACHABLE;
        }

    }

    TLS_LOG(1, "%s(): successfully connected TLS session to: %s\n", __func__, ui->scmsIpAddr);
    TLS_CONSOLE_LOG("LIBTLS %s(): successfully connected TLS session to: %s\n", __func__, ui->scmsIpAddr);

    /* Get a pointer to the certificate */
    ui->Ssl.cert = SSL_get_peer_certificate(ui->Ssl.ssl);

    if (ui->Ssl.cert == NULL) {
        TLS_LOG(1, "%s(): Error: Could not get a certificate from: %s.\n", __func__, ui->scmsIpAddr);

        if (TLS_CONSOLE_LOGGING) {
            BIO_printf(ui->Ssl.outbio, "Error: Could not get a certificate from: %s.\n", ui->scmsIpAddr);
        }
    }
    else {
        TLS_LOG(1, "%s(): Retrieved the server's certificate from: %s.\n", __func__, ui->scmsIpAddr);

        if (TLS_CONSOLE_LOGGING) {
            BIO_printf(ui->Ssl.outbio, "Retrieved the server's certificate from: %s.\n", ui->scmsIpAddr);
        }
    }

    if (cfgTlsRootCertificateCheck) {
        /* Verify the certificate */
        sslVerifyFuncReturn = SSL_get_verify_result(ui->Ssl.ssl);

        if (sslVerifyFuncReturn != X509_V_OK) {
            TLS_LOG(1, "%s(): SSL_get_verify_result returns Fail (%ld)\n", __func__, sslVerifyFuncReturn);
            printf("LIBTLS %s(): SSL_get_verify_result returns Fail (%ld)\n", __func__, sslVerifyFuncReturn);
            return AerolinkPKI_ERR_HOST_UNREACHABLE;
        }
    }

    // set for gui
    ui->connectAttempts = 0;
    ui->tlsConnected    = true;

    if ((TLS_API_LOGGING || TLS_CONSOLE_LOGGING) && (ui->Ssl.cert != NULL)) {
        //  extract various certificate information
        ui->Ssl.certname = X509_NAME_new();
        ui->Ssl.certname = X509_get_subject_name(ui->Ssl.cert);

        // display the cert subject here  REMOVE FOR PRODUCTION
        if (TLS_CONSOLE_LOGGING) {
            BIO_printf(ui->Ssl.outbio, "Displaying the certificate subject data:\n");
        }

        X509_NAME_print_ex(ui->Ssl.outbio, ui->Ssl.certname,3, 0);

        if (TLS_CONSOLE_LOGGING) {
            BIO_printf(ui->Ssl.outbio, "\n");
        }
    }

    return AerolinkPKI_ERR_NONE;
}

/**
 * This is a routine to get connect()ed to a server.
 */
int32_t libtlsConnect(connUserInfo *ui)
{
    int32_t retVal;

    TLS_LOG(1, "%s(): ipv%c sock: %d   address: %s\n", __func__, (ui->Ssl.ipVersion == AF_INET) ? '4' : '6', ui->tcpSocket, ui->scmsIpAddr);
    TLS_CONSOLE_LOG("LIBTLS %s(): ipv%c sock: %d   address: %s\n", __func__, (ui->Ssl.ipVersion == AF_INET) ? '4' : '6', ui->tcpSocket, ui->scmsIpAddr);

    /* If already connected, pretend we connected successfully */
    if (ui->tcpConnected) {
        TLS_LOG(1, "%s(): Already connected for this sock\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): Already connected for this sock\n", __func__);
        return AerolinkPKI_ERR_NONE;
    }

    ui->tlsConnected = false;

    /* Connect via TCP */
    retVal = libtlsConnectTCP(ui);

    if (retVal != AerolinkPKI_ERR_NONE) {
        goto tlsConnectErr;
    }

    ui->tcpConnected = true;

    /* Now TCP connect is complete so prepare for TLS connection */
    if (ui->secureConnection) {
        retVal = libtlsConnectSSL(ui);

        if (retVal != AerolinkPKI_ERR_NONE) {
            goto tlsConnectErr;
        }
    }

    if (TLS_API_LOGGING || TLS_CONSOLE_LOGGING) {
        printf("%s: Connected\n", __func__);
    }

    return AerolinkPKI_ERR_NONE;

tlsConnectErr:
    libtlsDisconnect(ui, false);
    return retVal;
}

/**
 * This is a routine to send a packet via TLS
 */
int32_t tlsSend(connUserInfo *ui, uint8_t const* const data, size_t len)
{
    uint16_t retval = 0;

    if (len > 0) {
        if (!ui->tlsConnected) {
            TLS_LOG(1, "%s(): write to a non-connected socket/fd. Exiting.\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s(): write to a non-connected socket/fd. Exiting.\n", __func__);
            return AerolinkPKI_ERR_CONNECTION_CLOSED;
        }

        retval = SSL_write(ui->Ssl.ssl, data, len);

        if (retval == len) {
            /* Write succeeded */
            TLS_LOG(1, "%s(): SSL_write success, retval: %d\n", __func__, retval);
            TLS_CONSOLE_LOG("LIBTLS %s(): SSL_write success, retval: %d\n", __func__, retval);
            return retval;
        }
        else if (retval <= 0) {
            /* Write failed */
            switch (SSL_get_error(ui->Ssl.ssl, retval)) {
            case SSL_ERROR_ZERO_RETURN:
                TLS_LOG(1, "%s(): ssl_write returns ERR ZERO RETURN (probably disconnect)\n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR ZERO RETURN (probably disconnect)\n", __func__);
                return AerolinkPKI_ERR_CONNECTION_CLOSED;
            case SSL_ERROR_NONE:
                TLS_LOG(1, "%s(): ssl_write returns ERR NONE\n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR NONE\n", __func__);
                break;
            case SSL_ERROR_WANT_READ:
                TLS_LOG(1, "%s(): ssl_write returns ERR WANT READ\n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR WANT READ\n", __func__);
                break;
            case SSL_ERROR_WANT_WRITE:
                TLS_LOG(1, "%s(): ssl_write returns ERR WANT WRITE\n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR WANT WRITE\n", __func__);
                break;
            case SSL_ERROR_WANT_CONNECT:
            case SSL_ERROR_WANT_ACCEPT:
                TLS_LOG(1, "%s(): ssl_write returns ERR WANT CONNECT, should call connect again \n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR WANT CONNECT, should call connect again \n", __func__);
                break;
            case SSL_ERROR_WANT_X509_LOOKUP:
                TLS_LOG(1, "%s(): ssl_write returns ERR WANT X509\n", __func__);
                printf("LIBTLS %s(): ssl_write returns ERR WANT X509\n", __func__);
                break;
            case SSL_ERROR_SYSCALL:
                TLS_LOG(1, "%s(): ssl_write returns ERR SYSCALL, errno: %d\n", __func__, errno);
                printf("LIBTLS %s(): ssl_write returns ERR SYSCALL, errno: %d\n", __func__, errno);
                break;
            case SSL_ERROR_SSL:
                TLS_LOG(1, "%s(): ssl_write returns ERR SSL, errno: %d\n", __func__, errno);
                printf("LIBTLS %s(): ssl_write returns ERR SSL, errno: %d\n", __func__, errno);
                break;
            default:
                TLS_LOG(1, "%s(): ssl_write returns UNKNOWN ERR, retval: %d\n", __func__, retval);
                printf("LIBTLS %s(): ssl_write returns UNKNOWN ERR, retval: %d\n", __func__, retval);
                break;
            }
        }
    }

    /* At this point, we either wrote fewer bytes than we requested, or the
     * write request was length zero */
    return AerolinkPKI_ERR_TRY_AGAIN;
}

/**
 * This is a routine to send a packet via unsecured
 */
int32_t unsecuredSend(connUserInfo *ui, uint8_t const* const data, size_t len)
{
    uint16_t retval = 0;

    if (len > 0) {
        retval = write(ui->tcpSocket, data, len);

        if (retval == len) {
            /* Write succeeded */
            TLS_LOG(1, "%s(): write success, retval: %d\n", __func__, retval);
            TLS_CONSOLE_LOG("LIBTLS %s(): write success, retval: %d\n", __func__, retval);
            return retval;
        }
        else if (retval == 0) {
            TLS_LOG(1, "%s(): write returns 0 (probably disconnect)\n", __func__);
            printf("LIBTLS %s(): write returns 0 (probably disconnect)\n", __func__);
            return AerolinkPKI_ERR_CONNECTION_CLOSED;
        }
        else if (retval <= 0) {
            /* Write failed */
            TLS_LOG(1, "%s(): write returns %u\n", __func__, retval);
            printf("LIBTLS %s(): write returns %u\n", __func__, retval);
            TLS_LOG(1, "%s(): write(): %s\n", __func__, strerror(errno));
            perror("LIBTLS write()");
        }
    }

    /* At this point, we either wrote fewer bytes than we requested, or the
     * write request was length zero */
    return AerolinkPKI_ERR_TRY_AGAIN;
}

/**
 * The thread to receive packets via TLS
 */
void *tlsRecvThread(connUserInfo *ui)
{
    int     retval = 0;
    static uint8_t rbuf[16000];
    fd_set         fdset;
    struct timeval tv;

    /* timeout on select() */
    /* Empirically it's found that we need to wait before 
       checking the socket in SSL_ERROR_WANT_READ */
    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    /* Wait here until the data becomes ready initially */    
    sleep(5);               


    TLS_LOG(3, "%s: Starting tlsRecvThread()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Starting tlsRecvThread()\n", __func__);

    for (;;) {
        if (!ui->tlsConnected) {
            TLS_LOG(3, "%s: ui->tlsConnected is false\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s: ui->tlsConnected is false\n", __func__);
            break;
        }

        memset(rbuf, 0, sizeof(rbuf));

        // if connected then look for data
        TLS_LOG(3, "%s(): Calling SSL_read(%p, %p, %d)\n", __func__, ui->Ssl.ssl, rbuf, sizeof(rbuf));
        retval = SSL_read(ui->Ssl.ssl, rbuf, sizeof(rbuf));
        TLS_LOG(3, "%s(): SSL_read() returned %d\n", __func__, retval);

        if (retval <= 0) {
            /* Read failed */
            TLS_LOG(3, "%s(): fails and returns: %d  \n", __func__, retval);
            //TLS_CONSOLE_LOG("LIBTLS %s(): fails and returns: %d  \n", __func__, retval);

            switch (SSL_get_error(ui->Ssl.ssl, retval)) {
            case SSL_ERROR_ZERO_RETURN:
                TLS_LOG(3, "%s(): ssl_read with ERR ZERO RETURN - probably server disconnect - exit this thread\n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read with ERR ZERO RETURN - probably server disconnect - exit this thread\n", __func__);
                goto tlsRecvThreadExit;
            case SSL_ERROR_NONE:
                TLS_LOG(3, "%s(): ssl_read returns ERR NONE\n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR NONE\n", __func__);
                break;
            case SSL_ERROR_WANT_READ:
                TLS_LOG(3, "%s(): ssl_read returns ERR WANT READ\n", __func__);

                /* It takes few tries to get past this error.  
                   So select call is implemented. It waits here 
                   until the select is successful (moves to another 
                   SSL state) or in error. */
                int rsock = SSL_get_rfd(ui->Ssl.ssl);

                FD_ZERO(&fdset);

                FD_SET(rsock,&fdset);

                int serr = select(rsock+1, &fdset, NULL, NULL, &tv);
                
                if (serr > 0)   {
                    TLS_LOG(3, "%s(): ssl_read Select returns %d\n", __func__,serr);
                    continue;
                }

                if ( serr == 0) {
                    TLS_LOG(3, "%s(): ssl_read Select Timesout %d\n", __func__,serr);
                    continue;
                }
                
                if ( serr == -1) {
                    TLS_LOG(3, "%s(): ssl_read Select Returned Error %d\n", __func__,serr);
                }

                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR WANT READ\n", __func__);
                break;
            case SSL_ERROR_WANT_WRITE:
                TLS_LOG(3, "%s(): ssl_read returns ERR WANT WRITE\n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR WANT WRITE\n", __func__);
                break;
            case SSL_ERROR_WANT_CONNECT:
            case SSL_ERROR_WANT_ACCEPT:
                TLS_LOG(3, "%s(): ssl_read returns ERR WANT CONNECT, should call connect again \n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR WANT CONNECT, should call connect again \n", __func__);
                break;
            case SSL_ERROR_WANT_X509_LOOKUP:
                TLS_LOG(3, "%s(): ssl_read returns ERR WANT X509\n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR WANT X509\n", __func__);
                break;
            case SSL_ERROR_SYSCALL:
                TLS_LOG(3, "%s(): ssl_read returns ERR SYSCALL - probably disconnect without notice, we should exit \n", __func__);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR SYSCALL - probably disconnect without notice, we should exit \n", __func__);
                goto tlsRecvThreadExit;
            case SSL_ERROR_SSL:
                TLS_LOG(3, "%s(): ssl_read returns ERR SSL, errno: %d\n", __func__, errno);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns ERR SSL, errno: %d\n", __func__, errno);
                break;
            default:
                TLS_LOG(3, "%s(): ssl_read returns UNKNOWN ERR, retval: %d\n", __func__, retval);
                //TLS_CONSOLE_LOG("LIBTLS %s(): ssl_read returns UNKNOWN ERR, retval: %d\n", __func__, retval);
                break;
            }

            continue;
        }

        /* Read succeeded */
        TLS_LOG(3, "%s():  succeeds and : %d bytes in the callback \n", __func__, retval);
        TLS_CONSOLE_LOG("LIBTLS %s():  succeeds and : %d bytes in the callback \n", __func__, retval);
        // fire the data recvd callback
        TLS_LOG_TXRX(rbuf, retval, false);
        TLS_LOG(3, "%s(): Calling callback (0p%p)(%d, 0p%p, %d, 0p%p)\n", __func__, ui->dataCb, AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
        //TLS_CONSOLE_LOG("LIBTLS %s(): Calling callback (0p%p)(%d, 0p%p, %d, 0p%p)\n", __func__, ui->dataCb, AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
        (ui->dataCb)(AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
    } // for (;;)

tlsRecvThreadExit:
    /* Report the connection closed to Aerolink */
    TLS_LOG(3, "%s: (ui->dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, %p)\n", __func__, ui->connectCbData);
    (ui->dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, ui->connectCbData);
    /* Close the connection to the server */
    ui->recvThr = (pthread_t)NULL;
    libtlsDisconnect(ui, true);
    TLS_LOG(3, "%s: Terminating tlsRecvThread()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Terminating tlsRecvThread()\n", __func__);
    pthread_exit(NULL);
}

/**
 * The thread to receive packets via unsecured
 */
void *unsecuredRecvThread(connUserInfo *ui)
{
    int            retval = 0;
    static uint8_t rbuf[16000];

    TLS_LOG(3, "%s: Starting unsecuredRecvThread()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Starting unsecuredRecvThread()\n", __func__);

    for (;;) {
        memset(rbuf, 0, sizeof(rbuf));

        // if connected then look for data
        TLS_LOG(3, "%s(): Calling read(%p, %p, %d)\n", __func__, ui->Ssl.ssl, rbuf, sizeof(rbuf));
        retval = read(ui->tcpSocket, rbuf, sizeof(rbuf));
        TLS_LOG(3, "%s(): read() returned %d\n", __func__, retval);
        
        if (retval == 0) {
            TLS_LOG(3, "%s(): read with return value of 0 - probably server disconnect - exit this thread\n", __func__);
            //TLS_CONSOLE_LOG("LIBTLS %s(): read with return value of 0 - probably server disconnect - exit this thread\n", __func__);
            goto unsecuredRecvThreadExit;
        }
        else if (retval <= 0) {
            /* Read failed */
            TLS_LOG(3, "%s(): fails and returns: %d  \n", __func__, retval);
            //TLS_CONSOLE_LOG("LIBTLS %s(): fails and returns: %d  \n", __func__, retval);
            TLS_LOG(1, "%s(): read(): %s\n", __func__, strerror(errno));
            perror("read(): ");
            continue;
        }

        /* Read succeeded */
        TLS_LOG(3, "%s():  succeeds and : %d bytes in the callback \n", __func__, retval);
        TLS_CONSOLE_LOG("LIBTLS %s():  succeeds and : %d bytes in the callback \n", __func__, retval);
        // fire the data recvd callback
        TLS_LOG_TXRX(rbuf, retval, false);
        TLS_LOG(3, "%s(): Calling callback (0p%p)(%d, 0p%p, %d, 0p%p)\n", __func__, ui->dataCb, AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
        //TLS_CONSOLE_LOG("LIBTLS %s(): Calling callback (0p%p)(%d, 0p%p, %d, 0p%p)\n", __func__, ui->dataCb, AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
        (ui->dataCb)(AerolinkPKI_ERR_NONE, rbuf, retval, ui->connectCbData);
    } // for (;;)

unsecuredRecvThreadExit:
    /* Report the connection closed to Aerolink */
    TLS_LOG(3, "%s: (ui->dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, %p)\n", __func__, ui->connectCbData);
    (ui->dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, ui->connectCbData);
    /* Close the connection to the server */
    ui->recvThr = (pthread_t)NULL;
    libtlsDisconnect(ui, true);
    TLS_LOG(3, "%s: Terminating unsecuredRecvThread()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Terminating unsecuredRecvThread()\n", __func__);
    pthread_exit(NULL);
}

