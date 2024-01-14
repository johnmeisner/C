/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: tlsconn.c                                                        */
/* Purpose: provide network handling API for any apps that need it            */
/*                                                                            */
/* Copyright (C) 2017 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2017-06-2x  DSJGREEN      Initial Release                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include "tlsImpl.h"
#include "v2v_general.h"
#include "aerolinkPKIConnection.h"
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

//************************* Include right libraries **************************//
#ifndef GCC_UNUSED_VAR
#define GCC_UNUSED_VAR __attribute__((unused))
#endif
#ifndef EOK
#define EOK 0
#endif
//************************* Include right libraries **************************//

#define SOCKET_ERROR     -1
#define TLSCONFFILE      "/rwflash/configs/tls.conf"
#define OTAP_LOGINTERVAL 30
#define TLS_CONF_MAX_BUF  150

/* Globals */

bool TLS_API_LOGGING        = false;
bool TLS_ADDITIONAL_LOGGING = false;
bool TLS_CONSOLE_LOGGING    = false;

// Configuration variables
uint8_t cfgIpVersion; // 4 or 6
bool    cfgTlsRootCertificateCheck;

// func defs
#ifdef OBU_IPV6
static void *otapMonitorThread(void);
static tlsNetworkType getNetworkAvail(void);
static void tlsStartOtapMonitorThread(void);
#endif
// network/connection info
tlsNetAvailInfo tlsNA[2]; // One for tlsEthernet and one for tlsRse
connUserInfo UCI[MaxConnections];
/* Since we support only one network connection type at a time, keep this
 * information here. This variable will need to be eliminated and a different
 * method of keeping track of network type will need to be determined if we
 * later support Ethernet and RSE (wireless) at the same time. */
static tlsNetworkType netType = tlsNetworkUnavail;
static char randstate[128];

static pthread_t otapMonitorThreadId;
static bool otapMonitorThreadRunning = false;

#define MAX_FILE_NAME_LEN 100
#define MAX_TIME_STR_LEN   32

static char tlsLogFileName[MAX_FILE_NAME_LEN];
char tlsLogLine1[MAX_LOG_LINE_LEN]; /* Used by the main thread */
char tlsLogLine2[MAX_LOG_LINE_LEN]; /* Used by the otap monitor thread */
char tlsLogLine3[MAX_LOG_LINE_LEN]; /* Used by the receive thread */
static bool tlsLogFileNameCreated = false;

static bool     naCallbackRegistered = false;
pthread_mutex_t disconnectMutex;

/**
 * This is a function to log a message to the TLS log file
 */
void tlsLog(const char *str)
{
    struct timeval tv;
    struct tm epochTime;
    char timeStr[MAX_TIME_STR_LEN];
    FILE *fd;

    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &epochTime);
    strftime(timeStr, MAX_TIME_STR_LEN, "%Y/%m/%d %H:%M:%S", &epochTime);

    if (!tlsLogFileNameCreated) {
        system("mkdir -p /mnt/rwflash/libaerolinkPKI_log");
        strftime(tlsLogFileName, MAX_FILE_NAME_LEN, "/mnt/rwflash/libaerolinkPKI_log/libaerolinkPKI_%Y%m%d_%H%M%S.log", &epochTime);
        fd = fopen(tlsLogFileName, "w");

        if (fd == NULL) {
            printf("LIBTLS %s(): Error opening TLS log file: %s\n", __func__, strerror(errno));
            return;
        }

        fprintf(fd, "%s.%06lu Log file created.\n", timeStr, tv.tv_usec);
        tlsLogFileNameCreated = true;
    }
    else {
        fd = fopen(tlsLogFileName, "a");

        if (fd == NULL) {
            printf("LIBTLS %s(): Error opening TLS log file: %s\n", __func__, strerror(errno));
            return;
        }
    }

    fprintf(fd, "%s.%06lu %s", timeStr, tv.tv_usec, str);
    fclose(fd);
}

/**
 * This is a function to log Tx or Rx packet to a log file
 */
void tlsLogTxRx(const uint8_t *data, int len, bool tx)
{
    struct timeval tv;
    struct tm epochTime;
    char timeStr[MAX_TIME_STR_LEN];
    char fileName[MAX_FILE_NAME_LEN];
    int fd;

    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &epochTime);
    strftime(timeStr, MAX_TIME_STR_LEN, "%Y%m%d_%H%M%S", &epochTime);
    snprintf(fileName, MAX_FILE_NAME_LEN, "/mnt/rwflash/libaerolinkPKI_log/libaerolinkPKI_%s_%06lu_%s.log", timeStr, tv.tv_usec, tx ? "TX" : "RX");

    if ((fd = open(fileName, O_CREAT | O_WRONLY)) == -1) {
        printf("LIBTLS %s(): Error opening Tx data file: %s\n", __func__, strerror(errno));
    }

    if (write(fd, data, len) == -1) {
        printf("LIBTLS %s(): Error writing Tx data file: %s\n", __func__, strerror(errno));
    }

    close(fd);
}

