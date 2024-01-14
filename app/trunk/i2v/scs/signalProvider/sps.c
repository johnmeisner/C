/**************************************************************************
 *                                                                        *
 *     File Name:  sps.c  (signal provider simulator)                     *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* 
  Simulator operation: simulator will either use map data or config file
  data (determined by config file setting).  Existing SCS code only understands
  Econolite ASC3 format (specifically for approaches - the approach doesn't
  match the CICAS-V format TOM approach) so simulator supplies same format.

  Simulator will send spat (tom format) to message queue at the frequency
  defined in the config file.  SCS will read from the message queue 
  instead of listening to the inet socket in simulator mode and process
  data just like a normal signal controller.

  Simulator state machine assumes simple countdown and provides updated
  info for each approach defined for intersection.  If map data is used
  simulator will address turn lanes arbitrarily (no config options for
  turn lanes).

*/
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "wsu_shm.h"
#include "i2v_shm_master.h"
#include "i2v_util.h"

/* TOM format:
   Byte      Field       Value 
    1        Msg Type     0xF1
    2        Msg Ver      0x02
   3-4       Total Lngth   unsigned 16 bit (big endian)
   5-6       CRC           unsigned 16 bit
   ...       data 
   N         term flag    0xF1

data is layers
spat layer:

   1        obj ID (layer)  0x01
   2        obj size        0x06
  3-4       layer type      0x02  (spat)
   5        layer id        0x01
   6        content ver     0x01  (increments)
   7        obj id          0x02  (intersection)
   8        obj size        0x06
  9-12      inter id       xxxxxxxx  (uint32)
   13       obj id          0x03  (approach)
   14       obj size        0x09
   15       appr id         (match intersection)
  16-17     sig state      (1-green, 2-yellow, 4-red, etc)
   18       timer           0x00   (ignore both)
  19-20     time to next    uint16   (hundredths of sec)
   21       yellow dur      uint8
...repeat (per approach)
   N-5      CLOSE            0
   N-4      Approach         3
   N-3      CLOSE            0
   N-2      Intersection     2
   N-1      CLOSE            0
    N       Layer            1

ASC3 - yellow dur is two bytes (ignored by scs anyway) and 
timer comes AFTER time to next which is in tenths of sec

this simulator must match ASC3 format to be parsed
*/

#define LAYER_TYPE_SPAT        2
#define OBJECT_ID_APPROACH     3
#define OBJECT_ID_LAYER        1
#define OBJECT_ID_INTERSECTION 2
#define CONTENT_SPAT_VERSION   1

#define TOM_SIZE_SPAT_LAYER    6
#define TOM_SIZE_INTERSECTION  6
#define TOM_SIZE_APPROACH      10

/* only support very simple states */
#define SIGNAL_GREEN           1
#define SIGNAL_YELLOW          2
#define SIGNAL_RED             4

/* in practice - the value below is illogical */
#define MAX_SUPPORTED_STATE_IDS  (MAX_APPROACH - 1)


/* mandatory logging defines */
#define MY_ERR_LEVEL    LEVEL_WARN
#define MY_NAME         "sps"

#define SPS_CFG_FILE    "rse-sim.conf"


/* JJG: may want to pull these to a common header
   if in the future updating spat and scs to use
   i2v structures instead of old wsu_tom format */
/* this is ASC3 format - not TOM format */
typedef struct {
    wuint8     objID;
    wuint8     objSize;
    wuint8     apprID;
    wuint16    sigstate;
    wuint16    timeNext;
    wuint8     timer;   /* leave as 0 */
    wuint16    ignore;
    wuint16    close;
} __attribute__((packed)) spsApproach;

typedef struct {
    wuint8     objID;
    wuint8     objSize;
    wuint32    intID;
    spsApproach approaches[MAX_APPROACH];
    wuint16    close;
} __attribute__((packed)) spsIntersection;

typedef struct {
    wuint8     objID;
    wuint8     objSize;
    wuint16    layerType;
    wuint8     layerID;
    wuint8     content;
    spsIntersection intersection;
    wuint16    close;
} __attribute__((packed)) spsSpatLayer;


