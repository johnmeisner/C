/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: wsm_recv.c                                                       */
/*  Purpose: Test program that receives WSMs                                  */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-04-04  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include "ris.h"
#include "tps_api.h"
#include "alsmi_api.h"
#include "vod_api.h"
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define MAX_CMD_LENGTH 256
#define WSM_PSID       0x20

/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/
#define ABS(x) (x) < 0 ? -(x) : (x)

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef struct {
    uint8_t  WsmData[8];
    uint32_t wsmSequenceNumber;
    uint32_t WsmSendSec;
    uint32_t WsmSendMicrosec;
    int32_t  latitude;
    int32_t  longitude;
    uint16_t altitude;
    uint8_t  tpsError;
    uint8_t  IPCharAddr[20];
} __attribute__((packed)) InWsmMsgData;

typedef struct {
    uint32_t unsecured;
    uint32_t securedButNotVerified;
    uint32_t securedVerifiedSuccess;
    uint32_t securedVerifiedFail;
    uint32_t securedVernNotNeeded;
    uint32_t other;
} securityStatusCounterT;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/
static void service_available(serviceInfoType *serviceInfo);
static void receive_wsm(inWSMType *Wsm);
static void receive_wsmDebug(inWSMType *Wsm);
static void logpFrameLogFileName(void);

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static char                    logfilename[80];
static rsRadioType             wsmRadioType          = RT_CV2X;
static uint8_t                 wsmRadio              = 0;
#ifdef CHINA_FREQS_ONLY
static int                     wsmRadio0Channel      = 153;
static int                     wsmRadio1Channel      = 153;
#else
static int                     wsmRadio0Channel      = 172;
static int                     wsmRadio1Channel      = 172;
#endif
static bool_t                  reduceCPULoading      = FALSE;
static FILE                   *logpLogFileP;
static char                   *filePath              = NULL;
static tpsDataType             tpsUserData           = {0};
static int                     wsmDebug              = 0;
static uint8_t                 IPCharAddr[20];
static uint8_t                 localTpsError         = 1;
static uint8_t                 callbackError         = 0;
static double                  deltaDist             = 0;
static bool_t                  mainLoop              = TRUE;
static uint32_t                wsmReceivedPkt[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]   = {{{0}}};
static uint32_t                wsmMissedPkt[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]     = {{{0}}};
static uint32_t                wsmOutofOrderPkt[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES] = {{{0}}};
static uint32_t                pktSendSec;
static uint32_t                pktSendMicrosec;
static double                  wsmTotalRssi          = 0;
static double                  curRssi               = 0;
static int32_t                 wsmTotalRssiCount     = 0;
static double                  wsmTotalRssiAntA      = 0;
static double                  curRssiAntA           = 0;
static int32_t                 wsmTotalRssiCountAntA = 0;
static double                  wsmTotalRssiAntB      = 0;
static double                  curRssiAntB           = 0;
static int32_t                 wsmTotalRssiCountAntB = 0;
static uint32_t                wsmSequenceNumber[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]      = {{{0}}};
static uint32_t                wsmPrevSequenceNumber[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]  = {{{0}}};
static bool_t                  wsmFirstPacketReceived[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES] = {{{FALSE}}};
static bool_t                  wsmProvider           = FALSE;
static bool_t                  wBSSJoined            = FALSE;
static int8_t                  wsmVersion;
static int8_t                  wsmRss[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES];
static float                   wsmRssAntA[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES];
static float                   wsmRssAntB[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES];
static float                   wsmNoiseAntA[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES];
static float                   wsmNoiseAntB[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES];
static uint8_t                 wsmChannelNumber;
static dataRateType            wsmDataRate;
static uint8_t                 wsmUserPriority;
static uint16_t                wsmRecvPlcpLength;
static uint32_t                wsmPSID               = WSM_PSID;
static uint16_t                wsmLog                = 0;
static uint8_t                 wsmPerformance        = 0;
static uint16_t                wsmDisplay            = 1;
static uint16_t                wsmRxAnt              = 1;
static bool_t                  wsmPktLog             = TRUE;
static uint16_t                wsmStatsLog           = 1000;
static uint16_t                wsmDisableRadio       = 999;
static float                   wsmLatency[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]       = {{{0}}};
static float                   wsmMaxLatency[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]    = {{{0}}};
static float                   wsmMinLatency[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES]    = {{{9999999}}};  //Must initialize this at run time
static double                  totalElapsedTime[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES] = {{{0}}};
static uint32_t                testStartTime         = 0;
static char                    wsmStr[7]             = {0};
static bool_t                  wsmAccum              = TRUE;
static struct itimerspec       dispTmrTv;
static struct itimerspec       pollRadioTmrTv;
static timer_t                 dispTmrId;
static timer_t                 pollRadioTmrId;
static bool_t                  dispTmrCreated        = FALSE;
static bool_t                  pollRadioTmrCreated   = FALSE;
static double                  wsmLatitude[MAX_RADIO_TYPES][MAX_RADIOS]  = {{0}};
static double                  wsmLongitude[MAX_RADIO_TYPES][MAX_RADIOS] = {{0}};
static double                  wsmAltitude[MAX_RADIO_TYPES][MAX_RADIOS]  = {{0}};
static uint8_t                 wsmTpsError[MAX_RADIO_TYPES][MAX_RADIOS]  = {{1}};
static uint32_t                rawChanBusyRatio      = 0;
static uint32_t                waveCCHBusyRatio      = 0;
static uint32_t                waveSCHBusyRatio      = 0;
static uint32_t                displayPSID           = 0;
static uint32_t                firstErrorPSID        = 0;
static securityStatusCounterT  securityStatusCounter = {0, 0, 0, 0, 0, 0};
static bool_t                  secEnabled            = FALSE;
static bool_t                  connectedToSM         = FALSE;
static bool_t                  smiDebug              = FALSE;
static bool_t                  useVOD                = FALSE;
static bool_t                  vernRes               = FALSE;
static uint32_t                VODMsgSeqNum;
static int8_t                  VODRequestHandle      = -1;
static pthread_t               VODThreadId;
static sem_t                   VOD_sem;
static uint32_t                secsPerVerification   = 1;
static uint32_t                vern_fail_cnt;
static uint32_t                wsmReceivedBytes[MAX_RADIO_TYPES][MAX_RADIOS][MAX_SERVICES] = {{{0}}};
static uint32_t                wsmRxPktGoodPrePeriod = 0;
static uint32_t                wsmRxPktBadPrePeriod  = 0;
static uint32_t                wsmReceivedPreBytes   = 0;
static uint8_t                 wBSSMacAddress[6];
static bool_t                  showAntRss            = FALSE;

static appCredentialsType appCredential =
{
    .cbServiceAvailable = service_available,
#if defined(NO_SECURITY)
    .cbReceiveWSM       = receive_wsmDebug,
#else
    .cbReceiveWSM       = receive_wsm,
#endif
};

static appCredentialsType appCredentialDebug =
{
    .cbServiceAvailable = service_available,
    .cbReceiveWSM       = receive_wsmDebug,
};

static WsmServiceType service;

static ProviderServiceType pService =
{
    .radioType                 = RT_CV2X,
    .radioNum                  = 0,
    .action                    = ADD,
    .destMacAddress            = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .wsaSecurity               = UNSECURED_WSA,
    .psid                      = WSM_PSID,
    .lengthPsc                 = 4,
    .psc                       = "Pro1",
    .servicePriority           = 20,
#ifdef CHINA_FREQS_ONLY
    .channelNumber             = 153,
#else
    .channelNumber             = 184,
#endif
    .channelAccess             = 2,
    .repeatRate                = 32,
    .ipService                 = 0,
    .IPv6ServicePort           = 23000,
    .providerMACAddress        = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    .rcpiThreshold             = 10,
    .wsaCountThreshold         = 20,
    .wsaCountThresholdInterval = 30,
};

