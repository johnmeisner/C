/**************************************************************************
 *                                                                        *
 *     File Name: i2v_util.h                                              *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* TODO: all of the function protos need headers that explain function
   purpose along with parameter description */

#ifndef I2V_UTIL_H
#define I2V_UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
/* required for active message compiled in */
#include <pthread.h>

#include "i2v_types.h"
#include "i2v_general.h"
#include "i2v_shm_master.h"
#include "i2v_cfg.h"
#include "ris_struct.h"
#include "ris.h"
#include "tps_api.h"

/* Call this first at boot once per task using this module. */
void i2vInitStatics(void);

/* For I2V_UTIL_DBG: Limit ouput of printf. No impact on syslog if enabled.*/
#define I2V_PRINTF_MAX  (128)

/* do not change below value randomly */
#define I2V_LOG_NAME_MAX     9

/* cmds to iwmh */
#define WMH_SERVICE_REQUEST_CMD    0xFC
#define WMH_SEND_WSM_CMD           0xCF
#define WMH_RX_CB_REGISTER_CMD     0xCB

/* cmds from iwmh */
#define WMH_IN_WSM                 0x13
#define WMH_VOD_RESP               0xA0

#define MAX_WMH_RESP_SZ            0x10
#define MIN_WMH_RESP_BUF           sizeof(i2vWMHRespMsg)

/* a mininum message size is required; queue can send
   junk; assume pid_t (mtype) + 10 bytes for a J2735 msg
*/
#define MIN_VALID_WMH_MSG_SZ       sizeof(pid_t) + 10

/* this will allow over 10 sec for a svc req
   but the time to make the requests to the radio
   stack varies */
#define WMH_SVC_TIMEOUT            10000

/* Utility macros */
#define crcLOBYTE(x) ((unsigned char)((x) & 0xFF))
#define crcHIBYTE(x) ((unsigned char)((x) >> 8))

#define CONVERT_TO_UTC(a, b, c, d, e) i2vConvertToUTC(a, b, c, d, e)

//
//This will add leap days without translation error
//
#define NUM_YEAR_SECONDS              (31536000) 
#define NUM_DAY_SECONDS               (86400) 

uint32_t i2vConvertToUTC(int32_t tm_year, int32_t tm_yday, int32_t tm_hour, int32_t tm_min, int32_t tm_sec);

/* 20190207: FIXME: these macros have been around for a long time; but they have a glaring defect,
   the macros need to be improved to check if the pointers are NULL */

/* all broadcast apps should use these macros - or duplicate the logic 
   the timeout is the max time the function should wait for a reply from
   iwmh if iwmh is enabled; the optIntStatusPtr is useful only when
   iwmh is enabled */
/* input: ProviderServiceType *svcptr
 *        cfgItemsT *cfgptr (i2vcfg)
 *        int numMsTO (should be the number of milliseconds desired i.e. 5 ms)
 *        int intRetValPtr (will be assigned to I2V_RETURN_xxx or RS_xxx if no iwmh)
 *        int *optIntStatusPtr (will always be assigned RS_xxx)
 */
#define I2V_UTIL_PROVIDER_SVC_REQ(svcptr, cfgptr, numMsTO, intRetValPtr, optIntStatusPtr) \
    if ((cfgptr)->iwmhAppEnable) { \
        *intRetValPtr = i2vUtilWMHServiceReq(MY_NAME, getpid(), WFALSE,  \
                svcptr, NULL, numMsTO, cfgptr, (uint8_t *)optIntStatusPtr);   \
    } else { \
        *intRetValPtr = *optIntStatusPtr = wsuRisProviderServiceRequest(svcptr); \
    }

/* input: UserServiceType *svcptr
 *        cfgItemsT *cfgptr (i2vcfg)
 *        int numMsTO (should be the number of milliseconds desired i.e. 5 ms)
 *        int intRetValPtr (will be assigned to I2V_RETURN_xxx or RS_xxx if no iwmh)
 *        int *optIntStatusPtr (will always be assigned RS_xxx)
 */
