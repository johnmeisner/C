/****************************************************************************
 *                                                                          *
 *  File Name: i2v_amh_data.h                                               *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *         Rutherford road                                                  *
 *         Carlsbad, CA 92008                                               *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_AMH_DATA_H
#define I2V_AMH_DATA_H
typedef struct {
    wsu_shm_hdr_t h;
    uint8_t radioNum;
    uint8_t  channelNum;
    bool_t   validateData;
    bool_t   EnableSAR; /*Store & Repeat*/
    uint16_t logRate;
    bool_t   bcastGeneric;
    bool_t   psidMatch;
    bool_t   lastChanceSend;
    bool_t   hdrExtra;
    /* imf items */
    bool_t   enableImf;
    bool_t   restrictIP;
    char_t   imfIP[I2V_CFG_MAX_STR_LEN];
    uint16_t imfPort;
    uint32_t amhFwdEnable; /* Mask based off amhBitmaskType */
    uint8_t  amhIntervalSelect;
} __attribute__((packed))  amhCfgItemsT;

typedef struct {
    wsu_shm_hdr_t h;
    bool_t        newmsg;
    uint8_t       data[MAX_WSM_DATA];
    uint16_t      count;
} amh2GenericMsgDataT;
#endif /* I2V_AMH_DATA_H */
