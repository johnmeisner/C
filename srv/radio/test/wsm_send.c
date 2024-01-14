/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: wsm_send.c                                                       */
/*  Purpose: Test program that sends WSMs                                     */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-04-04  VROLLINGER    Initial revision.                                */
/* 2021-05-05  TMACAULAY     Added HEAVY_DUTY aspects.                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include "ris.h"
#include "tps_api.h"
#include "alsmi_api.h"
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/
static void service_available(serviceInfoType *serviceInfo);
static void receive_wsm(inWSMType *Wsm);
static void txTmrHandler(int signum);
#if 0
static void precise_usleep(uint16_t txOffsetUs);
#endif

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define WSM_PSID               0x20
#define MAX_TX_EMULATION_ARRAY 50
#define MAXADDR_LEN            32

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef struct OutWsmVer {
    uint8_t  WsmData[8];
    uint32_t WsmSequenceNumber;
    uint32_t WsmSendSec;
    uint32_t WsmSendMicrosec;
    int32_t  latitude;
    int32_t  longitude;
    uint16_t altitude;
    uint8_t  tpsError;
    uint8_t  IPCharAddr[20];
} __attribute__((packed)) OutWsmMsgData;

typedef struct {
    struct timeval absoluteTxTime;
    uint32_t       relativeTxTimeMs;
} nextTxTimeStruct;

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static appCredentialsType appCredential =
{
    .cbServiceAvailable = service_available,
    .cbReceiveWSM       = receive_wsm,
};

static outWSMType OutWsmMsg =
{
#ifdef CHINA_FREQS_ONLY
    .channelNumber = 153,
#else
    .channelNumber = 178,
#endif
    .dataRate       = DR_6_MBPS,
    .txPwrLevel     = 20,
    .txPriority     = 2,
    .radioType      = RT_CV2X,
    .radioNum       = 0,
    .security       = SECURITY_UNSECURED,
    .securityFlag   = 0,
    .peerMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .wsmpHeaderExt  = 0,
    .psid           = WSM_PSID,
    .dataLength     = 10,
    .isBitmappedSsp = 0,
    .sspLen         = 0,
};

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
    .edca_count                = 3,
    .txop                      = {0, 0, 0, 0},
    .ecw                       = {0x43, 0x43, 0x43, 0x43},
    .aifs                      = {0x03, 0x27, 0x42, 0x62},
    .channelAccess             = 2,
    .repeatRate                = 50,
    .ipService                 = 0,
    .IPv6Address               = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0xf1, 0x01,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00},
    .IPv6ServicePort           = 23000,
    .providerMACAddress        = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    .rcpiThreshold             = 10,
    .wsaCountThreshold         = 20,
    .wsaCountThresholdInterval = 30,
    .wsaHeaderExtension        = WSA_EXT_WRA_BITMASK |
                                 WSA_EXT_2DLOCATION_BITMASK |
                                 WSA_EXT_3DLOCATION_BITMASK |
                                 WSA_EXT_ADVERTISER_ID_BITMASK |
                                 WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |
                                 WSA_EXT_IPV6ADDRESS_BITMASK |
                                 WSA_EXT_SERVICE_PORT_BITMASK |
                                 WSA_EXT_PROVIDER_MACADDRESS_BITMASK |
                                 WSA_EXT_EDCA_PARAM_SET_BITMASK |
                                 WSA_EXT_SECONDARY_DNS_BITMASK |
                                 WSA_EXT_GATEWAY_MACADDRESS_BITMASK |
                                 WSA_EXT_REPEAT_RATE_BITMASK |
                                 WSA_EXT_RCPI_THRESHOLD_BITMASK |
                                 WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK |
                                 WSA_EXT_CHANNEL_ACCESS_BITMASK |
                                 WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK,
    .signatureLifeTime         = 1000,
    .advertiseIdLength         = 11,
    .repeatRate                = 10,
    .latitude2D                = 331334000,
    .longitude2D               = -1172274667,
    .latitude3D                = 331334000,
    .longitude3D               = -1172274667,
    .elevation3D               = 1259,
    .confidence3D              = 0x11,
    .semiMajorAccuracy3D       = 1,
    .semiMinorAccuracy3D       = 1,
    .semiMajorOrientation3D    = 1,
    .advertiseId               = "advertiseId",
    .routerLifeTime            = 0x1234,
    .ipPrefix                  = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0xf1, 0x01,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .prefixLen                 = 64,
    .defaultGateway            = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0xf1, 0x01,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    .primaryDns                = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0xf1, 0x01,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01},
    .secondaryDns              = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0xf1, 0x01,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02},
    .gatewayMacAddress         = {0xaa, 0xbb, 0xec, 0xed, 0xfe, 0xff},
};

static UserServiceType uService =
{
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
    .extendedAccess    = 65535,
};

static OutWsmMsgData     OutWsmMsgPayload;
static uint8_t           IPCharAddr[20];
static bool_t            mainLoop = TRUE;
static uint8_t           callbackError        = 0;
static uint16_t          pktCollisionTestMode = 0; /* 0=disable pkt collision test,
                                                    * 1=set GPIO, delay, and Tx,
                                                    * 2=detect GPIO and Tx */
static uint16_t          txOffsetUs           = 0; /* amount of time to delay after
                                                    * GPIO is set for packet collision test */
static bool_t            wsmCCH               = TRUE;
static bool_t            wsmProvider          = FALSE;
static bool_t            wBSSJoined           = FALSE;
static bool_t            adjustDataRate       = FALSE;
static uint16_t          wsmDelay             = 20;
static uint16_t          wsmPayloadLen        = sizeof(OutWsmMsgData) + 3; // +3 for "WSM" at the end
static uint16_t          busyCounter          = 0;
static uint16_t          wsmDisplay           = 1;
static bool_t            hardCodeData         = 0;
static uint16_t          wsmTxAnt             = 1;
static char              wsmUni               = FALSE;
static char              wsmUnicast[20];
static char              wsmUnicast2[6]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static int               chInterval[2];
static int               randDelay            = 0;
static bool_t            busy                 = FALSE;
static struct itimerspec txTmrTv;
static struct itimerspec dispTmrTv;
static timer_t           txTmrId;
static timer_t           dispTmrId;
#ifdef CHINA_FREQS_ONLY
static int               wsmRadio0Channel     = 153;
static int               wsmRadio1Channel     = 153;
#else
static int               wsmRadio0Channel     = 172;
static int               wsmRadio1Channel     = 172;
#endif
static int               userServiceChan      = 0;
static rsRadioType       wsmRadioType         = RT_CV2X;
static int8_t            wsmRadio             = 0;
static bool_t            txTmrContinueFlag    = FALSE;
/* These variables are used for channel congestion testing */
static int               emulatedVehicles     = 1;
static int               emulatedTxRateHz     = 0;
static int               actualRadios         = 1;
static nextTxTimeStruct  nextTxTimeArray[MAX_TX_EMULATION_ARRAY];
static int               nextTxTimeIdx        = 1;
static int               lastTxTimeIdx        = 0;
static uint32_t          secEnabled           = 0;
static uint32_t          signRes              = 0;
static bool_t            smiDebug             = FALSE;
static pthread_mutex_t   txThreadMutex        = PTHREAD_MUTEX_INITIALIZER;
static bool_t            output_esc_codes     = TRUE;

#if defined(HEAVY_DUTY)
#define MIN_WSM_DATA (8)
static uint8_t my_wsm_data[MAX_WSM_DATA];
static int     override_payload = 0;
static int     my_wsm_len = 0;
#endif
/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
#if defined(HEAVY_DUTY)
/* Will convert 8-bit ascii hex value to 4 bit nibble.
   Will return between 0x0 to 0xf.
   Will return 0xff on out of range ascii value.
*/
uint8_t i2v_ascii_2_hex_nibble(uint8_t  value)
{       
  uint8_t dummy = 0xFF;

  if(( '0' <= value) && (value <= '9'))
      dummy =  value - 48;
  if(( 'A' <= value) && (value <= 'F'))
      dummy =  value - 55;
  if(( 'a' <= value) && (value <= 'f'))
      dummy =  value - 87;

  return dummy;
}
#endif

/* This function is used when calling qsort() */
int compare_ints (const void *a, const void *b)
{
    const int *da = (const int *) a;
    const int *db = (const int *) b;

    return (*da > *db) - (*da < *db);
}

/* Fill transmit emulation array with time for the next set of transmissions.  The
   transmit emulation array is filled with some number of random sets of times in
   which the application should transmit messages to meet the needs of the congestion
   testing. */
