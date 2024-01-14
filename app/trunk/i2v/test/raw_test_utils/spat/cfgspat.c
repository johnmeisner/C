/**************************************************************************
 *                                                                        *
 *     File Name:  spat.c                                                 *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "wsu_sharedmem.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_shm_master.h"
#include "i2v_util.h"

#include "DSRC.h"
#include "rtxsrc/rtxMemLeakCheck.h"
#include "rtxsrc/rtxDiag.h"

#define  DEFAULT_PRIORITY_CSPAT   6
#define  DEFAULT_TIMEOUT_CSPAT    100000
/* this value just needs to be enough for i2v max lanes per approach */
#define  MAX_NUM_LANE_IDS    10

typedef struct {
    wuint8            numLanes;
    wuint8            laneIDs[MAX_NUM_LANE_IDS];
} __attribute__((packed)) laneSetT;

typedef struct {
    PSIDType          psid;
    wuint16           timeout;
    wuint8            intName[I2V_CFG_MAX_STR_LEN];
    i2vSPATDataType   spatDat;
    laneSetT          laneSet[SPAT_MAX_APPROACHES];
} __attribute__((packed)) cfgSpatT;

static i2vShmMasterT *shmPtr = NULL;
static WBOOL          mainloop = WTRUE;
static amhSendParams  sendparams;
static wuint16        writelen = 0;
static wuint8         writebuf[MAX_WSM_DATA] = {0};
static wuint32        timeout = 0;

static OSCTXT         spatCtxt;

static void updateMovementCfg(int indx, wuint8 *val, void *configItem);
static void updateMovementLanes(int indx, wuint8 *val, void *configItem);

static  struct {
    wuint8 *tag;
    void (*updateFunction)(int indx, wuint8 *val, void *configItem);
} movementCfg[] = {
    {"MovementState", updateMovementCfg},
    {"MovementstateLanes", updateMovementLanes}
};