/* config struct */
typedef struct {
    wuint8     interval;
    WBOOL      usemap;
    wuint8     numapproaches;
    WBOOL      intOverride;   /* for now it only applies to one intersection type */
    wuint8     intDirection;
    wuint8     greendur;
    wuint8     yeldur;
    wuint8     reddur;
    wuint32    intID;
} __attribute__((packed)) spsCfgTypeT;


/* intersection state */
typedef struct s_spsapproach {
    spsApproach data;
    struct s_spsapproach *firstRecord;
    struct s_spsapproach *nextRecord;
} spsApproachSignalState;

typedef struct s_spsxstate {
    struct s_spsxstate *firstRecord;
    struct s_spsxstate *nextRecord;
    spsApproachSignalState *spssignal[MAX_SUPPORTED_STATE_IDS];
    wuint8   ids[MAX_SUPPORTED_STATE_IDS];
    wuint8   numids;
    wuint8   signalstatus;
    wuint8   direction;
    wuint32  lasttime;
    wuint16  timeNext;
    wuint16  sigState;
} spsIntersectionObject;

typedef struct {
    spsIntersectionObject *statelist;
    spsApproachSignalState *approachlist;
} spsIntersectionState;

static spsSpatLayer spsmsgdata;
static spsIntersectionState spsintersection; 
static wuint8 spsintercount = 0;
static int fd;   /* msg queue file descriptor */

static i2vShmMasterT *shmPtr = NULL;
static cfgItemsT cfg;
static WBOOL mainloop = WTRUE;
static i2vLogWrapper spsLogger;
static WBOOL dbgLogFlag = WFALSE;
/* need *fcsv although unused because utility function
   will generate a null ptr if fcsv not passed in */
static FILE *fcsv = NULL, *flog = NULL;

static spsCfgTypeT spsCfg;

extern unsigned short i2vComputeCRC(char *buf, size_t length);

static void spsSigHandler(int __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

static i2vReturnTypesT spsUpdateCfg(char *cfgFileName)
{
    cfgItemsTypeT cfgItems[] = {
        {"Interval",           i2vUtilUpdateUint8Value,  &spsCfg.interval,      NULL,0},
        {"UseI2VMap",          i2vUtilUpdateWBOOLValue,  &spsCfg.usemap,        NULL,0},
        {"NumApproaches",      i2vUtilUpdateUint8Value,  &spsCfg.numapproaches, NULL,0},
        {"IntersectionType",   i2vUtilUpdateUint8Value,  &spsCfg.intOverride,   NULL,0},
        {"IntersectionDirection", i2vUtilUpdateUint8Value,  &spsCfg.intDirection, NULL,0},
        {"GreenDuration",      i2vUtilUpdateUint8Value,  &spsCfg.greendur,      NULL,0},
        {"YellowDuration",     i2vUtilUpdateUint8Value,  &spsCfg.yeldur,        NULL,0},
        {"RedDuration",        i2vUtilUpdateUint8Value,  &spsCfg.reddur,        NULL,0},
        {"IntersectionID",     i2vUtilUpdateUint32Value, &spsCfg.intID,         NULL,0},
    };
    FILE *f;
    char fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
    wuint32 retVal;  /* used for i2vUtilParseConfFile - fxn returns i2vReturnTypesT although listed as wuint32 */
    i2vLogErrT result;

    WSU_SEM_LOCKR(&shmPtr->cfgData.h.ch_lock);
    memcpy(&cfg, &shmPtr->cfgData, sizeof(cfgItemsT));  /* save config for later use */
    WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);

    /* logging setup */
    dbgLogFlag = cfg.globalDebugFlag;
    i2vUtilCreateNewLogFiles(&fcsv, &flog, WFALSE, dbgLogFlag, WFALSE, dbgLogFlag, "", "sps", 
        cfg.config_cmn_applog_dir, cfg.config_cmn_dbglog_dir, &result);

    if (result & I2V_LOG_ERR_INPUT) {
        printf("sps internal error: all internal logging disabled\n");
    }
    if (result & I2V_LOG_ERR_DBG) {
        printf("sps internal error: debug logging not enabled\n");
    }
    if (result & I2V_LOG_ERR_CSV) {
        printf("sps internal error: app (csv) logging not enabled\n");
    }

    strncpy(fileloc, cfg.config_directory, I2V_CFG_MAX_STR_LEN);
    strcat(fileloc, cfgFileName);

    if ((f = fopen(fileloc, "r")) == NULL)
    {
        perror(fileloc);
        return I2V_RETURN_INV_FILE_OR_DIR;
    }

    retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, &spsLogger);
    fclose(f);

    return retVal;
}

