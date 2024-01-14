/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: wme.c                                                            */
/*  Purpose: The WME module of radioServices                                  */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.  All Rights Reserved. */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#define  __USE_GNU   /* Needed for pthread.h to enable pthread_setname_np() */
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include "rpsi.h"
#include "rsu_defs.h"
#include "shm_rsk.h"
#include "wme.h"
#include "nscfg.h"
#include "ns_cv2x.h"
#include "ipcsock.h"
#include "type2str.h"
#include "alsmi.h"
#include "ns_pcap.h"
#include "i2v_util.h"
#include "nscfg.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "wme"

extern shm_rsk_t *shm_rsk_ptr;
extern int  risSocket;
extern bool_t  smiInitialized;
extern bool_t  p2pEnabled;
extern bool_t  hdr1609p2;
/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/
//#define DEBUG_PARSE_RECEIVED_WSA
#ifdef DEBUG_PARSE_RECEIVED_WSA
#define DEBUG_PARSE_RECEIVED_WSA_PRINTF(fmt, args...) \
    { \
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"[ %s() %d]: " fmt, __func__, __LINE__, ##args); \
    }
#else
#define DEBUG_PARSE_RECEIVED_WSA_PRINTF(fmt, args...)
#endif

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define     WSA_INFORMATION_EXTENSION_ELEMENT_PRESENT 0x08
#define     WSA_SERVICE_INFO_SEGMENT_PRESENT          0x04
#define     WSA_CHANNEL_INFO_SEGMENT_PRESENT          0x02
#define     WSA_WAVE_ROUTING_ADVERTISEMENT_PRESENT    0x01
#define     WSA_IDENTIFIER_MASK                       0xF0
#define     WSA_CONTENT_COUNT_MASK                    0x0F
#define     WSA_CHANNEL_INDEX_MASK                    0xF8
#define     WSA_SERVICE_INFO_OPTION_PRESENT           0x01
#define     WSA_CHANNEL_INFO_OPTION_PRESENT           0x01
#define     WSA_CHANNEL_INFO_ADAPTABLE                0x80
#define     WSA_CHANNEL_INFO_DATA_RATE_MASK           0x7F

#define     VSC_MANAGEMENT_ID                         3
#define     RS_CV2X_RX_TIMEOUT_USEC                   5000000 /* 5 seconds */

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef enum {
    WSA_NO,
    WSA_ACTIVE,
    WSA_INACTIVE,
} WSAStatusType;

typedef struct {
    time_t WSALastSignTime;
    uint32_t length;
    uint8_t  data[MAX_WSA_DATA];
} toBeSignedWsaDataType;

typedef struct {
    ASTEntryType      *ASTEntry;
    rsRadioType        RadioType;
    uint8_t            RadioNum;
    uint8_t            ServicePriority;
    uint8_t            ChannelNumber;   // WSM Channel Number
    bool_t             Adaptable;
    uint8_t            DataRate;
    uint8_t            TxPwrLevel;
    bool_t             UserService;     // User entry = TRUE, Provider entry = FALSE
    uint32_t           PSID;
    uint8_t            LengthPsc;
    uint8_t            PSC[LENGTH_PSC];
    rsWsaSecurityType  WsaSecurity;
    /* 20220511: TODO update for ATLKS */
    bool_t             cv2xTxInited;

    union {
        struct {
            rsUserRequestType Access;
            uint8_t           SrcMacAddress[LENGTH_MAC]; // Source Mac address of packet
            uint8_t           LengthAdvertisId;
            uint8_t           AdvertiseId[LENGTH_ADVERTISE];
            int8_t            LinkQuality;
            bool_t            ImmediateAccess;
            uint16_t          ExtendedAccess;
        } user;

        struct {
            uint8_t  DestMacAddress[LENGTH_MAC];
                                    // if FF:FF:FF:FF:FF:FF, accept incoming WSA with
                                    // any source MAC address. Otherwise, incoming
                                    // WSA must match srcMacAddress field to be accepted.
            uint8_t  LengthSSP;
            uint8_t  ServiceSpecificPermissions[MAX_SECURITY_PERMISSIONS];
                                    // Security processing P1609.2
            uint8_t  LengthSSID;
            uint8_t  SecurityServiceId[MAX_SECURITY_SERVICE_ID];
                                    // Security service
                                    // (if length != 0)
            uint8_t  edca_count;    // Number of elements in the EDCA arrays below [1..4] (optional)
            uint16_t txop[4];       // Channel info EDCA, TxOp limit (b0-b15) (optional)
            uint8_t  ecw[4];        // Channel info EDCA, ECWmin b0-b3, ECWmax b4-b7 (optional)
            uint8_t  aifs[4];       // Channel info EDCA, Arbitration Interframe Spacing (optional)
            CHANType ChannelAccess; // 0: both CHANTYPE_CCH, CHANTYPE_SCH (continuous mode)
                                    // 1: CHANTYPE_CCH (alternating mode)
                                    // 2: CHANTYPE_SCH (alternating mode)
            uint8_t  RepeatRate;    // The number of WSA transmitted every 5 seconds.
                                    // If destination MAC address is an individual
                                    // address, Repeat rate is ignored
            bool_t   IpService;     // Indicates whether the advertised service is
                                    // IP-based, and a WAVE Routing Advertisement is
                                    // needed to support this service.
            uint8_t  IPv6Address[LENGTH_IPV6];
                                    // IPv6 Address
            uint16_t IPv6ServicePort;
                                    // IPv6 Service Port
            uint8_t  ProviderMacAddress[LENGTH_MAC];
                                    // Service MAC Address
            uint8_t  RcpiThreshold; // For insertion in WSA if present
            uint8_t  WsaCountThreshold;
                                    // For insertion in WSA if present
            uint8_t  WsaCountThresholdInterval;
                                    // For insertion in WSA if present
            uint32_t WsaHeaderExtension;
                                    // Indicates which of the WSA header extension
                                    // should be included in the transmitted WSA
            uint16_t SignatureLifeTime;
                                    // The number of milliseconds over which the WSA
                                    // signature should be valid (10 - 30,000 ms)
            uint8_t  advertiseIdLength;
                                    // Advertiser ID length
            int32_t  latitude2D;    // WSA header 2D Location, latitude
            int32_t  longitude2D;   // WSA header 2D Location, longitude
            int32_t  latitude3D;    // WSA header 3D Location and confidence, latitude
            int32_t  longitude3D;   // WSA header 3D Location and confidence, longitude
            int16_t  elevation3D;   // WSA header 3D Location and confidence, elevation
            uint8_t  confidence3D;  // WSA header 3D Location and confidence, confidence
            uint8_t  semiMajorAccuracy3D;
                                    // WSA header 3D Positional accuracy, semi-major
            uint8_t  semiMinorAccuracy3D;
                                    // WSA header 3D Positional accuracy, semi-minor
            uint16_t semiMajorOrientation3D;
                                    // WSA header 3D Positional accuracy, semi-major orientation
            uint8_t  advertiseId[WSA_ADVERTISE_ID_LEN];
                                    // WSA header advertiser ID
            uint16_t routerLifeTime;// Routing info router life time
            uint8_t  ipPrefix[LENGTH_IPV6];
                                    // Routing info IP Prefix
            uint8_t  prefixLen;     // Routing info prefix length
            uint8_t  defaultGateway[DEFAULT_GATEWAY_LEN];
                                    // Routing info default gateway
            uint8_t  primaryDns[LENGTH_IPV6];
                                    // Routing info primary DNS
            uint8_t  secondaryDns[LENGTH_IPV6];
                                    // Routing info secondary DNS
            uint8_t  gatewayMacAddress[LENGTH_MAC];
                                    // Routing info gateway MAC address
        } provider;
    } status;
} GeneralServiceStatusEntryType; /* Registered generic service status */

typedef struct {
    uint8_t                       GSTCount; // 0..MAX_SERVICES
    GeneralServiceStatusEntryType GSTEntry[MAX_SERVICES];
} GeneralServiceTableType;

typedef struct {
    ASTEntryType *ASTEntry;
    rsRadioType   RadioType;     /* RT_CV2X or RT_DSRC */
    uint32_t      PSID;          /* PSID */
    uint16_t      destPort;      /* Destination port number */
} WsmStatusEntryType; /* Registered wsm service status */

typedef struct {
    uint8_t            WSMCount; // 0..MAX_SERVICES
    WsmStatusEntryType WSMEntry[MAX_SERVICES];
} WsmStatusTableType;

typedef struct {
    int32_t latitude;
    int32_t longitude;
} WSA_2DLocation;

typedef struct {
    int32_t latitude;
    int32_t longitude;
    int16_t elevation;
} WSA_3DLocation;

typedef struct {
    uint32_t psid;
    int8_t   channelIndex;
    uint8_t  psc[LENGTH_PSC];
    uint8_t  ipv6Address[LENGTH_IPV6];
    uint16_t servicePort;
    uint8_t  providerMacAddress[LENGTH_MAC];
    int8_t   rcpi;
    uint8_t  countThreshold;
    uint8_t  countThresholdInterval;
} WSA_ServiceInfo;

typedef struct {
    uint8_t  qosInfo;
    uint16_t txop[4];
    uint8_t  ecw[4];
    uint8_t  aifs[4];
} WSA_ChannelInfo_EdcaParamSet;

typedef struct {
    uint8_t operatingClass;
    uint8_t channelNumber;
    uint8_t adaptable;
    uint8_t dataRate;
    uint8_t txPwrLevel;
    WSA_ChannelInfo_EdcaParamSet edca;
    uint8_t channelAccess; // 0: Both SCH and CCH interval access
                           // 1: CCH interval access only
                           // 2: SCH interval access only
} WSA_ChannelInfo;

typedef struct {
    uint16_t routerLifeTime;                      // router life time
    uint8_t  ipPrefix[LENGTH_IPV6];               // IP Prefix length
    uint8_t  prefixLen;                           // prefix length
    uint8_t  defaultGateway[DEFAULT_GATEWAY_LEN]; // default gateway length
    uint8_t  primaryDns[LENGTH_IPV6];             // primary DNS length
    uint8_t  secondaryDns[LENGTH_IPV6];           // secondary DNS length
    uint8_t  gatewayMacAddress[LENGTH_MAC];       // gateway MAC length
} WSA_RoutingInfo;

typedef struct {
    WSAStatusType          wsaStatus;
    struct timeval         receivedTimestamp;
    uint64_t               serviceAvailableTimestamp;
    dsrc_edca_param_set_t  edca_params;
    int8_t                 rss;
    uint8_t                peer_mac_addr[LENGTH_MAC];
    uint8_t                changeCount;
    bool_t                 versionChange;
    uint32_t               WSABitmask;
    WSA_2DLocation         wsa2d;
    WSA_3DLocation         wsa3d;
    uint8_t                lengthAdvertiseId;
    uint8_t                advertiseId[WSA_ADVERTISE_ID_LEN];
    uint8_t                repeatRate;
    uint8_t                serviceCount;
    uint32_t               serviceBitmask[MAX_SERVICE_INFO];
    uint8_t                servicePscLength[MAX_SERVICE_INFO];
    WSA_ServiceInfo        service[MAX_SERVICE_INFO];
    uint8_t                channelCount;
    uint32_t               channelBitmask[MAX_CHANNEL_INFO];
    WSA_ChannelInfo        channelInfo[MAX_CHANNEL_INFO];
    uint32_t               routingBitmask;
    WSA_RoutingInfo        routingInfo;
    uint8_t                wsaData[MAX_WSA_DATA];
    uint32_t               wsaLen;
    sem_t                  WSAStatusTableSem;
    sem_t                  WSAVerifySem;
} WSAStatusTableType;

/*----------------------------------------------------------------------------*/
/* Global Variables                                                           */
/*----------------------------------------------------------------------------*/
/* Registered Application Status Table */
ApplicationStatusTableType   AST                 = {0};
bool_t                       cv2x_started        = FALSE;
bool_t                       vodEnabled          = FALSE;
bool_t                       ignoreWSAs          = FALSE;
rsRadioType                  lastVerifyRadioType = RT_CV2X;

/* WME can impact if packets rx or tx above and beyond CV2X */
extern rskStatusType cv2xStatus;
/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static pthread_t cv2xRxThreadId;
static bool_t cv2x_inited          = FALSE;
static bool_t cv2x_tx_inited       = FALSE;
static bool_t cv2xRxThread_created = FALSE;
static bool_t cv2xRxThreadRunning  = FALSE;

/* WSA related */
static uint8_t                 WSAContentCount[MAX_RADIOS] = {0, 0};
static vru_wsa_req             wsaData[MAX_RADIOS];
static toBeSignedWsaDataType   toBeSignedWsa[MAX_RADIOS];
static wsaTxSecurityType       wsaTxSecurity[MAX_RADIOS];
/* WSA Tx Thread */
static pthread_t               wsaTxThreadID[MAX_RADIOS];
static bool_t                  wsaTxThreadRunning[MAX_RADIOS];
static vru_wsa_req            *currentWsaData[MAX_RADIOS];

/* Registered Service Status Table */
static GeneralServiceTableType GST = {0};

/* Registered WSM Service Status Table */
static WsmStatusTableType      WST = {0};

/* WSAStatusTable */
static WSAStatusTableType      WSAStatusTable[NUM_WSA] = {{0}};

/* WSA Timeout Thread */
static pthread_t               wsaTimeoutThreadID;
static bool_t                  wsaTimeoutThreadRunning = FALSE;

/* Can we re-use any of this for CV2X? */
/* Saved radio configuration information */
static bool_t                  radioConfigured[MAX_RADIOS] = {FALSE, FALSE};
static dsrc_sch_service_req_t  radioConfiguration[MAX_RADIOS];
/*----------------------------------------------------------------------------*/
/* Function protos                                                            */
/*----------------------------------------------------------------------------*/
static rsResultCodeType createWSMPHeader(sendWSMStruct_S *sendWSMStruct,
                                         uint8_t *hdr, uint8_t *wsmpHdrLen);

static void MLMEX_WSAEND_REQ(uint8_t radioNum, vru_wsa_req *wsaData);

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

void initWme(uint32_t debugEnable)
{
}

#ifdef RSU_DEBUG
static void dump(void *ptr, int len)
{
    uint8_t *p = (uint8_t *)ptr;
    int l;
    int i;

    while (len != 0) {
        l = ((len > 16) ? 16 : len);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%04lx ", (uint8_t *)p - (uint8_t *)ptr);

        for (i = 0; i < l; i++) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%02x ", p[i]);
        }

        for (i = 0; i < l; i++) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%c%c",
                    ((p[i] >= ' ') && (p[i] <= '~')) ? p[i] : '.',
                    (i == l - 1) ? '\n' : ' ');
        }

        p += l;
        len -= l;
    }
}

static void dumpSendWSMStruct(const sendWSMStruct_S *sendWSMStruct)
{
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmpVersion = %u\n", sendWSMStruct->wsmpVersion);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"channelNumber = %u\n", sendWSMStruct->channelNumber);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dataRate = %u\n", sendWSMStruct->dataRate);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"txPwrLevel = %u\n", sendWSMStruct->txPwrLevel);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"userPriority = %u\n", sendWSMStruct->userPriority);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"expireTime = %u\n", sendWSMStruct->expireTime);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioType = %s\n", (sendWSMStruct->radioType == RT_CV2X) ? "RT_CV2X" :
                                                                       "RT_????");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioNum = %u\n", sendWSMStruct->radioNum);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"channelInterval = %u\n", sendWSMStruct->channelInterval);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"security = %u\n", sendWSMStruct->security);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"securityFlag = 0x%08x\n", sendWSMStruct->securityFlag);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"destMac = %02x:%02x:%02x:%02x:%02x:%02x\n",
    sendWSMStruct->destMac[0], sendWSMStruct->destMac[1],
    sendWSMStruct->destMac[2], sendWSMStruct->destMac[3],
    sendWSMStruct->destMac[4], sendWSMStruct->destMac[5]);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmpHeaderExt = 0x%x\n", sendWSMStruct->wsmpHeaderExt);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"psid = 0x%x\n", sendWSMStruct->psid);
}

static void dumpApplicationStatusTableType(ApplicationStatusTableType *AST)
{
    int i;

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AST ASTCount=%u\n",
           AST->ASTCount);
    for (i = 0; i < MAX_APPS; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d pid=%u AppStatus=%d AppPriority=%u index=%d\n", 
               i, AST->ASTEntry[i].pid, AST->ASTEntry[i].AppStatus,
               AST->ASTEntry[i].AppPriority,AST->ASTEntry[i].index);
    }
}

static void dumpGeneralServiceTableType(GeneralServiceTableType *GST)
{
    int i;

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GSTCount = %u\n", GST->GSTCount);

    for (i = 0; i < MAX_SERVICES; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d ASTE%p RT%d RN%u SP%u CN%u A%d DR%u TxP%u US%d PSID0x%x cv2xI%d\n",
               i, GST->GSTEntry[i].ASTEntry, GST->GSTEntry[i].RadioType,
               GST->GSTEntry[i].RadioNum, GST->GSTEntry[i].ServicePriority,
               GST->GSTEntry[i].ChannelNumber, GST->GSTEntry[i].Adaptable,
               GST->GSTEntry[i].DataRate, GST->GSTEntry[i].TxPwrLevel,
               GST->GSTEntry[i].UserService, GST->GSTEntry[i].PSID,
               cv2x_tx_inited);
    }

}

static void dumpWsmStatusTableType(WsmStatusTableType *WST)
{
    int i;

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSMCount = %u\n", WST->WSMCount);
    
    for (i = 0; i < MAX_SERVICES; i++) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d ASTEntry=%p RadioType=%s PSID=0x%x destPort=%u\n", 
               i, WST->WSMEntry[i].ASTEntry,
               "RT_CV2X",
               WST->WSMEntry[i].PSID,
               WST->WSMEntry[i].destPort);
    }
}

/* Non-static to avoid compilation warning */
void dumpTables(void)
{
    dumpApplicationStatusTableType(&AST);
    dumpGeneralServiceTableType(&GST);
    dumpWsmStatusTableType(&WST);
}
#endif

/**
** Function:  addWST
** @brief  Add the WSM service entry to WST
** @param  ASTEntry  -- [input]The entry in the AST
**                            associated with the application's pid
** @param  pid       -- [input]the application PID
** @param  radioType -- [input]RT_CV2X or RT_DSRC
** @param  psid      -- [input]Unregister WSM service associated with this psid
** @return 0 for success.
**
** Details: First checks to see that the application is not already registered.
**          If it isn't, then an empty entry is found in WST and the
**          registration data is stored there.
**/
static WsmStatusEntryType *addWST(ASTEntryType *entry, pid_t pid,
                                  rsRadioType radioType, uint32_t psid)
{
    int     i;
    uint8_t count = 0;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pid = %u; psid = 0x%x\n", pid, psid);
#endif
    /* Check if radioType/psid is already registered */
    for (i = 0; ((i < MAX_SERVICES) && (count < WST.WSMCount)); i++) {
        if ((WST.WSMEntry[i].ASTEntry != NULL) &&
            (WST.WSMEntry[i].RadioType == radioType) &&
            (WST.WSMEntry[i].PSID == psid)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RadioType/PSID %s/0x%x already exists!\n",
                       "RT-CV2X",
                       psid);
            return NULL;
        }

        if (WST.WSMEntry[i].ASTEntry != NULL) {
            count++;
        }
    }

    if (WST.WSMCount >= MAX_SERVICES) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Too Many Services!\n");
        return NULL;
    }

    /* Find an empty spot and register */
    for (i = 0; i < MAX_SERVICES; i++) {
        if (WST.WSMEntry[i].ASTEntry == NULL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Found entry %d\n", i);
#endif
            WST.WSMEntry[i].ASTEntry      = entry;
            WST.WSMEntry[i].ASTEntry->pid = pid;
            WST.WSMEntry[i].RadioType     = radioType;
            WST.WSMEntry[i].PSID          = psid;
            WST.WSMCount++;
            return &WST.WSMEntry[i];
        }
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Shouldn't have gotten here!\n");
    return NULL;
}

/**
** Function:  getAppStatusTableEntry
** @brief  Get an entry from the AST.
** @param  pid -- [input]If non-zero, find the AST entry
**                associated with this pid. If zero, find an unallocated (new)
**                AST entry.
** @return Pointer to the AST entry. NULL if not found or error.
**
** Details: Searches through the AST, and finds an entry
**          that matches the pid. The pid will be 0 in all table entries that
**          are unallocated, so use pid = 0 to search for a new entry.
**/
static ASTEntryType *getAppStatusTableEntry(pid_t pid)
{
    int     i;
    int32_t entry_count = 0;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pid = %u\n", pid);
#endif
    if ((pid != 0) && (AST.ASTCount == 0)) {
        return NULL;
    }

    for (i = 0; i < MAX_APPS; i++) {
        if (AST.ASTEntry[i].pid == pid) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning entry %d\n", i);
#endif
            return &AST.ASTEntry[i];
        }

        if ((pid != 0) && (AST.ASTEntry[i].pid != 0)) {
            if (++entry_count >= AST.ASTCount) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(entry_count >= AST.ASTCount)\n");
                break;
            }
        }
    }
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Not found\n");
#endif
    return NULL;
}

/**
** Function:  getGeneralServiceStatusEntry
** @brief  Get an entry from the GST.
** @param  radioType -- [input]The radio type to search for in the table.
** @param  PSID      -- [input]The PSID to search for in the table.
** @return Pointer to the GST entry. NULL if not found or error.
**
** Details: Searches through the GST, and finds an entry that matches the radio
**          type and PSID.
**/
static GeneralServiceStatusEntryType *getGeneralServiceStatusEntry(
                                          rsRadioType radioType,
                                          uint32_t PSID
                                      )
{
    int i;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioType = %d; PSID = 0x%x\n", radioType, PSID);
#endif
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL) &&
            (GST.GSTEntry[i].RadioType == radioType) &&
            (GST.GSTEntry[i].PSID == PSID)) {
            return &GST.GSTEntry[i];
        }
    }

    return NULL;
}

/**
** Function:  getWsmStatusEntry
** @brief  Get an entry from the WST.
** @param  RadioType -- [input]RT_CV2X or RT_DSRC
** @param  PSID      -- [input]The PSID to search for in the table.
** @return Pointer to the WST entry. NULL if not found or error.
**
** Details: Searches through the WST, and finds an entry
**          that matches the PSID.
**/
static WsmStatusEntryType *getWsmStatusEntry(rsRadioType RadioType,
                                             uint32_t PSID)
{
    int i;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RadioType = %s; PSID = 0x%x\n","RT_CV2X", PSID);
#endif
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((WST.WSMEntry[i].ASTEntry != NULL) &&
            (WST.WSMEntry[i].RadioType == RadioType) &&
            (WST.WSMEntry[i].PSID == PSID)) {
            return &WST.WSMEntry[i];
        }
    }

    return NULL;
}

/**
** Function: sendServiceAvailableIndications
** @brief  Send service available or service not available indications to the
**         appropriate applications.
** @param  wsaNum       -- [input]The WSA associated with this service available
**                         indication.
** @param  serviceAvail -- [input]RS_SERVICE_AVAILABLE to send a service
**                         available indication. RS_SERVICE_NOT_AVAILABLE to
**                         send a service not available indication.
** @param  signedVerifiedFlags -- [input]For a service available indication,
**                                the signed/verified flags to send along with
**                                it. Not used for service not available
**                                indications.
** @return void
**
** Details: Compares the WSAStatusTable entry for the specified WSA with the GST
**          table to find PSIDs that were registered for by the applications
**          that match the PSIDs in the service info segments of the WSA. If
**          some are found, then one or more serviceInfoType structures are
**          initialized and sent to the proper port, i.e. an indication is sent
**          up to the application. For a service available indication, the
**          signed and verified flags in the service bitmask are overridden by
**          the value in signedVerifiedFlags.
**/
static void sendServiceAvailableIndications(int wsaNum, uint8_t serviceAvail,
                                            uint32_t signedVerifiedFlags)
{
    int32_t                        i, j, k;
    uint8_t                        serviceCount = 0;
    GeneralServiceStatusEntryType *gst[MAX_SERVICE_INFO];
    uint8_t                        serviceIdx[MAX_SERVICE_INFO];
    int8_t                         channelIdx;
    rsReceiveDataType              recv;
    uint16_t                       risRecvIndPort = 0;

    for (i = 0; i < MAX_SERVICE_INFO; i++) {
        gst[i] = NULL;
    }

    /* Do for each GST table entry */
    for (i = 0; i < MAX_SERVICES; i++) {
        /* GST table entry in use and usersService? */
        if ((GST.GSTEntry[i].ASTEntry == NULL) ||
            !GST.GSTEntry[i].UserService) {
            continue;
        }

        /* For each service info segment in the WSA */
        for (j = 0; j < WSAStatusTable[wsaNum].serviceCount; j++) {
            /* GST PSID matches WSA service info segment PSID and
             * application receiving service available indications? */
            if ((WSAStatusTable[wsaNum].service[j].psid != GST.GSTEntry[i].PSID) ||
                !GST.GSTEntry[i].ASTEntry->serviceAvailableInd) {
                continue;
            } /* PSIDs match and Application receiving service available indications*/

            /* Save pointer to the GST table entry, along with
             * the service bitmask, the PSC, and the service
             * info segment contents */
            gst[serviceCount]        = &GST.GSTEntry[i];
            serviceIdx[serviceCount] = j;
            serviceCount++;
        } /* For each service info segment */
    } /* For each GST table entry */

    /* For each saved GST table entry, fill out a serviceInfoType structure and
     * send it to the appropriate application */
    for (j = 0; j < serviceCount; j++) {
        recv.indType = IND_SERVICE_AVAILABLE;
        /* General WSA info */
        recv.u.serviceInfo.serviceAvail              = serviceAvail;
        recv.u.serviceInfo.rss                       = WSAStatusTable[wsaNum].rss;
        recv.u.serviceInfo.WSABitmask                = WSAStatusTable[wsaNum].WSABitmask;
        recv.u.serviceInfo.advertiseIdLength         = WSAStatusTable[wsaNum].lengthAdvertiseId;
        recv.u.serviceInfo.changeCount               = WSAStatusTable[wsaNum].changeCount;
        recv.u.serviceInfo.repeatRate                = WSAStatusTable[wsaNum].repeatRate;
        recv.u.serviceInfo.latitude2D                = WSAStatusTable[wsaNum].wsa2d.latitude;
        recv.u.serviceInfo.longitude2D               = WSAStatusTable[wsaNum].wsa2d.longitude;
        recv.u.serviceInfo.latitude3D                = WSAStatusTable[wsaNum].wsa3d.latitude;
        recv.u.serviceInfo.longitude3D               = WSAStatusTable[wsaNum].wsa3d.longitude;
        recv.u.serviceInfo.elevation3D               = WSAStatusTable[wsaNum].wsa3d.elevation;
        memcpy(&recv.u.serviceInfo.advertiseId,       &WSAStatusTable[wsaNum].advertiseId,   recv.u.serviceInfo.advertiseIdLength);
        memcpy(recv.u.serviceInfo.sourceMac,           WSAStatusTable[wsaNum].peer_mac_addr, LENGTH_MAC);

        /* Service info segment info */
        if (serviceAvail == RS_SERVICE_AVAILABLE) {
            /* Override the signed/verified flags in the service bitmask with
             * the values passed to this routine */
            WSAStatusTable[wsaNum].serviceBitmask[serviceIdx[j]] &= ~(WSA_SIGNED | WSA_VERIFIED);
            WSAStatusTable[wsaNum].serviceBitmask[serviceIdx[j]] |= (signedVerifiedFlags & (WSA_SIGNED | WSA_VERIFIED));
        }

        recv.u.serviceInfo.servicePscLength          = WSAStatusTable[wsaNum].servicePscLength[serviceIdx[j]];
        recv.u.serviceInfo.serviceBitmask            = WSAStatusTable[wsaNum].serviceBitmask[serviceIdx[j]];
        recv.u.serviceInfo.psid                      = WSAStatusTable[wsaNum].service[serviceIdx[j]].psid;
        recv.u.serviceInfo.channelIndex              = WSAStatusTable[wsaNum].service[serviceIdx[j]].channelIndex;
        memcpy(recv.u.serviceInfo.psc,                &WSAStatusTable[wsaNum].service[serviceIdx[j]].psc, LENGTH_PSC);
        memcpy(recv.u.serviceInfo.ipv6Address,        &WSAStatusTable[wsaNum].service[serviceIdx[j]].ipv6Address, LENGTH_IPV6);
        recv.u.serviceInfo.port                      = WSAStatusTable[wsaNum].service[serviceIdx[j]].servicePort;
        memcpy(recv.u.serviceInfo.providerMacAddress, &WSAStatusTable[wsaNum].service[serviceIdx[j]].providerMacAddress, LENGTH_MAC);
        recv.u.serviceInfo.rcpiThreshold             = WSAStatusTable[wsaNum].service[serviceIdx[j]].rcpi;
        recv.u.serviceInfo.wsaCountThreshold         = WSAStatusTable[wsaNum].service[serviceIdx[j]].countThreshold;
        recv.u.serviceInfo.wsaCountThresholdInterval = WSAStatusTable[wsaNum].service[serviceIdx[j]].countThresholdInterval;

        /* Channel info segment info */
        channelIdx = WSAStatusTable[wsaNum].service[serviceIdx[j]].channelIndex - 1;

        if ((channelIdx >= 0) && (channelIdx < WSAStatusTable[wsaNum].channelCount)) {
            recv.u.serviceInfo.channelBitmask = WSAStatusTable[wsaNum].channelBitmask[channelIdx];
            recv.u.serviceInfo.operatingClass = WSAStatusTable[wsaNum].channelInfo[channelIdx].operatingClass;
            recv.u.serviceInfo.channelNumber  = WSAStatusTable[wsaNum].channelInfo[channelIdx].channelNumber;
            recv.u.serviceInfo.adaptable      = WSAStatusTable[wsaNum].channelInfo[channelIdx].adaptable;
            recv.u.serviceInfo.dataRate       = WSAStatusTable[wsaNum].channelInfo[channelIdx].dataRate;
            recv.u.serviceInfo.txPwrLevel     = WSAStatusTable[wsaNum].channelInfo[channelIdx].txPwrLevel;
            recv.u.serviceInfo.qosInfo        = WSAStatusTable[wsaNum].channelInfo[channelIdx].edca.qosInfo;
            memcpy(recv.u.serviceInfo.txop,     WSAStatusTable[wsaNum].channelInfo[channelIdx].edca.txop, sizeof(recv.u.serviceInfo.txop));
            memcpy(recv.u.serviceInfo.ecw,      WSAStatusTable[wsaNum].channelInfo[channelIdx].edca.ecw,  sizeof(recv.u.serviceInfo.ecw));
            memcpy(recv.u.serviceInfo.aifs,     WSAStatusTable[wsaNum].channelInfo[channelIdx].edca.aifs, sizeof(recv.u.serviceInfo.aifs));
            recv.u.serviceInfo.channelAccess  = WSAStatusTable[wsaNum].channelInfo[channelIdx].channelAccess;
        }

        /* WRA info */
        recv.u.serviceInfo.routingBitmask          = WSAStatusTable[wsaNum].routingBitmask;
        recv.u.serviceInfo.routerLifeTime          = WSAStatusTable[wsaNum].routingInfo.routerLifeTime;
        memcpy(recv.u.serviceInfo.ipPrefix,          WSAStatusTable[wsaNum].routingInfo.ipPrefix,          LENGTH_IPV6);
        recv.u.serviceInfo.prefixLen               = WSAStatusTable[wsaNum].routingInfo.prefixLen;
        memcpy(recv.u.serviceInfo.defaultGateway,    WSAStatusTable[wsaNum].routingInfo.defaultGateway,    LENGTH_IPV6);
        memcpy(recv.u.serviceInfo.primaryDns,        WSAStatusTable[wsaNum].routingInfo.primaryDns,        LENGTH_IPV6);
        memcpy(recv.u.serviceInfo.secondaryDns,      WSAStatusTable[wsaNum].routingInfo.secondaryDns,      LENGTH_IPV6);
        memcpy(recv.u.serviceInfo.gatewayMacAddress, WSAStatusTable[wsaNum].routingInfo.gatewayMacAddress, LENGTH_MAC);

        /* Set signedWsaHandle and channelAvail */
        recv.u.serviceInfo.signedWsaHandle = (uint32_t)wsaNum + 1;
        recv.u.serviceInfo.channelAvail    = RS_SCH_NOT_ASSIGNED;

        if (serviceAvail == RS_SERVICE_AVAILABLE) {
            for (i = 0; i < MAX_RADIOS; i++) {
                k = shm_rsk_ptr->wsuNsCurService[RT_DSRC][i].CurServiceIndex;

                if (GST.GSTEntry[k].ASTEntry           != NULL &&
                    GST.GSTEntry[k].PSID               == recv.u.serviceInfo.psid &&
                    GST.GSTEntry[k].ChannelNumber      == recv.u.serviceInfo.channelNumber &&
                    GST.GSTEntry[k].status.user.Access != NO_SCH_ACCESS) {
                    recv.u.serviceInfo.channelAvail = RS_SCH_ASSIGNED;
                    break;
                }
            }
        }

        /* report WSAStatusTable event back to the application */
        risRecvIndPort = RIS_RECV_IND_PORT1 + gst[j]->ASTEntry->index;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sending indication to port %u\n", risRecvIndPort);
#endif
        if (!wsuSendData(risSocket, risRecvIndPort, &recv, sizeof(recv))) {
            RSU_PRINTF_MAX_N_TIMES(10, "wsuSendData() failed\n");
        }
    } /* For each saved GST table entry */
}