static void fillTxEmulationArray()
{
    int            totalTxPerSec;
    double         numTxPer100MsPeriodDouble;
    int            numTx, txPeriodMs, i;
    struct timeval currTime;
    int            randTxArrayMs[MAX_TX_EMULATION_ARRAY];
    static bool_t  haventPrintedYet = TRUE;
    uint32_t       unusedTimeFromLastPeriodMs;

    /* Guard against divide by zero */
    if ((emulatedVehicles == 0) || (emulatedTxRateHz == 0)) {
        printf("fillTxEmulationArray() Error Exit! emulatedVehicles=%d, emulatedTxRateHz=%d\n",
               emulatedVehicles, emulatedTxRateHz);
        exit(1);
    }

    /* The following global input parameters are needed:
           actualRadios = Total number of radios being used for congestion testing
           emulatedVehicles = Total number of vehicles being emulated in congestion testing
           emulatedTxRateHz = Tx Rate that each emulated vehicle is supposedly transmitting
       As a result, the following values are calculated:
           totalTxPerSec = Total number of packets transmitted each second by all radios to emulate all vehicles
           numTxPer100MsPeriodDouble = Number of transmissions needed per 100ms for each radio (floating point)
           numTx = Number of transmissions needed per txPeriodMs to meet the above goals (integer value)
           txPeriodMs = The period of time (ms) to repeat a new random sequence of transmissions (integer value) */
    totalTxPerSec             = emulatedVehicles * emulatedTxRateHz;
    numTxPer100MsPeriodDouble = (double)totalTxPerSec /
                                ((double)actualRadios * 10.0);
    numTx                     = (int) numTxPer100MsPeriodDouble;

    if (numTx < 1) {
        numTx = 1;
    }

    txPeriodMs = (int)(((double)numTx*100.0) / numTxPer100MsPeriodDouble);

    if (haventPrintedYet) {
        printf("emVeh=%d, actRadios=%d, emTxRate=%d, totTxPerSec=%d, numTxPer100msDouble=%f, numTx=%d, txPeriodMs=%d\n",
               emulatedVehicles, actualRadios, emulatedTxRateHz, totalTxPerSec, numTxPer100MsPeriodDouble,
               numTx, txPeriodMs);
        haventPrintedYet = FALSE;
    }

    /* Populate transmit emulation array with random numbers between 0..txPeriodMs.
       Sort array in ascending order. */
    if (numTx >= MAX_TX_EMULATION_ARRAY) {
        printf("# of required transmissions exceeds transmit emulation array (%d)\n", numTx);
        exit(1);
    }

    for (i = 0; i < numTx; i++) {
        randTxArrayMs[i] = random() % txPeriodMs;
    }

    qsort(randTxArrayMs, numTx, sizeof(int), compare_ints);

    /* Obtain remaining time from last period before over-writing array.  This will be
       used to adjust the new set of times so we don't start transmitting the new set too soon. */
    unusedTimeFromLastPeriodMs = txPeriodMs - nextTxTimeArray[numTx-1].relativeTxTimeMs;

    /* Create array of absolute times for when to transmit each message */
    gettimeofday(&currTime, NULL);

    for (i = 0; i < numTx; i++) {
        nextTxTimeArray[i].relativeTxTimeMs = randTxArrayMs[i];
        nextTxTimeArray[i].absoluteTxTime.tv_usec =
          (currTime.tv_usec + (unusedTimeFromLastPeriodMs + randTxArrayMs[i]) * 1000) % 1000000;

        if (((currTime.tv_usec + (unusedTimeFromLastPeriodMs + randTxArrayMs[i]) * 1000) / 1000000) == 0) {
            nextTxTimeArray[i].absoluteTxTime.tv_sec = currTime.tv_sec;
        }
        else {
            nextTxTimeArray[i].absoluteTxTime.tv_sec = currTime.tv_sec + 1;
        }
    }

#if 0
    printf("currTime = %ld.%06ld\n", currTime.tv_sec, currTime.tv_usec);
    for (i = 0; i < numTx; i++) {
        printf("nextTxTimeArray[%d] = %ld.%06ld, (%d ms)\n",
               i, nextTxTimeArray[i].absoluteTxTime.tv_sec, nextTxTimeArray[i].absoluteTxTime.tv_usec,
               nextTxTimeArray[i].relativeTxTimeMs);
    }
#endif

    /* Reset array index counters */
    nextTxTimeIdx = 0;
    lastTxTimeIdx = numTx - 1;
}

/* This function returns the next time that a message should be transmitted for the congestion
   testing.  The absolute time is put into nextTxTime and the relative time
   from current time is returned directly by this function (in ms). */
static int getNextTxTime(struct timeval *nextTxTime)
{
    struct timeval currTime;
    long           timeDiffMs;

    gettimeofday(&currTime, NULL);

    /* Check if its time to fill-up the transmit array */
    if (nextTxTimeIdx > lastTxTimeIdx) {
        fillTxEmulationArray();
    }

    nextTxTime->tv_sec = nextTxTimeArray[nextTxTimeIdx].absoluteTxTime.tv_sec;
    nextTxTime->tv_usec = nextTxTimeArray[nextTxTimeIdx].absoluteTxTime.tv_usec;
    nextTxTimeIdx++;

    if (currTime.tv_sec <= nextTxTime->tv_sec) {
        timeDiffMs = ((((nextTxTime->tv_sec - currTime.tv_sec) * 1000000) + nextTxTime->tv_usec) -
                         currTime.tv_usec + 500)/1000;
    }
    else {
        timeDiffMs = 0;
    }

    /* If we've fallen behind and should have transmitted already, set timeDiffMs to 0 */
    if (timeDiffMs < 0) {
        timeDiffMs = 0;
        //printf("currtime=%ld.%06ld, nextTxTime=%ld.%06ld\n",
        //  currTime.tv_sec, currTime.tv_usec, nextTxTime->tv_sec,nextTxTime->tv_usec);
    }

    return (int)timeDiffMs;
}

static void WsmGetTime(void)
{
    struct timeval currTime;

    gettimeofday(&currTime, NULL);
    currTime.tv_usec += 500;  // Round up 500us
    currTime.tv_usec /= 1000; // Convert to ms
    currTime.tv_usec *= 1000; // Convert back to us
    OutWsmMsgPayload.WsmSendSec      = htonl(currTime.tv_sec);
    OutWsmMsgPayload.WsmSendMicrosec = htonl(currTime.tv_usec);
}

void WsmExitHandler(int signal)
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");
    }

    mainLoop = FALSE;
}

