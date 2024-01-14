/****************************************************************************
 *                                                                          *
 *  File Name: i2v_srm_rx_cfgdata.h                                         *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/


#ifndef I2V_SRM_RX_CFGDATA_H
#define I2V_SRM_RX_CFGDATA_H

typedef struct {
    wsu_shm_hdr_t h;

    uint8_t       radioNum;
    bool_t        secDebug;
    bool_t        hdrExtra;
    PSIDType      psid;
    PSIDType      j2735_psid;
    uint32_t      decode_method;
    uint32_t      asn1_decode_method;
    uint8_t       channel_number;

    bool_t        srmForward;

    uint32_t      srmTxVehBasicRole;
    uint8_t       srmPermissive;
    uint16_t      srmVodMsgVerifyCount;

} srmCfgItemsT;


#endif /* I2V_SRM_RX_CFGDATA_H */
