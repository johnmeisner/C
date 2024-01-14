/**************************************************************************
 *                                                                        *
 *     File Name:  ipb.c  (ipbridge)                                      *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Drive                                               *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/* 20180413: TODO: all debug messages should have a time stamp */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_riscapture.h"
#include "conf_table.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
#else
#define MAIN main
#define dn_setsockopt     setsockopt
#define dn_socket         socket
#define dn_bind           bind
#define dn_pthread_create pthread_create
#define dn_pthread_detach pthread_detach

#define dn_select    select
#define dn_recvfrom  recvfrom
#define dn_inet_aton inet_aton
#define dn_inet_ntoa inet_ntoa

#define DN_FD_ZERO  FD_ZERO
#define DN_FD_SET   FD_SET
#define DN_FD_CLR   FD_CLR
#define DN_FD_ISSET FD_ISSET
#endif

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "ipb"

//#define EXTRA_DEBUG

#define IPB_CFG_FILE "ipb.conf"

#ifdef WSU_5001
#define WIRELESS_DEV_NAME "sch"
#else
#define WIRELESS_DEV_NAME "ath"
#endif /* WSU_5001 */

/* JJG: TODO for the build, this value needs to be determined
   based on the platform; i.e. for 5910 this is sc0, but
   on 5900 this is fec0
*/
#define ETH_IFC_V2X  "eth0"

/* for reference only:
 * From : srv/radio/common/llc_defs.h
 * #define MKX_UNIT_0 0
 * #define MKX_UNIT_0_DEV_NAME "cw-llc00"
 * #define MKX_UNIT_1_DEV_NAME "cw-llc10"
 */

#define LLCIFCNAME "llc-sch-ipv6"

/* 8 bytes (4 words) reserved for prefix in IPv6 */
#define IPV6_PREFIX_WORDS  4
/* a little extra than needed (32 chars + 7 colons + '0') */
#define MAX_IPV6_STRING_LENGTH  48
/* 20180327: updating max to 1/2 max TCP; full max not required */
#define MAX_TCP_DATA        32768

/* JJG 20180124 - V3 WSAs are actually WSMs and have a special PSID
   for security initialization purpose 0x8007; perhaps consider making this
   configurable; but for now hardcoding since it's not expected to change */
#define IPB_V3_SECPSID    0x8007

#define MAIN_CLEANUP()  \
    svctrack.u.providerData.action = DELETE; \
    I2V_UTIL_PROVIDER_SVC_REQ(&svctrack.u.providerData, &cfg, WMH_SVC_TIMEOUT, &svctrack.sdummy, &svctrack.udummy) \
    if (svctrack.sdummy != RS_SUCCESS) { \
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"deleting provider service failed (%d); radio may not restart correctly without reboot\n", svctrack.sdummy); \
    } \
    if (!cfg.iwmhAppEnable) { \
        svctrack.wsmData.action = DELETE; \
        if ((ret = wsuRisWsmServiceRequest(&svctrack.wsmData)) != RS_SUCCESS) { \
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmServiceRequest cleanup failed (%d), radio may not restart correctly without reboot\n", ret); \
        } \
        if ((ret = wsuRisTerminateRadioComm()) != RS_SUCCESS) { \
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminateRadioComm failed(%d)\n", ret); \
        } \
    } \
    if (ipbCfg.delayRcvr) { \
        wsu_kill_gate(&listenGate); \
    }

#define SVC_CLEANUP() \
            for (j=0; j<i; j++) { \
                wsu_kill_gate(ipv6ThrTracker[j].thrGate); \
                free(ipv6ThrTracker[i].pid); \
                free(ipv6ThrTracker[i].thrGate); \
                free(ipv6ThrTracker[i].thrLock); \
                free(ipv6ThrTracker[i].clientName); \
            }

#define GET_LOCK     while (0!=pthread_mutex_lock(&gpslock)) usleep(500);
#define PUT_LOCK     pthread_mutex_unlock(&gpslock);

/* no sense in having this block of memory allocated permanently -
   but also no sense in defining it in two places - thus this macro
   is for the provider service that ipb uses for registration */
/* psc length removes byte for null */
/* the 1609.3 requirement for service priority is 31 - however
   rsu spec has a snmp OID that can modify the priority - when it
   is modified both ipb.conf and i2v.conf are modified and the 
   bcastLockStep uwsaPriority is always 1 less than snmp value */
#define WSA_3DSETTING WSA_EXT_3DLOCATION_BITMASK 
#define DECLARE_PROVIDER_SERVICE    \
    ProviderServiceType pvService = {    \
        .action = ADD,    \
        .destMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},    \
        .wsaSecurity = UNSECURED_WSA,    \
        .psid = ipbPsid,    \
        .radioType = (cfg.RadioType) ? RT_CV2X : RT_DSRC,    \
        /* highest priority - this must be higher than all other apps to have correctly formatted WSA in lockstep mode */ \
        .servicePriority = (cfg.bcastLockStep) ? cfg.uwsaPriority + 1 : ipbCfg.wsaPriority,    \
        .channelNumber = ipbCfg.channelNumber,    \
        .edca_count = 4,    \
        .txop = {0, 0, 0, 0},    \
        .ecw = {0xa4, 0xa4, 0x43, 0x32},    \
        .aifs = {0x06, 0x29, 0x43, 0x62},    \
        .channelAccess = (ipbCfg.isContinuous) ? 0 : 2,    \
        .repeatRate = ipbCfg.wsaTxRate,    \
        .ipService = WTRUE,    \
        .IPv6ServicePort = ipbCfg.port,    \
        .rcpiThreshold = 0,    \
        .wsaCountThreshold = 0,    \
        .wsaCountThresholdInterval = 0,    \
        .wsaHeaderExtension = WSA_EXT_WRA_BITMASK |    \
                              WSA_3DSETTING |    \
                              WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |    \
                              WSA_EXT_ADVERTISER_ID_BITMASK |    \
                              WSA_EXT_IPV6ADDRESS_BITMASK |    \
                              WSA_EXT_SERVICE_PORT_BITMASK |    \
                              WSA_EXT_PROVIDER_MACADDRESS_BITMASK | \
                              WSA_EXT_SECONDARY_DNS_BITMASK | \
                              WSA_EXT_GATEWAY_MACADDRESS_BITMASK |    \
                              WSA_EXT_CHANNEL_ACCESS_BITMASK,    \
        .signatureLifeTime = 1000,    \
        .routerLifeTime = 65535,    \
        /* .prefixLen = IPV6_PREFIX_WORDS * 2, */    \
        .prefixLen = 64,    \
        /* 1609.3 requirement */ \
        .latitude2D = ipbCfg.latitude,    \
        .longitude2D = ipbCfg.longitude,    \
        .latitude3D = ipbCfg.latitude,    \
        .longitude3D = ipbCfg.longitude,    \
        .elevation3D = ipbCfg.elevation * 10,    \
        .confidence3D = ipbCfg.confidence,    \
        .semiMajorAccuracy3D = 0xFF,    \
        .semiMinorAccuracy3D = 0xFF,    \
        .semiMajorOrientation3D = 0xFFFF,    \
    };

#define SET_PROVIDER_SERVICE_BASICS \
    if (cfg.security) { \
        pvService.wsaSecurity = SECURED_WSA; \
    } \
    memcpy(pvService.providerMACAddress, ipbCfg.svcMac, LENGTH_MAC); \
    memcpy(pvService.ipPrefix, ipbCfg.ipv6addr, (IPV6_PREFIX_WORDS * 2)); \
    memcpy(pvService.primaryDns, ipbCfg.ipv6addr, LENGTH_IPV6); \
    memcpy(pvService.secondaryDns, ipbCfg.ipv6addr, LENGTH_IPV6); \
    memcpy(pvService.defaultGateway, ipbCfg.ipv6addr, LENGTH_IPV6); \
    memcpy(pvService.IPv6Address, ipbCfg.srvrIpv6addr, LENGTH_IPV6); \
    memcpy(pvService.gatewayMacAddress, gwBytes, LENGTH_MAC); \
    pvService.lengthPsc = strlen(ipbCfg.pscval); \
    pvService.lengthPsc = (pvService.lengthPsc >= LENGTH_IPB_PSC) ? LENGTH_IPB_PSC - 1 : pvService.lengthPsc; \
    strncpy((char_t *)pvService.psc, ipbCfg.pscval, pvService.lengthPsc); \
    if (ipbCfg.enableEDCA) { \
        pvService.wsaHeaderExtension |= WSA_EXT_EDCA_PARAM_SET_BITMASK; \
    }

#if defined(WSA_CATCHER)

/* So we can automatically pick up each others IP & MAC for p2p */
#define SET_PROVIDER_SERVICE_POINT_TO_POINT \
    if (cfg.security) { \
        pvService.wsaSecurity = SECURED_WSA; \
    } \
    memcpy(pvService.IPv6Address, ipbCfg.ipv6addr, LENGTH_IPV6);

#endif

/* lock functions expect pointer to lock */
#define SVC_GET_LOCK(a) \
    while (0 != pthread_mutex_lock(a)) usleep(500)

#define SVC_PUT_LOCK(a) \
    while (0 != pthread_mutex_unlock(a)) usleep(500)

/* this value is the actual limit for the config */
#define MAX_SUPPORTED_SVC_THREADS    20

typedef enum {
    GPS_INIT,
    GPS_LOADING,
    GPS_RELOAD,
    GPS_COMPLETE,
} ipbGPSStatus;

typedef struct clist_t {
    char_t          client[MAX_IPV6_STRING_LENGTH];
    struct clist_t *next;
} clientList;

/* this struct is created with pointers to conserve memory;
   the supported number of threads may vary, so no need to allocate
   extra bytes if threads aren't used -- having the array makes
   it easier to index, otherwise I would have done the usual
   malloc/free for records; should be obvious but the clientsock
   is actually allocated because its the same length as a pointer
*/
typedef struct {
    pthread_t       *pid;
    wsu_gate_t      *thrGate;
    pthread_mutex_t *thrLock;
    int32_t          clientSock;
    char_t          *clientName;
} ipv6ThrStruct;

static ipv6ThrStruct ipv6ThrTracker[MAX_SUPPORTED_SVC_THREADS];
static uint8_t ipv6ClientCnt = 0;
static pthread_mutex_t clientcntlock = PTHREAD_MUTEX_INITIALIZER;

STATIC i2vShmMasterT *shmPtr = NULL;
STATIC bool_t mainloop = WTRUE;
static PSIDType ipbPsid;
#if 0
/* this is here just in case a send is required in the future */
/* if ever using WSMs (I can't imagine why), set txPwrLevel to cfg.txPwrLevel
   once the cfg has been loaded */