/* based on id and direction (turn lanes get separated) create a new
   intersection object or add to existing one - assumes default ids from map - 
   see map_gen.conf */
static WBOOL spsInitStateDirection(wuint8 apprID, wuint8 direction, spsApproachSignalState *apprRecord)
{
#define ITERATE \
        if (NULL == (*thisrecord)->nextRecord) { \
            addrecord = WTRUE; \
            break; \
        } else { \
            thisrecord = &((*thisrecord)->nextRecord); \
        }
    
    spsIntersectionObject **thisrecord;
    WBOOL addrecord = WFALSE, addtothisrecord = WFALSE;
    WBOOL isturnlane = WFALSE, needturnlane = WFALSE; 
    wuint8 oppositedir;

    if ((NULL == spsintersection.statelist) || (NULL == apprRecord)) {
        return WFALSE;
    }

    thisrecord = &(spsintersection.statelist);
    while (WTRUE) {
        /* use any value (even blank) from id list to determine if current intersection
           object is for turn lane */
        switch((*thisrecord)->direction) {
            case MAP_NORTH: 
                oppositedir = MAP_SOUTH; 
                needturnlane = (apprID == 3 || apprID == 7) ? WTRUE : WFALSE;
                isturnlane = ((*thisrecord)->ids[0] == 3 || (*thisrecord)->ids[0] == 7) ? 
                                WTRUE : WFALSE;
                break;
            case MAP_SOUTH: 
                oppositedir = MAP_NORTH; 
                needturnlane = (apprID == 3 || apprID == 7) ? WTRUE : WFALSE;
                isturnlane = ((*thisrecord)->ids[0] == 3 || (*thisrecord)->ids[0] == 7) ? 
                                WTRUE : WFALSE;
                break;
            case MAP_EAST: 
                oppositedir = MAP_WEST; 
                needturnlane = (apprID == 1 || apprID == 5) ? WTRUE : WFALSE;
                isturnlane = ((*thisrecord)->ids[0] == 1 || (*thisrecord)->ids[0] == 5) ? 
                                WTRUE : WFALSE;
                break;
            case MAP_WEST: 
                oppositedir = MAP_EAST; 
                needturnlane = (apprID == 1 || apprID == 5) ? WTRUE : WFALSE;
                isturnlane = ((*thisrecord)->ids[0] == 1 || (*thisrecord)->ids[0] == 5) ? 
                                WTRUE : WFALSE;
                break;
            default: 
                oppositedir = MAP_NO_DIRECTION; 
                break;
        }
        if (((*thisrecord)->numids) && ((direction == (*thisrecord)->direction) ||
            (direction == oppositedir))) {
            /* matching direction but need to account for turn lanes - different state */
            if ((isturnlane && needturnlane) || (!isturnlane && !needturnlane)) {
                /* match */
                addtothisrecord = WTRUE;
                break;
            } else {
                /* no match */
                ITERATE
            }
        } else if ((*thisrecord)->numids) {
            /* different direction; move to next record */
            ITERATE
        } else {
            /* this record has no ids - (i.e. first execution) add to record */
            addtothisrecord = WTRUE;
            (*thisrecord)->direction = direction;
            spsintercount = 1;
            break;
        }
    }

    if (addtothisrecord) {
        (*thisrecord)->ids[(*thisrecord)->numids] = apprID;
        (*thisrecord)->spssignal[(*thisrecord)->numids] = apprRecord;
        (*thisrecord)->numids++;
        return WTRUE;
    }

    if (addrecord) {
        /* thisrecord already pointing to last record */
        (*thisrecord)->nextRecord = (spsIntersectionObject *)malloc(sizeof(spsIntersectionObject));
        if (NULL == (*thisrecord)->nextRecord) {
            return WFALSE;
        } else {
            memset((*thisrecord)->nextRecord, 0, sizeof(spsIntersectionObject));
            (*thisrecord)->nextRecord->firstRecord = (*thisrecord)->firstRecord;
            (*thisrecord)->nextRecord->spssignal[0] = apprRecord;
            (*thisrecord)->nextRecord->ids[0] = apprID;
            (*thisrecord)->nextRecord->numids = 1;
            (*thisrecord)->nextRecord->direction = direction;
            spsintercount++;
        }
    }

    return WTRUE;
#undef ITERATE
}