#ifndef NO_SECURITY
/**
** Function: verifyWSA
** @brief  Submit a WSA for verification.
** @param  signedWsaHandle: The handle to the signed WSA
** @param  vodResult: Pointer to the VOD result if submitting the WSA for
**                    verification fails.
** @return RS_SUCCESS if success, error code if failure
**
** Details: Calls alsmiVerifyWSA() with the saved WSA data from the
**          WSAStatusTable to submit the WSA for verification.
**/
rsResultCodeType verifyWSA(uint32_t signedWsaHandle, AEROLINK_RESULT *vodResult)
{
    rsResultCodeType risRet;

    shm_rsk_ptr->RISReqCnt.VerifyWsaReq++;

    if ((signedWsaHandle > NUM_WSA) ||
        ((WSAStatusTable[signedWsaHandle - 1].wsaStatus != WSA_ACTIVE) &&
         (WSAStatusTable[signedWsaHandle - 1].wsaStatus != WSA_INACTIVE))) {
        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
        *vodResult = WS_ERR_NOT_FOUND;
        return RS_EVERWSAFAIL;
    }

    risRet = alsmiVerifyWSA(WSAStatusTable[signedWsaHandle - 1].wsaData,
                            WSAStatusTable[signedWsaHandle - 1].wsaLen,
                            signedWsaHandle, vodResult);
    return risRet;
}
#endif // NO_SECURITY
/**
** Function: verifyWSACfm
** @brief  Send a service available indication to tha application if the WSA
**         verification was successful.
** @param  vodResult: The return code from Aerolink from the verification.
** @param  signedWsaHandle: The handle to the signed WSA
** @return none.
**
** Details:
**/
void verifyWSACfm(AEROLINK_RESULT vodResult, uint32_t signedWsaHandle)
{
    if (vodResult == WS_SUCCESS) {
        shm_rsk_ptr->WSACnt.WSAVerifySuccess++;
        sendServiceAvailableIndications(signedWsaHandle - 1, RS_SERVICE_AVAILABLE,
            (WSA_SIGNED | WSA_VERIFIED));
    }
    else {
        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
    }
}

/**
** Function: WSATimeout
** @brief  Time out the specified WSA
** @param  wsaNum: The WSA number to time out.
** @return void
**
** Details: The function is called when we wish to time out a WSA. A
**          SERVICE_NOT_AVAILABLE is sent to all applications waiting to join a
**          service or who have already joined a service. The WSAStatusTable entry is
**          deleted.
**
**          This function is called from wsaTimeoutThread(), which is a thread
**          that executes once a second. That routine checks to see if no WSA
**          from a certain RSE has been received within a certain amount of
**          time, and if so, it calls WSATimeout().
**/
static void WSATimeout(int wsaNum)
{

    if (WSAStatusTable[wsaNum].wsaStatus == WSA_NO) {
        return;
    }
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSA %d timed out\n", wsaNum);
#endif
    /* Send the indication to the appropriate applications */
    sendServiceAvailableIndications(wsaNum, RS_SERVICE_NOT_AVAILABLE, 0);

    /* Remove the WSAStatusTable entry from the table */
    WSAStatusTable[wsaNum].wsaStatus = WSA_NO;
}

/**
** Function: wsaTimeoutThread
** @brief  Checks periodically to see if any WSA's should be timed out.
** @param  ptr -- [input]Required by convention; not used.
** @return void *
**
** Details: This function checks once a seconds to see if no WSA from a certain
**          RSE has been received within a certain amount of time (returned by
**          getWSATimeoutInterval()), and if so, it times out the WSA.
**/
static void *wsaTimeoutThread(void *ptr)
{
    int            i;
    int            count;
    int32_t        WSATimeoutInterval;
    struct timeval tv, tv_diff;

    wsaTimeoutThreadRunning = TRUE;

    shm_rsk_ptr->bootupState |= 0x200; // wsaTimeoutThread running
    while (wsaTimeoutThreadRunning) {
        /* Delay 1 second */
        sleep(1);

        /* Exit if we want to terminate this thread */
        if (!wsaTimeoutThreadRunning) {
            break;
        }

        /* Get a count of the number of WSA's in the table */
        count = 0;

        for (i = 0; i < NUM_WSA; i++) {
            if (WSAStatusTable[i].wsaStatus != WSA_NO) {
                count++;
            }
        }

        /* Only process if we have a non-empty table */
        if (count == 0) {
            continue;
        }

        /* Get the current time and timeout interval */
        gettimeofday(&tv, NULL);
        WSATimeoutInterval = (count > 1) ? getWSATimeoutInterval2() :
                                           getWSATimeoutInterval();

        /* Check each WSA to see if it is expired */
        for (i = 0; i < NUM_WSA; i++) {
            if (WSAStatusTable[i].wsaStatus != WSA_NO) {
                /* Wait on the semaphore */
                if (sem_trywait(&WSAStatusTable[i].WSAStatusTableSem) < 0) {
                    shm_rsk_ptr->WSACnt.WSASemaphoreError++;
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Taking WSAStatusTable semaphore failed\n");
                    continue;
                }

                /* Compute how long since the WSA was last received. */
                tv_diff = tv;

                if (tv_diff.tv_usec < WSAStatusTable[i].receivedTimestamp.tv_usec) {
                    tv_diff.tv_usec += 1000000;
                    tv_diff.tv_sec--;
                }

                tv_diff.tv_sec  -= WSAStatusTable[i].receivedTimestamp.tv_sec;
                tv_diff.tv_usec -= WSAStatusTable[i].receivedTimestamp.tv_usec;

                /* If WSA has expired, time it out */
                if ((tv_diff.tv_sec > WSATimeoutInterval / 1000) ||
                    ((tv_diff.tv_sec == WSATimeoutInterval / 1000) &&
                     (tv_diff.tv_usec > (WSATimeoutInterval % 1000) * 1000))) {
                    WSATimeout(i);
                }

                /* Post the semaphore */
                if (sem_post(&WSAStatusTable[i].WSAStatusTableSem) < 0) {
                    fprintf(stderr, "%s: Error posting to semaphore: %s\n", __func__, strerror(errno));
                }

                /* Exit the for loop early if we run out of WSA's */
                if (--count == 0) {
                    break;
                }
            } // if (WSAStatusTable[i].wsaStatus != WSA_NO)
        } // for (i = 0; i < NUM_WSA; i++)
    } // while (wsaTimeoutThreadRunning)

    shm_rsk_ptr->bootupState &= ~0x200; // wsaTimeoutThread running
    pthread_exit(NULL);
    return NULL;
}

/**
** Function: initWSAStatusTable
** @brief  Initialize the WSAStatusTable
** @return void
**
** Details: Initializes the semaphores in the WSAStatusTable and starts the WSA
** timeout thread.
**/
void initWSAStatusTable(void)
{
    int i;


    /* Initialize the WSAStatusTable entry semaphores */
    for (i = 0; i < NUM_WSA; i++) {
        sem_init(&WSAStatusTable[i].WSAStatusTableSem, 0, 1);
        sem_init(&WSAStatusTable[i].WSAVerifySem, 0, 1);
    }

    /* Create the WSA timeout thread */
    if (pthread_create(&wsaTimeoutThreadID, NULL,
                       wsaTimeoutThread, NULL) != 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error creating WSA timeout thread: %s\n", strerror(errno));
    }
}

/**
** Function deinitWSAStatusTable
** @brief  De-initialize the WSAStatusTable
** @return void
**
** The function kills the WSA timeout thread and destroys the semaphores.
**/
void deinitWSAStatusTable(void)
{
    int i;


    /* Kill the WSA timeout thread */
    wsaTimeoutThreadRunning = FALSE;
    pthread_join(wsaTimeoutThreadID, NULL);

    /* Destroy the WSAStatusTable entry semaphores */
    for (i = 0; i < NUM_WSA; i++) {
        sem_destroy(&WSAStatusTable[i].WSAStatusTableSem);
        sem_destroy(&WSAStatusTable[i].WSAVerifySem);
    }
}

/**
** Function: findAvailableWSAStatusTableEntry
** @brief  Find an entry in the WSAStatusTable for this WSA.
** @param mac           -- [input]The MAC address the WSA was received on.
** @param wsaNum        -- [output]Pointer to a uint8_t. The WSAStatusTable
**                         entry number is stored into that uint8_t if
**                         successful.
** @param newTableEntry -- [output]Pointer to a bool_t. This boolean is set to
**                         TRUE if we allocate a new entry in the table for
**                         this WSA. It is set to FALSE if we use an existing
**                         entry that was already allocated for the WSA.
**
** Returns: RS_SUCCESS if successful.
**          RS_ENOWBSS if all radios are in provider mode (implying no one can
**                     be listening for WSA's) or no free entry in the table.
**
** Details: The function finds an entry in the WSAStatusTable for the specified
**          MAC address. It first makes sure that not all radios are in
**          provider mode. It then looks to see if a WSAStatusTable entry
**          matching the specified MAC address is already allocated; that
**          WSAStatusTable entry number is put into *wsaNum if so. Otherwise,
**          it looks for an unused entry and puts the entry number for it into
**          *wsaNum if it finds one.
**/
static int32_t findAvailableWSAStatusTableEntry(uint8_t *mac, uint8_t *wsaNum,
                                                bool_t *newTableEntry)
{
    uint8_t i            = 0;
    uint8_t numProviders = 0;
    uint8_t my_mac[LENGTH_MAC];

    *newTableEntry = FALSE;
    memset(my_mac,0xff,LENGTH_MAC);
    mac = mac;

    /* Get the number of provider services */
    for (i = 0; i < MAX_RADIOS; i++) {
        if (!shm_rsk_ptr->wsuNsCurService[RT_CV2X][i].UserService &&
            (shm_rsk_ptr->wsuNsCurService[RT_CV2X][i].Pid != 0)) {
            numProviders++;
        }
    }

    if (numProviders == MAX_RADIOS) {
        /* All the radios are in provider mode; can't receive WSAStatusTable!! */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Provider service will not receive WSAStatusTable\n");
        return RS_ENOWBSS;
    }

    /* See if this WSA is already in the table */
    for (i = 0; i < NUM_WSA; i++) {
        if ((WSAStatusTable[i].wsaStatus != WSA_NO) &&
            (memcmp(WSAStatusTable[i].peer_mac_addr, my_mac, LENGTH_MAC) == 0)) {
            *wsaNum = i;
            break;
        }
    }

    /* If not already in the table, look for an empty entry */
    if (i == NUM_WSA) {
        // assign not active one for WSAStatusTable
        for (i = 0; i < NUM_WSA; i++) {
            if (WSAStatusTable[i].wsaStatus == WSA_NO) {
                WSAStatusTable[i].wsaStatus = WSA_INACTIVE;
                *wsaNum = i;
                *newTableEntry = TRUE;
                break;
            }
        }
    }

    /* Check to see if table was full */
    if (i == NUM_WSA) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No free WSAStatusTable\n");
        return RS_ENOWBSS;
    }
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning RS_SUCCESS\n");
#endif
    return RS_SUCCESS;
}

/**
** Function:  processWSMP
** @brief Process a WSMP packet
** @param  wsm             -- [input]Points to the WSMP header in the packet.
** @param  wsmLength       -- [input]The length of the WSMP packet.
** @param  dataRateFromHdr -- [input]If TRUE, get data rate from the WSMP
**                            header. If FALSE, get data rate from the radio.
** @param  RadioType       -- [input]RT_CV2X or RT_DSRC
** @param  wst_out         -- [output]The result of getWsmStatusEntry() is
**                            written to here.
** @param  inWsm           -- [output]The WsmStatusEntryType structure pointer
**                            to by inWSM is filled out. This will NOT include
**                            the dataLength and data fields.
** @param  wsmpHdrLen_out  -- [output]The length of the WSMP reader is written
**                            to here.
** @return RS_SUCCESS for success; error code otherwise
**
** Details: If we have extension elements, process them and store their values
**          in pwr, channel, and dataRate; leave these variables at their
**          default values of 0xff if the corresponding extension elements are
**          not present. Ignore any unknown extension elements.
**
**          Then parse the TPID and its extension elements. Only a TPID of
**          TPID_PSID is supported; otherwise, return a result of RS_UNKNTPID.
**          Ignore the contents of any extension elements.
**
**          Then process the PSID. Return a result of RS_PSIDENCERR is a PSID
**          encoding error is seen.
**
**          Then get the WST entry corresponding to the PSID. Return a result
**          of RS_ENOSRVC if the PSID is not registered. Write the address of
**          the WST entry to wst_out.
**
**          Finally, decode the data length, fill out the inWSM structure
**          fields that we can (including the WSM data itself), and return a
**          result of RS_SUCCESS.
**
**          If any error not mentioned above is discovered, return a result of
**          RS_RS_EWSMPHDRERR.
**/
static rsResultCodeType processWSMP(uint8_t             *wsm,             /* Input */
                                    int                  wsmLength,       /* Input */
                                    bool_t               dataRateFromHdr, /* Input */
                                    rsRadioType          RadioType,       /* Input */
                                    WsmStatusEntryType **wst_out,         /* Output */
                                    inWSMType           *inWSM,           /* Output */
                                    uint8_t             *wsmpHdrLen_out)  /* Output */
{
    uint8_t            *wsmstart = wsm;
    uint8_t            *dataEnd  = wsm + wsmLength;
    uint8_t             elementCount;
    int                 i;
    uint8_t             pwr      = 0xff;
    uint8_t             channel  = 0xff;
    uint8_t             dataRate = 0xff;
    uint8_t             len;
    uint32_t            PSID;
    uint8_t             psidLen;
    int16_t             dataLength;
    WsmStatusEntryType *wst;
    uint8_t             wsmpHdrLen;
    int32_t             ret = RS_SUCCESS;

    /* See if we have optional wave element extensions */
    if (*wsm & WSM_WSMP_N_EXTENSIONS) {
        wsm++;
        elementCount = *wsm++;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Have %u WSMP extensions\n", elementCount);
#endif
        for (i = 0; i < elementCount; i++) {
            switch (*wsm) {
            case WAVE_ID_TRANSMIT_POWER:
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WAVE_ID_TRANSMIT_POWER\n");
#endif
                pwr = wsm[2];
                /* Change the scaling from 0..255 to -128..127 */
                pwr -= 128;
                wsm += sizeof(WsmExtType);
                break;

            case WAVE_ID_CHANNEL_NUMBER:
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WAVE_ID_CHANNEL_NUMBER\n");
#endif
                channel = wsm[2];
                wsm += sizeof(WsmExtType);
                break;

            case WAVE_ID_DATARATE:
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WAVE_ID_DATARATE\n");
#endif
                /* NOTE: This dataRate is currently only used with C-V2X
                 * communication (i.e. when dataRateFromHdr is set to TRUE). It
                 * is not used with DSRC communication (i.e. when
                 * dataRateFromHdr is set to FALSE). */
                dataRate = wsm[2];
                wsm += sizeof(WsmExtType);
                break;

            default:
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unknown element ID %u\n", *wsm);
#endif
                shm_rsk_ptr->WSMError.WSMUnknownElementId++;

                /* Unsupported extension field; get its length. */
                if (wsm[1] & 0x80) {
                    len = ((wsm[1] & 0x7F) << 8) + wsm[2];
                    /* Skip past the length, plus 3 bytes for the extension
                     * field ID and the 2-byte length itself. */
                    wsm += len + 3;
                }
                else {
                    len = wsm[1];
                    /* Skip past the length, plus 2 bytes for the extension
                     * field ID and the 1-byte length itself. */
                    wsm += len + 2;
                }

                break;
            } // switch (*wsm)

            /* If we increment the pointer too far, drop the packet. */
            if (wsm >= dataEnd) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Premature end of packet: wsm = %p; dataEnd = %p\n",
                        wsm, dataEnd);
                shm_rsk_ptr->WSMError.WSMPrematureEndOfPacket++;
                return RS_EWSMPHDRERR;
            }
        } // for (i = 0; i < elementCount; i++)
    } // (*wsm & WSM_WSMP_N_EXTENSIONS)
    else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No WSMP extension elements\n");
#endif
        /* No optional extension elements; just skip past version byte */
        wsm++;
    }

    /* Only TPID values of 0 and 1 are currently supported */
    if ((*wsm & TPID_MASK) != TPID_PSID) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unknown TPID %u\n", *wsm & TPID_MASK);
        /* If unknown TPID, drop the packet */
        shm_rsk_ptr->WSMError.WSMUnknownTPID++;
        return RS_UNKNTPID;
    }

    if (*wsm & TPID_EXTENSION_ELEMENTS_PRESENT) {
        /* Skip past the TPID and get the number of extensions */
        wsm++;
        elementCount = *wsm++;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Have %u TPID extensions\n", elementCount);
#endif
        /* All extension elements unsupported; skip past them */
        for (i = 0; i < elementCount; i++) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Skip past TPID element %u\n", *wsm);
#endif
            /* Skip past this field */
            if (wsm[1] & 0x80) {
                len = ((wsm[1] & 0x7F) << 8) + wsm[2];
                /* Skip past the length, plus 3 bytes for the extension
                 * field ID and the 2-byte length itself. */
                wsm += len + 3;
            }
            else {
                len = wsm[1];
                /* Skip past the length, plus 2 bytes for the extension
                 * field ID and the 1-byte length itself. */
                wsm += len + 2;
            }

            /* If we increment the pointer too far, drop the packet. */
            if (wsm >= dataEnd) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Premature end of packet: wsm = %p; dataEnd = %p\n",
                        wsm, dataEnd);
                shm_rsk_ptr->WSMError.WSMPrematureEndOfPacket++;
                return RS_EWSMPHDRERR;
            }
        }
    }
    else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No TPID extension elements\n");
#endif
        /* Just skip past the TPID */
        wsm++;
    }

    /* Process the PSID */
    if ((*wsm & 0x80) == PSID_ENCODE_MASK_1BYTE) {
        PSID = wsm[0];
        psidLen = 1;
    } else if ((*wsm & 0xC0) == PSID_ENCODE_MASK_2BYTE) {
        PSID = (wsm[0] << 8)  + wsm[1];
        psidLen = 2;
    } else if ((*wsm & 0xE0) == PSID_ENCODE_MASK_3BYTE) {
        PSID = (wsm[0] << 16) + (wsm[1] << 8)  + wsm[2];
        psidLen = 3;
    } else if ((*wsm & 0xF0) == PSID_ENCODE_MASK_4BYTE) {
        PSID = (wsm[0] << 24) + (wsm[1] << 16) + (wsm[2] << 8) + wsm[3];
        psidLen = 4;
    } else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Invalid PSID: %02x %02x %02x %02x\n", wsm[0], wsm[1], wsm[2], wsm[3]);
#endif
        shm_rsk_ptr->WSMError.PSIDEncodingFailure++;
        return RS_PSIDENCERR;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSID = 0x%x; psidLen = %u\n", PSID, psidLen);
#endif
    wsm += psidLen;

    if ((PSID == WSM_WSA_PSID) || (PSID == WSM_P2P_PSID)) {
        /* Allow the WSA or the P2P packet through, even though its PSID won't
         * be registered, so there will be no wst associated with it */
        *wst_out = NULL;
    }
    else {
        /* Be sure the PSID is registered */
        wst = getWsmStatusEntry(RadioType, PSID);

        if (wst == NULL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSID 0x%x not registered\n", PSID);
#endif
            shm_rsk_ptr->WSMError.PSIDNotRegistered++;
            ret = RS_ENOSRVC; /* Let packet fall through. Return value will tell calling function to drop WSM. */
        } else {
            *wst_out = wst;
        }
    }

    /* Get the data length */
    if (*wsm <= 127) {
        dataLength = *wsm++;
    }
    else {
        dataLength = ((wsm[0] & 0x7F) << 8) + wsm[1];
        wsm += 2;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dataLength = %u\n", dataLength);
    wsmpHdrLen = wsm - wsmstart;
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmpHdrLen = %u\n", wsmpHdrLen);
#else
    wsmpHdrLen = wsm - wsmstart;
#endif
    if (wsmLength != wsmpHdrLen + dataLength) {
        RSU_PRINTF_MAX_N_TIMES(10, "wsmLength != wsmpHdrLen + dataLength\n");
        RSU_PRINTF_MAX_N_TIMES(10, "wsmpHdrLen = wsm(%p) - wsmstart(%p) = %d; "
                               "wsmpHdrLen + dataLength(%d) = %d; "
                               "wsmLength = %d\n",
                               wsm, wsmstart, wsmpHdrLen,
                               dataLength, wsmpHdrLen + dataLength,
                               wsmLength);
        shm_rsk_ptr->WSMError.WSMLengthMismatch++;
        return RS_EMATCH;
    }

    /* Fill out part of the inWSM structure */
    inWSM->psid = PSID;

    /* Only fill out the PSID for WSA or P2P */
    if ((PSID != WSM_WSA_PSID) && (PSID != WSM_P2P_PSID)) {
        inWSM->wsmVersion       = WSM_VERSION_3;
        inWSM->channelNumber    = channel;  /* WSM Tx Channel Number
                                             * (0xff = not available) */
        inWSM->txPwrLevel       = pwr;      /* WSM Tx Power Level
                                             * (0xff = not available) */

        /* Get the data rate from the WSMP header if desired. For now, the data
         * rate is gotten from the header if using C-V2X, from the radio if
         * using DSRC. */
        if (dataRateFromHdr) {
            inWSM->dataRate     = dataRate; /* WSM Tx Power Level
                                             * (0xff = not available) */
        }

        /* inWSM->recvWsmOTALength is the same as wsmLength */
        inWSM->recvWsmOTALength = wsmLength;
    }

    *wsmpHdrLen_out = wsmpHdrLen;
    return ret;
}

/**
** Function:  sendIndicationToRisByPid
** @brief  Sends an indication to RIS
** @param  pid  -- [input]The pid of the application to send the indication to
** @param  recv -- [input]The rsReceiveDataType to send as the message
** @return None.
**
** Details: Looks through the AST table to try to find a matching pid. If
**          found, calculates the RIS receiving port, and sends the data to RIS
**          via wsuSendData().
**/
void sendIndicationToRisByPid(pid_t pid, rsReceiveDataType *recv)
{
    int      i;
    uint16_t risRecvIndPort = 0;

    /* Determine which RIS receive port to send the packet to, based on the
     * pid */
    for (i = 0; i < MAX_APPS; i++) {
        if (AST.ASTEntry[i].pid == pid) {
            risRecvIndPort = RIS_RECV_IND_PORT1 + AST.ASTEntry[i].index;
        }
    }

    if (risRecvIndPort == 0) {
        shm_rsk_ptr->SendIndError.PIDNoMatchAST++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No matching pid found; drop the indication\n");
        return;
    }

    /* Send the indication to RIS */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sending indication to port %u\n", risRecvIndPort);
#endif
    if (!wsuSendData(risSocket, risRecvIndPort, recv, sizeof(*recv))) {
        shm_rsk_ptr->SendIndError.PIDSendFailed++;
        RSU_PRINTF_MAX_N_TIMES(10, "wsuSendData() failed\n");
        return;
    }

    shm_rsk_ptr->SendIndError.PIDOK++;
}

/**
** Function:  sendIndicationToRisByRadioTypeAndPsid
** @brief  Sends an indication to RIS
** @param  radioType -- [input]The radio type that is associated with this
**                      indication
** @param  psid      -- [input]The PSID that is associated with this indication
** @param  recv      -- [input]The rsReceiveDataType to send as the message
** @param  wsm       -- [input]TRUE if this is a WSM, FALSE otherwise
** @return None.
**
** Details: Looks through the WST table (for WSM) or GST table (for everything
**          else) to try to find a matching PSID (and radioType for GST). If
**          found, calculates the RIS receiving port, and sends the data to RIS
**          via wsuSendData().
**/
void sendIndicationToRisByRadioTypeAndPsid(rsRadioType radioType, uint32_t psid,
                                           rsReceiveDataType *recv, bool_t wsm)
{
    WsmStatusEntryType            *wst;
    GeneralServiceStatusEntryType *gst;
    uint16_t                       risRecvPort;

    /* Note that we must look through the WST table instead of the GST table
     * for WSM's. There may be separate Tx and Rx applications, and the Rx
     * application may not have a GST entry but still have a WST entry. */
    if (wsm) {
        wst = getWsmStatusEntry(radioType, psid);

        if (wst == NULL) {
            shm_rsk_ptr->SendIndError.RTPSIDNoMatchWST++;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No matching radioType/PSID found; drop the indication\n");
#endif
            return;
        }

        if (!wst->ASTEntry->receiveWSMInd) {
            shm_rsk_ptr->SendIndError.RTPSIDNoCBRegistered++;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No receive WSM callback; drop the indication\n");
#endif
            return;
        }

        risRecvPort = RIS_RECV_IND_PORT1 + wst->ASTEntry->index;
    }
    else {
        gst = getGeneralServiceStatusEntry(radioType, psid);

        if (gst == NULL) {
            shm_rsk_ptr->SendIndError.RTPSIDNoMatchGST++;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No matching radioType/PSID found; drop the indication\n");
#endif
            return;
        }

        risRecvPort = RIS_RECV_IND_PORT1 + gst->ASTEntry->index;
    }

    /* Send the indication to RIS */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sending indication to port %u\n", risRecvPort);
#endif
    if (!wsuSendData(risSocket, risRecvPort, recv, sizeof(*recv))) {
        shm_rsk_ptr->SendIndError.RTPSIDSendFailed++;
        RSU_PRINTF_MAX_N_TIMES(10, "wsuSendData() failed\n");
        return;
    }

    shm_rsk_ptr->SendIndError.RTPSIDOK++;
}

/**
** Function: compareReceivedWsa
** @brief  (Possibly) send a SERVICE_AVAILABLE indication to any applications
**         listening for WSA's whose PSID matches this WSA's PSID.
** @param  rpci      -- [input]The received RSS
** @param  sourceMSC -- [input]The source MAC address
** @param  wsaNum    -- [input]This WSA's entry number in the WSAStatusTable.
** @param  signedVerifiedFlags -- [input]32-bit bitfield. WSA_SIGNED is set in
**                                the field if the WSA has been signed.
**                                WSA_VERIFIED is set if the WSA has been
**                                verified.
** @return void
**
** Details:
** This function looks at the WSA just received, sees if there are applications
** listening for WSA's whose PSID is equal to this WSA's PSID, and (possibly)
** sends up a SERVICE_AVAILABLE indication to those applications.
**
** Normally, SERVICE_AVAILABLE indications are sent up only once a second per
** RSE so that the applications listening for WSA's are not bombarded with them
** (we normally receive 10 WSA's per second per PSID). However, if the WSA has
** just been verified by the security software (as opposed to a WSA that was
** received from an RSE) then the applications will always need to know about
** this, so bypass the once-per-seconds test in this case.
**
** Assuming we get past the once-per-second test, we compare the PSID of the
** WSA to all of the PSID's that are in the GST table to see if there are any
** matches. If there are (meaning some applications are registerd for the
** service that this WSA advertises) then the WSA is marked ACTIVE; otherwise,
** it is marked INACTIVE.
**
** Finally, if there are applications listening for this WSA, SERVICE_AVAILABLE
** indications are build and sent up to the applications.
**/
static void compareReceivedWsa(int8_t rss, uint8_t *srcMacAddress, uint8_t wsaNum,
                        uint32_t signedVerifiedFlags)
{
    struct timeval                 tv;
    bool_t                         sendServiceAvailableInd = FALSE;
    uint8_t                        ii, jj;
    bool_t                         active = FALSE;
    GeneralServiceStatusEntryType *gst;
    uint8_t my_mac[LENGTH_MAC];

    memset(my_mac,0xff,LENGTH_MAC);
    srcMacAddress = srcMacAddress;

    /* If the WSA is signed and verified, always send it up. Don't do the
     * once-a-second logic. */
    if ((signedVerifiedFlags & (WSA_SIGNED | WSA_VERIFIED)) != (WSA_SIGNED | WSA_VERIFIED)) {

        if (sem_wait(&WSAStatusTable[wsaNum].WSAStatusTableSem) < 0) {
            shm_rsk_ptr->WSACnt.WSASemaphoreError++;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Taking WSAStatusTable semaphore interrupted\n");
            return;
        }

        gettimeofday(&tv, NULL);

        /* by using a "!=" comparison, this will adjust the timestamp to the
         * past for playback of SR recordings or the csv_player */
        if (tv.tv_sec != (int32_t)WSAStatusTable[wsaNum].serviceAvailableTimestamp) {
            WSAStatusTable[wsaNum].serviceAvailableTimestamp = tv.tv_sec;
            sendServiceAvailableInd = TRUE;
        }

        if (sem_post(&WSAStatusTable[wsaNum].WSAStatusTableSem) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error posting to semaphore: %s\n", strerror(errno));
        }

        if (!sendServiceAvailableInd) {
            return;
        }
    }

    /* Determine whether or not WSA is active */
    for (ii = 0; ii < MAX_SERVICES; ii++) {
        if ((GST.GSTEntry[ii].ASTEntry != NULL)) {
            gst = &GST.GSTEntry[ii];

            if (gst->UserService) {
                for (jj = 0; jj < WSAStatusTable[wsaNum].serviceCount; jj++) {
                    if (WSAStatusTable[wsaNum].service[jj].psid == gst->PSID) {
                        active = TRUE;
                        break;
                    }
                }

                if (active) {
                    break;
                }
            }
        }
    }

    WSAStatusTable[wsaNum].wsaStatus = active ? WSA_ACTIVE : WSA_INACTIVE;

    /* report WSAStatusTable event back to applications */
    WSAStatusTable[wsaNum].rss = rss;
    memcpy(WSAStatusTable[wsaNum].peer_mac_addr, my_mac, LENGTH_MAC);
    sendServiceAvailableIndications(wsaNum, RS_SERVICE_AVAILABLE,signedVerifiedFlags);
}

/**
** Function: processWSAExtensionElements
** @brief  Processes a WSA's Extension Elements.
** @param  segment -- [input]Pointer to the extension elements.
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  wsaTableEntry  -- [output]Output from this function is written to
**                           this structure.
** @param  bytesProcessed -- [output]The number of bytes in the service info
**                           segment to be skipped is written here.
** @return 0 if successful.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if the above error
**         condition occurs.
**/
static int processWSAExtensionElements(const uint8_t *segment, uint16_t wsaLen,
                                       WSAStatusTableType *wsaTableEntry,
                                       uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint16_t       count;
    uint8_t        id;
    uint16_t       len1, len2;
    int            i;

    if (wsaLen == 0) {
        /* No room for element count */
        return -2;
    }

    /* Get WSA information extension element count */
    if (*wsa & 0x80) {
        if (wsaLen < 2) {
            /* No room for an extension indicator count */
            return -2;
        }

        count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
        wsa += 2; wsaLen -= 2;
    }
    else {
        count = *wsa++; wsaLen--;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("WSA extension count = %u\n", count);

    /* Process each WSA information extension element */
    for (i = 0; i < count; i++) {
        if (wsaLen < 3) {
            /* No room for an extension id, count, and value */
            return -4;
        }

        id = *wsa++;

        if (*wsa & 0x80) {
            len1 = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 3;
        }
        else {
            len1 = *wsa++; wsaLen -= 2;
        }

        if (wsaLen < len1) {
            /* No room for the value */
            return -5;
        }

        switch(id) {
        case WAVE_ID_REPEAT_RATE:
            wsaTableEntry->WSABitmask |= WSA_EXT_REPEAT_RATE_BITMASK;
            wsaTableEntry->repeatRate = *wsa;
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; repeatRate = %u\n",
                                            "WAVE_ID_REPEAT_RATE",
                                            wsaTableEntry->repeatRate);
            break;

        case WAVE_ID_2DLOCATION:
            wsaTableEntry->WSABitmask |= WSA_EXT_2DLOCATION_BITMASK;
            wsaTableEntry->wsa2d.latitude =
                (*(wsa + 0) << 24) + (*(wsa + 1) << 16) +
                (*(wsa + 2) <<  8) +  *(wsa + 3);
            wsaTableEntry->wsa2d.longitude =
                (*(wsa + 4) << 24) + (*(wsa + 5) << 16) +
                (*(wsa + 6) <<  8) +  *(wsa + 7);
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; latitude = %d; longitude = %d\n",
                "WAVE_ID_2DLOCATION", wsaTableEntry->wsa2d.latitude,
                wsaTableEntry->wsa2d.longitude);
            break;

        case WAVE_ID_3DLOCATION:
            wsaTableEntry->WSABitmask |= WSA_EXT_3DLOCATION_BITMASK;
            /* latitude, longitude, and elevation are UPER encoded. Minimum
             * unencoded values for latitude, longitude, and elevation are
             * -900000000, -1799999999, and -4096 respectively. */
            wsaTableEntry->wsa3d.latitude =
                (*(wsa + 0) << 24) + (*(wsa + 1) << 16) +
                (*(wsa + 2) <<  8) +  *(wsa + 3) - 900000000;
            wsaTableEntry->wsa3d.longitude =
                (*(wsa + 4) << 24) + (*(wsa + 5) << 16) +
                (*(wsa + 6) <<  8) +  *(wsa + 7) - 1799999999;
            wsaTableEntry->wsa3d.elevation =
                (*(wsa + 8) <<  8) +  *(wsa + 9) - 4096;
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; latitude = %d; longitude = %d; elevation = %d\n",
                "WAVE_ID_3DLOCATION", wsaTableEntry->wsa3d.latitude,
                wsaTableEntry->wsa3d.longitude, wsaTableEntry->wsa3d.elevation);
            break;

        case WAVE_ID_ADVERTISER_ID:
            /* There is yet another length after the first length */
            if (*wsa & 0x80) {
                len2 = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            }
            else {
                len2 = *wsa;
            }

            if (len2 >= len1) {
                return -6;
            }

            if (len2 > WSA_ADVERTISE_ID_LEN) {
                len2 = WSA_ADVERTISE_ID_LEN;
            }

            wsaTableEntry->WSABitmask |= WSA_EXT_ADVERTISER_ID_BITMASK;
            memcpy(wsaTableEntry->advertiseId, (*wsa & 0x80) ? wsa + 2 : wsa + 1, len2);
            wsaTableEntry->lengthAdvertiseId = len2;
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; lengthAdvertiseId = %d; AdvertiseId = \"%*s\"\n",
                "WAVE_ID_ADVERTISER_ID", wsaTableEntry->lengthAdvertiseId,
                wsaTableEntry->lengthAdvertiseId, wsaTableEntry->advertiseId);
            break;
        } // switch(id)

        wsa += len1; wsaLen -= len1;
    } // for (i = 0; i < count; i++)

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("WSABitmask = 0x%x\n",
                                    wsaTableEntry->WSABitmask);
    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: skipServiceInfoSegment