void WsmCleanHandler()
{
    rsResultCodeType risRet;
    static int       term_times = 0;

    printf("CleanHandler is called.\n");
    if (term_times > 0) {
        exit(-1);
    }

    term_times++;

    if (wsmProvider) { // Provider mode
        printf("Set Station Role as Provider\n");

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
    }

    /* Done - Terminate Radio Communication */
    if ((risRet = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
        printf("wsuRisTerminateRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return;
    }

    printf("Total # Packets Tranmitted (Last Sequence Number)   = %d\n",
           ntohl(OutWsmMsgPayload.WsmSequenceNumber));
    printf("RadioComm Terminated\n");
    return;
}

void WsmDisplayHandler(int signal)
{
    static int Last_WsmSequenceNumber = 0;

    if (signRes) {
        /* Don't clear or print a lot of stuff on the screen if we want sign
         * results. This will make the results of the sign result easier to
         * see. */
        printf("Sequence number= %d\n", ntohl(OutWsmMsgPayload.WsmSequenceNumber));
        return;
    }

    // Clear and jump to top of screen
    if (output_esc_codes) {
        printf("\033[2J\033[1;1H");
    }
    // Print output
    {
        printf("Psid           = 0x%4x\n", uService.psid);
        printf("CCH            = %2d\n", wsmCCH);
        printf("TxPriority     = %2d \n", OutWsmMsg.txPriority);
        printf("DataRate (MCS) = %2d \n", OutWsmMsg.dataRate);
        printf("TxPower  (dBm) = %4.1f \n", OutWsmMsg.txPwrLevel);
        printf("Service Radio  = %s %2d, Priority = %d\n",
               uService.radioType == RT_CV2X ? "C-V2X" : "DSRC",
               uService.radioNum, uService.servicePriority);

        if (!wsmProvider) { // User mode
            printf("UserChannel    = %d\n", uService.channelNumber);
            printf("ExtendedAccess = %d\n", uService.extendedAccess);
        }
        else {
            printf("Provider Channel = %d\n", pService.channelNumber);
            printf("ChannelAccess  = %d\n", pService.channelAccess);
        }

        printf("TxChannel      = %d\n", OutWsmMsg.channelNumber);
        printf("Sequence number= %d      (%d)\n", ntohl(OutWsmMsgPayload.WsmSequenceNumber),
               (ntohl(OutWsmMsgPayload.WsmSequenceNumber)-Last_WsmSequenceNumber));
        if (!wsmProvider) {
            printf("WBSS Joined    = %d\n", wBSSJoined);
        }
        else {
            printf("Provider Mode\n");
        }
    }

    chInterval[0] = chInterval[1] = 0;

    if (wsmUni) {
        printf("Dest MACAddr   = %s\n", wsmUnicast);
    }
    else {
        printf("Dest MACAddr   = FFFFFFFFFFFF\n");
    }

    if (busyCounter) {
        printf("Queue is full for %d ms\n", busyCounter);
    }

    switch (pktCollisionTestMode) {
    case 1:
        printf("PktCollisionTestMode= %d (GPIO Initiator), Tx Offset Delay = %d us\n", pktCollisionTestMode, txOffsetUs);
        break;

    case 2:
        printf("PktCollisionTestMode= %d (GPIO Detector)\n", pktCollisionTestMode);
        break;
    }

    busyCounter = 0;
    Last_WsmSequenceNumber = ntohl(OutWsmMsgPayload.WsmSequenceNumber);

    fflush(stdout);
}

void WsmSendHandler(void)
{
    ////static unsigned long long callcounter = 0;          /*####DEBUG####*/
    rsResultCodeType risRet;
    static uint32_t  sequenceNumber = 1;
    int              i;

    bzero(OutWsmMsg.data, wsmPayloadLen);
    OutWsmMsg.dataLength = wsmPayloadLen;
    OutWsmMsgPayload.WsmSequenceNumber = htonl(sequenceNumber);
    strncpy((char *)&OutWsmMsgPayload.IPCharAddr[0], (char *)&IPCharAddr[0], sizeof(IPCharAddr));
    OutWsmMsgPayload.IPCharAddr[19] = '\0';
    ////printf("WsmSendHandler: wsmPayloadLen=%d\n",  wsmPayloadLen);

    if (busy && wsuRisIsRadioTxQFull(wsmRadioType, wsmRadio)) {
        printf("----------->TXQ is FULL\n");
        busyCounter ++;
        return;
    }

    sequenceNumber++;
    busy = FALSE;
    WsmGetTime();

    if ((size_t)wsmPayloadLen >= sizeof (OutWsmMsgPayload) + 3) {
        bcopy(&OutWsmMsgPayload, &OutWsmMsg.data, sizeof(OutWsmMsgData));
        OutWsmMsg.data[wsmPayloadLen-3] = 'W';
        OutWsmMsg.data[wsmPayloadLen-2] = 'S';
        OutWsmMsg.data[wsmPayloadLen-1] = 'M';
    }
    else {
        bcopy(&OutWsmMsgPayload, &OutWsmMsg.data, wsmPayloadLen);
    }

    /* If hard code data option is used, overwrite the entire payload with incremented data.
       This would be used for debugging or interop testing with other devices.  */
    if (hardCodeData) {
        for (i = 0; i < wsmPayloadLen; i++) {
            OutWsmMsg.data[i] = i % 256;
        }
    }

#if defined(HEAVY_DUTY)
    if(override_payload) {
        memcpy(OutWsmMsg.data,my_wsm_data,my_wsm_len);
        OutWsmMsg.dataLength = my_wsm_len;
    }
#endif

    ////printf("DEBUG: OutWsmMsg.dataLength=%d\n",OutWsmMsg.dataLength);

    if (OutWsmMsg.data) {
        if ((risRet = wsuRisSendWSMReq(&OutWsmMsg)) != RS_SUCCESS) {
            printf("wsuRisSendWSMReq failed (%s)\n", rsResultCodeType2Str(risRet));
            return;
        }

        ////printf("WsmSendHander: Calling  WsmSendHandler() [%llu]\n", callcounter++); /*####DEBUG####*/
        ////printf("[%llu]", callcounter++);             /*####DEBUG####*/

        if (wsuRisIsRadioTxQFull(wsmRadioType, wsmRadio)) {
            printf("----------->TXQ is FULL [BUSY]\n");
            busy = TRUE;
            busyCounter ++;
        }
    }
}

#if 0
/* This function will not return until the exact number of microseconds have elapsed.
   The CPU will be constantly used until this occurs */
static void precise_usleep(uint16_t txOffsetUs)
{
    struct timeval currTime;
    struct timeval nextTxTime;

    /* Get current time */
    gettimeofday(&currTime, NULL);

    /* Determine what time it will be in "txOffsetUs" from now */
    nextTxTime.tv_sec = currTime.tv_sec;
    nextTxTime.tv_usec = (currTime.tv_usec + txOffsetUs) % 1000000;

    if ((currTime.tv_usec + txOffsetUs) >= 1000000) {
        nextTxTime.tv_sec += 1;
    }

    /* Wait until current time reaches the desired time */
    while (1) {
        gettimeofday(&currTime, NULL);

        if (currTime.tv_sec > nextTxTime.tv_sec) {
            break;
        } else if (currTime.tv_sec == nextTxTime.tv_sec) {
            if (currTime.tv_usec >= nextTxTime.tv_usec) {
                break;
            }
        }
    }
}

/* This function will attempt to transmit with extreme accuracy, based on the value of
   "txOffsetUs" and "wsmDelay.  The value of wsmDelay must be less than 1000 (1 second)
   and must be evenly divisible into 1000.  This function will poll for the current time
   and transmit one message when the time is SS:MS.txOffsetUs,
      where MS is a multiple of (1000 / wsmDelay)
   So for example, if wsmDelay = 5 (ms) and txOffsetus = 100 (us), then we should see
   transmissions at the following times (where SS is the seconds and the value to the
   right of the decimal point is expressed in microseconds).
   SS.000100
   SS.005100
   SS.010100
   SS.015100
   ...

   The CPU loading is expected to be high when running in this mode.  This is ok for
   this particular test mode.
*/
void wsmSendHandlerMicroSecondAccuracy()
{
    struct timeval currTime;
    struct timeval nextTxTime;
    static bool_t  firstTime = TRUE;
    bool_t         txNow     = FALSE;

    /* Do one-time initialization */
    if (firstTime) {
        if ((1000 % wsmDelay) != 0) {
            printf("Tx delay must be less than 1 second (1000 ms) and evenly divisible into 1000 (wsmDelay=%d)\n", wsmDelay);
            exit(-1);;
        }

        firstTime = FALSE;
        gettimeofday(&currTime, NULL);
        nextTxTime.tv_sec = currTime.tv_sec + 1;
        nextTxTime.tv_usec = txOffsetUs;
        //printf("CurrTime = %lu.%06lu, ", currTime.tv_sec, currTime.tv_usec);
        //printf("NextTxTime = %lu.%06lu\n", nextTxTime.tv_sec, nextTxTime.tv_usec);
    }

    /* Get current time and check if it has reached our next transmit time */
    gettimeofday(&currTime, NULL);

    if (currTime.tv_sec > nextTxTime.tv_sec) {
        txNow = TRUE;
    } else if (currTime.tv_sec == nextTxTime.tv_sec) {
        if (currTime.tv_usec >= nextTxTime.tv_usec)
            txNow = TRUE;
    }

    /* Transmit now if we have reached the desired time and update next desired transmit time */
    if (txNow) {
        WsmSendHandler();

        if (nextTxTime.tv_usec + (wsmDelay * 1000) > 1000000) {
            nextTxTime.tv_sec += 1;
        }

        nextTxTime.tv_usec = (nextTxTime.tv_usec + (wsmDelay * 1000)) % 1000000;
        //printf("CurrTime = %lu.%06lu, ", currTime.tv_sec, currTime.tv_usec);
        //printf("NextTxTime = %lu.%06lu\n", nextTxTime.tv_sec, nextTxTime.tv_usec);
    }
}
#endif

static void service_available(serviceInfoType *serviceInfo)
{
    rsResultCodeType      risRet;
    UserServiceChangeType cfg;
    static bool_t         lastService = TRUE;

    printf("svcAvailable =%s, chAvailable=%s mac=%2x %2x %2x %2x %2x %2x \n",
           (serviceInfo->serviceAvail == 0) ? ("SERVICE_AVAILABLE") : ("SERVICE_NOT_AVAILABLE"),
           (serviceInfo->channelAvail == 0) ? ("SCH_ASSIGNED") : ("SCH_NOT_ASSIGNED"),
           serviceInfo->sourceMac[0], serviceInfo->sourceMac[1], serviceInfo->sourceMac[2],
           serviceInfo->sourceMac[3], serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);

    if (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) {
        lastService = FALSE;
    }

    if ((serviceInfo->serviceAvail == RS_SERVICE_AVAILABLE) && !lastService) {
        lastService = TRUE;
        wBSSJoined  = FALSE;
    }

    /* Accept Service */
    if (!wBSSJoined) {
        cfg.psid                       = uService.psid;
        cfg.command                    = CHANGE_ACCESS;
        cfg.user.access.channelNumber  = serviceInfo->channelNumber;
        cfg.user.access.adaptable      = serviceInfo->adaptable;
        cfg.user.access.dataRate       = serviceInfo->dataRate;
        cfg.user.access.txPwrLevel     = serviceInfo->txPwrLevel;
        cfg.user.access.userAccess     = AUTO_ACCESS_UNCONDITIONAL;
        cfg.user.access.extendedAccess = (serviceInfo->channelAccess == 0) ? 65535 : 0;

        if ((risRet = wsuRisChangeUserServiceRequest(&cfg)) != RS_SUCCESS) {
            printf("wsuRisChangeUserServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return;
        }

        printf("Change User Registered Service (PSID=0x%8x)\n", uService.psid);
        printf("psid=0x%8x\n", ntohl(serviceInfo->psid));
        printf("mac=%2x %2x %2x %2x %2x %2x\n", serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
               serviceInfo->sourceMac[2], serviceInfo->sourceMac[3], serviceInfo->sourceMac[4],
               serviceInfo->sourceMac[5]);
        printf("rss=%2x, servicemask=%8x, pscL=%d, channelmask=%8x, operatingClass=%d,"
               " psc=%s ch#=%d adat=%d dataR=%d \n",
               serviceInfo->rss, serviceInfo->serviceBitmask, serviceInfo->servicePscLength,
               serviceInfo->channelBitmask, serviceInfo->operatingClass,
               serviceInfo->psc, serviceInfo->channelNumber,
               serviceInfo->adaptable, serviceInfo->dataRate);
        wBSSJoined = TRUE;
    }
}

static void receive_wsm(inWSMType *Wsm)
{
    /* Wsm Received */
# if 0
    if (Wsm->ChannelNumber == CCH_NUM_DEFAULT) {
    printf("Received CCH Wsm - PSID:%d Version:%d Security:%d ChnlNum:%d Rate:%d PwrLvl:%d DataLen:%d\n",
           Wsm->PSID, Wsm->WSMVersion, Wsm->Security, Wsm->ChannelNumber,
           Wsm->DataRate, Wsm->TxPwrLevel, Wsm->DataLength);
    } else {
    printf("Received SCH Wsm - PSID:%d Version:%d Security:%d ChnlNum:%d Rate:%d PwrLvl:%d DataLen:%d\n",
           Wsm->PSID, Wsm->WSMVersion, Wsm->Security, Wsm->ChannelNumber,
           Wsm->DataRate, Wsm->TxPwrLevel, Wsm->DataLength);
    }
#endif
}

/* Call back function for TPS errors */
static void tpsErrorCallback(tpsResultCodeType code)
{
    char       errBuf[128];
    static int FirstError = TRUE;

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
        FirstError = FALSE;
    }
}

/* Call back function for Registering to get TPS data */
static void tpsCallback(tpsDataType *data)
{
    if (data->valid) {
        OutWsmMsgPayload.latitude  = htonl((int32_t)(data->latitude * 1E7));
        OutWsmMsgPayload.longitude = htonl((int32_t)(data->longitude * 1E7));
        OutWsmMsgPayload.altitude  = htons((int16_t)(data->altitude * 10.0));
        OutWsmMsgPayload.tpsError  = 0;
    }
    else {
        OutWsmMsgPayload.latitude  = 0;
        OutWsmMsgPayload.longitude = 0;
        OutWsmMsgPayload.altitude  = 0;
        OutWsmMsgPayload.tpsError  = 1;
    }
}

/*
 * Function callback to send sign results to the application.
 *
 * Input  parameters:  smiSignResults_t - The signing results.
 * Output parameters:  None
 * Return Value: Returns 0 on success, -1 on failure. Generally always returns 0.
 */
static int smi_sign_results(smiSignResults_t *results)
{
    fprintf(stderr, "%s: psid               = 0x%x\n", __func__, results->psid);
    fprintf(stderr, "%s: signErrorCode      = %u\n", __func__, results->signErrorCode);
    fprintf(stderr, "%s: wsmLenWithSecurity = %u\n", __func__, results->wsmLenWithSecurity);
    return 0;
}

static int securityInitialize(void)
{
    rsSecurityInitializeType init;
    int32_t                  retval;

    printf("Starting SMI Init.\n");
    init.debug                             = smiDebug;
    init.vodEnabled                        = FALSE;
    init.smi_cert_change_callback          = NULL;
    init.vtp_milliseconds_per_verification = 1000;
    init.smi_sign_results_options          = signRes ? smi_results_all :
                                                       smi_results_none;
    init.smi_sign_results_callback         = signRes ? smi_sign_results : NULL;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_none;
    init.smi_verify_wsa_results_callback   = NULL;
    strcpy(init.lcmName, "testLcm"); // from denso.wsc
    init.numSignResultsPSIDs               = 1;
    init.signResultsPSIDs[0]               = uService.psid;
    retval = smiInitialize(&init);

    if (retval != 0) {
        printf("Error initializing security %d\n", retval);
        return EXIT_FAILURE;
    }

    printf("Security initialized.\n");
    return EXIT_SUCCESS;
}

int WsmParseArg(int argc, char *argv[])
{
    int           k, l, m;
    int           value[2] = {0, 0};
    int           b[6];
    #define       TEMPBUF1_LEN 30
    char          tempBuf1[TEMPBUF1_LEN];
    char          tempBuf2[5];
    float         ftemp;
    int           i = 1;
    int           jj;
    uint16_t      index;
    uint8_t       kk;
    unsigned long temp = 0;

    const char *pCommandArray[] = {
        "TPRI",          // 0
        "SPRI",          // 1
        "CCHPRI",        // 2 not supported
        "DRD",           // 3
        "PWR",           // 4
        "PSID",          // 5
        "DELAY",         // 6
        "CHINTV",        // 7
        "PAYLOAD",       // 8
        "PROV",          // 9
        "PSC",           // 10
        "TXSYNC",        // 11
        "RND",           // 12
        "RAD",           // 13
        "TXCHAN",        // 14
        "SCH",           // 15
        "DISP",          // 16
        "TA",            // 17
        "WSA",           // 18 not supported
        "COMBINEDRADIO", // 19
        "EMULATEDVEH",   // 20
        "SECEN",         // 21
        "SIGNRES",       // 22
        "EXT",           // 23
        "ACCESS",        // 24
        "POWEROPT",      // 25
        "CHNLOPT",       // 26
        "DROPT",         // 27
        "HARDCODEDATA",  // 28
        "TXANT",         // 29
        "WSASEC",        // 30
        "TXMICROOFFSET", // 31
        "PKTCOLLISION",  // 32
        "SMIDEBUG",      // 33
        "UNICAST",       // 34
        "RTYPE",         // 35
        "ISBITMAPPEDSSP", // 36
        "SSPLEN",         // 37
        "SSPMASK",        // 38
        "SSP",            // 39
#if defined(HEAVY_DUTY)
        "INFILE",        // 40
        "NOESC",         // 41
#endif
    };

    /* Have MAX_CMD_BUF be the sze of the longest entry in pCommandArray */
    #define MAX_CMD_BUF sizeof("COMBINEDRADIO")
    #define MAX_CMDS    (sizeof(pCommandArray) / sizeof(pCommandArray[0]))
    char tempBuf[MAX_CMD_BUF];

#if defined(HEAVY_DUTY)
    int           n;
    char          my_file_loc_name[1024];
    FILE          * my_input_file = NULL;
    uint8_t       dummy   = 0;
    char          my_payload[MAX_WSM_DATA * 2];
    int           my_payload_len = 0;
#endif

    sprintf(wsmUnicast, "%02X%02X%02X%02X%02X%02X", 255, 255, 255, 255, 255, 255);

    while (i < argc) {
        /* Be sure parameter begins with "--" */
        if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
            return 0;
        }

        bzero(tempBuf, MAX_CMD_BUF);

        /* Get the parameter name, converted to all upper case, in tempBuf */
        for (kk = 0; (kk < MAX_CMD_BUF) && (kk < strlen(&argv[i][2])); kk ++) {
            tempBuf[kk] = (char)toupper((int)(argv[i][kk+2]));
        }

        tempBuf[kk] = '\0';

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
        case 4: // 4 = index of "PWR" command
            // Convert to a float
            if (sscanf(argv[i], "%f", &ftemp) != 1) {
                fprintf(stderr, "Invalid value for Power\n");
                return 0;
            }
            else {
                printf("Power=%4.1f\n", ftemp);
            }
            
            break;

        case 34: // 34 = index of "UNICAST" command
            snprintf(tempBuf1, TEMPBUF1_LEN, "%s", argv[i]);
            break;

        case 38: // 38 = index of "SSPMASK" command
        case 39: // 39 = indes of "SSP" command
            snprintf(tempBuf1, TEMPBUF1_LEN, "%s", argv[i]);
            k = strlen(tempBuf1);
            /* Make the length even */
            k /= 2;
            k *= 2;
            tempBuf1[k] = '\0';
            break;

        case 35: // 35 = index of "RTYPE" command
            if ((argv[i][0] == 'C') || (argv[i][0] == 'c')) {
                temp = 0; // Indicates radio type C-V2X
                break;
            }

            if ((argv[i][0] == 'D') || (argv[i][0] == 'd')) {
                temp = 1; // Indicates radio type DSRC
                break;
            }
#if defined(HEAVY_DUTY)
            fprintf(stderr, "Invalid RTYPE.\n");
            return 0;
            break;

        case 40: //INFILE
            override_payload = 0;
            my_payload_len = 0;
            my_wsm_len = 0;
            memset(my_payload,'\0',sizeof(my_payload));
            memset(my_wsm_data,0x0 ,sizeof(my_wsm_data));
            memset(my_file_loc_name,'\0',sizeof(my_file_loc_name));

            if(snprintf(my_file_loc_name, 1024, "%s", argv[i])) {
 
                /* min case is "./a" == strlen is 3 chars.
                 * snprintf will always add null so max determinent strlen is 1022 + NULL == 1023.
                 * anything higher we may have not parsed all contents of line so reject.
                 */
                if (strlen(my_file_loc_name) < 3) {
                    fprintf(stderr, "INFILE: Length Error: Too short: strlen=%lu.\n",strlen(my_file_loc_name));
                    return 0;
                }
                if (1023 <= strlen(my_file_loc_name)) {
                    fprintf(stderr, "INFILE: Length Error: Too long: strlen=%lu.\n",strlen(my_file_loc_name));
                    return 0;
                }
            } else {
                fprintf(stderr, "INFILE: Internal Error: I was unable to parse switch, sorry.\n");
                return 0;
            }

            if (NULL == (my_input_file = fopen(my_file_loc_name,"r"))) {
                fprintf(stderr, "INFILE: File open error on (%s)\n",my_file_loc_name);
                return 0;
            }
             
            //printf("\nfile=%s[",my_file_loc_name); 
            for(my_payload_len=0; my_payload_len < (MAX_WSM_DATA * 2) ;my_payload_len++) { 
 
                if(EOF == (my_payload[my_payload_len] = fgetc(my_input_file))){
                    break;
                }
                //printf("%c,",my_payload[my_payload_len]);

                if(   ( '\n' == my_payload[my_payload_len]) 
                   || ( '\0' == my_payload[my_payload_len])) 
                    break;

                if(feof(my_input_file))
                    break;
            }

            //printf("] payload_len=%d.\n",my_payload_len); 

            if(NULL != my_input_file) {
                fclose(my_input_file);
            }

            if(my_payload_len == MAX_WSM_DATA * 2) {
                fprintf(stderr, "INFILE: error string is too long: %d char max\n",MAX_WSM_DATA*2);
                return 0;
            }

            if(my_payload_len < MIN_WSM_DATA) {
                fprintf(stderr, "INFILE: error string is too short: %d chars min\n",MIN_WSM_DATA);
                return 0;
            }
            if(1 == (my_payload_len % 2)) {
                fprintf(stderr, "INFILE: error string is odd lenth=%d chars min\n",my_payload_len);
                return 0;
            }

            /* Now pack ascii data. */
            //printf("\nhex data[");

            for(n=0,my_wsm_len = 0;  (n < my_payload_len); n++) {

                dummy = i2v_ascii_2_hex_nibble(my_payload[n]);

                if(0xff == dummy) {

                    fprintf(stderr, "INFILE: error file contains non-hex data.\n");
                    return 0;
                  
                } else {

                    if(0 == (n % 2)) {
                        my_wsm_data[my_wsm_len] = (0xf) & dummy;
                    } else {
                        my_wsm_data[my_wsm_len]  =  my_wsm_data[my_wsm_len] << 4; 
                        my_wsm_data[my_wsm_len] |=  (0xf) & dummy;
                        //printf("0x%02X,",my_wsm_data[my_wsm_len]);
                        my_wsm_len++;
                    }
                }
            }
            //printf("] wsm_len=%d.",my_wsm_len);
            wsmPayloadLen = OutWsmMsg.dataLength = my_wsm_len;
            override_payload = 1;
            
            break;

        case 41: //NOESC
            output_esc_codes     = FALSE;
            break;

#endif /* HEAVY_DUTY */
        default:
            temp = strtoul(argv[i], NULL, 0);
            break;
        }

        /* Process the parameter */
        switch (index) {
        case 0:     // TPRI
            if (temp > 7) {
                return 0;
            }

            OutWsmMsg.txPriority = (uint8_t)temp;
            break;

        case 1:     // SPRI
            if (temp > 63) {
                return 0;
            }

            uService.servicePriority =
            pService.servicePriority = temp;    // Service Priority (0..63)
            break;

        case 2:     // CCHPRI   - not supported
            printf("--cchpri option not supported\n");
            return 0;
            break;

        case 3:     // DRD
            if ((temp !=  3) && (temp !=  4) && (temp !=  6) && (temp !=  9) &&
                (temp != 12) && (temp != 18) && (temp != 24) && (temp != 27)) {
                return 0;
            }

            adjustDataRate = TRUE;
            OutWsmMsg.dataRate = (dataRateType)temp;
            break;

        case 4:     // PWR
            // Moved checking for max power later (dependent on card in device)
            // The Power is input as a floating point value, but Radio Stack will
            // truncate power value to the nearest 0.5dB.  (e.g. 15.8dB will become 15.5dB)
            OutWsmMsg.txPwrLevel = ftemp;
            break;

        case 5:     // PSID
            OutWsmMsg.psid =
            uService.psid  =
            pService.psid  =  temp;
            break;

        case 6:     // DELAY
            wsmDelay = temp;
            break;

        case 7:     // CHINTV
            if (temp > 2) {
                return 0;
            }

            OutWsmMsg.channelInterval = temp;
            printf("Channle intv=%d \n", OutWsmMsg.channelInterval);
            break;

        case 8:     // PAYLOAD
            if (temp > MAX_WSM_DATA) {
                return 0;
            }

            if (temp < sizeof(OutWsmMsgData)) {
                printf("Warning - Payload length is less than recommended length\n");
                sleep(1);
            }

            wsmPayloadLen = OutWsmMsg.dataLength = temp;
            break;

        case 9:     // PROV
            if (temp > 1) {
                return 0;
            }

            wsmProvider = (temp != 0) ? TRUE: FALSE;
            break;

        case 10:    // PSC
            if (temp >= 10) {
                return 0;
            }

            pService.lengthPsc = 1;
            pService.psc[0] = temp + '0';
            break;

        case 11:    // TXSYNC
            printf("--txsync not currently supported\n");
            return 0;

        case 12:    // RND
            if (temp > 1000) {
                return 0;
            }

            randDelay = temp;
            break;

        case 13:    // RAD
            if (temp > 1) {
                return 0;
            }

            OutWsmMsg.radioNum  =
            pService.radioNum   =
            uService.radioNum   =
            wsmRadio            = temp;
            break;

        case 14:    // TXCHAN
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
            OutWsmMsg.channelNumber =
            uService.channelNumber  =
            wsmRadio0Channel        =
            wsmRadio1Channel        = temp;
            break;

        case 15:    // SCH
            if ((temp != 76) && (temp != 84) && (temp != 88) &&
                (temp != 156) && (temp != 164) && (temp != 168)) {
                if (temp < 172  || temp > 184) {
                    return 0;
                }
            }

            /* Set temporary variable now and after all input has been processed, set
               uService.channelNumber to this value.  If this option doesn't get used,
               then uService.channelNumber will be set based on CHAN option.  */
            userServiceChan        =
            pService.channelNumber = temp;
            break;

        case 16:    // DISP
            if (temp > 20) {
                return 0;
            }

            wsmDisplay = temp;
            break;

        case 17:    // TA
            printf("--ta not currently supported\n");
            return 0;

        case 18:    // WSA    - Not supported
            printf("--wsa not currently supported\n");
            return 0;

        case 19:    // COMBINEDRADIO
            if ((temp == 0) || (temp > 60)) {
                return 0;
            }

            actualRadios = temp;
            break;

        case 20:    // EMULATEDVEH
            if ((temp == 0) || (temp > 400)) {
                return 0;
            }

            emulatedVehicles = temp;
            break;

        case 21:    // SECEN
            if (temp > 1) {
                return 0;
            }

            secEnabled = (temp != 0) ? TRUE: FALSE;
            break;

        case 22:    // SIGNRES
            if (temp > 1) {
                return 0;
            }

            signRes = (temp != 0) ? TRUE: FALSE;
            break;

        case 23:    // EXT
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

        case 24:    // ACCESS
            if (temp > 2) {
                return 0;
            }
            uService.userAccess = temp;
            break;

        case 25:    // POWEROPT
            if (temp >= 1) {
                OutWsmMsg.wsmpHeaderExt |= WAVE_ID_TRANSMIT_POWER_BITMASK;
            }
            else {
                OutWsmMsg.wsmpHeaderExt &= (~WAVE_ID_TRANSMIT_POWER_BITMASK);
            }

            break;

        case 26:    // CHNLOPT
            if (temp >= 1) {
                OutWsmMsg.wsmpHeaderExt |= WAVE_ID_CHANNEL_NUMBER_BITMASK;
            }
            else {
                OutWsmMsg.wsmpHeaderExt &= (~WAVE_ID_CHANNEL_NUMBER_BITMASK);
            }

            break;

        case 27:    // DROPT
            if (temp >= 1) {
                OutWsmMsg.wsmpHeaderExt |= WAVE_ID_DATARATE_BITMASK;
            }
            else {
                OutWsmMsg.wsmpHeaderExt &= (~WAVE_ID_DATARATE_BITMASK);
            }

            break;

        case 28:    // HARDCODEDATA
            if (temp > 1) {
                return 0;
            }

            hardCodeData = (temp != 0) ? TRUE: FALSE;
            break;

        case 29:    // TXANT
            if (temp > 2) { // 0: auto, 1, 2
                return 0;
            }

            wsmTxAnt = temp;
            break;

        case 30:    // WSASEC
            if (temp > 1) {
                return 0;
            }

            pService.wsaSecurity = uService.wsaSecurity = temp;
            break;

        case 31:    // TXMICROOFFSET
            if (temp == 0) {
            }
             else if (temp <= 5000) {
                txOffsetUs = temp;
            }
            else {
                return 0;
            }

            break;

        case 32:    // PKTCOLLISION
            if (temp > 2)
                return 0;
            pktCollisionTestMode = temp;
            break;

        case 33:    // SMIDEBUG
            if (temp > 1) {
                return 0;
            }

            smiDebug = (temp != 0) ? TRUE: FALSE;
            break;

        case 34:    // UNICAST
            l = m = 0;

            for (k = 0; k < 17 && l < 6; k++) {
                if (((tempBuf1[k] >= '0' && tempBuf1[k] <= '9')) ||
                     (tempBuf1[k] == ':') ||
                    ((tempBuf1[k] <= 'F')  && (tempBuf1[k] >= 'A')) ||
                    ((tempBuf1[k] <= 'f')  && (tempBuf1[k] >= 'a'))) {
                    if (((tempBuf1[k] <= 'F')  && (tempBuf1[k] >= 'A'))) {
                        tempBuf1[k] = tempBuf1[k] + 'a' -'A';
                    }

                    if (((tempBuf1[k] <= 'f')  && (tempBuf1[k] >= 'a'))) {
                        value[m] = tempBuf1[k] - 'a' + 10;
                    }
                    else if (tempBuf1[k] != ':') {
                        value[m] = tempBuf1[k] - '0';
                    }

                    if (tempBuf1[k] == ':') {
                       if (! m || m > 2) { // syntax is :: or 234:23
                           return 0;
                       }

                       if (m == 1) { // only 1 digit like 5:23:3f:....
                           b[l++] = value[0];
                       }
                       else {
                           b[l++] = value[0] * 16 + value[1];
                       }

                       m = 0;
                       //  printf("value %d, %d %d", value[0], value[1], b[l-1]);
                       value[1] = value[0] = 0;
                    }
                    else {
                        m++;
                    }
                }
                else {
                    printf("Format of Unicast should be: 00:11:22:33:44:55\n");
                    return 0;
                }
            }

            if (m == 1) {// only 1 digit like 5:23:3f:....
                b[5] = value[0];
            }
            else {
                b[5] = value[0] * 16 + value[1];
            }

            //  printf("PAO %d %d %d %d %d %d\n", b[0], b[1], b[2], b[3], b[4], b[5]);
            sprintf(wsmUnicast, "%02x%02x%02x%02x%02x%02x", b[0], b[1], b[2], b[3], b[4], b[5]);

            for (jj=0; jj < 6; jj++) {
                wsmUnicast2[jj] = (char)b[jj];
            }

            wsmUni = TRUE;
            //printf("GOOD %s\n", wsmUnicast);
            break;

        case 35:    // RTYPE
            if (temp > 1) {
                return 0;
            }

            OutWsmMsg.radioType =
            pService.radioType  =
            uService.radioType  =
            wsmRadioType        = (temp == 0) ? RT_CV2X : RT_DSRC;
            break;

        case 36:    // ISBITMAPPEDSSP
            if (temp > 1) {
                return 0;
            }

            OutWsmMsg.isBitmappedSsp = (temp != 0) ? TRUE: FALSE;
            break;

        case 37:    // SSPLEN
            if (temp > MAX_SSP_DATA) {
                return 0;
            }

            OutWsmMsg.sspLen = (uint8_t)temp;
            break;

        case 38: // 38 = index of "SSPMASK" command
        case 39: // 39 = indes of "SSP" command
            k = strlen(tempBuf1) / 2;
            
            for (l = 0; l < k; l++) {
                tempBuf2[0] = '0';
                tempBuf2[1] = 'x';
                tempBuf2[2] = tempBuf1[l * 2];
                tempBuf2[3] = tempBuf1[l * 2 + 1];
                tempBuf2[4] = '\0';
                
                if (index == 38) {
                    OutWsmMsg.sspMask[l] = strtoul(tempBuf2, NULL, 0);
                }
                else {
                    OutWsmMsg.ssp[l] = strtoul(tempBuf2, NULL, 0);
                }
            }

            break;

#if defined(HEAVY_DUTY)
        case 40:  //INFILE
            wsmPayloadLen = OutWsmMsg.dataLength = my_wsm_len;
            override_payload = 1;
            break;

        case 41:  //NOESC
            break;
#endif
        default:
            return 0;
            break;
        } // switch (index)

        /* Move past the value to the next parameter, if any */
        i++;
    } // while (i < argc)

    /* After all input has been processed, set uServicechannelNumber to entered value */
    if (userServiceChan != 0)  {
        uService.channelNumber = userServiceChan;
    }

    return 1;
}