/**
 * Get the index into the UCI table from the connection handle.
 * Returns connection handle if found; -1 otherwise.
 */
static int getIndexFromCHandle(int32_t cHandle)
{
    int index = 0;

    /* Search through the UCI table for the connection handle */
    for (index = 0; index < MaxConnections; index++) {
        if (UCI[index].connectionHandle == cHandle) {
            return index;
        }
    }

    /* No connection handle match */
    return -1;
}

/**
 * Gets a bool value from the tls.conf file
 */
static void getBoolValue(void                   *configItem,
                         uint8_t                *val,
                         uint8_t GCC_UNUSED_VAR *min,
                         uint8_t GCC_UNUSED_VAR *max)
{
    bool *cfg = configItem;
    *cfg = (strtol((char *)val, NULL, 10) != 0);
}

/**
 * Gets a uint8_t value from the tls.conf file
 */
static void getUint8Value(void    *configItem, uint8_t *val,
                           uint8_t *min,        uint8_t *max)
{
    uint8_t *cfg = configItem;
    uint8_t value = strtoul((char *)val, NULL, 0);
    uint8_t minval = 0, maxval = 0;

    if (strcmp((char *)min, "Undefined") != 0) {
        minval = strtoul((char *)min, NULL, 0);
    }

    if (strcmp((char *)max, "Undefined") != 0) {
        maxval = strtoul((char *)max, NULL, 0);
    }

    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
    }
    else {
        if (value < minval) {
            *cfg = minval;
        } else if (value > maxval) {
            *cfg = maxval;
        }
    }
}

/**
 * Parses the tls.conf file
 * Returns -1 if error.
 */
static int8_t parseTlsConfFile(void)
{
    struct {
        char *tag;
        void (*updateFunction)(void *configItem, uint8_t *val,
                               uint8_t *min, uint8_t *max);
        void *itemToUpdate;
    } cfgItems[] = { {"IPVersion",               getUint8Value, &cfgIpVersion}, {"TLSRootCertificateCheck", getBoolValue,  &cfgTlsRootCertificateCheck},
    };

    char_t buf[150], min[100], max[100], dummy[TLS_CONF_MAX_BUF],*ptr;
    uint8_t val[100];
    int   i, j;
    FILE *f;

    if ((f = fopen("/rwflash/configs/tls.conf", "r")) == NULL) {
        perror("/rwflash/configs/tls.conf");
        return -1;
    }

    while (fgets(buf, 150, f)) {
        /* Identify and remove comment sections */
        if ((ptr = strrchr(buf, '#')) != NULL) {
            *ptr = '\0';
        }

#if defined(PORTME)
        /* Remove white space */
        while ((ptr = strpbrk(buf, " \t\n\r")) != NULL) {
            strcpy(ptr, ptr + 1);
        }
#else
        memset(dummy, '\0', sizeof(dummy));
        for(i=0,j=0;(i < TLS_CONF_MAX_BUF) && ('\0' != buf[i]); i++) {

            switch(buf[i]) {

                case ' ' :
                case '\t':
                case '\n':
                case '\r':

                    break;
                default:
                    dummy[j] = buf[i];
                    j++;
                    break;
            }

        }

        for(i=0;(i < TLS_CONF_MAX_BUF) && ('\0' != dummy[i]);i++) {
            buf[i] = dummy[i];
        }

        if(i < TLS_CONF_MAX_BUF)
            buf[i] = '\0';
        else
            buf[TLS_CONF_MAX_BUF-1] = '\0';
#endif

        /* Blank/Comment lines */
        if ((strlen(buf) == 0) || (buf[0] == '#')) {
            continue;
        }

        /* Identify value lines */
        if ((ptr = strrchr(buf, '=')) == NULL) {
            continue;
        }

        *ptr = '\0';
        strcpy((char_t *)val, ptr + 1);

        /* Read Range Minimum Value */
        if ((ptr = strrchr((char_t *)val, ';')) != NULL) {
            *ptr = '\0';
            strcpy(min, ptr + 1);

            /* Read Range Maximum Value */
            if ((ptr = strrchr(min, ',')) != NULL) {
                *ptr = '\0';
                strcpy(max, ptr + 1);
            }
            else {
                strcpy(min, "Undefined");
                strcpy(max, "Undefined");
            }
        }
        else {
            strcpy(min, "Undefined");
            strcpy(max, "Undefined");
        }

        /* Identify Configuration Item(s) */
        for (i = 0; i < (int)NUMITEMS(cfgItems); i++) {
            if (strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) {
                (*cfgItems[i].updateFunction)(cfgItems[i].itemToUpdate, val,
                                              (uint8_t *)min, (uint8_t *)max);
                break;
            }
        }
    }

    fclose(f);
    return 0;
}

