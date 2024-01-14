/****************************************************************************
 *                                                                          *
 *  File Name: i2v_spat_cfgdata.h                                           *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/


#ifndef I2V_SPAT_CFGDATA_H
#define I2V_SPAT_CFGDATA_H

#include "dn_types.h"

/* spat struct names retained for integrating with existing code */
typedef struct {
    wsu_shm_hdr_t h;

    uint8_t       radioNum;
    bool_t        wsaEnable;
    uint8_t       wsaPriority;
    bool_t        hdrExtra;
    uint32_t      psid;
    uint8_t       channel_number;
    uint8_t       interopMode;
    bool_t        bsmEnable;
    uint32_t      bsmPsidPeruUnsecure;
    uint32_t      IntersectionID; /* Must match MAP being broadcast by AMH. Was in map16.conf but needed here. */
    /* TSCBM.CONF gets read into these five fields */
    uint8_t       numEntries;
    char_t        phaseType[SPAT_MAX_APPROACHES];      /* v, o, or p */
    uint8_t       phaseNumber[SPAT_MAX_APPROACHES];      /* 1 to 16 */
    uint8_t       signalGroupID[SPAT_MAX_APPROACHES]; /* 1 to 255: 0 = unknown, 255 = perm green: Do not support 255! */
    uint8_t       channelGreenType[SPAT_MAX_APPROACHES]; /* NTCIP 1202: protected or permitted. */
} spatCfgItemsT; /* Must have unique CfgItems to support different version of J2735 */


#endif /* I2V_SPAT_CFGDATA_H */