void WsmShowSyntax(int argc, char *argv[])
{
    printf("Usage: %s [--access (0-2)][--txchan channel][--chintv (0-2)][--chnlopt (0-1)][--delay (MSEC)]"
           "[--disp (0-20)][--drd (3|4|6|9|12|18|24|27)][--dropt (0-1)][--ext (0-65535)][--hardcodedata (0-1)]"
           "[--payload (56 - %d)][--poweropt (0-1)][--psid (1000)][--pwr (0.5-23.0)][--psc (1-9)][--rtype (cv2x/dsrc)]"
           "[--rad (0-1)][--rnd (0-1000)][--safetylen (0-16)][--safetydata 0-65535][--secen (0-1)][--signres (0-1)]"
           "[--spri(0-63)][--ta (0-1)][--tpri (0-7)][--txant (0-2)][--txsync (0-1)][--sch channel][--wsasec (0-1)]"
           "[--txmicrooffset (0-5000)][--pktcollision (0-2)][--qsize (1-128)][--smidebug (0-1)]"
           "[--unicast xx:xx:xx:xx:xx:xx][--isBitmappedSSP (0-1)][--SSPLen (0-12)][--SSPMask mask][--SSP SSP]\n"
#if defined(HEAVY_DUTY)
           "[--noesc argIgnored]\n"
           "[--infile <location><name> i.e. /myfile.txt (%d to %d)]\n", argv[0], MAX_WSM_DATA,MIN_WSM_DATA,MAX_WSM_DATA * 2);
#else
           , argv[0], MAX_WSM_DATA);