static UserServiceType uService = {
    .radioType         = RT_CV2X,
    .radioNum          = 0,
    .action            = ADD,
    .userAccess        = AUTO_ACCESS_UNCONDITIONAL,
    .psid              = WSM_PSID,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
#ifdef CHINA_FREQS_ONLY
    .channelNumber     = 153,
#else
    .channelNumber     = 178,
#endif
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 65535
};
static void initStatic(void)
{
  memset(logfilename,0x0,sizeof(logfilename));
  wsmRadioType          = RT_CV2X;
  wsmRadio              = 0;
  wsmRadio0Channel      = 172;
  wsmRadio1Channel      = 172;
  reduceCPULoading      = FALSE;
  logpLogFileP = NULL;
  filePath              = NULL;
  memset(&tpsUserData,0x0,sizeof(tpsUserData));
  wsmDebug              = 0;
  memset(IPCharAddr,0x0,sizeof(IPCharAddr));
  localTpsError         = 1;
  callbackError         = 0;
  deltaDist             = 0.0;
  mainLoop              = TRUE;
  memset(wsmReceivedPkt,0,sizeof(wsmReceivedPkt));
  memset(wsmMissedPkt,0,sizeof(wsmMissedPkt));
  memset(wsmOutofOrderPkt,0,sizeof(wsmOutofOrderPkt));
  pktSendSec = 0;
  pktSendMicrosec = 0;
  wsmTotalRssi          = 0;
  curRssi               = 0;
  wsmTotalRssiCount     = 0;
  wsmTotalRssiAntA      = 0;
  curRssiAntA           = 0;
  wsmTotalRssiCountAntA = 0;
  wsmTotalRssiAntB      = 0;
  curRssiAntB           = 0;
  wsmTotalRssiCountAntB = 0;
  memset(wsmSequenceNumber,0,sizeof(wsmSequenceNumber));
  memset(wsmPrevSequenceNumber,0,sizeof(wsmPrevSequenceNumber));
  memset(wsmFirstPacketReceived,0,sizeof(wsmFirstPacketReceived));
   wsmProvider           = FALSE;
  wBSSJoined            = FALSE;
  wsmVersion = 0;
  memset(wsmRss,0,sizeof(wsmRss));
  memset(wsmRssAntA,0,sizeof(wsmRssAntA));
  memset(wsmRssAntB,0,sizeof(wsmRssAntB));
  memset(wsmNoiseAntA,0,sizeof(wsmNoiseAntA));
  memset(wsmNoiseAntB,0,sizeof(wsmNoiseAntB));
  wsmChannelNumber = 0;
  memset(&wsmDataRate,0x0,sizeof(wsmDataRate));
  wsmUserPriority = 0;
  wsmRecvPlcpLength = 0;
  wsmPSID               = WSM_PSID;
  wsmLog                = 0;
  wsmPerformance        = 0;
  wsmDisplay            = 1;
  wsmRxAnt              = 1;
  wsmPktLog             = TRUE;
  wsmStatsLog           = 1000;
  wsmDisableRadio       = 999;
  memset(wsmLatency,0,sizeof(wsmLatency));
  memset(wsmMaxLatency,0,sizeof(wsmMaxLatency));
  memset(wsmMinLatency,999999,sizeof(wsmMinLatency)); //Must initialize this at run time
  memset(totalElapsedTime,0,sizeof(totalElapsedTime));
  testStartTime         = 0;
  memset(wsmStr,0,sizeof(wsmStr));
  wsmAccum              = TRUE;
  memset(&dispTmrTv,0,sizeof(dispTmrTv));
  memset(&pollRadioTmrTv,0,sizeof(pollRadioTmrTv));
  memset(&dispTmrId,0,sizeof(dispTmrId));
  memset(&pollRadioTmrId,0,sizeof(pollRadioTmrId));
  dispTmrCreated        = FALSE;
  pollRadioTmrCreated   = FALSE;
  memset(wsmLatitude,0,sizeof(wsmLatitude));
  memset(wsmLongitude,0,sizeof(wsmLongitude));
  memset(wsmAltitude,0,sizeof(wsmAltitude));
  memset(wsmTpsError,0,sizeof(wsmTpsError));
  rawChanBusyRatio      = 0;
  waveCCHBusyRatio      = 0;
  waveSCHBusyRatio      = 0;
  displayPSID           = 0;
  firstErrorPSID        = 0;
  memset(&securityStatusCounter,0,sizeof(securityStatusCounter));
  secEnabled            = FALSE;
  connectedToSM         = FALSE;
  smiDebug              = FALSE;
  useVOD                = FALSE;
  vernRes               = FALSE;
  VODMsgSeqNum = 0;
  VODRequestHandle      = -1;
//static pthread_t               VODThreadId;
//static sem_t                   VOD_sem;
  secsPerVerification   = 1;
  vern_fail_cnt = 0;
  memset(wsmReceivedBytes,0,sizeof(wsmReceivedBytes));
  wsmRxPktGoodPrePeriod = 0;
  wsmRxPktBadPrePeriod  = 0;
  wsmReceivedPreBytes   = 0;
  memset(wBSSMacAddress,0x0,sizeof(wBSSMacAddress));
  showAntRss            = FALSE;
}
static double getRelativeDistance(double lat1, double lat2, double lon1, double lon2)
{
    double x1, y1, z1;
    double x2, y2, z2;
    double c, s;
    double a = 6378137;
    double f = 0.003353;
    double alt1, alt2;

    lat1 += 180;
    lat2 += 180;
    lon1 += 180;
    lon2 += 180;
    alt1 = 0;   // remove altitude since GPS HAE measurements are volatile
    alt2 = 0;

    c = 1 / sqrt((pow(cos(lat1 / 180.0 * M_PI), 2)) +
            pow(1 - f, 2) * (pow(sin(lat1 / 180.0 * M_PI), 2)));

    s = pow(1 - f, 2) * c;

    x1 = (a * c + alt1) * cos(lat1 / 180.0 * M_PI) * cos(lon1 / 180.0 * M_PI);
    y1 = (a * c + alt1) * cos(lat1 / 180.0 * M_PI) * sin(lon1 / 180.0 * M_PI);
    z1 = (a * s + alt1) * sin(lat1 / 180.0 * M_PI);

    c = 1 / sqrt(((pow(cos(lat2 / 180.0 * M_PI), 2))) +
            pow(1 - f, 2) * (pow(sin(lat2 / 180.0 * M_PI), 2)));

    s = pow(1 - f, 2) * c;

    x2 = (a * c + alt2) * cos(lat2 / 180.0 * M_PI) * cos(lon2 / 180.0 * M_PI);
    y2 = (a * c + alt2) * cos(lat2 / 180.0 * M_PI) * sin(lon2 / 180.0 * M_PI);
    z2 = (a * s + alt2) * sin(lat2 / 180.0 * M_PI);

    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

void WsmDisplayHandler(int signal)
{
    struct timeval  currTime;
    struct tm      *timeptr;
    char            gBuf[1000];
    char           *g_buf = &gBuf[0];
    float           success[MAX_RADIO_TYPES][MAX_RADIOS];
    int             i, j;
    unsigned long   svcIdx=displayPSID-wsmPSID;

    memset(gBuf,0x0,sizeof(gBuf));
    gettimeofday(&currTime, NULL);
    timeptr = localtime(&currTime.tv_sec);

    if (strftime(g_buf, 10, "%T", timeptr) == 0) {
        perror("printStatsLogFile-strftime");
        return;
    }

    g_buf += 8;

    for (i = 0; i < MAX_RADIO_TYPES; i++) {
        for (j = 0; j < MAX_RADIOS; j++) {
            if (wsmReceivedPkt[i][j][svcIdx]==0 && wsmMissedPkt[i][j][svcIdx] ==0) {
                success[i][j] = 0;
            }
            else {
                success[i][j]= ((float)wsmReceivedPkt[i][j][svcIdx] /
                             ((float)wsmReceivedPkt[i][j][svcIdx] + (float)wsmMissedPkt[i][j][svcIdx])) * 100;
            }
        }
    }

    /* Obtain configuration and mode information */
    if (!wsmProvider) { // User mode
        sprintf(&gBuf[8], " Radio=%s %d,Chan=%d,PSID=0x%4x,Ext=%5d, PlcpLen=%d",
                (wsmRadioType == RT_CV2X) ? "C-V2X" : "DSRC",
                wsmRadio, wsmChannelNumber, displayPSID, uService.extendedAccess, wsmRecvPlcpLength);
        sprintf(&gBuf[strlen(gBuf)], " User Mode, WBSSJoined=%d ", wBSSJoined);
    } else {
        sprintf(&gBuf[8], " Radio=%s %d,Chan=%d,PSID=0x%4x,Ext=%5d, PlcpLen=%d",
                (wsmRadioType == RT_CV2X) ? "C-V2X" : "DSRC",
                wsmRadio, wsmChannelNumber, displayPSID, pService.channelAccess, wsmRecvPlcpLength);
        sprintf(&gBuf[strlen(gBuf)], " Provider Mode ");
    }

    /* Obtain PER information */
    if ((wsmReceivedPkt[wsmRadioType][0][svcIdx] != 0) && (wsmReceivedPkt[wsmRadioType][1][svcIdx] != 0)) {
        /* We're receiving packets from both radios, so display info for both */
        sprintf(&gBuf[strlen(gBuf) - 1],
                " ServRadio=%s %d, SPri=%d, Rad0Seq#=%u, Rad1Seq#=%u,"
                " Rad0MisPkt=%u, Rad1MisPkt=%u, Rad0OorPkt=%u, Rad1OorPkt=%u, DataRate=%d, WsmPri=%d",
                (uService.radioType == RT_CV2X) ? "C-V2X" : "DSRC",
                uService.radioNum, uService.servicePriority,
                wsmSequenceNumber[wsmRadioType][0][svcIdx], wsmSequenceNumber[wsmRadioType][1][svcIdx],
                wsmMissedPkt[wsmRadioType][0][svcIdx], wsmMissedPkt[wsmRadioType][1][svcIdx],
                wsmOutofOrderPkt[wsmRadioType][0][svcIdx], wsmOutofOrderPkt[wsmRadioType][1][svcIdx], wsmDataRate, wsmUserPriority);
        sprintf(&gBuf[strlen(gBuf) - 1],
                "  Rad0RcvPkt=%u, Rad1RcvPkt=%u,  Rad0Success=%4.2f%%, Rad1Success=%4.2f%%, Rad0Rss=%d, Rad1Rss=%d, ",
                wsmReceivedPkt[wsmRadioType][0][svcIdx], wsmReceivedPkt[wsmRadioType][1][svcIdx], success[wsmRadioType][0], success[wsmRadioType][1],
                wsmRss[wsmRadioType][0][svcIdx], wsmRss[wsmRadioType][1][svcIdx]);

        if (showAntRss) {
#ifdef INCLUDE_ANTAB_NOISE
            sprintf(&gBuf[strlen(gBuf) - 1], " rssAntA=%5.1f, rssAntB=%5.1f, noiseAntA=%5.1f, noiseAntB=%5.1f",
                    wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx],
                    wsmNoiseAntA[wsmRadioType][wsmRadio][svcIdx], wsmNoiseAntB[wsmRadioType][wsmRadio][svcIdx]);
#else
            sprintf(&gBuf[strlen(gBuf) - 1], " rssAntA=%5.1f, rssAntB=%5.1f",
                    wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx]);
#endif
        }
    }
    else {
        /* Just display information for the configured radio */
        sprintf(&gBuf[strlen(gBuf) - 1],
                " Seq#=%u, SRadio=%s %d, SPri=%d, MisPkt=%u, OorPkt=%u, DataRate=%d, wsmPri=%d ",
                wsmSequenceNumber[wsmRadioType][wsmRadio][svcIdx],
                (uService.radioType == RT_CV2X) ? "C-V2X" : "DSRC",
                uService.radioNum, uService.servicePriority,
                wsmMissedPkt[wsmRadioType][wsmRadio][svcIdx],
                wsmOutofOrderPkt[wsmRadioType][wsmRadio][svcIdx], wsmDataRate, wsmUserPriority);
        sprintf(&gBuf[strlen(gBuf) - 1],
                " RcvPkt=%u, Success=%4.2f%%, Rss=%d, ",
                wsmReceivedPkt[wsmRadioType][wsmRadio][svcIdx], success[wsmRadioType][wsmRadio], wsmRss[wsmRadioType][wsmRadio][svcIdx]);

        if (showAntRss) {
#ifdef INCLUDE_ANTAB_NOISE
            sprintf(&gBuf[strlen(gBuf) - 1], " rssAntA=%5.1f, rssAntB=%5.1f, noiseAntA=%5.1f, noiseAntB=%5.1f",
                    wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx],
                    wsmNoiseAntA[wsmRadioType][wsmRadio][svcIdx], wsmNoiseAntB[wsmRadioType][wsmRadio][svcIdx]);
#else
            sprintf(&gBuf[strlen(gBuf) - 1], " rssAntA=%5.1f, rssAntB=%5.1f",
                    wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx]);
#endif
        }
    }

    /* Obtain other performance information such as latency, position, and channel busy.  Note that for
       position information, only WSMs from the "configured" radio is displayed. */
    if (wsmPerformance != 0) {
        if (wsmPerformance != 3) {
            /* Display latency info */
            if ((wsmReceivedPkt[wsmRadioType][0][svcIdx] !=0) && (wsmReceivedPkt[wsmRadioType][1][svcIdx] !=0)) {
                sprintf(&gBuf[strlen(gBuf) - 1], " Rad0Latency=(%4.2f, %4.2f, %4.2f) ms, ",
                        wsmMinLatency[wsmRadioType][0][svcIdx]/1000, wsmLatency[wsmRadioType][0][svcIdx]/1000, wsmMaxLatency[wsmRadioType][0][svcIdx]/1000);
                sprintf(&gBuf[strlen(gBuf) - 1], " Rad1Latency=(%4.2f, %4.2f, %4.2f) ms, ",
                        wsmMinLatency[wsmRadioType][1][svcIdx]/1000, wsmLatency[wsmRadioType][1][svcIdx]/1000, wsmMaxLatency[wsmRadioType][1][svcIdx]/1000);
            }
            else {
                sprintf(&gBuf[strlen(gBuf) - 1], " Latency=(%4.2f, %4.2f, %4.2f) ms, ",
                        wsmMinLatency[wsmRadioType][wsmRadio][svcIdx]/1000, wsmLatency[wsmRadioType][wsmRadio][svcIdx]/1000,
                        wsmMaxLatency[wsmRadioType][wsmRadio][svcIdx]/1000);
            }
        }

        /* Display position info */
        if (wsmTpsError[wsmRadioType][wsmRadio] || localTpsError) {
            deltaDist = 0;
        }
        else {
            deltaDist = getRelativeDistance(wsmLatitude[wsmRadioType][wsmRadio], tpsUserData.latitude,
                                            wsmLongitude[wsmRadioType][wsmRadio], tpsUserData.longitude);
        }

        sprintf(&gBuf[strlen(gBuf) - 1], " Remote lat=%4.2f, Remote long=%4.2f, Remote err=%d, ",
                wsmLatitude[wsmRadioType][wsmRadio],  wsmLongitude[wsmRadioType][wsmRadio], wsmTpsError[wsmRadioType][wsmRadio]);
        sprintf(&gBuf[strlen(gBuf) - 1], " Local lat=%4.2f, Local long=%4.2f, Local err=%d, Distance=%6.2f, ",
                tpsUserData.latitude, tpsUserData.longitude, localTpsError, deltaDist);

        /* Display channel busy info */
        if (uService.extendedAccess == 65535) {
            sprintf(&gBuf[strlen(gBuf) -1], " RawChanBusy=%d%% ", rawChanBusyRatio);
        }
        else {
            sprintf(&gBuf[strlen(gBuf) -1], " CCHBusy=%d%%, SCHBusy=%d%% ", waveCCHBusyRatio, waveSCHBusyRatio);
        }
    }

    sprintf(&gBuf[strlen(gBuf)], " \n");
    printf("%s", gBuf);

    /* Clear counters if accumulate flag is not set */
    if (!wsmAccum) {
        for (i=0; i < MAX_RADIO_TYPES; i++) {
            for (j=0; j < MAX_RADIOS; j++) {
                wsmMaxLatency[i][j][svcIdx] = 0;
                wsmMinLatency[i][j][svcIdx] = 9999999;
                totalElapsedTime[i][j][svcIdx] = 0;
                wsmReceivedPkt[i][j][svcIdx] = 0;
                wsmMissedPkt[i][j][svcIdx] = 0;
                wsmOutofOrderPkt[i][j][svcIdx] = 0;
            }
        }
    }
}

/* Converts milliseconds since epoch to TPS data TIME string format
 * which is HHMMSS.SS
 */