#define I2V_UTIL_USER_SVC_REQ(svcptr, cfgptr, numMsTO, intRetValPtr, optIntStatusPtr) \
    if ((cfgptr)->iwmhAppEnable) { \
        *intRetValPtr = i2vUtilWMHServiceReq(MY_NAME, getpid(), WTRUE,  \
                NULL, svcptr, numMsTO, cfgptr, (uint8_t *)optIntStatusPtr);   \
    } else { \
        *intRetValPtr = *optIntStatusPtr = wsuRisUserServiceRequest(svcptr); \
    }

/* input: outWSMType *wsmptr
 *        cfgItemsT *cfgptr (i2vcfg)
 *        int numMsTO (should be the number of milliseconds desired i.e. 5 ms)
 *        int intRetValPtr (will be assigned to I2V_RETURN_xxx or RS_xxx if no iwmh)
 *        int *optIntStatusPtr (will always be assigned RS_xxx)
 */
#define I2V_UTIL_WSM_REQ(wsmptr, cfgptr, numMsTO, intRetValPtr, optIntStatusPtr) \
    if ((cfgptr)->iwmhAppEnable) { \
        *intRetValPtr = i2vUtilWMHWSMReq(getpid(), wsmptr, numMsTO, cfgptr, (uint8_t *)optIntStatusPtr); \
    } else { \
        *intRetValPtr = *optIntStatusPtr = wsuRisSendWSMReq(wsmptr); \
    }

/* input: cfgItemsT *cfgptr (i2vcfg)
 *        int numMsTO (should be the number of milliseconds desired i.e. 5 ms)
 *        int intRetValPtr (will be assigned to I2V_RETURN_xxx or RS_xxx if no iwmh)
 *        uint8_t radioNum 
 *        PSIDType psid
 *        bool_t  *loopptr     (mainloop ptr)
 *        (void *)  rxFxn     (void wsmReceiveFunction(inWSMType *wsm))
 *        (void *)  vodFxn    (void smiVerifyOnDemand(wuint32 seqnum, wuint32 status)) OPTIONAL
 */
/* from ris_struct.h : Type definition of Provider Service Identifiers (PSID) (the actual values in this enum should not be used
   because they are out of date). */
#define I2V_UTIL_REGISTER_WSM_CB(cfgptr, numMsTO, intRetValPtr, radioNum, psid, loopptr, rxFxn, vodFxn) \
    *intRetValPtr = i2vUtilWMHRxCallback(getpid(), numMsTO, cfgptr, radioNum, psid, loopptr, rxFxn, vodFxn, NULL);

/* input:  uint8_t radioNum
 *         cfgItemsT *cfgptr
 *
 * Support for up to 4 radios (no wsu has that but i2v cfg supports that)
 *
 * MFM QNX port - -Waddress is enabled for QNX and this macro used to check if cfgptr is NULL. The problem is that the calling
 * code was always passing the address of a variable, and that will NEVER be NULL. So numerous warnings were issued by the compiler.
 */
#define IWMH_CHANNEL_NUMBER(radioNum, cfgptr) \
            (radioNum == 0) ? (cfgptr)->radio0wmh.radioChannel : (radioNum == 1) ? (cfgptr)->radio1wmh.radioChannel : \
            (radioNum == 2) ? (cfgptr)->radio2wmh.radioChannel : (cfgptr)->radio3wmh.radioChannel

/* input:  uint8_t radioNum
 *         cfgItemsT *cfgptr
 *         bool_t  setResult
 *
 * Support for up to 4 radios (no wsu has that but i2v cfg supports that)
 *
 * MFM QNX port - -Waddress is enabled for QNX and this macro used to check if cfgptr is NULL. The problem is that the calling
 * code was always passing the address of a variable, and that will NEVER be NULL. So numerous warnings were issued by the compiler.
 */
