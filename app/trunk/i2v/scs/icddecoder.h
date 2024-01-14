/***************************************************
 *                                                 *
 *     File Name: icddecoder.h                     *
 *     Author:    DENSO Int. America, Inc.         *
 *                Carlsbad, CA USA                 *
 *                                                 *
 ***************************************************
 * (C) Copyright 2021 DENSO Int. America, Inc.     *
 *      All rights reserved.                       *
 ***************************************************/
#ifndef ICDDECODER_H
#define ICDDECODER_H


#include "dn_types.h"


/* this is defined in the Battelle ICD as the first byte */

/* For Reference:
  See 
    "Table 4. Traffic Signal Controller Broadcast Message" 
  of
    "Interface Control Docuement For The Signal Phase and Timing and Related Messages For V-I Applications"

    "Document No. 60606-18A"
*/

#define ICD_SPAT_TYPE         0xcd
#define CMD_TYPE_INDEX        0

#define NUM_PHASES_INDEX      1

#define FIRST_PHASE_OFFSET    2

/* 1 byte: table 5 */

#define PHASE_GROUP_OFFSET              (210)

  /*
   *bitmask
   */
  #define SPAT_STATUS_MANUAL_CONTROL_ENABLE_ACTIVE_MASK (0x80)
  #define SPAT_STATUS_STOP_TIME_ACTIVE_MASK             (0x40)
  #define SPAT_STATUS_FAULT_FLASH_MASK                  (0x20)
  #define SPAT_STATUS_PREEMPT_ACTIVE_MASK               (0x10)
  #define SPAT_STATUS_TSP_ACTIVE_MASK                   (0x08)
  #define SPAT_STATUS_COORDINATION_ACTIVE_MASK          (0x04)
  #define SPAT_STATUS_COORDINATION_IN_TRANSITION_MASK   (0x02)
  #define SPAT_STATUS_PROGRAMMED_FLASH_ACTIVE_MASK      (0x01)
                                                     
#define PHASE_GROUP_PEDESTRIAN_OFFSET         (216) //Not implemented
#define PHASE_OVERLAP_OFFSET                  (222) //Not implemented                                                     
                                                     
#define SPAT_INTERSECTION_STATUS_OFFSET       (232)


#define SPAT_DISCONTINOUS_CHANGE_FLAG_OFFSET (233) // 1 byte: upper 5 bits message version

#define SPAT_MESSAGE_SEQUENCE_COUNTER_OFFSET (234) // 1 byte: lower byte of controller up-time

#define SPAT_TIMESTAMP_OFFSET                (235) // 5 bytes, 4 bytes(seconds since 1970) 1 byte(1/10th of a second)

#define SPAT_PEDESTRIAN_CALL_OFFSET          (240) // 2 bytes, bit-mapped per phase 1-16
#define SPAT_PEDESTRIAN_DETECT_OFFSET        (242) // 2 bytes, bit-mapped per phase 1-16


/* J2735 values */
#define STATE_CONFIDENCE_MIN_TIME   1
#define STATE_CONFIDENCE_MAX_TIME   2


#define SPAT_TIME_TO_CHANGE_OFFSET (194)

#define SPAT_PHASE_WALKS_OFFSET (216)

#define SPAT_PHASE_OVERLAP_OFFSET (222) //red
//#define SPAT_PHASE_OVERLAP_OFFSET (224) //yellow
//#define SPAT_PHASE_OVERLAP_OFFSET (226) //green

#define FLASH_OUT_PHASE_STATUS_OFFSET        (228)      /* 2 bytes bitmap for phases 1-16 */
#define FLASH_OUT_OVLP_STATUS_OFFSET         (230)      /* 2 bytes bitmap for overlaps 1-16 */

#define SPAT_PHASE_PED_CALL_OFFSET (240)


#define SPAT_PHASE_PED_DETECT_OFFSET (242)


typedef struct {
    uint8_t  phaseNum;
    uint16_t vehMinTimeChange;
    uint16_t vehMaxTimeChange;
    uint16_t pedMinTimeChange;
    uint16_t pedMaxTimeChange;
    uint16_t ovlpMinTimeChange;
    uint16_t ovlpMaxTimeChange;
} __attribute__((packed)) spatPhase;

typedef struct {
    uint16_t groupReds;
    uint16_t groupYels;
    uint16_t groupGrns;
} __attribute__((packed)) phaseGroups;

typedef struct {
    uint16_t groupDontWalk;
    uint16_t groupPedsClear;
    uint16_t groupWalk;
} __attribute__((packed))  phaseWalks;

i2vReturnTypesT icdParseSpat(i2vSPATDataType *dest, uint8_t *buf, bool_t useMaxTTL);
i2vReturnTypesT icdParseSNMPSpat(i2vSPATDataType *dest, scsCfgItemsT *cfg);
bool_t icdGetYelDur(char_t *hostip, uint16_t port, char_t *oid);
bool_t icdGetPhaseEnable(char_t *hostip, uint16_t port, char_t *oid, uint16_t *phaseMask);
bool_t icdGetOvlpAttr(scsCfgItemsT *cfg);

#endif /* ICDDECODER_H */