/**
 * Initialize the TLS library.
 * Returns tlsNetworkUnavail on error; the network type (tlsEthernet or tlsRse)
 * otherwise.
 */
static tlsNetworkType libtlsOneTimeInitialization(void)
{
    int            i;
    tlsNetworkType netType;

    TLS_LOG(1, "%s()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s()\n", __func__);

    /* Get the configuration variables */
    if (parseTlsConfFile() == -1) {
        return tlsNetworkUnavail;
    }

    if ((cfgIpVersion != 4) && (cfgIpVersion != 6)) {
        return tlsNetworkUnavail;
    }
    
    tlsNA[tlsEthernet].networkCb      = NULL;
    tlsNA[tlsEthernet].registerCbData = NULL;
    tlsNA[tlsRse].networkCb           = NULL;
    tlsNA[tlsRse].registerCbData      = NULL;

    /* init this array of connection blocks */
    for (i = 0; i < MaxConnections; i++) {
        UCI[i].connectionHandle = UnusedBlock;
        UCI[i].connectCbData    = NULL;
        UCI[i].scmsPort         = 0;
        UCI[i].tcpSocket        = -1;
        UCI[i].tcpConnected     = false;
        UCI[i].tlsConnected     = false;
        UCI[i].secureConnection = false;
        UCI[i].recvThr          = (pthread_t)NULL;
        strcpy(UCI[i].scmsIpAddr, " ");
        UCI[i].Ssl.ipVersion    = (cfgIpVersion == 4) ? AF_INET : AF_INET6;
    }

    /* init the random number generator */
    initstate(time(NULL), randstate, sizeof(randstate));
    setstate(randstate);

    netType = (cfgIpVersion == 4) ? tlsEthernet : tlsRse;
    TLS_LOG(1, "%s() completes, return: %d\n", __func__, netType);
    TLS_CONSOLE_LOG("LIBTLS %s() completes, return: %d\n", __func__, netType);
    return netType;
}

/**
 * Lets all registered users know about a change with the "network available"
 * status.
 */
static void notifyUsers(tlsNetworkType netType, uint32_t state)
{
    bool datCb = (NetworkAvailable == state);

    TLS_LOG(2, "%s(): netType = %u; network state: %d\n", __func__, netType, datCb);
    TLS_CONSOLE_LOG("LIBTLS %s(): netType = %u; network state: %d\n", __func__, netType, datCb);

    if (tlsNA[netType].networkCb != NULL) {
        TLS_LOG(2, "%s(): networkAvailable callback called with state: %d\n", __func__, datCb);
        TLS_CONSOLE_LOG("LIBTLS %s(): networkAvailable callback called with state: %d\n", __func__, datCb);
        TLS_LOG(2, "%s() Calling callback (0p%p)(%d, 0p%p)\n", __func__, tlsNA[netType].networkCb, datCb, tlsNA[netType].registerCbData);
        (tlsNA[netType].networkCb)(datCb, tlsNA[netType].registerCbData);
    }
}

/* Returns the value of a boolean environment variable */
bool boolenEnvironmentVariable(char *name)
{
    char *value = getenv(name);

    if (value == NULL) {
        return false;
    }

    return ((*value != 'F') && (*value != 'f'));
}

/*
 * API to register the network callback function.
 * Note that registering the callback ends with calling the callback.
 */
