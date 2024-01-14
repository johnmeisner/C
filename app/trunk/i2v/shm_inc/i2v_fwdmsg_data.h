/****************************************************************************
 *                                                                          *
 *  File Name: i2v_fwdmsg_data.h                                            *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_FWDMSG_DATA_H
#define I2V_FWDMSG_DATA_H

#define MAX_FWD_MSG_IP 5
#define MAX_FWD_MSG_DATA 2302

typedef enum { 
    BSM = 0x01,
    MAP = 0x02,
    PSM = 0x04,
    SPAT16 = 0x08,
    AMH = 0x10,
    SRM = 0x20,
    ALL = 0xFF,             /* Force enum is only a byte */
}fwdmsgTypeE;

typedef struct {
    fwdmsgTypeE fwdmsgType;
    uint16_t fwdmsgDataLen;
    uint8_t  fwdmsgData[MAX_FWD_MSG_DATA];
}__attribute__((packed)) fwdmsgData;

typedef struct {
    bool_t   fwdmsgEnable;
    char_t   fwdmsgFwdIp[I2V_CFG_MAX_STR_LEN];
    uint16_t fwdmsgFwdPort;
    uint8_t  fwdmsgMask;                         /* Mask for type of messages */
                                                /* 0000 0000    ==> None */
                                                /* 0000 0001    ==> BSM */
                                                /* 0000 0010    ==> MAP */
                                                /* 0000 0100    ==> PSM */
                                                /* 0000 1000    ==> SPaT16 */
                                                /* 0001 0000    ==> AMH */
                                                /* 0010 0000    ==> SRM */
}forwardMessageAddr;

typedef struct {
    wsu_shm_hdr_t h;
    forwardMessageAddr fwdmsgAddr[MAX_FWD_MSG_IP];
} fwdmsgCfgItemsT;

#endif /* I2V_FWDMSG_DATA_H */