static outWSMType outWSMMsg = {
    .dataRate = DR_6_MBPS,
    .txPwrLevel = 20,
    .txPriority = 2,
    .wsmpExpireTime = 0,
    .radioNum = 0,
    .channelInterval = 0,
    .security = SECURITY_UNSECURED,
    .securityFlag = 0,
    .peerMacAddress = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .wsmpHeaderExt = 0,

};
#endif
static uint8_t radioId;
STATIC cfgItemsT cfg;
STATIC ipbCfgItemsT ipbCfg;
STATIC char_t confFileName[I2V_CFG_MAX_STR_LEN];
static uint8_t ipbuf[MAX_TCP_DATA];
static wsu_gate_t listenGate;
/* default mac set below - overwritten during start up */
static uint8_t gwBytes[LENGTH_MAC] = {0xF0, 0xA1, 0xE2, 0xB3, 0xD4, 0xC5};
static uint8_t linklocal[LENGTH_IPV6] = {0};
/* for security */
static i2vTpsTypeT tpsUserData;
/* for gps */
static int32_t numsats = 0;
static timer_t gpstimer;
static pthread_mutex_t gpslock = PTHREAD_MUTEX_INITIALIZER;  /* probably overkill but there are 2 threads */
static ipbGPSStatus gpsstatus = GPS_INIT;

/* see comments in i2v_riscapture.h */
static i2vRisStruct svctrack;

/* function protos */
static bool_t bridgeInitPassThrough(int32_t *sock);
static bool_t bridgeTo4Server(int32_t *sock);
static bool_t bridgeTo6Server(int32_t *sock);
static bool_t bridgeXchg(bool_t direction, int32_t *client, int32_t *server);

static void ipbInitStatics(void)
{
  memset(ipv6ThrTracker,0x0,sizeof(ipv6ThrTracker));
  ipv6ClientCnt = 0;
  memset(&clientcntlock,0x0,sizeof(clientcntlock));
  shmPtr = NULL;
  mainloop = WTRUE;
  memset(&ipbPsid,0x0,sizeof(ipbPsid));
  radioId = 0;
  memset(&cfg,0x0,sizeof(cfg));
  memset(&ipbCfg,0x0,sizeof(ipbCfg));
  memset(confFileName,0x0,sizeof(confFileName));
  memset(ipbuf,0x0,sizeof(ipbuf));
  memset(&listenGate,0x0,sizeof(listenGate));
  memset(gwBytes,0x0,sizeof(gwBytes));
  memset(linklocal,0x0,sizeof(linklocal));
  memset(&tpsUserData,0x0,sizeof(tpsUserData));
  numsats = 0;
  memset(&gpstimer,0x0,sizeof(gpstimer));
  memset(&gpslock,0x0,sizeof(gpslock));
  gpsstatus = GPS_INIT;
  memset(&svctrack,0x0,sizeof(svctrack));
}

#if 0
/* security related functions */
static int32_t ipbGPSLocation(float64_t *latitude, float64_t *longitude, float64_t *elevation)
{
    GETGPSLOCATION(latitude, longitude, elevation);
}
#endif
static void ipbTPSCallback(tpsDataType *data)
{
    TPSCALLBACK(data);
    /* ipb ONLY */
    if (data->valid) {
        GET_LOCK
        numsats = data->numsats;
        PUT_LOCK
    }
}

static i2vReturnTypesT ipbTPSSetup(void)
{
    int32_t intret;
    TPSSETUP(ipbTPSCallback);
}
/* end security related functions */

static void ipbSigHandler(int32_t __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

/* RSU v4 functional requirement: log critical entry if min satellites
   not in use
   added function: reload provider service request to update
   wsa lat/long 
*/
static void ipbGPSTimerFxn(union sigval __attribute__((unused)) sig)
{
    struct itimerspec nv;
    int32_t ret;
    uint32_t optret = 0;
    DECLARE_PROVIDER_SERVICE
    if ((optret = strlen(ipbCfg.advertiseID)) < WSA_ADVERTISE_ID_LEN) {
        memcpy(pvService.advertiseId, ipbCfg.advertiseID, optret);
        pvService.advertiseIdLength = optret;
    } else {
        memcpy(pvService.advertiseId, ipbCfg.advertiseID, WSA_ADVERTISE_ID_LEN);
        pvService.advertiseIdLength = WSA_ADVERTISE_ID_LEN;
    }
    optret = 0;

    if (gpsstatus == GPS_INIT) {
        GET_LOCK
        if (numsats < ipbCfg.minSats) {
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"minimum number of satellites not acquired\n");
        } else {
            I2V_DBG_LOG(LEVEL_DBG,MY_NAME,"minimum number of satellites acquired: %d\n", numsats);
        }
        PUT_LOCK
        gpsstatus = GPS_LOADING;  /* can stay at this value if !usegps */
        if (!ipbCfg.usegps) {
            return;
        }
    }

    /* reload service */
    if (ipbCfg.usegps) {
        if (gpsstatus == GPS_LOADING) {
            gpsstatus = GPS_RELOAD;
            if (ipbCfg.gpstime > ipbCfg.sattime) {
                /* reset timer and exit */
                memset(&nv, 0, sizeof(nv));
                nv.it_value.tv_sec = (ipbCfg.gpstime - ipbCfg.sattime);
                if (0 != timer_settime(gpstimer, 0, &nv, NULL)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to reload GPS position for WSA. errno=%d.\n",errno);
                    gpsstatus = GPS_COMPLETE;   /* will not retry */
                }
                return;
            }
        }

        if (gpsstatus == GPS_RELOAD) {
            if (!tpsUserData.valid) {
                /* retry? */
                gpsstatus = GPS_COMPLETE;
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"tps data invalid - not retrying %f\n", ipbCfg.longitude);
                return;
            }
            gpsstatus = GPS_COMPLETE;
            /* reload service data */
            SET_PROVIDER_SERVICE_BASICS

#if defined(WSA_CATCHER)
            if (0 == strncmp(ipbCfg.pscval,"DP2P",LENGTH_IPB_PSC) ) {  
              SET_PROVIDER_SERVICE_POINT_TO_POINT 
            }
#endif

            /* if doing bridging, service mac must match rse mac */
            if (ipbCfg.enableBridge) {
                memcpy(pvService.providerMACAddress, pvService.gatewayMacAddress, LENGTH_MAC);
            }
            /* reset radio */
            pvService.radioNum = ipbCfg.radioNum;
            /* update GPS data */
            /* 2D location not included in WSA - values kept just in case (future) - 3D was only item used in interop testing */
            pvService.latitude2D = pvService.latitude3D = (int)(tpsUserData.latitude * 1e7);
            pvService.longitude2D = pvService.longitude3D = (int)(tpsUserData.longitude * 1e7);
            pvService.elevation3D = (int16_t)(tpsUserData.altitude * 10);
            pvService.action = DELETE;  /* to remove old service */
            I2V_UTIL_PROVIDER_SVC_REQ(&pvService, &cfg, WMH_SVC_TIMEOUT, &ret, &optret)
            if (ret != RS_SUCCESS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to refresh service registration (%x %x) - cannot reload GPS position for WSA\n", ret, optret);
                return;
            }
            pvService.action = ADD;   /* add new service with updated GPS */
            I2V_UTIL_PROVIDER_SVC_REQ(&pvService, &cfg, WMH_SVC_TIMEOUT, &ret, &optret)
            if (ret != RS_SUCCESS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to restore service registration (%x %x) - IP gateway service WSA disabled\n", ret, optret);
            } else {
                I2V_DBG_LOG(LEVEL_DBG,MY_NAME, "reloaded IP gateway service WSA with GPS lat/long\n");
            }
        }
    }
}

static void ipbEnableGPS(void)
{
    struct itimerspec nv;
    struct sigevent sige;

    memset(&nv, 0, sizeof(nv));
    memset(&sige, 0, sizeof(sige));

    sige.sigev_notify = SIGEV_THREAD;
    sige.sigev_notify_function = ipbGPSTimerFxn;
    if (0 != timer_create(CLOCK_REALTIME, &sige, &gpstimer)) {
        /* will not fail for this */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create gps timer. errno=%d.\n",errno);
        return;
    }

    if (ipbCfg.sattime) {   /* protect against 0 just in case */
        nv.it_value.tv_sec = ipbCfg.sattime;
        if (0 != timer_settime(gpstimer, 0, &nv, NULL)) {
            /* will not fail for this */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to initial load GPS timer\n");
        }
    }
}