** @brief  Skips over a service info segment in a WSA.
** @param  segment -- [input]Pointer to the segment to be skipped
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  bytesProcessed -- [output]The number of bytes in the service info
**                           segment to be skipped is written here.
** @return 0 if successful.
**         -1 if PSID error.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if either of the above 2
**         error conditions occur.
**/
static int skipServiceInfoSegment(const uint8_t *segment, uint16_t wsaLen,
                                  uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint8_t        psid_highbyte;
    uint16_t       count;
    uint16_t       len;
    int            i;

    /* Skip over the PSID */
    psid_highbyte = *wsa;

    if ((psid_highbyte & 0xF0) == PSID_ENCODE_MASK_4BYTE) {
        if (wsaLen < 5) {
            /* No room for PSID + channel index */
            return -7;
        }

        wsa += 4; wsaLen -= 4;
    } else if ((psid_highbyte & 0xE0) == PSID_ENCODE_MASK_3BYTE) {
        if (wsaLen < 4) {
            /* No room for PSID + channel index */
            return -8;
        }

        wsa += 3; wsaLen -= 3;
    } else if ((psid_highbyte & 0xC0) == PSID_ENCODE_MASK_2BYTE) {
        if (wsaLen < 3) {
            /* No room for PSID + channel index */
            return -9;
        }
        wsa += 2; wsaLen -= 2;
    } else if ((psid_highbyte & 0x80) == PSID_ENCODE_MASK_1BYTE) {
        if (wsaLen < 2) {
            /* No room for PSID + channel index */
            return -10;
        }
        wsa++; wsaLen--;
    } else {
        return -1;
    }

    /* Skip over the service info options */
    if (*wsa & WSA_SERVICE_INFO_OPTION_PRESENT) {
        /* Skip over channel index */
        wsa++; wsaLen--;

        /* Get service info option count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a service info option count */
                return -11;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            if (wsaLen == 0) {
                /* No room for a service info option count */
                return -12;
            }

            count = *wsa++; wsaLen--;
        }

        for (i = 0; i < count; i++) {
            if (wsaLen < 3) {
                /* No room for an extension id, count, and value */
                return -48;
            }

            /* Skip over ID */
            wsa++;

            /* Get length */
            if (*wsa & 0x80) {
                len = ((*wsa & 0x7F) << 8) + *(wsa + 1);
                wsa += 2; wsaLen -= 3;
            }
            else {
                len = *wsa++; wsaLen -= 2;
            }

            if (wsaLen < len) {
                /* No room for the value */
                return -13;
            }

            /* Skip over the data portion */
            wsa += len; wsaLen -= len;
        }
    }
    else {
        /* Just skip over channel index */
        wsa++; wsaLen--;
    }

    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: processServiceInfoSegment
** @brief  Processes a service info segment in a WSA.
** @param  segment -- [input]Pointer to the segment to be processed
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  signedVerifiedFlags -- [input]32-bit bitfield. WSA_SIGNED is set in
**                                the field if the WSA has been signed.
**                                WSA_VERIFIED is set if the WSA has been
**                                verified.
** @param  wsaTableEntry  -- [output]Output from this function is written to
**                           this structure.
** @param  bytesProcessed -- [output]The number of bytes in the service info
**                           segment is written here.
** @return 0 if successful.
**         -1 if PSID error.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if either of the above 2
**         error conditions occur.
**/
static int processServiceInfoSegment(const uint8_t *segment, uint16_t wsaLen,
                                     uint32_t signedVerifiedFlags,
                                     WSAStatusTableType *wsaTableEntry,
                                     uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint8_t        serviceCount = wsaTableEntry->serviceCount;
    uint8_t        psid_highbyte;
    uint16_t       count;
    int            i;
    uint8_t        id;
    uint16_t       len1, len2;

    /* Get the PSID */
    psid_highbyte = *wsa;

    if ((psid_highbyte & 0xF0) == PSID_ENCODE_MASK_4BYTE) {
        if (wsaLen < 5) {
            /* No room for PSID + channel index */
            return -15;
        }

        wsaTableEntry->service[serviceCount].psid =
            (*(wsa + 0) << 24) + (*(wsa + 1) << 16) +
            (*(wsa + 2) <<  8) +  *(wsa + 3);
        wsa += 4; wsaLen -= 4;
    } else if ((psid_highbyte & 0xE0) == PSID_ENCODE_MASK_3BYTE) {
        if (wsaLen < 4) {
            /* No room for PSID + channel index */
            return -15;
        }

        wsaTableEntry->service[serviceCount].psid =
            (*(wsa + 0) << 16) + (*(wsa + 1) << 8) +
             *(wsa + 2);
        wsa += 3; wsaLen -= 3;
    } else if ((psid_highbyte & 0xC0) == PSID_ENCODE_MASK_2BYTE) {
        if (wsaLen < 3) {
            /* No room for PSID + channel index */
            return -16;
        }

        wsaTableEntry->service[serviceCount].psid = (*(wsa + 0) << 8) + *(wsa + 1);
        wsa += 2; wsaLen -= 2;
    } else if ((psid_highbyte & 0x80) == PSID_ENCODE_MASK_1BYTE) {
        if (wsaLen < 2) {
            /* No room for PSID + channel index */
            return -17;
        }

        wsaTableEntry->service[serviceCount].psid = *wsa++;
        wsaLen--;
    } else {
        return -1;
    }

    wsaTableEntry->service[serviceCount].channelIndex = (*wsa & WSA_CHANNEL_INDEX_MASK) >> 3;
    wsaTableEntry->serviceBitmask[serviceCount] = signedVerifiedFlags;
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("service[%d].psid = 0x%x; channelIndex = %u\n",
        serviceCount, wsaTableEntry->service[serviceCount].psid,
        wsaTableEntry->service[serviceCount].channelIndex);

    /* Process the service info options */
    if (*wsa & WSA_SERVICE_INFO_OPTION_PRESENT) {
        /* Skip over channel index/service option info indicator */
        wsa++; wsaLen--;

        if (wsaLen == 0) {
            /* No room for a service info option count */
            return -19;
        }

        /* Get service info option count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a service info option count */
                return -18;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            count = *wsa++; wsaLen--;
        }

        DEBUG_PARSE_RECEIVED_WSA_PRINTF("Service info extension count = %u\n",
                                        count);

        for (i = 0; i < count; i++) {
            if (wsaLen < 3) {
                /* No room for an extension id, count, and value */
                return -49;
            }

            /* Process this service info option */
            id = *wsa++;

            if (*wsa & 0x80) {
                len1 = ((*wsa & 0x7F) << 8) + *(wsa + 1);
                wsa += 2; wsaLen -= 3;
            }
            else {
                len1 = *wsa++; wsaLen -= 2;
            }

            if (wsaLen < len1) {
                /* No room for the value */
                return -20;
            }

            switch(id) {
            case WAVE_ID_PROVIDER_SERVICE_CONTEXT:
                /* There is yet another length after the first length */
                if (*wsa & 0x80) {
                    len2 = ((*wsa & 0x7F) << 8) + *(wsa + 1);
                }
                else {
                    len2 = *wsa;
                }

                if (len2 >= len1) {
                    return -21;
                }

                if (len2 > LENGTH_PSC) {
                    len2 = LENGTH_PSC;
                }

                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK;
                memcpy(wsaTableEntry->service[serviceCount].psc, (*wsa & 0x80) ? wsa + 2 : wsa + 1, len2);
                wsaTableEntry->servicePscLength[serviceCount] = len2;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; psc = \"%*s\"\n",
                    "WAVE_ID_PROVIDER_SERVICE_CONTEXT",
                    wsaTableEntry->servicePscLength[serviceCount],
                    wsaTableEntry->service[serviceCount].psc);
                break;

            case WAVE_ID_IPV6ADDRESS:
                len2 = (len1 <= LENGTH_IPV6) ? len1 : LENGTH_IPV6;
                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_IPV6ADDRESS_BITMASK;
                memcpy(wsaTableEntry->service[serviceCount].ipv6Address, wsa, len2);
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; ipv6Address = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                    "WAVE_ID_IPV6ADDRESS",
                    wsaTableEntry->service[serviceCount].ipv6Address[0],
                    wsaTableEntry->service[serviceCount].ipv6Address[1],
                    wsaTableEntry->service[serviceCount].ipv6Address[2],
                    wsaTableEntry->service[serviceCount].ipv6Address[3],
                    wsaTableEntry->service[serviceCount].ipv6Address[4],
                    wsaTableEntry->service[serviceCount].ipv6Address[5],
                    wsaTableEntry->service[serviceCount].ipv6Address[6],
                    wsaTableEntry->service[serviceCount].ipv6Address[7],
                    wsaTableEntry->service[serviceCount].ipv6Address[8],
                    wsaTableEntry->service[serviceCount].ipv6Address[9],
                    wsaTableEntry->service[serviceCount].ipv6Address[10],
                    wsaTableEntry->service[serviceCount].ipv6Address[11],
                    wsaTableEntry->service[serviceCount].ipv6Address[12],
                    wsaTableEntry->service[serviceCount].ipv6Address[13],
                    wsaTableEntry->service[serviceCount].ipv6Address[14],
                    wsaTableEntry->service[serviceCount].ipv6Address[15]);
                break;

            case WAVE_ID_SERVICE_PORT:
                if (len1 > 2) {
                    return -22;
                }

                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_SERVICE_PORT_BITMASK;
                wsaTableEntry->service[serviceCount].servicePort =
                    (len1 == 2) ? (*wsa << 8) + *(wsa + 1) :
                    (len1 == 1) ? *wsa : 0;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; port = %u\n",
                    "WAVE_ID_SERVICE_PORT",
                    wsaTableEntry->service[serviceCount].servicePort);
                break;

            case WAVE_ID_PROVIDER_MACADDRESS:
                len2 = (len1 <= LENGTH_MAC) ? len1 : LENGTH_MAC;
                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_PROVIDER_MACADDRESS_BITMASK;
                memcpy(wsaTableEntry->service[serviceCount].providerMacAddress, wsa, len2);
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; mac = %02x:%02x:%02x:%02x:%02x:%02x\n", "WAVE_ID_PROVIDER_MACADDRESS",
                    wsaTableEntry->service[serviceCount].providerMacAddress[0],
                    wsaTableEntry->service[serviceCount].providerMacAddress[1],
                    wsaTableEntry->service[serviceCount].providerMacAddress[2],
                    wsaTableEntry->service[serviceCount].providerMacAddress[3],
                    wsaTableEntry->service[serviceCount].providerMacAddress[4],
                    wsaTableEntry->service[serviceCount].providerMacAddress[5]);
                break;

            case WAVE_ID_RCPI_THRESHOLD:
                if (len1 > 1) {
                    return -23;
                }

                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_RCPI_THRESHOLD_BITMASK;
                wsaTableEntry->service[serviceCount].rcpi = (len1 == 1) ? (int)(*wsa) : 0;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; rcpi = %d\n",
                    "WAVE_ID_RCPI_THRESHOLD",
                    wsaTableEntry->service[serviceCount].rcpi);
                break;

            case WAVE_ID_WSA_COUNT_THRESHOLD:
                if (len1 > 1) {
                    return -24;
                }

                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK;
                wsaTableEntry->service[serviceCount].countThreshold = (len1 == 1) ? (int)(*wsa) : 0;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; wsaCount.wsaCountThreshold = %u\n",
                    "WAVE_ID_WSA_COUNT_THRESHOLD",
                    wsaTableEntry->service[serviceCount].countThreshold);
                break;

            case WAVE_ID_WSA_COUNT_THRESHOLD_INTERVAL:
                if (len1 > 1) {
                    return -25;
                }

                wsaTableEntry->serviceBitmask[serviceCount] |= WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK;
                wsaTableEntry->service[serviceCount].countThresholdInterval = (len1 == 1) ? (int)(*wsa) : 0;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; wsaCountInternal.wsaCountThreshold = %u\n",
                    "WAVE_ID_WSA_COUNT_THRESHOLD_INTERVAL",
                    wsaTableEntry->service[serviceCount].countThresholdInterval);
                break;
            } // switch(id)

            wsa += len1; wsaLen -= len1;
        } // for (i = 0; i < count; i++)
    } // if (*wsa & WSA_SERVICE_INFO_OPTION_PRESENT)
    else {
        /* Just skip over channel index/service option info indicator */
        wsa++; wsaLen--;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("serviceBitmask[%d] = 0x%x\n",
        serviceCount,
        wsaTableEntry->serviceBitmask[serviceCount]);
    wsaTableEntry->serviceCount++;
    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: skipChannelInfoSegment
** @brief  Skips over a channel info segment in a WSA.
** @param  segment -- [input]Pointer to the segment to be skipped
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  bytesProcessed -- [output]The number of bytes in the channel info
**                           segment to be skipped is written here.
** @return 0 if successful.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if the above error
**         condition occurs.
**/
static int skipChannelInfoSegment(const uint8_t *segment, uint16_t wsaLen,
                                  uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint16_t       count;
    uint16_t       len;
    int            i;

    /* Must be room for operating class, channel number, transmit power,
     * adaptable/data rate, and channel info options */
    if (wsaLen < 5) {
        return -26;
    }

    /* Skip over the operating class, channel number, transmit power,
     * and adaptable/data rate */
    wsa += 4; wsaLen -= 4;

    /* Skip over the channel info options */
    if (*wsa & WSA_CHANNEL_INFO_OPTION_PRESENT) {
        /* Skip over channel info option indicator */
        wsa++; wsaLen--;

        if (wsaLen == 0) {
            /* No room for a channel info option count */
            return -29;
        }

        /* Get channel info option count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a channel info option count */
                return -28;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            count = *wsa++; wsaLen--;
        }

        for (i = 0; i < count; i++) {
            if (wsaLen < 3) {
                /* No room for an extension id, count, and value */
                return -50;
            }

            /* Skip over ID */
            wsa++;

            /* Get length */
            if (*wsa & 0x80) {
                len = ((*wsa & 0x7F) << 8) + *(wsa + 1);
                wsa += 2; wsaLen -= 3;
            }
            else {
                len = *wsa++; wsaLen -= 2;
            }

            if (wsaLen < len) {
                /* No room for the value */
                return -30;
            }

            /* Skip over the data portion */
            wsa += len; wsaLen -= len;
        }
    }
    else {
        /* Just skip over channel info option indicator */
        wsa++; wsaLen--;
    }

    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: processChannelInfoSegment
** @brief  Processes a channel info segment in a WSA.
** @param  segment -- [input]Pointer to the segment to be processed
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  wsaTableEntry  -- [output]Output from this function is written to
**                           this structure.
** @param  bytesProcessed -- [output]The number of bytes in the channel info
**                           segment to be skipped is written here.
** @return 0 if successful.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if the above error
**         condition occurs.
**/
static int processChannelInfoSegment(const uint8_t *segment, uint16_t wsaLen,
                                     WSAStatusTableType *wsaTableEntry,
                                     uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint8_t        channelCount = wsaTableEntry->channelCount;
    uint16_t       count;
    int            i, j;
    uint8_t        id;
    uint16_t       len;
    int            aci;

    /* Process fixed portion of channel info segment */
    if (wsaLen < 5) {
        return -31;
    }

    wsaTableEntry->channelInfo[channelCount].operatingClass = *wsa++;
    wsaTableEntry->channelInfo[channelCount].channelNumber  = *wsa++;
    wsaTableEntry->channelInfo[channelCount].txPwrLevel     = *wsa++;
    /* Change the scaling from 0..255 to -128..127 */
    wsaTableEntry->channelInfo[channelCount].txPwrLevel    -= 128;
    wsaTableEntry->channelInfo[channelCount].adaptable      =
        (*wsa & WSA_CHANNEL_INFO_ADAPTABLE) ? 1 : 0;
    wsaTableEntry->channelInfo[channelCount].dataRate       =
        (*wsa++) & WSA_CHANNEL_INFO_DATA_RATE_MASK;
    wsaTableEntry->channelBitmask[channelCount]             = 0;
    wsaLen -= 4;
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("channelInfo[%u].operatingClass = %u; channelNumber = %u; txPwrLevel = %u; adaptable = %u; dataRate = %u\n",
        channelCount,
        wsaTableEntry->channelInfo[channelCount].operatingClass,
        wsaTableEntry->channelInfo[channelCount].channelNumber,
        wsaTableEntry->channelInfo[channelCount].txPwrLevel,
        wsaTableEntry->channelInfo[channelCount].adaptable,
        wsaTableEntry->channelInfo[channelCount].dataRate);

    /* Process the channel info options */
    if (*wsa & WSA_CHANNEL_INFO_OPTION_PRESENT) {
        /* Skip over channel info option indicator */
        wsa++; wsaLen--;

        if (wsaLen == 0) {
            /* No room for a service info option count */
            return -34;
        }

        /* Get service info option count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a service info option count */
                return -33;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            count = *wsa++; wsaLen--;
        }

        DEBUG_PARSE_RECEIVED_WSA_PRINTF("Channel info extension count = %u\n",
                                        count);

        for (i = 0; i < count; i++) {
            if (wsaLen < 3) {
                /* No room for an extension id, count, and value */
                return -51;
            }

            /* Process this service info option */
            id = *wsa++;

            if (*wsa & 0x80) {
                len = ((*wsa & 0x7F) << 8) + *(wsa + 1);
                wsa += 2; wsaLen -= 3;
            }
            else {
                len = *wsa++; wsaLen -= 2;
            }

            if (wsaLen < len) {
                /* No room for the value */
                return -35;
            }

            switch(id) {
            case WAVE_ID_EDCA_PARAM_SET:
                if (len != 16) {
                     return -36;
                }

                wsaTableEntry->channelBitmask[channelCount] |= WSA_EXT_EDCA_PARAM_SET_BITMASK;

                for (j = 0; j < 4; j++) {
                    aci = ((*(wsa + (j * 4))) & 0x60) >> 5;
                    wsaTableEntry->channelInfo[channelCount].edca.aifs[aci] = *(wsa + (j * 4));
                    wsaTableEntry->channelInfo[channelCount].edca.ecw[aci]  = *(wsa + 1 + (j * 4));
                    wsaTableEntry->channelInfo[channelCount].edca.txop[aci] = ((*(wsa + 2 + (j * 4))) << 8) + *(wsa + 3 + (j * 4));
                    DEBUG_PARSE_RECEIVED_WSA_PRINTF("%d. id = %s; aifs[%d] = %u; ecw[%d] = %u; txop[%d] = %u\n",
                        j, "WAVE_ID_EDCA_PARAM_SET",
                        aci, wsaTableEntry->channelInfo[channelCount].edca.aifs[aci],
                        aci, wsaTableEntry->channelInfo[channelCount].edca.ecw[aci],
                        aci, wsaTableEntry->channelInfo[channelCount].edca.txop[aci]);
                }

                break;

            case WAVE_ID_CHANNEL_ACCESS:
                if (len != 1) {
                    return -37;
                }

                wsaTableEntry->channelBitmask[channelCount] |= WSA_EXT_CHANNEL_ACCESS_BITMASK;
                wsaTableEntry->channelInfo[channelCount].channelAccess = *wsa;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; channelAccess = %u\n",
                    "WAVE_ID_CHANNEL_ACCESS",
                    wsaTableEntry->channelInfo[channelCount].channelAccess);

                if (*wsa > 2) {
                    return -53;
                }

                break;
            } // switch(id)

            wsa += len; wsaLen -= len;
        } // for (i = 0; i < count2; i++)
    } // if (*wsa & WSA_CHANNEL_INFO_OPTION_PRESENT)
    else {
        /* Just skip over channel info option indicator */
        wsa++; wsaLen--;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("wsaTableEntry->channelBitmask[%d] = 0x%x\n",
        channelCount,
        wsaTableEntry->channelBitmask[channelCount]);
    wsaTableEntry->channelCount++;
    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: processWaveRoutingAdvertisement
** @brief  Processes a wave routing advertisement in a WSA.
** @param  segment -- [input]Pointer to the segment to be processed
** @param  wsaLen  -- [input]The remaining length of the WSA starting at
**                    "segment".
** @param  wsaTableEntry  -- [output]Output from this function is written to
**                           this structure.
** @param  bytesProcessed -- [output] The number of bytes in the routing
**                           advertisement segment is written here.
** @return 0 if successful.
**         <0 if "WSA length invalid" error.
**         Processing of the WSA should be aborted if the above error
**         condition occurs.
**/
static int processWaveRoutingAdvertisement(const uint8_t *segment,
                                           uint16_t wsaLen,
                                           WSAStatusTableType *wsaTableEntry,
                                           uint16_t *bytesProcessed)
{
    const uint8_t *wsa = segment;
    uint16_t       count;
    int            i;
    uint8_t        id;
    uint16_t       len1, len2;

    /* Process the fixed segments */
    if (wsaLen < 2 + LENGTH_IPV6 + 1 + DEFAULT_GATEWAY_LEN + LENGTH_IPV6) {
        return -38;
    }

    wsaTableEntry->routingBitmask |= WSA_EXT_WRA_BITMASK;
    wsaTableEntry->routingInfo.routerLifeTime = (*wsa << 8) + *(wsa + 1); wsa += 2;
    memcpy(wsaTableEntry->routingInfo.ipPrefix, wsa, LENGTH_IPV6); wsa += LENGTH_IPV6;
    wsaTableEntry->routingInfo.prefixLen = *wsa++;
    memcpy(wsaTableEntry->routingInfo.defaultGateway, wsa, DEFAULT_GATEWAY_LEN); wsa += DEFAULT_GATEWAY_LEN;
    memcpy(wsaTableEntry->routingInfo.primaryDns, wsa, LENGTH_IPV6); wsa += LENGTH_IPV6;
    wsaLen -= 2 + LENGTH_IPV6 + 1 + DEFAULT_GATEWAY_LEN + LENGTH_IPV6;
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("routerLifeTime = %u; ipPrefix = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x; prefixLen = %u\n",
        wsaTableEntry->routingInfo.routerLifeTime,
        wsaTableEntry->routingInfo.ipPrefix[0],
        wsaTableEntry->routingInfo.ipPrefix[1],
        wsaTableEntry->routingInfo.ipPrefix[2],
        wsaTableEntry->routingInfo.ipPrefix[3],
        wsaTableEntry->routingInfo.ipPrefix[4],
        wsaTableEntry->routingInfo.ipPrefix[5],
        wsaTableEntry->routingInfo.ipPrefix[6],
        wsaTableEntry->routingInfo.ipPrefix[7],
        wsaTableEntry->routingInfo.ipPrefix[8],
        wsaTableEntry->routingInfo.ipPrefix[9],
        wsaTableEntry->routingInfo.ipPrefix[10],
        wsaTableEntry->routingInfo.ipPrefix[11],
        wsaTableEntry->routingInfo.ipPrefix[12],
        wsaTableEntry->routingInfo.ipPrefix[13],
        wsaTableEntry->routingInfo.ipPrefix[14],
        wsaTableEntry->routingInfo.ipPrefix[15],
        wsaTableEntry->routingInfo.prefixLen);
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("defaultGateway = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x; primaryDns = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
        wsaTableEntry->routingInfo.defaultGateway[0],
        wsaTableEntry->routingInfo.defaultGateway[1],
        wsaTableEntry->routingInfo.defaultGateway[2],
        wsaTableEntry->routingInfo.defaultGateway[3],
        wsaTableEntry->routingInfo.defaultGateway[4],
        wsaTableEntry->routingInfo.defaultGateway[5],
        wsaTableEntry->routingInfo.defaultGateway[6],
        wsaTableEntry->routingInfo.defaultGateway[7],
        wsaTableEntry->routingInfo.defaultGateway[8],
        wsaTableEntry->routingInfo.defaultGateway[9],
        wsaTableEntry->routingInfo.defaultGateway[10],
        wsaTableEntry->routingInfo.defaultGateway[11],
        wsaTableEntry->routingInfo.defaultGateway[12],
        wsaTableEntry->routingInfo.defaultGateway[13],
        wsaTableEntry->routingInfo.defaultGateway[14],
        wsaTableEntry->routingInfo.defaultGateway[15],
        wsaTableEntry->routingInfo.primaryDns[0],
        wsaTableEntry->routingInfo.primaryDns[1],
        wsaTableEntry->routingInfo.primaryDns[2],
        wsaTableEntry->routingInfo.primaryDns[3],
        wsaTableEntry->routingInfo.primaryDns[8],
        wsaTableEntry->routingInfo.primaryDns[5],
        wsaTableEntry->routingInfo.primaryDns[6],
        wsaTableEntry->routingInfo.primaryDns[7],
        wsaTableEntry->routingInfo.primaryDns[8],
        wsaTableEntry->routingInfo.primaryDns[9],
        wsaTableEntry->routingInfo.primaryDns[10],
        wsaTableEntry->routingInfo.primaryDns[11],
        wsaTableEntry->routingInfo.primaryDns[12],
        wsaTableEntry->routingInfo.primaryDns[13],
        wsaTableEntry->routingInfo.primaryDns[14],
        wsaTableEntry->routingInfo.primaryDns[15]);

    /* Process the WRA extension elements */
    if (wsaLen == 0) {
        /* No room for a service info option count */
        return -40;
    }

    /* Get WRA extension element count */
    if (*wsa & 0x80) {
        if (wsaLen < 2) {
            /* No room for a service info option count */
            return -39;
        }

        count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
        wsa += 2; wsaLen -= 2;
    }
    else {
        count = *wsa++; wsaLen--;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("WRA extension count = %u\n", count);

    for (i = 0; i < count; i++) {
        /* Process this service info option */
        if (wsaLen < 3) {
            /* No room for an extension id, count, and value */
            return -52;
        }


        id = *wsa++;

        if (*wsa & 0x80) {
            len1 = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 3;
        }
        else {
            len1 = *wsa++; wsaLen -= 2;
        }

        if (wsaLen < len1) {
            /* No room for the value */
            return -41;
        }

        switch(id) {
        case WAVE_ID_SECONDARY_DNS:
            len2 = (len1 <= LENGTH_IPV6) ? len1 : LENGTH_IPV6;
            wsaTableEntry->routingBitmask |= WSA_EXT_SECONDARY_DNS_BITMASK;
            memcpy(wsaTableEntry->routingInfo.secondaryDns, wsa, len2);
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; secondaryDns = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", "WAVE_ID_SECONDARY_DNS",
                wsaTableEntry->routingInfo.secondaryDns[0],
                wsaTableEntry->routingInfo.secondaryDns[1],
                wsaTableEntry->routingInfo.secondaryDns[2],
                wsaTableEntry->routingInfo.secondaryDns[3],
                wsaTableEntry->routingInfo.secondaryDns[8],
                wsaTableEntry->routingInfo.secondaryDns[5],
                wsaTableEntry->routingInfo.secondaryDns[6],
                wsaTableEntry->routingInfo.secondaryDns[7],
                wsaTableEntry->routingInfo.secondaryDns[8],
                wsaTableEntry->routingInfo.secondaryDns[9],
                wsaTableEntry->routingInfo.secondaryDns[10],
                wsaTableEntry->routingInfo.secondaryDns[11],
                wsaTableEntry->routingInfo.secondaryDns[12],
                wsaTableEntry->routingInfo.secondaryDns[13],
                wsaTableEntry->routingInfo.secondaryDns[14],
                wsaTableEntry->routingInfo.secondaryDns[15]);
            break;

        case WAVE_ID_GATEWAY_MACADDRESS:
            len2 = (len1 <= LENGTH_MAC) ? len1 : LENGTH_MAC;
            wsaTableEntry->routingBitmask |= WSA_EXT_GATEWAY_MACADDRESS_BITMASK;
            memcpy(wsaTableEntry->routingInfo.gatewayMacAddress, wsa, len2);
            DEBUG_PARSE_RECEIVED_WSA_PRINTF("id = %s; gatewayMacAddress = %02x:%02x:%02x:%02x:%02x:%02x\n",
                "WAVE_ID_GATEWAY_MACADDRESS",
                wsaTableEntry->routingInfo.gatewayMacAddress[0],
                wsaTableEntry->routingInfo.gatewayMacAddress[1],
                wsaTableEntry->routingInfo.gatewayMacAddress[2],
                wsaTableEntry->routingInfo.gatewayMacAddress[3],
                wsaTableEntry->routingInfo.gatewayMacAddress[4],
                wsaTableEntry->routingInfo.gatewayMacAddress[5]);
            break;
        }

        wsa += len1; wsaLen -= len1;
    } // for (i = 0; i < count; i++)

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("wsaTableEntry->routingBitmask = 0x%x\n",
                                    wsaTableEntry->routingBitmask);
    *bytesProcessed = wsa - segment;
    return 0;
}

/**
** Function: parseReceivedWSA
** @brief  Parses the contents of a WSA.
** @param  wsa    -- [input]The WSA we are parsing.
** @param  wsaLen -- [input]The length of the above data.
** @param  wsaNum -- [input]The entry number in the WSAStatusTable for this WSA,
**                   allocated by a call to the
**                   findAvailableWSAStatusTableEntry() function.
**                   That table entry will be populated with the contents of
**                   this WSA.
** @param  rss    -- [input]The rss of the received WSA.
** @param  mac    -- [input]The MAC address of the sender of the WSA.
** @param  signedVerifiedFlags -- [input]32-bit bitfield. WSA_SIGNED is set in
**                                the field if the WSA has been signed.
**                                WSA_VERIFIED is set if the WSA has been
**                                verified.
** @return RS_SUCCESS if successful.
**         RS_ENOROOM if the WSA is too short to be valid.
**         RS_ERANGE if the WSA length is invalid.
**         RS_EWRNGVER if the 1609.3 protocol version is invalid.
**         RS_EWRNGSTATE if the length of a WSA field would go past the end of
**                       the WSA.
**         RS_ENOSRVC if the PSID format is not valid.
**
** This function parses the contents of the WSA and stores the parsed version
** of the WSA in the WSAStatusTable. Numerous checks for errors are done, and a
** non-RS_SUCCESS value is returned if it sees one.
**/
static int parseReceivedWSA(const uint8_t *wsa, uint16_t wsaLen, uint8_t wsaNum,
    int8_t rss, uint8_t *mac, uint32_t signedVerifiedFlags)
{
    WSAStatusTableType *wsaTableEntry = &WSAStatusTable[wsaNum];
    uint8_t             opt_ind;
    uint16_t            count;
    int                 i;
    int                 lenErr;
    uint16_t            bytesProcessed = 0;
    uint8_t my_mac[LENGTH_MAC];

    memset(my_mac,0xff,LENGTH_MAC);
    mac = mac;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsaLen = %u; wsaNum = %u; rss = %d; "
            "mac = %02x:%02x:%02x:%02x:%02x:%02x; signedVerifiedFlags = 0x%x\n",
            wsaLen, wsaNum, rss, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            signedVerifiedFlags);
#endif
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("%s(%p, %u, %u, %d, %02x:%02x:%02x:%02x:%02x:%02x, %u)\n", __func__,
        wsa, wsaLen, wsaNum, rss, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], signedVerifiedFlags);

    /* We have a length of at least 2, since this was tested for in
     * processReceivedWSA() */

    /* Check version number and get option indicators */
    if ((*wsa & WSA_16093_PROTOCOL_VER_MASK) != WSA_16093_PROTOCOL_VER_3) {
        /* Wrong version */
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Fail - Wrong version\n");
#endif
        shm_rsk_ptr->WSACnt.WSAWrongVersion++;
        return RS_EWRNGVER;
    }

    /* Initialize these fields */
    wsaTableEntry->rss            = rss;
    memcpy(wsaTableEntry->peer_mac_addr, my_mac, LENGTH_MAC);
    wsaTableEntry->serviceCount   = 0;
    wsaTableEntry->channelCount   = 0;
    wsaTableEntry->WSABitmask     = 0;
    wsaTableEntry->routingBitmask = 0;

    opt_ind = *wsa++;

    /* Check to see if content count value has changed */
    if ((*wsa & WSA_CONTENT_COUNT_MASK) != wsaTableEntry->changeCount) {
        wsaTableEntry->versionChange = TRUE;
        wsaTableEntry->changeCount = *wsa & WSA_CONTENT_COUNT_MASK;
    }
    else {
        wsaTableEntry->versionChange = FALSE;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("rss = %d; peer_mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
        wsaTableEntry->rss, wsaTableEntry->peer_mac_addr[0], wsaTableEntry->peer_mac_addr[1],
        wsaTableEntry->peer_mac_addr[2], wsaTableEntry->peer_mac_addr[3],
        wsaTableEntry->peer_mac_addr[4], wsaTableEntry->peer_mac_addr[5]);
    wsa++; wsaLen -= 2;

    /* Process WSA information extension elements */
    if (opt_ind & WSA_INFORMATION_EXTENSION_ELEMENT_PRESENT) {
        lenErr = processWSAExtensionElements(wsa, wsaLen, wsaTableEntry,
                                             &bytesProcessed);
        DEBUG_PARSE_RECEIVED_WSA_PRINTF("processWSAExtensionElements(); wsaLen = %u; bytesProcessed = %u; lenErr = %d\n",
            wsaLen, bytesProcessed, lenErr);

        if (lenErr < 0) {
            goto WSA_length_invalid;
        }

        wsa += bytesProcessed; wsaLen -= bytesProcessed;
    }

    /* Process service info segments */
    if (opt_ind & WSA_SERVICE_INFO_SEGMENT_PRESENT) {
        if (wsaLen == 0) {
            /* No room for a service info segment count */
            lenErr = -44;
            goto WSA_length_invalid;
        }

        /* Get service info segment count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a service info segment count */
                lenErr = -43;
                goto WSA_length_invalid;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            count = *wsa++; wsaLen--;
        }

        DEBUG_PARSE_RECEIVED_WSA_PRINTF("Service info count = %u\n", count);

        /* Process each service info segment */
        for (i = 0; i < count; i++) {
            if (wsaTableEntry->serviceCount >= MAX_SERVICE_INFO) {
                /* Skip over the service info segment */
                shm_rsk_ptr->WSACnt.WSAServiceCountExceeded++;
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("skipServiceInfoSegment\n");
                lenErr = skipServiceInfoSegment(wsa, wsaLen, &bytesProcessed);
            }
            else {
                /* Process the service info segment */
                lenErr = processServiceInfoSegment(wsa, wsaLen,
                                                   signedVerifiedFlags,
                                                   wsaTableEntry,
                                                   &bytesProcessed);
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("processServiceInfoSegment(); wsaLen = %u; bytesProcessed = %u; lenErr = %d\n",
                    wsaLen, bytesProcessed, lenErr);
            }

            if (lenErr < 0) {
                if (lenErr == -1) {
                    goto psid_error;
                }

                goto WSA_length_invalid;
            }

            wsa += bytesProcessed; wsaLen -= bytesProcessed;
        } // for (i = 0; i < count; i++)
    } // if (opt_ind & WSA_SERVICE_INFO_SEGMENT_PRESENT)

    /* Process channel info segments */
    if (opt_ind & WSA_CHANNEL_INFO_SEGMENT_PRESENT) {
        if (wsaLen == 0) {
            /* No room for a channel info segment count */
            lenErr = -46;
            goto WSA_length_invalid;
        }

        /* Get channel info segment count */
        if (*wsa & 0x80) {
            if (wsaLen < 2) {
                /* No room for a channel info segment count */
                lenErr = -45;
                goto WSA_length_invalid;
            }

            count = ((*wsa & 0x7F) << 8) + *(wsa + 1);
            wsa += 2; wsaLen -= 2;
        }
        else {
            count = *wsa++; wsaLen--;
        }

        DEBUG_PARSE_RECEIVED_WSA_PRINTF("Channel info count = %u\n", count);

        /* Process each channel info segment */
        for (i = 0; i < count; i++) {
            if (wsaTableEntry->channelCount >= MAX_CHANNEL_INFO) {
                /* Skip over this channel info segment */
                shm_rsk_ptr->WSACnt.WSAChannelCountInvalid++;
                lenErr = skipChannelInfoSegment(wsa, wsaLen, &bytesProcessed);
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("skipChannelInfoSegment\n");

                if (lenErr < 0) {
                    shm_rsk_ptr->WSACnt.WSAChannelLengthMismatch++;
                    goto WSA_length_invalid;
                }
            }
            else {
                lenErr = processChannelInfoSegment(wsa, wsaLen, wsaTableEntry,
                                                   &bytesProcessed);
                DEBUG_PARSE_RECEIVED_WSA_PRINTF("processChannelInfoSegment(); wsaLen = %u; bytesProcessed = %u; lenErr = %d\n",
                    wsaLen, bytesProcessed, lenErr);

                if (lenErr == -53) {
                    /* lenErr of -53 indicates invalid channelAccess value */
                    goto WSA_channelAccess_invalid;
                }
                else if (lenErr < 0) {
                    goto WSA_length_invalid;
                }
            }

            wsa += bytesProcessed; wsaLen -= bytesProcessed;
        } // for (i = 0; i < count; i++)
    } // if (opt_ind & WSA_CHANNEL_INFO_SEGMENT_PRESENT)

    /* Process Wave Routing Advertisement */
    if (opt_ind & WSA_WAVE_ROUTING_ADVERTISEMENT_PRESENT) {
        lenErr = processWaveRoutingAdvertisement(wsa, wsaLen, wsaTableEntry,
                                                 &bytesProcessed);
        DEBUG_PARSE_RECEIVED_WSA_PRINTF("processWaveRoutingAdvertisement(); wsaLen = %u; bytesProcessed = %u; lenErr = %d\n",
            wsaLen, bytesProcessed, lenErr);

        if (lenErr < 0) {
            goto WSA_length_invalid;
        }

        wsa += bytesProcessed; wsaLen -= bytesProcessed;
    } // if (opt_ind & WSA_WAVE_ROUTING_ADVERTISEMENT_PRESENT)

    /* Should have used up all of the bytes in the WSA */
    if (wsaLen != 0) {
        lenErr = -47;
        goto WSA_length_invalid;
    }

    DEBUG_PARSE_RECEIVED_WSA_PRINTF("return RS_SUCCESS\n");
    return RS_SUCCESS;