static void sigHandler(int __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

static void updateMovState(wuint8 GCC_UNUSED_VAR *tag, wuint8 *parsedItem, wuint8 *value, void *configItem, wuint32 *status)
{
    wint32 ret = 0;
    wuint32 i, indx;
    WBOOL matchFound = WFALSE;

    for (i = 0; i < NUMITEMS(movementCfg); i++) {
        if (strncmp(parsedItem, movementCfg[i].tag, strlen(movementCfg[i].tag)) == 0) {
            ret = i2vUtilParseCfgArrayIndex(parsedItem, &indx);
            if (ret == 0) {
                (*movementCfg[i].updateFunction)(indx, value, configItem);
            }
            matchFound = WTRUE;
            break;
        }
    }

    *status = (matchFound)?I2V_RETURN_OK:I2V_RETURN_FAIL;
}

/* the next two fxns overwrite any data previously set for a given movement
 - meaning if someone choses to have MovementState1 twice in the conf file
   the last one wins */
static void updateMovementCfg(int indx, wuint8 *val, void *configItem)
{
    cfgSpatT *mycfg = (cfgSpatT *)configItem;
    char *tok;
    wuint8 i;

    if ((NULL == mycfg) || (NULL == val)) return;

    for (i=0; i<SPAT_MAX_APPROACHES; i++) {
        if ((indx == mycfg->spatDat.spatApproach[i].approachID) ||
                (!mycfg->spatDat.spatApproach[i].approachID)) {
            if (!mycfg->spatDat.spatApproach[i].approachID) {
                mycfg->spatDat.numApproach++;
                mycfg->spatDat.spatApproach[i].approachID = (wuint8)indx;
            }
            tok = strtok(val, ",");
            GET_INT_TOKEN(tok, mycfg->spatDat.spatApproach[i].curSigPhase);
            GET_SHORT_TOKEN(tok, mycfg->spatDat.spatApproach[i].timeNextPhase);
            GET_BYTE_TOKEN(tok, mycfg->spatDat.spatApproach[i].yellowDur);
            break;
        }
    }
}

static void updateMovementLanes(int indx, wuint8 *val, void *configItem)
{
    cfgSpatT *mycfg = (cfgSpatT *)configItem;
    char *tok;
    wuint8 i,j;

    if ((NULL == mycfg) || (NULL == val)) return;

    for (i=0; i<SPAT_MAX_APPROACHES; i++) {
        if ((indx == mycfg->spatDat.spatApproach[i].approachID) ||
                (!mycfg->spatDat.spatApproach[i].approachID)) {
            if (!mycfg->spatDat.spatApproach[i].approachID) {
                mycfg->spatDat.numApproach++;
                mycfg->spatDat.spatApproach[i].approachID = (wuint8)indx;
            }
            tok = strtok(val, ",");
            for (j=0; j<MAX_NUM_LANE_IDS; j++) {
                GET_BYTE_TOKEN(tok, mycfg->laneSet[i].laneIDs[j]);
                if (!mycfg->laneSet[i].laneIDs[j]) {
                    break;
                } else {
                    mycfg->laneSet[i].numLanes++;
                }
            }
            break;
        }
    }
}

/* DER only supported for now */
static OSOCTET *asn1EncodeDER(wuint8 *buf, wuint16 buflen, SPAT *spat, wuint16 *outlen)
{
    int len = 0;

    xe_setp(&spatCtxt, buf, buflen);
    if ((len = asn1E_SPAT(&spatCtxt, spat, ASN1EXPL)) <= 0) {
        printf("SPAT ASN1 DER Encode error\n");
        rtxErrPrint(&spatCtxt);
        *outlen = 0;
        return NULL;
    } else {
        *outlen = (wuint16)len;
        return xe_getp(&spatCtxt);
    }
}

static WBOOL asn1FillSPAT(SPAT *spat, cfgSpatT *mycfg) 
{
    IntersectionState *intState = NULL;
    MovementState *moveState = NULL;
    wuint32 intId;
    int i, j;

    if ((NULL == spat) || (NULL == mycfg)) {
        return WFALSE;
    }
    spat->msgID = htonl(signalPhaseAndTimingMessage);
    
    rtxDListInit(&spat->intersections);
    intState = rtxMemAlloc(&spatCtxt, sizeof(IntersectionState));
    memset(intState, 0, sizeof(IntersectionState));
    rtxDListAppend(&spatCtxt, &spat->intersections, intState);

    if (strlen(mycfg->intName) > 0) {
        intState->m.namePresent = 1;
        intState->name = malloc(strlen(mycfg->intName) + 1);
        strcpy((char *)intState->name, mycfg->intName);
    }

    intState->id.numocts = htonl(sizeof(wuint32));
    intId = htonl(mycfg->spatDat.intID);
    memcpy(intState->id.data, &intId, sizeof(wuint32));

    intState->status.numocts = htonl(1);
    intState->status.data[0] = 0;

    intState->lanesCnt = mycfg->spatDat.numApproach;
    intState->m.lanesCntPresent = 1;

    rtxDListInit(&intState->states);


    for (i = 0; i < intState->lanesCnt; i++) {
        moveState = rtxMemAlloc(&spatCtxt, sizeof(MovementState));
        memset(moveState, 0, sizeof(MovementState));
        for (j=0; j<mycfg->laneSet[i].numLanes; j++) {
            moveState->laneSet.data[j] = mycfg->laneSet[i].laneIDs[j];
            moveState->laneSet.numocts++;
        }
        moveState->m.currStatePresent = 1;
        moveState->currState  = htonl(mycfg->spatDat.spatApproach[i].curSigPhase);
        moveState->timeToChange = htons(mycfg->spatDat.spatApproach[i].timeNextPhase);
        moveState->m.yellTimeToChangePresent = 1;
        moveState->yellTimeToChange = htons(mycfg->spatDat.spatApproach[i].yellowDur);

        rtxDListAppend(&spatCtxt, &intState->states, moveState);
    }

    return WTRUE;
}

static void asn1FreeSPAT(SPAT *spat)
{
    IntersectionState *intState;
    OSRTDListNode *node;

    node = rtxDListFindByIndex(&spat->intersections, 0); /* only one intersection */
    intState = node->data;

    rtxDListFreeAll(&spatCtxt, &intState->states);

    if (intState->m.namePresent == 1) {
        free((void *)intState->name);
    }

    rtxDListFreeAll(&spatCtxt, &spat->intersections);

    rtxMemReset(&spatCtxt);
}

static i2vReturnTypesT updateCfg(char *cfgFileName, cfgSpatT *mycfg)
{
    cfgItemsTypeT cfgItems[] = {
        {"PSID",                i2vUtilUpdateUint32Value, &mycfg->psid,          NULL,0},
        {"Timeout",             i2vUtilUpdateUint16Value, &mycfg->timeout,       NULL,0},
        {"IntersectionID",      i2vUtilUpdateUint32Value, &mycfg->spatDat.intID, NULL,0},
        {"MovementState",       NULL,                      mycfg,                updateMovState,0},
        {"MovementstateLanes",  NULL,                      mycfg,                updateMovState,0},
    };
    FILE *f;
    char fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
    wuint32 retVal;  /* used for i2vUtilParseConfFile - fxn returns i2vReturnTypesT although listed as wuint32 */
    WBOOL canrun = WFALSE;

    WSU_SEM_LOCKR(&shmPtr->cfgData.h.ch_lock);
    strncpy(fileloc, shmPtr->cfgData.config_directory, I2V_CFG_MAX_STR_LEN);
    canrun = shmPtr->cfgData.amhAppEnable;
    if (shmPtr->cfgData.security) {
        sendparams.security = SECURITY_SIGNED;
    } else {
        sendparams.security = SECURITY_UNSECURED;
    }
    WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
    if (!canrun) {
        printf("required i2v app for this app to run is not active: amh\n");
        return I2V_RETURN_FAIL;
    }

    strcat(fileloc, cfgFileName);

    if ((f = fopen(fileloc, "r")) == NULL) {
        perror(fileloc);
        return I2V_RETURN_INV_FILE_OR_DIR;
    }

    retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL);
    fclose(f);
    sendparams.psid = mycfg->psid;
    sendparams.priority = DEFAULT_PRIORITY_CSPAT;

    timeout = (mycfg->timeout) ? (mycfg->timeout * 1000) : DEFAULT_TIMEOUT_CSPAT;

    return retVal;
}