/* 20180413: this thread is duplicated for up to the max
   number of clients; the thread is created at start up
   rather than during runtime because I'm paranoid about
   problems seen in the past (legacy platforms) where
   terminating a child thread during runtime killed i2v
*/
static void *ipbridgeSvcThr(void *arg)
{
    /* input arg is the thread data struct that 'this' thread uses (ipv6ThrStruct) */
    int32_t csock = -1;  /* client - ipv6 network */
    int32_t tsock = -1;  /* target network */
    int32_t ret, fds = 0;
    fd_set bridgeFds;
    struct timeval timeout;
    struct sigaction sa;
    char_t cname[MAX_IPV6_STRING_LENGTH] = {0};
    i2vTimeT clock;
    ipv6ThrStruct *mydata = (ipv6ThrStruct *)arg;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, 0) == -1) {
        /* JJG: consider using the cfg option for degraded operation to decide whether to continue or fail */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: WARNING: broken pipes will terminate i2v. errno=%d.\n",errno);
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,"ipbridgeSvcThr","Entering mainloop.\n");
#endif
    while (mainloop) {
        wsu_wait_at_gate(mydata->thrGate);

        SVC_GET_LOCK(mydata->thrLock);
        csock = mydata->clientSock;
        strcpy(cname, mydata->clientName);
        SVC_PUT_LOCK(mydata->thrLock);

        if (csock < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: ipv6 service thread called with invalid client sock\n");
            continue;    /* no need to move forward */
        }

        i2vUtilGetUTCTime(&clock);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d/%02d/%02d-%02d:%02d:%02d,%s,STARTED\n", clock.year, clock.month, clock.day,
                      clock.hour, clock.min, clock.sec, cname);

        if (ipbCfg.ipv4bridge) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: bridging to ipv4 server (%d)\n", i2vUtilGetUTCTimeInSec());
            if (!ipbCfg.ipbBridgeOnce) {
                if (!bridgeTo4Server(&tsock)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: couldn't connect to IPv4 server - client %s\n", cname);
                    close(csock);
                    SVC_GET_LOCK(mydata->thrLock);
                    mydata->clientSock = -1;
                    SVC_PUT_LOCK(mydata->thrLock);
                    SVC_GET_LOCK(&clientcntlock);
                    if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                    SVC_PUT_LOCK(&clientcntlock);
                    csock = -1;  /* this is separate from the assignment above to save processing time with lock */
                    if (ipbCfg.delayRcvr) {
                        wsu_open_gate(&listenGate);
                    }
                    continue;
                }
            }
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: bridging to ipv6 server\n");
            if (!bridgeTo6Server(&tsock)) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: couldn't connect to IPv6 server - client %s\n", cname);
                close(csock);
                SVC_GET_LOCK(mydata->thrLock);
                mydata->clientSock = -1;
                SVC_PUT_LOCK(mydata->thrLock);
                SVC_GET_LOCK(&clientcntlock);
                if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                SVC_PUT_LOCK(&clientcntlock);
                csock = -1;  /* this is separate from the assignment above to save processing time with lock */
                if (ipbCfg.delayRcvr) {
                    wsu_open_gate(&listenGate);
                }
                continue;
            }
        }

        while (mainloop) {
            FD_ZERO(&bridgeFds);
            FD_SET(csock, &bridgeFds);
            FD_SET(tsock, &bridgeFds);
            fds = (csock > tsock) ? csock : tsock;

            timeout.tv_sec = ipbCfg.cnxnTOSec;
            timeout.tv_usec = 0;

            if ((ret = select(fds + 1, &bridgeFds, NULL, NULL, &timeout)) < 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: error listening for exchange between client (%s) and server %d, errno=%d.\n", cname, i2vUtilGetUTCTimeInSec(),errno);
                close(csock); csock = -1;
                SVC_GET_LOCK(mydata->thrLock);
                mydata->clientSock = -1;
                SVC_PUT_LOCK(mydata->thrLock);
                SVC_GET_LOCK(&clientcntlock);
                if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                SVC_PUT_LOCK(&clientcntlock);
                if (!ipbCfg.ipbBridgeOnce) {
                    close(tsock); tsock = -1;
                }
                if (ipbCfg.delayRcvr) {
                    wsu_open_gate(&listenGate);
                }
                break;    /* inner while */
            } else if (!ret) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: timeout listening for exchange between client (%s) and server %d\n", cname, i2vUtilGetUTCTimeInSec());
                close(csock); csock = -1;
                SVC_GET_LOCK(mydata->thrLock);
                mydata->clientSock = -1;
                SVC_PUT_LOCK(mydata->thrLock);
                SVC_GET_LOCK(&clientcntlock);
                if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                SVC_PUT_LOCK(&clientcntlock);
                if (!ipbCfg.ipbBridgeOnce) {
                    close(tsock); tsock = -1;
                }
                if (ipbCfg.delayRcvr) {
                    wsu_open_gate(&listenGate);
                }
                break;    /* inner while */
            }  /* have valid data to process */
            if ((FD_ISSET(tsock, &bridgeFds)) && (!bridgeXchg(WTRUE, &csock, &tsock))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: exchange from server to client (%s) ended %d, errno=%d.\n", cname, i2vUtilGetUTCTimeInSec(),errno);
                close(csock); csock = -1;
                SVC_GET_LOCK(mydata->thrLock);
                mydata->clientSock = -1;
                SVC_PUT_LOCK(mydata->thrLock);
                SVC_GET_LOCK(&clientcntlock);
                if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                SVC_PUT_LOCK(&clientcntlock);
                if (!ipbCfg.ipbBridgeOnce) {
                    close(tsock); tsock = -1;
                }
                if (ipbCfg.delayRcvr) {
                    wsu_open_gate(&listenGate);
                }
                break;    /* inner while */
            }
            if ((FD_ISSET(csock, &bridgeFds)) && (!bridgeXchg(WFALSE, &csock, &tsock))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: exchange from client (%s) to server ended %d\n", cname, i2vUtilGetUTCTimeInSec());
                close(csock); csock = -1;
                SVC_GET_LOCK(mydata->thrLock);
                mydata->clientSock = -1;
                SVC_PUT_LOCK(mydata->thrLock);
                SVC_GET_LOCK(&clientcntlock);
                if (ipv6ClientCnt > 0) ipv6ClientCnt--;
                SVC_PUT_LOCK(&clientcntlock);
                if (!ipbCfg.ipbBridgeOnce) {
                    close(tsock); tsock = -1;
                }
                if (ipbCfg.delayRcvr) {
                    wsu_open_gate(&listenGate);
                }
                break;    /* inner while */
            }
        }
        i2vUtilGetUTCTime(&clock);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d/%02d/%02d-%02d:%02d:%02d,%s,ENDED\n", clock.year, clock.month, clock.day,
                      clock.hour, clock.min, clock.sec, cname);
    }

    if (csock > 0) close(csock);   /* not setting mydata clientsock since program terminating */
    if (tsock > 0) close(tsock);

    return NULL;
}

/* JJG: though this breaks coding fundamentals, this thread does not use locks to access
   ipv6ClientCnt; the service thread(s) do use locks because they are decrementing the value
   and perform a check for greater than 0; this thread only increments the value; there
   still may be potential where over time this thread and the service thread(s) get out of 
   sync, but for now, who cares (there are other issues that affect performance well before
   a perceived deadlock may occur at lower layers in the LLC) */
static void *ipbridgeThr(void __attribute__((unused)) *arg)
{
    /* receive data off ipv6 wireless ifc and pass to network ifc */
    int32_t csock = -1;  /* client - ipv6 network */
    int32_t ptsock = -1; /* pass through - sock listening for clients */
    int32_t tsock = -1;  /* target network */
    struct sockaddr_in6 client;
    int32_t locclientcnt = 0; 
    uint32_t size = (uint32_t)sizeof(struct sockaddr_in6);
    char_t cname[MAX_IPV6_STRING_LENGTH] = {0};
    char_t sname[MAX_IPV6_STRING_LENGTH] = {0};
    uint8_t clientidx;
    clientList **cptr, *clist = NULL;
    bool_t skipclient;
    struct linger l = { .l_onoff=1, .l_linger=1 };  /* magic numbers mean enable linger for max 1 sec */

    if (ipbCfg.ipbBridgeOnce) {
        if (ipbCfg.ipv4bridge) {
            if (!bridgeTo4Server(&tsock)) {
                return NULL;
            }
        } else {
            if (!bridgeTo6Server(&tsock)) {
                return NULL;
            }
        }
    }

    if (!bridgeInitPassThrough(&ptsock)) {
        if (ipbCfg.ipbBridgeOnce) {
            close(tsock);
        }
        return NULL;
    }

    if (ipbCfg.ipv4bridge) {
        sprintf(sname, "%d.%d.%d.%d", ipbCfg.ipv4addr[0],ipbCfg.ipv4addr[1],ipbCfg.ipv4addr[2],ipbCfg.ipv4addr[3]);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: using ipv4 server connection - server %s\n", sname);
    } else {
        inet_ntop(AF_INET6, &(ipbCfg.srvrIpv6addr), sname, MAX_IPV6_STRING_LENGTH);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: using ipv6 server connection - server %s\n", sname);
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbridgeThr: Entering main bridging operation %d\n", i2vUtilGetUTCTimeInSec());
#endif
    /* listening for client connections */
    while (mainloop) {
        if ((csock = accept(ptsock, (struct sockaddr *)&client, &size)) >= 0) {
            /* client connection */
            inet_ntop(AF_INET6, &(client.sin6_addr), cname, MAX_IPV6_STRING_LENGTH);
            clientidx = 0;
            SVC_GET_LOCK(&clientcntlock);
            locclientcnt = ipv6ClientCnt;
            SVC_PUT_LOCK(&clientcntlock);
            if (NULL == clist) {
                clist = (clientList *)calloc(1, sizeof(clientList));
                if (NULL == clist) {
                    /* ignore the request - out of memory */
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: ERROR: out of memory for servicing clients. errno=%d\n", errno);
                    usleep(1000);  /* wait a little time; if multiple requests, don't overwhelm system */
                    continue;
                }
                strcpy(clist->client, cname);
            } else {
                if (locclientcnt >= ipbCfg.bridgeClientNum) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: too many clients; ignoring request\n");
                    if (ipbCfg.delayRcvr) {
                        if (locclientcnt == ipbCfg.bridgeClientNum) {
                            wsu_wait_at_gate(&listenGate);
                        }
                    } else {
                        usleep(1000);
                    }
                    continue;
                }
                cptr = &clist;
                skipclient = WFALSE;
                while (cptr) {
                    SVC_GET_LOCK(ipv6ThrTracker[clientidx].thrLock);
                    if (ipv6ThrTracker[clientidx].clientSock == -1) {
                        /* clear record and use it; port is closed */
                        memset(ipv6ThrTracker[clientidx].clientName, 0, MAX_IPV6_STRING_LENGTH);
                        SVC_PUT_LOCK(ipv6ThrTracker[clientidx].thrLock);
                        strncpy((*cptr)->client, cname, MAX_IPV6_STRING_LENGTH);
                        cptr = NULL;   /* no more iterations required */
                        continue;
                    }
                    SVC_PUT_LOCK(ipv6ThrTracker[clientidx].thrLock);
                    if (!strcmp(cname, (*cptr)->client)) {
                        SVC_GET_LOCK(ipv6ThrTracker[clientidx].thrLock);
                        if (ipv6ThrTracker[clientidx].clientSock != csock){
                            skipclient = WTRUE; /* intermittent use for debug print - don't print while holding lock */
                        }
                        SVC_PUT_LOCK(ipv6ThrTracker[clientidx].thrLock);
                        if (skipclient) {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: ERROR: existing client detected, new request received, IGNORING %d\n", i2vUtilGetUTCTimeInSec());
                        } else {
                            /* the svc thread *should* be handling this request otherwise */
                            skipclient = WTRUE;
                        }
                        cptr = NULL;
                        continue;
                    }
                    if ((*cptr)->next) {
                        cptr = &((*cptr)->next);
                        clientidx++;
                        continue;
                    }
                    /* no next client, add new record */
                    (*cptr)->next = (clientList *)calloc(1, sizeof(clientList));
                    if (NULL == (*cptr)->next) {
                        /* out of memory */
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: ERROR: out of memory for servicing clients. errno=%d\n",errno);
                        skipclient = WTRUE;
                    } else {
                        strcpy((*cptr)->next->client, cname);
                        clientidx++;
                    }
                    cptr = NULL;
                }
                if (ipbCfg.delayRcvr) {
                    if (locclientcnt == ipbCfg.bridgeClientNum) {
                        wsu_wait_at_gate(&listenGate);
                    }
                }
                if (skipclient) {
                    usleep(1000);
                    continue;
                }
            }
            /* reaching here means a new client detected */
            SVC_GET_LOCK(&clientcntlock);
            ipv6ClientCnt++;
            locclientcnt = ipv6ClientCnt;
            SVC_PUT_LOCK(&clientcntlock);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge(%d): client%d detected %s (%d)\n", clientidx, locclientcnt, cname, i2vUtilGetUTCTimeInSec());
            /* now pass to service thread; by convention, clientidx is the matching thread to notify */
            SVC_GET_LOCK(ipv6ThrTracker[clientidx].thrLock);
            /* try to avoid closing sock until data flushed */
            setsockopt(csock, SOL_SOCKET, SO_LINGER, (void *)&l, sizeof(l));    /* no check for error */
            ipv6ThrTracker[clientidx].clientSock = csock;
            strcpy(ipv6ThrTracker[clientidx].clientName, cname);
            SVC_PUT_LOCK(ipv6ThrTracker[clientidx].thrLock);
            wsu_open_gate(ipv6ThrTracker[clientidx].thrGate);
            continue;
        }
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            /* accept returned recoverable errors - wait and then continue */
            usleep(100000);  /* temporary test change; waiting 1 ms seems like overkill; waiting 100 ms */
            continue;
        }
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: listening for clients generated error (%s)\n", strerror(errno));
        continue;
    }

    if (csock > 0) close(csock);
    if (tsock > 0) close(tsock);
    if (ptsock > 0) close(ptsock);

    /* for cppcheck: this isn't needed but add it to suppress the error */
    if (clist) free(clist);

    return NULL;
}