psid_error:
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("psid_error\n");
    shm_rsk_ptr->WSACnt.WSAPsidInvalid++;
#ifndef DEBUG_PARSE_RECEIVED_WSA
    {
        static int count1 = 0;
        if (count1++ < 10)
#endif
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Fail - PSID encoding variable PSID bytes fail!\n");
#ifndef DEBUG_PARSE_RECEIVED_WSA
    }
#endif

    return RS_ENOSRVC;

WSA_length_invalid:
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("WSA_length_invalid\n");
    shm_rsk_ptr->WSACnt.WSALengthInvalid++;
#ifndef DEBUG_PARSE_RECEIVED_WSA
    {
        static int count1 = 0;
        if (count1++ < 10)
#endif
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Fail - Length invalid! (%d)\n", lenErr);
#ifndef DEBUG_PARSE_RECEIVED_WSA
    }
#endif
    return RS_ENOROOM;

WSA_channelAccess_invalid:
    DEBUG_PARSE_RECEIVED_WSA_PRINTF("WSA_channelAccess_invalid\n");
    shm_rsk_ptr->WSACnt.WSAChannelAccessInvalid++;
#ifndef DEBUG_PARSE_RECEIVED_WSA
    {
        static int count1 = 0;
        if (count1++ < 10)
#endif
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Fail - Channel Access invalid!\n");
#ifndef DEBUG_PARSE_RECEIVED_WSA
    }
#endif
    return RS_ERANGE;
}

/**
** Function:  is1609p2Hdr
** @brief  Returns whether or not ptr points to a 1609.2 header
** @param  ptr -- [input]Pointer to the place in the packet where the 1609.2
**                header may or may not be.
** @param  len -- [input]The length of the data pointed to by ptr.
** @return TRUE if ptr points to a 1609.2 header. FALSE otherwise.
**
** Details: If ptr points to a byte containing VERSION_1609p2 (0x03) and a byte
**          containing either UNSECURED_TYPE_1609p2 or SECURED_TYPE_1609p2
**          (0x80 or 0x81), then we assume that ptr points to a 1609.2 header.
**          Otherwise, if definitely does not.
**/
static inline bool_t is1609p2Hdr(uint8_t *ptr, uint16_t len)
{
    if (len < 2) {
        return FALSE;
    }

    return (ptr[0] == VERSION_1609p2) &&
           ((ptr[1] == UNSECURED_TYPE_1609p2) || (ptr[1] == SECURED_TYPE_1609p2));
}

#ifdef NO_SECURITY
/**
** Function:  strip1609p2Header
** @brief  Strips the 1609.2 header off of a packet.
** @param  inData     -- [input]The input packet
** @param  inDataLen  -- [input]The length of the input packet
** @param  outData    -- [output]Pointer to the first bytes after the 1609.2
**                       header is written here
** @param  outDataLen -- [output]Length of the data pointed to by outData is
**                       written here
** @return void
**/
void strip1609p2Header(uint8_t *inData, uint16_t inDataLen,
                       uint8_t const **outData, uint16_t *outDataLen)
{
    uint8_t  hdrLen;
    uint16_t length;

    if ((inData[0] != 0x03) || (inData[1] != 0x80)) {
        /* Invalid 1609.2 header; return the entire packet */
        *outData    = inData;
        *outDataLen = inDataLen;
        return;
    }

    if (inData[2] == 0x82) {
        hdrLen = 5;
        length = (inData[3] << 8) + inData[4];
    }
    else if (inData[2] == 0x81) {
        hdrLen = 4;
        length = inData[3];
    }
    else {
        hdrLen = 3;
        length = inData[2];
    }

    *outData    = &inData[hdrLen];
    *outDataLen = length;
}

#endif

/**
** Function:  processReceivedWSA
** @brief  Receive WSA as a WSM routine.
** @param  rss           -- [input]The receive RSS, passed up from the radio.
** @param  srcMacAddress -- [input]The source MAC address, passed up from the
** @param  wsa    --        [input]Pointer to the WSA data
** @param  wsaLen --        [input]Length of the WSA data
** @return void
**
** Details: This function is called when a WSA is received.
**
** The first thing we do is look for an available entry in the WSAStatusTable
** to hold the parsed contents of the WSA by calling
** findAvailableWSAStatusTableEntry(). If we don't find an entry, we return
** RS_ESECPTR. Otherwise, processing continues. Note that
** findAvailableWSAStatusTableEntry() will return the status of whether or not
** a new WSAStatusTable entry was allocated for this WSA.
**
** We only parse the WSA if we have allocated a new WSAStatusTable entry, or
** the WSA change count has changed from the last WSA we received from the same
** RSE. Otherwise, we assume the WSA has not changed from last time, and we use
** the parsed WSA we had previously to save processing time. To parse the WSA
** we call parseReceivedWSA().
**
** If the WSA is secured, we save a copy of the raw WSA in the WSAStatusTable
** (in addition to the parsed copy) for use with later verification. The WSA is
** only saved if we have a new WSAStatusTable entry or the change count has
** changed.
**
** We call compareReceivedWsa() to (possibly) send a SERVICE_AVAILABLE
** indication up to the appropriate applications. This is done whether the WSA
** is secured or not. For a secured WSA, it is assumed the application will
** later request that the WSA be verified (via the wsuRisVerifyWsaRequest()
** function, which calls wsuVerifyWSARequest() via a WSA_NS_VERIFY_WSA_REQ
** message to the Radio_ns resource manager) before it joins the service.
**/
static void processReceivedWSA(int8_t   rss,
                               uint8_t *srcMacAddress,
                               uint8_t *wsa,
                               uint16_t wsaLen)
{
    uint8_t        wsaNum = 0;
    bool_t         newWSATableEntry = FALSE;
    int            ret;
    uint8_t const *payload;
    uint16_t       payloadLen;
    securityType   security;
    uint32_t       signedVerifiedFlags;
#ifndef NO_SECURITY
    uint32_t       securityFlags;
    uint32_t       vodMsgSeqNum;
#endif
    uint8_t       *origWsa;
    uint16_t       origWsaLen;
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Recv WME WSA as WSM, "
            "Source Mac=%02X:%02X:%02X:%02X:%02X:%02X, len=%d, "
            "rss=%d\n",
            srcMacAddress[0], srcMacAddress[1], srcMacAddress[2],
            srcMacAddress[3], srcMacAddress[4], srcMacAddress[5],
            wsaLen, rss);
#endif
    shm_rsk_ptr->WSACnt.WSAReceived++;

    /* Make sure we have enough bytes for a minimum-length WSA, which would
     * have a 1609.3 protocol version, an unsecured type byte, a length byte,
     * a protocol version/header opt id byte, and a WSA identifier/content
     * count byte. So the minimum length is 5 bytes. Of course, since a 5-byte
     * WSA would have no room for a service info segment, a channel info
     * segment, or a WRA segment, it wouldn't be very useful! */
    if (wsaLen < 5) {
        goto processReceivedWSATooShort;
    }

    /* If the next thing in the WSA is not a 1609.2 header, drop the WSA */
    if (!is1609p2Hdr(wsa, wsaLen)) {
        shm_rsk_ptr->WSACnt.WSAWrongVersion++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"1609.2 protocol version incorrect.\n");
        goto processReceivedWSAErr;
    }

    /* Look for an available entry in the WSAStatusTable */
    if (findAvailableWSAStatusTableEntry(srcMacAddress, &wsaNum,
                                         &newWSATableEntry) != RS_SUCCESS) {
        shm_rsk_ptr->WSACnt.WSANoAvailableWSAStatusTableEntry++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No available WSAStatusTable.\n");
        goto processReceivedWSAErr;
    }

    /* Put a received time stamp on the WSA */
    if (sem_wait(&WSAStatusTable[wsaNum].WSAStatusTableSem) < 0) {
        shm_rsk_ptr->WSACnt.WSASemaphoreError++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Taking WSA semaphore interrupted\n");
        goto processReceivedWSAErr;
    }

    gettimeofday(&WSAStatusTable[wsaNum].receivedTimestamp, NULL);

    if (sem_post(&WSAStatusTable[wsaNum].WSAStatusTableSem) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error posting to semaphore: %s\n", strerror(errno));
    }

    /* Strip off the security info */
    origWsa    = wsa;
    origWsaLen = wsaLen;
    shm_rsk_ptr->WSACnt.WSASecurityInfoStripAttempt++;

#ifndef NO_SECURITY
    if (alsmiVerifyThenProcess(origWsa, origWsaLen, &payload, &payloadLen,
                               ALSMI_PKT_TYPE_WSA, &security, &securityFlags,
                               0, NULL, NULL, NULL, &vodMsgSeqNum,
                               WSM_WSA_PSID, RT_CV2X) < 0) {
        shm_rsk_ptr->WSACnt.WSASecurityInfoStripFailed++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSA security strip failed.\n");
        goto processReceivedWSAErr;
    }
#else
    /* Strip off the 1609.2 header */
    strip1609p2Header(origWsa, origWsaLen, &payload, &payloadLen);
    security = 0;
#endif

    if ((security != SECURITY_SIGNED) && (security != SECURITY_UNSECURED)) {
        shm_rsk_ptr->WSACnt.WSASecurityUnsupportedSecurityType++;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSA security unsupported security type (%d).\n", security);
    }

    /* For reasons given above, we must have at least 2 bytes at this point */
    if (payloadLen < 2) {
        goto processReceivedWSATooShort;
    }

    /* Indicate signed but not verified */
    signedVerifiedFlags = (security == SECURITY_SIGNED) ? WSA_SIGNED : 0;

    /* Process WSA */
    if (newWSATableEntry ||
        ((payload[1] & WSA_CONTENT_COUNT_MASK) != WSAStatusTable[wsaNum].changeCount)) {
        /* Do this logic only if the WSA content count changed. */
        shm_rsk_ptr->WSACnt.WSAAnalyzed++;
        ret = parseReceivedWSA(payload, payloadLen, wsaNum, rss,
                               srcMacAddress, signedVerifiedFlags);

        if (ret != RS_SUCCESS) {
            shm_rsk_ptr->WSACnt.WSAParseError++;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"parseReceivedWSA Fail!\n");
#endif
            goto processReceivedWSAErr;
        }
    }

    /* If secured, save for possible later verification.
       This needs to be done regardless of the CONTENT_COUNT field changing because the sender may
       re-sign the WSA without updating the CONTENT_COUNT.  It may be more efficient to attempt to
       compare the previous and old signatures before saving the latest WSA, but we will just do the
       copy each time for simplicity. */
    if (security == SECURITY_SIGNED) {
        /* Lock the WSAVerifySem while saving the WSA so that we don't
         * try to do a verification in the middle of copying over a new
         * WSA */
        if (sem_wait(&WSAStatusTable[wsaNum].WSAVerifySem) < 0) {
            shm_rsk_ptr->WSACnt.WSAVerifySemaphoreError++;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error taking WSA Verify semaphore: %s\n",
                       strerror(errno));
        }

        memcpy(WSAStatusTable[wsaNum].wsaData, origWsa, origWsaLen);
        WSAStatusTable[wsaNum].wsaLen = origWsaLen;

        if (sem_post(&WSAStatusTable[wsaNum].WSAVerifySem) < 0) {
            shm_rsk_ptr->WSACnt.WSAVerifySemaphoreError++;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error posting to WSA Verify semaphore: %s\n",
                       strerror(errno));
        }
    }

    /* Send a WSA indication. */
    compareReceivedWsa(rss, srcMacAddress, wsaNum, signedVerifiedFlags);
    return;

processReceivedWSATooShort:
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSA too short\n");
#endif
    shm_rsk_ptr->WSACnt.WSALengthInvalid++;

processReceivedWSAErr:
    if (newWSATableEntry) {
        /* If error, and this is a new WSAStatusTable entry, don't take up a
         * new WSAStatusTable entry for this. */
        WSAStatusTable[wsaNum].wsaStatus = WSA_NO;
    }

    shm_rsk_ptr->WSACnt.WSAProcessError++;
}

#ifndef NO_SECURITY
/**
** Function:  processReceivedP2P
** @brief  Process a received P2P packet
** @param  data -- [input]Pointer to the P2P data
** @param  len  -- [input]Length of the P2P data
** @return void
**
** Details: This function is called when a P2P packet is received. The data is
**          passed to ALSMI for processing.
**/
static void processReceivedP2P(uint8_t *data, uint16_t len)
{
    int result;

    shm_rsk_ptr->P2PCnt.P2PReceived++;

    /* Pass the P2P data to alsmi for processing */
    if ((result = alsmiProcessP2PData(data, len)) != 0) {
        shm_rsk_ptr->P2PCnt.P2PProcessError++;
        shm_rsk_ptr->P2PCnt.P2PProcessLastErrorCode = result;
    }
    else {
        shm_rsk_ptr->P2PCnt.P2PProcessed++;
    }
}
#endif

/**
** Function:  processRxIeee
** @brief  Process a received IEEE packet.
** @param  pPacket       -- [input]Pointer to the received packet structure,
**                          passed up the radio. Only used if radioType ==
**                          RT_DSRC.
** @param  rss           -- [input]The receive RSS, passed up from the radio.
**                          Only used if radioType == RT_DSRC.
** @param  dataRate      -- [input]The receive data rate, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  userPriority  -- [input]The user priority, passed up from the radio
**                          Only used if radioType == RT_DSRC.
** @param  srcMacAddress -- [input]The source MAC address, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  radioChannel  -- [input]The channel received on,  passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  data          -- [input]Pointer to the received packet, pointing to
*                           the WSMP header.
** @param  len           -- [input]The length of data.
** @param  radioType     -- [input]The radio type the packet was received on.
**                          RT_CV2X or RT_DSRC.
** @return void
**
** Details: An rsReceiveDataType structure is filled out via processWSMP() and
**          this routine. Then the WST is searched to find out which entry the
**          PSID is in. The corrsponding AST entry is found, and a port is
**          calculated (RIS_RECV_IND_PORT1 through RIS_RECV_IND_PORT8) to send
**          the receive indication back to. Then the packet is sent back to RIS
**          via that port.
**/
static void processRxIeee(tMKxRxPacketData *pPacket,
                          int8_t            rss,
                          dataRateType      dataRate,
                          uint8_t           userPriority,
                          uint8_t          *srcMacAddress,
                          uint8_t           radioChannel,
                          uint8_t          *data,
                          uint16_t          len,
                          rsRadioType       radioType,
                          uint32_t          ppp)
{
    rsReceiveDataType   recv;
    WsmStatusEntryType *wst;
    uint8_t             wsmpHdrLen;
    rsResultCodeType    risRet;
    struct timeval      ts;
    uint8_t            *packet = data;
    uint16_t            packetLength = len;
    uint8_t const      *payload;
    uint16_t            payloadLength;


    shm_rsk_ptr->RISIndCnt.ReceiveWSMData[radioType]++;

    /* Check the WSMP version */
    if ((*packet & WSM_VERSION_MASK) != WSM_VERSION_3) {
        shm_rsk_ptr->RISIndCnt.ReceiveWSMError[radioType]++;
        shm_rsk_ptr->WSMError.WSMWrongVersion++;
        return;
    }

    /* Process the WSMP header and fill out some of the rsReceiveDataType
     * structure */
    risRet = processWSMP(packet, packetLength, (radioType == RT_CV2X),
                         radioType, &wst, &recv.u.wsm, &wsmpHdrLen);
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"processWSMP: risRet = %s\n", rsResultCodeType2Str(risRet));
#endif

    if (risRet != RS_SUCCESS) {
        /* Don't print an error message for RS_UNKNTPID (unknown TPID, which is
         * legal but we don't support) or RS_ENOSRVC (unregisted PSID, which is
         * very likely to happen under normal circumstances) */
        if ((risRet != RS_UNKNTPID) && (risRet != RS_ENOSRVC)) {
            RSU_PRINTF_MAX_N_TIMES(10, "processWSMP() failed: risRet = %s\n",
                                   rsResultCodeType2Str(risRet));
        }

        /* Drop the packet */
        shm_rsk_ptr->RISIndCnt.ReceiveWSMError[radioType]++;
        cv2xStatus.wme_rx_bad++;
        return;
    }

    /* Skip past the WSMP header */
    if (packetLength < wsmpHdrLen) {
        /* Drop the packet if too short */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSM too short\n");
        shm_rsk_ptr->RISIndCnt.ReceiveWSMError[radioType]++;
        shm_rsk_ptr->WSMError.WSMLengthInvalid++;
        cv2xStatus.wme_rx_bad++;
        return;
    }

    packet       += wsmpHdrLen;
    packetLength -= wsmpHdrLen;

    /* if the next thing in the packet is not a 1609.2 header, drop the
     * packet */
    if ((!is1609p2Hdr(packet, packetLength)) && (recv.u.wsm.psid != WSM_P2P_PSID)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSM 1609.3 header not followed by 1609.2 header\n");
        shm_rsk_ptr->RISIndCnt.ReceiveWSMError[radioType]++;
        shm_rsk_ptr->WSMError.WSMNo1609p2HeaderDropped++;
        cv2xStatus.wme_rx_bad++;
        return;
    }

    shm_rsk_ptr->RISIndCnt.ReceiveWSMDataOK[radioType]++;

    /* If WSA, send to WSA processing */
    if ((radioType == RT_CV2X) && (recv.u.wsm.psid == WSM_WSA_PSID)) {
        processReceivedWSA(rss, srcMacAddress, packet, packetLength);
        return;
    }

#ifndef NO_SECURITY
    /* If P2P, send to P2P processing if P2P is enabled */
    if (recv.u.wsm.psid == WSM_P2P_PSID) {
        /* Process the P2P packet if P2P is enabled */
        if (p2pEnabled) {
            processReceivedP2P(packet, packetLength);
        }

        return;
    }

#endif
    if (radioType == RT_DSRC) {
        /* Fill out the parameters that were passed in from the radio */
        recv.u.wsm.rss            = rss;
        recv.u.wsm.radioNum       = pPacket->RadioID;
        recv.u.wsm.dataRate       = dataRate;
        recv.u.wsm.userPriority   = userPriority;
        memcpy(recv.u.wsm.srcMacAddress, srcMacAddress, LENGTH_MAC);
        recv.u.wsm.recvPlcpLength = pPacket->RxFrameLength;
        recv.u.wsm.rssAntA        = (float32_t)pPacket->RxPowerA / 2.0;
        recv.u.wsm.rssAntB        = (float32_t)pPacket->RxPowerB / 2.0;
        recv.u.wsm.noiseAntA      = (float32_t)pPacket->RxNoiseA / 2.0;
        recv.u.wsm.noiseAntB      = (float32_t)pPacket->RxNoiseB / 2.0;
        recv.u.wsm.radioChannel   = radioChannel;
        recv.u.wsm.timeSlot       = pPacket->ChannelID;
    }
    else {
        /* Fill in some fields with dummy values. Probably at some time in the
         * future, the C-V2X radio driver may be updated to give us these
         * values. */
        recv.u.wsm.rss            = -60;
        recv.u.wsm.radioNum       = 0;
        /* Data rate will be filled in from WSMP header */
        recv.u.wsm.userPriority   = 3;
        memset(recv.u.wsm.srcMacAddress, 0, LENGTH_MAC);
        recv.u.wsm.recvPlcpLength = len;
        recv.u.wsm.rssAntA        = -60.0;
        recv.u.wsm.rssAntB        = -95.0;
        recv.u.wsm.noiseAntA      = -95.0;
        recv.u.wsm.noiseAntB      = -95.0;
        recv.u.wsm.radioChannel  = recv.u.wsm.channelNumber;
        recv.u.wsm.timeSlot       = 0;
        recv.u.wsm.ppp            = ppp;   
    }

    /* Fill out the rsReceiveDataType fields not filled out by processWSMP() */
    recv.indType         = IND_WSM; // This is a WSM indication
    recv.u.wsm.radioType = radioType;

    /* Get time stamp */
    gettimeofday(&ts, NULL);
    recv.u.wsm.timestamp_tv_sec = ts.tv_sec;
    recv.u.wsm.timestamp_tv_usec = ts.tv_usec;

    /* The securityID field is not currently used */
    recv.u.wsm.securityID       = 0;
#ifndef NO_SECURITY

    /* Do verify then process logic */
    lastVerifyRadioType = radioType;

    if (alsmiVerifyThenProcessIeee(&recv, packet, packetLength, &payload,
                                   &payloadLength, radioType) < 0) {
        shm_rsk_ptr->WSMError.WSMVerifyThenProcessError++;
    }

    /* If discarding verification failed messages, drop this message if
     * verification failed */
    if (getDiscardVerFailMsg() == TRUE) {
        if ((recv.u.wsm.securityFlags & 0xff) == securedVerifiedFail) {
            shm_rsk_ptr->WSMError.WSMVerificationFailedDropped++;
            return;
        }
    }

    /* If discarding unsecure messages, and security has been initialized,
     * drop this message if unsecured */
    if ((getDiscardUnsecureMsg() == TRUE) && smiInitialized) {
        if ((recv.u.wsm.securityFlags & 0xff) == unsecured) {
            shm_rsk_ptr->WSMError.WSMUnsecuredDropped++;
            return;
        }
    }
#else
    recv.u.wsm.securityFlags = unsecured;
    recv.u.wsm.vodMsgSeqNum  = 0;

    /* Strip off the 1609.2 header */
    strip1609p2Header(packet, packetLength, &payload, &payloadLength);
#endif

    if (payloadLength > MAX_WSM_DATA) {
        /* Drop the packet if too long */
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSM too long\n");
#endif
        shm_rsk_ptr->RISIndCnt.ReceiveWSMError[radioType]++;
        shm_rsk_ptr->WSMError.WSMLengthInvalid++;
        return;
    }

    /* Update the data length and copy over the data */
    recv.u.wsm.dataLength = payloadLength;
    memcpy(recv.u.wsm.data, payload, payloadLength);
    cv2xStatus.wme_rx_good++;
    /* Send the packet to the proper RIS receive WSM port */
    sendIndicationToRisByRadioTypeAndPsid(recv.u.wsm.radioType, recv.u.wsm.psid,
                                          &recv, TRUE);
}

/**
** Function:  processRxIso
** @brief  Process a received ISO packet.
** @param  pPacket       -- [input]Pointer to the received packet structure,
**                          passed up the radio. Only used if radioType ==
**                          RT_DSRC.
** @param  rss           -- [input]The receive RSS, passed up from the radio.
**                          Only used if radioType == RT_DSRC
** @param  dataRate      -- [input]The receive data rate, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  userPriority  -- [input]The user priority, passed up from the radio
**                          Only used if radioType == RT_DSRC.
** @param  srcMacAddress -- [input]The source MAC address, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  radioChannel  -- [input]The channel received on,  passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  data          -- [input]Pointer to the received packet, pointing to
*                           the WSMP header.
** @param  len           -- [input]The length of data.
** @param  radioType     -- [input]The radio type the packet was received on.
**                          RT_CV2X or RT_DSRC.
** @return None.
**
** Details: This is a dummy routine, which will be populated if we ever decide
**          to support ISO packets.
**/
static void processRxIso(tMKxRxPacketData *pPacket,
                         int8_t            rss,
                         dataRateType      dataRate,
                         uint8_t           userPriority,
                         uint8_t          *srcMacAddress,
                         uint8_t           radioChannel,
                         uint8_t          *data,
                         uint16_t          len,
                         rsRadioType       radioType)
{
    RSU_PRINTF_MAX_N_TIMES(10, "Family ID ISO not supported\n");
}

/**
** Function:  processRxEtsi
** @brief  Process a received ISO packet.
** @param  pPacket       -- [input]Pointer to the received packet structure,
**                          passed up the radio. Only used if radioType ==
**                          RT_DSRC.
** @param  rss           -- [input]The receive RSS, passed up from the radio.
**                          Only used if radioType == RT_DSRC
** @param  dataRate      -- [input]The receive data rate, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  userPriority  -- [input]The user priority, passed up from the radio
**                          Only used if radioType == RT_DSRC.
** @param  srcMacAddress -- [input]The source MAC address, passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  radioChannel  -- [input]The channel received on,  passed up from the
**                          radio. Only used if radioType == RT_DSRC.
** @param  data          -- [input]Pointer to the received packet, pointing to
*                           the WSMP header.
** @param  len           -- [input]The length of data.
** @param  radioType     -- [input]The radio type the packet was received on.
**                          RT_CV2X or RT_DSRC.
** @return void
**
** Details: This is a dummy routine, which will eventually be populated to
**          support ETSI-ITS packets.
**/
static void processRxEtsi(tMKxRxPacketData *pPacket,
                          int8_t            rss,
                          dataRateType      dataRate,
                          uint8_t           userPriority,
                          uint8_t          *srcMacAddress,
                          uint8_t           radioChannel,
                          uint8_t          *data,
                          uint16_t          len,
                          rsRadioType       radioType)
{
    /* VJR WILLBEREVISITED Add support for EU packets here */
}

/**
** Function:  cv2xRxPcap
** @brief Write a received C-V2X packet into the PCAP buffer.
** @param  pcapBuffer       -- [input]The PCAP buffer. Note that this contains
**                             the family ID byte followed by the WSM (ETH_HLEN
**                             - 1) bytes into the buffer. This allows room to
**                             delete the family ID byte and add an Ethernet
**                             header.
** @param  pcapBufferLength -- [input]The length of pcapBuffer. This length
**                             includes the ETH_HLEN-byte Ethernet header on
**                             the front of pcapBuffer.
** @return None.
**
** Details: Extracts the family ID byte, leaving ETH_HLEN bytes on the front
**          of pcapBuffer to write the Ethernet header. The Ethernet header is
**          written to the front of pcapBuffer, overwriting the family ID byte,
**          and determining the value for the ether_type field based on what
**          the value of the family ID byte was. Then the packet is written to
**          the PCAP buffer by calling pcap_write(). pcapBufferLength was
**          designed to be passed to pcap_write() without modification. The
**          family ID byte is restored after calling pcap_write(), since it is
**          used later.
**/
static void cv2xRxPcap(uint8_t *pcapBuffer, int pcapBufferLength)
{
    uint8_t              familyID = pcapBuffer[ETH_HLEN - 1];
    struct ether_header *ethhdr   = (struct ether_header *)pcapBuffer;

    if (pcapBufferLength < ETH_HLEN) {
        /* Return if too short. Note that pcapBufferLength will never be larger
         * than MAX_WSM_DATA + 32 because of the limit placed in the cv2x_rx()
         * call in cv2xRxThread(). */
        return;
    }

    /* Add an Ethernet header to the front of the packet, overwriting the
     * family ID byte */
    /* VJR WILLBEREVISITED Need upgrade to Qualcomm API to be able to get the
     * source and destination MAC addresses */
    memset(ethhdr->ether_dhost, 0xff, ETH_ALEN); /* dst_mac = broadcast */
    memset(ethhdr->ether_shost, 0x00, ETH_ALEN); /* src_mac = zero */
    ethhdr->ether_type = htons((familyID == FAMILY_ID_IEEE) ? ETH_P_WSMP :
                               (familyID == FAMILY_ID_ETSI) ? ETH_P_EU   :
                               /* for unknown familyID */     0x0800);
    /* Write to the PCAP buffer */
    pcap_write(pcapBuffer, pcapBufferLength, RT_CV2X, 0, 0, 0, 0, FALSE);
    /* Restore the family ID */
    pcapBuffer[ETH_HLEN - 1] = familyID;
}

/**
** Function:  cv2xRxThread
** @brief Thread to receive C-V2X packets.
** @param  arg -- [input]Required by convention; not used
** @return None.
**
** Details: Opens a socket to communicate to RIS with. Then repeatedly calls
**          cv2x_rx(). When the routine returns, indicating a packet has been
**          received, an rsReceiveDataType structure is filled out and is
**          sent back to RIS via the RIS_RECV_IND_PORT1.
**/
static void *cv2xRxThread(void *arg)
{
    /* Allow room for WSM + WSMP header + family ID. If PCAP is enabled, also
     * allow room for an Ethernet header after removing the family ID byte. */
    static uint8_t        pcapBuffer[MAX_WSM_DATA + 32];
    /* Point the address of the receive packet buffer to be ETH_HLEN - 1 bytes
     * into pcapBuffer. This will allow adding an ETH_HLEN-byte Ethernet header
     * after removing the family ID byte. */
    static uint8_t const *buffer = &pcapBuffer[ETH_HLEN - 1];
    uint16_t              dataLength;
    int32_t  ret = 0;
    uint32_t ppp = 0;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2xRxThread: Starting.\n");
#endif

    cv2xRxThreadRunning = TRUE;

    shm_rsk_ptr->bootupState |= 0x400; // cv2xRxThread running
    while (cv2xRxThreadRunning) {
        /* Attempt to receive a packet from the C-V2X radio */
        ret = rs_cv2x_rx((uint8_t *)buffer, MAX_WSM_DATA + 19, RS_CV2X_RX_TIMEOUT_USEC,&ppp);
        if (!cv2xRxThreadRunning) {
            break;
        }
        if (ret == 0) { /* TO on read is normal. */
            continue;
        }
        if (ret < 0) {
            RSU_PRINTF_MAX_N_TIMES(10, "cv2x_rx() failed: ret = %d\n",ret);
            cv2xStatus.wme_rx_bad++;
            continue;
        }
        dataLength = (uint16_t)ret;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dataLength = %u\n", dataLength);
#endif
        /* If C-V2X PCAP enabled, write the packet to the PCAP buffer. Include
         * the ETH_HLEN - 1 bytes in front of the family ID in the length. */
        if (pcap_enabled(RT_CV2X, 0, FALSE)) {
            cv2xRxPcap(pcapBuffer, dataLength + ETH_HLEN - 1);
        }

        /* Decide how to process the packet, based on the family ID (first
         * byte) */
        switch(buffer[0]) {
        case FAMILY_ID_IEEE:
            /* Parameters are dummy except buffer, dataLength, and RT_CV2X.
             * Don't include the family ID byte. */
            processRxIeee(NULL, 0, 0, 0, NULL, 0, (uint8_t *)&buffer[1], dataLength - 1,
                          RT_CV2X,ppp);
            break;

        case FAMILY_ID_ISO:
            /* Parameters are dummy except buffer, dataLength, and RT_CV2X.
             * Don't include the family ID byte. */
            processRxIso(NULL, 0, 0, 0, NULL, 0, (uint8_t *)&buffer[1], dataLength - 1,
                         RT_CV2X);
            break;

        case FAMILY_ID_ETSI:
            /* Parameters are dummy except buffer, dataLength, and RT_CV2X.
             * Don't include the family ID byte. */
            processRxEtsi(NULL, 0, 0, 0, NULL, 0, (uint8_t *)&buffer[1], dataLength - 1,
                          RT_CV2X);
            break;
        default:
            cv2xStatus.wme_rx_bad++;
            break;
        }
    } // while (cv2xRxThreadRunning)

    shm_rsk_ptr->bootupState &= ~0x400; // cv2xRxThread running
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Exiting\n");
#endif
    pthread_exit(NULL);
    return NULL;
}