/* 0 based index into total number of approaches;
   return value is direction of approach 
   - arbitrary assignment */
static wuint8 spsSetDefaultApproach(wuint8 index)
{
#define CREATE_APPROACH(a, b) \
    if (spsintersection.approachlist->data.objID) { \
        spsintersection.approachlist->nextRecord = (spsApproachSignalState *)malloc(sizeof(spsApproachSignalState)); \
        if (NULL == spsintersection.approachlist->nextRecord) { \
            printf("failed to allocate memory for default approach\n"); \
            return MAP_NO_DIRECTION; \
        } else { \
            memset(spsintersection.approachlist->nextRecord, 0, sizeof(spsApproachSignalState)); \
            spsintersection.approachlist->nextRecord->firstRecord = spsintersection.approachlist->firstRecord; \
            spsintersection.approachlist = spsintersection.approachlist->nextRecord; \
        } \
    } \
    spsintersection.approachlist->data.objID = OBJECT_ID_APPROACH; \
    spsintersection.approachlist->data.objSize = TOM_SIZE_APPROACH; \
    spsintersection.approachlist->data.apprID = a; \
    spsintersection.approachlist->data.close = htons(OBJECT_ID_APPROACH); \
    return b;

    struct {
        union pat
            wuint32 totalval;
            wuint8  laneids[4];
        } id;
        union {
            wuint32 totalval;
            wuint8  dirids[4]; 
        } dir;
    } defdatarray;

    switch (spsCfg.intDirection) {
        case MAP_NORTH:
            defdatarray.id.totalval = htonl(0x06080204);
            defdatarray.dir.totalval = htonl(
            MAP_WEST << 24 | MAP_NORTH << 16 | MAP_EAST << 8 | MAP_SOUTH );
            break;
        case MAP_EAST:
            defdatarray.id.totalval = htonl(0x08020406);
            defdatarray.dir.totalval = htonl(
            MAP_NORTH << 24 | MAP_EAST << 16 | MAP_SOUTH << 8 | MAP_WEST );
            break;
        case MAP_SOUTH:
            defdatarray.id.totalval = htonl(0x02040608);
            defdatarray.dir.totalval = htonl(
            MAP_EAST << 24 | MAP_SOUTH << 16 | MAP_WEST << 8 | MAP_NORTH );
            break;
        case MAP_WEST:
            defdatarray.id.totalval = htonl(0x04060802);
            defdatarray.dir.totalval = htonl(
            MAP_SOUTH << 24 | MAP_WEST << 16 | MAP_NORTH << 8 | MAP_EAST );
            break;
        default:
            printf("invalid intersection direction\n");
            return MAP_NO_DIRECTION;
    }

    switch (spsCfg.numapproaches) {
        case 1:   /* 'I' intersection */
            if (index) {
                printf("invalid index for default approach [%d %d]\n", spsCfg.numapproaches, index);
                return MAP_NO_DIRECTION;
            }
            /* 'east' to 'west' - rotated ids accounted for in switch above */
            CREATE_APPROACH(defdatarray.id.laneids[2], defdatarray.dir.dirids[2])
        case 2:   /* 'L' or 'I' */
            if (index > 1) {
                printf("invalid index for default approach [%d %d]\n", spsCfg.numapproaches, index);
                return MAP_NO_DIRECTION;
            }
            if (spsCfg.intOverride) {
                /* use 'I' intersection */
                if (index) {
                    /* 'west' to 'east' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[0], defdatarray.dir.dirids[0])
                } else {
                    /* 'east' to 'west' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[2], defdatarray.dir.dirids[2])
                }
            } else {
                /* use 'L' intersection */
                if (index) {
                    /* 'north' to 'south' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[1], defdatarray.dir.dirids[1])
                } else {
                    /* 'east' to 'west' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[2], defdatarray.dir.dirids[2])
                }
            }
        case 3:   /* 'T' intersection */
            switch (index) {
                case 0:    /* 'east' to 'west' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[2], defdatarray.dir.dirids[2])
                case 1:    /* 'west' to 'east' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[0], defdatarray.dir.dirids[0])
                case 2:    /* 'south' to 'north' - rotated ids accounted for in switch above */
                    CREATE_APPROACH(defdatarray.id.laneids[3], defdatarray.dir.dirids[3])
                default:
                    printf("invalid index for default approach [%d %d]\n", spsCfg.numapproaches, index);
                    return MAP_NO_DIRECTION;
            }
        case 4:   /* all directions present */
            if (index > 3) {
                printf("invalid index for default approach [%d %d]\n", spsCfg.numapproaches, index);
                return MAP_NO_DIRECTION;
            }
            CREATE_APPROACH(defdatarray.id.laneids[index], defdatarray.dir.dirids[index])
    }

    return MAP_NO_DIRECTION;   /* should never be reached but may with invalid numapproaches */
}

/* decide to use map or cfg data and setup 
   initial intersection state */
static i2vReturnTypesT spsInitStateData(void)
{
    wuint8 i, numapproaches = 0;
    WBOOL ignoremap = WFALSE, validdata = WFALSE;
    mapShrIntDataT mapmem;

    spsintersection.approachlist = (spsApproachSignalState *)malloc(sizeof(spsApproachSignalState));
    if (NULL == spsintersection.approachlist) {
        I2V_ERR_LOG("failed to allocate memory for approaches\n");
        return I2V_RETURN_FAIL;
    } else {
        memset(spsintersection.approachlist, 0, sizeof(spsApproachSignalState));
        spsintersection.approachlist->firstRecord = spsintersection.approachlist;
    }
    spsintersection.statelist = (spsIntersectionObject *)malloc(sizeof(spsIntersectionObject));
    if (NULL == spsintersection.statelist) {
        I2V_ERR_LOG("failed to allocate memory for state list\n");
        return I2V_RETURN_FAIL;
    } else {
        memset(spsintersection.statelist, 0, sizeof(spsIntersectionObject));
        spsintersection.statelist->firstRecord = spsintersection.statelist;
    }

#if !defined(J2735_2016_03_UPER_TDF) //TM !!TODO!! MAP_GEN broken with new MAP definiton from 2016
    if (spsCfg.usemap) {
        if (!cfg.mapAppEnable) {
            ignoremap = WTRUE;
        }
        /* harvest map data - will possibly need to account for turn lanes */
        while (!validdata && !ignoremap) {
            WSU_SEM_LOCKR(&shmPtr->mapIntersectionData.h.ch_lock);
            if (shmPtr->mapIntersectionData.h.ch_data_valid) {
                memcpy(&mapmem, &shmPtr->mapIntersectionData, sizeof(mapmem));
                validdata = WTRUE;
            }
            WSU_SEM_UNLOCKR(&shmPtr->mapIntersectionData.h.ch_lock);
            if (mapmem.ignoreData) {
                I2V_DBG_LOG("map data unavailable - using default\n");
                ignoremap = WTRUE;
                validdata = WFALSE;
                break;
            }
            if (!validdata) {
                I2V_DBG_LOG("waiting for map data to be valid\n");
                usleep(50000);
            }
        }

        if (!ignoremap) {
            /* have map data now process it */
            for (i=0; i<MAX_APPROACH; i++) {
                if (mapmem.approachSet[i].approachID) {
                    /* valid approach */
                    numapproaches++;
                    if (i) {
                        /* need to allocate an approach record */
                        spsintersection.approachlist->nextRecord = 
                            (spsApproachSignalState *)malloc(sizeof(spsApproachSignalState));
                        if (NULL == spsintersection.approachlist->nextRecord) {
                            I2V_ERR_LOG("failed to allocate memory while creating approach record\n");
                            return I2V_RETURN_FAIL;
                        } else {
                            memset(spsintersection.approachlist->nextRecord, 0, sizeof(spsApproachSignalState));
                            spsintersection.approachlist->nextRecord->firstRecord = 
                                spsintersection.approachlist->firstRecord;
                            spsintersection.approachlist = spsintersection.approachlist->nextRecord;
                        }
                    }
                
                    spsintersection.approachlist->data.objID = OBJECT_ID_APPROACH;
                    spsintersection.approachlist->data.objSize = TOM_SIZE_APPROACH;
                    spsintersection.approachlist->data.apprID = mapmem.approachSet[i].approachID;
                    spsintersection.approachlist->data.close = htons(OBJECT_ID_APPROACH);

                    if (!spsInitStateDirection(mapmem.approachSet[i].approachID, mapmem.approachSet[i].directionVal, 
                      spsintersection.approachlist)) {
                        I2V_ERR_LOG("failed in updating intersection object record with approach\n");
                        return I2V_RETURN_FAIL;
                    }
                } else {
                   /* assume approachID = 0 means no more approaches */
                   if (!numapproaches) {
                       validdata = WFALSE;
                   }
                   break;
                }
            }
        }
    } 
#endif /* if !defined(J2735_2016_03_UPER_TDF) */

    if (!validdata) {
        for (i=0; i<spsCfg.numapproaches; i++) {
            /* hijacking 'numapproaches' it is used for direction value, the name is not indicative in this block of code */
            numapproaches = spsSetDefaultApproach(i);
            if (!spsInitStateDirection(spsintersection.approachlist->data.apprID, numapproaches,
              spsintersection.approachlist)) {
                I2V_ERR_LOG("failed in updating intersection object record with approach\n");
                return I2V_RETURN_FAIL;
            }
        }
    } else {
        /* update intersection id with map data */

#if defined(J2735_2016_03_UPER_TDF) //TM !!TODO!!
        spsCfg.intID         = 0;
        spsCfg.numapproaches = 0;
#else
        spsCfg.intID = mapmem.intersectionID;
        spsCfg.numapproaches = numapproaches;

#endif

    }

    /* MUST reset approachlist now */
    spsintersection.approachlist = spsintersection.approachlist->firstRecord;

    return I2V_RETURN_OK;
}

static void spsMainProcess(WBOOL firsttime)
{
/*
 * Editor's note: after writing most of this code the author realized that
 * the need to keep the approach records in memory wasn't necessary -
 * these can be freed after the main process executes first time - for now 
 * just keeping them.
 */
    struct s_tomhdr {
        wuint8 msgtype;
        wuint8 msgver;
        wuint16 length;
        wuint16 crc;
    } *tomhdr;
    spsApproachSignalState *apprrecord;
    spsIntersectionObject *intrecord;
    wuint8 i, j, count = 0, *dptr, *buf = NULL;
    wuint16 msgcrc;
    wuint32 timenow, sendsize;
#ifdef EXTRA_DEBUG
    wuint32 idx;
#endif /* EXTRA_DEBUG */
    i2vTimeT clock;

    i2vUtilGetUTCTime(&clock);
    timenow = CONVERT_TO_UTC(clock.year,clock.yday,clock.hour,clock.min,clock.sec);
    /* assuming if at this point at least one intersection object set */
    intrecord = spsintersection.statelist;

    if (firsttime) {
        /* set vals to be reused later when sending */
        spsmsgdata.objID = OBJECT_ID_LAYER;
        spsmsgdata.objSize = TOM_SIZE_SPAT_LAYER;
        spsmsgdata.layerType = htons(LAYER_TYPE_SPAT);
        spsmsgdata.layerID = OBJECT_ID_LAYER;
        spsmsgdata.content = CONTENT_SPAT_VERSION;
        spsmsgdata.intersection.objID = OBJECT_ID_INTERSECTION;
        spsmsgdata.intersection.objSize = TOM_SIZE_INTERSECTION;
        spsmsgdata.intersection.intID = htonl(spsCfg.intID);
        spsmsgdata.intersection.close = htons(OBJECT_ID_INTERSECTION);
        spsmsgdata.close = htons(OBJECT_ID_LAYER);

        for (i=0; i<spsintercount; i++) {
            /* index i=0 gets green, any other gets red */
            if (i) {
                /* red = index * (greendur + yellowdur + reddur[clearance] ) */
                intrecord->timeNext = i * (spsCfg.greendur + spsCfg.yeldur + spsCfg.reddur);
                intrecord->sigState = SIGNAL_RED;
            } else {
                /* green */
                intrecord->timeNext = spsCfg.greendur;
                intrecord->sigState = SIGNAL_GREEN;
            }
            for (j=0; j<intrecord->numids; j++) {
                apprrecord = intrecord->spssignal[j];
                apprrecord->data.timeNext = htons(intrecord->timeNext * 10);  /* tenths of seconds */
                apprrecord->data.sigstate = htons(intrecord->sigState);
                apprrecord->data.ignore = htons(spsCfg.yeldur);  /* setting this anyway */
                memcpy(&spsmsgdata.intersection.approaches[count++], &apprrecord->data, sizeof(spsApproach));
            }
            intrecord->lasttime = timenow;
            if ((count >= spsCfg.numapproaches) || (NULL == intrecord->nextRecord)) break;
            intrecord = intrecord->nextRecord;
        }

        return;   /* skip the rest - not needed */
    }

    /* normal path of operation */
    for (i=0; i<spsintercount; i++) {
        /* check if shift of state required */
#ifdef EXTRA_DEBUG
        printf("tn: %d td: %d (s: %d c: %d)\n", intrecord->timeNext, (timenow - intrecord->lasttime), intrecord->sigState, count);
#endif /* EXTRA_DEBUG */
        intrecord->timeNext -= (timenow - intrecord->lasttime);
        intrecord->lasttime = timenow;
        if (!intrecord->timeNext) {  /* shift state */
            switch (intrecord->sigState) {
                case SIGNAL_GREEN: 
                    intrecord->sigState = SIGNAL_YELLOW;
                    intrecord->timeNext = spsCfg.yeldur;
                    break;
                case SIGNAL_YELLOW:
                    intrecord->sigState = SIGNAL_RED;
                    intrecord->timeNext = (spsintercount - 1) * (spsCfg.greendur + spsCfg.yeldur + spsCfg.reddur);
                    break;
                case SIGNAL_RED: 
                    intrecord->sigState = SIGNAL_GREEN;
                    intrecord->timeNext = spsCfg.greendur;
                    break;
            }
        }
        
        /* update every approach record - they always get sent out */
        for (j=0; j<intrecord->numids; j++) {
            /* apprrecord = intrecord->spssignal[j];
            apprrecord->data.timeNext = htons(intrecord->timeNext * 10);
            apprrecord->data.sigstate = htons(intrecord->sigState); */

            /* see editor's note at the top */
            spsmsgdata.intersection.approaches[count].timeNext = htons(intrecord->timeNext * 10);  /* tenths of seconds */
            spsmsgdata.intersection.approaches[count++].sigstate = htons(intrecord->sigState);
        }
        intrecord = intrecord->nextRecord;
        if (NULL == intrecord) break;    /* should never be null */
    }

    /* can now send data - need to ONLY send approaches defined - not full approaches[MAX_APPROACH] data */
    sendsize = (sizeof(spsSpatLayer) - (MAX_APPROACH - spsCfg.numapproaches) * sizeof(spsApproach));
    sendsize += sizeof(struct s_tomhdr) + 1;   /* see TOM format outlined at the top */
    buf = (wuint8 *)malloc(sendsize);
    if (buf == NULL) {
        I2V_ERR_LOG("error sending spat data - allocation\n");
        return;
    }
    memset(buf, 0, sendsize);
    tomhdr = (struct s_tomhdr *)buf;
    /* see TOM format outlined at the top for these values */
    tomhdr->msgtype = buf[sendsize - 1] = 0xF1;
    tomhdr->msgver = 2;
    tomhdr->length = htons(sendsize);
    dptr = buf + sizeof(struct s_tomhdr);
    /* logic below: do not copy unused approach space in spsmsgdata - to avoid that copy first
       X number of bytes matching assigned approaches + intersection data and spat layer - this
       results in everything in sendsize minus tom header and footer and 2 close objects */
    memcpy(dptr, &spsmsgdata, (sendsize - (sizeof(struct s_tomhdr) + 1 + 2 * sizeof(wuint16))));
    /* now add omitted close objects */
    dptr = buf + (sendsize - (1 + 2 * sizeof(wuint16)));
    memcpy(dptr, &spsmsgdata.intersection.close, 2 * sizeof(wuint16));
    /* crc */
    msgcrc = i2vComputeCRC(buf, sendsize);
#ifdef EXTRA_DEBUG
    printf("sending spat msg: %d %#X\n", sendsize, msgcrc);
    for (idx=0; idx<sendsize; idx++) {
        printf("%.2x ", buf[idx]);
    }
    printf("\n");
#endif /* EXTRA_DEBUG */
    tomhdr->crc = htons(msgcrc);
    write(fd, buf, sendsize);
    free(buf);
}

/* free all allocated memory - in theory this will happen anyway
   when program exits - but be nice */
static void spsCleanUp(void)
{
    spsApproachSignalState *apprrecord;
    spsIntersectionObject *intrecord;

    while (WTRUE) {
        if (NULL == spsintersection.approachlist) {
            break;
        }
        apprrecord = spsintersection.approachlist->nextRecord;
        free(spsintersection.approachlist);
        spsintersection.approachlist = apprrecord;        
    }

    while(WTRUE) {
        if (NULL == spsintersection.statelist) {
            break;
        }
        intrecord = spsintersection.statelist->nextRecord;
        free(spsintersection.statelist);
        spsintersection.statelist = intrecord;        
    }
}

int main(void)
{
    struct sigaction sa;
    int ret = I2V_RETURN_UNKNOWN;
    wuint8 txrate = 1;

    memset(&spsCfg, 0, sizeof(spsCfg));
    memset(&spsmsgdata, 0, sizeof(spsmsgdata));
    memset(&spsintersection, 0, sizeof(spsintersection));
    
    shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shmPtr == NULL) {
        I2V_ERR_LOG("failed to init shared mem\n");
        return I2V_RETURN_FAIL;
    }
 
    if ((ret = spsUpdateCfg(SPS_CFG_FILE)) != I2V_RETURN_OK) {
        I2V_ERR_LOG("failed to load config\n");
        return I2V_RETURN_FAIL;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = spsSigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    txrate = (spsCfg.interval) ? spsCfg.interval : txrate;   /* protect against 0 */
    if (!cfg.scsAppEnable) {
        I2V_ERR_LOG("invalid config for signal simulator - scs not active - simulator will do nothing\n");
    }
    while (WTRUE) {
        /* wait until scs ready - otherwise queue open fails */
        WSU_SEM_LOCKR(&shmPtr->scsCfgData.h.ch_lock);
        if (shmPtr->scsCfgData.h.ch_data_valid) {
            WSU_SEM_UNLOCKR(&shmPtr->scsCfgData.h.ch_lock);
            break;
        }
        WSU_SEM_UNLOCKR(&shmPtr->scsCfgData.h.ch_lock);
        if (!mainloop) break;   /* this will get set if this loop takes a long time on i2v exit */
        I2V_DBG_LOG("waiting for scs...\n");
        usleep(100000);
    }
    if ((fd = open(I2V_SIM_SPAT_QUEUE, I2V_Q_SENDER)) < 0) {
        I2V_ERR_LOG("failed to open send queue: %s\n", strerror(errno));
        return I2V_RETURN_FAIL;
    }
    if (I2V_RETURN_OK != spsInitStateData()) {
        spsCleanUp();
        return I2V_RETURN_FAIL;
    }
    spsMainProcess(WTRUE);

    while(mainloop) {
        spsMainProcess(WFALSE);
        usleep(txrate * 100000);
        if (!mainloop) {
            break; 
        }
    }

    spsCleanUp();
    close(fd);

    wsu_share_detach(shmPtr);
    I2V_DBG_LOG("exiting signal controller simulator\n");

    return ret;
}