char *epo_ms_to_tps_time_str(char *timestr, int32_t strlen, long long epo_ms)
{
    time_t    epo_sec = 0;
    struct tm epo_tm;
    uint32_t  ms;
    char      msStr[8];

    ms = (int)rint((epo_ms % 1000) / 10.0);
    sprintf(msStr, ".%02d", ms);
    epo_sec = (time_t)(epo_ms / 1000);

    gmtime_r(&epo_sec, &epo_tm);
    strftime(timestr, strlen, "%H%M%S", &epo_tm);

    strcat(timestr, msStr);
    return timestr;
}

/* Converts milliseconds since epoch to TPS data DATE string format
 * which is DDMMYY
 */
char *epo_ms_to_tps_date_str(char *datestr, int32_t strlen, long long epo_ms)
{
    time_t    epo_sec = 0;
    struct tm epo_tm;

    epo_sec = (time_t)(epo_ms / 1000);

    gmtime_r(&epo_sec, &epo_tm);
    strftime(datestr, strlen, "%Y%m%d", &epo_tm);
    return datestr;
}

void pollRadioHandler()
{
    rsResultCodeType risRet;
    DeviceTallyType  tallies;
    char             buf[300];
    struct timeval   currTime;
    unsigned long    svcIdx=displayPSID-wsmPSID;
    double           wsmRxPktMbps;
    uint32_t         wsmRxPktGood, wsmRxPktBad;
    double           wsmRxPktsPerSec = 0, wsmRxPktMbpsCum, wsmPktsPerSecCum;
    uint32_t         wsmPktGoodCum, wsmPktBadCum;
    int32_t          wsmRssIAvg, wsmPktPERAvg;
    int32_t          wsmRssIAvgAntA, wsmRssIAvgAntB;
    double           wsmRxPktPER;
    uint32_t         totalTestElapsedTime;

    memset(buf,0x0,sizeof(buf));
    if ((risRet = wsuRisGetRadioTallies(wsmRadioType, wsmRadio, &tallies)) != RS_SUCCESS) {
        if (risRet != RS_ENOSUPPORT) {
            printf("wsuRisGetRadioTallies failed (%s)\n", rsResultCodeType2Str(risRet));
        }

        return;
    }

    /* Get channel busy values.  If in raw mode, set the WAVE CCH value to unknown.
       If in WAVE mode, set the raw value to unknown. */
    rawChanBusyRatio = tallies.rawchan_busyratio;
    waveCCHBusyRatio = tallies.wavecch_busyratio;
    waveSCHBusyRatio = tallies.wavesch_busyratio;

    if (uService.extendedAccess == 65535) {
        waveCCHBusyRatio = 999;
        waveSCHBusyRatio = 999;
    }
    else if (uService.extendedAccess == 0) {
        rawChanBusyRatio = 999;
    }

    gettimeofday(&currTime, NULL);
    totalTestElapsedTime  = currTime.tv_sec * 1000000 + currTime.tv_usec - testStartTime;
    wsmRxPktMbps          = ((double)(wsmReceivedBytes[wsmRadioType][wsmRadio][svcIdx] - wsmReceivedPreBytes) * 8 * (1000 / wsmStatsLog) / 1000000);
    wsmReceivedPreBytes   = wsmReceivedBytes[wsmRadioType][wsmRadio][svcIdx];
    wsmRxPktGood          = wsmReceivedPkt[wsmRadioType][wsmRadio][svcIdx] - wsmRxPktGoodPrePeriod;
    wsmRxPktGoodPrePeriod = wsmReceivedPkt[wsmRadioType][wsmRadio][svcIdx];
    wsmRxPktBad           = wsmMissedPkt[wsmRadioType][wsmRadio][svcIdx] - wsmRxPktBadPrePeriod;
    wsmRxPktBadPrePeriod  = wsmMissedPkt[wsmRadioType][wsmRadio][svcIdx];

    if (wsmStatsLog) {
        wsmRxPktsPerSec = ((double)wsmRxPktGood) * (1000 / wsmStatsLog);
    }

    if ((wsmRxPktGood + wsmRxPktBad) != 0) {
        wsmRxPktPER = ((double) wsmRxPktBad / (double)(wsmRxPktGood + wsmRxPktBad)) * 100.0;
    }
    else {
        wsmRxPktPER = 100.0;
    }

    wsmRxPktMbpsCum  = (((double)(wsmReceivedBytes[wsmRadioType][wsmRadio][svcIdx])) * 8 / (((double)totalTestElapsedTime) / 1000000)) /1000000;
    wsmPktGoodCum    = wsmReceivedPkt[wsmRadioType][wsmRadio][svcIdx];
    wsmPktBadCum     = wsmMissedPkt[wsmRadioType][wsmRadio][svcIdx];
    wsmPktsPerSecCum = (((double)(wsmReceivedPkt[wsmRadioType][wsmRadio][svcIdx])) / (double)totalTestElapsedTime) * 1000000;
    wsmPktPERAvg     = (uint32_t) (wsmRxPktBad / totalTestElapsedTime * 100);
    wsmRssIAvg       = (int32_t) (10 * (log10((double)(wsmTotalRssi) / wsmTotalRssiCount)));
    wsmRssIAvgAntA   = (int32_t) (10 * (log10((double)(wsmTotalRssiAntA) / wsmTotalRssiCountAntA)));
    wsmRssIAvgAntB   = (int32_t) (10 * (log10((double)(wsmTotalRssiAntB) / wsmTotalRssiCountAntB)));

    if (wsmLog) {
        if (wsmStatsLog) {
            char curDateStr[16];
            char curTimeStr[16];
            char recvDateStr[16];
            char recvTimeStr[16];
            char deltaDistStr[16];      /*Need blank (non-numeric) value if TPS data not available), not zero*/
            long long milliseconds = (long long)currTime.tv_sec * 1000 + (long long)currTime.tv_usec / 1000;
            long long recvMilliseconds = (long long)pktSendSec * 1000 + (long long)pktSendMicrosec / 1000;

            epo_ms_to_tps_date_str(curDateStr, 20, milliseconds);
            epo_ms_to_tps_time_str(curTimeStr, 20, milliseconds);

            epo_ms_to_tps_date_str(recvDateStr, 20, recvMilliseconds);
            epo_ms_to_tps_time_str(recvTimeStr, 20, recvMilliseconds);

            /* Add Distance to STATSlog*/
            if (wsmTpsError[wsmRadioType][wsmRadio] || localTpsError) {
                deltaDist = 0;
                sprintf(deltaDistStr, " ");
            }
            else {
                deltaDist = getRelativeDistance(wsmLatitude[wsmRadioType][wsmRadio], tpsUserData.latitude,
                                                wsmLongitude[wsmRadioType][wsmRadio], tpsUserData.longitude);
                sprintf(deltaDistStr,"%6.2f",deltaDist);
            }

            sprintf(buf,"STATS, %4.3f, %d, %d, %4.3f, %.2f, %4.3f, %d, %d, %4.3f, %d, %d, %d, %s, %s, %4.6f, %4.6f, %s, %s, %4.6f, %4.6f, %s, %s",
                    wsmRxPktMbps, wsmRxPktGood, wsmRxPktBad, wsmRxPktsPerSec, wsmRxPktPER,
                    wsmRxPktMbpsCum, wsmPktGoodCum, wsmPktBadCum, wsmPktsPerSecCum,
                    wsmPktPERAvg, wsmRss[wsmRadioType][wsmRadio][svcIdx], wsmRssIAvg,
                    curDateStr, curTimeStr, tpsUserData.latitude, tpsUserData.longitude,
                    recvDateStr, recvTimeStr, wsmLatitude[wsmRadioType][wsmRadio], wsmLongitude[wsmRadioType][wsmRadio],
                    IPCharAddr, deltaDistStr);

            if (showAntRss) {
                sprintf(&buf[strlen(buf)-1], ", %d, %d ",
                        wsmRssIAvgAntA, wsmRssIAvgAntB);
            }

            sprintf(&buf[strlen(buf)], " \n");
            fwrite(buf, 1, strlen(buf), logpLogFileP);
        }
    }
}

int setIntSysctlVal(char *procFileName, int value)
{
    FILE *fd;
    char str[32];
    memset(str,0x0,sizeof(str));
    fd = fopen(procFileName , "w");

    if (fd == NULL) {
        perror (procFileName);
        return EXIT_FAILURE;
    }

    sprintf(str, "%d", value);
    fwrite(str, 1, strlen(str), fd);
    fclose(fd);
    return EXIT_SUCCESS;
}

int getIntSysctlVal(char *procFileName, int *pValue)
{
    FILE *fd;
    char  str[32];

    memset(str,0x0,sizeof(str));

    fd = fopen(procFileName , "r");

    if (fd == NULL) {
        perror (procFileName);
        return EXIT_FAILURE;
    }

    fread(str, 1, 32, fd);
    *pValue = atoi(str);
    fclose(fd);
    return EXIT_SUCCESS;
}

void WsmExitHandler(int signal)
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");

        /* If using VOD, shut down the VOD thread */
        if (useVOD) {
            if (VODRequestHandle >= 0) {
                cancel_vod_request(VODRequestHandle);
            }

            sem_post(&VOD_sem);
            pthread_join(VODThreadId, NULL);
        }

        mainLoop = FALSE;
    }
}