/**
** Function:  stop_cv2x
** @brief Stops the entire C-V2X subsystem
** @return void
**
** Details: Stops the entire C-V2X subsystem by killing the cv2xRxThread,
**          calling cv2x_term(), and calling cv2x_module_term().
**
**          This is called at shutdown if the C-V2X subsystem has been started,
**          if the nsconfig command changes CV2X_ENABLE from TRUE to FALSE,
**          or if an error occurs in start_cv2x().
**/
void stop_cv2x(void)
{
    int result;

    /* Kill the cv2xRxThread */
    if (cv2xRxThread_created) {
        cv2xRxThreadRunning = FALSE;
#ifdef RSU_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pthread_kill(cv2xRxThreadId, SIGUSR1)\n");
#endif
        result = pthread_kill(cv2xRxThreadId, SIGUSR1);

        if (result != 0) {
#ifdef RSU_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_kill() failed: %s\n", strerror(result));
#endif
        }
        else {
#ifdef RSU_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_kill() succeeded: Calling pthread_join(cv2xRxThreadId, NULL)\n");
#endif
            result = pthread_join(cv2xRxThreadId, NULL);
#ifdef RSU_DEBUG
            if (result != 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_join() failed: %s\n", strerror(result));
            }
            else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_join() succeeded\n");
            }
#endif
        }
    }

    cv2xRxThread_created = FALSE;

    /* Terminate C-V2X Tx communication */
    if (cv2x_tx_inited) {
#ifdef RSU_DEBUG
       I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling rs_cv2x_term_tx(0x%x);\n", getPrimaryServiceId());
#endif
        rs_cv2x_term_tx(getPrimaryServiceId());
    }

    /* Terminate C-V2X communication */
    if (cv2x_inited) {
#ifdef RSU_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling rs_cv2x_term()\n");
#endif
        rs_cv2x_term();
    }

    cv2x_inited = FALSE;
}

/**
** Function:  start_cv2x
** @brief Starts up the entire C-V2X subsystem
** @return 0 if successful, -1 if failure
**
** Details: Starts up the entire C-V2X subsystem by calling cv2x_init() and
**          creating the cv2xRxThread.
**
**          This is called at startup if the initial value of CV2X_ENABLE is
**          TRUE, or if the nsconfig command changes CV2X_ENABLE from FALSE to
**          TRUE.
**/
int start_cv2x(void)
{
    int    result;

    /* Initialize C-V2X communication */
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling rs_cv2x_init()\n");
#endif
    result = rs_cv2x_init();

    if (result < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init() failed\n");
        goto start_cv2x_err;
    }
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init() succeeded\n");
#endif
    cv2x_inited = TRUE;

    /* Create the cv2xRxThread */
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pthread_create(cv2xRxThreadId, NULL, &cv2xRxThread, NULL)\n");
#endif
    result = pthread_create(&cv2xRxThreadId, NULL, &cv2xRxThread, NULL);

    if (result != 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2xRxThread creation failed: %s\n", strerror(result));
        goto start_cv2x_err;
    }
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2xRxThread creation succeeded\n");
#endif
    cv2xRxThread_created = TRUE;
    return 0;

start_cv2x_err:
    stop_cv2x();
    return -1;
}

/**
** Function:  configure_radio
** @brief Configure the radio
** @param  radioType -- [input]Radio Type
** @param  radioNum  -- [input]Radio Number
** @param  service   -- [input]Pointer to dsrc_sch_service_req_t containing
**                      information about the radio configuration
** @return None.
**
** Details: If the radio is DSRC, configures it according to the information
**          pointed to by "service" and sets radioConfigured to TRUE for the
**          specified radio.
**/
static void configure_radio(rsRadioType radioType, uint8_t radioNum,
                            dsrc_sch_service_req_t *service)
{
#if defined(ENABLE_DSRC)
    /* Only configure the radio for DSRC */
    if (radioType == RT_DSRC) {
        /* DevID for Hercules is always 0 */
        dsrc_configure_radio(DEV_ID0, radioNum, service);
        radioConfigured[radioNum]    = TRUE;
        radioConfiguration[radioNum] = *service;
    }
#endif
    /* Only configure the radio for CV2X */
    if (radioType == RT_CV2X) {
        radioConfigured[radioNum]    = TRUE;
        radioConfiguration[radioNum] = *service;
    }
}

/**
** Function:  wsuSelectService
** @brief Configure the Service
** @param  radioType  -- [input]Radio Type
** @param  radioNum   -- [input]Radio Number
** @param  serviceIdx -- [input]current service index
** @param  gst        -- [input]GST table entry for the service. This parameter
**                       must not be NULL.
** @return None.
**
** Details: GST is searched to find a service to run. Once found, if the
**          service is utilizing the DSRC radio, then the radio is configured.
**/
static void wsuSelectService(rsRadioType                    radioType,
                             uint8_t                        radioNum,
                             int                            serviceIdx,
                             GeneralServiceStatusEntryType *gst)
{
    dsrc_sch_service_req_t service;

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuSelectService:radioType = %s; radioNum = %d; pid = %u; serviceIdx = %d\n", (radioType == RT_CV2X) ? "C-V2X" : "DSRC", radioNum, gst->ASTEntry->pid, serviceIdx);
#endif
    if (gst->UserService) {
        /* User Service */
        if (gst->status.user.ExtendedAccess == SCH_EXTENDED_ACCESS)  {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(gst->status.user.ExtendedAccess == SCH_EXTENDED_ACCESS)\n");
#endif
            /* For extendedAccess = SCH_EXTENDED_ACCESS, set radio to Continious mode on SCH channel. */
            service.immediateAccess = gst->status.user.ImmediateAccess;
            service.extendedAccess  = SCH_EXTENDED_ACCESS;
            service.channel         = gst->ChannelNumber;
        }
        else {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"!(gst->status.user.ExtendedAccess == SCH_EXTENDED_ACCESS)\n");
#endif
            /* For extendedAccess = [0-65534] set to radio to Switched mode
             * (No support for extendedAccess 1-65534, default to 0(switched).) */
            service.immediateAccess = 0;
            service.extendedAccess  = 0;

            if (gst->status.user.Access == NO_SCH_ACCESS) {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(gst->status.user.Access == NO_SCH_ACCESS)\n");
#endif
                /* Switched + NO_SCH_ACCESSS means stay on CCH RF channel during SCH interval. */
                service.channel = getCCHNumber(radioNum);
            }
            else {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"!(gst->status.user.Access == NO_SCH_ACCESS)\n");
#endif
                /* For AUTO_ACCESS_MATCH and AUTO_ACCESS_UNCONDITIONAL configure SCH interval with SCH channel. */
                service.channel = gst->ChannelNumber;
            }
        }
    }
    else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"!(gst->UserService)\n");
#endif
        /* Provider Service */
        service.extendedAccess =
            (gst->status.provider.ChannelAccess == 0) ? SCH_EXTENDED_ACCESS : 0;
        service.channel = gst->ChannelNumber;
    }

    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurExtendedAccess = service.extendedAccess;
    service.edca_params.len        = 0;
    service.operationalRateSet.len = 0;

    if (gst->UserService) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(gst->UserService)\n");
#endif
        /* User Service */
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].UserService = TRUE;

        if (gst->status.user.Access == AUTO_ACCESS_UNCONDITIONAL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(gst->status.user.Access == AUTO_ACCESS_UNCONDITIONAL)\n");
#endif
            /* Start SCH Access Immediately */
            shm_rsk_ptr->wsuNsCurService[radioType][radioNum].ServiceRunning = TRUE;
            /* DevID for Hercules is always 0 */
            configure_radio(radioType, radioNum, &service);
        }
        else {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"!(gst->status.user.Access == AUTO_ACCESS_UNCONDITIONAL)\n");
#endif
            shm_rsk_ptr->wsuNsCurService[radioType][radioNum].ServiceRunning = FALSE;

            if (gst->status.user.Access == NO_SCH_ACCESS) {
                /* Radio is switched and listening to CCH during both intervals (and TX in one interval).
                 * Configure LLC 1st interval ("SCH") to CCH channel to listen on CCH during SCH interval.
                 * The SCH (RFchannel) is not in use, but during SCH interval CCH is monitored. */
                /* DevID for Hercules is always 0 */
                configure_radio(radioType, radioNum, &service);
            }
        }
    }
    else {
        /* Provider service */
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"!(gst->UserService)\n");
#endif
        service.extendedAccess = (gst->status.provider.ChannelAccess == 0) ? SCH_EXTENDED_ACCESS : 0;
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurProvAccessType = gst->status.provider.ChannelAccess;
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].ServiceRunning    = TRUE;
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].UserService       = FALSE;

        /* Configure the service channel radio */
        configure_radio(radioType, radioNum, &service);

#if 0
/* JJG: 20220330: when setting iwmh into continuous mode, the below creates a problem for the radio setup; disabling the below
   which is only invoked with ipb enabled; iwmh needs to normally operate in continuous mode and ipb is likely going to go
   away; for now remove this block of code but if DSRC continues to persist, evaluate restoring it */
        /* Also, if the service channel radio is in continuous channel
         * mode, and the service channel and the control channel are NOT
         * the same, configure the other radio for the control channel in
         * channel switching mode */
        if (radioType == RT_DSRC) {
            if ((service.extendedAccess != 0) &&
                (service.channel != getCCHNumber(radioNum))) {
                // Switch to channel switching mode 
                service.extendedAccess = 0;     
                /* DevID for Hercules is always 0 */
                configure_radio(radioType, (radioNum == 0) ? 1 : 0, &service);
            }
        }
#endif /* see JJG comment above */
    }

    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].Pid             = gst->ASTEntry->pid;
    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurServiceIndex = serviceIdx;
    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurChannel      = gst->ChannelNumber;
    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurPriority     = gst->ServicePriority;
    shm_rsk_ptr->wsuNsCurService[radioType][radioNum].CurAccessType   = gst->status.user.Access;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning\n");
#endif
}

/**
** Function:  getHighestPriorityServices
** @brief  Find the service that has the highest priority.
** @param  radioType  -- [input]Radio type
** @param  radioNum   -- [input]Radio number
** @param  serviceIdx -- [output]Index into GST is written to here
** @return A pointer to the GST entry that contains the highest priority
**         service.
**
** Details: Looks through GST to find the entry with the highest
**          ServicePriority. Access of AUTO_ACCESS_MATCH and
**          AUTO_ACCESS_UNCONDITIONAL take precedence over NO_SCH_ACCESS.
**          "serviceIdx" has written to it the index into GST
**/
static GeneralServiceStatusEntryType
*getHighestPriorityServices(rsRadioType radioType, uint8_t radioNum,
                            int *serviceIdx)
{
    int               i;
    uint8_t           highPriority = 0;
    rsUserRequestType curSchAccess = NO_SCH_ACCESS;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getHighestPriorityServices: radioNum = %u\n", radioNum);
#endif
    /* Assume no entry will be found */
    *serviceIdx = -1;

    /* Check through all of the entries in GST for the highest priority */
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL) &&
             GST.GSTEntry[i].UserService &&
            (GST.GSTEntry[i].RadioType == radioType) &&
            (GST.GSTEntry[i].RadioNum  == radioNum)) {
            /* Access of AUTO_ACCESS_MATCH or AUTO_ACCESS_UNCONDITIONAL takes
             * precedence over NO_SCH_ACCESS. Otherwise, higher priority takes
             * precedence over lower priority. */
            if ((GST.GSTEntry[i].status.user.Access != NO_SCH_ACCESS) &&
                (curSchAccess == NO_SCH_ACCESS)) {
                curSchAccess = GST.GSTEntry[i].status.user.Access;
                highPriority = GST.GSTEntry[i].ServicePriority;
                *serviceIdx = i;
            }
            else if ((GST.GSTEntry[i].ServicePriority > highPriority) &&
                     ((curSchAccess == NO_SCH_ACCESS) ||
                      (GST.GSTEntry[i].status.user.Access != NO_SCH_ACCESS))) {
                highPriority = GST.GSTEntry[i].ServicePriority;
                *serviceIdx = i;
            }
        } else if ((GST.GSTEntry[i].ASTEntry != NULL) &&
                   (GST.GSTEntry[i].RadioType == radioType) &&
                   (GST.GSTEntry[i].RadioNum == radioNum)) {
            /* provider service support; this function didn't exist for provider service requests in past implementation
               but since it is buried in the registration of all services, needs some additional handling; provider
               service will win (first come first serve) */
            *serviceIdx = i;
            break;      // This is derived HD code. Needed still?
        }
    }
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getHighestPriorityServices:serviceIdx = %d\n", *serviceIdx);
#endif
    if (*serviceIdx == -1) {
        /* No entry was found */
        return NULL;
    }

    /* An entry was found */
    return &GST.GSTEntry[*serviceIdx];
}

/**
** Function:  scheduleService
** @brief  Schedule the highest priority service to run for the given radio
**         type and radio number.
** @param radioType -- [input]Radio type
** @param radioNum  -- [input]Radio number
** @return None.
**
** Details:
**   Gets the highest priority service for the given radio type and radio
**   number. Then calls wsuSelectService() select the service to run.
**/
static void scheduleService(rsRadioType radioType, uint8_t radioNum)
{
    GeneralServiceStatusEntryType *gst = NULL;
    int                            serviceIdx;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,":scheduleService: radioNum = %u\n", radioNum);
#endif
    /* Find any entry has higher priority than the current one. */
    gst = getHighestPriorityServices(radioType, radioNum, &serviceIdx);

    if (gst != NULL) { // find the highest priority services gst entry
        wsuSelectService(radioType, radioNum, serviceIdx, gst);
        gst->ASTEntry->AppStatus = APP_REGISTERED;
    }
    else {
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].ServiceRunning = FALSE;
        shm_rsk_ptr->wsuNsCurService[radioType][radioNum].Pid            = 0;
    }
}

/**
** Function:  unregisterWsmService
** @brief  Unregisters the WSM service associated with a given pid
**         (application) and a given psid.
** @param  pid       -- [input]Unregister WSM service associated with this pid
** @param  radioType -- [input]RT_CV2X or RT_DSRC
** @param  psid      -- [input]Unregister WSM service associated with this psid
** @return None.
**
** Details: Deletes all entries associated with the pid and psid from
**          WST.
**/
static void unregisterWsmService(pid_t pid, rsRadioType radioType,
                                 uint32_t psid)
{
    int i;

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterWsmService:pid = %u; psid = 0x%x\n", pid, psid);
#endif
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((WST.WSMEntry[i].ASTEntry != NULL) &&
            (WST.WSMEntry[i].ASTEntry->pid == pid) &&
            (WST.WSMEntry[i].RadioType == radioType) &&
            (WST.WSMEntry[i].PSID == psid)) {
            WST.WSMEntry[i].ASTEntry  = NULL;
            WST.WSMEntry[i].PSID      = 0;

            if (WST.WSMCount != 0) {
                WST.WSMCount--;
            }
        }
    }

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterWsmService:WST Count = %d\n", WST.WSMCount);
#endif
}

/**
** Function:  unregisterService
** @brief  Unregisters the service associated with a given pid (application)
**         and a given psid.
** @param  pid       -- [input]Unregister service associated with this pid
** @param  radioType -- [input]RT_CV2X or RT_DSRC
** @param  psid      -- [input]Unregister service associated with this psid
** @param  radioNum  -- [output]Pointer to where to write the radio number of
**                      the service that is unregistered.
** @return None.
**
** Details: Deletes all entries associated with the pid and psid from
**          GST. Also deletes its associated WSM service. Clears certain shared
**          memory values. Outputs the radio type and radio number of the
**          service that was unregistered.
**/
static void unregisterService(pid_t    pid,  rsRadioType  radioType,
                              uint32_t psid, uint8_t     *radioNum)
{
    int i, j;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterService:pid = %u; psid = 0x%x\n", pid, psid);
#endif
    /* Clear out matching entries in GST */
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL) &&
            (GST.GSTEntry[i].ASTEntry->pid == pid) &&
            (GST.GSTEntry[i].RadioType == radioType) &&
            (GST.GSTEntry[i].PSID == psid)) {
            *radioNum = GST.GSTEntry[i].RadioNum;

            unregisterWsmService(pid, radioType, psid);
            GST.GSTEntry[i].ASTEntry = NULL;
            GST.GSTEntry[i].PSID     = 0;

            if (GST.GSTCount != 0) {
                GST.GSTCount--;
            }
        }
    }

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterService:radioNum = 0x%x\n", *radioNum);
#endif
    /* Set up shared memory. Take all radio types and radio numbers into
     * account */
    for (i = 0; i < MAX_RADIO_TYPES; i++) {
        for (j = 0; j < MAX_RADIOS; j++) {
            if (shm_rsk_ptr->wsuNsCurService[i][j].Pid == pid) {
                shm_rsk_ptr->wsuNsCurService[i][j].Pid = 0;

                if (shm_rsk_ptr->wsuNsCurService[i][j].ServiceRunning) {
                    /* VJR WILLBEREVISITED Implement and uncomment */
                    ///MLMEX_WSAEND_REQ(wsaTxSecurity[i][j].wsaRadio, &wsaData[i][j]);
                    shm_rsk_ptr->wsuNsCurService[i][j].UserService    = TRUE;
                    shm_rsk_ptr->wsuNsCurService[i][j].ServiceRunning = FALSE;
                    shm_rsk_ptr->wsuNsCurService[i][j].CurPriority    = 0;
                }
            }
        }
    }
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterService:GST Count = %d\n", GST.GSTCount);
#endif
}

/**
** Function:  unregisterAllServices
** @brief  Unregisters all services associated with a given pid (application).
** @param  pid -- [input]Unregister all services associated with this pid
** @return None.
**
** Details: Deletes all entries associated with the pid from
**          GST and WST. Clears certain shared
**          memory values.
**/
static void unregisterAllServices (pid_t pid)
{
   int i, j;

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterAllService:pid = %u\n", pid);
#endif
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL) &&
            (GST.GSTEntry[i].ASTEntry->pid == pid)) {
            GST.GSTEntry[i].ASTEntry = NULL;
            GST.GSTEntry[i].PSID     = 0;

            if (GST.GSTCount != 0) {
                GST.GSTCount--;
            }
        }

        if ((WST.WSMEntry[i].ASTEntry != NULL) &&
            (WST.WSMEntry[i].ASTEntry->pid == pid)) {
            WST.WSMEntry[i].ASTEntry  = NULL;
            WST.WSMEntry[i].PSID      = 0;

            if (WST.WSMCount != 0) {
                WST.WSMCount--;
            }
        }
    }

    for (i = 0; i < MAX_RADIO_TYPES; i++) {
        for (j = 0; j < MAX_RADIOS; j++) {
            if (shm_rsk_ptr->wsuNsCurService[i][j].Pid == pid) {
                shm_rsk_ptr->wsuNsCurService[i][j].Pid = 0;

                if (shm_rsk_ptr->wsuNsCurService[i][j].ServiceRunning) {
                    /* VJR WILLBEREVISITED Implement and uncomment */
                    //MLMEX_WSAEND_REQ(wsaTxSecurity[i][j].wsaRadio, &wsaData[i]);
                    shm_rsk_ptr->wsuNsCurService[i][j].UserService    = TRUE;
                    shm_rsk_ptr->wsuNsCurService[i][j].ServiceRunning = FALSE;
                    shm_rsk_ptr->wsuNsCurService[i][j].CurPriority    = 0;
                }
            }
        }
    }

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unregisterAllService:WST Count = %d; GST Count = %d\n",
            WST.WSMCount, GST.GSTCount);
#endif
}

/**
** Function:  deleteGeneralService
** @brief  Delete service entry
** @param  pid       -- [input]the PID
** @param  radioType -- [input]RT_CV2X or RT_DSRC
** @param  psid      -- [input]the PSID
** @return 0 for success.
**
** Details: Unregisters the service associated with the given application (pid)
**          and paid. Then schedules a new service to take its place if
**          possible.
**/
static rsResultCodeType deleteGeneralService(pid_t pid, rsRadioType radioType,
                                             uint32_t psid)
{
    uint8_t radioNum = 0xff;

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"deleteGeneralService:pid = %u; psid = 0x%x\n", pid, psid);
#endif
    unregisterService(pid, radioType, psid, &radioNum);

    if (radioNum != 0xff) {
        scheduleService(radioType, radioNum);
    }

    return RS_SUCCESS;
}

/**
** Function:  initRPS
** @brief  Register an application with a given pid.
** @param  req -- [input]Information about the init RPS request
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Sees if the application is already registered, and returns an error
**          if it is. Otherwise, sees if the table is full and returns an error
**          if it is. Otherwise, allocates a new AST entry
**          and fills it out.
**/
rsResultCodeType initRPS(rsRPSMsgType *req)
{
    ASTEntryType *ASTEntry;

    /* Application already exists */
    if (getAppStatusTableEntry(req->pid) != NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"initRPS: ERROR:  RS_EDUPPID (Application(PID) already exists in AST)\n");
        return RS_EDUPPID;
    }

    /* Already maximum number of applications */
    if (AST.ASTCount >= MAX_APPS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"initRPS: ERROR:  RS_ENOROOM (Aleady maximum number of Application(PID))\n");
        return RS_ENOROOM;
    }

    /* No free AST Entry */
    if ((ASTEntry = getAppStatusTableEntry(0)) == NULL) {
        /* This should never occur */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"initRPS: ERROR:  RS_ENOROOM (No free AST entry)\n");
        return RS_ENOROOM;
    }

    /* Make sure it is a clean slate */
    AST.ASTCount++;
    ASTEntry->pid                     = req->pid;
    ASTEntry->serviceAvailableInd     = req->serviceAvailableInd;
    ASTEntry->receiveWSMInd           = req->receiveWSMInd;
    ASTEntry->AppStatus               = APP_INITIALIZED;
    unregisterAllServices(req->pid);

#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"initRPS: RS_SUCCESS [pid=0x%02x,ASTCount=%d]\n", req->pid, AST.ASTCount);
#endif
    return RS_SUCCESS;
}

/**
** Function:  releaseRPS
** @brief  Unregister an application with a given pid.
** @param  pid -- [input]The pid of the application to unregister.
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Sees if the application is registered, and returns an error if it
**          is not. Otherwise, clears out the AppStatusTable entry associated
**          with the pid of the application.
**/
rsResultCodeType releaseRPS(rsRPSMsgType *req)
{
    ASTEntryType *ASTEntry;

    /* Get the AST Entry */
    if ((ASTEntry = getAppStatusTableEntry(req->pid)) == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR:  RS_ENOPID (Application (PID) does not exist in AST)\n");
        return RS_ENOPID;
    }

    /* Clear the App info in MIB */
    unregisterAllServices(req->pid);
    ASTEntry->pid = 0;

    ASTEntry->AppStatus = APP_UNINITIALIZED;

    if (AST.ASTCount != 0) {
        AST.ASTCount--;
    }

    return RS_SUCCESS;
}

/**
** Function:  getIndPort
** @brief  Gets the port number for listening for incoming indications
** @param  pid  -- [input]The pid of the application to get the port number for
** @param  port -- [output]Pointer to where to write the port number
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Looks up the pid in the AST table; returns RS_ENOPID if not found.
**          Otherwise, gets the port number from the AST table, writes it to
**          the location pointed to by port, and returns RS_SUCCESS.
**/
rsResultCodeType getIndPort(pid_t pid, uint16_t *port)
{
    ASTEntryType *ASTEntry = getAppStatusTableEntry(pid);
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pid = %u\n", pid);
#endif
    /* Get the AST Entry */
    if (ASTEntry == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR:  RS_ENOPID (Application(PID) does not exist in AST)\n");
        return RS_ENOPID;
    }

    *port = RIS_RECV_IND_PORT1 + ASTEntry->index;
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"*port = %u\n", *port);
#endif
    return RS_SUCCESS;
}

/* Thread that periodically sends out WSA's */
static void *wsaTxThread(void *ptr)
{
    uint8_t radio = *((uint8_t *)ptr);
    uint8_t ipv6Radio = getIPv6RadioNum();
    struct timeval tv;
    uint32_t delay;
    vru_wsa_req *wsaData = currentWsaData[radio];
    rsResultCodeType risRet;
    uint8_t *contentCountPtr;

    while (wsaTxThreadRunning[radio]) {
        /* Get to a 100ms boundary + 10ms */
        gettimeofday(&tv, NULL);
        delay = (100000 - (tv.tv_usec % 100000)) + 10000;

        if (delay > 100000) {
            delay -= 100000;
        }

        usleep(delay);

        /* Transmit the WSA */
        if ((risRet = sendWSMActual(&wsaData->sendWSMStruct, wsaData->wsaBuf, wsaData->wsaLen)) != 0) {
            RSU_PRINTF_MAX_N_TIMES(10, "Radio_ns:%s: V2X TX Fail - Lower layer reported error %d(%s)\n",
                          __func__, risRet, rsResultCodeType2Str(risRet));
        }

        /* If the WSA is secured, see how long since the last WSA signing. If
         * over 25 seconds, re-sign the WSA. */
        if (wsaTxSecurity[ipv6Radio].enabled) {
            gettimeofday(&tv, NULL);

            if (tv.tv_sec - toBeSignedWsa[ipv6Radio].WSALastSignTime >= 25) {
                toBeSignedWsa[ipv6Radio].WSALastSignTime = tv.tv_sec;
                /* Put in the WSA identifier and content count. For now, always use a
                 * WSA identifier (bits 4-7) of 1 */
                contentCountPtr = ((uint8_t *)&toBeSignedWsa[ipv6Radio].data) + 1;
                *contentCountPtr = 0x10 + (WSAContentCount[radio] % 16);
                WSAContentCount[radio]++;
                /* Set this flag to WFALSE so that when the WSA sign confirm
                 * routine runs, it will launch a new thread. */
                wsaTxThreadRunning[radio] = FALSE;
                if(0x1 == getALSMISignVerifyEnable()){
#if !defined(NO_SECURITY)
                    alsmiSign(toBeSignedWsa[ipv6Radio].data, toBeSignedWsa[ipv6Radio].length,
                          NULL, 0, ALSMI_PKT_TYPE_WSA, (void *)&wsaTxSecurity[ipv6Radio]);
                    /* A new thread to transmit WSA's is going to be created when
                     * the signing of the WSA completes. So terminate this
                     * thread. We don't want to count on wsaTxThreadRunning[radio]
                     * being WFALSE to break out of the loop because alsmiSign()
                     * might finish very quickly, and wsaTxThreadRunning[radio]
                     * might be set to WTRUE and a new thread might be launched,
                     * resulting in two or more WSA Tx threads for the same WSA
                     * running. */
#endif
                }
                break; /* Out of the while (wsaTxThreadRunning[radio]) loop */
            }
        }

        /* Sleep for the WSA period, minus a small amount (10ms) */
        usleep((useconds_t)(1000000.0 / ((float)wsaData->repeat / 5.0)) - 10000);
    }

    pthread_exit(0);
}

/* Schedules the periodic transmission of WSA's */
static void MLMEX_WSA_REQ(uint8_t radioNum, vru_wsa_req *wsaData)
{
    int rc = 0;
    struct timeval tv;
    uint8_t ipv6Radio = getIPv6RadioNum();

    if (radioNum < MAX_RADIOS) {
        currentWsaData[radioNum] = wsaData;

        if (!wsaTxThreadRunning[radioNum]) {
            wsaTxThreadRunning[radioNum] = TRUE;
            gettimeofday(&tv, NULL);
            toBeSignedWsa[ipv6Radio].WSALastSignTime = tv.tv_sec;
            rc = pthread_create(&wsaTxThreadID[radioNum], NULL, wsaTxThread, &radioNum);
            if (rc != 0) {
                fprintf(stderr, "%s: pthread_create() failed (%d)\n", __func__, rc);
            }
            rc = pthread_setname_np(wsaTxThreadID[radioNum],  "WSAperiodicTX");
            if (rc != 0) fprintf(stderr,"ERROR: %s: pthread_setname_np returned %d\n","WSAperiodicTX",rc);
        }
    }
}

/* Cancels the periodic transmission of WSA's */
static void MLMEX_WSAEND_REQ(uint8_t radioNum, vru_wsa_req *wsaData)
{
    if ((radioNum < MAX_RADIOS) &&
        (wsaTxThreadRunning[radioNum])) {
        wsaTxThreadRunning[radioNum] = FALSE;
        pthread_join(wsaTxThreadID[radioNum], NULL);
    }
}

/**
 * fillWaveServiceAdvertisement - Fill the contents of a WSA
 *
 * @gst: The entry in the GST table to user for parameters outside of the
 *       service info segments.
 * @wsa: Buffer to hold the WSA data.
 * @wsaEnd: Pointer to a pointer. This is written with a pointer just past the
 *          end of the WSA data after it is successfully written.
 *
 * Returns: None.
 *
 * The function constructs the raw WSA data to be transmitted.
 */