static bool_t bridgeInitPassThrough(int32_t *sock)
{
    int32_t var = 1;  /* reused */
    struct sockaddr_in6 svcaddr;

    if ((*sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: socket error for pass through (%s)\n", strerror(errno));
        return WFALSE;
    }

    /* reusable socket descriptor */
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (char_t *)&var, sizeof(var)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: setsocketopt error for pass through (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

/* JJG 20180327: LLC has a blocking behavior when transferring files; pulling files to OBU
   can result in a delay up to 10 seconds; the delay causes the interface to be unavailable
   and no amount of retries can fix it (trust me I tried - even with retries the thread dies);
   the only solution is to remove non blocking option from socket; maybe it will be fixed???
*/
#if 0
    /* non blocking support for graceful termination */
    if ((var = fcntl(*sock, F_GETFL, 0)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: pass through unable to get socket flags\n");
        close(*sock);
        return WFALSE;
    }

    var |= O_NONBLOCK;

    if (fcntl(*sock, F_SETFL, var) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: pass through unable to set socket flags\n");
        close(*sock);
        return WFALSE;
    }
#endif

    /* bind to service port defined in cfg */
    memset(&svcaddr, 0, sizeof(struct sockaddr_in6));
    svcaddr.sin6_family = AF_INET6;
    svcaddr.sin6_addr = in6addr_any;
    svcaddr.sin6_port = htons(ipbCfg.port);

    if (bind(*sock, (struct sockaddr *)&svcaddr, sizeof(svcaddr)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: bind error for pass through (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

    if (listen(*sock, ipbCfg.bridgeClientNum) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: listen error for pass through (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: listen gate successfully initialized\n");
    return WTRUE;
}

static bool_t bridgeTo6Server(int32_t *sock)
{
    int32_t var = 1;  /* reused */
    struct sockaddr_in6 svraddr;
    //char_t radio_ifc[10];
    uint8_t i;

    //sprintf(radio_ifc, WIRELESS_DEV_NAME "%d", ipbCfg.radioNum);

    if ((*sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: socket error for ipv6 server (%s)\n", strerror(errno));
        return WFALSE;
    }

    /* reusable socket descriptor */
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (char_t *)&var, sizeof(var)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: setsocketopt error for ipv6 server (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

    /* connect to service port and addr defined in cfg */
    memset(&svraddr, 0, sizeof(struct sockaddr_in6));
    svraddr.sin6_family = AF_INET6;
    for (i=0; i<LENGTH_IPV6; i++) {
        svraddr.sin6_addr.s6_addr[i] = ipbCfg.srvrIpv6addr[i];
    }
    svraddr.sin6_port = (uint16_t)htons(ipbCfg.port);
    svraddr.sin6_scope_id = (uint32_t)if_nametoindex(ETH_IFC_V2X);

    if (connect(*sock, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: connect error for ipv6 server (%s)\n", strerror(errno));
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"server ip: ");
        for (var=0; var<(LENGTH_IPV6-2); var=var+2) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%X%.2X:", svraddr.sin6_addr.s6_addr[var], svraddr.sin6_addr.s6_addr[var+1]);
        }
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%X%.2X\n",svraddr.sin6_addr.s6_addr[var], svraddr.sin6_addr.s6_addr[var+1]);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scope: %d\n", svraddr.sin6_scope_id);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"family: %d %d\n", svraddr.sin6_family, AF_INET6);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"port: %d\n", svraddr.sin6_port);
        close(*sock);
        return WFALSE;
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: successfully connected to ipv6 server\n");
    return WTRUE;
}

static bool_t bridgeTo4Server(int32_t *sock)
{
    int32_t var = 1;  /* reused */
    struct sockaddr_in svraddr;

    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: socket error for ipv4 server (%s)\n", strerror(errno));
        return WFALSE;
    }

    /* reusable socket descriptor */
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (char_t *)&var, sizeof(var)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: setsocketopt error for ipv4 server (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

    /* connect to service port and addr defined in cfg */
    memset(&svraddr, 0, sizeof(struct sockaddr_in));
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = (uint32_t)(ipbCfg.ipv4addr[3] << 24 | 
                                         ipbCfg.ipv4addr[2] << 16 |
                                         ipbCfg.ipv4addr[1] << 8 |
                                         ipbCfg.ipv4addr[0]);
    svraddr.sin_port = htons(ipbCfg.ipv4port);

    if (connect(*sock, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: connect error for ipv4 server (%s)\n", strerror(errno));
        close(*sock);
        return WFALSE;
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: successfully connected to ipv4 server %d\n", i2vUtilGetUTCTimeInSec());
    return WTRUE;
}

static bool_t bridgeXchg(bool_t direction, int32_t *client, int32_t *server)
{
    /* WFALSE is from client to server */
    int32_t c1, c2, newcnt, tmp;
    int32_t in = (direction) ? *server : *client;
    int32_t out = (direction) ? *client : *server;
    int32_t retries = 5;

    memset(ipbuf, 0, MAX_TCP_DATA);

    c1 = read(in, ipbuf, MAX_TCP_DATA);  /* TODO: verify radio stack chunks data when 
                                          IPv6 being used - this affects write mostly */
    if (c1 < 0 ) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: failed to read in exchanging data\n");
        return WFALSE;
    } else if (!c1) {  /* no data */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: party disconnected %d\n", i2vUtilGetUTCTimeInSec());
        return WFALSE;
    }

    c2 = write(out, ipbuf, c1);
    if ((c2 < 0) && (errno == EPIPE)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: broken link; cleaning up, errno=%d.\n",errno);
        return WFALSE;
    }

    if (c2 != c1) {
        if (c2 < 0) {
            /* nothing written, just an error */
            c2 = 0;
        }
        for (; retries; retries--) {
            newcnt = c1 - c2;
            tmp = write(out, &ipbuf[newcnt], newcnt);
            if (tmp < 0) {
                usleep(50000);
            } else {
                c2 += tmp;
            }
            if (c2 >= c1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: retry succeeded in writing full data\n");
                return WTRUE;
            }
        }
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb bridge: failed to write %d bytes data; write %d (%s)\n", c1, c2, strerror(errno));
        return WFALSE;
    }

    return WTRUE;
}

static void ipbUpdatePrfx(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    char_t *tok;
    int32_t i;
    uint16_t val;
    uint8_t *buf = (uint8_t *)configItem;

    tok = strtok(value, ":");
    for (i=0; i<IPV6_PREFIX_WORDS; i++) {
        if (NULL != tok) {
            val = (uint16_t)strtoul(tok, NULL, 16);
        } else {
            val = 0;
        }
        buf[i*2] = (uint8_t)(val >> 8);
        buf[i*2+1] = (uint8_t)val;
        tok = strtok(NULL, ":");
    }

    *status = I2V_RETURN_OK;
}

static void ipbUpdateIpv4(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    char_t  *tok;
    uint8_t *buf = (uint8_t *)configItem;
    uint8_t i;

    tok = strtok(value, ".");
    for (i=0; i<LENGTH_IPV4; i++) {
        if (NULL != tok) {
            buf[i] = (uint8_t)strtoul(tok, NULL, 0);
        } else {
            buf[i] = 0;
        }
        tok = strtok(NULL, ".");
    }

    *status = I2V_RETURN_OK;
}

static void ipbUpdateMac(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    char_t *tok;
    uint8_t *buf = (uint8_t *)configItem;
    uint8_t i;
    bool_t allzeros = WTRUE;

    tok = strtok(value, ":");
    for (i=0; i<LENGTH_MAC; i++) {
        if (NULL != tok) {
            buf[i] = (uint8_t)strtoul(tok, NULL, 16);
        } else {
            buf[i] = 0;
        }
        tok = strtok(NULL, ":");
        if (buf[i]) allzeros = WFALSE;
    }

    if (allzeros) {
        memset(buf, 0xee, LENGTH_MAC);   /* default value */
    }

    *status = I2V_RETURN_OK;
}

/*
 * This function when called overwrites the IP Server; it will check if bridging is enabled and whether the IPv4
 * data must be overwritten; for the WSA, if the URL does not provide an IPv6 address (not checked for IPv4), use
 * the defaults in config
 *
 * Clearly this function should be called AFTER the normal config parsing of all the other parameters
 */
static void ipbUpdateSrvrURL(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void GCC_UNUSED_VAR *configItem, uint32_t *status)
{
    struct addrinfo *result;
    struct addrinfo *res;
    int32_t ret = 0;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    bool_t usingipv4 = (ipbCfg.enableBridge && ipbCfg.ipv4bridge) ? WTRUE : WFALSE;
    char_t dummy[INET6_ADDRSTRLEN] = {0};

    /* If string is of zero length, it's not set, which is OK.  Don't do anything */
    if (strlen(value) == 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," IPBServerURL is empty, no override\n");
        *status = I2V_RETURN_OK;
        return;
    }

    /* preliminary setup; will be adjusted if value obtained */
    *status = I2V_RETURN_FAIL;

    /* if successful, call below generates list of addresses */
    ret = getaddrinfo(value, NULL, NULL, &result);
    if (ret) {
        if (ret == EAI_SYSTEM) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," system error executing getaddrinfo. errno=%d.\n",errno);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," error executing getaddrinfo (%s) [%s]\n", gai_strerror(ret), value);
        }
        /* no further processing; just exit function (status already set) */
        return;
    }

    /* look for FIRST appropriate (IPv4 vs IPv6) address */
    for (res = result; res != NULL; res = res->ai_next) {
        if (usingipv4 && (res->ai_addr->sa_family == AF_INET)) {
            sin = (struct sockaddr_in *)res->ai_addr;
            memcpy(ipbCfg.ipv4addr, &sin->sin_addr.s_addr, sizeof(ipbCfg.ipv4addr));
            inet_ntop(AF_INET, ipbCfg.ipv4addr, dummy, sizeof(dummy));
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," using IP address %s for server URL %s\n", dummy, value);
            *status = I2V_RETURN_OK;
            break;
        } else if (!usingipv4 && (res->ai_addr->sa_family == AF_INET6)) {
            sin6 = (struct sockaddr_in6 *)res->ai_addr;
            memcpy(ipbCfg.srvrIpv6addr, &sin6->sin6_addr.s6_addr, sizeof(ipbCfg.srvrIpv6addr));
            inet_ntop(AF_INET6, ipbCfg.srvrIpv6addr, dummy, sizeof(dummy));
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," using IP address %s for server URL %s\n", dummy, value);
            *status = I2V_RETURN_OK;
            break;
        }
    }

    if (NULL == res) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," unable to find appropriate server IP for IPv%d type\n", (usingipv4) ? 4 : 6);
    }

    freeaddrinfo(result);
    /* status already set */
}