static WBOOL spatInitialize(void)
{
    cfgSpatT mycfg;
    SPAT spat;
    OSOCTET *spatMsg = NULL;

    shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shmPtr == NULL) {
        printf("%d-%s: Main shared memory init failed\n", getpid(), __FUNCTION__);
        return I2V_RETURN_FAIL;
    }
    memset(&spat, 0, sizeof(spat));
    asn1Init_SPAT(&spat);
    if (rtInitContext(&spatCtxt) != 0) {
        printf("%d-%s: failed to initialize spat structs\n", getpid(), __FUNCTION__);
        rtxErrPrint(&spatCtxt);
        rtxErrReset(&spatCtxt);
        return WFALSE;
    }

    /* update cfg and build spat */
    memset(&mycfg, 0, sizeof(mycfg));

    if (I2V_RETURN_OK != updateCfg("cfgspat.conf", &mycfg)) {
        printf("%d-%s: failed to update cfg\n", getpid(), __FUNCTION__);
        return WFALSE;
    }

    printf("spat from file preparing data\n");
    if (!asn1FillSPAT(&spat, &mycfg)) {
        printf("%d-%s: failed to set J2735 data struct\n", getpid(), __FUNCTION__);
        return WFALSE;
    }

    printf("spat from file encoding data\n");
    if (NULL == (spatMsg = asn1EncodeDER(writebuf, MAX_WSM_DATA, &spat, &writelen))) {
        printf("%d-%s: failed to encode spat data\n", getpid(), __FUNCTION__);
        rtxErrReset(&spatCtxt);
        return WFALSE;
    }
    memcpy(writebuf, spatMsg, writelen);

#ifdef EXTRA_DEBUG
    {
        int i;
        for (i=0; i<mycfg.spatDat.numApproach; i++) {
            printf("apprch cfg dat: %d %d %d\n", mycfg.spatDat.spatApproach[i].curSigPhase, 
                                                 mycfg.spatDat.spatApproach[i].timeNextPhase,
                                                 mycfg.spatDat.spatApproach[i].yellowDur);
        }
        printf("spat dat %d:\n", writelen);
        for (i=0; i<writelen; i++){
            printf("%.2x ", writebuf[i]);
            if (i && !(i%20)) printf("\n");
        }
        printf("\n");
    }
#endif /* EXTRA_DEBUG */

    asn1FreeSPAT(&spat);
    /* code below should never happen - but safety in case writelen somehow corrupted */
    if (writelen > MAX_WSM_DATA) {
        printf("%d-%s: truncating send message; result was too big for wsm send: %d\n", getpid(), __FUNCTION__, writelen);
        writelen = MAX_WSM_DATA;
    }

    return WTRUE;
}


int main(void)
{
    struct sigaction sa;
    
    memset(&spatCtxt, 0, sizeof(spatCtxt));
    memset(&sendparams, 0, sizeof(sendparams));

    /* signal handle */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("\ninitializing spat from file\n\n");
    if (!spatInitialize()) {
        return -1;
    }

    printf("spat from file being sent to spat...\n");

    while(mainloop) {
        /* in theory with this test app, should only need to write once but
           since using amh buffer - need to write because amh is active and
           could have received message that overwrites this one */
        WSU_SEM_LOCKR(&shmPtr->spatBcastData.h.ch_lock);
        memcpy(shmPtr->spatBcastData.data, writebuf, writelen);
        memcpy(&shmPtr->spatBcastData.sendparams, &sendparams, sizeof(sendparams));
        shmPtr->spatBcastData.count = writelen;
        shmPtr->spatBcastData.newmsg = WTRUE;    /* must set this flag or msg is ignored */
        WSU_SEM_UNLOCKR(&shmPtr->spatBcastData.h.ch_lock);

        if (mainloop) {
            usleep(timeout);
        }
    }

    return 0;
}

