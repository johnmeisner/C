/**************************************************************************
 *                                                                        *
 *     File Name:  iwmh.c                                                 *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* Functional notes:
 *   1. multiple instances of iwmh expected when multiple radios enabled
 *   2. input radio number determines which msg queues used 
 *   3. iwmh allows for sharing PSIDs among senders on the same radio
 */
/* NOTE on EU Mode - from lab testing the socket file descriptor passed
 * back between different processes from the BTP library was the same
 * value.  So there's no need to have a single process for EU mode since
 * the library is already handling that - as such no EU Mode code in iwmh
 */
/* TODO: 20210310: security functions are #if 0'ed out in most cases
 *  follow up quickly before this becomes a blur!  Either delete or replace
 *  security blocks and clean up code.
 */
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_riscapture.h"
#include "amh_types.h"
#include "rsuhealth.h"
#include "alsmi_api.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN    my_main
#else
#define dn_open open
#define dn_read read
#define MAIN    main
#endif

/* From amh_types.h. */
#define MAX_EXTRA_PSID MAX_AMH_EXTRA_PSID

//#define EXTRA_DEBUG
#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

#define INVALID_WMH_RADIO     0xFF

/* have support for extra messages in buffer
   that may be received at the same time */
#define IWMH_MAX_BUF          3 * (sizeof(union wmhmsgtypes) + 1)
#define REC_EXTRA             4

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "iwmh"

/* error states */
#define IWMH_RSUHEALTH_SHM_NOT_READY 0x1
#define IWMH_RSUHEALTH_TX_NOT_READY  0x2
#define IWMH_RSUHEALTH_RX_NOT_READY  0x4
#define IWMH_TXHEALTH_THR_EXIT       0x8

typedef struct regrec_s {
    struct regrec_s  *nextRecord;
    struct regrec_s  *firstRecord;
    pid_t             owner;
    char_t            ownername[I2V_LOG_NAME_MAX + 1];
    PSIDType          psid;
    pid_t             extraOwners[REC_EXTRA];
} __attribute__((packed)) regRecord;

typedef struct cbreg_s {
    struct cbreg_s   *nextRecord;
    pid_t             owner;
    PSIDType          psid;
    uint32_t           lastSeqNum;
} __attribute__((packed)) cbRecord;

#if 0
/* add any new structs here for supported message types */
union wmhmsgtypes {
    i2vWMHServiceReq  svcreq;
    i2vWMHWsmReq      wsmreq;
    i2vWMHCBReq       cbreq;
} __attribute__((packed)) wmhmsgtypesU;
#endif

/* ditto above - but remember these are pointers */
typedef union {
    i2vWMHServiceReq  *svcreq;
    i2vWMHWsmReq      *wsmreq;
    i2vWMHCBReq       *cbreq;
} wmhRequestType;

static i2vShmMasterT *shmPtr = NULL;
STATIC bool_t mainloop = WTRUE;
STATIC cfgItemsT cfg;
static regRecord *reglist = NULL;
static cbRecord  *cblist  = NULL;
static int32_t qFd = -1;    /* message queue handle */
static uint8_t rcvbuf[IWMH_MAX_BUF];
static uint8_t myradio = INVALID_WMH_RADIO;
static int32_t mylistener = -1;
static uint8_t numpsids = 0;
static uint32_t mypsidlist[MAX_EXTRA_PSID];
static bool_t vodInited = WFALSE;
static pthread_t vodthrID;
STATIC pthread_t txhlthID;
static pid_t mypid;
static uint8_t mychannel = 0;
static bool_t myisContinuous = WFALSE;
static bool_t iwmhCv2xTxReady = WFALSE;
static bool_t iwmhCv2xRxReady = WFALSE;
static uint32_t iwmhTx_notReady = 0x0;
static uint32_t iwmhRx_notReady = 0x0;
static uint32_t iwmhTx_cnt = 0x0;
static uint32_t iwmhTx_err = 0x0;
static uint32_t iwmhRx_cnt = 0x0;
static uint32_t iwmhRx_err = 0x0;
static uint64_t iwmh_error_states = 0x0;
static uint32_t check_tx_status = 0; /* Unitl you check you dont know anything. */

static void iwmh_initStatics(void)
{
  mainloop = WTRUE;
#if !defined(MY_UNIT_TEST) /* Unit Test will set for test, don't clear. */
  memset(&cfg,0x0,sizeof(cfg));
#endif
  reglist = NULL;
  cblist  = NULL;
  qFd = -1;    
  memset(rcvbuf,0x0,sizeof(rcvbuf));
  myradio = INVALID_WMH_RADIO;
  mylistener = -1;
  numpsids = 0;
  memset(mypsidlist,0x0,sizeof(mypsidlist));
  vodInited = WFALSE;
  memset(&vodthrID,0x0,sizeof(vodthrID));
  memset(&mypid,0x0,sizeof(mypid));
  mychannel = 0;
  myisContinuous = WFALSE;
  iwmh_error_states = 0x0;
  memset(&vodthrID,0x0,sizeof(vodthrID));
  memset(&txhlthID,0x0,sizeof(txhlthID));
  memset(&mypid,0x0,sizeof(mypid));
  iwmhCv2xTxReady = WFALSE;
  iwmhCv2xRxReady = WFALSE;
  iwmhTx_cnt = 0x0;
  iwmhTx_err = 0x0;
  iwmhRx_cnt = 0x0;
  iwmhRx_err = 0x0;
}
#if 0
/* for security */
static i2vTpsTypeT tpsUserData;

