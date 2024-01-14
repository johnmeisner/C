/****************************************************************************
 *                                                                          *
 *  File Name: i2v_scs_data.h                                               *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_SCS_DATA_H
#define I2V_SCS_DATA_H

#include "wsu_shm_inc.h"
#include "i2v_general.h"
/* Big to fit all blob defintions for ICD, Tware... */
#define SPAT_BUF_SIZE         (2048)
#define GS2_BUF_SIZE          (2048)

#define SCS_SNMP_MAX_STR_LEN  I2V_CFG_MAX_STR_LEN

/* DO NOT REORDER unless updating the config file
   appropriately; see comment above data element
   localSignalControllerHW for more details */
enum supportedIndependentControllers {
    TrafficWare = 1,
    TrafficWareStrict = 2,
    TrafficWareFinal = 3,
    TrafficWarePromiscuous = TrafficWareFinal,   /* new entries for trafficware need to be aware of the final entry */
};

/* this union exists only to enable
   continued use of legacy code originally
   implemented in support of TOM formatted
   spat signal controller data */
typedef union {
/* Room for growth */
    i2vSPATDataType flexSpat;
} scsSpatInfo;

/* Metrics on spat16's performance */
typedef struct {
    uint32_t         max_num_groups;
    uint32_t         min_num_groups;
    uint32_t         cnt_empty_spat_fills;
    uint32_t         cnt_nonempty_spat_fills;
    uint32_t         cnt_spat_process_main_calls;
    uint32_t         cnt_asn1fill_spat_calls;
} scsMetricsT;

/* Copy of content of last j2735 message for i2vmonitor and webgui */
typedef struct {
    uint32_t         intersection_id;
    uint8_t          revision_counter;
    time_t           scs_process_time;
    uint16_t         scs_process_time_secs_in_hour;
    uint8_t          num_groups;
    uint8_t          signal_group_id[SPAT_MAX_APPROACHES];
    uint32_t         min_end_time[SPAT_MAX_APPROACHES];
    uint32_t         max_end_time[SPAT_MAX_APPROACHES];
    uint8_t          tsc_phase_number[SPAT_MAX_APPROACHES];
    uint8_t          tsc_phase_type[SPAT_MAX_APPROACHES];
    i2vSigPhases     signal_phase[SPAT_MAX_APPROACHES];
    uint8_t          channel_green_type[SPAT_MAX_APPROACHES];
    i2vMovementPhaseState prior_event_state[SPAT_MAX_APPROACHES];
    i2vMovementPhaseState final_event_state[SPAT_MAX_APPROACHES];
    uint16_t         TSCBMIntersectionStatus;
    uint16_t         IntersectionStatus;
} scsOutputT;

/* spat struct names retained for integrating with existing code */
typedef struct {
    wsu_shm_hdr_t h;

    wsu_gate_t       spatTriggerGate;
    uint8_t          spat_msg[SPAT_BUF_SIZE];
    uint32_t         spat_len;
    time_t           spat_parse_timestamp;       // When scs_app processed its data
    time_t           spat16_process_timestamp;   // When spat16_app processed its data
    bool_t            spatSelector;    /* WTRUE = flexSpat */
    scsSpatInfo      spat_info;
    uint64_t         packetCount;
    scsMetricsT      metrics;
    scsOutputT       output_copy;
} scsSpatShmDataT; 

/* these types relocated here for sharing */
#define EXPECTED_NUM_PHASES  0x10
typedef struct {
    uint16_t OverlapReds;
    uint16_t OverlapYels;
    uint16_t OverlapGrns;

} __attribute__((packed)) phaseOverlaps;

typedef struct{
    uint16_t  Phase2OvlpMask;     /* Phase to overlap map mask */
    uint16_t  Ovlp2PhaseMask;     /* Overlap to phase map mask */
    uint8_t   SignalGroup;        /* Which Sig Group gets the overlap or phase? */
} __attribute__((packed)) ovlpMapping;

typedef struct {
    uint16_t OverlapMask;    /* 16 bit Mask: PONM|LKJI|HGFE|DCBA */
    uint8_t  ovlpPhaseMapArray[EXPECTED_NUM_PHASES];
    ovlpMapping ovlpMappingArray[EXPECTED_NUM_PHASES];
    uint16_t FlashOutOvlpStatus;
    uint16_t FlashOutPhaseStatus;
} __attribute__((packed)) ovlGroupMap;

typedef struct {
    wsu_shm_hdr_t h;

    bool_t   ifcSCS;   /* enables or disables controller interface */
    bool_t   tomformat;
    bool_t   useMaxTTC;
    bool_t   useSnmpScs;
    bool_t   nontcip;
#ifdef GS2_SUPPORT
    bool_t   ifcGS2;
    char_t  localGS2IP[I2V_CFG_MAX_STR_LEN];
    uint16_t localGS2Port;
    uint32_t GS2DemoDebugLevel;
#endif
    bool_t   RSUHeartBeatEnable;
    uint16_t RSUHeartBeatInt;
    bool_t   SRMFwdTest;         /* SRM forward test mode */
    //Should be I2V_CFG_MAX_STR_LEN and no less
    char_t  SRMFile[128];       /* SRM playback file */
    bool_t   SRMFwdEnable;
    uint16_t SRMFwdInterval;
    char_t  SRMFwdIP[I2V_CFG_MAX_STR_LEN];
    uint16_t SRMFwdPort;
    uint8_t  SRMFwdInterface;
    bool_t   requireTTI;
    char_t  localSignalControllerIP[I2V_CFG_MAX_STR_LEN];
    uint16_t localSignalControllerPort;
    uint16_t snmpPort;
    uint32_t spat_intersection_id;
    uint8_t  staticyeldur;
    /* JJG: it would be better to have used tomformat field rather than add an extra field;
    but I didn't want to rework and possibly break a value that's been working for years and
    is overloaded; the purpose of this variable is to identify the type of controller but
    the value is not always needed; it is needed only for new controllers with non ICD formats
    such as TrafficWare which is set to 1 */
    uint8_t  localSignalControllerHW;   /* controller type */

    char_t  enable_spat_val[I2V_CFG_MAX_STR_LEN];  /* value is a string but can only be 0 - 6 ascii */
    char_t  enable_spat[SCS_SNMP_MAX_STR_LEN];    /* snmp command */

    char_t  yellowDurOID[SCS_SNMP_MAX_STR_LEN];
    char_t  phaseOptsOID[SCS_SNMP_MAX_STR_LEN];
    char_t  phaseGrnOID[SCS_SNMP_MAX_STR_LEN];
    char_t  phaseYlwOID[SCS_SNMP_MAX_STR_LEN];
#if 0   /* see icddecoder.c for comment on why this is removed */
    char_t  phaseRedOID[SCS_SNMP_MAX_STR_LEN];
#endif /* if 0 */
    char_t  phaseNumOID[SCS_SNMP_MAX_STR_LEN];
    char_t  phaseMaxCntdnOID[SCS_SNMP_MAX_STR_LEN];
    char_t  phaseMinCntdnOID[SCS_SNMP_MAX_STR_LEN];
} scsCfgItemsT;

#endif /* I2V_SCS_DATA_H */