#define IWMH_CHECK_CONTINUOUS(radioNum, cfgptr, setResult) \
            if (!(cfgptr)->iwmhAppEnable) { \
                setResult = WFALSE; \
            } else if (radioNum == (cfgptr)->cchradio) { \
                setResult = WFALSE; \
            } else { \
                switch (radioNum) { \
                    case 0: setResult = (cfgptr)->radio0wmh.radioMode; break; \
                    case 1: setResult = (cfgptr)->radio1wmh.radioMode; break; \
                    case 2: setResult = (cfgptr)->radio2wmh.radioMode; break; \
                    case 3: setResult = (cfgptr)->radio3wmh.radioMode; break; \
                    default: setResult = WFALSE; \
                } \
            }

/* interface logging */
#define ACTIVATE_ETH0_LOGGING  i2vUtilIfcLogRequest(PHY_ETH0, IFC_ON);
#define DEACTIVATE_ETH0_LOGGING  i2vUtilIfcLogRequest(PHY_ETH0, IFC_OFF);
#define ACTIVATE_ETH1_LOGGING  i2vUtilIfcLogRequest(PHY_ETH1, IFC_ON);
#define DEACTIVATE_ETH1_LOGGING  i2vUtilIfcLogRequest(PHY_ETH1, IFC_OFF);
#define ACTIVATE_DSRC0_LOGGING  i2vUtilIfcLogRequest(PHY_DSRC0, IFC_ON);
#define DEACTIVATE_DSRC0_LOGGING  i2vUtilIfcLogRequest(PHY_DSRC0, IFC_OFF);
#define ACTIVATE_DSRC1_LOGGING  i2vUtilIfcLogRequest(PHY_DSRC1, IFC_ON);
#define DEACTIVATE_DSRC1_LOGGING  i2vUtilIfcLogRequest(PHY_DSRC1, IFC_OFF);
#define ACTIVATE_CV2X0_LOGGING  i2vUtilIfcLogRequest(PHY_CV2X0, IFC_ON);
#define DEACTIVATE_CV2X0_LOGGING  i2vUtilIfcLogRequest(PHY_CV2X0, IFC_OFF);
#define ACTIVATE_CV2X1_LOGGING  i2vUtilIfcLogRequest(PHY_CV2X1, IFC_ON);
#define DEACTIVATE_CV2X1_LOGGING  i2vUtilIfcLogRequest(PHY_CV2X1, IFC_OFF);


/* interface logging tracking files */
/* LOGMGR and ifclogger share these. Symbolic links. */
#define ETH0_IFC_TRACKER  "/tmp/eth0-tracker"
#define ETH1_IFC_TRACKER  "/tmp/eth1-tracker"
#define DSRC0_IFC_TRACKER "/tmp/dsrc0-tracker"
#define DSRC1_IFC_TRACKER "/tmp/dsrc1-tracker"
#define CV2X0_IFC_TRACKER "/tmp/cv2x0-tracker"
#define CV2X1_IFC_TRACKER "/tmp/cv2x1-tracker"

/* read note! do not change without doing so. */
#define TRACKER_LENGTH    (31)   /* count characters above + null */

/* IPB needs TPS. Security may need them in the future. */

/* touches static tpsUserData */
#define GETGPSLOCATION(a, b, c)  *a = tpsUserData.latitude; \
                                 *b = tpsUserData.longitude; \
                                 *c = tpsUserData.altitude; \
                                 return 0;

/* a is the data tpsDataType pointer - touches static i2vTpsTypeT tpsUserData 
   if data is ever valid we never set it back to invalid */
#define TPSCALLBACK(a) if (a->valid) { \
                           tpsUserData.latitude = a->latitude; \
                           tpsUserData.longitude = a->longitude; \
                           tpsUserData.altitude = a->altitude; \
                           tpsUserData.valid = WTRUE; \
                       } else { \
                           tpsUserData.latitude = 0; \
                           tpsUserData.longitude = 0; \
                           tpsUserData.altitude = 0; \
                       }

/* for debug support - if using this do not use TPSSETUP below
   a is assumed to be cfgItemsT struct */
#define DBGSETGPSLOCATION(a)   tpsUserData.latitude = a.gpsOvrLat; \
                               tpsUserData.longitude = a.gpsOvrLong; \
                               tpsUserData.altitude = a.gpsOvrElev; \
                               tpsUserData.valid = WTRUE;