/* security related functions */
static int32_t iwmhGPSLocation(float64_t *latitude, float64_t *longitude, float64_t *elevation)
{
    GETGPSLOCATION(latitude, longitude, elevation);
}

static void iwmhTPSCallback(tpsDataType *data)
{
    TPSCALLBACK(data);
}

static i2vReturnTypesT iwmhTPSSetup(void)
{
    int32_t intret;
    TPSSETUP(iwmhTPSCallback);
}
/* end security related functions */
#endif

static void iwmhSighandler(int __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

static bool_t getCBOwnerRec(pid_t pid, cbRecord *target)
{
    cbRecord *curRec = cblist;

    if ((cblist == NULL) || (target == NULL)) {
        return WFALSE;
    }

    while (WTRUE) {
        if (NULL == curRec) {
            return WFALSE;
        }
        if (curRec->owner == pid) {
            target = curRec;
            return WTRUE;
        } else {
            curRec = curRec->nextRecord;
        }
    }

    return WFALSE;  /* should never reach here */
}

static bool_t getCBPsidRec(PSIDType psid, cbRecord **target, bool_t getNext)
{
    cbRecord *curRec;

    if ((cblist == NULL) || (target == NULL)) {
        return WFALSE;
    }
    curRec = (getNext) ? (*target)->nextRecord : cblist;

    while (WTRUE) {
        if (NULL == curRec) {
            return WFALSE;
        }
        if (curRec->psid == psid) {
            *target = curRec;
            return WTRUE;
        } else {
            curRec = curRec->nextRecord;
        }
    }

    return WFALSE;  /* should never reach here */
}

/* TODO: if multiple radios are active and different
   apps are listening for WSMs (and VOD enabled) the iwmhSecQ
   may load up quickly - used for all radios - may need
   individual queues (max of 16) OR may need different 
   messaging framework - queues are better for messaging
   than shared memory fyi */
/* uses cfg.iwmhSecQ */
static void iwmhRxHandler(inWSMType *wsm)
{
    i2vWSMXferMsg msg;
    inWSMType *msgdata;
    cbRecord *curRec = NULL;
    bool_t status = WTRUE, lookup = WFALSE;

    /* pass wsm to pid(s) with matching psid - 
       could be multiple registered with same psid - 
       they'll have to figure out if its theirs */
    memset(&msg, 0, sizeof(msg));
    msg.mtext[0] = WMH_IN_WSM;
    msgdata = (inWSMType *)&msg.mtext[1];
    memcpy(msgdata, wsm, sizeof(inWSMType));
    iwmhRx_cnt++;
    while (status) {
        if (getCBPsidRec(wsm->psid, &curRec, lookup)) {
            lookup = WTRUE;  /* to now lookup next matching psid - and prevent infinite loop */
            msg.mtype = (int64_t)curRec->owner;
            if (msgsnd(cfg.iwmhSecQ, (void *)&msg, sizeof(i2vWSMXferMsg), IPC_NOWAIT) < 0) {
                /* no retries - vod on calling app should have a timeout */
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," failed to send inWSM (%d %s)\n", curRec->owner, strerror(errno));
#endif /* EXTRA_DEBUG */
                status = WFALSE; /* don't try to send any more */
                iwmhRx_err++;      
            }
        } else {
            status = WFALSE;
        }
    }
}
#if 0 /* Legacy security model */
/* uses cfg.iwmhSecQ */
static void iwmhVODCallback(uint32_t result, uint32_t seqnum)
{
    cbRecord *curRec = cblist;
    i2vWMHVODMsg *msgdata;
    i2vWMHVerifyQMsg msg;
    uint8_t msize = sizeof(msg);

    memset(&msg, 0, msize);
    msgdata = (i2vWMHVODMsg *)msg.mtext;

    /* pass identical results back to owning pid */
    while (WTRUE) {
        if (NULL == curRec) {
            break;
        }
        if (seqnum == curRec->lastSeqNum) {
            msg.mtype = curRec->owner;
            msgdata->vodSeqNum = seqnum;
            msgdata->result = result;
            if (msgsnd(cfg.iwmhSecQ, (void *)&msg, msize, IPC_NOWAIT) < 0) {
                /* no retries - vod on calling app should have a timeout */
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," failed to send reply (%d %s)\n", curRec->owner, strerror(errno));
#endif /* EXTRA_DEBUG */
                return;   /* this is redundant but needed if !defined EXTRA_DEBUG */
            }
            return;
        } else {
            curRec = curRec->nextRecord; /* if nextRecord null, covered by first part of loop */
        }
    }
}
#endif
/* no validation of source - just sending and moving on */
static void *iwmhVODThread(void __attribute__((unused)) *arg)
{
    i2vWMHVerifyQMsg *msg;
    i2vWMHVODMsg *msgdata;
    uint8_t inbuf[sizeof(i2vWMHVerifyQMsg)];
    uint8_t bufsz = sizeof(i2vWMHVerifyQMsg);
    cbRecord *curRec = NULL;

    msg = (i2vWMHVerifyQMsg *)&inbuf;
    msgdata = (i2vWMHVODMsg *)msg->mtext;
    /* listen for requests and send to smi */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,"iwmhVODThread","Entering mainloop.\n");
#endif
    while (mainloop) {
        if (((msgrcv(cfg.iwmhSecQ, (void *)inbuf, bufsz, (int64_t)mypid, IPC_NOWAIT)) < 0) &&
           ((errno == EAGAIN) || (errno == ENOMSG))) {
            usleep(1000);
        } else {
            if (getCBOwnerRec(msgdata->pid, curRec)) {
                /* cppcheck: this is not necessary but to suppress the error */
                if (NULL != curRec) {
                    curRec->lastSeqNum = msgdata->vodSeqNum;
                }
#if 0
                smiVerifyThisOne(msgdata->vodSeqNum);
#endif
            }  /* will not execute if blank list; ignore request */
        }
    }

    pthread_exit(NULL);
}