void WsmCleanHandler()
{
    static int       term_times = 0;
    cfgType          cfg;
    rsResultCodeType risRet;

    printf("CleanHandler is called.\n");

    if (term_times > 0) {
        exit(-1);
    }

    term_times++;

    if (wsmLog) {
        free (filePath);
        fclose(logpLogFileP);
    }

    service.action    = DELETE;
    service.radioType = wsmRadioType;
    service.psid      = wsmPSID;

    if ((risRet = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
        printf("wsuRisWsmServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
        return;
    }
    else {
        printf("wsuRisWsmServiceRequest Service (PSID=0x%8x)\n", service.psid);
    }

    if (wsmProvider) {
        printf("Delete Role as Provider\n");

        /* Register Services */
        pService.action = DELETE;

        if ((risRet = wsuRisProviderServiceRequest(&pService)) != RS_SUCCESS) {
            printf("wsuRisProviderServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return;
        }
        else {
            printf("wsuRisProviderServiceRequest (PSID=0x%8x)\n", pService.psid);
        }
    }
    else { // user mode
        /* Quit Listening to WSAs */
        uService.action = DELETE;

        if ((risRet = wsuRisUserServiceRequest(&uService)) != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return;
        }

        printf("All User Services Unregistered\n");
    }

    /* Re-enable 2nd radio receiver, if necessary */
    if (reduceCPULoading) {
        cfg.u.RecvMode = RECEIVER_ENABLE;
        cfg.radioType  = wsmRadioType;

        if ((risRet = wsuRisSetCfgReq(CFG_ID_RECEIVER_MODE_RADIO_1, &cfg)) != RS_SUCCESS) {
            printf("wsuRisSetCfgReq CFG_ID_RECEIVER_MODE_RADIO_1, error setting failed (%s)\n"
                   "[Radio 1 NOT Re-Enabled]", rsResultCodeType2Str(risRet));
        }
    }

    /* Disconnect from Security Manager. */
    if (connectedToSM == TRUE) {
        smiTerm();
        printf("Disconnected from SM.\n");
    }

    /* Done - Terminate Radio Communication */
    if ((risRet = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
        printf("wsuRisTerminateRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return;
    }

    printf("RadioComm Terminated\n");

    /* If using VOD, destroy the VOD semaphore */
    if (useVOD) {
        denit_with_vod();
        sem_destroy(&VOD_sem);
    }
}


static void printServiceInfo(serviceInfoType *serviceInfo)
{
    char str[40];

    memset(str,0x0,sizeof(str));
    printf("rss=%d sourceMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
           serviceInfo->rss,
           serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
           serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
           serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);
    printf("changeCount=%d psid=0x%08x channelIndex=%d\n",
           serviceInfo->changeCount, serviceInfo->psid,
           serviceInfo->channelIndex);

    if (serviceInfo->WSABitmask & (WSA_EXT_REPEAT_RATE_BITMASK |
                                   WSA_EXT_ADVERTISER_ID_BITMASK)) {
        if (serviceInfo->WSABitmask & WSA_EXT_REPEAT_RATE_BITMASK) {
            printf("repeatRate=%d ", serviceInfo->repeatRate);
        }

        if (serviceInfo->WSABitmask & WSA_EXT_ADVERTISER_ID_BITMASK) {
            printf("advertiseId=\"%.*s\" ", serviceInfo->advertiseIdLength, serviceInfo->advertiseId);
        }

        printf("\n");
    }

    if (serviceInfo->WSABitmask & WSA_EXT_2DLOCATION_BITMASK) {
        printf("latitude2D=%d.%d longitude2D=%d.%d\n",
               serviceInfo->latitude2D / 10000000,
               ABS(serviceInfo->latitude2D % 10000000),
               serviceInfo->longitude2D / 10000000,
               ABS(serviceInfo->longitude2D % 10000000));
    }

    if (serviceInfo->WSABitmask & WSA_EXT_3DLOCATION_BITMASK) {
        printf("latitude3D=%d.%d longitude3D=%d.%d elevation3D=%d.%d "
               "confidence3D=0x%02x\n",
               serviceInfo->latitude3D / 10000000,
               ABS(serviceInfo->latitude3D % 10000000),
               serviceInfo->longitude3D / 10000000,
               ABS(serviceInfo->longitude3D % 10000000),
               serviceInfo->elevation3D / 10,
               ABS(serviceInfo->elevation3D % 10),
               serviceInfo->confidence3D);
    }

    printf("semiMajorAccuracy3D=%d semiMinorAccuracy3D=%d "
           "semiMajorOrientation3D=%d\n",
           serviceInfo->semiMajorAccuracy3D, serviceInfo->semiMinorAccuracy3D,
           serviceInfo->semiMajorOrientation3D);

    if (serviceInfo->serviceBitmask & (WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |
                                       WSA_EXT_IPV6ADDRESS_BITMASK |
                                       WSA_EXT_SERVICE_PORT_BITMASK)) {
        if (serviceInfo->serviceBitmask & WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK) {
            printf("psc=\"%.*s\" ", serviceInfo->servicePscLength,
                   serviceInfo->psc);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_IPV6ADDRESS_BITMASK) {
            inet_ntop(AF_INET6, serviceInfo->ipv6Address, str, 40);
            printf("ipv6Address=%s", str);
        }

        printf("\n");
    }

    if (serviceInfo->serviceBitmask & (WSA_EXT_SERVICE_PORT_BITMASK |
                                       WSA_EXT_RCPI_THRESHOLD_BITMASK |
                                       WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK |
                                       WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK)) {
        if (serviceInfo->serviceBitmask & WSA_EXT_SERVICE_PORT_BITMASK) {
            printf("port=%d ", serviceInfo->port);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_RCPI_THRESHOLD_BITMASK) {
            printf("rcpiThreshold=%d ", serviceInfo->rcpiThreshold);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK) {
            printf("wsaCountThreshold=%d ", serviceInfo->wsaCountThreshold);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK) {
            printf("wsaCountThresholdInterval=%d",
                   serviceInfo->wsaCountThresholdInterval);
        }

        printf("\n");
    }

    if (serviceInfo->serviceBitmask & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
        printf("providerMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
               serviceInfo->providerMacAddress[0],
               serviceInfo->providerMacAddress[1],
               serviceInfo->providerMacAddress[2],
               serviceInfo->providerMacAddress[3],
               serviceInfo->providerMacAddress[4],
               serviceInfo->providerMacAddress[5]);
    }

    printf("operatingClass=%d channelNumber=%d adaptable=%d "
           "dataRate=%d txPwrLevel=%d\n",
           serviceInfo->operatingClass, serviceInfo->channelNumber,
           serviceInfo->adaptable, serviceInfo->dataRate,
           serviceInfo->txPwrLevel);

    if (serviceInfo->channelBitmask & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
        printf("channelAccess=%d (%s)\n", serviceInfo->channelAccess,
               (serviceInfo->channelAccess == 2) ? "alternating slot 1" :
               (serviceInfo->channelAccess == 1) ? "alternating slot 0" :
               (serviceInfo->channelAccess == 0) ? "continuous" : "?");
    }

    if (serviceInfo->channelBitmask & WSA_EXT_EDCA_PARAM_SET_BITMASK) {
        int i;

        printf("qosInfo=%d\n", serviceInfo->qosInfo);

        for (i = 0; i < 4; i++) {
            printf("aifs[%d]=0x%02x ecw[%d]=0x%02x txop[%d]=%d\n",
                   i, serviceInfo->aifs[i], i, serviceInfo->ecw[i],
                   i, serviceInfo->txop[i]);
        }
    }

    if (serviceInfo->routingBitmask & WSA_EXT_WRA_BITMASK) {
        printf("routerLifeTime=%d prefixLen=%d\n",
            serviceInfo->routerLifeTime, serviceInfo->prefixLen);
        inet_ntop(AF_INET6, serviceInfo->ipPrefix, str, 40);
        printf("ipPrefix=%s\n", str);
        inet_ntop(AF_INET6, serviceInfo->defaultGateway, str, 40);
        printf("defaultGateway=%s\n", str);
        inet_ntop(AF_INET6, serviceInfo->primaryDns, str, 40);
        printf("primaryDns=%s\n", str);

        if (serviceInfo->routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK) {
            inet_ntop(AF_INET6, serviceInfo->secondaryDns, str, 40);
            printf("secondaryDns=%s\n", str);
        }

        if (serviceInfo->routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK) {
            printf("gatewayMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   serviceInfo->gatewayMacAddress[0],
                   serviceInfo->gatewayMacAddress[1],
                   serviceInfo->gatewayMacAddress[2],
                   serviceInfo->gatewayMacAddress[3],
                   serviceInfo->gatewayMacAddress[4],
                   serviceInfo->gatewayMacAddress[5]);
        }
    }
}

static void service_available(serviceInfoType *serviceInfo)
{
    rsResultCodeType        risRet;
    UserServiceChangeType cfg;

    if ((serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) &&
        (wBSSJoined == TRUE) &&
        (memcmp(wBSSMacAddress, serviceInfo->sourceMac, 6) == 0)) {
        printf("\nsvcAvailable=SERVICE_NOT_AVAILABLE "
               "mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
               serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
               serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
               serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);


        cfg.psid                       = uService.psid;
        cfg.command                    = CHANGE_ACCESS;
#ifdef CHINA_FREQS_ONLY
        cfg.user.access.channelNumber  = 153;
#else
        cfg.user.access.channelNumber  = 178;
#endif
        cfg.user.access.adaptable      = serviceInfo->adaptable;
        cfg.user.access.dataRate       = serviceInfo->dataRate;
        cfg.user.access.txPwrLevel     = serviceInfo->txPwrLevel;
        cfg.user.access.userAccess     = NO_SCH_ACCESS;
        cfg.user.access.extendedAccess = 65535;

        if ((risRet = wsuRisChangeUserServiceRequest(&cfg)) != RS_SUCCESS) {
            printf("wsuRisChangeUserServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
        }

        wBSSJoined = FALSE;
    }
    else if ((serviceInfo->serviceAvail == RS_SERVICE_AVAILABLE) &&
             (wBSSJoined == FALSE)) {
        if (((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) == WSA_SIGNED)
                && (uService.wsaSecurity == SECURED_WSA)) {
            /* WSA signed but not verified; verify the WSA. */
            printf("service_available:  Security Not Implemented\n");

            if (localTpsError == 0) {
                wsuRisVerifyWsaRequest(serviceInfo->signedWsaHandle);
            }
        }
        else {
            /* Ignore if invalid channel number in WSA. */
#ifdef CHINA_FREQS_ONLY
            if ((serviceInfo->channelNumber < 147) ||
                (serviceInfo->channelNumber > 169) ||
                (serviceInfo->channelNumber % 2 == 0)) {
                return;
            }
#else
            if ((serviceInfo->channelNumber != 84) &&
                (serviceInfo->channelNumber != 88) &&
                (serviceInfo->channelNumber != 164) &&
                (serviceInfo->channelNumber != 168)) {
                if ((serviceInfo->channelNumber < 172)  ||
                    (serviceInfo->channelNumber > 184)) {
                    return;
                }
            }
#endif

            printf("\nsvcAvailable=SERVICE_AVAILABLE "
                   "mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
                   serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
                   serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);

            /* Accept Service */
            cfg.psid                       = uService.psid;
            cfg.command                    = CHANGE_ACCESS;
            cfg.user.access.channelNumber  = serviceInfo->channelNumber;
            cfg.user.access.adaptable      = serviceInfo->adaptable;
            cfg.user.access.dataRate       = serviceInfo->dataRate;
            cfg.user.access.txPwrLevel     = serviceInfo->txPwrLevel;
            cfg.user.access.userAccess     = AUTO_ACCESS_UNCONDITIONAL;
            cfg.user.access.extendedAccess = (serviceInfo->channelAccess == 0) ?
                                             65535 : 0;

            if ((risRet = wsuRisChangeUserServiceRequest(&cfg)) != RS_SUCCESS) {
                printf("wsuRisChangeUserServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
                return;
            }
            else {
                printf("WSA Detected (PSID = 0x%x)\n", uService.psid);
            }

            if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
                (WSA_SIGNED | WSA_VERIFIED)) {
                printf("Verify WSA succeeded.\n");
            }

            printServiceInfo(serviceInfo);

            wBSSJoined = TRUE;
            memcpy(wBSSMacAddress, serviceInfo->sourceMac, 6);
        }
    }
}

/* Count up the verify statistics and periodically print out */
static void printVernStatus(uint32_t securityFlags)
{
    struct timeval currTime;
    static uint32_t last_tv_sec = 0;

    switch (securityFlags & 0xff) {
    case unsecured:
        securityStatusCounter.unsecured++;
        break;

    case securedButNotVerified:
        securityStatusCounter.securedButNotVerified++;
        break;

    case securedVerifiedSuccess:
        securityStatusCounter.securedVerifiedSuccess++;
        break;

    case securedVerifiedFail:
        securityStatusCounter.securedVerifiedFail++;
        break;

    case securedVernNotNeeded:
        securityStatusCounter.securedVernNotNeeded++;
        break;

    default:
        securityStatusCounter.other++;
        break;
    }

    gettimeofday(&currTime, NULL);

    if (currTime.tv_sec != last_tv_sec) {
        printf("unsecured = %u; not verified = %u; success = %u; fail = %u; not needed = %u; other = %u\n",
               securityStatusCounter.unsecured,
               securityStatusCounter.securedButNotVerified,
               securityStatusCounter.securedVerifiedSuccess,
               securityStatusCounter.securedVerifiedFail,
               securityStatusCounter.securedVernNotNeeded,
               securityStatusCounter.other);
        last_tv_sec = currTime.tv_sec;
    }
}

static void receive_wsmDebug (inWSMType *Wsm)
{
    int i;
    static int counter = 0;

    if ((Wsm->channelNumber < 172) || ((Wsm->channelNumber & 0x01) == 0)) { // 10MHz
        switch (Wsm->dataRate) {
        case DR_3_MBPS:
            wsmDataRate = 3;
            break;
        case DR_4_5_MBPS:
            wsmDataRate = 4;
            break;
        case DR_6_MBPS:
            wsmDataRate = 6;
            break;
        case DR_9_MBPS:
            wsmDataRate = 9;
            break;
        case DR_12_MBPS:
            wsmDataRate = 12;
            break;
        case DR_18_MBPS:
            wsmDataRate = 18;
            break;
        case DR_24_MBPS:
            wsmDataRate = 24;
            break;
        case DR_27_MBPS:
            wsmDataRate = 27;
            break;
        default:
            wsmDataRate = 0;
            break;
        }
    }
    else { // 20 MHz
        switch (Wsm->dataRate) {
        case DR_6_MBPS_20MHz:
            wsmDataRate = 6;
            break;
        case DR_9_MBPS_20MHz:
            wsmDataRate = 9;
            break;
        case DR_12_MBPS_20MHz:
            wsmDataRate = 12;
            break;
        case DR_18_MBPS_20MHz:
            wsmDataRate = 18;
            break;
        case DR_24_MBPS_20MHz:
            wsmDataRate = 24;
            break;
        case DR_36_MBPS_20MHz:
            wsmDataRate = 36;
            break;
        case DR_48_MBPS_20MHz:
            wsmDataRate = 48;
            break;
        case DR_54_MBPS_20MHz:
            wsmDataRate = 54;
            break;
        default:
            wsmDataRate = 0;
            break;
        }
    }

    counter++;
    /* Obtain configuration and mode information */
    printf("Cnt=%d,Radio=%s %d,Ver=%d,Chn=%d,Rte=%d,Pwr=%d,PSID=0x%x,\nPPP=%u,Pri=%d,WSMLen=%d,PLCPLen=%d,OTALength=%d,RSS=%d\n",
            counter, (Wsm->radioType == RT_CV2X) ? "C-V2X" : "DSRC", Wsm->radioNum, Wsm->wsmVersion, Wsm->channelNumber,
            wsmDataRate,  Wsm->txPwrLevel, Wsm->psid,Wsm->ppp, Wsm->userPriority, Wsm->dataLength, Wsm->recvPlcpLength,
            Wsm->recvWsmOTALength, (signed char)(Wsm->rss));
    printf("SrcMac=%02x:%02x:%02x:%02x:%02x:%02x,SecFl=%d\n",
            Wsm->srcMacAddress[0], Wsm->srcMacAddress[1], Wsm->srcMacAddress[2],
            Wsm->srcMacAddress[3], Wsm->srcMacAddress[4], Wsm->srcMacAddress[5],
            (Wsm->securityFlags & 0xff));

    for (i = 0; i < Wsm->dataLength; i++) {
        if ((i % 16 == 0) && (i != 0)) {
            printf("\n");
        }

        printf("%02x ", Wsm->data[i]);
    }

    printf("\n\n");

    if (vernRes) {
        if (useVOD) {
            /* If using VOD, activate the VOD thread to do the SMI verify
             * call */
            VODMsgSeqNum = Wsm->vodMsgSeqNum;
            sem_post(&VOD_sem);
        }
        else {
            /* If want verification results, and doing verify-then-process,
             * print the security flags */
            printVernStatus(Wsm->securityFlags);
        }
    }
}

static void receive_wsm(inWSMType *Wsm)
{
    InWsmMsgData   *InWsmVer;
#if !defined(NO_SECURITY)
    char           *charPtr = (char *) &Wsm->data[0];
    char           *charEndPtr = charPtr + Wsm->dataLength - 3;
#endif
    char            buf[225];
    rsRadioType     rxRadioType;
    uint8_t         rxRadio;
    unsigned int    svcIdx;
    uint32_t        currentPSID;
    uint32_t        temp = 0;
    struct timeval  currTime;


    memset(buf,0x0,sizeof(buf));
#if !defined(NO_SECURITY)
    if (!useVOD && vernRes) {
        /* If want verification results, and doing verify-then-process, print
         * the security flags */
        printVernStatus(Wsm->securityFlags);
    }

    if (Wsm->security &&
        ((Wsm->securityFlags & 0xff) == securedVerifiedFail)) {
        vern_fail_cnt++;
        return;
    }
#endif
    /* If desired, only process WSMs received on configured radio.
     * Handling both radios causes too much CPU loading */
    if (reduceCPULoading &&
        ((Wsm->radioType != wsmRadioType) || (Wsm->radioNum != wsmRadio))) {
        return;
    }

    gettimeofday(&currTime, NULL);
    currTime.tv_usec += 500;  // Round up 500us
    currTime.tv_usec /= 1000; // Convert to ms
    currTime.tv_usec *= 1000; // Convert back to us

#if 0
    /* Used for debugging */
    {
        int i;

        printf("receive_wsm (len=%d)\n", Wsm->DataLength);

        for (i = 0; i < Wsm->DataLength; i += 8) {
            printf("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
                   Wsm->Data[i + 0], Wsm->Data[i + 1], Wsm->Data[i + 2],
                   Wsm->Data[i + 3], Wsm->Data[i + 4], Wsm->Data[i + 5],
                   Wsm->Data[i + 6], Wsm->Data[i + 7]);
        }
    }
#endif

    if (Wsm->dataLength < sizeof(InWsmMsgData)) { // Data 10 bytes + 4 bytes sequence number + ....
        printf("Error: wsm_length = %d\n", Wsm->dataLength);
        return;
    }

    rxRadioType = Wsm->radioType;
    rxRadio     = Wsm->radioNum;

    if (((rxRadioType != RT_CV2X) && (rxRadioType != RT_DSRC)) ||
        (rxRadio > 1)) {
        printf("WSM received on invalid radio interface (%s %d)\n",
               (rxRadioType == RT_CV2X) ? "C-V2X" :
               (rxRadioType == RT_DSRC) ? "DSRC" : "????", rxRadio);
        return;
    }

    wsmVersion = Wsm->wsmVersion;
    wsmChannelNumber = Wsm->channelNumber;

    if ((wsmChannelNumber < 172) ||
        ((wsmChannelNumber & 0x01) == 0)) { // 10MHz
        switch (Wsm->dataRate) {
        case DR_3_MBPS:
            wsmDataRate = 3;
            break;
        case DR_4_5_MBPS:
            wsmDataRate = 4;
            break;
        case DR_6_MBPS:
            wsmDataRate = 6;
            break;
        case DR_9_MBPS:
            wsmDataRate = 9;
            break;
        case DR_12_MBPS:
            wsmDataRate = 12;
            break;
        case DR_18_MBPS:
            wsmDataRate = 18;
            break;
        case DR_24_MBPS:
            wsmDataRate = 24;
            break;
        case DR_27_MBPS:
            wsmDataRate = 27;
            break;
        default:
            wsmDataRate = 0;
            break;
        }
    }
    else { // 20 MHz
        switch (Wsm->dataRate) {
        case DR_6_MBPS_20MHz:
            wsmDataRate = 6;
            break;
        case DR_9_MBPS_20MHz:
            wsmDataRate = 9;
            break;
        case DR_12_MBPS_20MHz:
            wsmDataRate = 12;
            break;
        case DR_18_MBPS_20MHz:
            wsmDataRate = 18;
            break;
        case DR_24_MBPS_20MHz:
            wsmDataRate = 24;
            break;
        case DR_36_MBPS_20MHz:
            wsmDataRate = 36;
            break;
        case DR_48_MBPS_20MHz:
            wsmDataRate = 48;
            break;
        case DR_54_MBPS_20MHz:
            wsmDataRate = 54;
            break;
        default:
            wsmDataRate = 0;
            break;
        }
    } // 10 MHz / 20 MHz

    wsmUserPriority = Wsm->userPriority;
    wsmRecvPlcpLength = Wsm->recvPlcpLength;
    currentPSID = Wsm->psid;
    svcIdx = currentPSID - wsmPSID;

    if (svcIdx >= MAX_SERVICES) {
        printf("WSM received with PSID out of range (0x%8x, 0x%8x)\n",
               currentPSID, wsmPSID);
        return;
    }

    wsmRss[rxRadioType][rxRadio][svcIdx] = Wsm->rss;
    curRssi = pow(10, ((double) (signed char)(Wsm->rss)/10.0));
    wsmTotalRssiCount ++;
    wsmTotalRssi += curRssi;

    curRssiAntA = pow(10, ((double) (signed char)(Wsm->rssAntA)/10.0));
    wsmTotalRssiCountAntA++;
    wsmTotalRssiAntA += curRssiAntA;
    curRssiAntB = pow(10, ((double) (signed char)(Wsm->rssAntB)/10.0));
    wsmTotalRssiCountAntB++;
    wsmTotalRssiAntB += curRssiAntB;

    wsmRssAntA[rxRadioType][rxRadio][svcIdx]   = Wsm->rssAntA;   /*Raw RSS for antenna A (not part of statistics)*/
    wsmRssAntB[rxRadioType][rxRadio][svcIdx]   = Wsm->rssAntB;   /*Raw RSS for antenna B (not part of statistics)*/
    wsmNoiseAntA[rxRadioType][rxRadio][svcIdx] = Wsm->noiseAntA; /*Raw Noise for antenna A*/
    wsmNoiseAntB[rxRadioType][rxRadio][svcIdx] = Wsm->noiseAntB; /*Raw Noise for antenna B*/

    InWsmVer = (InWsmMsgData *) &Wsm->data[0];
#if !defined(NO_SECURITY)
    if (strncmp((const char *)charPtr, (const char *)"wsmtest.", 8) == 0) {
#else
    if(1) {
#endif
        if (Wsm->txPwrLevel != 255) {
            temp = 3;
        }

        if (Wsm->dataRate != 255) {
            temp += 3;
        }

        if (Wsm->channelNumber != 255) {
            temp += 3;
        }

        wsmReceivedBytes[rxRadioType][rxRadio][svcIdx] = wsmReceivedBytes[rxRadioType][rxRadio][svcIdx] +
                                                         (7 + temp + Wsm->dataLength);
#if !defined(NO_SECURITY)
        if (Wsm->dataLength >= sizeof(InWsmMsgData) + 3) {
            if ((strncmp((const char *)charEndPtr, (const char *)"WSM", 3) != 0) &&
                (wsmPerformance != 3)) {
                printf("Error on payload \n");
            }
        }
#endif
        uint32_t seqNum = ntohl(InWsmVer->wsmSequenceNumber);

        if (seqNum > 10) {
            if (seqNum > wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx]) {
                if ((wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx] != (seqNum - 1)) &&
                    wsmFirstPacketReceived[rxRadioType][rxRadio][svcIdx]) {
                    wsmMissedPkt[rxRadioType][rxRadio][svcIdx] +=
                        (seqNum - 1 - wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx]);
                }
            }
            else {
                wsmOutofOrderPkt[rxRadioType][rxRadio][svcIdx] ++;
                wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx] = seqNum;
            }

            wsmReceivedPkt[rxRadioType][rxRadio][svcIdx]++;

            pktSendSec = ntohl(InWsmVer->WsmSendSec);
            pktSendMicrosec = ntohl(InWsmVer->WsmSendMicrosec);
            /* In case the sender did not round the number of microseconds up
             * to the nearest millisecond, do so now. This will not affect any
             * number of microseconds that have not already been rounded up. */
            pktSendMicrosec += 500;
            pktSendMicrosec /= 1000;
            pktSendMicrosec *= 1000;

            if ((wsmPerformance != 0) && (wsmPerformance != 3)) {
                int32_t elapsedTime = 0;
                uint32_t receivedPktCnt = wsmReceivedPkt[rxRadioType][rxRadio][svcIdx];

                //printf("pktRecvSec=%lu pktSendSec=%u pktRecvMic=%lu pktSendMicro=%u \n",
                //         currTime.tv_sec, pktSendSec, currTime.tv_usec, pktSendMicrosec);
                if (currTime.tv_sec == pktSendSec) {
                    elapsedTime = currTime.tv_usec - pktSendMicrosec;

                    if (wsmPerformance == 2) {
                        printf("%d\n", elapsedTime);
                    }
                    else if ((currTime.tv_usec < pktSendMicrosec) &&
                             (!reduceCPULoading)) {
                        printf("Receive time is smaller than transmit time.   delta = %lu\n",
                               ((long unsigned)pktSendMicrosec)-currTime.tv_usec);
                    }

                    totalElapsedTime[rxRadioType][rxRadio][svcIdx] += elapsedTime;

                    if (elapsedTime > wsmMaxLatency[rxRadioType][rxRadio][svcIdx]) {
                        wsmMaxLatency[rxRadioType][rxRadio][svcIdx] = elapsedTime;
                    }
                    else if (elapsedTime < wsmMinLatency[rxRadioType][rxRadio][svcIdx]) {
                        wsmMinLatency[rxRadioType][rxRadio][svcIdx] = elapsedTime;
                    }

                    if (receivedPktCnt != 0) {
                        wsmLatency[rxRadioType][rxRadio][svcIdx] = totalElapsedTime[rxRadioType][rxRadio][svcIdx] /
                                                                   receivedPktCnt;
                    }

                    //printf("wsmLatency=%4.2f elapsedTime=%u\n", wsmLatency, elapsedTime);
                }
                else if (currTime.tv_sec > pktSendSec) {
                    elapsedTime = 1000000 * (currTime.tv_sec - pktSendSec) +
                                  currTime.tv_usec - pktSendMicrosec;

                    if (wsmPerformance == 2) {
                        printf("%d\n", elapsedTime);
                    }
                    else if (elapsedTime < 0) {
                        printf("should not happen..\n");
                    }

                    //if (elapsedTime > 1000000) {
                    //    printf("pktRecvSec=%lu pktSendSec=%u pktRecvMic=%lu pktSendMicro=%u\n",
                    //           currTime.tv_sec, pktSendSec, currTime.tv_usec, pktSendMicrosec);
                    //}
                    totalElapsedTime[rxRadioType][rxRadio][svcIdx] += elapsedTime;

                    if (elapsedTime > wsmMaxLatency[rxRadioType][rxRadio][svcIdx]) {
                        wsmMaxLatency[rxRadioType][rxRadio][svcIdx] = elapsedTime;
                    }
                    else if (elapsedTime < wsmMinLatency[rxRadioType][rxRadio][svcIdx]) {
                        wsmMinLatency[rxRadioType][rxRadio][svcIdx] = elapsedTime;
                    }

                    if (receivedPktCnt != 0) {
                        wsmLatency[rxRadioType][rxRadio][svcIdx] = totalElapsedTime[rxRadioType][rxRadio][svcIdx] /
                                                      receivedPktCnt;
                        //printf("wsmLatency=%4.2f elapsedTime=%u\n", wsmLatency, elapsedTime);
                    }
                }
                else {
                    if (!reduceCPULoading) {
                        printf("Time not synchronized. micro delta = %lu\n",
                               (pktSendSec - currTime.tv_sec) * 1000000 +
                               ((long unsigned)pktSendMicrosec) -
                               currTime.tv_usec);
                    }
                } // currTime stuff
            } // if ((wsmPerformance != 0) && (wsmPerformance != 3))

            strncpy((char *)IPCharAddr, (char *)InWsmVer->IPCharAddr, sizeof(IPCharAddr));

            if (seqNum > wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx]) {
                wsmPrevSequenceNumber[rxRadioType][rxRadio][svcIdx] = seqNum;
            }

            wsmSequenceNumber[rxRadioType][rxRadio][svcIdx] = seqNum;

            if (!reduceCPULoading) {
                wsmLatitude[rxRadioType][rxRadio]  = ((double)((int32_t)ntohl(InWsmVer->latitude))) / 1E7;
                wsmLongitude[rxRadioType][rxRadio] = ((double)((int32_t)ntohl(InWsmVer->longitude))) / 1E7;
                wsmAltitude[rxRadioType][rxRadio]  = ((double)ntohs(InWsmVer->altitude)) / 10.0;
                wsmTpsError[rxRadioType][rxRadio]  = InWsmVer->tpsError;
            }

            wsmFirstPacketReceived[rxRadioType][rxRadio][svcIdx] = TRUE;

            if (wsmLog) {
                uint32_t pktSendSec = ntohl(InWsmVer->WsmSendSec);
                uint32_t pktSendMicrosec = ntohl(InWsmVer->WsmSendMicrosec);
                int dataRateDivisor;

                /* 10 MHz channels use half the data rate in WSM input structure
                 * 20 MHz channels use the exact data rate in WSM input structure */
                if ((wsmChannelNumber == 175) || (wsmChannelNumber == 181)) {
                    dataRateDivisor = 1;
                }
                else {
                    dataRateDivisor = 2;
                }

                if (reduceCPULoading) {
                    /* Only log what is critical for congestion testing */
                    sprintf(buf, "RX_PKT, %lu.%06lu, %u.%06u, 0x%8x,%u, %d, %u, %3.1f, %2u, %u, %d, %d \n",
                            (long unsigned)currTime.tv_sec, (long unsigned)currTime.tv_usec,
                            pktSendSec, pktSendMicrosec, currentPSID,
                            wsmSequenceNumber[rxRadioType][rxRadio][svcIdx], wsmRss[rxRadioType][rxRadio][svcIdx],
                            wsmChannelNumber, ((float) (Wsm->dataRate)) / dataRateDivisor,
                            Wsm->txPwrLevel, Wsm->dataLength,
                            localTpsError, Wsm->radioNum);
                }
                else {
                    if (wsmTpsError[rxRadio] || localTpsError) {
                        deltaDist = 0;
                    }
                    else {
                        deltaDist = getRelativeDistance(wsmLatitude[rxRadioType][rxRadio], tpsUserData.latitude,
                                                        wsmLongitude[rxRadioType][rxRadio], tpsUserData.longitude);
                    }

                    sprintf(buf, "RX_PKT, %lu.%06lu, %u.%06u, 0x%8x, %u, %d, %u, %3.1f, %2u, %u, %4.6f, %4.6f, %4.2f, %d, %4.6f, %4.6f, %4.2f, %d, %6.2f, %d",
                            (long unsigned)currTime.tv_sec, (long unsigned)currTime.tv_usec,
                            pktSendSec, pktSendMicrosec, currentPSID,
                            wsmSequenceNumber[rxRadioType][rxRadio][svcIdx], wsmRss[rxRadioType][rxRadio][svcIdx],
                            wsmChannelNumber, ((float) (Wsm->dataRate)) / dataRateDivisor,
                            Wsm->txPwrLevel, Wsm->dataLength, wsmLatitude[rxRadioType][rxRadio],
                            wsmLongitude[rxRadioType][rxRadio], wsmAltitude[rxRadioType][rxRadio],
                            wsmTpsError[rxRadioType][rxRadio], tpsUserData.latitude,
                            tpsUserData.longitude, tpsUserData.altitude,
                            localTpsError, deltaDist, Wsm->radioNum);

                    if (showAntRss) {
#ifdef INCLUDE_ANTAB_NOISE
                        sprintf(&buf[strlen(buf)], ", %5.1f, %5.1f, %5.1f, %5.1f ",
                                wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx],
                                wsmNoiseAntA[wsmRadioType][wsmRadio][svcIdx], wsmNoiseAntB[wsmRadioType][wsmRadio][svcIdx]);
#else
                        sprintf(&buf[strlen(buf)], ", %5.1f, %5.1f ",
                                wsmRssAntA[wsmRadioType][wsmRadio][svcIdx],   wsmRssAntB[wsmRadioType][wsmRadio][svcIdx]);
#endif
                    }

                    sprintf(&buf[strlen(buf)], " \n");
                }

                if (wsmPktLog) {
                    fwrite(buf, 1, strlen(buf), logpLogFileP);
                }
            } // if (wsmLog)
#if !defined(NO_SECURITY)
            /* If using VOD, activate the VOD thread to do the SMI verify call */
            if (useVOD) {
                VODMsgSeqNum = Wsm->vodMsgSeqNum;
                sem_post(&VOD_sem);
            }
#endif
        } // if (seqNum > 10)
    } // if (strncmp((const char *)charPtr, (const char *)"wsmtest.", 8) == 0)
}

static void logpFrameLogFileName(void)
{
    time_t     result1;
    struct tm *timeptr;
    uint32_t   logNamePSID;

    result1 = time(NULL);
    timeptr = localtime(&result1);

    /* Normally, we want the filename to have the displayPSID in it.  However, if some PSID could not
       be successfully registered for, then that most likely occured because that PSID was being used by the
       transmitting application.  If so, its better to use it as the identifier in the log filename. */
    if (firstErrorPSID == 0) {
        logNamePSID = displayPSID;
    }
    else {
        logNamePSID = firstErrorPSID;
    }

    sprintf(logfilename, "%s-%s-%d%.2d%.2d_%.2d%.2d%.2d-%X%s",
            "WSM",wsmStr, 1900 + timeptr->tm_year,
            (timeptr->tm_mon + 1),
            timeptr->tm_mday, timeptr->tm_hour,
            timeptr->tm_min, timeptr->tm_sec, logNamePSID, ".csv");
}


/* Call back function for TPS errors */
static void tpsErrorCallback(tpsResultCodeType code)
{
    char       errBuf[128];
    static int FirstError=TRUE;

    switch (code) {
    case TPS_CONN_LOST:
        printf("TPS Connection Lost. Exiting this app");
        callbackError = code || 1;
        break;

    case TPS_REG_LIST_FULL:
        sprintf(errBuf,"TPS Register List Full. Can't register now");
        break;

    case GPS_WRITE_ERROR:
        sprintf(errBuf,"GPS Write Error");
        break;

    case GPS_READ_ERROR_EXIT:
        sprintf(errBuf,"GPS READ Error. TPS Exits");
        break;

    case GPS_NMEA_CKSUM_ERROR:
        sprintf(errBuf,"GPS NMEA message checksum verification failed");
        break;

    case GPS_NMEA_MSG_PARSE_ERROR:
        sprintf(errBuf,"GPS NMEA message parsing error");
        break;

    case GPS_NO_NMEA_OUTPUT:
        sprintf(errBuf,"GPS no NMEA output");
        break;

    case GPS_NO_PPS_OUTPUT:
        sprintf(errBuf,"GPS no PPS output");
        break;

    default:
        sprintf(errBuf,"Unknown TPS error");
        break;
    }

    if (FirstError) {
        printf(">>>>>>>>>>>>>>>>>>>>>>>>ERROR: TPS ERROR callback returned %d [%s]\n", code, errBuf);
        printf(">>>>>>>>>>>>>>>>>>>>>>>>ERROR: TPS ERROR callback returned %d [%s]\n", code, errBuf);
        FirstError=FALSE;
    }
}

/* Call back function for Registering to get TPS data */
static void tpsCallback(tpsDataType *data)
{
    /* Copy the data and print it */
    if (data->valid) {
        tpsUserData.latitude  = data->latitude;
        tpsUserData.longitude = data->longitude;
        tpsUserData.altitude  = data->altitude;
        localTpsError         = 0;
    }
    else {
        tpsUserData.latitude  = 0;
        tpsUserData.longitude = 0;
        tpsUserData.altitude  = 0;
        localTpsError         = 1;
    }
}
#if !defined(NO_SECURITY)
static int securityInitialize(void)
{
    rsSecurityInitializeType init;
    int32_t                  retval;

    printf("Starting SMI Init.\n");

    init.debug                             = smiDebug;
    init.vodEnabled                        = (!vernRes || useVOD) ?
                                             TRUE : FALSE;
    init.smi_cert_change_callback          = NULL;
    init.vtp_milliseconds_per_verification = secsPerVerification * 1000;
    init.smi_sign_results_options          = smi_results_none;
    init.smi_sign_results_callback         = NULL;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_none;
    init.smi_verify_wsa_results_callback   = NULL;
    strcpy(init.lcmName, ""); // lcmName = NULL to signify no signing, only verification
    init.numSignResultsPSIDs               = 0; // Not signing anything
    retval = smiInitialize(&init);

    if (retval != 0) {
        printf("Error initializing security %d\n",
            retval);
        return EXIT_FAILURE;
    }

    connectedToSM = TRUE;
    printf("Security initialized.\n");
    return EXIT_SUCCESS;
}
#endif
int WsmParseArg(int argc, char *argv[])
{
    int i = 1;
    uint32_t temp;
    uint16_t index;
    uint8_t k;

    const char *pCommandArray[] =
    {
        "PROV",         // 0
        "PSID",         // 1
        "PERF",         // 2
        "SMIDEBUG",     // 3
        "DISP",         // 4
        "SPRI",         // 5
        "CCHPRI",       // 6 not supported
        "LOG",          // 7
        "STR",          // 8
        "ACCUM",        // 9
        "RAD",          // 10
        "SCH",          // 11
        "LESSCPU",      // 12
        "SECEN",        // 13
        "EXT",          // 14
        "ACCESS",       // 15
        "DEBUG",        // 16
        "RXANT",        // 17
        "PKTLOG",       // 18
        "STATSLOG",     // 19
        "WSASEC",       // 20
        "DISABLERADIO", // 21
        "VOD",          // 22
        "SPV",          // 23
        "VERNRES",      // 24
        "ANTRSS",       // 25
        "RTYPE",        // 26
    };

    /* Have MAX_CMD_BUF be the sze of the longest entry in pCommandArray */
    #define MAX_CMD_BUF sizeof("DISABLERADIO")
    #define MAX_CMDS    (sizeof(pCommandArray) / sizeof(pCommandArray[0]))
    char tempBuf[MAX_CMD_BUF];

    while (i < argc) {
        /* Be sure parameter begins with "--" */
        if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
            return 0;
        }

        bzero(tempBuf, MAX_CMD_BUF);

        /* Get the parameter name, converted to all upper case, in tempBuf */
        for (k = 0; (k < MAX_CMD_BUF) && (k < strlen(&argv[i][2])); k ++) {
            tempBuf[k] = (char)toupper((int)(argv[i][k + 2]));
        }

        tempBuf[k] = '\0';

        /* Find where the command is in pCommandArray */
        for (index = 0; index < MAX_CMDS; index++) {
            if (strncmp((const char *)pCommandArray[index],
                        (const char *)tempBuf,
                        strlen(pCommandArray[index])) == 0) {
                break;
            }
        }

        /* Error if not found */
        if (index == MAX_CMDS) {
            return 0;
        }

        /* Point to the value; error if command specified without a value */
        if (++i >= argc) {
            return 0;
        }

        /* Convert the parameter */
        switch (index) {
        case 8: // 8 = index of "STR" command
            /* String value is copied below */
            break;

        case 26: // 26 = index of "RTYPE" command
            if ((argv[i][0] == 'C') || (argv[i][0] == 'c')) {
                temp = 0; // Indicates radio type C-V2X
                break;
            }

            if ((argv[i][0] == 'D') || (argv[i][0] == 'd')) {
                temp = 1; // Indicates radio type DSRC
                break;
            }

            // fall thru

        default:
            temp = strtoul(argv[i], NULL, 0);
            break;
        }

        /* Process the parameter */
        switch (index) {
        case 0:     // PROV
            if (temp > 1) {
                return 0;
            }

            wsmProvider = (temp != 0) ? TRUE: FALSE;
            break;

        case 1:     // PSID
            uService.psid  =
            pService.psid  =
            displayPSID    =
            wsmPSID        = temp;
            break;

        case 2:     // PERF
            if (temp > 3) { // 0: disable packet detail log 1: enable packet detail log
                            // 2: display negative latencies, 3: disable latency calculations
                return 0;
            }

            wsmPerformance = temp;
            break;

        case 3:     // SMIDEBUG
            if (temp > 1) {
                return 0;
            }

            smiDebug = (temp != 0) ? TRUE: FALSE;
            break;

        case 4:     // DISP
            if (temp > 20) {
                return 0;
            }

            wsmDisplay = temp;
            break;

        case 5:     // SPRI
            if (temp > 63) {
               return 0;
            }

            uService.servicePriority = temp;
            break;

        case 6:     // CCHPRI
            /* Not supported */
            printf("CCHPRI not supported\n");
            return 0;
            break;

        case 7:     // LOG
            if (temp > 2) {
               return 0;
            }

            wsmLog = temp;
            break;

        case 8:     // STR
            strncpy(wsmStr, argv[i], 6);
            break;

        case 9:     // ACCUM
            if (temp > 1) {
                return 0;
            }

            wsmAccum = (temp != 0) ? TRUE: FALSE;
            break;

        case 10:    // RAD
            if (temp > 1) {
                return 0;
            }

            pService.radioNum =
            uService.radioNum =
            wsmRadio          = temp;
            break;

        case 11:    // SCH
#ifdef CHINA_FREQS_ONLY
            if (temp < 147  || temp > 169) {
                return 0;
            }

            if (temp % 2 == 0) {
                return 0;
            }

#else
            if ((temp != 76) && (temp != 84) && (temp != 88) &&
                (temp != 156) && (temp != 164) && (temp != 168)) {
                if (temp < 172  || temp > 184) {
                    return 0;
                }
            }
#endif
            uService.channelNumber = temp;
            wsmRadio0Channel = temp;
            wsmRadio1Channel = temp;
            break;

        case 12:    // LESSCPU
            if (temp > 1) {
                return 0;
            }

            reduceCPULoading = (temp != 0) ? TRUE: FALSE;
            break;

        case 13:    // SECEN
            if (temp > 1) {
                return 0;
            }

            secEnabled = (temp != 0) ? TRUE: FALSE;
            break;

        case 14:    // EXT
            if (temp > 65535) {
                return 0;
            }

            if (wsmProvider) {
                if (temp == 0) {
                    pService.channelAccess= 2;
                }
                else {
                    pService.channelAccess= 0;
                }
            }
            else {
                uService.extendedAccess = temp;    // Service Priority (0..65535)
            }

            break;

        case 15:    // ACCESS
            if (temp > 2) {
                return 0;
            }

            uService.userAccess = temp;
            break;

        case 16:    // DEBUG
            if (temp > 2) {
                return 0;
            }

            wsmDebug = temp;
            break;
        case 17:    // RXANT
            if (temp > 2) { // 0: diversity, 1, 2
                return 0;
            }

            wsmRxAnt = temp;
            break;

        case 18:    // PKTLOG
            if (temp > 1) {
                return 0;
            }

            wsmPktLog = (temp != 0) ? TRUE: FALSE;
            break;

        case 19:    // STATSLOG
            if ((temp != 0) && (temp != 100) &&
                (temp != 200) && (temp != 500) && (temp != 1000)) {
                return 0;
            }

            wsmStatsLog = temp;
            break;

        case 20:    // WSASEC
            if (temp > 1) {
                return 0;
            }

            pService.wsaSecurity = uService.wsaSecurity = temp;
            break;

        case 21:    // DISABLERADIO
            if (temp > 1) { // 0: Radio 0, 1: Radio 1
                return 0;
            }

            wsmDisableRadio = temp;
            break;
        case 22:    // VOD
            if (temp > 1) {
                return 0;
            }

            useVOD = (temp != 0) ? TRUE: FALSE;
            break;

        case 23:    // SPV
            secsPerVerification = temp;
            break;

        case 24:    // VERNRES
            if (temp > 1) {
                return 0;
            }

            vernRes = (temp != 0) ? TRUE: FALSE;
            break;

        case 25:    // ANTRSS
            if (temp > 1) {
                return 0;
            }

            showAntRss = (temp != 0) ? TRUE: FALSE;
            break;

        case 26:    // RTYPE
            if (temp > 1) {
                return 0;
            }

            pService.radioType =
            uService.radioType =
            wsmRadioType       = (temp == 0) ? RT_CV2X : RT_DSRC;
            break;

        default:
            return 0;
        } // switch (index)

        /* Move past the value to the next parameter, if any */
        i++;
    } // while (i < argc)

    return 1;
}

