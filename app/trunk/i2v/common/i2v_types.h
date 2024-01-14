/**************************************************************************
 *                                                                        *
 *     File Name: i2v_types.h                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#ifndef I2V_TYPES_H
#define I2V_TYPES_H
#include "dn_types.h"
#define LMGR_IFC_REQ_CMD  255

#define I2V_NUM_SUPPORTED_IFCS 6 /* Should be in line with ntcip-1218.h */

/* 16 phases and 16 overlaps. */
#define SPAT_MAX_APPROACHES  32

/* for log manager */
typedef enum {
    IFC_UNKNOWN,   /* do not remove this entry */
    IFC_ON,
    IFC_OFF,
} __attribute__((packed)) lmgrIfcReqState;


/* supported list of system interfaces - 
   add to this list for more radios */
/* log manager is hard coded for only 4 ifcs
   - if adding more radios - you need to update
   logmgr to add support for logging those ifcs */
typedef enum {
    PHY_NONE,
    PHY_ETH0,
    PHY_ETH1,  /* Room for one more. Not used. */
    PHY_DSRC0,
    PHY_DSRC1, /* Alt mode only. */
    PHY_CV2X0,
    PHY_CV2X1,  /* Room for one more. Not used. */
} __attribute__((packed)) i2vIfcList;


/* these values are used as masks
20140905: and now used by more than amh */
typedef enum {
    AMH_MSG_UNKNOWN_MASK = 0x0,
    AMH_MSG_MAP_MASK     = 0x1,
    AMH_MSG_SPAT_MASK    = 0x2,
    AMH_MSG_CSR_MASK     = 0x4,
    AMH_MSG_EVA_MASK     = 0x8,
    AMH_MSG_ICA_MASK     = 0x10,
    AMH_MSG_NMEA_MASK    = 0x20,
    AMH_MSG_PDM_MASK     = 0x40,
    AMH_MSG_PVD_MASK     = 0x80,
    AMH_MSG_RSA_MASK     = 0x100,
    AMH_MSG_RTCM_MASK    = 0x200,
    AMH_MSG_SRM_MASK     = 0x400,
    AMH_MSG_SSM_MASK     = 0x800,
    AMH_MSG_TIM_MASK     = 0x1000,
    AMH_MSG_PSM_MASK     = 0x2000,
    AMH_MSG_TEST00_MASK  = 0x4000,
    AMH_MSG_TEST01_MASK  = 0x8000,
    AMH_MSG_TEST02_MASK  = 0x10000,
    AMH_MSG_TEST03_MASK  = 0x20000,
    AMH_MSG_TEST04_MASK  = 0x40000,
    AMH_MSG_TEST05_MASK  = 0x80000,
    AMH_MSG_TEST06_MASK  = 0x100000,
    AMH_MSG_TEST07_MASK  = 0x200000,
    AMH_MSG_TEST08_MASK  = 0x400000,
    AMH_MSG_TEST09_MASK  = 0x800000,
    AMH_MSG_TEST10_MASK  = 0x1000000,
    AMH_MSG_TEST11_MASK  = 0x2000000,
    AMH_MSG_TEST12_MASK  = 0x4000000,
    AMH_MSG_TEST13_MASK  = 0x8000000,
    AMH_MSG_TEST14_MASK  = 0x10000000,
    AMH_MSG_TEST15_MASK  = 0x20000000,
    AMH_MSG_BSM_MASK     = 0x40000000,
    AMH_MSG_RESERVED1_MASK  = 0x80000000 /* RESERVED not used */
} amhBitmaskType;

/* 
 * ==>>> Be kind <<<=== Keep these uptodate if you change amhBitmaskType. 
 *
 * Make first(bottom) and last(top) of message you want amc to send. AMC relies on these!! 
 *
 */
#define AMH_MSG_BOTTOM_MASK  AMH_MSG_MAP_MASK
#define AMH_MSG_TOP_MASK     AMH_MSG_BSM_MASK

/* this bitmask is defined in other header files but added here
   for future extension in i2v where all types are i2v types */

/* This is different than Table 9 of spat_info.pdf?*/
typedef enum {
    SIG_PHASE_DARK             = 0x00000000,
    SIG_PHASE_GREEN            = 0x00000001,
    SIG_PHASE_YELLOW           = 0x00000002,
    SIG_PHASE_RED              = 0x00000004,
    SIG_PHASE_FLASHING_GREEN   = 0x00000008,
    SIG_PHASE_FLASHING_YELLOW  = 0x00000010,
    SIG_PHASE_FLASHING_RED     = 0x00000020,
    SIG_PHASE_UNKNOWN          = 0xFFFFFFFF,
} i2vSigPhases;