/* Only check RSUHEALTH SHM for status of radio. Either ready or not. */
static void *iwmhTXHealthCheck(void __attribute__((unused)) *arg)
{
/* Keep going if RSUDIAG not ready, not fatal: 2 second intervals */ 
#define RSUDIAG_SHM_WAIT_MAX 20
    //FILE *pf;
    char_t buf[60];
    rsuhealth_t * shm_rsuhealth_ptr = NULL;
    rsuhealth_t localrsuhealth;
    size_t shm_rsuhealth_size = sizeof(rsuhealth_t);
    uint32_t rsudiag_notReady = 0x0;

    memset(&localrsuhealth,0x0,sizeof(localrsuhealth));
    /* Need to be true at least once for event message to come out the first time. ie no gps at start. */
    iwmhCv2xTxReady  = WFALSE; /* Until check_tx_status=1 we have no idea if these are ready. */
    iwmhCv2xRxReady  = WFALSE;
    if ((shm_rsuhealth_ptr = wsu_share_init(shm_rsuhealth_size, RSUHEALTH_SHM_PATH)) == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSUHEALTH SHM init failed.\n");
        shm_rsuhealth_ptr = NULL;
    } else {
        while (mainloop) {
            sleep(2);  /* Put sleep here so continue doens't bypaass below. Every other second should be acceptable */
            memset(buf, 0, sizeof(buf));
            /* no support for dual modes, either dsrc (RadioType = 0) or cv2x (RadioType = 1) */
            if (cfg.RadioType) {
                if(rsudiag_notReady < RSUDIAG_SHM_WAIT_MAX) {
                    WSU_SHMLOCK_LOCKR(&shm_rsuhealth_ptr->h.ch_lock);
                    if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                        memcpy(&localrsuhealth,shm_rsuhealth_ptr,sizeof(localrsuhealth));
                        rsudiag_notReady = 0x0;
                    } else {
                        localrsuhealth.h.ch_data_valid = WFALSE;
                        rsudiag_notReady++;
                    }
                    WSU_SHMLOCK_UNLOCKR(&shm_rsuhealth_ptr->h.ch_lock);
                } else {
                    /* RSUDIAG handshake failed, keep going, not fatal. */
                    localrsuhealth.h.ch_data_valid = WTRUE;
                    localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_TX_READY;
                    localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_RX_READY;
                }
                if(WFALSE == localrsuhealth.h.ch_data_valid) {
                    if(iwmhCv2xTxReady || iwmhCv2xRxReady) {
                        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"RSUHEALTH SHM not ready: TX & RX inactive\n");
                    }
                    iwmhCv2xTxReady = WFALSE;
                    iwmhTx_notReady++;
                    iwmhCv2xRxReady = WFALSE;
                    iwmhRx_notReady++;
                    iwmh_error_states |= IWMH_RSUHEALTH_SHM_NOT_READY;
                    continue; /* Nothing to do let logic fall through to bottom. */
                }
                iwmh_error_states &= ~IWMH_RSUHEALTH_SHM_NOT_READY;
                if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY) {
                    if (!iwmhCv2xTxReady) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"starting/resuming TX\n");
                    }
                    iwmh_error_states &= ~IWMH_RSUHEALTH_TX_NOT_READY;
                    iwmhCv2xTxReady = WTRUE;
                } else {
                    if(iwmhCv2xTxReady) {
                        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"TX inactive\n");
                    }
                    iwmhCv2xTxReady = WFALSE;
                    iwmhTx_notReady++;
                    iwmh_error_states |= IWMH_RSUHEALTH_TX_NOT_READY;
                }
                if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY) {
                    if (!iwmhCv2xRxReady) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"starting/resuming RX\n");
                    }
                    iwmhCv2xRxReady = WTRUE;
                    iwmh_error_states &= ~IWMH_RSUHEALTH_RX_NOT_READY;
                } else {
                    if(iwmhCv2xRxReady) {
                        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"RX inactive\n");
                    }
                    iwmhCv2xRxReady = WFALSE;
                    iwmhRx_notReady++;
                    iwmh_error_states |= IWMH_RSUHEALTH_RX_NOT_READY;
                }
            } else {
#if defined(PLATFORM_HD_RSU_5912)
                /* dsrc checks tpstestapp using new status data */
                /* TODO: should we consider a minimal duration before turning off the radio? */
                if (NULL != (pf = popen(GNSS_CHECK_OK, "r"))) {
                    /* if we have something, continue */
                    fgets(buf, sizeof(buf), pf);
                    pclose(pf);
                    if (strlen(buf) > 2) {
                        if (!iwmhCv2xTxReady) {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"detected good gps, starting/resuming TX\n");
                        }
                        iwmhCv2xTxReady = WTRUE;
                    } else {
                        if (iwmhCv2xTxReady) {
                            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"detected bad gps, ceasing TX\n");
                        }
                        iwmhCv2xTxReady = WFALSE;
                    }
                } else {
                    /* even in error condition set to false; an error would mean a problem on 9150 */
                    if (iwmhCv2xTxReady) {
                        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"couldn't check gps, ceasing TX\n");
                    }
                    iwmhCv2xTxReady = WFALSE;
                }
