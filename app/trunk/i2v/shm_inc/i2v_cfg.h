/****************************************************************************
 *                                                                          *
 *  File Name: i2v_cfg.h                                                    *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_CFG_H
#define I2V_CFG_H

#include "wsu_shm_inc.h"
#include "dn_types.h"
#include "i2v_general.h"

typedef struct {
    bool_t     radioEnable;
    bool_t     radioMode;        /* probably bad name - true = continuous mode */
    uint8_t    radioChannel;
    int32_t    msgListener;      /* msg queue identifier */
    pid_t      pid;
} iwmhVars;

typedef struct {
    wsu_shm_hdr_t h;
    wsu_gate_t     iwmhGate;     /* one time usage if iwmh enabled */
    
    /* global config items */
    char_t     config_directory[I2V_CFG_MAX_STR_LEN];
    char_t     config_filename[I2V_CFG_MAX_STR_LEN];
    char_t     config_cmn_dbglog_dir[I2V_CFG_MAX_STR_LEN];
    char_t     config_cmn_data_dir[I2V_CFG_MAX_STR_LEN];
    char_t     config_cmn_applog_dir[I2V_CFG_MAX_STR_LEN];
    char_t     override_cfg_filename[I2V_CFG_MAX_STR_LEN];
    char_t     securityDir[I2V_CFG_MAX_STR_LEN];
    char_t     overrideTime[I2V_TIME_MAX_STR_LEN];
    char_t     i2vipv6addr[I2V_IP_MAX_STR_LEN];
    char_t     i2vipv6gwaddr[I2V_IP_MAX_STR_LEN];
    uint16_t   i2vrsuID;
    bool_t      override_cfg_enable;

    /* app enable flags */
    bool_t      scsAppEnable;  /* enables spat */
    bool_t      srmAppEnable;
    bool_t      spatAppEnable; 
    bool_t      mapAppEnable; 
    bool_t      timAppEnable; 
    bool_t      ipbAppEnable; 
    bool_t      amhAppEnable; 
    bool_t      iwmhAppEnable; 
    bool_t      rtcmAppEnable; 
#ifdef RSE_SIMULATOR
    bool_t      spsAppEnable;
#endif /* RSE_SIMULATOR */
    bool_t      fwdmsgAppEnable; 

    /* log enable flags */
    bool_t      globalDebugFlag; 
    bool_t      globalApplogsEnableFlag;
    bool_t      i2vDebugEnableFlag;
    bool_t      i2vApplogEnableFlag;

    /* gps override */
    bool_t      enableGPSOverride;
    float64_t     gpsOvrLat;
    float64_t     gpsOvrLong;
    int16_t     gpsOvrElev;

    /* time override */
    bool_t      enableTimeOverride;
    uint8_t     cfgTimeMode;
    bool_t      tmgrRunning;

    /* network settings */
    bool_t      ethIPv6Enable;
    uint8_t     ethIPv6Scope;

    /* i2v settings */
    bool_t      termOnChildExit;
    bool_t      bcastFB;          /* when iwmh enabled, fallback flag if iwmh fails */
    bool_t      bcastTimeShift;
    bool_t      wsaEnable;
    uint8_t     cchradio;         /* used by iwmh only */
    bool_t      iwmhVerifyOD;     
    int32_t     iwmhSecQ;         
    iwmhVars   radio0wmh;        /* do NOT insert anything between these fields */
    iwmhVars   radio1wmh;        /*     |   |   |   */
    iwmhVars   radio2wmh;        /*     |   |   |   */
    iwmhVars   radio3wmh;        /*     V   V   V   */
    bool_t      bcastLockStep;
    uint8_t     uchannel;
    uint8_t     uradioNum; 
    uint8_t     uwsaPriority;
    float32_t      txPwrLevel;      /* 0 - 23; 0.5 dBm increments; make sure decimal is either 0 or 5 */
    bool_t      security;
    bool_t      vodEnable;
    uint32_t    securityVtPMsgRateMs;
    uint8_t     secStartTO;
    uint8_t     certAttach;
    bool_t      dfltCert;
    bool_t      regenCert;
    uint32_t    secSpatPsid;
    uint32_t    secMapPsid;
    uint32_t    secTimPsid;
    uint32_t    secSpat16Psid;
    uint32_t    secMap16Psid;
    uint32_t    secTim16Psid;
    uint32_t    secIpbPsid;
    uint32_t    bsmPsidDer;
    uint32_t    bsmPsidPerA;
    uint32_t    bsmPsidPerU;
    uint32_t    srmPsid;
    bool_t      nocflash;
    bool_t      nosyslog;
    bool_t      limitconsoleprints;
#ifdef HSM
    bool_t      spatSspEnable;
    bool_t      spatBitmappedSspEnable;
    char_t      spatSsp[MAX_SSP_STRING_LEN];
    char_t      spatSspMask[MAX_SSP_STRING_LEN];
    bool_t      mapSspEnable;
    bool_t      mapBitmappedSspEnable;
    char_t      mapSsp[MAX_SSP_STRING_LEN];
    char_t      mapSspMask[MAX_SSP_STRING_LEN];
    bool_t      timSspEnable;
    bool_t      timBitmappedSspEnable;
    char_t      timSsp[MAX_SSP_STRING_LEN];
    char_t      timSspMask[MAX_SSP_STRING_LEN];
#endif /* HSM */
    bool_t      enableASCSNMP;

    /* DSRC = 0, CV2X = 1 */
    uint32_t    RadioType;

    /* Interface Logging: Only works if (_nosyslog == WFALSE).
     *     none  = 0 
     *     eth0  = 1
     *     eth1  = 2
     *     dsrc0 = 3
     *     dsrc1 = 4
     *     cv2x0 = 5
     *     cv2x1 = 6
     *
     * For max simultaneous interface logs see logmgr.c::MAX_CHILD_PROCS
     */
    uint8_t     InterfaceLogMask;
    bool_t      i2vUseDeviceID;
    bool_t      i2vDeviceIDLoaded;    /* this does not go into the i2v.conf file */
    char_t      i2vDeviceID[I2VRSUIDSIZE];    /* this does not go into the i2v.conf file */
    bool_t      gpsCtlTx;
} cfgItemsT;

#endif /* I2V_CFG_H */