static void ipbSetIPv6AddrInfo(void)
{
    uint8_t mac[LENGTH_MAC];
    int32_t ret;

    /* need to save the mac all the time */
    /* DSRC only operation. */
    if ((ret = wsuRisGetRadioMacaddress(RT_DSRC,ipbCfg.radioNum, mac)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," unable to retrieve default MAC address - using a hardcoded value\n");
    } else {
        memcpy(gwBytes, mac, LENGTH_MAC);
    }  /* if ret != RS_SUCCESS bytes already set from initialization */

    /* create link local all the time */
    memcpy(&linklocal[8], gwBytes, 3); /* first 3 bytes starting at this offset */
    memcpy(&linklocal[13], &gwBytes[3], 3); /* last 3 bytes starting at this offset */
    linklocal[8] ^= 0x02;  /* this bit is inverted - ipv6 requirement using mac addr for identifier */

    /* prefix value is arbitrary - not guaranteed to be unique beyond local network */
    linklocal[0] = 0xFE;
    linklocal[1] = 0x80;
    //linklocal[2] = 0x40;   /* FIXME: find the IPv6 requirement for setting this byte; current link local only sets first 2 bytes */

    /* common across ipv6 address */
    linklocal[11] = 0xFF;
    linklocal[12] = 0xFE;
    
    /* if config says use local, create link local addr from MAC */
    if (ipbCfg.uselocal) {
        memset(ipbCfg.ipv6addr, 0, LENGTH_IPV6);   /* clear what was loaded from config */
        memcpy(ipbCfg.ipv6addr, linklocal, LENGTH_IPV6);
    }
}

static void ipbEnableIPv6(void)
{
    char_t str[MAX_IPV6_STRING_LENGTH] = {0};
    char_t cmd[200] = {0};   /* a bit of buffer room */

    /* another required item; inform llc driver that ipv6 is enabled */
    if(system("echo 1 > /sys/module/cw_llc/parameters/IPv6Enabled") < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbEnableIPv6 system() llc Failed: errno=(%d)\n",errno);
        return;
    }
    inet_ntop(AF_INET6, ipbCfg.ipv6addr, str, MAX_IPV6_STRING_LENGTH);
    sprintf(cmd, "ip -6 addr add %s/64 dev %s", str, LLCIFCNAME);
#ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb dbg: enable cmd - %s\n", cmd);
#endif /* EXTRA_DEBUG */
    if(system(cmd) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbEnableIPv6 system() ipv6addr Failed: errno=(%d)\n",errno);
        return;
    }

    if (ipbCfg.enableBridge) {
        /* add the advertised IP to the llc interface; when OBU connects, it gets bridged;
           JJG: need to check this with routing between QNX io-pkt instances and the 
           impact of IPv6 to IPv6 bridging */
        inet_ntop(AF_INET6, ipbCfg.srvrIpv6addr, str, MAX_IPV6_STRING_LENGTH);
        sprintf(cmd, "ip -6 addr add %s/64 dev %s", str, LLCIFCNAME);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb dbg: link local enable cmd - %s\n", cmd);
#endif /* EXTRA_DEBUG */
        if(system(cmd) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbEnableIPv6 system() srvrIpv6addr Failed: errno=(%d)\n",errno);
            return;
        }
    }
}

static void ipbDisableIPv6(void)
{
    char_t str[MAX_IPV6_STRING_LENGTH] = {0};
    char_t cmd[200] = {0};   /* a bit of buffer room */

    /* inform llc driver that ipv6 is disabled */
    if(system("echo 0 > /sys/module/cw_llc/parameters/IPv6Enabled") < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbDisableIPv6 system() llc Failed: errno=(%d)\n",errno);
        return;
    }
    inet_ntop(AF_INET6, ipbCfg.ipv6addr, str, MAX_IPV6_STRING_LENGTH);
    /* sprintf(cmd, "ip -6 addr delete %s/64 dev " WIRELESS_DEV_NAME "%d", str, ipbCfg.radioNum); */
    sprintf(cmd, "ip -6 addr delete %s/64 dev %s", str, LLCIFCNAME);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb dbg: disable cmd - %s\n", cmd);
    if(system(cmd) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbDisableIPv6 system() ipv6addr Failed: errno=(%d)\n",errno);
        return;
    }
    /* remove link local (only if it wasn't used as IP) */
    if (!ipbCfg.uselocal) {
        inet_ntop(AF_INET6, linklocal, str, MAX_IPV6_STRING_LENGTH);
        /* sprintf(cmd, "ip -6 addr delete %s/64 dev " WIRELESS_DEV_NAME "%d", str, ipbCfg.radioNum); */
        sprintf(cmd, "ip -6 addr delete %s/64 dev %s", str, LLCIFCNAME);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb dbg: link local delete cmd - %s\n", cmd);
#endif /* EXTRA_DEBUG */
        if(system(cmd) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbDisableIPv6 system() linklocal Failed: errno=(%d)\n",errno);
            return;
        }
    }

    if (ipbCfg.enableBridge) {
        inet_ntop(AF_INET6, ipbCfg.srvrIpv6addr, str, MAX_IPV6_STRING_LENGTH);
        sprintf(cmd, "ip -6 addr delete %s/64 dev %s", str, LLCIFCNAME);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb dbg: link local enable cmd - %s\n", cmd);
#endif /* EXTRA_DEBUG */
        if(system(cmd) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbDisableIPv6 system() srvrIpv6addr Failed: errno=(%d)\n",errno);
            return;
        }
    }
}