void AerolinkPKI_registerNetworkAvailCallback(NetworkAvailCallback networkCallback, void *userCallbackData)
{
    static bool firstcall = true;

    if (naCallbackRegistered) {
        TLS_LOG(1, "%s(): Network available callback already registered!\n", __func__);
        return;
    }

    if (firstcall) {
        /* Need to set TLS_API_LOGGING and TLS_ADDITIONAL_LOGGING for the
         * TLS_LOG() and TLS_CONSOLE_LOG() calls below */
        TLS_API_LOGGING        = boolenEnvironmentVariable("TLS_API_LOGGING");
        TLS_ADDITIONAL_LOGGING = boolenEnvironmentVariable("TLS_ADDITIONAL_LOGGING");
        TLS_CONSOLE_LOGGING    = boolenEnvironmentVariable("TLS_CONSOLE_LOGGING");
    }

    if (networkCallback == NULL) {
        TLS_LOG(1, "%s: networkCallback is NULL!\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s: networkCallback is NULL!\n", __func__);
        return;
    }

    TLS_LOG(1, "%s(0p%p, 0p%p)\n", __func__, networkCallback, userCallbackData);
    TLS_CONSOLE_LOG("LIBTLS %s(0p%p, 0p%p)\n", __func__, networkCallback, userCallbackData);

    /* Initialize TLS. This needs to happen only once */
    if (firstcall) {
        /* This happens only once */
        netType   = libtlsOneTimeInitialization();
        firstcall = false;

        if (netType == tlsNetworkUnavail) {
            TLS_LOG(1, "%s: netType is tlsNetworkUnavail!\n", __func__);
            TLS_CONSOLE_LOG("LIBTLS %s: netType is tlsNetworkUnavail!\n", __func__);
            return;
        }

        TLS_LOG(1, "%s(): netType set to: %s\n",
                __func__, (netType == tlsEthernet) ? "tlsEthernet" : "tlsRse");
        TLS_CONSOLE_LOG("LIBTLS %s(): netType set to: %s\n",
                        __func__, (netType == tlsEthernet) ? "tlsEthernet" : "tlsRse");
    }

    /* Set the network available callback function and the user callback data
     * for both network types. Note that if in the future separate Ethernet and
     * RSE (wireless) network types are supported, then we would set only those
     * values for tlsNA[netType]. */
    tlsNA[tlsEthernet].networkCb      = networkCallback;
    tlsNA[tlsEthernet].registerCbData = userCallbackData;
    tlsNA[tlsRse].networkCb           = networkCallback;
    tlsNA[tlsRse].registerCbData      = userCallbackData;

    TLS_LOG(1, "%s: Network type configured for %s\n", __func__,
            (netType == tlsRse) ? "RSE" : "Ethernet");
    TLS_CONSOLE_LOG("LIBTLS %s: Network type configured for %s\n", __func__,
                    (netType == tlsRse) ? "RSE" : "Ethernet");

    if (netType == tlsRse) {
#ifdef OBU_IPV6
        /* start thread for ongoing monitor of network availability */
        tlsStartOtapMonitorThread();
#endif
    }
    else { // (netType == tlsEthernet)
        /* Tell users that the network is available */
        notifyUsers(tlsEthernet, NetworkAvailable);
    }

    pthread_mutex_init(&disconnectMutex, NULL);
    naCallbackRegistered = true;
}

/*
 * API to unregister the network callback function.
 */
void AerolinkPKI_unregisterNetworkAvailCallback(void)
{
    int i;

    if (!naCallbackRegistered) {
        TLS_LOG(1, "%s(): Network available callback not registered!\n", __func__);
        return;
    }

    for (i = 0; i < MaxConnections; i++) {
        if ((UCI[i].connectionHandle != UnusedBlock) && (UCI[i].recvThr != (pthread_t)NULL)) {
            TLS_LOG(1, "%s(): Disconnect from all servers first!\n", __func__);
            return;
        }
    }

    TLS_LOG(1, "%s()\n", __func__);
    //TLS_CONSOLE_LOG("LIBTLS %s(0p%p, 0p%p)\n", __func__);

    if (otapMonitorThreadRunning) {
        /* Kill the network available thread */
        TLS_LOG(1, "%s(): Canceling otapMonitorThread() pthread\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): Canceling otapMonitorThread() pthread\n", __func__);
        otapMonitorThreadRunning = false;
        pthread_join(otapMonitorThreadId, NULL);
        TLS_LOG(1, "%s(): otapMonitorThread() successfully shut down\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): otapMonitorThread() successfully shut down\n", __func__);
    }

    tlsNA[tlsEthernet].networkCb      = NULL;
    tlsNA[tlsEthernet].registerCbData = NULL;
    tlsNA[tlsRse].networkCb           = NULL;
    tlsNA[tlsRse].registerCbData      = NULL;
    pthread_mutex_destroy(&disconnectMutex);
    naCallbackRegistered              = false;
}

/*
 * Thread to establish connections or receive data from a servers. this is done
 * on a per socket basis.
 */