void WsmShowSyntax(int argc, char *argv[])
{
    printf("Usage: %s [--prov (0-1)][--psid (n)][--perf (0-3)][--smidebug (0-1)][--disp (0-20)]"
           "[--spri (0-63)][--log (0-2)][--str (string)][--accum (0-1)][--rtype (cv2x/dsrc)][--rad (0-1)]"
#ifdef CHINA_FREQS_ONLY
           " [--sch (147-169 odd)]\n"
#else
           " [--sch (76|84|88|156|164|168|172-184)]\n"
#endif
           "[--lesscpu (0-1)][--secen (0-1)][--vod (0-1)][--spv (n)][--wsasec (0-1)][--ext (0-65535)]"
           "[--access (0-2)][--debug (0-2)][--rxant (0-2)][--pktlog (0-1)][--statslog (0|100|200|500|1000)]"
           "[--vernres (0-1)][--antrss (0-1)][--disableradio (0-1)]\n", argv[0]);
}

/* This thread waits for the VOD semaphore to be given, then submits a WSM to
 * be verified */
static void *VODThread(void *arg)
{
    uint32_t submittedVODMsgSeqNum;
    int rc;
    int result;

    while (mainLoop) {
        sem_wait(&VOD_sem);

        if (!mainLoop) {
            break;
        }

        submittedVODMsgSeqNum = VODMsgSeqNum;
        VODRequestHandle = request_msg_verification(submittedVODMsgSeqNum, 100);

        if (VODRequestHandle < 0) {
            printf("Error submitting VODMsgSeqNum %u for verification (%d)\n",
                   submittedVODMsgSeqNum, VODRequestHandle);
            continue;
        }

        rc = wait_for_vod_result(VODRequestHandle);

        if (!mainLoop) {
            break;
        }

        if (!rc) {
            printf("wait_for_vod_result(%d) error (%d)\n", VODRequestHandle, rc);
            continue;
        }

        result = retrieve_vod_result(VODRequestHandle);
        VODRequestHandle = -1;

        if (vernRes) {
            printf("Verify result = %d; seqnum = %u\n", result, submittedVODMsgSeqNum);
        }
    }

    pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
    rsResultCodeType  risRet;
    tpsResultCodeType tpsRet;
    int               i, j, k;
    cfgType           cfg;
    cfgType           getCfg;
    bool_t            cv2x_enable = FALSE;
    bool_t            dsrc_enable = FALSE;
    struct sigaction  sa;
    struct sigevent   evnt;
    char              buf[380];
    struct timeval    currTime;
    uint16_t           pollingTime;

    initStatic();

    memset(buf,0x0,sizeof(buf));

    /* Do any initialization first */
    for (i = 0; i < MAX_RADIO_TYPES; i++) {
        for (j = 0; j < MAX_RADIOS; j++) {
            for (k = 0; k < MAX_SERVICES; k++) {
                wsmMinLatency[i][j][k] = 9999999;
            }
        }
    }

    if (argc > 1) {
        if (argv[1][0] == '?') {
            WsmShowSyntax(argc, argv);
            exit(0);
        }

        if (WsmParseArg(argc, argv) == 0) {
            WsmShowSyntax(argc, argv);
            exit(0);
        }
    }

    /* Get the value of the nsconfig variable CV2X_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_CV2X_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        cv2x_enable = getCfg.u.CV2XEnable;
    }

    /* Get the value of the nsconfig variable DSRC_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_DSRC_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        dsrc_enable = getCfg.u.DSRCEnable;
    }

    /* Error if no radios enabled */
    if (!cv2x_enable && !dsrc_enable) {
        printf("No radios enabled! Enable either the C-V2X or the DSRC radio.\n");
        exit(0);
    }

    /* If one radio is enabled, but not the other, override the --rtype value */
    if (cv2x_enable && !dsrc_enable) {
        pService.radioType =
        uService.radioType =
        wsmRadioType       = RT_CV2X;
    }
    else if (!cv2x_enable && dsrc_enable) {
        pService.radioType =
        uService.radioType =
        wsmRadioType       = RT_DSRC;
    }

    /* If using VOD, initialize the VOD semaphore and spawn the VOD thread */
    if (useVOD) {
        if (!init_with_vod("WSM_RECV")) {
            printf("Error initializing VOD\n");
            exit(0);
        }

        sem_init(&VOD_sem, 0, 0);
        pthread_create(&VODThreadId, NULL, VODThread, NULL);
    }

    if (wsmLog) {
        if (wsmLog == 1) {
            if ((logpLogFileP = fopen("/mnt/rwflash/wsm/logs", "r")) == NULL)
            {
                system("mkdir -p /mnt/rwflash/wsm/logs");
                system("sleep 3");
            }
        } else if (wsmLog == 2) {
            if ((logpLogFileP = fopen("/mnt/usbdrv/wsm/logs", "r")) == NULL)
            {
                system("mkdir -p /mnt/usbdrv/wsm/logs");
                system("sleep 3");
            }
        }
    }


    /* Start - Init Radio Communication */
    if (wsmDebug == 0) {
        if ((risRet = wsuRisInitRadioComm("wsm_recv", &appCredential)) != RS_SUCCESS) {
            printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
            return EXIT_FAILURE;
        }
    }
    else if ((risRet = wsuRisInitRadioComm("wsm_recv", &appCredentialDebug)) != RS_SUCCESS) {
        printf("wsuRisInitRadioComm Debug failed (%s)\n", rsResultCodeType2Str(risRet));
        return EXIT_FAILURE;
    }

    printf("RadioComm Initialized\n");

    /*
     * Set Radio ANTENNA (1st Antenna, 2nd Antenna, Both Antennas(Diversity)
     */
    if (wsmRadioType == RT_DSRC) {
        if ((wsmRxAnt == 1) || (wsmRxAnt == 2) || (wsmRxAnt == 0)) {
            printf("Set Rx Antenna config wsmRxAnt=%d %s\n", wsmRxAnt, (wsmRxAnt ? "" : "[Diversity Enabled]"));
            cfg.u.RxAntenna = wsmRxAnt;

            if (wsmRadio == 0) { /*Radio 0*/
                cfg.radioType = wsmRadioType;

                if ((risRet = wsuRisSetCfgReq(CFG_ID_ANTENNA_RADIO_0, &cfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisSetCfgReq UNSUPPORTED: Unable to set CFG_ID_ANTENNA_RADIO_0. Continuing.\n");
                    }
                    else {
                        printf("Error: wsuRisSetCfgReq CFG_ID_ANTENNA_RADIO_0 setting failed (%s)\n", rsResultCodeType2Str(risRet));
                        exit(1);
                    }
                }

                getCfg.radioType = wsmRadioType;

                if ((risRet = wsuRisGetCfgReq(CFG_ID_ANTENNA_RADIO_0, &getCfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisGetCfgReq UNSUPPORTED: Unable to get CFG_ID_ANTENNA_RADIO_0. Continuing.\n");
                        getCfg.u.RxAntenna = cfg.u.RxAntenna;   //To allow program to run.
                    }
                    else {
                        printf("Error: wsuRisGetCfgReq CFG_ID_ANTENNA_RADIO_0 getting failed (%s)\n", rsResultCodeType2Str(risRet));
                        exit(1);
                    }
                }

                if (cfg.u.RxAntenna != getCfg.u.RxAntenna) {
                    printf("RxAntenna value 0 is not setting/getting properly. Exiting...\n");
                    exit(1);
                }
                else {
                    printf("Radio 0 RxAntenna = %d\n", cfg.u.RxAntenna);
                }
            }
            else { /*Radio 1*/
                cfg.radioType = wsmRadioType;

                if ((risRet = wsuRisSetCfgReq(CFG_ID_ANTENNA_RADIO_1, &cfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisSetCfgReq UNSUPPORTED: Unable to set CFG_ID_ANTENNA_RADIO_1. Continuing.\n");
                    }
                    else {
                        printf("Error: wsuRisSetCfgReq CFG_ID_ANTENNA_RADIO_1 setting failed (%s)\n", rsResultCodeType2Str(risRet));
                        exit(1);
                    }
                }

                getCfg.radioType = wsmRadioType;

                if ((risRet = wsuRisGetCfgReq(CFG_ID_ANTENNA_RADIO_1, &getCfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisGetCfgReq UNSUPPORTED: Unable to get CFG_ID_ANTENNA_RADIO_1. Continuing.\n");
                        getCfg.u.RxAntenna = cfg.u.RxAntenna;   //To allow program to run.
                    }
                    else {
                        printf("Error: wsuRisGetCfgReq CFG_ID_ANTENNA_RADIO_1 getting failed (%s)\n", rsResultCodeType2Str(risRet));
                        exit(1);
                    }
                }
                if (cfg.u.RxAntenna != getCfg.u.RxAntenna) {
                    printf("RxAntenna value 1 is not setting/getting properly. Exiting... \n");
                    exit(1);
                }
                else {
                    printf("Radio 1 RxAntenna = %d\n", cfg.u.RxAntenna);
                }
            }
        }
    }

    if (wsmRadioType == RT_DSRC) {
        if (wsmDisableRadio == 0 || wsmDisableRadio == 1) {
            printf("Set Receiver Mode to RecvMode=%d (RECEIVER_DISABLE)\n", RECEIVER_DISABLE);
            cfg.u.RecvMode = RECEIVER_DISABLE;
            cfg.radioType  = wsmRadioType;

            if (wsmDisableRadio == 0) {
                if ((risRet = wsuRisSetCfgReq(CFG_ID_RECEIVER_MODE_RADIO_0, &cfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisSetCfgReq UNSUPPORTED: Unable to set CFG_ID_RECEIVER_MODE_RADIO_0. Continuing.\n");
                    }
                    else {
                        printf("Error: wsuRisSetCfgReq CFG_ID_RECEIVER_MODE_RADIO_0 setting failed (%s)\n", rsResultCodeType2Str(risRet));
                        return risRet;
                    }
                }
            }
            else {
                if ((risRet = wsuRisSetCfgReq(CFG_ID_RECEIVER_MODE_RADIO_1, &cfg)) != RS_SUCCESS) {
                    if (risRet == RS_ENOSUPPORT) {
                        printf("wsuRisSetCfgReq UNSUPPORTED: Unable to get CFG_ID_RECEIVER_MODE_RADIO_1. Continuing.\n");
                        printf("-----Radio NOT Disabled");
                    }
                    else {
                        printf("Error: wsuRisSetCfgReq CFG_ID_RECEIVER_MODE_RADIO_1 getting failed (%s)\n", rsResultCodeType2Str(risRet));
                        return risRet;
                    }
                }
            }
        }
    }
    
    tpsRet = wsuTpsInit();

    if (tpsRet != TPS_SUCCESS) {
        printf("ERROR: TPS API init failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        exit(1);
    }

    wsuTpsRegisterErrorHandler(tpsErrorCallback);
    tpsRet = wsuTpsRegister(tpsCallback);

    if (tpsRet != TPS_SUCCESS) {
        printf("ERROR: TPS API register failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        exit(1);
    }

    if (wsmProvider) { // Provider mode
        printf("Set Station Role as Provider\n");

        /* Register Services */
        pService.action = ADD;
        pService.wsaHeaderExtension = WSA_EXT_REPEAT_RATE_BITMASK |
            WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |
            WSA_EXT_CHANNEL_ACCESS_BITMASK;

        if ((risRet = wsuRisProviderServiceRequest(&pService)) != RS_SUCCESS) {
            printf("wsuRisProviderServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return EXIT_FAILURE;
        }
        else {
            printf("wsuRisProviderServiceRequest (PSID=0x%8x)\n", pService.psid);
        }

        service.action    = ADD;
        service.radioType = wsmRadioType;
        service.psid =     uService.psid;

        if ((risRet = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
            printf("wsuRisWsmServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("wsuRisWsmServiceRequest ADD WSM Service (PSID=0x%8x)\n", service.psid);
        }

        displayPSID = wsmPSID;
    }
    else { // User mode
        /* Register Services */
        uService.action = ADD;
        if ((risRet = wsuRisUserServiceRequest(&uService)) != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return EXIT_FAILURE;
        }
        else {
            printf("User Registered Service (PSID=0x%8x)\n", uService.psid);
        }

        service.action    = ADD;
        service.radioType = wsmRadioType;
        service.psid      = uService.psid;

        if ((risRet = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
            printf("wsuRisWsmServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("wsuRisWsmServiceRequest ADD WSM Service (PSID=0x%8x)\n", service.psid);
        }

        displayPSID = wsmPSID;
    }

    /* To reduce CPU loading for channel congestion tests, disable receive on 2nd radio */
    if (reduceCPULoading) {
        cfg.u.RecvMode = RECEIVER_DISABLE;
        cfg.radioType  = wsmRadioType;
        printf("Disabling receive on radio 1\n");

        if ((risRet = wsuRisSetCfgReq(CFG_ID_RECEIVER_MODE_RADIO_1, &cfg)) != RS_SUCCESS) {
            printf("wsuRisSetCfgReq CFG_ID_RECEIVER_MODE_RADIO_1, error setting failed (%s)\n"
                    "[Radio 1 NOT Disabled]", rsResultCodeType2Str(risRet));
        }
    }

    signal(SIGINT,  WsmExitHandler);
    signal(SIGTERM, WsmExitHandler);
    signal(SIGKILL, WsmExitHandler);

    /* If enabled, create log file and write header information */
    if (wsmLog) {
       char tempStr[48];

       logpFrameLogFileName();

       if (wsmLog == 1) {
           sprintf(tempStr, "/mnt/rwflash/wsm/logs/");
       }
       else if (wsmLog == 2) {
           sprintf(tempStr, "/mnt/usbdrv/wsm/logs/");
       }

       filePath = malloc(strlen(tempStr) + strlen(logfilename) + 1);

       if (filePath == NULL) {
           perror("malloc failed");
           exit(1);
       }

       strcpy(filePath, tempStr);
       strcat(filePath, logfilename);

       if ((logpLogFileP = fopen(filePath, "w")) == NULL) {
           perror("fopen");
           free(filePath);
           exit(1);
       }

       if (wsmPktLog) {
           if (reduceCPULoading) {
               /* Log less information to reduce CPU loading */
               sprintf(buf, "Rx_Pkt Type, Rx Time(secs.us), Tx Time(secs.us), PSID, Sequence #,"
                       " RSS, Channel, Data Rate, Tx Power, Data Len, Local GPS Err, Rx Radio\n");
           }
           else {
               sprintf(buf, "Rx_Pkt Type, Rx Time(secs.us), Tx Time(secs.us), PSID, Sequence #, RSS, Channel, Data Rate, Tx Power, Data Len, Remote lat, Remote long, Remote alt, Remote err, Local lat, Local long, Local alt, Local err, Distance, Rx Radio");

               if (showAntRss) {
#ifdef INCLUDE_ANTAB_NOISE
                   sprintf(&buf[strlen(buf)], ", rssAntA, rssAntB, noiseAntA, noiseAnt");
#else
                   sprintf(&buf[strlen(buf)], ", rssAntA, rssAntB");
#endif
               }

               sprintf(&buf[strlen(buf)], " \n");
           }

           fwrite(buf, 1, strlen(buf), logpLogFileP);
       }

       if (wsmStatsLog) {
           sprintf(buf,"STATS, RxPktMbps, RxPktGood, RxPktBad, RxPktsPerSec, RxPktPER, RxPktMbpsCum, PktGoodCum, PktBadCum, PktsPerSecCum, PktPERAvg, Rss, RssIAvg, LocUTCDate, LocUTCTime, LocalLat, LocalLong, RemUTCDate, RemUTCTime, RemoteLat, RemoteLong, RemoteIP, Distance");

           if (showAntRss) {
               sprintf(&buf[strlen(buf)], ", RssIAvgAntA, RssIAvgAntB");
           }

           sprintf(&buf[strlen(buf)], " \n");
           fwrite(buf, 1, strlen(buf), logpLogFileP);
       }
    }
#if !defined(NO_SECURITY)
    if (secEnabled) {
        if (securityInitialize() == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }

    if (wsmDisplay && wsmDebug == 0) {
        memset(&evnt, 0, sizeof(evnt));
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = WsmDisplayHandler;
        sa.sa_flags   = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR1, &sa, NULL);
        evnt.sigev_signo = SIGUSR1;

        if (timer_create(CLOCK_REALTIME, &evnt, &dispTmrId) == -1) {
            printf("Error: Display timer_create failed\n");
            exit(1);
        }

        dispTmrCreated = TRUE;
        dispTmrTv.it_value.tv_sec = wsmDisplay;
        dispTmrTv.it_value.tv_nsec = 0;
        dispTmrTv.it_interval.tv_sec = wsmDisplay;
        dispTmrTv.it_interval.tv_nsec = 0;

        if (timer_settime(dispTmrId, 0, &dispTmrTv, NULL) == -1) {
           printf("Error setting Display Timer\n");
           exit(1);
        }
    }
#endif
    if (wsmPerformance != 0 || wsmStatsLog !=0) {
        if (wsmStatsLog) {
            pollingTime = wsmStatsLog;
        }
        else {
            pollingTime = 500;
        }

        /* Create radio poll timer to execute every 500 ms */
        memset(&evnt, 0, sizeof(evnt));
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = pollRadioHandler;
        sa.sa_flags   = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR2, &sa, NULL);
        evnt.sigev_signo = SIGUSR2;

        if (timer_create(CLOCK_REALTIME, &evnt, &pollRadioTmrId) == -1) {
            printf("Error: Poll Radio timer_create failed\n");
            exit(1);
        }

        pollRadioTmrCreated = TRUE;
        pollRadioTmrTv.it_value.tv_sec = pollingTime / 1000;
        pollRadioTmrTv.it_value.tv_nsec = (pollingTime % 1000) * 1000000;
        pollRadioTmrTv.it_interval.tv_sec = pollingTime / 1000;
        pollRadioTmrTv.it_interval.tv_nsec = (pollingTime % 1000) * 1000000;

        if (timer_settime(pollRadioTmrId, 0, &pollRadioTmrTv, NULL) == -1) {
           printf("Error setting Poll Radio Timer\n");
           exit(1);
        }
    }

    gettimeofday(&currTime, NULL);
    testStartTime = currTime.tv_sec * 1000000 + currTime.tv_usec;

    // ready to catch signal
    while (mainLoop && (callbackError == 0)) {
        sleep(1);
    }

    if (!mainLoop) {
        WsmCleanHandler();
    }

    /* Stop Display Timer */
    if (dispTmrCreated) {
        memset(&dispTmrTv, 0, sizeof(dispTmrTv));

        if (timer_settime(dispTmrId, 0, &dispTmrTv, NULL) == -1) {
            printf("Error stoping Display Timer(%d)\n",errno);
        }

        timer_delete(dispTmrId);
    }

    /* Stop Poll Radio Timer */
    if (pollRadioTmrCreated) {
        memset(&pollRadioTmrTv, 0, sizeof(pollRadioTmrTv));
        
        if (timer_settime(pollRadioTmrId, 0, &pollRadioTmrTv, NULL) == -1) {
            printf("Error stoping Tx Timer (%d)\n",errno);
        }

        timer_delete(pollRadioTmrId);
    }

    /* Tell TPS we don't need him any more */
    tpsRet = wsuTpsDeregister();

    if (tpsRet != TPS_SUCCESS) {
        printf("ERROR: wsuTpsDeregister failed (%s)\n", tpsResultCodeType2Str(tpsRet));
    }

    wsuTpsTerm();
    return EXIT_SUCCESS;
}