/* a is the tpsCallback function defined above, intret is int value */
#define TPSSETUP(a)   wsuTpsInit(); \
                      if ((intret = wsuTpsRegister(a)) != TPS_SUCCESS) { \
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuTpsRegister failed (%d)\n", intret); \
                          return I2V_RETURN_FAIL; \
                      } else { \
                          return I2V_RETURN_OK; \
                      }

typedef struct {
    float64_t latitude;
    float64_t longitude;
    float64_t altitude;
    bool_t  valid;
} i2vTpsTypeT;

enum i2vCreateFileOptionsE {
    createFileNone           = 0,
    createFileAddDateTime    = 1,
    createFileBinaryFile     = 4,
    createFileAddDirSlash    = 8
};

/* bit mask for status of debug log creation */
typedef enum {
    I2V_LOG_ERR_NONE = 0x00,
    I2V_LOG_ERR_INPUT = 0x01,
    I2V_LOG_ERR_DBG = 0x02,
    I2V_LOG_ERR_CSV = 0x04,
    I2V_LOG_ERR_SUCCESS = 0x08,
} i2vLogErrT;

typedef struct {
    int32_t app_enable;
    int32_t create_log;
} i2vUtilLogStateT;

/* simple container for struct tm vals */
typedef struct {
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   min;
    uint8_t   sec;
    uint16_t  yday;  /* day of year */
    uint8_t   wday;  /* week day */
} i2vTimeT;

/* parsed time string struct */
typedef struct {
    int32_t    epochsecs;
    int32_t    epochusecs;
} i2vUtilTimeParams;


/* for iwmh */
typedef union {
    ProviderServiceType provider;
    UserServiceType     user;
} i2vWMHServiceTypes;

typedef union {
    uint8_t     byteT[0];
    uint32_t    intT;
} i2vWMHIntRespType;

typedef struct {
    char_t              name[I2V_LOG_NAME_MAX + 1];  /* + null */
    pid_t               pid;
    uint8_t             seqnum;
    bool_t              isUserReq;
    i2vWMHServiceTypes  servreq;
} i2vWMHServiceReq;

typedef struct {
    pid_t             pid;
    uint8_t           seqnum;
    outWSMType        wsm;
} i2vWMHWsmReq;

typedef struct {
    pid_t               pid;
    uint8_t             seqnum;
    PSIDType            psid;
} i2vWMHCBReq;

/* for both send and rcv msgs */
typedef struct {
    pid_t               pid;
    uint32_t            vodSeqNum;
    uint32_t            result;
} i2vWMHVODMsg;

/* MAX_WMH_RESP_SZ should be smaller than sizeof(i2vWMHServiceReq) and
   sizeof(i2vWMHWsmReq) */
typedef struct {
    int64_t              mtype;   /* will be the pid */
    uint8_t              mtext[MAX_WMH_RESP_SZ];
} i2vWMHRespMsg;

typedef struct {
    int64_t              mtype;   /* will be the pid */
    uint8_t              mtext[sizeof(i2vWMHVODMsg) + 1];
} i2vWMHVerifyQMsg;

typedef struct {
    int64_t              mtype;   /* will be the pid */
    uint8_t              mtext[sizeof(inWSMType) + 1];
} i2vWSMXferMsg;

/* this union is for messages sent by iwmh specific
   to input wsm related items */
typedef union {
    i2vWMHVerifyQMsg vodmsg;
    i2vWSMXferMsg    inwsmmsg;
} i2vWMHRcvTypes;

/* add any new structs here for supported message types */
typedef union wmhmsgtypes {
    i2vWMHServiceReq  svcreq;
    i2vWMHWsmReq      wsmreq;
    i2vWMHCBReq       cbreq;
} __attribute__((packed)) wmhmsgtypesU;

/* have support for extra messages in buffer
 *    that may be received at the same time */
#define IWMH_MAX_BUF          3 * (sizeof(union wmhmsgtypes) + 1)

typedef enum {
    INIT_NONE = 0,     /* this name must be this value */
    INIT_IN_PROGRESS,
    INIT_COMPLETE,
} i2vInitStatusT;
/* Syslogs has 0 to 7 levels only. See syslogdot.conf 
    REFERENCE "The Syslog Protocol (RFC5424): Table 2"

           emerg           (0),  -- emergency; system is unusable
           alert           (1),  -- action must be taken immediately
           crit            (2),  -- critical condition
           err             (3),  -- error condition
           warning         (4),  -- warning condition
           notice          (5),  -- normal but significant condition
           info            (6),  -- informational message
           debug           (7)   -- debug-level messages
*/
typedef enum {
    LEVEL_EMER = 0,
    LEVEL_ALERT,
    LEVEL_CRIT,
    LEVEL_ERR,
    LEVEL_WARN,
    LEVEL_NOTE,
    LEVEL_INFO,
    LEVEL_DBG,
    LEVEL_PRIV, /* Do not send to serial. Send to syslog only if enabled. */
} i2vLogSevT;


void i2vUtilCreateNewLogFiles(
                           FILE **csvlog,
                           FILE **dbglog,
                           bool_t logflg,
                           bool_t debuglog,
                           bool_t global_log,
                           bool_t global_debuglog,
                           char_t *title,
                           char_t *app_str,
                           char_t *applogs_dirname,
                           char_t *logs_dirname,
                           i2vLogErrT *status
                           );
i2vReturnTypesT i2vUtilCreateFile(
                       FILE **fd_in,
                       char_t *app_str,
                       char_t *path,
                       char_t *suffix,
                       int8_t options);

void            i2vUtilAppendTimeDateToFileName(char_t *filename);
void            i2vUtilGetUTCTime(i2vTimeT *clock);
uint32_t        i2vUtilGetUTCTimeInSec(void);
uint64_t        i2vUtilGetTimeInMs(void);
void            i2vUtilCloseFile(FILE **fd);
i2vReturnTypesT i2vUtilMakeDir(char_t *dirname);
i2vReturnTypesT i2vWriteDataToFile(char_t *fname, char_t *buf, uint32_t size, bool_t useAscii);
bool_t           i2vCheckDirOrFile(const char_t *pname);
bool_t           i2vScanDirForFile(const char_t *dirname, const char_t *pname);
i2vReturnTypesT i2vDirectoryListing(char_t *dirname, char_t *fname, char_t *extension);

uint16_t i2vUtilComputeCRC(uint8_t *data, size_t length);

uint32_t i2vUtilUpdateDoubleValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateInt8Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateInt16Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateInt32Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateStrValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateUint8Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateUint16Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateUint32Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdateWBOOLValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
uint32_t i2vUtilUpdatePsidValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
/* 'custom' functions */
void i2vUtilUpdateTimeStr(char_t __attribute__((unused)) *tag, char_t __attribute__((unused)) *parsedItem, char_t *value,
                          void *configItem, uint32_t *status);

uint32_t i2vUtilParseConfFile(FILE *f, cfgItemsTypeT *cfgItems, uint32_t numCfgItems, bool_t overrideFlag, i2vLogWrapper *logger);
int32_t  i2vUtilParseCfgArrayIndex(uint8_t *configstr, int32_t *index);

void i2vUtilIfcLogRequest(i2vIfcList ifc, lmgrIfcReqState state);
bool_t i2vUtilCertOrDigest(i2vShmMasterT *shrptr, cfgItemsT *cfg, uint32_t msgType);

/* iwmh functions (for apps broadcasting when iwmh enabled */
uint32_t i2vUtilWMHServiceReq(const char_t *name, pid_t pid, bool_t isUserReq, ProviderServiceType *pvservice, 
                              UserServiceType *uservice,uint16_t msecTO, cfgItemsT *mycfg, uint8_t *respBuf);
uint32_t i2vUtilWMHWSMReq(pid_t pid, outWSMType *wsm, uint16_t msecTO, cfgItemsT *mycfg, uint8_t *respBuf);
i2vReturnTypesT i2vUtilWMHRxCallback(pid_t pid, uint16_t msecTO, cfgItemsT *mycfg, uint8_t radioNum, PSIDType psid, 
                                     bool_t *mainloopptr, void *rxFunction, void *vodFunction, uint8_t *respBuf);
/* call this at termination when registering for receiving WSMs */
void i2vUtilRxCBCleanup(void);

/* call this in place of smiVerifyThisOne when iwmh enabled */
bool_t i2vUtilWMHVODVerify(uint32_t seqnum);

/* debug time - only one instance allowed */
bool_t timeMgrActivate(char_t *shmpath, i2vUtilTimeParams *data, bool_t fixedTrue);
void timeMgrDeactive(void);

/* TODO Historic note for QNX. Linux?
 * Should we have central place to setup and tear down mq's? I think so.
 * Queues persist across soft resets so if they get jacked they seem to stay jacked.
 * Only way I can think to resolve is to have orderly setup & tear down.
 */
uint32_t i2v_Util_Open_Queue(void); /* Used to open fwdmsg mq */
void  i2v_Util_Close_Queue(void);
wtime i2vTimeGetEpochMS( struct timeval *tv );
char_t * i2vTimeEpochMSToLogString(char_t *timeStr, int32_t size, uint64_t epochMS);
uint32_t J2735_DSRC_MSG_TYPE(amhBitmaskType bitmask);
uint8_t i2v_ascii_2_hex_nibble(uint8_t value); /* 8 bit hex to 4 bit hex (nibble) */

char_t * i2v_sigphase_to_traffic_light_color_string(i2vSigPhases sigPhase, bool_t mention_flashing);
char_t * i2v_movement_phase_state_to_string(i2vMovementPhaseState mps);
char_t * i2v_movement_phase_state_to_traffic_light_color_string(i2vMovementPhaseState mps, bool_t mention_flashing);
i2vMovementPhaseState i2v_signal_phase_to_movement_phase_state(i2vSigPhases sigPhase, uint8_t channelGreenType, i2vMovementPhaseState prior_movement_phase_state);
bool_t is_flashing_phase(uint32_t cur_phase);
#ifdef HSM
uint8_t fromHexDigit(char_t digit);
#endif

/* NOTE: for I2V_DBG_LOG()
 *   serial only available after dcu mounts blob.
 *   syslog only available after LOGMGR running and has setup mq.
 */
/* Enable serial or syslog output from I2V_DBG_LOG, you must call i2vUtilEnableDebug FIRST. */
void i2vUtilEnableDebug(char_t *name); /* Will enable enhanced debug by default */
void i2vUtilDisableDebug(void);
/* Enable syslog output from I2V_DBG_LOG, you must call i2vUtilEnableSyslog SECOND. */
int32_t i2vUtilEnableSyslog(i2vLogSevT level, char_t *name); /* Hard start, setup mq */
void i2vUtilDisableSyslog(void); /* Runtime disable. */
/* Enable/Disable Enhanced Debug:
 * Mutes all I2V_DBG_LOG with LEVEL greater than CRIT (see i2vLogSevT above).
 * Enabled by default by i2vUtilEnableDebug.
 */
void i2vUtilEnableEnhanced(void);
void i2vUtilDisableEnhanced(void);
/* Handles calls to I2V_DBG_LOG. Both serial and syslog output. */
void i2vUtilAddToSysLog(i2vLogSevT logLevel, const char_t *name, const char_t *msg, ...);

/* required for special cases of forwarding; return is number of bytes written or -1 (analagous to write() when config for device id disabled) */
int32_t i2vUtilTaggingWrite(int32_t fd, void *buf, size_t length, cfgItemsT *cfg);
void i2vUtilLoadDeviceID(cfgItemsT *cfg);
/* AMH support. */
uint32_t dsrcMsgId_to_amhMsgType(uint32_t dsrc_msgId);

/* For CFGMGR and I2V_UTIL for parsing of conf values. */
int32_t i2vCheckMac(char_t * value);
int32_t i2vCheckSspString(char_t * data);
int32_t i2vCheckPSID(char_t * data);
int32_t i2vCheckTxPower(char_t * value);
int32_t i2vCheckIPAddr(char_t * value, uint8_t ip_version);

#endif /* I2V_UTIL_H */