#endif /* PLATFORM_HD_RSU_5912 */
            }
            check_tx_status = 1;
        } /* While */
        /* remove shared mem */
        wsu_share_kill(shm_rsuhealth_ptr, shm_rsuhealth_size);
        shm_rsuhealth_ptr = NULL;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"TX health thread exiting: %s\n", (cfg.RadioType) ? "C-V2X" : "DSRC");
#endif
    iwmh_error_states |= IWMH_TXHEALTH_THR_EXIT;
    pthread_exit(NULL);
#undef RSUDIAG_SHM_WAIT_MAX
}

/* added this for future expansion if needed */
static bool_t iwmhArgs(int argc, char *argv[])
{
    int32_t c = 0;
#if defined(MY_UNIT_TEST)
    optind = 0; /* If calling more than once you must reset to zero for getopt.*/
#endif
    while ((c=getopt(argc, argv, "p:r:")) != -1) {
        switch (c) {
            case 'p':     /* psid (multiple allowed */
                if (numpsids < MAX_EXTRA_PSID) {
                    mypsidlist[numpsids++] = (uint32_t)strtoul(optarg, NULL, 16);
                }
                break;
            case 'r':     /* radionum */
                myradio = (uint8_t)strtoul(optarg, NULL, 10);
                break;
            default:
                break;
        }
    }

    if (myradio == INVALID_WMH_RADIO) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid usage - input radio argument not set\n");
        return WFALSE;
    }
    /* myradio value will be validated later */

    return WTRUE;
}

/*  This function intializes security related items for RSU with
    VOD and VtP related config items. */
static void iwmhSecurityInit()
{
    rsSecurityInitializeType    security;
    int                         retVal;

        security.debug                             = FALSE;
        security.vodEnabled                        = cfg.vodEnable;
        security.smi_cert_change_callback          = NULL;
        security.vtp_milliseconds_per_verification = cfg.securityVtPMsgRateMs;
        security.smi_sign_results_options          = smi_results_all;
        security.smi_sign_results_callback         = NULL;
        security.smi_sign_wsa_results_options      = smi_results_none;
        security.smi_sign_wsa_results_callback     = NULL;
        security.smi_verify_wsa_results_options    = smi_results_none;
        security.smi_verify_wsa_results_callback   = NULL;
        strcpy(security.lcmName, "rsuLcm"); // from denso.wsc
        security.numSignResultsPSIDs               = 1;
        security.signResultsPSIDs[0]               = cfg.srmPsid;

#if !defined(MY_UNIT_TEST)
        retVal = smiInitialize(&security);

        if (retVal != 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "IWMH ERROR: SMI Failed to Initialize (%d). exiting\n", retVal);
            mainloop=FALSE;
        }
#endif
}

static int32_t iwmhInitialize(void)
{
    //i2vLogErrT result;
    char_t qname[I2V_CFG_MAX_STR_LEN] = {0};
    struct sigaction sa;
    int32_t i=0;

    shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shmPtr == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Main shared memory init failed\n");
        return WFALSE;
    }
    mypid = getpid();
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = iwmhSighandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#if !defined(MY_UNIT_TEST)
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
#endif
    /* If we fail to get I2V SHM update or sig'ed to exit beforehand then fail. */
    if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)");
        return I2V_RETURN_FAIL;
    }

    /* IPC */
    switch (myradio) {
        case 0: 
            mylistener = cfg.radio0wmh.msgListener; 
            mychannel = cfg.radio0wmh.radioChannel;
            myisContinuous = cfg.radio0wmh.radioMode;
            break;
        case 1: 
            mylistener = cfg.radio1wmh.msgListener; 
            mychannel = cfg.radio1wmh.radioChannel;
            myisContinuous = cfg.radio1wmh.radioMode;
            break;
        case 2: 
            mylistener = cfg.radio2wmh.msgListener; 
            mychannel = cfg.radio2wmh.radioChannel;
            myisContinuous = cfg.radio2wmh.radioMode;
            break;
        case 3: 
            mylistener = cfg.radio3wmh.msgListener; 
            mychannel = cfg.radio3wmh.radioChannel;
            myisContinuous = cfg.radio3wmh.radioMode;
            break;
        default: 
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid radio configuration: %d\n", myradio);
            return WFALSE;
    }
    /* 20210311: temporary HACK, overwrite channel; this may become permanent;
       if this is permanent, remember that it applies to all 'radios', which would
       likely break the system if more than one radio; however, will there ever be
       more than one radio at once? */
    if (cfg.bcastLockStep) {
        mychannel = cfg.uchannel;
    }

    /* Do smiInit here */
    if (cfg.security)
        iwmhSecurityInit();

    /* open msgq for receiving */
    sprintf(qname, "%s%d", I2V_WMH_QUEUE, myradio);
    if ((qFd = dn_open(qname, I2V_Q_READER)) < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error opening message queue (%s): %s\n", qname, strerror(errno));;
        return WFALSE;
    }
    if (myisContinuous) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v operating on radio %d in continuous mode\n", myradio);
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v operating on radio %d in alternating mode\n", myradio);
    }

    if (cfg.gpsCtlTx) {
        if (pthread_create(&txhlthID, NULL, iwmhTXHealthCheck, NULL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create TX health thread. errno=%d\n",errno);
            return WFALSE;
        }
    } else {
        iwmhCv2xTxReady = WTRUE;
        iwmhCv2xRxReady = WTRUE;
        check_tx_status = 1;
    }

    return WTRUE;
}