static int32_t tlsInitRecvThread(connUserInfo *UCI)
{
    pthread_attr_t attr;
    int32_t        old;
    int            result;

    TLS_LOG(1, "%s(): called to start thread for: %s on sock: %d \n", __func__, UCI->scmsIpAddr, UCI->tcpSocket);
    //TLS_CONSOLE_LOG("LIBTLS %s(): called to start thread for: %s on sock: %d \n", __func__, UCI->scmsIpAddr, UCI->tcpSocket);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);

    if (UCI->secureConnection) {
        result = pthread_create(&UCI->recvThr, &attr, (void*)&tlsRecvThread, (void *)UCI);
    }
    else {
        result = pthread_create(&UCI->recvThr, &attr, (void*)&unsecuredRecvThread, (void *)UCI);
    }

    if (result != EOK) {
        UCI->recvThr = 0;
        TLS_LOG(1, "%s(): pthread_create() failed\n", __func__);
        printf("LIBTLS %s(): pthread_create() failed\n", __func__);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    TLS_LOG(1, "%s(): pthread_create() succeeded 0x%x\n", __func__, (unsigned int) UCI->recvThr);
    TLS_CONSOLE_LOG("LIBTLS %s(): pthread_create() succeeded 0x%x\n", __func__, (unsigned int) UCI->recvThr);
    return AerolinkPKI_ERR_NONE;
}

/*
 * Starts a thread to monitor otap shared-mem for rse presence/absence.
 * Note that only one of these is requried for the system, not a per socket
 * instance.
 */
#ifdef OBU_IPV6
static void tlsStartOtapMonitorThread(void)
{
    pthread_attr_t attr;
    int32_t old;

    if (otapMonitorThreadRunning) {
        TLS_LOG(1, "%s(): otapMonitorThread() already running!\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): otapMonitorThread() already running!\n", __func__);
        return;
    }

    TLS_LOG(1, "%s(): creating otap monitor thread\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s(): creating otap monitor thread\n", __func__);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);
    otapMonitorThreadRunning = true;

    if (pthread_create(&otapMonitorThreadId, &attr, (void*)&otapMonitorThread, (void *)NULL) == EOK) {
        TLS_LOG(1, "%s(): otap pthread_create() succeeded\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): otap pthread_create() succeeded\n", __func__);
    }
    else {
        otapMonitorThreadRunning = false;
        TLS_LOG(1, "%s(): otap pthread_create() failed\n", __func__);
        TLS_CONSOLE_LOG("LIBTLS %s(): otap pthread_create() failed\n", __func__);
    }
}
#endif
/*
 * Code common to AerolinkPKI_connect() and AerolinkPKI_unsecure_connect().
 */
static int32_t AerolinkPKI_connect_common(char const *address, uint16_t port, uint8_t networkPreference, ReceiveDataCallback dataCallback, void *userCallbackData, bool secureConnection)
{
    static bool tlsLibInited = false;
    int         i;
    int         sock;
    int32_t     returnCode;

    TLS_LOG(1, "%s(\"%s\", %u, %u, 0p%p, 0p%p)\n", __func__, address, port, networkPreference, dataCallback, userCallbackData);
    //TLS_CONSOLE_LOG("LIBTLS %s(\"%s\", %u, %u, 0p%p, 0p%p)\n", __func__, address, port, networkPreference, dataCallback, userCallbackData);

    if (networkPreference != 0) {
        TLS_LOG(1, "%s: networkPreference must be 0!\n", __func__);
        //TLS_CONSOLE_LOG("LIBTLS %s: networkPreference must be 0!\n", __func__);
        return AerolinkPKI_ERR_TLS_INIT_FAILURE;
    }

    /* Look for a free connection entry */
    for (i = 0; i < MaxConnections; i++) {
        if ((UCI[i].connectionHandle == UnusedBlock) && (UCI[i].connectCbData == NULL)) {
            if (secureConnection) {
                // Call tlsLibInit() only once
                if (!tlsLibInited) {
                    if (SslInitialized != tlsLibInit()) {
                        TLS_LOG(1, "%s: tlsLibInit() failed, returning AerolinkPKI_ERR_TLS_INIT_FAILURE\n", __func__);
                        printf("LIBTLS Aerolink_connect(): tlsLibInit failed, returning AerolinkPKI_ERR_TLS_INIT_FAILURE\n");
                        return AerolinkPKI_ERR_TLS_INIT_FAILURE;
                    }

                    tlsLibInited = true;
                }

                tlsInitSsl(&UCI[i]);
            }

            if ((netType != tlsEthernet) && (netType != tlsRse)) {
                TLS_LOG(1, "%s: netType(%u) not tlsEthernet or tlsRse!\n", __func__, i);
                printf("LIBTLS %s(): netType(%u) not tlsEthernet or tlsRse!\n", __func__, i);
                return AerolinkPKI_ERR_HOST_UNREACHABLE;
            }

            // this ties our earlier determination of network type to our current need to use it
            UCI[i].Ssl.ipVersion = (netType == tlsEthernet) ? AF_INET : AF_INET6;
            returnCode = libtlsCheckAddress(&UCI[i], address, port, &sock);

            if (returnCode != AerolinkPKI_ERR_NONE) {
                TLS_LOG(1, "%s: libtlsCheckAddress() failed; error code = %d\n", __func__, returnCode);
                //printf("LIBTLS %s: libtlsCheckAddress() failed; error code = %d\n", __func__, returnCode);
                return AerolinkPKI_ERR_HOST_UNREACHABLE;
            }

            UCI[i].dataCb           = dataCallback;
            UCI[i].connectCbData    = userCallbackData;
            UCI[i].scmsPort         = port;
            UCI[i].tcpSocket        = sock;
            UCI[i].secureConnection = secureConnection; // boolean values passed in
            /* Be sure connection handle is not negative */
            UCI[i].connectionHandle = (random() & 0x7fffffff);

            returnCode = libtlsConnect(&UCI[i]);

            if (returnCode != AerolinkPKI_ERR_NONE) {
                TLS_LOG(1, "%s: libtlsConnect() unsuccssful; error code = %d\n", __func__, returnCode);
                printf("LIBTLS %s(): libtlsConnect() unsuccssful; error code = %d\n", __func__, returnCode);
                /* libtlsDisconnect() is called automatically by libtlsConnect()
                 * before it returns if connect fails */
                return returnCode;
            }

            returnCode = tlsInitRecvThread(&UCI[i]);

            if (returnCode != AerolinkPKI_ERR_NONE) {
                TLS_LOG(1, "%s: tlsInitRecvThread() unsuccessful; error code = %d\n", __func__, returnCode);
                printf("LIBTLS %s(): tlsInitRecvThread() unsuccessful; error code = %d\n", __func__, returnCode);
                libtlsDisconnect(&UCI[i], false);
                return returnCode;
            }

            break;
        }
    }

    if (i == MaxConnections) {
        TLS_LOG(1, "%s: Max TLS connections exceeded\n", __func__);
        //TLS_CONSOLE_LOG("LIBTLS %s: Max TLS connections exceeded\n", __func__);
        return AerolinkPKI_ERR_BAD_CONNECTION_HANDLE;
    }

    TLS_LOG(1, "%s: Returning cHandle: %d\n", __func__, UCI[i].connectionHandle);
    TLS_CONSOLE_LOG("LIBTLS %s(): done and returning: %d\n", __func__, UCI[i].connectionHandle);
    return UCI[i].connectionHandle;
}