#endif
}

int32_t getIPv4Address(char *ifName, uint8_t *IPCharAddr)
{
    char  loopback_addr[] = "127.000.000.001";

    strcpy((char *)IPCharAddr, loopback_addr);
    return 0;
}

int main (int argc, char *argv[])
{
    rsResultCodeType  risRet;
    tpsResultCodeType tpsRet;
    cfgType           cfg;
    cfgType           getCfg;
    bool_t            cv2x_enable = FALSE;
    bool_t            dsrc_enable = FALSE;
    struct sigaction  sa;
    struct sigevent   evnt;
    struct timeval    currTime;
    unsigned short    temp;
    struct timeval    nextTxTime;
    int               nextTxTimeMs;

    pthread_mutex_init(&txThreadMutex, NULL);
    getIPv4Address("eth0", IPCharAddr);

    /* Initialize random number generator */
    gettimeofday(&currTime, NULL);
    srandom(currTime.tv_usec);

    if (argc > 1) {
        if (argv[1][0] == '?') {
            WsmShowSyntax(argc, argv);
            exit(0);
        }

        OutWsmMsg.dataRate = DR_6_MBPS;

        if (WsmParseArg(argc, argv) == 0) {
            WsmShowSyntax(argc, argv);
            exit(0);
        }

        if (adjustDataRate) {
            if ((wsmRadioType == RT_DSRC && wsmRadio == 0 && wsmRadio0Channel != 175 && wsmRadio0Channel != 181) ||
                (wsmRadioType == RT_DSRC && wsmRadio == 1 && wsmRadio1Channel != 175 && wsmRadio1Channel != 181)) {
                temp = OutWsmMsg.dataRate;

                if (temp == 3  || temp == 4  || temp == 6  || temp == 9 ||
                    temp == 12 || temp == 18 || temp == 24 || temp == 27) {
                    if (temp == 4) {
                        temp = 9;
                    }
                    else {
                        temp *= 2;
                    }

                    OutWsmMsg.dataRate = (dataRateType)temp;
                }
                else {
                    WsmShowSyntax(argc, argv);
                    exit(0);
                }
            }
            else if ((wsmRadioType == RT_DSRC && wsmRadio == 0 && (wsmRadio0Channel == 175 || wsmRadio0Channel == 181)) ||
                     (wsmRadioType == RT_DSRC && wsmRadio == 1 && (wsmRadio1Channel == 175 || wsmRadio1Channel == 181))) {
               temp = OutWsmMsg.dataRate;

               if (temp == 6  || temp == 9  || temp == 12 || temp == 18 ||
                   temp == 24 || temp == 36 || temp == 48 || temp == 54) {
                   OutWsmMsg.dataRate = (dataRateType)temp;
               }
               else {
                   WsmShowSyntax(argc, argv);
                   exit(0);
               }
            }
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
        OutWsmMsg.radioType =
        pService.radioType  =
        uService.radioType  =
        wsmRadioType        = RT_CV2X;
    }
    else if (!cv2x_enable && dsrc_enable) {
        OutWsmMsg.radioType =
        pService.radioType  =
        uService.radioType  =
        wsmRadioType        = RT_DSRC;
    }

    if (uService.extendedAccess == 65535) {
        OutWsmMsg.channelNumber = uService.channelNumber;
    }

    printf("Running %s\n", argv[0]);

    /* This is used for congestion testing and is derived from wsmDelay */
    if (wsmDelay != 0) {
        emulatedTxRateHz = 1000 / wsmDelay; // will normally be 5 or 10 (Hz)
    }
    else {
        emulatedTxRateHz = 1000;
    }

    /* Open GPIO if doing packet collision testing */
    if (pktCollisionTestMode != 0) {
        printf("Packet Collision Testing NOT IMPLEMENTED!\n");
        /* Increase priority of process */
        nice(-10);
    }

    /* Normally, we will transmit via this timer.  If the pktCollisionTestMode==2
       then Tx will be done by constantly polling a GPIO instead of using
       a timer */
    if (pktCollisionTestMode != 2) {
        /* Create transmit timer */
        memset(&evnt, 0, sizeof(evnt));
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = txTmrHandler;
        sa.sa_flags   = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR2, &sa, NULL);
        evnt.sigev_signo = SIGUSR2;

        if (timer_create(CLOCK_REALTIME, &evnt, &txTmrId) == -1) {
           printf("main: Error: Tx timer_create failed\n");
           exit(1);
        }
    }

    /* Start - Init Radio Communication */
    if ((risRet = wsuRisInitRadioComm("wsm_send", &appCredential)) != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return EXIT_FAILURE;
    }

    printf("RadioComm Initialized\n");

    /* Check for max power limit (based on what card is in system) */
    if (OutWsmMsg.radioType == RT_DSRC) {
        getCfg.radioType = RT_DSRC;
        risRet = wsuRisGetCfgReq((OutWsmMsg.radioNum == 0) ?
                                 CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0 :
                                 CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1,
                                 &getCfg);

        if (risRet == RS_ENOSUPPORT) {
            printf("Unable to determing hardware power limit. Continuing (with TxPwr=%4.1f)...\n", OutWsmMsg.txPwrLevel);
        }
        else {
            if (OutWsmMsg.txPwrLevel > getCfg.u.MaxTxPowerLevel) {
                printf("Requested Tx Power (%4.1f) exceeds Max Tx power (%d)\n", OutWsmMsg.txPwrLevel, getCfg.u.MaxTxPowerLevel);

                if ((risRet = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                    printf("wsuRisTerminateRadioComm failed (%s) -- ", rsResultCodeType2Str(risRet));
                }

                printf("RadioComm Terminated\n");
                exit(1);
            }
        }
    }

    /* Set the Radio Tx Antenna */
    cfg.u.Antenna = wsmTxAnt;
    printf("Set Antenna config \n");

    if (wsmRadioType == RT_DSRC) {
        if (wsmRadio == 0) {
            cfg.radioType = RT_DSRC;

            if ((risRet = wsuRisSetCfgReq(CFG_ID_ANTENNA_RADIO_0, &cfg)) != RS_SUCCESS) {
                if (risRet == RS_ENOSUPPORT) {
                    printf("wsuRisSetCfgReq UNSUPPORTED: Unable to set CFG_ID_ANTENNA_RADIO_0. Continuing.\n");
                    /* If no support for setting TX Antenna, then continue with existing setting. */
                }
                else {
                    printf("Error: wsuRisSetCfgReq CFG_ID_ANTENNA_RADIO_0 setting failed (%s)\n", rsResultCodeType2Str(risRet));
                    exit(1);
                }
            }

            getCfg.radioType = RT_DSRC;

            if ((risRet = wsuRisGetCfgReq(CFG_ID_ANTENNA_RADIO_0, &getCfg)) != RS_SUCCESS) {
                if (risRet == RS_ENOSUPPORT) {
                    printf("wsuRisGetCfgReq UNSUPPORTED: Unable to get CFG_ID_ANTENNA_RADIO_0. Continuing.\n");
                    printf("-------Continuing  cfg.u.Antenna=%d,  getCfg.u.Antenna=%d--------\n",cfg.u.Antenna,getCfg.u.Antenna);
                    getCfg.u.Antenna = cfg.u.Antenna;   //To allow program to run.
                }
                else {
                    printf("Error: wsuRisGetCfgReq CFG_ID_ANTENNA_RADIO_0 getting failed (%s)\n", rsResultCodeType2Str(risRet));
                    exit(1);
                }
            }

            if (cfg.u.Antenna != getCfg.u.Antenna) {
                printf("Tx Antenna value for radio 0 is not setting/getting properly. \n");
                printf("--------cfg.u.Antenna=%d,  getCfg.u.Antenna=%d--------\n",cfg.u.Antenna,getCfg.u.Antenna);
                exit(1);
            }
            else
                printf("Tx Antenna value for Radio 0 is=%d \n", getCfg.u.Antenna);
        }
        else { /*wsmRadio==1*/
            cfg.radioType = RT_DSRC;

            if ((risRet = wsuRisSetCfgReq(CFG_ID_ANTENNA_RADIO_1, &cfg)) != RS_SUCCESS) {
                if (risRet == RS_ENOSUPPORT) {
                    printf("wsuRisSetCfgReq UNSUPPORTED: Unable to set CFG_ID_ANTENNA_RADIO_1. Continuing.\n");
                    /* If no support for setting TX Antenna, then continue with existing setting. */
                }
                else {
                    printf("Error: wsuRisSetCfgReq CFG_ID_ANTENNA_RADIO_1 setting failed (%s)\n", rsResultCodeType2Str(risRet));
                    exit(1);
                }
            }

            getCfg.radioType = RT_DSRC;

            if ((risRet = wsuRisGetCfgReq(CFG_ID_ANTENNA_RADIO_1, &getCfg)) != RS_SUCCESS) {
                if (risRet == RS_ENOSUPPORT) {
                    printf("wsuRisGetCfgReq UNSUPPORTED: Unable to get CFG_ID_ANTENNA_RADIO_1. Continuing.\n");
                    printf("-------Continuing  cfg.u.Antenna=%d,  getCfg.u.Antenna=%d--------\n",cfg.u.Antenna,getCfg.u.Antenna);
                    getCfg.u.Antenna = cfg.u.Antenna;   //To allow program to run.
                }
                else {
                    printf("Error: wsuRisGetCfgReq CFG_ID_ANTENNA_RADIO_1 getting failed (%s)\n", rsResultCodeType2Str(risRet));
                    exit(1);
                }
            }

            if (cfg.u.Antenna != getCfg.u.Antenna) {
                printf("Tx Antenna value for radio 1 is not setting/getting properly. \n");
                printf("--------cfg.u.Antenna=%d,  getCfg.u.Antenna=%d--------\n",cfg.u.Antenna,getCfg.u.Antenna);
                exit(1);
            }
            else {
                printf("Tx Antenna value for Radio 1 is=%d \n", getCfg.u.Antenna);
            }
        }
    }

    tpsRet = wsuTpsInit();

    if (tpsRet != TPS_SUCCESS) {
        printf("TPS init failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        exit(1);
    }

    /* Call back function for Registering to get TPS data */
    wsuTpsRegisterErrorHandler(tpsErrorCallback);
    OutWsmMsgPayload.tpsError = 1;
    tpsRet = wsuTpsRegister(tpsCallback);

    if (tpsRet != TPS_SUCCESS) {
        printf("TPS register failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        exit(1);
    }

    if (wsmProvider) { // Provider mode
        printf("Set Station Role as Provider\n");

        /* Register Services */
        pService.action = ADD;

        if (pService.edca_count != 0) {
            pService.wsaHeaderExtension |= WSA_EXT_EDCA_PARAM_SET_BITMASK;
        }
        else {
            pService.wsaHeaderExtension &= ~WSA_EXT_EDCA_PARAM_SET_BITMASK;
        }

        if ((risRet = wsuRisProviderServiceRequest(&pService)) != RS_SUCCESS) {
            printf("wsuRisProviderServiceRequest failed (%s)\n", rsResultCodeType2Str(risRet));
            return EXIT_FAILURE;
        }
        else {
            printf("wsuRisProviderServiceRequest (PSID=0x%8x)\n", pService.psid);
        }
    }
    else { // User mode
        printf("Set Station Role as User\n");
        /* Register Services */
        uService.action = ADD;

        if ((risRet = wsuRisUserServiceRequest(&uService)) != RS_SUCCESS) {
            printf("wsuRisRegisterUserReq failed (%s)\n", rsResultCodeType2Str(risRet));
            return EXIT_FAILURE;
        }
        else {
            printf("wsuRisUserServiceRequest (PSID=0x%8x)\n", uService.psid);
        }
    }

    bzero(&OutWsmMsgPayload, sizeof(OutWsmMsgPayload));
    memcpy(&OutWsmMsg.peerMacAddress, wsmUnicast2, 6);

    strncpy((char *)&OutWsmMsgPayload.WsmData[0], "wsmtest.", 8);

    signal(SIGINT, WsmExitHandler);
    signal(SIGTERM, WsmExitHandler);
    signal(SIGKILL, WsmExitHandler);

    if (wsmDisplay) {
        /* Create display timer to execute every second */
        memset(&evnt, 0, sizeof(evnt));
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = WsmDisplayHandler;
        sa.sa_flags = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR1, &sa, NULL);
        evnt.sigev_signo = SIGUSR1;

        if (timer_create(CLOCK_REALTIME, &evnt, &dispTmrId) == -1) {
           printf("Error: Display timer_create failed\n");
           exit(1);
        }

        printf("main: Display Timer update period = %d msec\n",(wsmDisplay*1000));

        dispTmrTv.it_value.tv_sec = wsmDisplay;
        dispTmrTv.it_value.tv_nsec = 0;
        dispTmrTv.it_interval.tv_sec = wsmDisplay;
        dispTmrTv.it_interval.tv_nsec = 0;

        if (timer_settime(dispTmrId, 0, &dispTmrTv, NULL) == -1) {
            printf("Error setting Display Timer\n");
            exit(1);
        }
    }

    /*
     *  Initialize Tx Timer (unless continuous mode or using 1609.4 channel
     *  intervals for synchronized transmission)
     */
    if ((wsmDelay != 0) && (pktCollisionTestMode != 2)) {
        txTmrContinueFlag = TRUE;

        if (emulatedVehicles > 1) {
#if 0
#define TEST_LOOP_CNT 50
            int rc;
            long sum=0;
            for (i=0; i<TEST_LOOP_CNT; i++) {
                rc = getNextTxTime(&nextTxTime);
                sum += rc;
                printf("nextTxTime = %ld.%06ld, delta = %d\n",
                       nextTxTime.tv_sec, nextTxTime.tv_usec, rc);
            }
            printf("Average time = %ld ms\n", sum/TEST_LOOP_CNT);
            exit(1);
#else
            nextTxTimeMs = getNextTxTime(&nextTxTime);
            if (nextTxTimeMs == 0) nextTxTimeMs=1;
#endif
            txTmrTv.it_value.tv_sec = nextTxTimeMs/1000;
            txTmrTv.it_value.tv_nsec = 1000000 * (nextTxTimeMs%1000);
        }
        else {
            txTmrTv.it_value.tv_sec = wsmDelay/1000;
            txTmrTv.it_value.tv_nsec = 1000000 * (wsmDelay%1000);
        }

        if (timer_settime(txTmrId, 0, &txTmrTv, NULL) == -1) {
            printf("main: Error Setting Tx Timer (%d)\n",errno);
            exit(1);
        }
        printf("main:  wsmDelay Tx Timer Update period = %ld sec + %ld nsec\n",
                txTmrTv.it_value.tv_sec, txTmrTv.it_value.tv_nsec);
    }

    if (secEnabled) {
        OutWsmMsg.security     = SECURITY_SIGNED;
        OutWsmMsg.securityFlag = 2; /* Let Aerolink decide whether to sign with
                                     * a digest or a full certificate */
    }

    /*
     * Wait in outer-loop for signal from signal handlers.
     */
    while(mainLoop && callbackError == 0) {
        if (secEnabled && (smiDebug || signRes)) {
            if (securityInitialize() == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
        }

        /*
         * Main loop, stay here forever - Until Ctrl-C is pressed
         */
        do {
            if (pktCollisionTestMode==2) {
                printf("Packet Collision Testing NOT IMPLEMENTED!\n");
                /* Wait for trigger and then send WSM */
            }
            else {
                /* If transmitting continously, do it now (if its valid to do so).
                 * Otherwise, sleep for 100ms before checking again
                 */
                ////printf("DEBUG: mainloop: wsmDelay=%d, wsmCCH=%d, wBSSJoined=%d\n",
                ////                         wsmDelay, wsmCCH, wBSSJoined);

                if (wsmDelay == 0) {
                    if (wsmCCH || wBSSJoined) {
                        WsmSendHandler();
                    }
                }
                else {
                    ///printf("-----usleep(100000) instead of callling WsmSendHandler....\n");
                    ///printf("DEBUG: mainloop: wsmDelay=%d, wsmCCH=%d, wBSSJoined=%d\n",
                    ///                         wsmDelay, wsmCCH, wBSSJoined);
                    usleep(100000);
                }
            }
        } while(mainLoop);

        if (secEnabled && (smiDebug || signRes)) {
            smiTerm();
        }
    }  /*end: outer while(mainLoop)*/

    if (!mainLoop) {
        WsmCleanHandler();
    }

    /*
     * Shutdown timers.
     */
    memset(&txTmrTv, 0, sizeof(txTmrTv));

    if (timer_settime(txTmrId, 0, &txTmrTv, NULL) == -1) {
        printf("Error stoping Tx Timer (%d)\n",errno);
    }

    timer_delete(txTmrId);

    if (wsmDisplay) {
        memset(&dispTmrTv, 0, sizeof(dispTmrTv));

        if (timer_settime(dispTmrId, 0, &dispTmrTv, NULL) == -1) {
            printf("Error stoping Display Timer(%d)\n",errno);
        }

        timer_delete(dispTmrId);
    }

    /* Tell TPS we don't need him any more */
    tpsRet = wsuTpsDeregister();

    if (tpsRet != TPS_SUCCESS) {
        printf("wsuTpsDeregister() failed (%s)\n", tpsResultCodeType2Str(tpsRet));
    }

    wsuTpsTerm();
    return EXIT_SUCCESS;
} /*end:main*/

static void txTmrHandler(int signum)
{
    ////static unsigned long long callcounter=0;          /*####DEBUG####*/
    static unsigned int nextR = 0;
    static unsigned int lastR = 0;
    unsigned int        nextTxTimeMs;
    struct timeval      nextTxTime;

    pthread_mutex_lock(&txThreadMutex);

    /* Ensure we didn't try to exit */
    if (!mainLoop) {
        goto txTmrHandlerExit;
    }

    /* Reset Tx timer for next time if necessary */
    if (txTmrContinueFlag) {
        /* If we are emulating some number of vehicles, transmit the message and then
           obtain the next time that we need to transmit the next message.  Timers less
           than 2 ms don't work, so any times less than that should just be sent immediately.
           Otherwise, set the timer for the next desired transmission. */
        if (emulatedVehicles > 1) {
            do {
                WsmSendHandler();
                nextTxTimeMs = getNextTxTime(&nextTxTime);
            } while (nextTxTimeMs < 2);

            txTmrTv.it_value.tv_sec  = nextTxTimeMs/1000;
            txTmrTv.it_value.tv_nsec = 1000000 * (nextTxTimeMs%1000);

            if (timer_settime(txTmrId, 0, &txTmrTv, NULL) == -1) {
                printf("txTmrHandler: Error Setting Tx Timer\n");
                exit(1);
            }

            goto txTmrHandlerExit;
        }

        /* Transmit 1 packet every "wsmDelay" ms, randomly over the interval
           0..randDelay ms. */
        if (randDelay) {
            nextR = random() %  randDelay;
        }

        nextTxTimeMs = nextR + (wsmDelay - lastR);
        lastR = nextR;
        txTmrTv.it_value.tv_sec  = nextTxTimeMs/1000;
        txTmrTv.it_value.tv_nsec = 1000000 * (nextTxTimeMs%1000);

        if (timer_settime(txTmrId, 0, &txTmrTv, NULL) == -1) {
            printf("txTmrHandler: Error Setting Tx Timer\n");
            exit(1);
        }
    }

    /* Transmit data */
    ////printf("txTmrHandler: Calling  WsmSendHandler() [%llu]\n", callcounter++); /*####DEBUG####*/
    WsmSendHandler();

txTmrHandlerExit:
    pthread_mutex_unlock(&txThreadMutex);
}