static bool_t iwmhInitRadio(void)
{
    int32_t retVal;

    appCredentialsType appCredential = {
        .cbServiceAvailable = NULL,
        .cbReceiveWSM = iwmhRxHandler,
    };

#if 0
    cfgType risCfg;
    /* time shift support */
    if (cfg.bcastTimeShift) {
        risCfg.u.AdjustCCLTxTiming = 3;   /* Magic number from the radio stack that enables time shifter mode */
        if ((retVal = wsuRisSetCfgReq(CFG_ID_ADJUST_CCL_TX_TIMING, &risCfg)) != RS_SUCCESS) {
            I2V_DBG_LOG("CCL timeshifter enable failed (%d)\n", retVal);
        }
        /* fail startup ? - not now - do not return wfalse */
    }  
#endif
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Call wsuRisInitRadioComm.\n");
#endif
    if ((retVal = wsuRisInitRadioComm(MY_NAME, &appCredential)) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v radio init failed (%d)\n", retVal);
        return WFALSE;
    }

#if 0
#define SMILOG "/mnt/rwflash/I2V/applogs/iwmhsmi.log"
#define SMISTATS "/mnt/rwflash/I2V/applogs/iwmhsmistats.log"
    i2vOptionalSecArgsT args;
    uint32_t mainpsid;
    if (cfg.security) {
        I2V_DBG_LOG("starting security init - security enabled\n");
        memset(&args, 0, sizeof(args));
        memset(&tpsUserData, 0, sizeof(i2vTpsTypeT));
        if (cfg.enableGPSOverride) {
            I2V_DBG_LOG("using file override GPS instaed of actual GPS data\n");
            DBGSETGPSLOCATION(cfg);
        } else {
            if (I2V_RETURN_OK != iwmhTPSSetup()) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"could not register for tps data for security\n");
                if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminate radio comm failed (%d)\n", retVal);
                }
                return WFALSE;
            }
        }

        /* wsu 1.0/1.5 note on security initialize: the input psid and psidlist is passed to the SMI layer
           and used by the SMI layer only for input WSMs - thus it doesn't matter what value is
           used for security initialization so long as ALL supported psids for Rx WSMs are
           included - i2v passes in the Rx psids */
        mainpsid = (numpsids) ? mypsidlist[0] : cfg.secSpatPsid;  /* choose any secure psid if none passed in */
        if ((numpsids > 1) || (cfg.iwmhVerifyOD)) {
            if (cfg.iwmhVerifyOD) {
                args.resultFxn = iwmhVODCallback;
                args.f.securityOnDemand = 1;
            }
            if (numpsids > 1) {
                memcpy(args.secPsidList, &mypsidlist[1], (numpsids - 1)*sizeof(PSIDType));
                args.numSecPsid = numpsids - 1;
                args.f.psidList = 1;
            }
            if (I2V_RETURN_OK != securityInitialize(myradio, 0xFFFF, mainpsid, iwmhGPSLocation,
                    cfg.wsaEnable, cfg.globalApplogsEnableFlag, SMILOG, SMISTATS, &args, &iwmhLogger)) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"could not connect to security module\n");
                if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminate radio comm failed (%d)\n", retVal);
                }
                return WFALSE;
            }
        } else {
            if (I2V_RETURN_OK != securityInitialize(myradio, 0xFFFF, mainpsid, iwmhGPSLocation,
                    cfg.wsaEnable, cfg.globalApplogsEnableFlag, SMILOG, SMISTATS, NULL, &iwmhLogger)) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"could not connect to security module\n");
                if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminate radio comm failed (%d)\n", retVal);
                }
                return WFALSE;
            }
        }
    }
#undef SMILOG
#undef SMISTATS
#endif

    return WTRUE;
}

static bool_t serviceRegistration(i2vWMHServiceReq *req, uint8_t *ret)
{
    regRecord *parentRec = NULL, *curRec = NULL;
    PSIDType reqpsid;
    bool_t adding;
    rsResultCodeType retVal = RS_SUCCESS;
    uint8_t i;

    if ((req == NULL) || (ret == NULL)) {
        return WFALSE;
    }
    *ret = RS_SUCCESS;
    /* verify radio just in case */
    if (req->isUserReq) {
        if (req->servreq.user.radioNum != myradio) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"user service request received for invalid radio: m[%d] r[%d]\n", 
                myradio, req->servreq.user.radioNum);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }
    } else {
        if (req->servreq.provider.radioNum != myradio) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"provider service request received for invalid radio: m[%d] r[%d]\n", 
                myradio, req->servreq.provider.radioNum);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }
    }
    reqpsid = (req->isUserReq) ? req->servreq.user.psid : req->servreq.provider.psid;
    if (req->isUserReq) {
        adding = (req->servreq.user.action == ADD) ? WTRUE : WFALSE;
        /* overwrite channel */
        if (req->servreq.user.channelNumber != I2V_CONTROL_CHANNEL) {
            req->servreq.user.channelNumber = mychannel;
        }
        /* alternating/continuous mode - cch radio handled later (myisContinuous already biased to cch radio) */
        if (myisContinuous) {
            req->servreq.user.immediateAccess = 1;
            req->servreq.user.extendedAccess = 65535;
#ifndef RSE_EUPROTO
        } else {
            req->servreq.user.immediateAccess = 0;
            req->servreq.user.extendedAccess = 0;
#endif /* !RSE_EUPROTO */
        }
    } else {
        adding = (req->servreq.provider.action == ADD) ? WTRUE : WFALSE;
        /* overwrite channel */
    }
#ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," svc registration details: psid[%X] adding[%d] user[%d]\n", reqpsid, adding, req->isUserReq);
#endif /* EXTRA_DEBUG */

    if (adding) {
        /* have to add registration details to list (or traverse list if already there)
           before making actual service request */
        if (reglist == NULL) {
            if (NULL == (reglist = (regRecord *)malloc(sizeof(regRecord)))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to allocate memory for reglist. errno=%d\n",errno);
                return WFALSE;
            }
            memset(reglist, 0, sizeof(regRecord));
            reglist->firstRecord = reglist;
            reglist->owner = req->pid;
            strncpy(reglist->ownername, req->name, I2V_LOG_NAME_MAX);  /* in case input req->name not null term */
            reglist->psid = reqpsid;
            curRec = reglist;
        } else {
            curRec = reglist;
            while (WTRUE) {
                if (curRec->psid == reqpsid) {
                    /* do not have to add registration - just return ok after saving owner if needed */
                    if (curRec->owner != req->pid) {
                        for (i=0; i<REC_EXTRA; i++) {
                            if (curRec->extraOwners[i] == req->pid) {
                                break;
                            }
                            if (!curRec->extraOwners[i]) {
                                curRec->extraOwners[i] = req->pid;
                                break;
                            }
                            /* if neither condition above true, extra owner pid will be ignored */
                        }
                    }
                    return WTRUE;
                }
                if (NULL != curRec->nextRecord) {
                    parentRec = curRec;
                    curRec = curRec->nextRecord;
                } else {
                    /* add new record and break */
                    if (NULL == (curRec->nextRecord = (regRecord *)malloc(sizeof(regRecord)))) {
#ifdef EXTRA_DEBUG
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to allocate additional record space in reglist\n");
#endif /* EXTRA_DEBUG */
                        return WFALSE;
                    }
                    memset(curRec->nextRecord, 0, sizeof(regRecord));
                    curRec->nextRecord->firstRecord = curRec->firstRecord;
                    curRec = curRec->nextRecord;
                    curRec->owner = req->pid;
                    strncpy(curRec->ownername, req->name, I2V_LOG_NAME_MAX);
                    curRec->psid = reqpsid;
                    break;
                }
            }
        }
    } else {
        /* removing */
        if (reglist == NULL) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"request to remove registration but none present\n");
#endif /* EXTRA_DEBUG */
            return WTRUE;
        } else {
            curRec = reglist;
            while (WTRUE) {
                if (curRec->psid == reqpsid) {
                    if (req->pid == curRec->owner) {
                        /* break to make request */
                        /* NOTE: this implementation does not support keeping a registration 
                           if additional pids made a request (curRec->extraOwners[]) - this is 
                           a known limitation - it should never actually occur since the only
                           app that should deregister is ipb and its psid is unique */
                        break;
                    } else {
                        /* check if extraOwner first */
                        for (i=0; i<REC_EXTRA; i++) {
                            if (req->pid == curRec->extraOwners[i]) {
                                do {
                                    if (i == (REC_EXTRA - 1)) {
                                        curRec->extraOwners[i] = 0;
                                        return WTRUE;
                                    }
                                    curRec->extraOwners[i] = curRec->extraOwners[i + 1];
                                    i++;
                                } while (i<(REC_EXTRA));  /* this loop will exit at the return */
                            }
                        }
                        /* check the ownername if pid doesn't match (could be a child from owner) */
                        if (strcmp(req->name, curRec->ownername)) {
                            /* not the owner - fail the request */
#ifdef EXTRA_DEBUG
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"request to remove service registration from invalid owner %d[%s] valid: %d[%s]\n",
                                          req->pid, req->name, curRec->owner, curRec->ownername);
#endif /* EXTRA_DEBUG */
                            return WFALSE;
                        }
                        /* break to make request */
                        break;
                    }
                }
                if (curRec->nextRecord == NULL) {
                    /* nothing to remove just return ok */
                    return WTRUE;
                }
                parentRec = curRec;
                curRec = curRec->nextRecord;
            }
        }
    }

    /* at this point perform registration request - make sure request
       on CCH channel is properly set for channel switching */
    if (req->isUserReq) {
//#ifndef RSE_EUPROTO
/* 20210421 (JJG): this code was written a long time ago when the theory
   was that there was a dedicated control channel; several years later and
   several FCC sucker punches later, a second channel is no longer a
   practice; so don't assume channel switching is required; stick to the
   config value; removing this block completely
*/
#if 0
        if (req->servreq.user.radioNum == cfg.cchradio) {
            /* overwrite channel switching vars as appropriate */
            req->servreq.user.immediateAccess = 0;
            req->servreq.user.extendedAccess = 0;
        }
#endif /* RSE_EUPROTO */
        if (RS_SUCCESS != (retVal = wsuRisUserServiceRequest(&req->servreq.user))) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to make user service request (%d) %s %d\n", retVal, req->name, req->servreq.user.action);
#endif /* EXTRA_DEBUG */
        }
    } else {
        if (RS_SUCCESS != (retVal = wsuRisProviderServiceRequest(&req->servreq.provider))) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to make provider service request (%d) %s %d\n", retVal, req->name, req->servreq.provider.action);
#endif /* EXTRA_DEBUG */
        }
    }

    /* result & req type determines next step */
    if (retVal == RS_SUCCESS) {
        /* if adding - nothing else to do; if deleting free record, curRec already pointing to removal record */
        if (!adding) {
            if (NULL != curRec->nextRecord) {
                if (parentRec == NULL) {
                    /* this is the first record */
                    reglist = curRec->nextRecord;
                } else {
                    parentRec->nextRecord = curRec->nextRecord;
                }
                free(curRec);
            } else {
                if (NULL == parentRec) {
                    /* first record and no more */
                    reglist = NULL;
                } else {
                    parentRec->nextRecord = NULL;
                }
                free(curRec);
            }
        }
    } else {
        /* did not succeed on registration - if deleting nothing else to do, if adding remove created rec */
        if (adding) {
            if (curRec == reglist) {
                /* one and only record */ 
                reglist = NULL;
            } else {
                parentRec->nextRecord = NULL;
            }
            free(curRec);
        }
    }

    *ret = (uint8_t)retVal;

    return (retVal == RS_SUCCESS) ? WTRUE : WFALSE;
}