static void fillWaveServiceAdvertisement(
    GeneralServiceStatusEntryType *gst, uint8_t *wsa, uint8_t **wsaEnd)
{
    uint8_t *wsaTemp = wsa;
    uint8_t *versionOptionPtr;
    uint8_t *countPtr1;
    uint8_t *countPtr2;
    uint8_t *optionPtr;
    GeneralServiceStatusEntryType *gstTemp;
    uint8_t  i;
    uint8_t  serviceCount;
    uint32_t ext = gst->status.provider.WsaHeaderExtension;

    /* Set the WSA version */
    versionOptionPtr = wsaTemp;
    *wsaTemp++ = WSA_16093_PROTOCOL_VER_3;
    /* Put in the WSA identifier and content count. For now, always use a
     * WSA identifier (bits 4-7) of 1 */
    *wsaTemp++ = 0x10 + (WSAContentCount[gst->RadioNum] % 16);
    WSAContentCount[gst->RadioNum]++;

    /* Handle WSA information extension elements */
    if (ext & (WSA_EXT_REPEAT_RATE_BITMASK           |
               WSA_EXT_2DLOCATION_BITMASK            |
               WSA_EXT_3DLOCATION_BITMASK |
               WSA_EXT_ADVERTISER_ID_BITMASK)) {
        /* Set the extension element bit in the version/option byte */
        *versionOptionPtr |= WSA_INFORMATION_EXTENSION_ELEMENT_PRESENT;
        /* Initialize the extension element count */
        countPtr1 = wsaTemp;
        *wsaTemp++ = 0;

        if (ext & WSA_EXT_REPEAT_RATE_BITMASK) {
            *wsaTemp++ = WAVE_ID_REPEAT_RATE;
            *wsaTemp++ = 1;
            *wsaTemp++ = gst->status.provider.RepeatRate;
            (*countPtr1)++;
        }

        if (ext & WSA_EXT_2DLOCATION_BITMASK) {
            *wsaTemp++ = WAVE_ID_2DLOCATION;
            *wsaTemp++ = 8;
            *wsaTemp++ = HIGH_HIGH_BYTE(gst->status.provider.latitude2D);
            *wsaTemp++ = HIGH_LOW_BYTE(gst->status.provider.latitude2D);
            *wsaTemp++ = HIGH_BYTE(gst->status.provider.latitude2D);
            *wsaTemp++ = LOW_BYTE(gst->status.provider.latitude2D);
            *wsaTemp++ = HIGH_HIGH_BYTE(gst->status.provider.longitude2D);
            *wsaTemp++ = HIGH_LOW_BYTE(gst->status.provider.longitude2D);
            *wsaTemp++ = HIGH_BYTE(gst->status.provider.longitude2D);
            *wsaTemp++ = LOW_BYTE(gst->status.provider.longitude2D);
            (*countPtr1)++;
        }

        if (ext & WSA_EXT_3DLOCATION_BITMASK) {
            uint32_t temp32;
            uint16_t temp16;

            *wsaTemp++ = WAVE_ID_3DLOCATION;
            *wsaTemp++ = 10;
            /* Use UPER-encoded values for latitude, longitude, and elevation,
             * Minimum unencoded values for latitude, longitude, and elevation
             * are -900000000, -1799999999, and -4096 respectively. */
            temp32 = gst->status.provider.latitude3D + 900000000;
            *wsaTemp++ = HIGH_HIGH_BYTE(temp32);
            *wsaTemp++ = HIGH_LOW_BYTE(temp32);
            *wsaTemp++ = HIGH_BYTE(temp32);
            *wsaTemp++ = LOW_BYTE(temp32);
            temp32 = gst->status.provider.longitude3D + 1799999999;
            *wsaTemp++ = HIGH_HIGH_BYTE(temp32);
            *wsaTemp++ = HIGH_LOW_BYTE(temp32);
            *wsaTemp++ = HIGH_BYTE(temp32);
            *wsaTemp++ = LOW_BYTE(temp32);
            temp16 = gst->status.provider.elevation3D + 4096;
            *wsaTemp++ = HIGH_BYTE(temp16);
            *wsaTemp++ = LOW_BYTE(temp16);
            (*countPtr1)++;
        }

        if (ext & WSA_EXT_ADVERTISER_ID_BITMASK) {
            *wsaTemp++ = WAVE_ID_ADVERTISER_ID;
            *wsaTemp++ = gst->status.provider.advertiseIdLength + 1; // Length
            *wsaTemp++ = gst->status.provider.advertiseIdLength;     // UTF-8 Length (weird)
            memcpy(wsaTemp, gst->status.provider.advertiseId, gst->status.provider.advertiseIdLength);
            wsaTemp += gst->status.provider.advertiseIdLength;
            (*countPtr1)++;
        }
    } // WSA information extension element present

    /* Service Info Segments */
    countPtr1 = NULL;
    serviceCount = 0;

    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL) &&
             !GST.GSTEntry[i].UserService &&
            (GST.GSTEntry[i].ChannelNumber == gst->ChannelNumber) &&
            (serviceCount < MAX_SERVICE_INFO) &&
            (GST.GSTEntry[i].status.provider.ChannelAccess == gst->status.provider.ChannelAccess)) {
            /* Set a pointer to the extension element count the first time around */
            if (countPtr1 == NULL) {
                countPtr1 = wsaTemp;
                wsaTemp++;
            }

            serviceCount++;
            gstTemp = &GST.GSTEntry[i];

            /* Set the PSID */
            if (gstTemp->PSID >= PSID_4BYTE_MIN_VALUE && gstTemp->PSID <= PSID_4BYTE_MAX_VALUE) {
                *wsaTemp++ = HIGH_HIGH_BYTE(gstTemp->PSID);
                *wsaTemp++ = HIGH_LOW_BYTE(gstTemp->PSID);
                *wsaTemp++ = HIGH_BYTE(gstTemp->PSID);
                *wsaTemp++ = LOW_BYTE(gstTemp->PSID);
            } else if (gstTemp->PSID >= PSID_3BYTE_MIN_VALUE && gstTemp->PSID <= PSID_3BYTE_MAX_VALUE) {
                *wsaTemp++ = HIGH_LOW_BYTE(gstTemp->PSID);
                *wsaTemp++ = HIGH_BYTE(gstTemp->PSID);
                *wsaTemp++ = LOW_BYTE(gstTemp->PSID);
            } else if (gstTemp->PSID >= PSID_2BYTE_MIN_VALUE && gstTemp->PSID <= PSID_2BYTE_MAX_VALUE) {
                *wsaTemp++ = HIGH_BYTE(gstTemp->PSID);
                *wsaTemp++ = LOW_BYTE(gstTemp->PSID);
            } else if (gstTemp->PSID <= PSID_1BYTE_MAX_VALUE) {
                *wsaTemp++ = LOW_BYTE(gstTemp->PSID);
            } else {
                /* RSK_NS_LOG(KERN_DEBUG, "fillWaveServiceAdvertisement Fail - PSID encoding variable PSID bytes fail!\n"); */
                return;
            }

            /* Always set the channel index (bits 3-7) to 1 */
            optionPtr = wsaTemp;
            *wsaTemp++ = 1 << 3;

            if (ext & (WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |
                       WSA_EXT_IPV6ADDRESS_BITMASK              |
                       WSA_EXT_SERVICE_PORT_BITMASK             |
                       WSA_EXT_PROVIDER_MACADDRESS_BITMASK      |
                       WSA_EXT_RCPI_THRESHOLD_BITMASK           |
                       WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK      |
                       WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK)) {
                /* Set the WSA_SERVICE_INFO_OPTION_PRESENT bit and initialize the count */
                *optionPtr |= WSA_SERVICE_INFO_OPTION_PRESENT;
                countPtr2 = wsaTemp;
                *wsaTemp++ = 0;

                if (ext & WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK) {
                    *wsaTemp++ = WAVE_ID_PROVIDER_SERVICE_CONTEXT;
                    *wsaTemp++ = gstTemp->LengthPsc + 1; // element length
                    *wsaTemp++ = gstTemp->LengthPsc;     // PSC length (weird!)
                    memcpy(wsaTemp, gstTemp->PSC, gstTemp->LengthPsc);
                    wsaTemp += gstTemp->LengthPsc;
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_IPV6ADDRESS_BITMASK) {
                    *wsaTemp++ = WAVE_ID_IPV6ADDRESS;
                    *wsaTemp++ = LENGTH_IPV6;
                    memcpy(wsaTemp, gstTemp->status.provider.IPv6Address, LENGTH_IPV6);
                    wsaTemp += LENGTH_IPV6;
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_SERVICE_PORT_BITMASK) {
                    *wsaTemp++ = WAVE_ID_SERVICE_PORT;
                    *wsaTemp++ = 2;
                    *wsaTemp++ = HIGH_BYTE(gstTemp->status.provider.IPv6ServicePort);
                    *wsaTemp++ = LOW_BYTE(gstTemp->status.provider.IPv6ServicePort);
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
                    *wsaTemp++ = WAVE_ID_PROVIDER_MACADDRESS;
                    *wsaTemp++ = LENGTH_MAC;
                    memcpy(wsaTemp, gstTemp->status.provider.ProviderMacAddress, LENGTH_MAC);
                    wsaTemp += LENGTH_MAC;
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_RCPI_THRESHOLD_BITMASK) {
                    *wsaTemp++ = WAVE_ID_RCPI_THRESHOLD;
                    *wsaTemp++ = 1;
                    *wsaTemp++ = gstTemp->status.provider.RcpiThreshold;
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK) {
                    *wsaTemp++ = WAVE_ID_WSA_COUNT_THRESHOLD;
                    *wsaTemp++ = 1;
                    *wsaTemp++ = gstTemp->status.provider.WsaCountThreshold;
                    (*countPtr2)++;
                }

                if (ext & WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK) {
                    *wsaTemp++ = WAVE_ID_WSA_COUNT_THRESHOLD_INTERVAL;
                    *wsaTemp++ = 1;
                    *wsaTemp++ = gstTemp->status.provider.WsaCountThresholdInterval;
                    (*countPtr2)++;
                }
            } // if service info extension elements present
        } // if this service is valid
    } // for (i = 0; i < MAX_V3_SERVICES; i++)

    /* Set the WSA_SERVICE_INFO_SEGMENT_PRESENT bit and the service count if
     * service info segments are present */
    if (countPtr1 != NULL) {
        *versionOptionPtr |= WSA_SERVICE_INFO_SEGMENT_PRESENT;
        *countPtr1 = serviceCount;
    }

    /* Set the (one and only) channel info segment */
    *versionOptionPtr |= WSA_CHANNEL_INFO_SEGMENT_PRESENT;
    *wsaTemp++ = 1; // count = 1
    *wsaTemp++ = WSA_CHANNEL_INFO_OPERATING_CLASS;
    *wsaTemp++ = gst->ChannelNumber;
    /* Change the scaling of Tx power from -128..127 to 0..255 */
    *wsaTemp++ = getIPv6TxPowerLevel() + 128;
    *wsaTemp++ = ((getIPv6Adaptable() != 0) ? WSA_CHANNEL_INFO_ADAPTABLE : 0) |
                 (getIPv6DataRate() & 0x7F);
    optionPtr = wsaTemp;
    *wsaTemp++ = 0;

    /* Set the channel info extension elements */
    if (ext & (WSA_EXT_EDCA_PARAM_SET_BITMASK |
               WSA_EXT_CHANNEL_ACCESS_BITMASK)) {
        *optionPtr |= WSA_CHANNEL_INFO_OPTION_PRESENT;
        countPtr1 = wsaTemp;
        *wsaTemp++ = 0;

        /* For 1609 v3, the edca_count must be 4, and the 4 EDCA parameter
         * sets are for AC_BE, AC_BK, AC_VI, and AC_VO. If
         * gst->status.provider.edca_count, it is an error, and don't output
         * anything. */
        if ((ext & WSA_EXT_EDCA_PARAM_SET_BITMASK) &&
            (gst->status.provider.edca_count == 4)) {
            *wsaTemp++ = WAVE_ID_EDCA_PARAM_SET;
            *wsaTemp++ = 16;

            for (i = 0; i < 4; i++) {
                *wsaTemp++ = gst->status.provider.aifs[i];
                *wsaTemp++ = gst->status.provider.ecw[i];
                *wsaTemp++ = HIGH_BYTE(gst->status.provider.txop[i]);
                *wsaTemp++ = LOW_BYTE(gst->status.provider.txop[i]);
            }

            (*countPtr1)++;
        }

        if (ext & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
            *wsaTemp++ = WAVE_ID_CHANNEL_ACCESS;
            *wsaTemp++ = 1;
            *wsaTemp++ = gst->status.provider.ChannelAccess;
            (*countPtr1)++;
        }
    }

    /* Set the WAVE routing advertisement segment */
    if (ext & WSA_EXT_WRA_BITMASK) {
        *versionOptionPtr |= WSA_WAVE_ROUTING_ADVERTISEMENT_PRESENT;
        *wsaTemp++ = HIGH_BYTE(gst->status.provider.routerLifeTime);
        *wsaTemp++ = LOW_BYTE(gst->status.provider.routerLifeTime);
        memcpy(wsaTemp, gst->status.provider.ipPrefix, LENGTH_IPV6);
        wsaTemp += LENGTH_IPV6;
        *wsaTemp++ = gst->status.provider.prefixLen;
        memcpy(wsaTemp, gst->status.provider.defaultGateway, DEFAULT_GATEWAY_LEN);
        wsaTemp += DEFAULT_GATEWAY_LEN;
        memcpy(wsaTemp, gst->status.provider.primaryDns, DNS_LEN);
        wsaTemp += DNS_LEN;
        countPtr1 = wsaTemp;
        *wsaTemp++ = 0;

        if (ext & WSA_EXT_SECONDARY_DNS_BITMASK) {
            *wsaTemp++ = WAVE_ID_SECONDARY_DNS;
            *wsaTemp++ = DNS_LEN;
            memcpy(wsaTemp, gst->status.provider.secondaryDns, DNS_LEN);
            wsaTemp += DNS_LEN;
            (*countPtr1)++;
        }

        if (ext & WSA_EXT_GATEWAY_MACADDRESS_BITMASK) {
            *wsaTemp++ = WAVE_ID_GATEWAY_MACADDRESS;
            *wsaTemp++ = LENGTH_MAC;
            memcpy(wsaTemp, gst->status.provider.gatewayMacAddress, LENGTH_MAC);
            wsaTemp += LENGTH_MAC;
            (*countPtr1)++;
        }
    }

    *wsaEnd = wsaTemp;
}

/**
 * wsaSignSecurityCfm - Confirm the WSA has been successfully signed.
 *
 * @result: The return code from Aerolink
 * @wsaSignedData: Pointer to the signed WSA
 * @wsaSignedDataLength: The signed WSA data length
 * @wsaTxSecurity: Security data
 *
 * Returns: None
 *
 * The function is called when a WSA sign succeeds. The elements of the wsaData
 * table entry for the specified radio are updated. Then a request is made to
 * start transmitting the WSA.
 *
 * This function is called from alsmi's sign callback function.
 */
void wsaSignSecurityCfm(AEROLINK_RESULT    returnCode,
                        uint8_t           *wsaSignedData,
                        uint32_t           wsaSignedDataLength,
                        wsaTxSecurityType *wsaTxSecurity)
{
    uint8_t  radio = wsaTxSecurity->wsaRadio;
    uint8_t *wsa   = wsaData[radio].wsaBuf;
#if 0
    /* see JJG comment below */
    uint8_t hdrLen;
#endif

    /* RSK_NS_LOG(KERN_DEBUG, "wsaSignSecurityCfm: \n"); */

    if (returnCode != WS_SUCCESS) {
        /* RSK_NS_LOG(KERN_DEBUG, "wsaSignSecurityCfm Fail - returnCode = %d \n", returnCode); */
        return;
    }

    /* Fill out the needed sendWSMV3Struct fields */
    memset(&wsaData[radio].sendWSMStruct, 0, sizeof(wsaData[radio].sendWSMStruct));
    memcpy(wsaData[radio].sendWSMStruct.destMac, wsaTxSecurity->destMac, LENGTH_MAC);
    wsaData[radio].sendWSMStruct.wsmpVersion     = WSM_VERSION_3;
    wsaData[radio].sendWSMStruct.channelNumber   = wsaTxSecurity->channelNumber;
    wsaData[radio].sendWSMStruct.userPriority    = wsaTxSecurity->priority;
    wsaData[radio].sendWSMStruct.channelInterval = wsaTxSecurity->channelInterval;
    wsaData[radio].sendWSMStruct.dataRate        = wsaTxSecurity->dataRate;
    wsaData[radio].sendWSMStruct.txPwrLevel      = (wsaTxSecurity->txPwrLevel * 2);  // value to LLC needs to be in 0.5 dB increments
    wsaData[radio].sendWSMStruct.radioNum        = radio;
    wsaData[radio].sendWSMStruct.radioType       = wsaData[radio].radioType;
    wsaData[radio].sendWSMStruct.wsmpHeaderExt   = WAVE_ID_CHANNEL_NUMBER_BITMASK |
                                                   WAVE_ID_DATARATE_BITMASK       |
                                                   WAVE_ID_TRANSMIT_POWER_BITMASK;
    wsaData[radio].sendWSMStruct.psid            = WSM_WSA_PSID;
    wsaData[radio].sendWSMStruct.eth_proto       = ETH_P_WSMP;

/* JJG: (20210519) to be revisited once security enabled; the original code ported used the following
   to create the header; however, this function was moved into the send function; need to check and
   see if the correct data is transmitted once a signed packet is available; may need to reintroduce
   this code and modify the send code
*/
#if 0
    /* Generate the WSA's WSMP header */
    wsa += createWSMPHeader(&wsaData[radio].sendWSMStruct, wsa, &hdrLen);
#endif

    /* Copy over the WSA data */
    if (wsaSignedDataLength > MAX_WSA_DATA - (wsa - wsaData[radio].wsaBuf)) {
        return;
    }

    memcpy(wsa, wsaSignedData, wsaSignedDataLength);

    /* Fill out the rest of the wsaData fields */
    wsaData[radio].mgmtId      = wsaTxSecurity->mgmtId;
    wsaData[radio].repeat      = wsaTxSecurity->repeat;
    wsaData[radio].channelType = wsaTxSecurity->channelType;
    wsaData[radio].wsaLen      = (wsa - wsaData[radio].wsaBuf) + wsaSignedDataLength;

    /* Start WSA transmission */
    MLMEX_WSA_REQ(radio, &wsaData[radio]);
}

#ifdef UNUSED_FUNCTION_20220524_IFDEF_ED_OUT_TO_STOP_UNUSED_FUNCTION_WARNING
/**
 * wsaUnsignedCfm - Function to prepare unsigned WSA.
 *
 * @radio: The radio of the service (IPv6) to be used; alternate of radio with WSAs
 *
 * Returns: None
 *
 * The function is called to prepare an unsigned WSA. The elements of the wsaData
 * table entry for the specified radio are updated. Then a request is made to
 * start transmitting the WSA.
 */
static void wsaUnsignedCfm(uint8_t radio)
{
    uint8_t *wsa   = wsaData[radio].wsaBuf;

    /* RSK_NS_LOG(KERN_DEBUG, "wsaSignSecurityCfm: \n"); */

    /* Fill out the needed sendWSMV3Struct fields */
    memset(&wsaData[radio].sendWSMStruct, 0, sizeof(wsaData[radio].sendWSMStruct));
    memcpy(wsaData[radio].sendWSMStruct.destMac, wsaTxSecurity[radio].destMac, LENGTH_MAC);
    wsaData[radio].sendWSMStruct.wsmpVersion     = WSM_VERSION_3;
    wsaData[radio].sendWSMStruct.channelNumber   = wsaTxSecurity[radio].channelNumber;
    wsaData[radio].sendWSMStruct.userPriority    = wsaTxSecurity[radio].priority;
    wsaData[radio].sendWSMStruct.channelInterval = wsaTxSecurity[radio].channelInterval;
    wsaData[radio].sendWSMStruct.dataRate        = wsaTxSecurity[radio].dataRate;
    wsaData[radio].sendWSMStruct.txPwrLevel      = (wsaTxSecurity[radio].txPwrLevel * 2);  // value to LLC needs to be in 0.5 dB increments
    wsaData[radio].sendWSMStruct.radioNum        = !radio;
    wsaData[radio].sendWSMStruct.radioType       = wsaData[radio].radioType;
    wsaData[radio].sendWSMStruct.wsmpHeaderExt   = WAVE_ID_CHANNEL_NUMBER_BITMASK |
                                                   WAVE_ID_DATARATE_BITMASK       |
                                                   WAVE_ID_TRANSMIT_POWER_BITMASK;
    wsaData[radio].sendWSMStruct.psid            = WSM_WSA_PSID;
    wsaData[radio].sendWSMStruct.eth_proto       = ETH_P_WSMP;

    /* Fill out the rest of the wsaData fields */
    wsaData[radio].mgmtId      = wsaTxSecurity[radio].mgmtId;
    wsaData[radio].repeat      = wsaTxSecurity[radio].repeat;
    wsaData[radio].channelType = wsaTxSecurity[radio].channelType;
    wsaData[radio].wsaLen      = toBeSignedWsa[radio].length;

    memcpy(wsa, toBeSignedWsa[radio].data, wsaData[radio].wsaLen);

#ifdef RSU_DEBUG
    dumpSendWSMStruct(&(wsaData[radio].sendWSMStruct));
#endif
    /* Start WSA transmission */
    MLMEX_WSA_REQ(!radio, &wsaData[radio]);
}
#endif // UNUSED_FUNCTION_20220524_IFDEF_ED_OUT_TO_STOP_UNUSED_FUNCTION_WARNING

/**
 * fillWsaReq - Construct a WSA to be transmitted.
 *
 * @gst: The entry in the GST table to user for parameters outside of the
 *       service info segments.
 * @radio: The radio number it will be transmitted on.
 *
 * Returns: WSA_UNKNOWN_SECURED_TYPE if an error occurred.
 *          WSA_SECURED_TYPE if the WSA is secured. WSA transmission will be
 *                              started from wsaSignSecurityCfm(), which is
 *                              called by alsmi, when signing is completed.
 *          WSA_UNSECURED_TYPE if the WSA is unsecured. WSA IS ready to be
 *                                transmitted immediately.
 *
 * The function constructs the raw WSA data to be transmitted.
 * fillWaveServiceAdvertisement()
 * is called to do the bulk of the work.
 *
 * If the WSA is unsecured, then the first 3 bytes are filled in properly, and
 * wsaData[radio].vsc will be properly filled out and ready for transmit upon
 * return from this function.
 *
 * If the WSA is secured, it is constructed in toBeSignedWsa[radio].data, then
 * is sent to Escrypt's software to be signed. When fillWsaReq() returns, the
 * request to sign has been accepted, but the WSA is NOT yet ready to be
 * transmitted. Later, ESBSMI will call wsuRisWSASignConfirm() which will call
 * wsaSignSecurityCfm() via the WSU_NS_WSA_SIGN_SECURITY_CFM ioctl. Then
 * the WSA data will be copied to wsaData[radio].vsc, and the WSA will THEN be
 * ready to be transmitted.
 */
static int fillWsaReq(GeneralServiceStatusEntryType *gst, uint8_t radio, rsRadioType radioType)
{
    uint8_t *wsa;
    uint8_t *wsaEnd = NULL; // Starts NULL so compiler warning "used uninited" below is happy

    if ((gst == NULL) ||
        ((gst->WsaSecurity != SECURED_WSA) &&
         (gst->WsaSecurity != UNSECURED_WSA))) {
        return WSA_UNKNOWN_SECURED_TYPE;
    }

    /* If in continuous channel mode, and the service channel does not equal
     * control channel for the current radio, send the WSA's out of the other
     * radio. Otherwise, send the WSA's out of the current radio */
    if ((gst->status.provider.ChannelAccess == 0) &&
        (gst->ChannelNumber != getCCHNumber(radio))) {
        wsaTxSecurity[radio].wsaRadio    = (radio == 0) ? 1 : 0;
    }
    else {
        wsaTxSecurity[radio].wsaRadio    = radio;
    }

    wsaTxSecurity[radio].enabled         = (gst->WsaSecurity == SECURED_WSA);
    wsaTxSecurity[radio].mgmtId          = VSC_MANAGEMENT_ID;
    wsaTxSecurity[radio].repeat          = gst->status.provider.RepeatRate;
    wsaTxSecurity[radio].channelNumber   = getCCHNumber(radio);
    wsaTxSecurity[radio].channelInterval = getWSAChannelInterval();
    wsaTxSecurity[radio].dataRate        = getWSADataRate();
    wsaTxSecurity[radio].txPwrLevel      = getWSATxPowerLevel();
    wsaTxSecurity[radio].channelType     = CHANTYPE_CCH;
    wsaTxSecurity[radio].priority        = getWSAPriority();
    memcpy(wsaTxSecurity[radio].destMac, gst->status.provider.DestMacAddress,
           LENGTH_MAC);
    wsa = toBeSignedWsa[radio].data;
    fillWaveServiceAdvertisement(gst, wsa, &wsaEnd);
    toBeSignedWsa[radio].length = wsaEnd - wsa;
    /* this needs to be done now */
    wsaData[radio].radioType = radioType;
    wsaData[!radio].radioType = radioType;          

    if (wsaEnd == NULL) {
        return WSA_UNKNOWN_SECURED_TYPE;
    }
#if !defined(NO_SECURITY)
    if((0x1 == getALSMISignVerifyEnable()) && (gst->WsaSecurity == SECURED_WSA))  {
        if (alsmiSign(wsa, wsaEnd - wsa, NULL, 0, ALSMI_PKT_TYPE_WSA,
                      (void *)&wsaTxSecurity[radio]) == 0) {
            return WSA_SECURED_TYPE;
        }
    }
    else {
        if (alsmiAddUnsecured1609p2Hdr(TRUE, wsa, wsaEnd - wsa,
                              (void *)&wsaTxSecurity[radio]) == 0) {
            return WSA_UNSECURED_TYPE;
        }
    }
#endif
    return WSA_UNKNOWN_SECURED_TYPE;
}

/**
** Function:  providerServiceDeleteReq
** @brief  Process a user service delete request
** @param  service  -- [input]The user service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Set the status to initialized and calls deleteGeneralService() to
**          delete the service.
**/
static rsResultCodeType providerServiceDeleteReq(ProviderServiceType *service,
                                             ASTEntryType    *ASTEntry)
{
    rsResultCodeType risRet; 

    ASTEntry->AppStatus = APP_INITIALIZED;
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DELETE: (pid=%u, radioType=%s, psid=0x%x)\n",
            service->pid,
            "RT_CV2X",
            service->psid);
    risRet = deleteGeneralService(service->pid, service->radioType,
                                service->psid);
    MLMEX_WSAEND_REQ(wsaTxSecurity[service->radioNum].wsaRadio, &wsaData[service->radioNum]);
    /* TODO: original code searched for next service; skipping that for now */

    return risRet;
}

/**
** Function:  providerServiceAddReq
** @brief  Process a user service add request
** @param  service  -- [input]The user service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: First, we make sure we're not already registered. Then we find an
**          empty spot in the General Service Table. Then we copy over the
**          information from the UserServiceType structure to the General
**          Service Table entry. Finally, we schedule whatever is the highest
**          priority service registered as the "current" service.
**/
static rsResultCodeType providerServiceAddReq(ProviderServiceType *service,
                                          ASTEntryType    *ASTEntry)
{
    int i, j;

    /* Make sure we aren't already registered */
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL)     &&
            (GST.GSTEntry[i].PSID      == service->psid)      &&
            (GST.GSTEntry[i].RadioType == service->radioType) &&
            (GST.GSTEntry[i].RadioNum  == service->radioNum)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ADD ERROR: PSID/RadioType/RadioNum (0x%x/%s/%u) has already registered\n",
                    service->psid, "C-V2X", service->radioNum);
            return RS_ENOSRVC;
        }
    }

    /* Find an empty spot and register */
    if (GST.GSTCount >= MAX_SERVICES) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ADD ERROR: Too many services\n");
        return RS_ENOROOM;
    }

    for (i = 0; i < MAX_SERVICES; i++) {    // find available ST entry
        if (GST.GSTEntry[i].ASTEntry == NULL) {
            GST.GSTCount++;
            break;
        }
    }

    if (i >= MAX_SERVICES) { // Add user Services info into GST entries
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ADD ERROR: Exceeded Max ST Entries\n");
        return RS_ENOROOM;
    }

    /* Copy everything over */
    GST.GSTEntry[i].UserService        = FALSE;
    /* 20220511: TODO update for ATLKS */
    GST.GSTEntry[i].cv2xTxInited       = FALSE;
    GST.GSTEntry[i].ASTEntry           = ASTEntry;
    GST.GSTEntry[i].PSID               = service->psid;
    GST.GSTEntry[i].RadioType          = service->radioType;
    GST.GSTEntry[i].RadioNum           = (!service->radioNum);    /* this radio is the radio for the IPv6 service, it needs to be opposite WSA/everything else */
    GST.GSTEntry[i].ServicePriority    = service->servicePriority;
    GST.GSTEntry[i].WsaSecurity        = service->wsaSecurity;
    GST.GSTEntry[i].LengthPsc          = service->lengthPsc;

    if (service->lengthPsc > 0) {
        memcpy(GST.GSTEntry[i].PSC, service->psc, service->lengthPsc);
    }

    memcpy(GST.GSTEntry[i].status.provider.DestMacAddress, service->destMacAddress, LENGTH_MAC);
    GST.GSTEntry[i].ChannelNumber = service->channelNumber;
#if 0
    /* Unit Test build error:
    wme.c:4961:69: error: comparison is always true due to limited range of data type [-Werror=type-limits]
    */
    GST.GSTEntry[i].status.provider.LengthSSP = (service->lengthSSP <= MAX_SECURITY_PERMISSIONS) ? service->lengthSSP : MAX_SECURITY_PERMISSIONS;
#else
    /* service->lengthSSP <= MAX_SECURITY_PERMISSIONS(255) is always true */
    GST.GSTEntry[i].status.provider.LengthSSP = service->lengthSSP;
#endif
    memcpy(GST.GSTEntry[i].status.provider.ServiceSpecificPermissions, service->serviceSpecificPermissions, GST.GSTEntry[i].status.provider.LengthSSP);
    GST.GSTEntry[i].status.provider.LengthSSID = (service->lengthSecurityServiceId <= MAX_SECURITY_SERVICE_ID) ? service->lengthSecurityServiceId : MAX_SECURITY_SERVICE_ID;
    memcpy(GST.GSTEntry[i].status.provider.SecurityServiceId, service->securityServiceId, GST.GSTEntry[i].status.provider.LengthSSID);
    GST.GSTEntry[i].status.provider.edca_count = (service->edca_count <= 4) ? service->edca_count : 4;

    for (j=0; j<GST.GSTEntry[i].status.provider.edca_count; j++) {
        GST.GSTEntry[i].status.provider.txop[j] = service->txop[j];
        GST.GSTEntry[i].status.provider.ecw[j] = service->ecw[j];
        GST.GSTEntry[i].status.provider.aifs[j] = service->aifs[j];
    }

    GST.GSTEntry[i].status.provider.ChannelAccess = service->channelAccess;
    GST.GSTEntry[i].status.provider.RepeatRate = service->repeatRate;
    GST.GSTEntry[i].status.provider.IpService = service->ipService;

    memcpy(GST.GSTEntry[i].status.provider.IPv6Address, service->IPv6Address, LENGTH_IPV6);
    GST.GSTEntry[i].status.provider.IPv6ServicePort = service->IPv6ServicePort;
    memcpy(GST.GSTEntry[i].status.provider.ProviderMacAddress, nsGetSrcMac(!(service->radioNum), 1), LENGTH_MAC);  /* 20220113: verified nsGetSrcMac */
    
    GST.GSTEntry[i].status.provider.RcpiThreshold = service->rcpiThreshold;
    GST.GSTEntry[i].status.provider.WsaCountThreshold = service->wsaCountThreshold;
    GST.GSTEntry[i].status.provider.WsaCountThresholdInterval = service->wsaCountThresholdInterval;
    GST.GSTEntry[i].status.provider.WsaHeaderExtension = service->wsaHeaderExtension;
    GST.GSTEntry[i].status.provider.SignatureLifeTime = service->signatureLifeTime;
    GST.GSTEntry[i].status.provider.advertiseIdLength = service->advertiseIdLength;
    GST.GSTEntry[i].status.provider.latitude2D = service->latitude2D;
    GST.GSTEntry[i].status.provider.longitude2D = service->longitude2D;
    GST.GSTEntry[i].status.provider.latitude3D = service->latitude3D;
    GST.GSTEntry[i].status.provider.longitude3D = service->longitude3D;
    GST.GSTEntry[i].status.provider.elevation3D = service->elevation3D;
    GST.GSTEntry[i].status.provider.confidence3D = service->confidence3D;
    GST.GSTEntry[i].status.provider.semiMajorAccuracy3D = service->semiMajorAccuracy3D;
    GST.GSTEntry[i].status.provider.semiMinorAccuracy3D = service->semiMinorAccuracy3D;
    GST.GSTEntry[i].status.provider.semiMajorOrientation3D = service->semiMajorOrientation3D;
    memcpy(GST.GSTEntry[i].status.provider.advertiseId, service->advertiseId, WSA_ADVERTISE_ID_LEN);
    GST.GSTEntry[i].status.provider.routerLifeTime = service->routerLifeTime;
    memcpy(GST.GSTEntry[i].status.provider.ipPrefix, service->ipPrefix, LENGTH_IPV6);
    GST.GSTEntry[i].status.provider.prefixLen = (service->prefixLen <= LENGTH_IPV6) ? service->prefixLen : LENGTH_IPV6;
    memcpy(GST.GSTEntry[i].status.provider.defaultGateway, service->defaultGateway, DEFAULT_GATEWAY_LEN);
    memcpy(GST.GSTEntry[i].status.provider.primaryDns, service->primaryDns, LENGTH_IPV6);
    memcpy(GST.GSTEntry[i].status.provider.secondaryDns, service->secondaryDns, LENGTH_IPV6);
    memcpy(GST.GSTEntry[i].status.provider.gatewayMacAddress, nsGetSrcMac(!(service->radioNum), 1), LENGTH_MAC);  /* 20220113: verified nsGetSrcMac */

    /* copied from 5910 implementation */
    if (GST.GSTEntry[i].status.provider.ChannelAccess == 0) {
        GST.GSTEntry[i].status.provider.WsaHeaderExtension |= WSA_EXT_GATEWAY_MACADDRESS_BITMASK;
    }

    /*
     * NOTE: "Adaptable" is marked elsewhere in RadioStack as "NOT SUPPORTED"
     *       and so hardcoded as FALSE here.  If implemented, then change to
     *       use nscfg.c function to get current value.
     */
    GST.GSTEntry[i].Adaptable  = FALSE;
    GST.GSTEntry[i].TxPwrLevel = getSCHTxPowerLevel();
    GST.GSTEntry[i].DataRate   = getSCHDataRate();

    ASTEntry->AppStatus = APP_REGISTERED;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServices %s ADD: APP_REGISTERED: GST.GSTEntry[%d]\n",
               __func__, i);
#endif

    /* Schedule which service to run */
    scheduleService(service->radioType, !service->radioNum);

    if (fillWsaReq(&GST.GSTEntry[i], !service->radioNum, service->radioType) == WSA_UNSECURED_TYPE) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSA Security Type: UNSECURED \n");
    }
    return RS_SUCCESS;
}

/**
** Function:  providerServiceReq
** @brief  Process a user service request
** @param  service -- [input]The user service structure associated with the request
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: We look up the entry of the application's PID in the Application
**          Status Table; if not there, we have an error. The application
**          should also be in a state of "initialized", "registered" or
**          "active"; error if not. Then either providerServiceReq() or
**          providerServiceAddReq() is called, depending on whether we want to
**          delete or add the service.
**/
rsResultCodeType providerServiceReq(ProviderServiceType *service)
{
    ASTEntryType *ASTEntry;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"providerServiceReq: action = %s\n", (service->action == ADD) ? "ADD" : "DELETE");
#endif
    ASTEntry = getAppStatusTableEntry(service->pid);

    if (ASTEntry == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: RS_ENOPID (PID not in AST)\n");
        return RS_ENOPID;
    }

    /* App should be Initialized, Registered, or Active state */
    if (!((ASTEntry->AppStatus == APP_INITIALIZED) ||
          (ASTEntry->AppStatus == APP_REGISTERED)) ||
          (ASTEntry->AppStatus == APP_ACTIVE)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: RS_EWRNGSTATE (App not INIT/REG/ACTIVE)\n");
        return RS_EWRNGSTATE;
    }

    switch (service->action) {
    case DELETE:
        return providerServiceDeleteReq(service, ASTEntry);

    case ADD:
        return providerServiceAddReq(service, ASTEntry);

    default:
        return RS_EINVACT;
    }
}