/* This is *slightly* different than DSRC.h? */
typedef enum {
    MPS_UNAVAILABLE                 = 0,    // Unknown or error
    MPS_DARK                        = 1,    // Unlit
    MPS_STOP_THEN_PROCEED           = 2,    // flashing red
    MPS_STOP_AND_REMAIN             = 3,    // Red light
    MPS_PRE_MOVEMENT                = 4,    // Red+yellow (Not used in US)
    MPS_PERMISSIVE_MOVEMENT_ALLOWED = 5,    // Flashing yellow (future: Permissive green?)
    MPS_PROTECTED_MOVEMENT_ALLOWED  = 6,    // Protected green
    MPS_PERMISSIVE_CLEARENCE        = 7,    // Permissive yellow
    MPS_PROTECTED_CLEARENCE         = 8,    // Protected yellow
    MPS_CAUTION_CONFLICTING_TRAFFIC = 9     // Flashing yellow
} i2vMovementPhaseState;



/* i2vIntersectionStatus
 *
 *#define IntersectionStatusObject_manualControlIsEnabled          0    // MS-bit
 *#define IntersectionStatusObject_stopTimeIsActivated             1
 *#define IntersectionStatusObject_failureFlash                    2
 *#define IntersectionStatusObject_preemptIsActive                 3
 *#define IntersectionStatusObject_signalPriorityIsActive          4
 *#define IntersectionStatusObject_fixedTimeOperation              5
 *#define IntersectionStatusObject_trafficDependentOperation       6
 *#define IntersectionStatusObject_standbyOperation                7
 *#define IntersectionStatusObject_failureMode                     8
 *#define IntersectionStatusObject_off                             9
 *#define IntersectionStatusObject_recentMAPmessageUpdate          10
 *#define IntersectionStatusObject_recentChangeInMAPassignedLanesIDsUsed 11
 *#define IntersectionStatusObject_noValidMAPisAvailableAtThisTime 12
 *#define IntersectionStatusObject_noValidSPATisAvailableAtThisTime 13
*/

typedef enum {
    MANUAL_CONTROL_IS_ENABLED                       = 0x8000,  // 0x8000 >> IntersectionStatusObject_manualControlIsEnabled
    STOP_TIME_IS_ACTIVATED                          = 0x4000,  // 0x8000 >> IntersectionStatusObject_stopTimeIsActivated 
    FAILURE_FLASH                                   = 0x2000,  // ...
    PREEMPT_IS_ACTIVE                               = 0x1000, 
    SIGNAL_PRIORITY_IS_ACTIVE                       = 0x0800, 
    FIXED_TIME_OPERATION                            = 0x0400,
    TRAFFIC_DEPENDENT_OPERATION                     = 0x0200,
    STANDBY_OPERATION                               = 0x0100,
    FAILURE_MODE                                    = 0x0080,
    CONTROLLER_IS_OFF                               = 0x0040,
    RECENT_MAP_MESSAGE_UPDATE                       = 0x0020,
    RECENT_CHANGE_IN_MAP_ASSIGNED_LANES_IDS_USED    = 0x0010,
    NO_VALID_MAP_IS_AVAIL_AT_THIS_TIME              = 0x0008,
    NO_VALID_SPAT_IS_AVAIL_AT_THIS_TIME             = 0x0004,
    // RESERVED1                                    = 0x0002,
    // RESERVED2                                    = 0x0001
}i2vIntersectionStatus;


/* DO NOT REMOVE COMMENT BELOW!!! */
/* do not add any 'wuint' items */
typedef struct {
    uint8_t          approachID;
    uint32_t         curSigPhase;   /* bit mask of signal phases */
    uint8_t          confidence;    /* countdown timer confidence only */
    uint16_t         timeNextPhase;
    uint16_t         secondaryTimeNextPhase;    /* JJG: for compatibility */
    uint8_t          yellowDur;
    uint8_t          pedPresent;
} __attribute__((packed)) i2vSPATApproachType;

typedef struct {
    uint32_t              intID;
    uint8_t               numApproach;
    int32_t               intersectionStatus; /*TM TODO Optional but useful...maybe?*/
    i2vSPATApproachType   spatApproach[SPAT_MAX_APPROACHES];
} __attribute__((packed)) i2vSPATDataType;


#endif /* I2V_TYPES_H */