static bool_t iwmhWsmSend(i2vWMHWsmReq *req, uint8_t *ret)
{
    rsResultCodeType risRet = RS_SUCCESS;

    if ((req == NULL) || (ret == NULL)) {
        return WFALSE;
    }
    if ((!iwmhCv2xTxReady) || (check_tx_status == 0)) { /* Not really a conf option for CV2X. */
        *ret = (uint8_t)abs(RS_AT_CV2X_TX_NOT_READY);
        iwmhTx_err++;
        return WFALSE;
    }
    if (req->wsm.channelNumber == I2V_CONTROL_CHANNEL) {
        req->wsm.channelInterval = 1;  /* CCH interval */
    } else {
        /* overwrite channel just in case */
        req->wsm.channelNumber = mychannel;
        req->wsm.channelInterval = 0;  /* setting just in case */
    }

    /* the assignment below will work because the enum currently 
       has no values more than 255 (absolute) - but it does 
       change the return value to unsigned */
    risRet = wsuRisSendWSMReq(&req->wsm);
    *ret = (uint8_t)abs(risRet);
    if(RS_SUCCESS == risRet) {
        iwmhTx_cnt++;
        return WTRUE;
    } else {
        iwmhTx_err++;
        return WFALSE;
    }
}
//TODO: How to de-reg?
static bool_t iwmhRxCBRegistration(i2vWMHCBReq *req, uint32_t *retpid)
{
    cbRecord *curRec = cblist;
    WsmServiceType service;
    rsResultCodeType retval;
    bool_t foundPsid = WFALSE;

    if (cfg.iwmhVerifyOD && !vodInited) {
        /* create thread that services vod requests from other apps */
        if (pthread_create(&vodthrID, NULL, iwmhVODThread, NULL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create VOD thread. errno=%d\n",errno);
            return WFALSE;
        }
    }

    *retpid = mypid;

    /* add req to our list */
    while (WTRUE) {
        if (cblist == NULL) {
            if ((curRec = (cbRecord *)malloc(sizeof(cbRecord))) == NULL) {
                /* won't be too many of these so logging this is fine */
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to add callback record to app requesting wsms. errno=%d\n",errno);
                return WFALSE;
            } else {
                /* radio stack call to enable receiving psid */
                service.action = ADD;
                service.radioType = (cfg.RadioType == 1) ? RT_CV2X : RT_DSRC;
                service.psid = req->psid;
                if ((retval = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to add wsm psid to radio: %d\n", retval);
                    free(curRec);
                    return WFALSE;
                }

                memset(curRec, 0, sizeof(cbRecord));
                curRec->owner = req->pid;
                curRec->psid = req->psid;
                /* safety check anyway */
                cblist = curRec;
                return WTRUE;
            }
        } else if ((curRec->owner == req->pid) && (curRec->psid == req->psid)) {
            /* do not add extra records for duplicate requests */
            return WTRUE;
        } else if (curRec->nextRecord != NULL) {
            if (!foundPsid) {
                foundPsid = (curRec->psid == req->psid) ? WTRUE : WFALSE;
            }
            curRec = curRec->nextRecord;
            continue;
        } else {
            /* null next record, create new record */
            if ((curRec->nextRecord = (cbRecord *)malloc(sizeof(cbRecord))) == NULL) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to add callback record to app requesting wsms. errno=%d.\n",errno);
                return WFALSE;
            } else {
                if (!foundPsid) {
                    service.action = ADD;
                    service.radioType = (cfg.RadioType == 1) ? RT_CV2X : RT_DSRC;
                    service.psid = req->psid;
                    if ((retval = wsuRisWsmServiceRequest(&service)) != RS_SUCCESS) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to add wsm psid to radio: %d\n", retval);
                        free(curRec->nextRecord);
                        return WFALSE;
                    }
                }

                memset(curRec->nextRecord, 0, sizeof(cbRecord));
                curRec->nextRecord->owner = req->pid;
                curRec->nextRecord->psid = req->psid;
                return WTRUE;
            }
        }
    }

    return WFALSE;   /* should never reach here */
}

static void iwmhSendResponse(pid_t pid, uint8_t seqnum, bool_t result, uint8_t *data, uint32_t size)
{
    i2vWMHRespMsg respBuf;

    if (NULL == data) {
        size = 0;
    }
    if (size > MAX_WMH_RESP_SZ - 2) {
        /* provide something */
        size = MAX_WMH_RESP_SZ - 2;
    }
    memset(&respBuf, 0, sizeof(respBuf));
    respBuf.mtype = (int64_t)pid;
    respBuf.mtext[0] = seqnum;
    respBuf.mtext[1] = result;
    if (size) {
        memcpy(&respBuf.mtext[2], data, size);
    }
    if (msgsnd(mylistener, (void *)&respBuf, size + 2, IPC_NOWAIT) < 0) {
        /* no retries */
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," failed to send reply (%d %s)\n", pid, strerror(errno));
#endif /* EXTRA_DEBUG */
        return;   /* this is redundant but needed if !defined EXTRA_DEBUG */
    }
}