/**
** Function:  userServiceDeleteReq
** @brief  Process a user service delete request
** @param  service  -- [input]The user service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Set the status to initialized and calls deleteGeneralService() to
**          delete the service.
**/
static rsResultCodeType userServiceDeleteReq(UserServiceType *service,
                                             ASTEntryType    *ASTEntry)
{
    ASTEntry->AppStatus = APP_INITIALIZED;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DELETE: (pid=%u, radioType=%s, psid=0x%x)\n",
            service->pid, "RT_CV2X", service->psid);
#endif
    return deleteGeneralService(service->pid, service->radioType,
                                service->psid);
}

/**
** Function:  userServiceAddReq
** @brief  Process a user service add request
** @param  service  -- [input]The user service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: First, we make sure we're not already registered. Then we find an
**          empty spot in the General Service Table. Then we copy over the
**          information from the UserServiceType structure to the General
**          Service Table entry. Finally, we schedule whatever is the highest
**          priority service registered as the "current" service.
**/
static rsResultCodeType userServiceAddReq(UserServiceType *service,
                                          ASTEntryType    *ASTEntry)
{
    int i;

    /* Make sure we aren't already registered */
    for (i = 0; i < MAX_SERVICES; i++) {
        if ((GST.GSTEntry[i].ASTEntry != NULL)     &&
            (GST.GSTEntry[i].PSID      == service->psid)      &&
            (GST.GSTEntry[i].RadioType == service->radioType) &&
            (GST.GSTEntry[i].RadioNum  == service->radioNum)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"userServiceAddReq: ADD ERROR: PSID/RadioType/RadioNum (0x%x/%s/%u) has already registered\n",
                    service->psid, "C-V2X", service->radioNum);
            return RS_ENOSRVC;
        }
    }

    /* Find an empty spot and register */
    if (GST.GSTCount >= MAX_SERVICES) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"userServiceAddReq: ADD ERROR: Too many services\n");
        return RS_ENOROOM;
    }

    for (i = 0; i < MAX_SERVICES; i++) {    // find available ST entry
        if (GST.GSTEntry[i].ASTEntry == NULL) {
            GST.GSTCount++;
            break;
        }
    }

    if (i >= MAX_SERVICES) { // Add user Services info into GST entries
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"userServiceAddReq: ADD ERROR: Exceeded Max ST Entries\n");
        return RS_ENOROOM;
    }

    /* Copy everything over */
    GST.GSTEntry[i].UserService        = TRUE;
    /* 20220511: TODO update for ATLKS */
    GST.GSTEntry[i].cv2xTxInited       = FALSE;
    GST.GSTEntry[i].ASTEntry           = ASTEntry;
    GST.GSTEntry[i].PSID               = service->psid;
    GST.GSTEntry[i].RadioType          = service->radioType;
    GST.GSTEntry[i].RadioNum           = service->radioNum;
    GST.GSTEntry[i].status.user.Access = service->userAccess;
    GST.GSTEntry[i].ServicePriority    = service->servicePriority;
    GST.GSTEntry[i].WsaSecurity        = service->wsaSecurity;
    GST.GSTEntry[i].LengthPsc          = service->lengthPsc;

    if (service->lengthPsc > 0) {
        memcpy(GST.GSTEntry[i].PSC, service->psc, service->lengthPsc);
    }

    GST.GSTEntry[i].ChannelNumber = service->channelNumber;
    memcpy(GST.GSTEntry[i].status.user.SrcMacAddress, service->srcMacAddress, LENGTH_MAC);
    GST.GSTEntry[i].status.user.LengthAdvertisId = service->lengthAdvertiseId;

    if (service->lengthAdvertiseId > 0) {
        memcpy(GST.GSTEntry[i].status.user.AdvertiseId, service->advertiseId, service->lengthAdvertiseId);
    }

    GST.GSTEntry[i].status.user.LinkQuality     = service->linkQuality;
    GST.GSTEntry[i].status.user.ImmediateAccess = service->immediateAccess;
    /* For C-V2X, force ExtendedAccess to 65535 */
    GST.GSTEntry[i].status.user.ExtendedAccess  =
        (service->radioType == RT_CV2X) ? SCH_EXTENDED_ACCESS : service->extendedAccess;

    /*
     * NOTE: "Adaptable" is marked elsewhere in RadioStack as "NOT SUPPORTED"
     *       and so hardcoded as FALSE here.  If implemented, then change to
     *       use nscfg.c function to get current value.
     */
    GST.GSTEntry[i].Adaptable  = FALSE;
    GST.GSTEntry[i].TxPwrLevel = getSCHTxPowerLevel();
    GST.GSTEntry[i].DataRate   = getSCHDataRate();

    ASTEntry->AppStatus = APP_REGISTERED;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServices %s ADD: APP_REGISTERED: GST.GSTEntry[%d]\n",
               __func__, i);
#endif

    /* Schedule which service to run */
    scheduleService(service->radioType, service->radioNum);
    return RS_SUCCESS;
}

/**
** Function:  userServiceReq
** @brief  Process a user service request
** @param  service -- [input]The user service structure associated with the request
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: We look up the entry of the application's PID in the Application
**          Status Table; if not there, we have an error. The application
**          should also be in a state of "initialized", "registered" or
**          "active"; error if not. Then either userServiceReq() or
**          userServiceAddReq() is called, depending on whether we want to
**          delete or add the service.
**/
rsResultCodeType userServiceReq(UserServiceType *service)
{
    ASTEntryType *ASTEntry;
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"userServiceReq:action = %s\n", (service->action == ADD) ? "ADD" : "DELETE");
#endif
    ASTEntry = getAppStatusTableEntry(service->pid);

    if (ASTEntry == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"serServiceReq: ERROR: RS_ENOPID (PID not in AST)\n");
        return RS_ENOPID;
    }

    /* App should be Initialized, Registered, or Active state */
    if (!((ASTEntry->AppStatus == APP_INITIALIZED) ||
          (ASTEntry->AppStatus == APP_REGISTERED)) ||
          (ASTEntry->AppStatus == APP_ACTIVE)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"serServiceReq:ERROR: RS_EWRNGSTATE (App not INIT/REG/ACTIVE)\n");
        return RS_EWRNGSTATE;
    }

    switch (service->action) {
    case DELETE:
        return userServiceDeleteReq(service, ASTEntry);

    case ADD:
        return userServiceAddReq(service, ASTEntry);

    default:
        return RS_EINVACT;
    }
}

/**
** Function:  channelConflicts
** @brief Checks to see if the channel number we are setting the specified
**        radio to conflicts with what the other radio is set to.
** @param  radioNum       -- [input]Radio Number whose channel we are potentially setting
** @param  channelNum     -- [input]The channel number we want to set the above radio to
** @param  extendedAccess -- [input]The extended access we want to set the above radio to
** @return TRUE if we have a conflict, FALSE otherwise
**
** Details: If the other radio is not configured, we don't have a conflict.
**          Otherwise, if we are trying to change to the same channel as the
**          service channel of the other radio, we have a conflict. Otherwise,
**          if we are changing to the same channel as the control channel of
**          the other radio, and the other radio is in alternating mode, we
**          have a conflict. Otherwise, if we are in continuous mode, we don't
**          have a conflict. Otherwise, repeat all of the above tests with our
**          radio's control channel.
**/
static bool_t channelConflicts(uint8_t radioNum, uint8_t channelNum,
                               uint16_t extendedAccess)
{
    uint8_t otherRadio      = (radioNum == 0) ? 1 : 0;
    uint8_t channelsChecked = 0;

    /* If the other radio is not configured, we don't have a conflict */
    if (!radioConfigured[otherRadio]) {
        return FALSE;
    }

    while (channelsChecked < 2) {
        /* If we are setting the channel number to the service channel number of
         * the other radio, we have a conflict */
        if (channelNum == radioConfiguration[otherRadio].channel) {
            return TRUE;
        }

        /* If we are setting the channel number to the control channel number of
         * the other radio, and the other radio is in alternating mode, we have
         * a conflict */
        if ((radioConfiguration[otherRadio].extendedAccess != SCH_EXTENDED_ACCESS) &&
            (channelNum == getCCHNumber(otherRadio))) {
            return TRUE;
        }

        /* If the radio is being set to continuous mode, we are done */
        if (extendedAccess == SCH_EXTENDED_ACCESS) {
            break;
        }

        /* If the radio is being set to alternating mode, check the control
         * channel as well */
        if (++channelsChecked == 1) {
            channelNum = getCCHNumber(radioNum);
        }
    }

    /* We don't have a conflict */
    return FALSE;
}

/**
** Function:  changeUserServiceReq
** @brief  Change the service associated with a PSID to use a different radio
**         configuration
** @param  change -- The parameters to use to configure the radio
** @return RS_SUCCESS for success, error code for error.
**
** Details: If command is CHANGE_ACCESS and the pid is registerd and in the
**          right state, search the GST table for a user service whose PSID
**          matches. If found, and we have no conflict. change the GST table
**          entry with the parameters specified in "change". Then find the
**          highest priority service and use wsuSelectService() to configure
**          the radio.
**/
rsResultCodeType changeUserServiceReq(UserServiceChangeType *change)
{
    ASTEntryType                  *ASTEntry;
    int                            i;
    GeneralServiceStatusEntryType *gst;
    int                            serviceIdx;

    if (change->command != CHANGE_ACCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Comand is not CHANGE_ACCESS\n");
        return RS_SUCCESS;
    }

    if ((ASTEntry = getAppStatusTableEntry(change->pid)) == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: RS_ENOPID (PID not in AST)\n");
        return RS_ENOPID;
    }

    /* App should be Initialized or in Registered state */
    if (!((ASTEntry->AppStatus == APP_INITIALIZED) ||
          (ASTEntry->AppStatus == APP_REGISTERED)) ||
          (ASTEntry->AppStatus == APP_ACTIVE)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Application is in wrong stats (%d)\n",
                ASTEntry->AppStatus);
        return RS_EWRNGSTATE;
    }

    for (i = 0; i < MAX_SERVICES; i++) {
        gst = &GST.GSTEntry[i];

        /* See if GST entry non-empty, PSID matches, and is a user service */
        if ((gst->ASTEntry != NULL) && (gst->RadioType == RT_DSRC) &&
            (gst->PSID == change->psid) && gst->UserService) {
            break;
        }

        /* No service matches */
        if (i == MAX_SERVICES) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No service matches\n");
            return RS_SUCCESS;
        }
    }

    /* Return an error if channel number conflicts with one of the channels
     * the other radio is set to. */
    if (channelConflicts(gst->RadioNum, change->user.access.channelNumber,
                         change->user.access.extendedAccess)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Channel conflicts\n");
        return RS_ECHNLCONFL;
    }

    gst->ChannelNumber              = change->user.access.channelNumber;
    gst->Adaptable                  = change->user.access.adaptable;
    gst->DataRate                   = change->user.access.dataRate;
    gst->TxPwrLevel                 = (uint8_t)
                                      (change->user.access.txPwrLevel * 2.0);
    gst->status.user.Access         = change->user.access.userAccess;
    gst->status.user.ExtendedAccess = change->user.access.extendedAccess;

    /* Get the highest priority service to run */
    gst = getHighestPriorityServices(gst->RadioType, gst->RadioNum,
                                     &serviceIdx);

    if (gst != NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling wsuSelectService(%u, %u, %d)\n",
                gst->RadioType, gst->RadioNum, serviceIdx);
        wsuSelectService(gst->RadioType, gst->RadioNum, serviceIdx, gst);
        ASTEntry->AppStatus = APP_REGISTERED;
    }
    else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"No service selected\n");
        shm_rsk_ptr->wsuNsCurService[gst->RadioType][gst->RadioNum].ServiceRunning = FALSE;
        shm_rsk_ptr->wsuNsCurService[gst->RadioType][gst->RadioNum].Pid            = 0;
    }

    return RS_SUCCESS;
}

/**
** Function:  wsmServiceDeleteReq
** @brief  Process a WSM service delete request
** @param  service  -- [input]The user service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: Calls unregisterWsmService() to unregister the WSM service.
**/
static rsResultCodeType wsmServiceDeleteReq(WsmServiceType *service,
                                            ASTEntryType   *ASTEntry)
{
    /* Unregister the service */
    unregisterWsmService(service->pid, service->radioType, service->psid);
    ASTEntry->AppStatus = APP_INITIALIZED;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServices %s DELETE: (pid=%u, psid=0x%x)\n",
               __func__, service->pid, service->psid);
#endif
    return RS_SUCCESS;
}

/**
** Function:  wsmServiceAddReq
** @brief  Process a WSM service add request
** @param  service  -- [input]The WSM service structure associated with the request
** @param  ASTEntry -- [input]The entry in the AST
**                            associated with the application's pid
** @return RS_SUCCESS if successful. Error code if error.
**
** Details: First, we make sure we're not already registered. Then we register
**          the WSM service in WST via addWST().
**/
static rsResultCodeType wsmServiceAddReq(WsmServiceType *service,
                                         ASTEntryType   *ASTEntry)
{

    /* Add the service to WST */
    if ((addWST(ASTEntry, service->pid, service->radioType,
                service->psid)) == NULL) {
        if (WST.WSMCount >= MAX_SERVICES) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ADD ERROR: Too many services\n");
            return RS_ENOROOM;
        }

        /* Otherwise, app is already registered */
        return RS_ENOSRVC;
    }

    ASTEntry->AppStatus = APP_REGISTERED;
    return RS_SUCCESS;
}

/**
** Function:  wsmServiceReq
** @brief  WSM Service Request -- Register a PSID .
** @param  request -- [input]The WSM Service Request Msg data structure
** @return RS_SUCCESS for success; error code otherwise
**
** Details: If action == DELETE, calls wsmServiceDeleteReq()
**          If action == ADD, calls wsmServiceAddReq()
**          Returns RS_EINVACT otherwise.
**/
rsResultCodeType wsmServiceReq(WsmServiceType *service)
{
    ASTEntryType *ASTEntry = getAppStatusTableEntry(service->pid);
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"action = %s\n", (service->action == ADD) ? "ADD" : "DELETE");
#endif
    /* Get the AST Entry */
    if (ASTEntry == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: RS_ENOPID (PID not in AST)\n");
        return RS_ENOPID;
    }
#if defined(RSU_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pid=0x%x, psid=0x%x [%s]\n", service->pid, service->psid,
            (service->action == ADD) ? "ADD" : "DELETE");
#endif
    switch (service->action) {
    case DELETE:
        return wsmServiceDeleteReq(service, ASTEntry);

    case ADD:
        return wsmServiceAddReq(service, ASTEntry);

    default:
        return RS_EINVACT;
    }
}

/**
** Function:  addExtensionElements
** @brief  Add an extension element to the WSMP header.
** @param  start          -- [output]The starting address of the WSMP extension
**                           element to add.
** @param  wsmpExtensions -- [input]Bitmap specifying which extension elements
**                           to add.
** @param  twPwrLevel     -- [input]If adding a Tx power level extension
**                           element, its value
** @param  channelNumber  -- [input]If adding a channel number extension
**                           element, its value
** @param  dataRate       -- [input]If adding a Tx data rate extension element
**                           its value
** @return The length of the extension element created.
**
** Details: Creates the extension elements in the location pointed to by
**          "start". There are no error returns from this routine.
**/
/*
 * Creates the extension elements in the location pointed to by "start".
 * Returns the length of the extension elements created.
 */
static uint16_t addExtensionElements(uint8_t *start,     uint32_t wsmpExtensions,
                                     uint8_t txPwrLevel, uint8_t channelNumber,
                                     uint8_t dataRate)
{
    uint16_t offset = 0;

    if (wsmpExtensions & WAVE_ID_TRANSMIT_POWER_BITMASK) {
        start[offset++] = WAVE_ID_TRANSMIT_POWER;
        start[offset++] = 1;
        start[offset++] = txPwrLevel;
    }

    if (wsmpExtensions & WAVE_ID_CHANNEL_NUMBER_BITMASK) {
        start[offset++] = WAVE_ID_CHANNEL_NUMBER;
        start[offset++] = 1;
        start[offset++] = channelNumber;
    }

    if (wsmpExtensions & WAVE_ID_DATARATE_BITMASK) {
        start[offset++] =  WAVE_ID_DATARATE;
        start[offset++] =  1;

        if (channelNumber & 1) {
            /* Odd channel is a 20 MHz channel.  Double the data rate in the extension field because the
               units should be in increments of 500 kbps/s.  */
            start[offset++] =  dataRate * 2;
        } else {
            /* Even channel is a 10 MHz channel.  Use specified data rate in the extension field because this
               value is already double the desired data rate value. */
            start[offset++] =  dataRate;
        }
    }

    return offset;
}

/**
** Function:  addPSID
** @brief Add the PSID to the WSMP header
** @param  start   -- [output]place to write the encoded PSID
** @param  psid    -- [input]32-bit value of the PSID
** @param  psidLen -- [output]The place to write the PSID length
** @return RS_SUCCESS for success.
**
** Details: Writes the 1, 2, 3, or 4-byte PSID value to the location pointed to
**          by "start", writes the length of the PSID in bytes to *psidLen, and
**          returns RS_SUCCESS. Returns RS_PSIDENCERR if we get a PSID encoding
**          error.
**/
static rsResultCodeType addPSID(uint8_t *start, uint32_t psid, uint8_t *psidLen)
{
    uint8_t offset = 0;

    if (psid <= PSID_1BYTE_MAX_VALUE) {
        start[offset++] = psid & 0xFF;
    }
    else if (psid >= PSID_2BYTE_MIN_VALUE &&
             psid <= PSID_2BYTE_MAX_VALUE) {
        start[offset++] = (psid >> 8) & 0xFF;
        start[offset++] = psid & 0xFF;
    }
    else if (psid >= PSID_3BYTE_MIN_VALUE &&
             psid <= PSID_3BYTE_MAX_VALUE) {
        start[offset++] = (psid >> 16) & 0xFF;
        start[offset++] = (psid >> 8)  & 0xFF;
        start[offset++] = psid & 0xFF;
    }
    else if (psid >= PSID_4BYTE_MIN_VALUE &&
             psid <= PSID_4BYTE_MAX_VALUE) {
        start[offset++] = (psid >> 24) & 0xFF;
        start[offset++] = (psid >> 16) & 0xFF;
        start[offset++] = (psid >> 8)  & 0xFF;
        start[offset++] = psid & 0xFF;
    }
    else {
        return RS_PSIDENCERR;
    }

    *psidLen = offset;
    return RS_SUCCESS;
}

/**
** Function:  createWSMPHeader
** @brief  Create the WSMP header
** @param  sendWSMStruct -- [input]Metadata associated with the packet we are
**                          createing the WSMP header for.
** @param  hdr           -- [output]Address to write the WSMP header to.
** @param  wsmpHdrLen    -- [output]The length of the WSMP header is written
**                          here. Note that this will NOT include the length of
**                          the data length field.
** @return RS_SUCCESS for success; error code otherwise
**
** Details: Writes out the extension elements if any. Then writes out the TPID
**          for PSID. Then writes out the PSID itself, writes out the length of
**          the WSMP header MINUS THE LEGNTH OF THE DATA LENGTH FIELD, and
**          returns RS_SUCCESS.
**
**          Returns RS_PSIDENCERR if we have a PSID encoding error.
**/
static rsResultCodeType createWSMPHeader(sendWSMStruct_S *sendWSMStruct,
                                         uint8_t *hdr, uint8_t *wsmpHdrLen)
{
    uint8_t          numberExtensions = 0;
    uint32_t         offset = 0;
    /* pwr in 1dB units; + 5 for rounding */
    uint8_t          txPwrLevel = (((sendWSMStruct->txPwrLevel * 5) + 5 +
                                  getConducted2RadiatedPowerOffset(sendWSMStruct->radioNum)) / 10) + 128;
    uint8_t          psidLen;
    rsResultCodeType risRet;

    /* Calcluate number of extensions */
    if (sendWSMStruct->wsmpHeaderExt & WAVE_ID_TRANSMIT_POWER_BITMASK) {
        numberExtensions++;
    }

    if (sendWSMStruct->wsmpHeaderExt & WAVE_ID_CHANNEL_NUMBER_BITMASK) {
        numberExtensions++;
    }

    if (sendWSMStruct->wsmpHeaderExt & WAVE_ID_DATARATE_BITMASK) {
        numberExtensions++;
    }

    /* Output the version number and the optional extension elements */
    if (numberExtensions == 0) {
        hdr[offset++] = WSM_VERSION_3 | WSM_SUBTYPE_NULL_NETWORKING_PROTOCOL;
    }
    else {
        hdr[offset++] = WSM_VERSION_3 | WSM_WSMP_N_EXTENSIONS |
                        WSM_SUBTYPE_NULL_NETWORKING_PROTOCOL;
        /* Extension count */
        hdr[offset++] = numberExtensions;
        /* Output the header extensions */
        /* Change the scaling of Tx power from -128..127 to 0..255 */
        offset += addExtensionElements(&hdr[offset],
                                       sendWSMStruct->wsmpHeaderExt,
                                       txPwrLevel,
                                       sendWSMStruct->channelNumber,
                                       sendWSMStruct->dataRate);
    }

    /* Output the TPID */
    hdr[offset++] = TPID_PSID;

    /* Output the PSID */
    risRet = addPSID(&hdr[offset], sendWSMStruct->psid, &psidLen);

    if (risRet != RS_SUCCESS) {
        RSU_PRINTF_MAX_N_TIMES(10, "risRet = %s\n",
                               rsResultCodeType2Str(risRet));
        return risRet;
    }

    *wsmpHdrLen = offset + psidLen;
    return RS_SUCCESS;
}

/**
** Function:  calcWsmpHdrLength
** @brief  Calculates the length of the WSMP header
** @param  psid           -- [input]The PSID
** @param  wsmpHeaderExt  -- [input]Bitfield indicating which extension
**                           elements are present
** @param  wsmLength      -- [input]The length of the WSM data
** @param  wsmpHdrLen_out -- [output]Number of bytes needed for the WSMP header
** @return RS_SUCCESS on success; error code otherwise
**
** Details: Calculates the length of the WSMP header. Note that this takes
**          into account the length of the PSID as it will appear in the packet
**          when it is sent over the air (1-4 bytes). This is different from
**          the length of the PSID as it appears inside various structures we
**          use (which are often always 4 bytes). Note also the length of the
**          header calculated includes the 1 or 2-byte data length at the end.
**/
static rsResultCodeType calcWsmpHdrLength(uint32_t psid, uint32_t wsmpHeaderExt,
                                          uint16_t wsmLength,
                                          uint8_t *wsmpHdrLen_out)
{
    uint8_t  wsmpHdrLen = 1; // Start off with 1 byte for the WSMP version
    uint32_t wsmpExtensions;
    uint8_t  numberExtensions = 0;

    /*
     * Actual WsmpElement is variable length wsmpHeaderExt contains a bitmask,
     * with one 3-byte WsmpElement field for each bit set in wsmpExtensions
     * (currently 0-3 * 3-byte fields).
     */
    wsmpExtensions = wsmpHeaderExt;

    while (wsmpExtensions) { /* count # of set bits in bitmask */
        if (wsmpExtensions & 0x01) {
            numberExtensions ++;
        }

        wsmpExtensions = wsmpExtensions >> 1;
    }

    if (numberExtensions != 0) {
        wsmpHdrLen++; // Allow room for the header extensions count
        // Reserve bytes for WSMP extension element id usage
        wsmpHdrLen += numberExtensions * sizeof(WsmExtType);
    }

    wsmpHdrLen++; // Allow room for the TPID

    /*
     * Actual PSID is variable length (1-4 bytes). The high-bit(s) of each byte
     * determines if there is another PSID byte to follow. (i.e. No struct
     * field references beyond from PSID field to end-of-struct)
     */
    // 1609.3 defines PSID variable bytes from the b7 b6 b5 b4 bit
    //                    PSID 4 bytes             0  x  x  x
    //                    PSID 3 bytes             1  0  x  x
    //                    PSID 2 bytes             1  1  0  x
    //                    PSID 1 bytes             1  1  1  0
    if (psid <= PSID_1BYTE_MAX_VALUE) {
        wsmpHdrLen += 1;
    }
    else if (psid >= PSID_2BYTE_MIN_VALUE &&
             psid <= PSID_2BYTE_MAX_VALUE) {
        wsmpHdrLen += 2;
    }
    else if (psid >= PSID_3BYTE_MIN_VALUE &&
             psid <= PSID_3BYTE_MAX_VALUE) {
        wsmpHdrLen += 3;
    }
    else if (psid >= PSID_4BYTE_MIN_VALUE &&
             psid <= PSID_4BYTE_MAX_VALUE) {
        wsmpHdrLen += 4;
    }
    else {
        return RS_PSIDENCERR;
    }

    /* Add in the number of bytes for the data length field */
    *wsmpHdrLen_out = wsmpHdrLen + ((wsmLength < 0x80) ? 1 : 2);
    return RS_SUCCESS;
}

/**
** Function:  cv2xTxPcap
** @brief Write a to-be-sent C-V2X packet into the PCAP buffer.
** @param  packet       -- [input]The packet data to be written to the PCAP
**                         buffer. This includes the family ID as the first
**                         byte. The family ID byte will be deleted and be
**                         replaced by the appropriate Ethertype field in the
**                         Ethernet header.
** @param  packetLength -- [input]The length of pcaket.
** @return None.
**
** Details: Extracts the family ID byte. The Ethernet header is
**          written to the front of pcapBuffer. The the packet data, minus the
**          family ID byte, is written. Then the packet is written to the PCAP
**          PCAP buffer by calling pcap_write().
**/
static void cv2xTxPcap(uint8_t *packet, int packetLength)
{
    uint8_t              familyID = *packet;
    /* Allow room for WSM, WSMP header, length, and Ethernet header */
    static uint8_t       pcapBuffer[MAX_WSM_DATA + 18 + ETH_HLEN];
    struct ether_header *ethhdr   = (struct ether_header *)pcapBuffer;

    if ((packetLength < 2) || (packetLength > MAX_WSM_DATA + 18)) {
        /* Return if invalid length */
        return;
    }

    /* Add an Ethernet header to the front of the packet */
    /* VJR WILLBEREVISITED Need upgrade to Qualcomm API to be able to get the
     * source and destination MAC addresses */
    memset(ethhdr->ether_dhost, 0xff, ETH_ALEN); /* dst_mac = broadcast */
    memset(ethhdr->ether_shost, 0xff, ETH_ALEN); /* src_mac = broadcast */
    ethhdr->ether_type = htons((familyID == FAMILY_ID_IEEE) ? ETH_P_WSMP :
                               (familyID == FAMILY_ID_ETSI) ? ETH_P_EU   :
                               /* for unknown familyID */     0x0800);
    /* Copy over the WSM data, minus the family ID byte */
    memcpy(&pcapBuffer[ETH_HLEN], &packet[1], packetLength - 1);
    /* Write to the PCAP buffer */
    pcap_write(pcapBuffer, packetLength + ETH_HLEN - 1, RT_CV2X, 0, 0, 0, 0,
               TRUE);
}

/**
** Function:  cv2x_init_tx_if_necessary
** @brief  Initializes C-V2X transmit if necessary
** @param  sendWSMStruct -- [input]The WSMP metadata associated with the packet
** @return RS_SUCCESS for success; error code otherwise.
**/
//NOTE: first wsm in will determine the peak pwr.
static rsResultCodeType cv2x_init_tx_if_necessary(sendWSMStruct_S *sendWSMStruct)
{
    if (!cv2x_tx_inited) {
        /* If C-V2X power control not enabled, use the default peak Tx
         * power */
        if (!getCV2XPwrCtrlEnable()) {
            /* Passing a peak Tx power value of 0 tells the ns_cv2x module
             * to use the default peak Tx power */
            sendWSMStruct->txPwrLevel = 0;
        }
#ifdef RSU_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling rs_cv2x_init_tx(0x%x, %u, %u)\n", getPrimaryServiceId(),
                MAX_WSM_DATA, sendWSMStruct->txPwrLevel);
#endif
        /* txPwr has been doubled to remove fractional half step of float. 
         * That makes it an odd number so dont divide by 2 or we'll lose half step in int32_t.
         */
        if (rs_cv2x_init_tx(getPrimaryServiceId(), MAX_WSM_DATA, sendWSMStruct->txPwrLevel) < 0) {
            shm_rsk_ptr->CCLError.SentWSMPData[sendWSMStruct->radioType]++;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init_tx(0x%x, %u, %u) == RS_ECV2XRAD\n", 
                getPrimaryServiceId(), MAX_WSM_DATA, sendWSMStruct->txPwrLevel);
            return RS_ECV2XRAD;
        }

        cv2x_tx_inited = TRUE;
    }

    return RS_SUCCESS;
}

/**
** Function:  sendWSMActual
** @brief  Does the actual send of thw WSM packet.
** @param  sendWSMStruct -- [input]The WSMP metadata associated with the packet
** @param  wsm           -- [input]The WSM data to be sent. Note that there
**                          must be wsmpHdrLen bytes free in front of wsm to
**                          add the WSMP header. There must also be 1
**                          additional byte to add the family ID if
**                          transmitting via C-V2X.
** @param  wsmLength     -- [input]The length of the WSM data
** @return RS_SUCCESS for success; error code otherwise.
**
** Details: Does error checking for right WSMP version, if the PSID is
**          registered and if the application is in the right state; returns
**          error codes if not. Otherwise, it adds on the WSMP header and
**          non-IP fmaily ID, and does the physical transmission of the packet.
**
**          It is expected that for packets that use IEEE security, that the
**          security information will be added on to the packet before this
**          routine will be called.
**/
rsResultCodeType sendWSMActual(sendWSMStruct_S *sendWSMStruct,
                               uint8_t *wsm, uint16_t wsmLength)
{
    uint8_t                        wsmpHdrLen;
    uint8_t                       *packet = wsm;
    uint16_t                       packetLength = wsmLength;
    int                            result;
    rsResultCodeType               risRet;
    GeneralServiceStatusEntryType *GSTEntry;
    uint8_t                        hdrLen;
    GeneralServiceStatusEntryType *gst;

#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmLength = %u; sendWSMStruct:\n", wsmLength);
    dumpSendWSMStruct(sendWSMStruct);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsm:\n");
    dump(wsm, wsmLength);
#endif

    /* Calculate the number of bytes necessary for the WSMP header */
    risRet = calcWsmpHdrLength(sendWSMStruct->psid,
                               sendWSMStruct->wsmpHeaderExt,
                               wsmLength, &wsmpHdrLen);

    if (risRet != RS_SUCCESS) {
        RSU_PRINTF_MAX_N_TIMES(10, "risRet = %s\n", rsResultCodeType2Str(risRet));
        return risRet;
    }

    if ((sendWSMStruct->wsmpVersion & WSM_VERSION_MASK) != WSM_VERSION_3) {
        RSU_PRINTF_MAX_N_TIMES(10, "Fail - Wrong WSMP version %d\n",
                               sendWSMStruct->wsmpVersion);

        return RS_ENOSRVC;
    }

    /* Skip the GST tests for P2P and WSAs. */
    if ((sendWSMStruct->psid != WSM_WSA_PSID) && (sendWSMStruct->psid != WSM_P2P_PSID)) {
        /* The PSID should be registered */
        GSTEntry = getGeneralServiceStatusEntry(sendWSMStruct->radioType,
                                                sendWSMStruct->psid);

        if (GSTEntry == NULL) {
            RSU_PRINTF_MAX_N_TIMES(10, "Fail - PSID %d (0x%x) not registered! (RS_ENOSRVC)\n",
                                   sendWSMStruct->psid, sendWSMStruct->psid);
            return RS_ENOSRVC;
        }

        /* The App should be in the proper state */
        if (!((GSTEntry->ASTEntry->AppStatus == APP_REGISTERED)  ||
              (GSTEntry->ASTEntry->AppStatus == APP_INITIALIZED) ||
              (GSTEntry->ASTEntry->AppStatus == APP_ACTIVE))) {
            RSU_PRINTF_MAX_N_TIMES(10, "Fail - App in wrong state %d; PSID = 0x%x (RS_EWRNGSTATE)\n",
                                   GSTEntry->ASTEntry->AppStatus,
                                   sendWSMStruct->psid);
            return RS_EWRNGSTATE;
        }
    }

    /* Count this packet */
    shm_rsk_ptr->CCLCnt.SentWSMPData[sendWSMStruct->radioType]++;

    /* Generate the WSMP header. hdrLen returned will not include the
     * length of the data length field (1 or 2 bytes) */
    packet       -= wsmpHdrLen;
    packetLength += wsmpHdrLen;
    risRet = createWSMPHeader(sendWSMStruct, packet, &hdrLen);

    /* Abort if error */
    if (risRet != RS_SUCCESS) {
        RSU_PRINTF_MAX_N_TIMES(10, "createWSMPHeader() returned %s\n",
                               rsResultCodeType2Str(risRet));
        shm_rsk_ptr->CCLError.SentWSMPData[sendWSMStruct->radioType]++;
        return risRet;
    }
#ifdef RSU_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"createWSMPHeader(): hdrLen = %u\n", hdrLen);
#endif
    /* Put the data length into the packet, and add in the length of the data
     * length field */
    if (wsmLength < 0x80) {
        packet[hdrLen + 0] = wsmLength;
        hdrLen++;
    }
    else {
        packet[hdrLen + 0] = ((wsmLength & 0x7F00) >> 8) | 0x80;
        packet[hdrLen + 1] = (wsmLength & 0xFF);
        hdrLen += 2;
    }

    if (sendWSMStruct->radioType == RT_CV2X) {
        /* Put the family ID into the packet */
        packet--;
        packetLength++;
        packet[0] = FAMILY_ID_IEEE;
        hdrLen++;
    }

    /* Error if the WSM data is not in the right place */
    if (&packet[hdrLen] != wsm) {
        RSU_PRINTF_MAX_N_TIMES(10, "Error: wsmpHdrLen = %u; hdrLen = %u; &packet[hdrLen] = %p; wsm = %p\n",
                               wsmpHdrLen, hdrLen, &packet[hdrLen], wsm);
        shm_rsk_ptr->CCLError.SentWSMPData[sendWSMStruct->radioType]++;
        return RS_ERANGE;
    }

    switch (sendWSMStruct->radioType) {
    case RT_CV2X:
#ifdef RSU_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RT_CV2X\n");
#endif
        gst = getGeneralServiceStatusEntry(RT_CV2X, sendWSMStruct->psid);

        /* Initialize C-V2X transmit if necessary */
        if (gst != NULL) {
            risRet = cv2x_init_tx_if_necessary(sendWSMStruct);

            if (risRet != RS_SUCCESS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_init_tx_if_necessary failed.\n");
                break;
            }
        }

        /* If C-V2X PCAP enabled, write the packet to the PCAP buffer. */
        if (pcap_enabled(RT_CV2X, 0, TRUE)) {
            cv2xTxPcap(packet, packetLength);
        }
#ifdef RSU_DEBUG
        /* Send the packet via C-V2X */
        RSU_DBG_MAX_N_TIMES(10, "result = rs_cv2x_tx(0x%x, packet, %u);\n",getPrimaryServiceId(), packetLength);
#endif
        result = rs_cv2x_tx(getPrimaryServiceId(), packet, packetLength,sendWSMStruct->psid);

        if (result < 0) {
#if defined(EXTRA_DEBUG)
            RSU_PRINTF_MAX_N_TIMES(1000, "Error sending packet via C-V2X: result=%d.\n",result);
#endif
            risRet = result;
        }
#if defined(EXTRA_DEBUG)
        else RSU_DBG_MAX_N_TIMES(10, "Success sending packet via C-V2X.\n");
#endif
        break;
#if defined(ENABLE_DSRC)
    case RT_DSRC:
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RT_DSRC\n");

        /* Send the packet via DSRC */
        RSU_DBG_MAX_N_TIMES(10, "result = dsrc_tx(%d, sendWSMStruct, packet, %u, FALSE);\n",
                            DEV_ID0, packetLength);
        /* DevID for Hercules is always 0 */
        result = dsrc_tx(DEV_ID0, sendWSMStruct, packet, packetLength, FALSE);

        if (result < 0) {
            /* dsrc_tx() will have printed an error message */
            risRet = RS_ESOCKSEND;
        }

        break;
#endif
    default:
        RSU_PRINTF_MAX_N_TIMES(10, "Invalid radioType (%u)\n",
                               sendWSMStruct->radioType);
        risRet = RS_ERANGE;
        break;
    }

    return risRet;
}