static i2vReturnTypesT ipbInitRadio(void)
{
    int32_t retVal;
    uint32_t optRetVal = 0;
    WsmServiceType service;
    cfgType risCfg;
    int32_t i;
    rsResultCodeType risRet = RS_SUCCESS;
    uint8_t idx = 0;
    int32_t val;
    uint8_t *pval = (uint8_t *)&val;
    char_t buf[30];    /* extra space just for future developers who may overlook buffer size when refactoring */
    FILE *pf;
    uint32_t seed[2] = {0};
    uint64_t uuid;
    struct timeval tv;
    static appCredentialsType appCredential = {
        .cbServiceAvailable = NULL,
        .cbReceiveWSM = NULL,
    };

    /* IPB is a provider service ONLY */
    DECLARE_PROVIDER_SERVICE
    if ((optRetVal = strlen(ipbCfg.advertiseID)) < WSA_ADVERTISE_ID_LEN) {
        memcpy(pvService.advertiseId, ipbCfg.advertiseID, optRetVal);
        pvService.advertiseIdLength = optRetVal;
    } else {
        memcpy(pvService.advertiseId, ipbCfg.advertiseID, WSA_ADVERTISE_ID_LEN);
        pvService.advertiseIdLength = WSA_ADVERTISE_ID_LEN;
    }
    optRetVal = 0;

    risCfg.radioType = RT_DSRC;    /* JJG 20211213: always set for DSRC; no support on C-V2X */
    /* 20180308: additional 59xx configuration for deciding radio settings;
       this MUST be called before the initialization of the radio, why - I have no f'ing clue */
    if (ipbCfg.isNotCCH) {
        /* JJG: call riscfg for modifying the CCH channel; this affects all apps; in testing
           both radios need to be configured; instructions were a bit different, but this is
           what I needed to do to make this work; note this 'may' impact diversity??? */
        risCfg.u.CCHNum = ipbCfg.wsaChannel;
        if ((retVal = wsuRisSetCfgReq(CFG_ID_CCH_NUM_RADIO_0, &risCfg)) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CCH radio channel enable failed (%d)\n", retVal);
        }
        risCfg.u.CCHNum = ipbCfg.wsaChannel;
        if ((retVal = wsuRisSetCfgReq(CFG_ID_CCH_NUM_RADIO_1, &risCfg)) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CCH radio channel enable failed (%d)\n", retVal);
        }
    }
    /* 20220112: relocating this block; need to set the MAC BEFORE initiating the wsm thread */
    /* we need to set the MAC on the IPv6 radio; just fyi, we must always use radio 1, so that's why
       this code just assumes radio one; revisit in the future if the radio stack changes; don't ask me
       how or why this block of code is written this way; reused from dipv6.c from Hercules */
    /* we want microseconds, so rather than call i2vUtilGetTimeInMs, we use timeval struct for time */
    /* below command obtains unique ID bytes (not in actual uuid order but who cares, they're unique) */
    if (NULL != (pf = popen("hexdump -n 72 -e ' 8/4 \"%02x\" \"\\n\" ' /sys/devices/platform/ocotp/imx-ocotp0/nvmem | tail -1", "r"))) {
        if (NULL != fgets(buf, sizeof(char_t)*sizeof(buf), pf)) {
            uuid = strtoull(buf, NULL, 16);
            pclose(pf);
            seed[0] = (unsigned int)uuid;
            seed[1] = (unsigned int)(uuid >> 32);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbInitRadio fget Failed: errno=(%d)\n",errno);
            return I2V_RETURN_FAIL;
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to read unique ID; using time value only. errno=%d.\n", errno);
        gettimeofday(&tv, NULL);
        seed[0] = (unsigned int)tv.tv_usec;   /* ignore second seed; just use one seed */
    }

    /* seed random() */
    srandom(seed[0]);
    for (i=0; i<LENGTH_MAC; i++) {
        val = random();
        idx = random()%4;
        risCfg.u.RadioMacAddress[i] = pval[idx];
        if ((i==2) && seed[1]) {
            /* adjust seed to protect against duplicate UUID bytes */
            srandom(seed[1]);
        }
    }
    risCfg.u.RadioMacAddress[0] &= 0xFE;
    risCfg.u.RadioMacAddress[0] |= 0x02;

    risCfg.radioType = RT_DSRC;
    /* risRet = wsuRisGetCfgReq(CFG_ID_MACADDRESS_RADIO_1, &risCfg); */
    if (RS_SUCCESS != (risRet = wsuRisSetCfgReq(CFG_ID_MACADDRESS_RADIO_1, &risCfg))) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error setting MAC on IPv6 radio; ipb service inactive (%d)\n", risRet);
        /* should we terminate?  no; the log is present, ipv6 will simply be unavailable */
    }

    /* JJG: 20180124 - Cohda/LLC system isn't like legacy wsus allowing retrieval of
       radio characteristics without first initializing communication to the radio */ 
    if (!cfg.iwmhAppEnable) {
        if ((retVal = wsuRisInitRadioComm(MY_NAME, &appCredential)) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IPB radio init failed (%d)\n", retVal);
            return I2V_RETURN_FAIL;
        }
    }
    /* JJG: need to test this with iwmh; how does comment above impact different threads -
       there's a valid reason for the differences, but damn!!! */
    ipbSetIPv6AddrInfo();
    SET_PROVIDER_SERVICE_BASICS

#if defined(WSA_CATCHER)
    if (0 == strncmp(ipbCfg.pscval,"DP2P",LENGTH_IPB_PSC) ) {  
      SET_PROVIDER_SERVICE_POINT_TO_POINT 
    }
#endif

    /* if doing bridging, service mac must match rse mac */
    if (ipbCfg.enableBridge) {
        memcpy(pvService.providerMACAddress, pvService.gatewayMacAddress, LENGTH_MAC);
    }

    /* JJG: LLC configuration items (59xx); must be done before provider svc request but after comm init */
    risCfg.u.TxPwrLevel = ipbCfg.txPwr;
    if ((retVal = wsuRisSetCfgReq(CFG_ID_IPV6_TXPWRLEVEL, &risCfg)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to set ipv6 tx power level (%d)\n", retVal);
    }
    /* don't forget WSA power */
    risCfg.u.TxPwrLevel = ipbCfg.wsaPwr;
    if ((retVal = wsuRisSetCfgReq(CFG_ID_WSA_TXPWRLEVEL, &risCfg)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to set wsa power level (%d)\n", retVal);
    }
    risCfg.u.DataRate = ipbCfg.dataRate;
    if ((retVal = wsuRisSetCfgReq(CFG_ID_IPV6_DATARATE, &risCfg)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to set ipv6 tx rate (%d)\n", retVal);
    }
    risCfg.u.ProviderMode = WTRUE;
    if ((retVal = wsuRisSetCfgReq(CFG_ID_IPV6_PROVIDER_MODE, &risCfg)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to set provider mode (%d)\n", retVal);
    }

    pvService.radioNum = ipbCfg.radioNum;

    if (!cfg.wsaEnable) {
        /* this app should never be started if wsa disabled */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," WSAs not enabled - invalid config\n");
        return I2V_RETURN_FAIL;
    }

    I2V_UTIL_PROVIDER_SVC_REQ(&pvService, &cfg, WMH_SVC_TIMEOUT, &retVal, &optRetVal)
    if (retVal != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," wsuRisProviderServiceRequest failed (%d %d)\n", retVal, optRetVal);
        if (!cfg.iwmhAppEnable) {
            if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," terminateRadioComm failed (%d)\n", retVal);
            }
        }
        return I2V_RETURN_FAIL;
    } else {
        /* see comments in i2v_riscapture.h */
        memcpy(&svctrack.u.providerData, &pvService, sizeof(ProviderServiceType));
        svctrack.isUserService = WFALSE;
    }

    if (!cfg.iwmhAppEnable) {
        /* this call only needed if receiving wsms - disabling for iwmh */
        service.action = ADD;
        service.psid = ipbPsid;
        if ((retVal = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," wsuRisWsmServiceRequest failed (%d)\n", retVal);
            if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," terminateRadioComm failed (%d)\n", retVal);
            }
            return I2V_RETURN_FAIL;
        }
        memcpy(&svctrack.wsmData, &service, sizeof(WsmServiceType));
    }

#if 0
    outWSMMsg.radioNum = radioId;
    outWSMMsg.psid = ipbPsid;
    outWSMMsg.channelNumber = ipbCfg.channelNumber;
    outWSMMsg.txPwrLevel = cfg.txPwrLevel;
    memset(outWSMMsg.data, 0, sizeof(outWSMMsg.data));