/*
 * API to connect to the SCMS server.
 * Returns a connection handle on success, failure code (< 0) on failure.
 */
int32_t AerolinkPKI_connect(char const *address, uint16_t port, uint8_t networkPreference, ReceiveDataCallback dataCallback, void *userCallbackData)
{
    if (!naCallbackRegistered) {
        TLS_LOG(1, "%s(): Network available callback not registered!\n", __func__);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    return AerolinkPKI_connect_common(address, port, networkPreference, dataCallback, userCallbackData, true);
}

/*
 * API to connect unsecured to the SCMS server.
 * Returns a connection handle on success, failure code (< 0) on failure.
 */
int32_t AerolinkPKI_unsecure_connect(char const *address, uint16_t port, uint8_t networkPreference, ReceiveDataCallback dataCallback, void *userCallbackData)
{
    if (!naCallbackRegistered) {
        TLS_LOG(1, "%s(): Network available callback not registered!\n", __func__);
        return AerolinkPKI_ERR_HOST_UNREACHABLE;
    }

    return AerolinkPKI_connect_common(address, port, networkPreference, dataCallback, userCallbackData, false);
}

/*
 * API to send a packet to the SCMS server.
 * Returns the number of bytes sent on success, failure code (< 0) on failure.
 */
int32_t AerolinkPKI_send(int32_t cHandle, uint8_t const* const data, int32_t len)
{
    int     index;
    int32_t res   = 0;

    TLS_LOG(1, "%s(0x%x, 0p%p, %d)\n", __func__, cHandle, data, len);
    //TLS_CONSOLE_LOG("LIBTLS %s(0x%x, 0p%p, %d)\n", __func__, cHandle, data, len);

    index = getIndexFromCHandle(cHandle);

    if (index >= 0) {
        TLS_LOG(1, "%s(): using index: %d\n", __func__, index);
        TLS_CONSOLE_LOG("LIBTLS %s(): using index: %d\n", __func__, index);

        if ((index != -1) && (index < MaxConnections)) {
            TLS_LOG_TXRX(data, len, true);

            if (UCI[index].secureConnection) {
                res = tlsSend(&UCI[index], data, len);
                TLS_LOG(1, "%s: tlsSend(0p%p, 0p%p, %d) returned %d\n", __func__, &UCI[index], data, len, res);
                //TLS_CONSOLE_LOG("LIBTLS %s: tlsSend(0p%p, 0p%p, %d) returned %d\n", __func__, &UCI[index], data, len, res);
            }
            else {
                res = unsecuredSend(&UCI[index], data, len);
                TLS_LOG(1, "%s: unsecured(0p%p, 0p%p, %d) returned %d\n", __func__, &UCI[index], data, len, res);
                //TLS_CONSOLE_LOG("LIBTLS %s: unsecured(0p%p, 0p%p, %d) returned %d\n", __func__, &UCI[index], data, len, res);
            }

            return res;
        }
    }

    TLS_LOG(1, "%s: Can't find connection info for cHandle 0x%x\n", __func__, cHandle);
    //TLS_CONSOLE_LOG("LIBTLS %s(): can't find connection info for cHandle 0x%x, returning fail\n", __func__, cHandle);
    return AerolinkPKI_ERR_BAD_CONNECTION_HANDLE;
}

/*
 * API to close the connection to the SCMS server.
 */
void AerolinkPKI_close(int32_t cHandle)
{
    int index;

    if (!naCallbackRegistered) {
        TLS_LOG(1, "%s(): Network available callback not registered!\n", __func__);
        return;
    }

    TLS_LOG(1, "%s(0x%x)\n", __func__, cHandle);
    //TLS_CONSOLE_LOG("LIBTLS %s(0x%x)\n", __func__, cHandle);

    if (cHandle != NULL) {
        index = getIndexFromCHandle(cHandle);

        if (index >= 0) {
            TLS_LOG(1, "%s(): Using index: %d\n",__func__, index);
            TLS_CONSOLE_LOG("LIBTLS %s(): Using index: %d\n",__func__, index);
            /* Do a TLS disconnect from the server */
            libtlsDisconnect(&UCI[index], false);
        }
        else if (TLS_ADDITIONAL_LOGGING) {
            TLS_LOG(1, "%s(): Can't find connection info for cHandle 0x%x, fail\n", __func__, cHandle);
            //TLS_CONSOLE_LOG("LIBTLS %s(): can't find connection info for cHandle 0x%x, fail\n", __func__, cHandle);
        }
    }
}

/**
 * Get the "network available" status.
 *
 * Returns tlsNetworkUnavail if network is unavailable, tlsRse or tlsEthernet
 * otherwise.
 *
 * This function is only called from otapMonitorThread(), which is called only
 * if we are configured for IPv6.
 */
#ifdef OBU_IPV6
static tlsNetworkType getNetworkAvail(void)
{
    tlsNetworkType availNetwork = tlsNetworkUnavail;


    int i = 0;
    dIPv6sShmDataT rse;

    memset(&rse, 0, sizeof(rse));

    WSU_SEM_LOCKR(&otapShm->h.ch_lock);
    memcpy(&rse, &otapShm->dipv6Data, sizeof(dIPv6sShmDataT));
    WSU_SEM_UNLOCKR(&otapShm->h.ch_lock);

    for (i = 0; i < MAX_RSES; i++) {
        //TLS_CONSOLE_LOG("LIBTLS %s() rse.rseAvailable[i] = %u rse.rseConnectedTo[i] = %u i: %u\n", __func__, rse.rseAvailable[i], rse.rseConnectedTo[i], i);

        if ((rse.rseAvailable[i] == RSE_AVAILABLE) && rse.rseConnectedTo[i] == true) {
            availNetwork = tlsRse;
            //TLS_CONSOLE_LOG("LIBTLS %s(): otap and rse are available, set neworkavailable to rse\n", __func__);
            break;
        }
    }

    return availNetwork;
}
#endif
/**
 * Monitors otap to see if it is up. Prints a warning message every 30 seconds
 * that it is not.
 *
 * Otherwise, monitors for WSA/RSE changing status information.
 *
 * Question: if there are no registered users, should this thread exit? That
 *           would not be too hard to implement. Have notifyUsers return a
 *           count of those notified and if it is zero, have this exit. Just be
 *           sure  to restart it with the next
 *           AerolinkPKI_registerNetworkAvailCallback request() (which will
 *           need to change a little).
 */
#ifdef OBU_IPV6
static void *otapMonitorThread(void)
{
    tlsNetworkType  avail        = tlsNetworkUnavail;
    uint32_t        currentState = NetworkNotAvailable;
    static uint32_t otapState    = ~OTAP_ALIVE;
    static int      otapTimer    = 0;
    int             i;

    TLS_LOG(2, "%s: Starting otapMonitorThread(); mounting otap shared memory\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Starting otapMonitorThread(); mounting otap shared memory\n", __func__);

    /* Mount otap shared memory */
    otapShm = wsu_share_init(sizeof(otapShmDataT), OTAP_SHM_PATH);

    if (otapShm == NULL) {
        TLS_LOG(2, "%s: OTAP shared memory init fail (\"%s\")\n", __func__, OTAP_SHM_PATH);
        //TLS_CONSOLE_LOG("LIBTLS %s: OTAP shared memory init fail (\"%s\")\n", __func__, OTAP_SHM_PATH);
        goto otapMonitorThreadExit;
    }

    TLS_LOG(2, "%s: Mounting of otap shared memory succeeded\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Mounting of otap shared memory succeeded\n", __func__);

    /* Set otapShm->otapRunning to something other than OTAP_ALIVE so that we
     * can see when otap comes to life when it sets otapShm->otapRunning to
     * OTAP_ALIVE. This is necessary because the TLS library can be invoked
     * before prm is run. Do not surround this with WSU_SEM_LOCK() or
     * WSU_SEM_UNLOCK, because if otap has not initialized otapShm->otapRunning
     * to OTAP_ALIVE, it may not have initialized the semaphore either. */
    otapShm->otapRunning = ~OTAP_ALIVE;

    while (otapMonitorThreadRunning) {
        if (otapState != OTAP_ALIVE) {
            /* Wait for otap to start running */
            TLS_LOG(2, "%s(): looping until a otap is available, currentState: %d\n", __func__, otapState);
            //TLS_CONSOLE_LOG("LIBTLS %s(): looping until a otap is available, currentState: %d\n", __func__, otapState);

            if (otapShm != NULL) {
                /* See if otap is running. Do not surround this with
                 * WSU_SEM_LOCK() or WSU_SEM_UNLOCK, because if otap has not
                 * initialized otapShm->otapRunning to OTAP_ALIVE, it may not
                 * have initialized otapShm->h.ch_lock either. */
                if (otapShm->otapRunning == OTAP_ALIVE) {
                    /* otap is now running */
                    TLS_LOG(2, "%s(): OTAP is alive\n",__func__);
                    TLS_CONSOLE_LOG("LIBTLS %s(): OTAP is alive\n",__func__);
                    otapState = OTAP_ALIVE;
                }
                else {
                    /* otap is still not running, print a message occasionally */
                    if (++otapTimer == OTAP_LOGINTERVAL) {
                        TLS_LOG(2, "%s: OTAP has not shown-up alive (otapRunning = %u). Is it enabled in prm.conf?\n", __func__, otapShm->otapRunning);
                        //TLS_CONSOLE_LOG("LIBTLS %s: OTAP (addr: 0p%p) has not shown-up alive (contents: 0x%x). Is it enabled in prm.conf?\n", __func__, &otapShm->otapRunning, otapShm->otapRunning);
                        otapTimer = 0;
                    }
                }
            }

            /* No hurry */
            sleep(1);
        }
        else {
            /* otap is up and running; check its shared memory to see if the
             * OBE has connected to the RSE */
            /* We only want to notify users when the availablity changes */
            avail = getNetworkAvail();

            /* Note: users only get notified when state changes */
            if ((tlsRse == avail) && (currentState == NetworkNotAvailable)) {
                TLS_LOG(2, "%s(): network now available\n",__func__);
                TLS_CONSOLE_LOG("LIBTLS %s(): network now available\n",__func__);
                notifyUsers(tlsRse, NetworkAvailable);
                currentState = NetworkAvailable;
            }
            else if ((tlsRse != avail) && (currentState == NetworkAvailable)) {
                TLS_LOG(2, "%s(): network no longer available\n",__func__);
                TLS_CONSOLE_LOG("LIBTLS %s(): network no longer available\n",__func__);

                /* Disconnect all TLS connections */
                for (i = 0; i < MaxConnections; i++) {
                    if (UCI[i].tlsConnected) {
                        /* Report the connection closed to Aerolink */
                        TLS_LOG(2, "%s(): (UCI[i].dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, %p)\n",__func__, UCI[i].connectCbData);
                        (UCI[i].dataCb)(AerolinkPKI_ERR_CONNECTION_CLOSED, NULL, 0, UCI[i].connectCbData);
                        /* Close the connection to the server */
                        libtlsDisconnect(&UCI[i], false);
                        notifyUsers(tlsRse, NetworkNotAvailable);
                    }

                }

                currentState = NetworkNotAvailable;
            }

            /* simple pause is all that is necessary  */
            usleep(400000);
        }
    }

    /* Unmount otap shared memory */
    TLS_LOG(2, "%s: Unmounting otap shared memory\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Unmounting otap shared memory\n", __func__);
    wsu_share_kill(otapShm, sizeof(otapShmDataT));

otapMonitorThreadExit:
    TLS_LOG(2, "%s: Terminating otapMonitorThread()\n", __func__);
    TLS_CONSOLE_LOG("LIBTLS %s: Terminating otapMonitorThread()\n", __func__);
    otapMonitorThreadRunning = false;
    pthread_exit(NULL);
}
#endif

#ifdef __cplusplus
}
#endif