/**
** Function:  sendWSM
** @brief  Sends a 1609 WSM
** @param  WSM -- [input]Containings the WSM with meta-data header to be
**                transmitted.
** @return RS_SUCCESS for success; error code otherwise.
**
** Details: Does some error checking and returns an error if found. Otherwise,
**          fills out the sendWSMStruct_S structure.
**
**          If either a security signature or a 1609.2 header is to be added,
**          then the packet is sent to the security module to have the
**          signature or header added. The security module will call sendWSM1()
**          (allowing room for the WSMP header) when it is done.
**
**          Otherwise, the amount of room necesssary for the WSMP header is
**          calculated, the pointer to the packet is backed up enough to allow
**          room for the 1609.2 header plus one byte for the non-IP famild ID,
**          and sendWSM1() is called to transmit the packet.
**/
rsResultCodeType sendWSM(outWSMType *WSM)
{
    sendWSMStruct_S                sendWSMStruct;
    GeneralServiceStatusEntryType *gst = getGeneralServiceStatusEntry(WSM->radioType,
                                                                      WSM->psid);
#ifndef NO_SECURITY
    rsResultCodeType risRet;
    AEROLINK_RESULT                result;
#else
    uint8_t                        buffer[MAX_WSM_DATA + HDRS_LEN];
    uint8_t                       *outData;
    uint8_t                        hdrLen;
#endif

    /* Do some error checking */
    if ((gst == NULL) || (WSM->psid != gst->PSID)) {
#ifdef RSU_DEBUG
        if (gst == NULL) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gst == NULL\n");
        }
        else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSID's don't match: WSM->psid = 0x%x; gst->PSID = 0x%x\n",
                    WSM->psid, gst->PSID);
        }
#endif

        return RS_ENOSRVC;
    }

    /* Fill out sendWSMStruct */
    sendWSMStruct.wsmpVersion     = WSM->version;
    sendWSMStruct.channelNumber   = WSM->channelNumber;
    sendWSMStruct.dataRate        = WSM->dataRate;
    sendWSMStruct.txPwrLevel      = (uint8_t)(WSM->txPwrLevel * 2.0f);

    if (sendWSMStruct.txPwrLevel > (MAX_TX_PWR_LEVEL * 2)) { /*txPwrLevel in 0.5db units, MAX_TX_PWR in 1dB units*/
        /* Ensure TX doesn't exceed maximum conforming power level. */
        sendWSMStruct.txPwrLevel = (MAX_TX_PWR_LEVEL * 2);
    }

    sendWSMStruct.userPriority    = WSM->txPriority;
    sendWSMStruct.expireTime      = WSM->wsmpExpireTime;
    sendWSMStruct.radioType       = WSM->radioType;
    sendWSMStruct.radioNum        = WSM->radioNum;
    sendWSMStruct.channelInterval = WSM->channelInterval;
    sendWSMStruct.security        = WSM->security;
    sendWSMStruct.securityFlag    = WSM->securityFlag;
    memcpy(sendWSMStruct.destMac, WSM->peerMacAddress, LENGTH_MAC);
    sendWSMStruct.wsmpHeaderExt   = WSM->wsmpHeaderExt;
    sendWSMStruct.psid            = WSM->psid;
    sendWSMStruct.eth_proto       = ETH_P_WSMP;
    sendWSMStruct.isBitmappedSsp  = WSM->isBitmappedSsp;
    sendWSMStruct.sspLen          = WSM->sspLen;
    memcpy(sendWSMStruct.sspMask, WSM->sspMask, sizeof(WSM->sspMask));
    memcpy(sendWSMStruct.ssp,     WSM->ssp,     sizeof(WSM->ssp));

#ifndef NO_SECURITY
    if (sendWSMStruct.security != 0) {
        /* The security module will take care of signing the packet and
         * transmitting the packet in the background */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling alsmiSign(%p, %u, NULL, 0, ALSMI_PKT_TYPE_WSMP, %p)\n",
                WSM->data, WSM->dataLength, &sendWSMStruct);
        result = alsmiSign(WSM->data, WSM->dataLength, NULL, 0,
                           ALSMI_PKT_TYPE_WSMP, &sendWSMStruct);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"alsmiSign() returned %s\n", ws_strerror(result));
        return (result == WS_SUCCESS)                            ? RS_SUCCESS                  :
               (result == WS_ERR_UNKNOWN_PACKET_TYPE)            ? RS_ESECUPKTT                :
               (result == WS_ERR_INVALID_PSID)                   ? RS_ESECINVPSID              :
               (result == WS_ERR_PAYLOAD_TOO_LARGE)              ? RS_SECURITY_LENGTH_TOO_LONG :
               (result == WS_ERR_CERTIFICATE_CHANGE_IN_PROGRESS) ? RS_ESECCERTCHGIP            :
               (result == WS_ERR_SIGN_BUFFER_OVERFLOW)           ? RS_ESECSBF                  :
                                                                   RS_ESECSIGNF;
    }

    if (hdr1609p2) {
        /* The security module will take care of adding the 1609.2 header to
         * the packet and transmitting the packet in the background. */
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling alsmiAddUnsecured1609p2Hdr(FALSE, %p, %u, %p, txPwr = %u, %f)\n",
                WSM->data, WSM->dataLength, &sendWSMStruct, sendWSMStruct.txPwrLevel, WSM->txPwrLevel);
        #endif

        if(RS_SUCCESS != (risRet = alsmiAddUnsecured1609p2Hdr(FALSE, WSM->data, WSM->dataLength,&sendWSMStruct))){
            cv2xStatus.wme_tx_not_ready++;
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"alsmiAddUnsecured1609p2Hdr: risRet=%d.\n",risRet);
            #endif
            risRet=RS_ESECA16092F;
        }
        return risRet;
    }
#ifdef RSU_DEBUG
    /* If no 1609.2 header, just send the packet as is */
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling sendWSMActual(%p, %p, %u, txPwr = %u, %f)\n",
            &sendWSMStruct, WSM->data, WSM->dataLength,sendWSMStruct.txPwrLevel, WSM->txPwrLevel);
#endif
    return sendWSMActual(&sendWSMStruct, WSM->data, WSM->dataLength);
#else
    /* Calculate the length of the 1609.2 header */
    if (WSM->dataLength <= 127) {
        hdrLen = 3;
    }
    else if (WSM->dataLength <= 255) {
        hdrLen = 4;
    }
    else {
        hdrLen = 5;
    }

    /* Error if not enough room for the packet */
    if (WSM->dataLength + hdrLen > MAX_WSM_DATA) {
        RSU_PRINTF_MAX_N_TIMES(10, "Packet too large\n");
        return RS_ENOROOM;
    }

    /* Set the address of 1609.2 header. Allow room in front of it for other
     * headers such as the MK5 header, the IEEE QOS header, the SNAP header,
     * and the WSMP header. */
    outData = &buffer[HDRS_LEN];

    /* Add on the 1609.2 header */
    outData[0] = 0x03;
    outData[1] = 0x80;

    if (WSM->dataLength <= 127) {
        outData[2] = WSM->dataLength;
    }
    else if (WSM->dataLength <= 255) {
        outData[2] = 0x81;
        outData[3] = WSM->dataLength;
    }
    else {
        outData[2] = 0x82;
        outData[3] = (WSM->dataLength & 0xff00) >> 8;
        outData[4] = (WSM->dataLength & 0x00ff);
    }

    /* Copy over the packet data itself */
    memcpy(outData + hdrLen, WSM->data, WSM->dataLength);

    /* Transmit the packet */
    return sendWSMActual(&sendWSMStruct, outData, WSM->dataLength + hdrLen);
#endif
}

/**
 * wsuWMEP2PCallback - P2P callback function.
 *
 * @userData: Required by convention; not used
 * @pduLength: Length of the data to send
 * @pdu : Data to send
 *
 * Returns: Non-zero if successful. Zero if error.
 *
 * This function is called by Aerolink when it wants us to send a P2P packet
 * over-the-air. The radio number is set to whichever radio has a service
 * channel defined. The P2P packet is transmitted over the air with a 1609.2
 * header.
 */
uint8_t wsuWMEP2PCallback(void *userData, uint32_t pduLength, uint8_t *pdu)
{
    uint8_t            channel;
    uint8_t            radioNum;
    sendWSMStruct_S    sendWSMStruct;
    #define            MAX_WSMP_HDR_LEN 18
    static uint8_t     p2pBuf[MAX_WSM_DATA + MAX_WSMP_HDR_LEN];
    rsResultCodeType   risRet;

    shm_rsk_ptr->P2PCnt.P2PCallbackCalled++;

    if (pduLength > MAX_WSM_DATA) {
        /* PDU too long */
        shm_rsk_ptr->P2PCnt.P2PCallbackLengthError++;
        return 0;
    }

    /* Don't allow any C-V2X transmission attempts until it has been successfully initialized.
       This avoids the case in which we are receiving messages OTA, failed verification, and want
       to initiate this P2P transmission.  But, we haven't ourselves done any C-V2X WSM (BSM) transmissions yet.
       In this case, we would encounter an error when attempting to transmit this P2P message, so we'll
       just ignore this attempt until a normal C-V2X transmission and initialization is completed. */
    if ((lastVerifyRadioType == RT_CV2X) && !cv2x_tx_inited) {
        return 0;
    }

    /* Determine which radio number and channel to use */
    if (lastVerifyRadioType == RT_CV2X) {
        radioNum = 0;
        channel  = 172; /* Not used */
    }
    else {
        if ((channel = getSCHNumber(0)) != 0) {
            /* Use radio 0 service channel */
            radioNum = 0;
        }
        else if ((channel = getSCHNumber(1)) != 0) {
            /* Use radio 1 service channel */
            radioNum = 1;
        }
        else {
            /* No radio configured; can't send */
            shm_rsk_ptr->P2PCnt.P2PCallbackNoRadioConfigured++;
            return 0;
        }
    }

    /* Initialize the sendWSMStruct structure */
    memset(&sendWSMStruct, 0, sizeof(sendWSMStruct));
    memset(sendWSMStruct.destMac, 0xff, LENGTH_MAC); /* broadcast */
    sendWSMStruct.channelNumber   = channel;
    sendWSMStruct.userPriority    = getP2PPriority();
    sendWSMStruct.channelInterval = getP2PChannelInterval();
    sendWSMStruct.dataRate        = getP2PDataRate();
    /* LLC needs power in 0.5 dB increments */
    sendWSMStruct.txPwrLevel      = getP2PTxPowerLevel();
    if (sendWSMStruct.txPwrLevel > MAX_TX_PWR_LEVEL)
        sendWSMStruct.txPwrLevel = MAX_TX_PWR_LEVEL;
    sendWSMStruct.txPwrLevel     *= 2;
    sendWSMStruct.wsmpVersion     = WSM_VERSION_3;
    sendWSMStruct.radioType       = lastVerifyRadioType;
    sendWSMStruct.radioNum        = radioNum;
    sendWSMStruct.wsmpHeaderExt   = 0;
    sendWSMStruct.psid            = WSM_P2P_PSID;
    sendWSMStruct.eth_proto       = ETH_P_WSMP;
    /* Copy over the data. Note that 18 bytes is the longest a WSMP header can
     * be. */
    memcpy(p2pBuf + MAX_WSMP_HDR_LEN, pdu, pduLength);
    /* Transmit the P2P */
    risRet = sendWSMActual(&sendWSMStruct, p2pBuf + MAX_WSMP_HDR_LEN,
                           pduLength);

    if (risRet != RS_SUCCESS) {
        shm_rsk_ptr->P2PCnt.P2PCallbackSendError++;
        shm_rsk_ptr->P2PCnt.P2PCallbackSendLastErrorCode = risRet;
        return 0;
    }

    shm_rsk_ptr->P2PCnt.P2PCallbackSendSuccess++;
    return 1; /* Success */
}

/*------------------------------------------------------------------------------
** Function:  ignoreWSAsThread
** @brief  Thread to ignore WSA's for the specified number of seconds
** @param  ptr     --Contains the number of seconds to ignore WSA's
** @return void *
**----------------------------------------------------------------------------*/
void *ignoreWSAsThread(void *ptr)
{
    uint16_t seconds = *((uint16_t *)ptr);

    shm_rsk_ptr->bootupState |= 0x20; // ignoreWSAsThread running
    ignoreWSAs = TRUE;
    sleep(seconds);
    ignoreWSAs = FALSE;
    shm_rsk_ptr->bootupState &= ~0x20; // ignoreWSAsThread running
    pthread_exit(NULL);
    return NULL;
}

/**
** Function:  getWBSSInfo
** @brief  Gets information about WBSS in a very large string.
** @param  buf -- [output]Pointer to the buffer containing the very large
**                string to store the information into
** @param  buf_size -- [input]The length of the above buffer
** @return Length of the string generated.
**
** Details: Goes through the AST, GST, and WST tables and stores information
**          about what is in there in a very large string. Used by the "nsstats
**          -w" command.
**/
int getWBSSInfo(char *buf, uint32_t buf_size)
{
    const char *app_state[] = {"Uninitialized", "Initialized  ",
                               "Registered   ", "Active       "};
    char       *buf_ptr = buf;
    int         i, j;
    uint8_t     count;

    for (i = 0; i < MAX_RADIO_TYPES; i++) {
        for (j = 0; j < MAX_RADIOS; j++) {
            if (shm_rsk_ptr->wsuNsCurService[i][j].ServiceRunning) {
                if (shm_rsk_ptr->wsuNsCurService[i][j].UserService) {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "*** User MIB Information ***\n");
                }
                else if (!shm_rsk_ptr->wsuNsCurService[i][j].UserService) {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "*** Provider MIB Information ***\n");
                }
            }
        }
    }

    count = 0;
    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                        "App Cnt: %d,   GlobServ Cnt: %d,   WSM Cnt: %d\n",
                        AST.ASTCount, GST.GSTCount, WST.WSMCount);

    if (AST.ASTCount != 0) {
        buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                            "\tPID\tStatus\t\tPriority\tSAInd\tWSMInd\tIndex\n");

        for (i = 0; i < MAX_APPS; i++) {
            if (AST.ASTEntry[i].pid != 0) {
                count++;

                if (AST.ASTEntry[i].AppStatus < sizeof(app_state) / sizeof(app_state[0])) {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "[%d]\t%d\t%s\t%u\t\t%s\t%s\t%d\n",
                                        count, AST.ASTEntry[i].pid,
                                        app_state[AST.ASTEntry[i].AppStatus],
                                        AST.ASTEntry[i].AppPriority,
                                        AST.ASTEntry[i].serviceAvailableInd ? "true" : "false",
                                        AST.ASTEntry[i].receiveWSMInd ? "true" : "false",
                                        AST.ASTEntry[i].index);
                }
                else {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "[%d]\t%d\t?????????????\n",
                                        count, AST.ASTEntry[i].pid);
                }
            }
        }

        buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr, "\n");
    }

    count = 0;

    if (GST.GSTCount != 0) {
        buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                            "ST \trtype\trad\tPSID\t PID \tPri\tChan\tExtended\tAccess Mode\n");

        for (i = 0; (i < MAX_SERVICES) && (count < GST.GSTCount); i++) {
            if (GST.GSTEntry[i].ASTEntry != NULL) {
                if (GST.GSTEntry[i].UserService) {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "%d\t%s\t%d\t0x%x \t %d \t%d \t%d  \t%d \t\tUser %s\n",
                                        count, "CV2X",
                                        GST.GSTEntry[i].RadioNum, GST.GSTEntry[i].PSID,
                                        GST.GSTEntry[i].ASTEntry->pid, GST.GSTEntry[i].ServicePriority,
                                        GST.GSTEntry[i].ChannelNumber, GST.GSTEntry[i].status.user.ExtendedAccess,
                                        (GST.GSTEntry[i].status.user.Access == 1) ? "UNCOND" : "NO SCH");
                }
                else {
                    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                                        "%d\t%s\t%d\t0x%x \t %d \t%d \t%d  \t%d \t\tProvider\n",
                                        count, "CV2X",
                                        GST.GSTEntry[i].RadioNum, GST.GSTEntry[i].PSID,
                                        GST.GSTEntry[i].ASTEntry->pid, GST.GSTEntry[i].ServicePriority,
                                        GST.GSTEntry[i].ChannelNumber,
                                        (GST.GSTEntry[i].status.provider.ChannelAccess) ? 0: 2);
                }

                count++;
            }
        }

        buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr, "\n");
    }

    count = 0;

    if (WST.WSMCount != 0) {
        buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr,
                            "WSM \trtype\tPSID\t PID\n");

        for (i = 0; (i < MAX_SERVICES) && (count < WST.WSMCount); i++) {
            if ((WST.WSMEntry[i].ASTEntry != NULL)) {
                buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr, "%d\t%s\t0x%x\t %d\n",
                                    count,  "CV2X",
                                    WST.WSMEntry[i].PSID, WST.WSMEntry[i].ASTEntry->pid);
                count++;
            }
        }
    }

    buf_ptr += snprintf(buf_ptr, buf + buf_size - buf_ptr, "\n");
    return buf_ptr - buf + 1;
}

/**
** Function:  getWsaInfo
** @brief  Gets information about a WSA in a very large string.
** @param  buf      -- [output]Pointer to the buffer containing the very large
**                     string to store the information into
** @param  buf_size -- [input]The length of the above buffer
** @param  wsa_num  -- [input]The WSA number
** @return Length of the string generated.
**
** Details: Goes through the WSAStatusTable and stores information about what is in
**          there in a very large string. Used by the "nsstats -x" command.
**/
int getWsaInfo(char *buf, uint32_t buf_size, int wsa_num)
{
    char *buf_ptr = buf;
    int i, len;

    if (wsa_num >= NUM_WSA) {
        return 0;
    }

    if (WSAStatusTable[wsa_num].wsaStatus == WSA_NO) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Status[%d]:                    NONE\n", wsa_num);
        return buf_ptr - buf + 1;
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr,     "Status[%d]:                    %s\n",   wsa_num, WSAStatusTable[wsa_num].wsaStatus == WSA_ACTIVE ? ("ACTIVE"): ("INACTIVE"));
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr,     "Timestamp[%d]:                 %ld, %ld\n",
                                                                                                        wsa_num, WSAStatusTable[wsa_num].receivedTimestamp.tv_sec, WSAStatusTable[wsa_num].receivedTimestamp.tv_usec);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr,     "Version Change[%d]:            %s\n",   wsa_num, WSAStatusTable[wsa_num].versionChange ? ("true"): ("false"));
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr,     "Peer Mac Address[%d]:          %02x:%02x:%02x:%02x:%02x:%02x\n",
                                                                                                        wsa_num, WSAStatusTable[wsa_num].peer_mac_addr[0], WSAStatusTable[wsa_num].peer_mac_addr[1],
                                                                                                                 WSAStatusTable[wsa_num].peer_mac_addr[2], WSAStatusTable[wsa_num].peer_mac_addr[3],
                                                                                                                 WSAStatusTable[wsa_num].peer_mac_addr[4], WSAStatusTable[wsa_num].peer_mac_addr[5]);

    if (WSAStatusTable[wsa_num].versionChange) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Change Count[%d]:              %d\n",   wsa_num, WSAStatusTable[wsa_num].changeCount);
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr,     "WSA Bitmask[%d]:               0x%x\n", wsa_num, WSAStatusTable[wsa_num].WSABitmask);
    if (WSAStatusTable[wsa_num].WSABitmask & WSA_EXT_REPEAT_RATE_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Repeat Rate[%d]:               %d\n",   wsa_num, WSAStatusTable[wsa_num].repeatRate);
    }

    if (WSAStatusTable[wsa_num].WSABitmask & WSA_EXT_2DLOCATION_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "2D Latitude[%d]:               %d\n",   wsa_num, WSAStatusTable[wsa_num].wsa2d.latitude);
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "2D Longitude[%d]:              %d\n",   wsa_num, WSAStatusTable[wsa_num].wsa2d.longitude);
    }

    if (WSAStatusTable[wsa_num].WSABitmask & WSA_EXT_3DLOCATION_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "3D Latitude[%d]:               %d\n",   wsa_num, WSAStatusTable[wsa_num].wsa3d.latitude);
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "3D Longitude[%d]:              %d\n",   wsa_num, WSAStatusTable[wsa_num].wsa3d.longitude);
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "3D Elevation[%d]:              %d\n",   wsa_num, WSAStatusTable[wsa_num].wsa3d.elevation);
    }

    if (WSAStatusTable[wsa_num].WSABitmask & WSA_EXT_ADVERTISER_ID_BITMASK) {
        len = WSAStatusTable[wsa_num].lengthAdvertiseId;
        for(i = 0; i < len; i++) {
            if (WSAStatusTable[wsa_num].advertiseId[i] == 0) {
                len = i;
                break;
            }
        }
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Advertiser ID[%d]:             \"",     wsa_num);

        if ((char *)buf + buf_size - buf_ptr > len) {
            memcpy(buf_ptr, WSAStatusTable[wsa_num].advertiseId, len);
        }
        buf_ptr += len;
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "\"\n");
    }

    return buf_ptr - buf + 1;
}

/**
** Function:  getWsaServiceInfo
** @brief  Gets information about a WSA's service info segment in a very large
**         string.
** @param  buf         -- [output]Pointer to the buffer containing the very
**                        large string to store the information into
** @param  buf_size    -- [input]The length of the above buffer
** @param  wsa_num     -- [input]The WSA number
** @param  service_num -- [input]The service info segment number
** @return Length of the string generated.
**
** Details: Goes through the WSAStatusTable and stores information about what is in
**          there in a very large string. Used by the "nsstats -x" command.
**/
int getWsaServiceInfo(char *buf, uint32_t buf_size, int wsa_num,
                      int service_num)
{
    char *buf_ptr = (char *)buf;
    int i;

    if (wsa_num >= NUM_WSA) {
        return 0;
    }

    if (WSAStatusTable[wsa_num].wsaStatus == WSA_NO) {
        return 0;
    }

    if (service_num >= WSAStatusTable[wsa_num].serviceCount) {
        return 0;
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Service[%d] #%d:\n",                    wsa_num, service_num);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  PSID:                       0x%x\n",  WSAStatusTable[wsa_num].service[service_num].psid);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Channel Index:              %d\n",    WSAStatusTable[wsa_num].service[service_num].channelIndex);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Service Bitmask:            0x%x\n",  WSAStatusTable[wsa_num].serviceBitmask[service_num]);

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  PSC:                        ");

        for(i = 0; i < WSAStatusTable[wsa_num].servicePscLength[service_num]; i++) {
            buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "%02x%c",
                WSAStatusTable[wsa_num].service[service_num].psc[i],
                (i == WSAStatusTable[wsa_num].servicePscLength[service_num] - 1) ? '\n' : ' ');
        }
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_IPV6ADDRESS_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  IPV6 Address:               %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[0],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[1],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[2],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[3],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[4],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[5],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[6],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[7],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[8],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[9],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[10],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[11],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[12],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[13],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[14],
            WSAStatusTable[wsa_num].service[service_num].ipv6Address[15]);
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_SERVICE_PORT_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Service Port:               %d\n",   WSAStatusTable[wsa_num].service[service_num].servicePort);
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Provider Mac Address:       %02x:%02x:%02x:%02x:%02x:%02x\n",
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[0],
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[1],
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[2],
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[3],
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[4],
            WSAStatusTable[wsa_num].service[service_num].providerMacAddress[5]);
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_RCPI_THRESHOLD_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  RCPI Threshold:             %d\n",   WSAStatusTable[wsa_num].service[service_num].rcpi);
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  WSA Count Threshold:        %d\n",   WSAStatusTable[wsa_num].service[service_num].countThreshold);
    }

    if (WSAStatusTable[wsa_num].serviceBitmask[service_num] & WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  WSA Cnt Threshold Interval: %d\n",   WSAStatusTable[wsa_num].service[service_num].countThresholdInterval);
    }

    return buf_ptr - buf + 1;
}

/**
** Function:  getWsaChannelInfo
** @brief  Gets information about a WSA's channel info segment in a very large
**         string.
** @param  buf         -- [output]Pointer to the buffer containing the very
**                        large string to store the information into
** @param  buf_size    -- [input]The length of the above buffer
** @param  wsa_num     -- [input]The WSA number
** @param  channel_idx -- [input]The channel info segment number
** @return Length of the string generated.
**
** Details: Goes through the WSAStatusTable and stores information about what is in
**          there in a very large string. Used by the "nsstats -x" command.
**/
int getWsaChannelInfo(char *buf, uint32_t buf_size, int wsa_num, int channel_idx)
{
    char *buf_ptr = (char *)buf;

    if (wsa_num >= NUM_WSA) {
        return 0;
    }

    if (WSAStatusTable[wsa_num].wsaStatus == WSA_NO) {
        return 0;
    }

    if (channel_idx >= WSAStatusTable[wsa_num].channelCount) {
        return 0;
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Channel Info[%d] #%d:\n",               wsa_num, channel_idx);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Operating Class:            %d\n",    WSAStatusTable[wsa_num].channelInfo[channel_idx].operatingClass);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Channel Number:             %d\n",    WSAStatusTable[wsa_num].channelInfo[channel_idx].channelNumber);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Adaptable:                  %s\n",    WSAStatusTable[wsa_num].channelInfo[channel_idx].adaptable ? ("true"): ("false"));
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Data Rate:                  %d\n",    WSAStatusTable[wsa_num].channelInfo[channel_idx].dataRate);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Tx Power Level:             %d\n",    WSAStatusTable[wsa_num].channelInfo[channel_idx].txPwrLevel);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Channel Bitmask:            0x%x\n",  WSAStatusTable[wsa_num].channelBitmask[channel_idx]);

    if (WSAStatusTable[wsa_num].channelBitmask[channel_idx] & WSA_EXT_EDCA_PARAM_SET_BITMASK) {
        int i;

        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  QoS Info:                   %d\n", WSAStatusTable[wsa_num].channelInfo[channel_idx].edca.qosInfo);

        for (i = 0; i < 4; i++) {
            buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  AIFS[%d]:                    %d\n",   i, WSAStatusTable[wsa_num].channelInfo[channel_idx].edca.aifs[i]);
            buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  ECS[%d]:                     %d\n",   i, WSAStatusTable[wsa_num].channelInfo[channel_idx].edca.ecw[i]);
            buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  TXOP[%d]:                    %d\n",   i, WSAStatusTable[wsa_num].channelInfo[channel_idx].edca.txop[i]);
        }
    }

    if (WSAStatusTable[wsa_num].channelBitmask[channel_idx] & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "  Channel Access:             %d (%s)\n",   WSAStatusTable[wsa_num].channelInfo[channel_idx].channelAccess,
                                                                                                                    (WSAStatusTable[wsa_num].channelInfo[channel_idx].channelAccess == 2) ? "alternating slot 1" :
                                                                                                                    (WSAStatusTable[wsa_num].channelInfo[channel_idx].channelAccess == 1) ? "alternating slot 0 (not supported)" :
                                                                                                                    (WSAStatusTable[wsa_num].channelInfo[channel_idx].channelAccess == 0) ? "continuous" : "?");
    }

    return buf_ptr - buf + 1;
}

/**
** Function:  getWsaWraInfo
** @brief  Gets information about a WSA's WRA segment in a very large string.
** @param  buf      -- [output]Pointer to the buffer containing the very large
**                     string to store the information into
** @param  buf_size -- [input]The length of the above buffer
** @param  wsa_num  -- [input]The WSA number
** @return Length of the string generated.
**
** Details: Goes through the WSAStatusTable and stores information about what is in
**          there in a very large string. Used by the "nsstats -x" command.
**/
int getWsaWraInfo(char *buf, uint32_t buf_size, int wsa_num)
{
    char *buf_ptr = (char *)buf;

    if (wsa_num >= NUM_WSA) {
        return 0;
    }

    if (WSAStatusTable[wsa_num].wsaStatus == WSA_NO) {
        return 0;
    }

    if (!(WSAStatusTable[wsa_num].routingBitmask & WSA_EXT_WRA_BITMASK)) {
        return 0;
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Router Lifetime[%d]:           %d\n",   wsa_num, WSAStatusTable[wsa_num].routingInfo.routerLifeTime);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "IP Prefix[%d]:                 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",   wsa_num,
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[0],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[1],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[2],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[3],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[4],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[5],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[6],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[7],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[8],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[9],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[10],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[11],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[12],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[13],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[14],
        WSAStatusTable[wsa_num].routingInfo.ipPrefix[15]);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Prefix Length[%d]:             %d\n",   wsa_num, WSAStatusTable[wsa_num].routingInfo.prefixLen);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Default Gateway[%d]:           %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",   wsa_num,
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[0],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[1],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[2],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[3],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[4],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[5],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[6],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[7],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[8],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[9],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[10],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[11],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[12],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[13],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[14],
        WSAStatusTable[wsa_num].routingInfo.defaultGateway[15]);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Primary DNS[%d]:               %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",   wsa_num,
        WSAStatusTable[wsa_num].routingInfo.primaryDns[0],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[1],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[2],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[3],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[4],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[5],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[6],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[7],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[8],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[9],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[10],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[11],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[12],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[13],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[14],
        WSAStatusTable[wsa_num].routingInfo.primaryDns[15]);
    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "WRA Bitmask[%d]:               0x%x\n", wsa_num, WSAStatusTable[wsa_num].routingBitmask);

    if (WSAStatusTable[wsa_num].routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Secondary DNS[%d]:             %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",   wsa_num,
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[0],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[1],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[2],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[3],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[4],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[5],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[6],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[7],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[8],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[9],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[10],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[11],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[12],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[13],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[14],
            WSAStatusTable[wsa_num].routingInfo.secondaryDns[15]);
    }

    if (WSAStatusTable[wsa_num].routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK) {
        buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "Gateway MAC Address[%d]:       %02x:%02x:%02x:%02x:%02x:%02x\n",   wsa_num,
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[0],
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[1],
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[2],
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[3],
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[4],
            WSAStatusTable[wsa_num].routingInfo.gatewayMacAddress[5]);
    }

    buf_ptr += snprintf(buf_ptr, (char *)buf + buf_size - buf_ptr, "\n");
    return buf_ptr - buf + 1;
}

/*------------------------------------------------------------------------------
** Function:  sendRawPacket
** @brief  Does the actual raw Packet send.
** @param  packet -- [input]Pointer to to the outRawType structure containing
**                          the packet to be transmitted. Note that this data
**                          must contain 802.11 headers in order to be
**                          transmitted and received properly.
** @return RS_SUCCESS for success, failure otherwise.
**----------------------------------------------------------------------------*/
rsResultCodeType sendRawPacket(outRawType *packet)
{
    sendWSMStruct_S   sendWSMStruct;
    rsResultCodeType  risRet;
    int               result = -1;
    uint8_t          *packet_data    = packet->data;
    uint16_t          packet_dataLen = packet->dataLen;

    if (packet->radioType == RT_CV2X) {
        /* Initialize C-V2X transmit if necessary */
        risRet = cv2x_init_tx_if_necessary(&sendWSMStruct);

        if (risRet != RS_SUCCESS) {
            return risRet;
        }
        /* Since this is raw test packet just assume BSM, doesn't matter. */
        result = rs_cv2x_tx(getPrimaryServiceId(), packet_data, packet_dataLen,0x20);
    }

    if (result < 0) {
        RSU_PRINTF_MAX_N_TIMES(10, "Error sending raw packet via %s\n",
                               (packet->radioType == RT_CV2X) ? "C-V2X" : "DSRC");
        return RS_ESOCKSEND;
    }

    return RS_SUCCESS;
}
