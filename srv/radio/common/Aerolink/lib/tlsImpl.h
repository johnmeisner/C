/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: tlsconn.h                                                        */
/* Purpose: provide network handling API for any apps that need it            */
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

#ifndef TLS_CONN_H_
#define TLS_CONN_H_

#include <stdint.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "aerolinkPKIConnection.h"

// for openssl -  note, may not need all of these
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#ifdef __cplusplus
extern "C" {
#endif

// macros
#define MaxConnections      1
#define UnusedBlock         ((int32_t)0xdeafbeef)
#define SslInitialized      0xc1a0d00d
#define NetworkAvailable    0xbabef00d
#define NetworkNotAvailable 0xfeedbabe

// enums
/* Note that tlsRse and tlsEthernet must be 0 and 1, as they are used as an
 * index into the tlsNA array. */
typedef enum {
    tlsEthernet       = 0,
    tlsRse            = 1,
    tlsNetworkUnavail = 400,
} tlsNetworkType;

// for openssl specific and tls in general
typedef struct {
    unsigned int      initialized;
    BIO              *certbio;
    BIO              *outbio;
    X509             *cert;
    X509_NAME        *certname;
    const SSL_METHOD *method;
    SSL_CTX          *ctx;
    SSL              *ssl;
    int               server;
    uint8_t           ipVersion;
} SSL_t;


// the network available struct
// note that we could have one connection ipv4 and one ipv6
typedef struct{
    NetworkAvailCallback  networkCb;
    void                 *registerCbData;
} tlsNetAvailInfo;

// the data that defines a tls users connection
typedef struct {
    int32_t               connectionHandle;
    ReceiveDataCallback   dataCb;
    void                 *connectCbData;
    uint16_t              scmsPort;
    char                  scmsIpAddr[INET6_ADDRSTRLEN];
    struct sockaddr_in6   tlsSockaddr; /* Must be large enough to hold
                                        * either struct sockaddr_in or
                                        * struct sockaddr_in6 */
    socklen_t             tlsSockaddrLen;
    pthread_t             recvThr;
    SSL_t                 Ssl;
    int                   tcpSocket;
    int16_t               connectAttempts;
    bool                  tcpConnected;
    bool                  tlsConnected;
    bool                  secureConnection;
} connUserInfo;

/* External functions */

/**
 * This is a routine to initialize SSL
 */
int32_t tlsInitSsl(connUserInfo *uci);

/**
 * Initializer for openSsl library, should only be done once.
 */
uint32_t tlsLibInit(void);

/**
 * This is a routine to check a server address and create a socket to connect
 * to it. This supports the OBS's need to verify the address right away and
 * tell it if it fails to verify correctly.
 */
int32_t libtlsCheckAddress(connUserInfo *ui, char const *address, uint16_t port, int32_t  *sockin);

/**
 * This is a routine to get connect()ed to a server.
 */
int32_t libtlsConnect(connUserInfo *ui);

/**
 * This is a routine to get dis-connect()ed from a server. initiatedByServer is
 * set to true if the disconnect was initiated by the server.
 */
void libtlsDisconnect(connUserInfo *ui, bool initiatedByServer);

/**
 * This is a routine to send a packet via TLS
 */
int32_t tlsSend(connUserInfo *ui, uint8_t const* const data, size_t len);

/**
 * This is a routine to send a packet via unsecured
 */
int32_t unsecuredSend(connUserInfo *ui, uint8_t const* const data, size_t len);

/**
 * The thread to receive packets via TLS
 */
void *tlsRecvThread(connUserInfo *ui);

/**
 * The thread to receive packets via unsecured
 */
void *unsecuredRecvThread(connUserInfo *ui);

#define MAX_LOG_LINE_LEN  256

extern char tlsLogLine1[]; /* Used by the main thread */
extern char tlsLogLine2[]; /* Used by the otap monitor thread */
extern char tlsLogLine3[]; /* Used by the receive threads */

extern pthread_mutex_t disconnectMutex; /* Used by the receive threads */

/**
 * This is a function to log a message to the TLS log file
 */
void tlsLog(const char *str);

/**
 * This is a function to log Tx or Rx packet to a log file
 */
void tlsLogTxRx(const uint8_t *data, int len, bool tx);

#define TLS_CONSOLE_LOG(fmt, args...) \
{ \
    if (TLS_CONSOLE_LOGGING) printf(fmt, ##args); \
}

#define TLS_LOG(bufNum, fmt, args...) \
if (TLS_API_LOGGING) { \
    snprintf(tlsLogLine##bufNum, MAX_LOG_LINE_LEN, fmt, ##args); \
    tlsLog(tlsLogLine##bufNum); \
}

#define TLS_LOG_TXRX(data, len, tx) \
{ \
    if (TLS_ADDITIONAL_LOGGING) tlsLogTxRx((data), (len), (tx)); \
}

#ifdef __cplusplus
}
#endif

#endif