#endif

    /* 20180308: this print should be after service registration; relocated from before registration */
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"service parameters\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"---------------------------\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Advertised addr: \n");
    for (i=0; i<LENGTH_IPV6; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X.\n", pvService.IPv6Address[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Advertised gateway: \n");
    for (i=0; i<LENGTH_IPV6; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X.\n", pvService.defaultGateway[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Advertised dns: \n");
    for (i=0; i<LENGTH_IPV6; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X.\n", pvService.primaryDns[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Advertised gateway mac: \n");
    for (i=0; i<LENGTH_MAC; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X.\n", pvService.gatewayMacAddress[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"---------------------------\n");

#ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb svc ipv6 addr: ");
    for (i=0; i<LENGTH_IPV6; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," %.2X", pvService.IPv6Address[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipb svc gateway addr: ");
    for (i=0; i<LENGTH_IPV6; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," %.2X", pvService.defaultGateway[i]);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
#endif /* EXTRA_DEBUG */

    return I2V_RETURN_OK;
}
STATIC uint32_t ipbSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      if(NULL != tag) {
          if(0 == strcmp("IPBServerURL",tag)) {  
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) is optinal, ignoring.\n",tag);
          } else {
              /* Nothing to do. */
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbSetConfDefault: NULL input.\n");
              ret = I2V_RETURN_NULL_PTR;
          }
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbSetConfDefault: NULL input.\n");
          ret = I2V_RETURN_NULL_PTR;
      }
  } else {
      if(0 == strcmp("RadioNum",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("WSAPriority",tag)) {
          *(uint8_t *)itemToUpdate = I2V_WSA_PRIORITY_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("WSATxRate",tag)) {
          *(uint8_t *)itemToUpdate = I2V_WSA_TX_RATE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("DataRate",tag)) {
          *(uint8_t *)itemToUpdate = I2V_WSA_DATA_RATE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("TransmitPower",tag)) {
          *(float32_t *)itemToUpdate = I2V_TX_POWER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%f)\n",tag,*(float32_t *)itemToUpdate);
      } else if(0 == strcmp("WSAPower",tag)) {
          *(float32_t *)itemToUpdate = I2V_TX_POWER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%f)\n",tag,*(float32_t *)itemToUpdate);
      } else if(0 == strcmp("OverridePower",tag)) {
          *(bool_t *)itemToUpdate = IPB_OVERRIDE_POWER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("EnableContinuous",tag)) {
          *(bool_t *)itemToUpdate = IPB_ENABLE_CONT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("OverrideCCH",tag)) {
          *(bool_t *)itemToUpdate = IPB_OVERRIDE_CHANNEL_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate)
      } else if(0 == strcmp("CCHChannel",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SecurityDebugEnable",tag)) {
          *(bool_t *)itemToUpdate = IPB_SECURITY_DEBUG_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBPSID",tag)) {
          *(uint32_t *)itemToUpdate = I2V_IPB_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("ChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("IPBEnableEDCA",tag)) {
          *(bool_t *)itemToUpdate = IPB_ENABLE_EDCA_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBAdvertiseID",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_AD_ID_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBMinSatellites",tag)) {
          *(uint8_t *)itemToUpdate = IPB_SV_COUNT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("IPBSatelliteTO", tag)) {
          *(uint8_t *)itemToUpdate = IPB_GNSS_TO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("IPBGPSOverride", tag)) {
          *(bool_t *)itemToUpdate = IPB_GNSS_OVERRIDE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBGPSSettleTime", tag)) {
          *(uint16_t *)itemToUpdate = IPB_GNSS_SETTLE_TIME_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("Elevation", tag)) {
          *(int16_t *)itemToUpdate = I2V_GPS_ELEV_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%d)\n",tag,*(int16_t *)itemToUpdate);
      } else if(0 == strcmp("Latitude", tag)) {
          *(float64_t *)itemToUpdate = I2V_GPS_LAT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%lf)\n",tag,*(float64_t *)itemToUpdate);
      } else if(0 == strcmp("Longitude", tag)) {
          *(float64_t *)itemToUpdate = I2V_GPS_LON_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%lf)\n",tag,*(float64_t *)itemToUpdate);
      } else if(0 == strcmp("PosConfidence", tag)) {
          *(uint8_t *)itemToUpdate = IPB_POS_CONFIDENCE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("IPBIPv6Prefix",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_IPV6_PREFIX_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBIPv6Identifier",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_IPV6_ID_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBIPv6Port",tag)) {
          *(uint16_t *)itemToUpdate = IPB_IPV6_PORT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("IPv6ServerPrefix",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_IPV6_SERVER_PREFIX_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPv6ServerID",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_IPV6_SERVER_ID_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBServiceMac",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_SRV_MAC_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBProviderCtxt",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_PROVIDER_CTXT_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPBIPv6UseLinkLocal",tag)) {
          *(bool_t *)itemToUpdate = IPB_IPV6_USE_LOCAL_LINK_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBEnableBridge",tag)) {
          *(bool_t *)itemToUpdate = IPB_ENABLE_BRIDGE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBBridgeOnce",tag)) {
          *(bool_t *)itemToUpdate = IPB_BRIDGE_ONCE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPBIPv4Bridge",tag)) {
          *(bool_t *)itemToUpdate = IPB_IPV4_BRIDGE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("BridgeMaxClients",tag)) {
          *(uint8_t *)itemToUpdate = IPB_BRIDGE_CLIENT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("ConnectionTimeout",tag)) {
          *(uint8_t *)itemToUpdate = IPB_CONNECT_TO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("BridgeDelayClients",tag)) {
          *(bool_t *)itemToUpdate = IPB_BRIDGE_DELAY_CLIENTS_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("IPv4ServerIP",tag)) {
          strcpy((char_t *)itemToUpdate,IPB_IPV4_SERVER_IP_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("IPv4ServerPort",tag)) {
          *(uint16_t *)itemToUpdate = IPB_IPV4_SERVER_PORT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("IPBServerURL",tag)) {  
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) is optinal, ignoring.\n",tag);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipbSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
/* config initialization function */
static i2vReturnTypesT ipbUpdateCfg(char_t *cfgFileName)
{
    cfgItemsTypeT cfgItems[] = {
        {"RadioNum",           (void *)i2vUtilUpdateUint8Value,  &ipbCfg.radioNum,      NULL,(ITEM_VALID| UNINIT)},
        {"WSAPriority",        (void *)i2vUtilUpdateUint8Value,  &ipbCfg.wsaPriority,   NULL,(ITEM_VALID| UNINIT)},
        {"WSATxRate",          (void *)i2vUtilUpdateUint8Value,  &ipbCfg.wsaTxRate,     NULL,(ITEM_VALID| UNINIT)},
        {"TransmitPower",      (void *)i2vUtilUpdateFloatValue,  &ipbCfg.txPwr,         NULL,(ITEM_VALID| UNINIT)},
        {"WSAPower",           (void *)i2vUtilUpdateFloatValue,  &ipbCfg.wsaPwr,        NULL,(ITEM_VALID| UNINIT)},
        {"OverridePower",      (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.forceIPBCfgPwr,NULL,(ITEM_VALID| UNINIT)},
        {"DataRate",           (void *)i2vUtilUpdateUint8Value,  &ipbCfg.dataRate,      NULL,(ITEM_VALID| UNINIT)},
        {"EnableContinuous",   (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.isContinuous,  NULL,(ITEM_VALID| UNINIT)},
        {"OverrideCCH",        (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.isNotCCH,      NULL,(ITEM_VALID| UNINIT)},
        {"CCHChannel",         (void *)i2vUtilUpdateUint8Value,  &ipbCfg.wsaChannel,    NULL,(ITEM_VALID| UNINIT)},
        {"SecurityDebugEnable",(void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.secDebug,      NULL,(ITEM_VALID| UNINIT)},
        {"IPBPSID",            (void *)i2vUtilUpdatePsidValue,   &ipbCfg.psid,          NULL,(ITEM_VALID| UNINIT)},
        {"ChannelNumber",      (void *)i2vUtilUpdateUint8Value,  &ipbCfg.channelNumber, NULL,(ITEM_VALID| UNINIT)},
        {"IPBAdvertiseID",     (void *)i2vUtilUpdateStrValue,    &ipbCfg.advertiseID,   NULL,(ITEM_VALID| UNINIT)},
        {"IPBMinSatellites",   (void *)i2vUtilUpdateUint8Value,  &ipbCfg.minSats,       NULL,(ITEM_VALID| UNINIT)},
        {"IPBGPSOverride",     (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.usegps,        NULL,(ITEM_VALID| UNINIT)},
        {"IPBSatelliteTO",     (void *)i2vUtilUpdateUint8Value,  &ipbCfg.sattime,       NULL,(ITEM_VALID| UNINIT)},
        {"IPBGPSSettleTime",   (void *)i2vUtilUpdateUint16Value, &ipbCfg.gpstime,       NULL,(ITEM_VALID| UNINIT)},
        {"Latitude",           (void *)i2vUtilUpdateDoubleValue, &ipbCfg.latitude,      NULL,(ITEM_VALID| UNINIT)},
        {"Longitude",          (void *)i2vUtilUpdateDoubleValue, &ipbCfg.longitude,     NULL,(ITEM_VALID| UNINIT)},
        {"Elevation",          (void *)i2vUtilUpdateInt16Value,  &ipbCfg.elevation,     NULL,(ITEM_VALID| UNINIT)},
        {"PosConfidence",      (void *)i2vUtilUpdateUint8Value,  &ipbCfg.confidence,    NULL,(ITEM_VALID| UNINIT)},
        {"IPBIPv6Port",        (void *)i2vUtilUpdateUint16Value, &ipbCfg.port,          NULL,(ITEM_VALID| UNINIT)},
        {"IPBProviderCtxt",    (void *)i2vUtilUpdateStrValue,    &ipbCfg.pscval,        NULL,(ITEM_VALID| UNINIT)},
        {"IPBIPv6UseLinkLocal",(void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.uselocal,      NULL,(ITEM_VALID| UNINIT)},
        {"IPBEnableBridge",    (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.enableBridge,  NULL,(ITEM_VALID| UNINIT)},
        {"IPBBridgeOnce",      (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.ipbBridgeOnce, NULL,(ITEM_VALID| UNINIT)},
        {"IPBIPv4Bridge",      (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.ipv4bridge,    NULL,(ITEM_VALID| UNINIT)},
        {"BridgeMaxClients",   (void *)i2vUtilUpdateUint8Value,  &ipbCfg.bridgeClientNum,  NULL,(ITEM_VALID| UNINIT)},
        {"ConnectionTimeout",  (void *)i2vUtilUpdateUint8Value,  &ipbCfg.cnxnTOSec,     NULL,(ITEM_VALID| UNINIT)},
        {"BridgeDelayClients", (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.delayRcvr,     NULL,(ITEM_VALID| UNINIT)},
        {"IPBIPv6Prefix",      NULL,                             &ipbCfg.ipv6addr,      (void *)ipbUpdatePrfx,(ITEM_VALID| UNINIT)},
        {"IPBIPv6Identifier",  NULL,                             &ipbCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],(void *)ipbUpdatePrfx,(ITEM_VALID| UNINIT)},
        {"IPv4ServerIP",       NULL,                             &ipbCfg.ipv4addr,      (void *)ipbUpdateIpv4,(ITEM_VALID| UNINIT)},
        {"IPv4ServerPort",     (void *)i2vUtilUpdateUint16Value, &ipbCfg.ipv4port,      NULL,(ITEM_VALID| UNINIT)},
        {"IPv6ServerPrefix",   NULL,                             &ipbCfg.srvrIpv6addr,  (void *)ipbUpdatePrfx,(ITEM_VALID| UNINIT)},
        {"IPv6ServerID",       NULL,                             &ipbCfg.srvrIpv6addr[IPV6_PREFIX_WORDS * 2],(void *)ipbUpdatePrfx,(ITEM_VALID| UNINIT)},
        {"IPBServiceMac",      NULL,                             &ipbCfg.svcMac,         (void *)ipbUpdateMac,(ITEM_VALID| UNINIT)},
        {"IPBEnableEDCA",      (void *)i2vUtilUpdateWBOOLValue,  &ipbCfg.enableEDCA,     NULL,(ITEM_VALID| UNINIT)},
        {"IPBServerURL",       NULL,                             NULL, (void *)ipbUpdateSrvrURL,(ITEM_VALID| UNINIT)},
    };
    FILE *f;
    char_t fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
    uint32_t retVal;  /* used for i2vUtilParseConfFile - fxn returns i2vReturnTypesT although listed as uint32_t */
    //i2vLogErrT result;
    uint32_t i;
#if 0
    mapShrIntDataT mapmem;
    uint8_t ctr = 20;   /* 10 seconds max - will make sense later */
#endif

    /* Wait on I2V SHM to get I2V CFG. If fail then error out. No stand alone support.*/
    for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
        WSU_SEM_LOCKR(&shmPtr->cfgData.h.ch_lock);
        if(WTRUE == shmPtr->cfgData.h.ch_data_valid) {
            memcpy(&cfg, &shmPtr->cfgData, sizeof(cfgItemsT));  /* Copy config for later use. */
            WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
            break; /* Got it so break out.*/
        }
        WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
        usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    /* If we fail to get I2V SHM update or sig'ed to exit beforehand then fail. */
    if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
        return I2V_RETURN_FAIL;
    }

    strncpy(fileloc, cfg.config_directory, I2V_CFG_MAX_STR_LEN);
    strcat(fileloc, cfgFileName);

    if ((f = fopen(fileloc, "r")) == NULL) {
        return I2V_RETURN_INV_FILE_OR_DIR;
    }

    if(I2V_RETURN_OK != (retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL))) {
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: not happy i2vRet=%u.\n",retVal);
        //set_ipb_error_state(IPB_READ_CFG_FAIL);
    }
    if (f != NULL) fclose(f);

    /* Check if conf was missing needed values or had bogus values. Recover and keep going. */       
    for (i = 0; i < NUMITEMS(cfgItems); i++) {
        if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
            || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
            if(I2V_RETURN_OK == (retVal = ipbSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                cfgItems[i].state = (ITEM_VALID|INIT);
                //set_ipb_error_state(IPB_HEAL_CFGITEM);
            } else {
                I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override FAILED.\n"); /* FATAL. */
                break; /* Heal has failed. FATAL. */
            }
        }
    }
    if (retVal != I2V_RETURN_OK) {
        return retVal;
    }

    /* data rate and transmit power verification */
    if ((ipbCfg.dataRate != 3)  && (ipbCfg.dataRate != 6) && (ipbCfg.dataRate != 9) &&
        (ipbCfg.dataRate != 12)  && (ipbCfg.dataRate != 18) && (ipbCfg.dataRate != 24) &&
        (ipbCfg.dataRate != 36)  && (ipbCfg.dataRate != 48) && (ipbCfg.dataRate != 54)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid IPB data rate\n");
        return I2V_RETURN_FAIL;
    }
    if (!cfg.bcastLockStep || ipbCfg.forceIPBCfgPwr) {
        if ((ipbCfg.txPwr > 23) || (ipbCfg.txPwr < 0) || (ipbCfg.wsaPwr > 23) || (ipbCfg.wsaPwr < 0)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid IPB cfg pwr [S:%.1f I:%.1f]; valid range 0 - 23 dBm\n", ipbCfg.txPwr, ipbCfg.wsaPwr);
            return I2V_RETURN_FAIL;
        }
        if (((ipbCfg.txPwr - ((int)ipbCfg.txPwr)) != 0) && ((ipbCfg.txPwr - ((int)ipbCfg.txPwr)) != 0.5)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid IPB tx pwr increments [%.1f] (valid is 0.5 dBm steps); rounding down\n", ipbCfg.txPwr);
            ipbCfg.txPwr = ((int)ipbCfg.txPwr);
        }
        if (((ipbCfg.wsaPwr - ((int)ipbCfg.wsaPwr)) != 0) && ((ipbCfg.wsaPwr - ((int)ipbCfg.wsaPwr)) != 0.5)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid IPB wsa pwr increments [%.1f] (valid is 0.5 dBm steps); rounding down\n", ipbCfg.wsaPwr);
            ipbCfg.wsaPwr = ((int)ipbCfg.wsaPwr);
        }
    }

    if (cfg.bcastLockStep) {  /* all rse apps broadcast on same radio and channel */
        /* 20190910: do not use same channel as other services (spat/map) */
        /* ipbCfg.channelNumber = cfg.uchannel; */
        /* 20190910: the radio number here is for the IPv6 service; this should never be on the same radio as
           the other I2V apps (spat/map/etc); always use the opposite in lockstep mode */
        ipbCfg.radioNum = !cfg.uradioNum;
        if (!ipbCfg.forceIPBCfgPwr) {
            ipbCfg.txPwr = cfg.txPwrLevel;   /* special case just for IPB */
            ipbCfg.wsaPwr = cfg.txPwrLevel;   /* special case just for IPB */
        }
    }
    if (cfg.iwmhAppEnable) {
        /* overrides bcastLockStep and local cfg channel setting */
        ipbCfg.radioNum = cfg.cchradio;  /* ONLY ipb makes this modification */
    }

    WSU_SEM_LOCKR(&shmPtr->ipbCfgData.h.ch_lock);
    memcpy(&ipbCfg.h, &shmPtr->ipbCfgData.h, sizeof(wsu_shm_hdr_t)); /* preserve header */
    ipbCfg.h.ch_data_valid = WTRUE;
    memcpy(&shmPtr->ipbCfgData, &ipbCfg, sizeof(ipbCfgItemsT));
    WSU_SEM_UNLOCKR(&shmPtr->ipbCfgData.h.ch_lock);

    ipbPsid = (cfg.security) ? cfg.secIpbPsid : ipbCfg.psid;
    radioId = ipbCfg.radioNum;

    /* for multithreading, do not allow bridge once option */
    if (ipbCfg.bridgeClientNum > 1) ipbCfg.ipbBridgeOnce = WFALSE;

    /* update lat/long values - multiply by 10000000 for J2735 format (WSA format) */
    if (cfg.enableGPSOverride) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"overriding IPB cfg gps with I2V cfg gps\n");
        ipbCfg.latitude = cfg.gpsOvrLat;
        ipbCfg.longitude = cfg.gpsOvrLong;
        ipbCfg.elevation = cfg.gpsOvrElev;
    }
    ipbCfg.latitude *= 1e7;
    ipbCfg.longitude *= 1e7;

    /* JJG: FIXME: this was broken with addition of 2016 apps; for now ignore; 
       see if restoration possible */
    /* 
     * All map data encode prior to arrival via AMH. Data not available.
     * Map Intersection center different than RSU location.
     */
#if 0 
    if (cfg.mapAppEnable) {
        WSU_SEM_LOCKR(&shmPtr->mapIntersectionData.h.ch_lock);
        mapmem = shmPtr->mapIntersectionData;
        WSU_SEM_UNLOCKR(&shmPtr->mapIntersectionData.h.ch_lock);

        while (!mapmem.h.ch_data_valid && (ctr > 0) && !mapmem.ignoreData) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," waiting for map data\n");
            usleep(500000);   /* wait 1/2 second */
            ctr--;
            WSU_SEM_LOCKR(&shmPtr->mapIntersectionData.h.ch_lock);
            mapmem = shmPtr->mapIntersectionData;
            WSU_SEM_UNLOCKR(&shmPtr->mapIntersectionData.h.ch_lock);
        }

        if (mapmem.h.ch_data_valid) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," using MAP intersection elev, lat, long dat\n");
            ipbCfg.latitude = mapmem.intLat;
            ipbCfg.longitude = mapmem.intLong;
            ipbCfg.elevation = mapmem.intElev;
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," MAP intersection data not available - using config\n");
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," MAP app not running - using config\n");
    }
#else
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," MAP app not supported - using AMH & DOT website for MAP\n");
#endif

    return retVal;
}
int32_t MAIN(void)
{
#define SMILOG "/mnt/rwflash/I2V/applogs/ipbsmi.log"
#define SMISTATS "/mnt/rwflash/I2V/applogs/ipbsmistats.log"
    struct sigaction sa;
    int32_t ret = I2V_RETURN_UNKNOWN;
    pthread_t brThr;
    pthread_attr_t attr;
    uint8_t i, j;
    pthread_mutex_t blankmux = PTHREAD_MUTEX_INITIALIZER;

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,IPB_CFG_FILE,sizeof(confFileName));
#endif
    ipbInitStatics();
    if(0 != pthread_mutex_init(&blankmux ,NULL)){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"blankmux Mutex init failed\n");
        return I2V_RETURN_FAIL;
    }
    /* Not used today. */
    if(0 != pthread_mutex_init(&gpslock ,NULL)){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gpslock Mutex init failed\n");
        pthread_mutex_destroy(&blankmux);
        return I2V_RETURN_FAIL;
    }
    shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shmPtr == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Main shared memory init failed\n");
        return I2V_RETURN_FAIL;
    }

    if (I2V_RETURN_OK != ipbUpdateCfg(confFileName)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Failed to load config\n");
        return I2V_RETURN_FAIL;
    }

    if (I2V_RETURN_OK != ipbInitRadio()) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Failed to init radio\n");
        return I2V_RETURN_FAIL;
    }

    ipbEnableGPS();

    /* catch SIGINT/SIGTERM */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = ipbSigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* ipb needs tps with ipbCfg.usegps */
    if (!cfg.security && ipbCfg.usegps) {
        if (cfg.enableGPSOverride) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Using file override GPS instead of actual GPS data\n");
            DBGSETGPSLOCATION(cfg);
        } else {
            if (I2V_RETURN_OK != ipbTPSSetup()) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not register for tps data for security\n");
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
        }
    }

    if (cfg.iwmhAppEnable) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"iwmh enabled - security and radio handled by iwmh\n");
    }
    if (cfg.security) {
        /* enable security */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," security enabled - starting security init\n");
        memset(&tpsUserData, 0, sizeof(i2vTpsTypeT));
        if (cfg.enableGPSOverride) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Using file override GPS instead of actual GPS data\n");
            DBGSETGPSLOCATION(cfg);
        } else {
            if (I2V_RETURN_OK != ipbTPSSetup()) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not register for tps data for security\n");
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
        }