static void iwmhMain(void)
{
    wmhRequestType req;
    uint8_t ret;
    int32_t count, expectedcount;
    bool_t result;
    i2vWMHIntRespType intret;
    uint32_t control_count = 0x0;

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%d) ready to operate (%d, %d)\n", getpid(), qFd, mylistener);
#if !defined(MY_UNIT_TEST)
    wsu_open_gate(&shmPtr->cfgData.iwmhGate);
#endif
    while (mainloop) {
        if(0 == (control_count % (OUTPUT_MODULUS * 1000))){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(0x%lx) tx,E,R(%u,%u,%d) rx,E,R(%u,%u,%d)\n",
                iwmh_error_states, iwmhTx_cnt, iwmhTx_err,iwmhCv2xTxReady,iwmhRx_cnt, iwmhRx_err,iwmhCv2xRxReady);
        }
        control_count++;
        memset(rcvbuf, 0, IWMH_MAX_BUF);
        ret = 0;
        result = WFALSE;
        memset(&intret, 0, sizeof(intret));
        count = expectedcount = 0;
        if ((count = dn_read(qFd, rcvbuf, IWMH_MAX_BUF)) <= 0) {
            usleep(1000);   /* 1 ms */
            continue;
        }
        if (!mainloop) {
            break;
        }
extraMsg:
        /* first byte in buf is the command */
        switch (rcvbuf[0]) {
            case WMH_SERVICE_REQUEST_CMD:
                req.svcreq = (i2vWMHServiceReq *)&rcvbuf[1];
                expectedcount = sizeof(i2vWMHServiceReq) + 1;
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rcvd request from %d (%d)\n", req.svcreq->pid, count);
#endif /* EXTRA_DEBUG */
                result = serviceRegistration(req.svcreq, &ret);
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"replying to %d (%d)\n", req.svcreq->pid, result);
#endif /* EXTRA_DEBUG */
                iwmhSendResponse(req.svcreq->pid, req.svcreq->seqnum, result, &ret, 1);
                break;
            case WMH_SEND_WSM_CMD:
                req.wsmreq = (i2vWMHWsmReq *)&rcvbuf[1];
                expectedcount = sizeof(i2vWMHWsmReq) + 1;
                result = iwmhWsmSend(req.wsmreq, &ret);
                iwmhSendResponse(req.wsmreq->pid, req.wsmreq->seqnum, result, &ret, 1);
                break;
            case WMH_RX_CB_REGISTER_CMD:
                req.cbreq = (i2vWMHCBReq *)&rcvbuf[1];
                expectedcount = sizeof(i2vWMHCBReq) + 1;
                result = iwmhRxCBRegistration(req.cbreq, &intret.intT);
                iwmhSendResponse(req.cbreq->pid, req.cbreq->seqnum, result, intret.byteT, sizeof(intret));
                break;
            /* add new commands here */
            default:
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," invalid command received: %d (%d)\n", rcvbuf[0], count);
#endif /* EXTRA_DEBUG */
                /* have to ignore the rest - yikes - there could be something valid after but
                   how can it be processed? */
                expectedcount = count = 0;
                break;
        }
        /* validate size as multiple messages may be read at the same time */
        if (count > expectedcount) {
            memcpy(&rcvbuf[0], &rcvbuf[expectedcount], (count - expectedcount));  /* this moves unprocessed bytes up */
            count -= expectedcount;
            memset(&rcvbuf[count], 0, (IWMH_MAX_BUF - count));  /* remove duplicate bytes to prevent unexpected errors */
            goto extraMsg;
        }
        #if defined(MY_UNIT_TEST)
        if(2 < control_count)
            break;
        sleep(1);
        #endif
    } /* While */
}

static void iwmhCleanup(void)
{
    int32_t retVal = 0;
    regRecord *rcurRec;
    cbRecord  *ccurRec;

    /* free allocated memory */
    if (NULL != reglist) {
        while (reglist) {
            rcurRec = reglist;
            reglist = reglist->nextRecord;
            free(rcurRec);
        }
    }
    if (NULL != cblist) {
        while (cblist) {
            ccurRec = cblist;
            cblist = cblist->nextRecord;
            free(ccurRec);
        }
    }

    if ((retVal = wsuRisTerminateRadioComm()) != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminate radio comm failed (%d)\n", retVal);
    }

#if 0
    if (cfg.security) {
        smiTerm();
    }
#endif

    if (vodInited) {
        pthread_join(vodthrID, NULL);
    }

#if 0
    wsu_share_detach(shmPtr);
#endif
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: ret(%d) (0x%lx) tx,E,R(%u,%u,%d) rx,E,R(%u,%u,%d)\n",
                0, iwmh_error_states, iwmhTx_cnt, iwmhTx_err,iwmhCv2xTxReady,iwmhRx_cnt, iwmhRx_err,iwmhCv2xRxReady);
#endif

    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();

    if(NULL != shmPtr) {
        wsu_share_kill(shmPtr, sizeof(shmPtr));
    }
}

int32_t MAIN(int32_t argc, char_t *argv[])
{
    /* Clear statics across soft reset. */
    iwmh_initStatics();

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    /* these fxns have prints in them */
    if (!iwmhArgs(argc, argv)) {
        return 1;
    }

    if (!iwmhInitialize()) {
        return 2;
    }

    if (!iwmhInitRadio()) {
        return 3;
    }

    iwmhMain();

    iwmhCleanup();

    return 0;
}