#if 0
        if (!cfg.iwmhAppEnable) {
            /* most calls to securityinitialize will block forever (until terminated) -
               i2v library function has a loop counter - in theory we could do the same
               here by looping the call - but for now just make the counter really 
               really big */
            if (I2V_RETURN_OK != securityInitialize(radioId, 0xFFFF, IPB_V3_SECPSID, (void *)ipbGPSLocation, 
            WTRUE, ipbCfg.secDebug, SMILOG, SMISTATS, NULL, NULL)) { /* WTRUE = wsa enable */
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not connect to security module\n");
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
        }
#endif
    }

    ipbEnableIPv6();

    if (ipbCfg.enableBridge) {
        if (ipbCfg.delayRcvr) {
            wsu_init_gate(&listenGate);
        }
        pthread_attr_init(&attr);
        if (pthread_create(&brThr, &attr, ipbridgeThr, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create bridge thread\n");
            MAIN_CLEANUP();
            return I2V_RETURN_FAIL;
        }
        pthread_detach(brThr);
        memset(&ipv6ThrTracker, 0, sizeof(ipv6ThrTracker));
        for (i=0; i<ipbCfg.bridgeClientNum; i++) {
            /* svc thread setup requires allocation for lock and gates */
            ipv6ThrTracker[i].thrGate = (wsu_gate_t *)calloc(1, sizeof(wsu_gate_t));
            if (NULL == ipv6ThrTracker[i].thrGate) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create bridge service thread %d gate\n", i+1);
                mainloop = WFALSE;  /* for pthread */
                SVC_CLEANUP()
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
            wsu_init_gate(ipv6ThrTracker[i].thrGate);
            ipv6ThrTracker[i].thrLock = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
            if (NULL == ipv6ThrTracker[i].thrLock) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create bridge service thread %d Lock\n", i+1);
                mainloop = WFALSE;  /* for pthread */
                free(ipv6ThrTracker[i].thrGate);
                SVC_CLEANUP()
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
            memcpy(ipv6ThrTracker[i].thrLock, &blankmux, sizeof(pthread_mutex_t));
            ipv6ThrTracker[i].clientName = (char_t *)calloc(1, MAX_IPV6_STRING_LENGTH);
            if (NULL == ipv6ThrTracker[i].clientName) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create bridge service thread %d name\n", i+1);
                mainloop = WFALSE;  /* for pthread */
                free(ipv6ThrTracker[i].thrGate);
                free(ipv6ThrTracker[i].thrLock);
                SVC_CLEANUP()
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
            ipv6ThrTracker[i].pid = (pthread_t *)calloc(1, sizeof(pthread_t));
            if (NULL == ipv6ThrTracker[i].pid) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create bridge service thread %d pid\n", i+1);
                mainloop = WFALSE;  /* for pthread */
                free(ipv6ThrTracker[i].thrGate);
                free(ipv6ThrTracker[i].thrLock);
                free(ipv6ThrTracker[i].clientName);
                SVC_CLEANUP()
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
            if (pthread_create(ipv6ThrTracker[i].pid, &attr, ipbridgeSvcThr, &ipv6ThrTracker[i]) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not create svc bridge thread %d\n", i+1);
                mainloop = WFALSE;  /* for pthread */
                free(ipv6ThrTracker[i].thrGate);
                free(ipv6ThrTracker[i].thrLock);
                free(ipv6ThrTracker[i].clientName);
                free(ipv6ThrTracker[i].pid);
                SVC_CLEANUP()
                MAIN_CLEANUP();
                return I2V_RETURN_FAIL;
            }
            pthread_detach(*ipv6ThrTracker[i].pid);
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," in full pass through mode, no bridging\n");
    }

    while (mainloop) {
        if (!mainloop) {
            break;
        }
        usleep(1000000);  /* nothing to do - just sleep a sec */
#if defined(MY_UNIT_TEST)
        break; /* One and done. */
#endif
    }

    if (ipbCfg.enableBridge) {
        i = ipbCfg.bridgeClientNum;  /* to take advantage of macro */
        SVC_CLEANUP()
    }
    ipbDisableIPv6();
    timer_delete(gpstimer);

    /* see comments in i2v_riscapture.h */
    svctrack.u.providerData.action = DELETE;
    I2V_UTIL_PROVIDER_SVC_REQ(&svctrack.u.providerData, &cfg, WMH_SVC_TIMEOUT, &svctrack.sdummy, &svctrack.udummy)
    if (svctrack.sdummy != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"deleting provider service failed (%d); radio may not restart correctly without reboot\n", svctrack.sdummy);
    }

    if (!cfg.iwmhAppEnable) {
        svctrack.wsmData.action = DELETE;
        if ((ret = wsuRisWsmServiceRequest(&svctrack.wsmData)) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmServiceRequest cleanup failed (%d), radio may not restart correctly without reboot\n", ret);
        }
        if ((ret = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminateRadioComm failed (%d)\n", ret);
        }
    }

    if (ipbCfg.delayRcvr) {
        wsu_kill_gate(&listenGate);
    }

    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();

    pthread_mutex_destroy(&gpslock);
    pthread_mutex_destroy(&blankmux);

    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));
    return I2V_RETURN_OK;
#undef SMILOG
#undef SMISTATS
}
