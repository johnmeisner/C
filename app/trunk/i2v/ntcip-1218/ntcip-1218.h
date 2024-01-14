/**************************************************************************
 *                                                                        *
 *     File Name:  ntcip-1218.h                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *

 **************************************************************************/
#ifndef NTCIP_1218_H
#define NTCIP_1218_H

/*
 * Description:
 *
 * rsu MODULE-IDENTITY
 *     LAST-UPDATED     "202009040000Z"
 *     DESCRIPTION      "This MIB defines the Roadside Unit (RSU) Objects"
 *     REVISION         "202009040000Z"
 */

/* MIB */
#define RSU_MIB_VERSION "202009040000Z" /* NTCIP-1218 */

/* DENSO */
#define DN_MIB_VERSION         0x2  /* Internal version control. Change ntcip_1218_mib_t please increment. */
#define DN_RESERVED_BLOCK_MAX  64   /* 20 crc32s for all major OIDs 5.2 - 5.21 plus 44 spare for anything forgotten. */

/* Radio */
#define RSU_RADIO_MAC_LENGTH    6

/* For PSIDs: From rs.h */
#define RSU_RADIO_PSID_SIZE      4 /* https://standards.ieee.org/products-services/regauth/psid/public.html */
#define RSU_RADIO_PSID_MIN_SIZE  1
#define PSID_ENCODE_MASK_1BYTE 0x0       /* PSID 1 bytes (bit 7) 0 (bit 6) 0 (bit 5) 0 (bit 4)0 */
#define PSID_ENCODE_MASK_2BYTE 0x80      /* PSID 2 bytes (bit 7) 1 (bit 6) 0 (bit 5) x (bit 4)x */
#define PSID_ENCODE_MASK_3BYTE 0xC0      /* PSID 3 bytes (bit 7) 1 (bit 6) 1 (bit 5) 0 (bit 4)x */
#define PSID_ENCODE_MASK_4BYTE 0xE0      /* PSID 4 bytes (bit 7) 1 (bit 6) 1 (bit 5) 1 (bit 4)0 */
#define PSID_1BYTE_MAX_VALUE  0x7F       /* max value for 1 byte encoding  (0  encoding) */
#define PSID_2BYTE_MIN_VALUE  0x8000     /* min value for 2 byte encoding  (10 encoding) */
#define PSID_2BYTE_MAX_VALUE  0xBFFF     /* max value for 2 byte encoding  (10 encoding) */
#define PSID_3BYTE_MIN_VALUE  0xC00000   /* min value for 3 byte encoding (110 encoding) */
#define PSID_3BYTE_MAX_VALUE  0xDFFFFF   /* max value for 3 byte encoding (110 encoding) */
#define PSID_4BYTE_MIN_VALUE  0xE0000000 /* min value for 4 byte encoding (1110 encoding) */
#define PSID_4BYTE_MAX_VALUE  0xEFFFFFFF /* max value for 4 byte encoding (1110 encoding) */

/* Network */
#define URI255_LENGTH_MIN     0  /* From supporting MIBs . */
#define URI255_LENGTH_MAX     255

#define URI1024_LENGTH_MIN    0
#define URI1024_LENGTH_MAX    1024

#define RSU_DEST_PORT_MIN     0
#define RSU_DEST_PORT_MAX     65535

#define RSU_DEST_IP_MIN       0 /* IPv4 and IPv6 */
#define RSU_DEST_IP_MAX       64

/* Date & Time. */
#define RFC2579_DATEANDTIME_LENGTH 8    /* fixed length RFC2579 */
#define MIB_DATEANDTIME_LENGTH     8    /* UTC'ish: 8 OCTETS: DateAndTime: SNMPv2-TC */

/* Location */
#define RSU_LAT_MIN     -900000000 /* degrees scaled to integer 10^-7. */
#define RSU_LAT_UNKNOWN 900000001
#define RSU_LAT_MAX     RSU_LAT_UNKNOWN

#define RSU_LON_MIN     -1800000000 /* degrees scaled to integer 10^-7. */
#define RSU_LON_UNKNOWN 1800000001 
#define RSU_LON_MAX     RSU_LON_UNKNOWN 

#define RSU_ELV_MIN     -100000    /* cm */
#define RSU_ELV_UNKNOWN 1000001 
#define RSU_ELV_MAX     RSU_ELV_UNKNOWN

/* SNMP Table Row Status: See snmp.h. */
#define ROW_STATUS_VALID_MIN           SNMP_ROW_NONEXISTENT
#define ROW_STATUS_VALID_MAX           SNMP_ROW_DESTROY
#define ROW_STATUS_VALID_DEFAULT       ROW_STATUS_VALID_MIN

typedef enum {
    mib_off = 0,
    mib_on  = 1, 
} onOff_e;

typedef enum {
    protocol_other = 1,
    protocol_udp   = 2,
} protocol_e;

typedef enum {
    inboundOnly  = 1, /* in file only. */
    outboundOnly = 2, /* out file only. */
    biSeparate   = 3, /* in and out files. */
    biCombined   = 4, /* in and out combined in one file. */
} ifclogDir_e;

/****************************************************************************** 
 * 5.2.1 - 5.2.2: RSU Radios: { rsu 1 }
 *   int32_t          maxRsuRadios;                      RO
 *   RsuRadioEntry_t  rsuRadioTable[RSU_RADIOS_MAX];
 *
 *  typedef struct  {
 *    int32_t rsuRadioIndex;                             RO : 1 to 16(RSU_RADIOS_MAX)
 *    uint8_t  rsuRadioDesc[RSU_RADIO_DESC_MAX];         R/W: 0 to 144
 *    onOff_e rsuRadioEnable;                            R/W: 0ff == 0, On == 1
 *    int32_t rsuRadioType;                              R0 : 1 == other, 2 == dsrc, 3 == pC5
 *    uint8_t rsuRadioMacAddress1[RSU_RADIO_MAC_LENGTH]; RO : radio A(0). If unknown set to all 1's
 *    uint8_t rsuRadioMacAddress2[RSU_RADIO_MAC_LENGTH]; RO : radio B(1). If unknown set to all 1's. ALT mode only.
 *    int32_t rsuRadioChanMode;                          R/W: other == 1, 2 == uknown, 3 == cont, 4 == alt, 5 == immediate
 *    int32_t rsuRadioCh1;                               R/W: DSRC is 172 to 184. For pC5?
 *    int32_t rsuRadioCh2;                               R/W: DSRC is 172 to 184. For pC5?
 *    int32_t rsuRadioTxPower1;                          R/W: Dbm (-128..127): Default is -128
 *    int32_t rsuRadioTxPower2;                          R/W: Dbm (-128..127): Default is -128
 * } RsuRadioEntry_t;
 *
 ******************************************************************************/

/* 5.2.1: 1 DSRC radio and 1 CV2X = 2 radios max. Not to be confused with Radio A and Radio B of each radio. */
#if defined (PLATFORM_HD_RSU_5940) /* 5912 has both dsrc & cv2x. 5940 only cv2x. */
#define RSU_CV2X_RADIO      1
#define RSU_RADIOS_MIN      1
#define RSU_RADIOS_MAX      1
#else
#define RSU_DSRC_RADIO      1
#define RSU_CV2X_RADIO      2
#define RSU_RADIOS_MAX      2
#endif
/* 5.2.2 */
#define RSU_RADIO_DESC_MIN  0
#define RSU_RADIO_DESC_MAX  144

#define RSU_RADIO_ENABLE_OFF 0
#define RSU_RADIO_ENABLE_ON  1

#define RSU_RADIO_TYPE_MIN   1
#define RSU_RADIO_TYPE_MAX   3

#define RSU_RADIO_TYPE_OTHER 1
#define RSU_RADIO_TYPE_DSRC  2
#define RSU_RADIO_TYPE_PC3   3

#define RSU_RADIO_TYPE_MODE_OTHER    1
#define RSU_RADIO_TYPE_MODE_UNKNOWN  2
#define RSU_RADIO_TYPE_MODE_CONT     3 /* Only Mode Radios support. */
#define RSU_RADIO_TYPE_MODE_ALT      4
#define RSU_RADIO_TYPE_MODE_IMMEDATE 5

#define RSU_CV2X_CHANNEL_DEFAULT  183 /* RO for CV2X. */

#if defined (PLATFORM_HD_RSU_5940)
#define RSU_RADIO_CHANNEL_MIN  RSU_CV2X_CHANNEL_DEFAULT
#define RSU_RADIO_CHANNEL_MAX  RSU_CV2X_CHANNEL_DEFAULT
#else
#define RSU_RADIO_CHANNEL_MIN  172
#define RSU_RADIO_CHANNEL_MAX  184
#endif

/* IEEE defintions. */
#define RSU_RADIO_TX_POWER_MIN 0
#define RSU_RADIO_TX_POWER_MAX 23 /* default */

/* 1218 defintions.
 * #define RSU_RADIO_TX_POWER_MIN -128
 * #define RSU_RADIO_TX_POWER_MAX  127
 */

typedef struct  {
    int32_t rsuRadioIndex;
    uint8_t rsuRadioDesc[RSU_RADIO_DESC_MAX];
    int32_t rsuRadioDesc_length;
    onOff_e rsuRadioEnable;
    int32_t rsuRadioType;                              
    uint8_t rsuRadioMacAddress1[RSU_RADIO_MAC_LENGTH]; 
    uint8_t rsuRadioMacAddress2[RSU_RADIO_MAC_LENGTH]; 
    int32_t rsuRadioChanMode;                          
    int32_t rsuRadioCh1;                              
    int32_t rsuRadioCh2;               
    int32_t rsuRadioTxPower1; 
    int32_t rsuRadioTxPower2;
} __attribute__((packed)) RsuRadioEntry_t;

void install_rsuRadioTable_handlers(void);
void destroy_rsuRadio(void);

/* 5.2.2 */
int32_t get_rsuRadioDesc(int32_t radio, uint8_t * data_out);
int32_t preserve_rsuRadioDesc(int32_t radio);
int32_t action_rsuRadioDesc(int32_t radio, uint8_t * data_in, int32_t length);
int32_t commit_rsuRadioDesc(int32_t radio);
int32_t undo_rsuRadioDesc(int32_t radio);
/* 5.2.3 */
int32_t get_rsuRadioEnable(int32_t radio);
int32_t preserve_rsuRadioEnable(int32_t radio);
int32_t action_rsuRadioEnable(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioEnable(int32_t radio);
int32_t undo_rsuRadioEnable(int32_t radio);
/* 5.2.4 */
int32_t get_rsuRadioType(int32_t radio);
int32_t get_rsuRadioMacAddress1(int32_t radio, uint8_t * data_out);
int32_t get_rsuRadioMacAddress2(int32_t radio, uint8_t * data_out);
/* 5.2.5 */
int32_t get_rsuRadioChanMode(int32_t radio);
int32_t preserve_rsuRadioChanMode(int32_t radio);
int32_t action_rsuRadioChanMode(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioChanMode(int32_t radio);
int32_t undo_rsuRadioChanMode(int32_t radio);
/* 5.2.6 */
int32_t get_rsuRadioCh1(int32_t radio);
int32_t preserve_rsuRadioCh1(int32_t radio);
int32_t action_rsuRadioCh1(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioCh1(int32_t radio);
int32_t undo_rsuRadioCh1(int32_t radio);
/* 5.2.7 */
int32_t get_rsuRadioCh2(int32_t radio);
int32_t preserve_rsuRadioCh2(int32_t radio);
int32_t action_rsuRadioCh2(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioCh2(int32_t radio);
int32_t undo_rsuRadioCh2(int32_t radio);
/* 5.2.8 */
int32_t get_rsuRadioTxPower1(int32_t radio, int32_t * data_out);
int32_t preserve_rsuRadioTxPower1(int32_t radio);
int32_t action_rsuRadioTxPower1(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioTxPower1(int32_t radio);
int32_t undo_rsuRadioTxPower1(int32_t radio);
/* 5.2.9 */
int32_t get_rsuRadioTxPower2(int32_t radio, int32_t * data_out);
int32_t preserve_rsuRadioTxPower2(int32_t radio);
int32_t action_rsuRadioTxPower2(int32_t radio, int32_t data_in);
int32_t commit_rsuRadioTxPower2(int32_t radio);
int32_t undo_rsuRadioTxPower2(int32_t radio);

/* RSU support. */

/* DCU. */
#define DCU_READ_I2V_RADIO_TYPE   "/usr/bin/conf_agent READ i2v I2VRadioType"
#define DCU_WRITE_I2V_RADIO_TYPE  "/usr/bin/conf_agent WRITE i2v I2VRadioType %d"

#define DCU_READ_I2V_TX_POWER   "/usr/bin/conf_agent READ i2v I2VTransmitPower"
#define DCU_WRITE_I2V_TX_POWER  "/usr/bin/conf_agent WRITE i2v I2VTransmitPower %d"

#if defined (PLATFORM_HD_RSU_5940)
#define DCU_READ_I2V_TX_CHAN   "/usr/bin/conf_agent READ i2v I2VUnifiedChannelNumber"
#define DCU_WRITE_I2V_TX_CHAN  "/usr/bin/conf_agent WRITE i2v I2VUnifiedChannelNumber %d"
#else
#define DCU_READ_DSRC_CHAN   "/usr/bin/conf_agent READ i2v I2VUnifiedChannelNumber"
#define DCU_WRITE_DSRC_CHAN  "/usr/bin/conf_agent WRITE i2v I2VUnifiedChannelNumber %d"

/* NSSTATS. */
#define NS_GET_DSRC_MAC_A "/usr/bin/nsstats -c | grep \"Radio A M\" | awk \'{print $NF}\'"
#define NS_GET_DSRC_MAC_B "/usr/bin/nsstats -c | grep \"Radio B M\" | awk \'{print $NF}\'"

/* CV2XCFG. */
#define CV2X_GET_CHAN "/usr/local/bin/cv2xcfg channel | head -1"
#endif
/****************************************************************************** 
 * 5.3.1 - 5.3.2: RSU GNSS: { rsu 2 }: rsuGnss
 *
 *  int32_t               rsuGnssStatus;
 *  rsuGnssAugmentation_e rsuGnssAugmentation;
 *
 ******************************************************************************/

/* 5.3.1 GNSS Status: rsuGnss */
#define GNSS_STATUS_MIN  0
#define GNSS_STATUS_MAX  128

/* 5.3.2 GNSS Augmentation: rsuGnssAugmentation */
typedef enum {
   rsuGnssAugmentation_other = 1,
   rsuGnssAugmentation_none  = 2,
   rsuGnssAugmentation_waas  = 3, 
}rsuGnssAugmentation_e;

void install_rsuGnssStatus_handlers(void);
void destroy_rsuGnssStatus(void);

/* 5.3.1 */
int32_t get_rsuGnssStatus(void);
/* 5.3.2 */
int32_t get_rsuGnssAugmentation(rsuGnssAugmentation_e * data_out);

/****************************************************************************** 
 * 5.4 Store and Repeat Messages: { rsu 3 }: rsuMsgRepeatStatusTable
 *     Shared with AMH, SDM and i2v_types.h 
 * 
 * typedef struct {
 * 
 *     int32_t  rsuMsgRepeatIndex;                                 RO: 1 to 255 SAR messages.
 *     uint8_t  rsuMsgRepeatPsid[RSU_RADIO_PSID_SIZE];             RC: 1 to 4 OCTETS:
 *     int32_t  rsuMsgRepeatTxChannel;                             RC: RSU_RADIO_CHANNEL_MIN to RSU_RADIO_CHANNEL_MAX.
 *     int32_t  rsuMsgRepeatTxInterval;                            RC: msecs between tx: 1 to 0x7fffffff. 2 seconds seems slow?
 *     uint8_t  rsuMsgRepeatDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC: "2d-1d-1d,1d:1d:1d.1d": 8 OCTETS: DateAndTime: SNMPv2-TC
 *     uint8_t  rsuMsgRepeatDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC: Same as start above.
 *     uint8_t  rsuMsgRepeatPayload[RSU_MSG_PAYLOAD_MAX];          RC: 0..2302 OCTETS:IEEE1609dot3-MIB. UPER MessageFrame.       
 *     onOff_e  rsuMsgRepeatEnable;                                RC: 0=off, 1=on tx: 
 *     int32_t  rsuMsgRepeatStatus;                                RC: Create (4) & (6) destroy:
 *     int32_t  rsuMsgRepeatPriority;                              RC: (0..63) : DSRC see IEEE 1609.3-2016.
 *     uint8_t  rsuMsgRepeatOptions;   RC: bitmask: 32 bits
 *                                       bit0(0x0 = bypass, use THEIR 1609.2, 0x1 = use OUR 1609.2)
 *                                       bit1(0x0 = secure(Sign), 0x1 = unsecure) 
 *                                       bit2(0x0 = continue tx, 0x1 = stop when > rsuShortCommLossTime: default 0)
 *                                       bit3(0x0 = continue tx, 0x1 = stop when > rsuLongtCommLossTime: default 0)
 * } RsuMsgRepeatStatusEntry_t;
 * 
 ******************************************************************************/

/* 5.4.1 Maximum Number of Store and Repeat Messages */
#define RSU_SAR_MESSAGE_MIN 1 
#define RSU_SAR_MESSAGE_MAX 100
#define RSU_SAR_MESSAGE_DEFAULT RSU_SAR_MESSAGE_MAX
/* 5.4.2 rsuMsgRepeatStatusTable */
#define RSU_MSG_REPEAT_TX_INTERVAL_MIN     1           /* msec */
#define RSU_MSG_REPEAT_TX_INTERVAL_DEFAULT 1000        /* 1 second seems reasonable. */
#define RSU_MSG_REPEAT_TX_INTERVAL_MAX     0x7FFFFFFF  /* Seems unreasonable max. Days between tx. */

#define RSU_MSG_REPEAT_PRIORITY_MIN     0
#define RSU_MSG_REPEAT_PRIORITY_MAX     63
#define RSU_MSG_REPEAT_PRIORITY_DEFAULT 7   /* Priority values are defined by IEEE 1609.3-2016 for DSRC radios. */

/* Bypass 1609.2 = OFF, Security = OFF, Continue Short & Long term Comm after loss of contact with host station. */
#define RSU_MSG_REPEAT_OPTIONS_MAX      0xf0 /* 4 bits only:lsb */
#define RSU_MSG_REPEAT_OPTIONS_LENGTH   1    /* Bytes */

#define MSG_REPEAT_OPT_1609_BYPASS_OFF_MASK  0x80
#define MSG_REPEAT_OPT_SECURITY_OFF_MASK     0x40
#define MSG_REPEAT_OPT_STERM_STOP_MASK       0x20
#define MSG_REPEAT_OPT_LTERM_STOP_MASK       0x10
#define RSU_MSG_REPEAT_OPTIONS_MASK          0xf0
#define RSU_MSG_REPEAT_OPTIONS_DEFAULT  (MSG_REPEAT_OPT_1609_BYPASS_OFF_MASK|MSG_REPEAT_OPT_SECURITY_OFF_MASK)

#define RSU_MSG_PAYLOAD_MIN  0
#define RSU_MSG_PAYLOAD_MAX  2302  /* see IEEE1609dot3-MIB. UPER encoded MessageFrame. */

#define RSU_MSG_FILENAME_LENGTH_MIN 6     /* strlen("%s/%u.dat") where %s is could be null. */
#define RSU_MSG_FILENAME_LENGTH_MAX 1024  /* Shared with RSU. */

/* Internal version control. */
#define MSG_REPEAT_VERSION 1

typedef struct {
    int32_t  rsuMsgRepeatIndex;
    uint8_t  rsuMsgRepeatPsid[RSU_RADIO_PSID_SIZE];
    int32_t  rsuMsgRepeatPsid_length; /* Given at snmpset(). Preserve and use rather than re-calculate.*/
    int32_t  rsuMsgRepeatTxChannel;
    int32_t  rsuMsgRepeatTxInterval;
    uint8_t  rsuMsgRepeatDeliveryStart[MIB_DATEANDTIME_LENGTH];
    uint8_t  rsuMsgRepeatDeliveryStop[MIB_DATEANDTIME_LENGTH];
    uint8_t  rsuMsgRepeatPayload[RSU_MSG_PAYLOAD_MAX];
    int32_t  rsuMsgRepeatPayload_length; /* Given at snmpset(). Preserve and use rather than re-calculate.*/   
    onOff_e  rsuMsgRepeatEnable;
    int32_t  rsuMsgRepeatStatus;
    int32_t  rsuMsgRepeatPriority;
    uint8_t  rsuMsgRepeatOptions;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuMsgRepeatStatusEntry_t;

void    install_rsuMsgRepeat_handlers(void);
void    destroy_rsuMsgRepeat(void);
void    set_default_row_rsuMsgRepeat(RsuMsgRepeatStatusEntry_t * rsuMsgRepeat, int32_t index);
int32_t commit_msgRepeat_to_disk(RsuMsgRepeatStatusEntry_t * msgRepeat); /* Will write single row to AMH_ACTIVE_DIR for AMH to accept or reject. */
int32_t rebuild_rsuMsgRepeat(RsuMsgRepeatStatusEntry_t * rsuMsgRepeat);
void    destroy_rsuMsgRepeatStatusTable(void);

/* 5.4.1 */
int32_t get_maxRsuMsgRepeat(void);
/* 5.4.2.1 */
int32_t  get_rsuMsgRepeatIndex(int32_t index, int32_t * data_out);
/* 5.4.2.2 */
int32_t get_rsuMsgRepeatPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMsgRepeatPsid(int32_t index);
int32_t action_rsuMsgRepeatPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMsgRepeatPsid(int32_t index);
int32_t undo_rsuMsgRepeatPsid(int32_t index);
/* 5.4.2.3 */
int32_t get_rsuMsgRepeatTxChannel(int32_t index, int32_t * data_out);
int32_t preserve_rsuMsgRepeatTxChannel(int32_t index);
int32_t action_rsuMsgRepeatTxChannel(int32_t index, int32_t data_in);
int32_t commit_rsuMsgRepeatTxChannel(int32_t index);
int32_t undo_rsuMsgRepeatTxChannel(int32_t index);
/* 5.4.2.4 */
int32_t get_rsuMsgRepeatTxInterval(int32_t index, int32_t * data_out);
int32_t preserve_rsuMsgRepeatTxInterval(int32_t index);
int32_t action_rsuMsgRepeatTxInterval(int32_t index, int32_t data_in);
int32_t commit_rsuMsgRepeatTxInterval(int32_t index);
int32_t undo_rsuMsgRepeatTxInterval(int32_t index);
/* 5.4.2.5 */
int32_t get_rsuMsgRepeatDeliveryStart(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMsgRepeatDeliveryStart(int32_t index);
int32_t action_rsuMsgRepeatDeliveryStart(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMsgRepeatDeliveryStart(int32_t index);
int32_t undo_rsuMsgRepeatDeliveryStart(int32_t index);
/* 5.4.2.6 */
int32_t get_rsuMsgRepeatDeliveryStop(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMsgRepeatDeliveryStop(int32_t index);
int32_t action_rsuMsgRepeatDeliveryStop(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMsgRepeatDeliveryStop(int32_t index);
int32_t undo_rsuMsgRepeatDeliveryStop(int32_t index);
/* 5.4.2.7 */
int32_t get_rsuMsgRepeatPayload(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMsgRepeatPayload(int32_t index);
int32_t action_rsuMsgRepeatPayload(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMsgRepeatPayload(int32_t index);
int32_t undo_rsuMsgRepeatPayload(int32_t index);
/* 5.4.2.8 */
int32_t get_rsuMsgRepeatEnable(int32_t index, int32_t * data_out);
int32_t preserve_rsuMsgRepeatEnable(int32_t index);
int32_t action_rsuMsgRepeatEnable(int32_t index, int32_t data_in);
int32_t commit_rsuMsgRepeatEnable(int32_t index);
int32_t undo_rsuMsgRepeatEnable(int32_t index);
/* 5.4.2.9 */
int32_t get_rsuMsgRepeatStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuMsgRepeatStatus(int32_t index);
int32_t action_rsuMsgRepeatStatus(int32_t index, int32_t data_in);
int32_t commit_rsuMsgRepeatStatus(int32_t index);
int32_t undo_rsuMsgRepeatStatus(int32_t index);
/* 5.4.2.10 */
int32_t get_rsuMsgRepeatPriority(int32_t index, int32_t * data_out);
int32_t preserve_rsuMsgRepeatPriority(int32_t index);
int32_t action_rsuMsgRepeatPriority(int32_t index, int32_t data_in);
int32_t commit_rsuMsgRepeatPriority(int32_t index);
int32_t undo_rsuMsgRepeatPriority(int32_t index);
/* 5.4.2.11 */
int32_t get_rsuMsgRepeatOptions(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMsgRepeatOptions(int32_t index);
int32_t action_rsuMsgRepeatOptions(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuMsgRepeatOptions(int32_t index);
int32_t undo_rsuMsgRepeatOptions(int32_t index);
/* 5.4.3 */
int32_t get_rsuMsgRepeatDeleteAll(void);
int32_t preserve_rsuMsgRepeatDeleteAll(void);
int32_t action_rsuMsgRepeatDeleteAll(int32_t data_in);
int32_t commit_rsuMsgRepeatDeleteAll(void);
int32_t undo_rsuMsgRepeatDeleteAll(void);

/* Back end support for rsuRepeatMsg in AMH */

/****************************************************************************** 
 * 5.5 Immediate Forward Messages: { rsu 4 }: rsuIFMStatusTable
 *
 * typedef struct {
 *     int32_t  rsuIFMIndex;                     RO: 1 to 255 IFM messages.
 *     uint8_t  rsuIFMPsid[RSU_RADIO_PSID_SIZE]; RC: 1 to 4 OCTETS:
 *     int32_t  rsuIFMTxChannel;                 RC: RSU_RADIO_CHANNEL_MIN to RSU_RADIO_CHANNEL_MAX.
 *     onOff_e  rsuIFMEnable;                    RC: 0=off, 1= on tx: 
 *     int32_t  rsuIFMStatus;                    RC: Create (4) & (6) destroy
 *     int32_t  rsuIFMPriority;                  RC: (0..63) : DSRC see IEEE 1609.3-2016.  
 *
 *     uint8_t  rsuIFMOptions; RC: bitmask: 32 bits
 *                                 bit0(0x0 = bypass, use THEIR 1609.2, 0x1 = use OUR 1609.2)
 *                                 bit1(0x0 = secure(Sign or Encrypt), 0x1 = unsecure) 
 *                                 bit2(0x0 = continue tx, 0x1 = stop when > rsuShortCommLossTime: default 0)
 *                                 bit3(0x0 = continue tx, 0x1 = stop when > rsuLongtCommLossTime: default 0)
 *
 *    uint8_t rsuIFMPayload[RSU_MSG_PAYLOAD_MAX]; RC: 0..2302 OCTETS:IEEE1609dot3-MIB. UPER MessageFrame.
 *
 * } RsuIFMStatusEntry_t;
 *
 ******************************************************************************/ 

/* 5.5.1 Maximum Number of Immediate Forward Messages. */
#define RSU_IFM_MESSAGE_MIN 1 
#define RSU_IFM_MESSAGE_MAX 100

#define RSU_IFM_PRIORITY_MIN     0
#define RSU_IFM_PRIORITY_MAX     63
#define RSU_IFM_PRIORITY_DEFAULT RSU_IFM_PRIORITY_MIN   /* Priority values are defined by IEEE 1609.3-2016 for DSRC radios. */

/* Bypass 1609.2 = OFF, Security = OFF, Continue Short & Long term Comm after loss of contact with host station. */
#define RSU_IFM_OPTIONS_LENGTH   1    /* Bytes */

#define IFM_OPT_1609_BYPASS_OFF_MASK  0x80
#define IFM_OPT_SECURITY_OFF_MASK     0x40
#define IFM_OPT_STERM_STOP_MASK       0x20
#define IFM_OPT_LTERM_STOP_MASK       0x10
#define RSU_IFM_OPTIONS_MASK          0xf0
#define RSU_IFM_OPTIONS_DEFAULT  (IFM_OPT_1609_BYPASS_OFF_MASK|IFM_OPT_SECURITY_OFF_MASK)

/* Internal version control. */
#define IFM_VERSION 1

/* 5.5.2 Immediate Forward Table */
typedef struct {
    int32_t  rsuIFMIndex;
    uint8_t  rsuIFMPsid[RSU_RADIO_PSID_SIZE];
    int32_t  rsuIFMPsid_length; /* Given at snmpset(). Preserve and use rather than re-calculate.*/
    int32_t  rsuIFMTxChannel;
    onOff_e  rsuIFMEnable;
    int32_t  rsuIFMStatus;
    int32_t  rsuIFMPriority;
    uint8_t  rsuIFMOptions;
    uint8_t  rsuIFMPayload[RSU_MSG_PAYLOAD_MAX];
    int32_t  rsuIFMPayload_length; /* Given at snmpset(). Preserve and use rather than re-calculate.*/
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuIFMStatusEntry_t;

/* "Skinny" means we are only preserving a subset of rsuIFM to disk. */
typedef struct {
    uint8_t  rsuIFMPsid[RSU_RADIO_PSID_SIZE];
    int32_t  rsuIFMPsid_length;
    int32_t  rsuIFMTxChannel;
    onOff_e  rsuIFMEnable;
    int32_t  rsuIFMStatus;
    int32_t  rsuIFMPriority;
    uint8_t  rsuIFMOptions;
    int32_t  version;
    uint32_t reserved;
    uint32_t crc;
} __attribute__((packed)) IFMSkinny_t;

void install_rsuIFM_handlers(void);
void destroy_rsuIFM(void);
void set_default_row_rsuIFM(RsuIFMStatusEntry_t * rsuIFM, int32_t index); /* Set single instance only. */
int32_t commit_IFM_to_disk(RsuIFMStatusEntry_t * IFM);
int32_t rebuild_rsuIFM_live(RsuIFMStatusEntry_t * IFM);
/* 5.5.1 */
int32_t get_maxRsuIFMs(void);
/* 5.5.2.1 */
int32_t  get_rsuIFMIndex(int32_t index, int32_t * data_out);
/* 5.5.2.2 */
int32_t get_rsuIFMPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIFMPsid(int32_t index);
int32_t action_rsuIFMPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIFMPsid(int32_t index);
int32_t undo_rsuIFMPsid(int32_t index);
/* 5.5.2.3 */
int32_t get_rsuIFMTxChannel(int32_t index, int32_t * data_out);
int32_t preserve_rsuIFMTxChannel(int32_t index);
int32_t action_rsuIFMTxChannel(int32_t index, int32_t data_in);
int32_t commit_rsuIFMTxChannel(int32_t index);
int32_t undo_rsuIFMTxChannel(int32_t index);
/* 5.5.2.4 */
int32_t get_rsuIFMEnable(int32_t index, int32_t * data_out);
int32_t preserve_rsuIFMEnable(int32_t index);
int32_t action_rsuIFMEnable(int32_t index, int32_t data_in);
int32_t commit_rsuIFMEnable(int32_t index);
int32_t undo_rsuIFMEnable(int32_t index);
/* 5.5.2.5 */
int32_t get_rsuIFMStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuIFMStatus(int32_t index);
int32_t action_rsuIFMStatus(int32_t index, int32_t data_in);
int32_t commit_rsuIFMStatus(int32_t index);
int32_t undo_rsuIFMStatus(int32_t index);
/* 5.5.2.6 */
int32_t get_rsuIFMPriority(int32_t index, int32_t * data_out);
int32_t preserve_rsuIFMPriority(int32_t index);
int32_t action_rsuIFMPriority(int32_t index, int32_t data_in);
int32_t commit_rsuIFMPriority(int32_t index);
int32_t undo_rsuIFMPriority(int32_t index);
/* 5.5.2.7 */
int32_t get_rsuIFMOptions(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIFMOptions(int32_t index);
int32_t action_rsuIFMOptions(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuIFMOptions(int32_t index);
int32_t undo_rsuIFMOptions(int32_t index);
/* 5.5.2.8 */
int32_t get_rsuIFMPayload(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIFMPayload(int32_t index);
int32_t action_rsuIFMPayload(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIFMPayload(int32_t index);
int32_t undo_rsuIFMPayload(int32_t index);

/* Back end support for rsuIFM in AMH */
#define IFM_ACTIVE_DIR "/rwflash/I2V/ifm"   /* MIB needs to know since it can run when I2V is not. */
#define DEFAULT_MQ_PRIORITY  0
#define IFM_MIB_QUEUE "/IfmMibQ"
#define IFM_QUEUE_DEFAULT_FLAGS  O_NONBLOCK  /* Seems ok to block in this case. */
#define IFM_QUEUE_MSG_SIZE  (int32_t)sizeof(RsuIFMStatusEntry_t)

#if 0 /* Seems like way too much. */
#define IFM_QUEUE_DEPTH     RSU_IFM_MESSAGE_MAX
#else
#define IFM_QUEUE_DEPTH     10
#endif
int32_t ifm_enqueue(const char_t * data_in);

/****************************************************************************** 
 * 5.6 (OTA) Received Messages(to fwd via socket): { rsu 5 }: rsuReceivedMsgTable
 *
 * typedef struct {
 *    int32_t    rsuReceivedMsgIndex;                                 RO: 1 to maxRsuReceivedMsgs.
 *    uint8_t    rsuReceivedMsgPsid;                                  RC: 1 to 4 OCTETS:
 *    uint8_t    rsuReceivedMsgDestIpAddr[RSU_DEST_IP_MAX];           RC: 0 to 64: IPv4 or IPv6.
 *    int32_t    rsuReceivedMsgDestPort;                              RC: 1024..65535.
 *    protocol_e rsuReceivedMsgProtocol;                              RC: 1 = other , 2 = udp: default is udp.
 *    int32_t    rsuReceivedMsgRssi;                                  RC: -100 to -60: no default.
 *    int32_t    rsuReceivedMsgInterval;                              RC: 0 to 10: 0 == disable. No default.
 *    uint8_t    rsuReceivedMsgDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *    uint8_t    rsuReceivedMsgDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *    int32_t    rsuReceivedMsgStatus;                                RC: Create (4) & (6) destroy:
 *
 * 
 *    A value of 0 indicates the RSU is to forward only the WSM message payload without security headers. 
 *    Specifically this means that either of the following is forwarded, 
 *    depending on whether the message is signed (a) or unsigned (b): 
 *    (a) Ieee1609Dot2Data.signedData.tbsData.payload.data.unsecuredData or 
 *    (b) Ieee1609Dot2Data.unsecuredData.
 *    A value of 1 indicates the RSU is to forward the entire WSM including the security headers. 
 *    Specifically this means that the entire Ieee1609Dot2Data frame is forwarded in COER format.
 *
 *    int32_t rsuReceivedMsgSecure;       RC: 0 == only fwd wsm without 1609.2, 1 == with header.
 *
 * 
 *    Interval with which the RSU authenticates messages received from a specific device over the V2X Interface 
 *    and to be forwarded to the Server (as controlled by rsuDsrcFwdMsgInterval). If enabled, the RSU authenticates 
 *    the first valid (e.g., as defined by rsuDsrcFwdRssi, rsuDsrcFwdDeliveryStart, rsuDsrcFwdDeliveryStop for this row) 
 *    message received from a specific device. For a value of 4, the RSU then authenticates every 4th message 
 *    (after the first message) for that specific device that is marked for forwarding (as determined by rsuDsrcFwdMsgInterval). 
 *    A value of 0 disables authentication of message to be forward for this particular entry. For example, an 
 *    RSU receives 12 messages that satisfies the criteria for this row (rsuDsrcFwdPsid, rsuDsrcFwdRssi, 
 *    rsuDsrcFwdDeliveryStart, rsuDsrcFwdDeliveryStop). Messages 1, 2, 5, 6, 7, 10 and 12 are from device A 
 *    and messages 3, 4, 8, 9 and 11 are from device B. Assuming rsuDsrcFwdMsgInterval has a value of 2, 
 *    only messages 1, 3, 5, 7, 9, and 11 are 'marked' for forwarding. Of these messages, 
 *    only messages 1 (the first message from device A), 3 (the first message from device B), 
 *    7 (the 2nd message from device A after the first message), and 11 (the 2nd message from device B after the first message) 
 *    are authenticated.
 *
 *    int32_t rsuReceivedMsgAuthMsgInterval; RC: 0 to 10: 0 == disable. No default.
 *  
 * } RsuReceivedMsgEntry_t;
 *
 ******************************************************************************/

/* 5.6.1    Maximum Number of Messages */
#define RSU_RX_MSG_MIN 1   
#define RSU_RX_MSG_MAX 5  /* 5 ip+port combos in FWDMSG. Ignore PSID as dont care. */

/* 5.6.2 Received Messages Table. */
#define RSU_RX_MSG_PORT_MIN           1024
#define RSU_RX_MSG_PORT_MAX           RSU_DEST_PORT_MAX
#define RSU_RX_MSG_PORT_DEFAULT       5114

#define RSU_RX_MSG_RSSI_THRESHOLD_MIN -100
#define RSU_RX_MSG_RSSI_THRESHOLD_MAX -60

#define RSU_RX_MSG_INTERVAL_MIN       0
#define RSU_RX_MSG_INTERVAL_MAX       10

#define RSU_RX_MSG_AUTH_INTERVAL_MIN  0
#define RSU_RX_MSG_AUTH_INTERVAL_MAX  10

/* Internal version control. */
#define RX_MSG_VERSION 1

typedef struct {
    int32_t    rsuReceivedMsgIndex;
    uint8_t    rsuReceivedMsgPsid[RSU_RADIO_PSID_SIZE];
    int32_t    rsuReceivedMsgPsid_length; 
    uint8_t    rsuReceivedMsgDestIpAddr[RSU_DEST_IP_MAX];
    int32_t    rsuReceivedMsgDestIpAddr_length; 
    int32_t    rsuReceivedMsgDestPort;
    protocol_e rsuReceivedMsgProtocol;
    int32_t    rsuReceivedMsgRssi;
    int32_t    rsuReceivedMsgInterval;
    uint8_t    rsuReceivedMsgDeliveryStart[MIB_DATEANDTIME_LENGTH];
    uint8_t    rsuReceivedMsgDeliveryStop[MIB_DATEANDTIME_LENGTH];
    int32_t    rsuReceivedMsgStatus;
    int32_t    rsuReceivedMsgSecure;
    int32_t    rsuReceivedMsgAuthMsgInterval;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuReceivedMsgEntry_t;

void install_rsuReceivedMsg_handlers(void);
void destroy_rsuReceivedMsg(void);
int32_t rebuild_rsuReceivedMsg_live(RsuReceivedMsgEntry_t * rsuReceivedMsg);
int32_t commit_rsuReceivedMsg_to_disk(RsuReceivedMsgEntry_t * rsuReceivedMsg);
/* 5.6.1 */
int32_t get_maxRsuReceivedMsgs(void);
/* 5.6.2.1 */
int32_t  get_rsuReceivedMsgIndex(int32_t index, int32_t * data_out);
/* 5.6.2.2 */
int32_t get_rsuReceivedMsgPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuReceivedMsgPsid(int32_t index);
int32_t action_rsuReceivedMsgPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuReceivedMsgPsid(int32_t index);
int32_t undo_rsuReceivedMsgPsid(int32_t index);
/* 5.6.2.3 */
int32_t get_rsuReceivedMsgDestIpAddr(int32_t index, uint8_t * data_out);
int32_t preserve_rsuReceivedMsgDestIpAddr(int32_t index);
int32_t action_rsuReceivedMsgDestIpAddr(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuReceivedMsgDestIpAddr(int32_t index);
int32_t undo_rsuReceivedMsgDestIpAddr(int32_t index);
/* 5.6.2.4 */
int32_t get_rsuReceivedMsgDestPort(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgDestPort(int32_t index);
int32_t action_rsuReceivedMsgDestPort(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgDestPort(int32_t index);
int32_t undo_rsuReceivedMsgDestPort(int32_t index);
/* 5.6.2.5 */
int32_t get_rsuReceivedMsgProtocol(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgProtocol(int32_t index);
int32_t action_rsuReceivedMsgProtocol(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgProtocol(int32_t index);
int32_t undo_rsuReceivedMsgProtocol(int32_t index);
/* 5.6.2.6 */
int32_t get_rsuReceivedMsgRssi(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgRssi(int32_t index);
int32_t action_rsuReceivedMsgRssi(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgRssi(int32_t index);
int32_t undo_rsuReceivedMsgRssi(int32_t index);
/* 5.6.2.7 */
int32_t get_rsuReceivedMsgInterval(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgInterval(int32_t index);
int32_t action_rsuReceivedMsgInterval(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgInterval(int32_t index);
int32_t undo_rsuReceivedMsgInterval(int32_t index);
/* 5.6.2.8 */
int32_t get_rsuReceivedMsgDeliveryStart(int32_t index, uint8_t * data_out);
int32_t preserve_rsuReceivedMsgDeliveryStart(int32_t index);
int32_t action_rsuReceivedMsgDeliveryStart(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuReceivedMsgDeliveryStart(int32_t index);
int32_t undo_rsuReceivedMsgDeliveryStart(int32_t index);
/* 5.6.2.9 */
int32_t get_rsuReceivedMsgDeliveryStop(int32_t index, uint8_t * data_out);
int32_t preserve_rsuReceivedMsgDeliveryStop(int32_t index);
int32_t action_rsuReceivedMsgDeliveryStop(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuReceivedMsgDeliveryStop(int32_t index);
int32_t undo_rsuReceivedMsgDeliveryStop(int32_t index);
/* 5.6.2.10 */
int32_t get_rsuReceivedMsgStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgStatus(int32_t index);
int32_t action_rsuReceivedMsgStatus(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgStatus(int32_t index);
int32_t undo_rsuReceivedMsgStatus(int32_t index);
/* 5.6.2.11 */
int32_t get_rsuReceivedMsgSecure(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgSecure(int32_t index);
int32_t action_rsuReceivedMsgSecure(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgSecure(int32_t index);
int32_t undo_rsuReceivedMsgSecure(int32_t index);
/* 5.6.2.12 */
int32_t get_rsuReceivedMsgAuthMsgInterval(int32_t index, int32_t * data_out);
int32_t preserve_rsuReceivedMsgAuthMsgInterval(int32_t index);
int32_t action_rsuReceivedMsgAuthMsgInterval(int32_t index, int32_t data_in);
int32_t commit_rsuReceivedMsgAuthMsgInterval(int32_t index);
int32_t undo_rsuReceivedMsgAuthMsgInterval(int32_t index);

/* RSU App support. */

//TODO: Maybe be able to chop, but is useful to init stuff later.
void *receivedMsgThr(void __attribute__((unused)) *arg);

/* FWDMSG defines */
#define FWDMSG_BSM_MASK  0x01
#define FWDMSG_MAP_MASK  0x02
#define FWDMSG_PSM_MASK  0x04
#define FWDMSG_SPAT_MASK 0x08
#define FWDMSG_AMH_MASK  0x10
#define FWDMSG_SRM_MASK  0x20
#define FWDMSG_ALL_PSID_MASK 0x3f

#define DCU_READ_FWDMSG_ENABLE_REPLY "%d"
#define DCU_READ_FWDMSG_IP_REPLY     "%s"   /* char_t cfg_string[I2V_CFG_MAX_STR_LEN] */
#define DCU_READ_FWDMSG_PORT_REPLY   "%u"   /* uint16_t */ 
#define DCU_READ_FWDMSG_MASK_REPLY   "0x%x" /* uint8_t  */

#define DCU_WRITE_FWDMSG_ENABLE_REPLY "%d"
#define DCU_WRITE_FWDMSG_IP_REPLY     "%s"   /* char_t cfg_string[I2V_CFG_MAX_STR_LEN] */
#define DCU_WRITE_FWDMSG_PORT_REPLY   "%d"
#define DCU_WRITE_FWDMSG_MASK_REPLY   "%d"

#define DCU_READ_FWDMSG_ENABLE_1 "/usr/bin/conf_agent READ fwdmsg ForwardMessageEnable1"
#define DCU_READ_FWDMSG_IP_1     "/usr/bin/conf_agent READ fwdmsg ForwardMessageIP1"
#define DCU_READ_FWDMSG_PORT_1   "/usr/bin/conf_agent READ fwdmsg ForwardMessagePort1"
#define DCU_READ_FWDMSG_MASK_1   "/usr/bin/conf_agent READ fwdmsg ForwardMessageMask1"

#define DCU_WRITE_FWDMSG_ENABLE_1 "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageEnable1 %d"
#define DCU_WRITE_FWDMSG_IP_1     "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageIP1 %s"
#define DCU_WRITE_FWDMSG_PORT_1   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessagePort1 %d"
#define DCU_WRITE_FWDMSG_MASK_1   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageMask1 0x%x"

#define DCU_READ_FWDMSG_ENABLE_2 "/usr/bin/conf_agent READ fwdmsg ForwardMessageEnable2"
#define DCU_READ_FWDMSG_IP_2     "/usr/bin/conf_agent READ fwdmsg ForwardMessageIP2"
#define DCU_READ_FWDMSG_PORT_2   "/usr/bin/conf_agent READ fwdmsg ForwardMessagePort2"
#define DCU_READ_FWDMSG_MASK_2   "/usr/bin/conf_agent READ fwdmsg ForwardMessageMask2"

#define DCU_WRITE_FWDMSG_ENABLE_2 "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageEnable2 %d"
#define DCU_WRITE_FWDMSG_IP_2     "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageIP2 %s"
#define DCU_WRITE_FWDMSG_PORT_2   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessagePort2 %d"
#define DCU_WRITE_FWDMSG_MASK_2   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageMask2 0x%x"

#define DCU_READ_FWDMSG_ENABLE_3 "/usr/bin/conf_agent READ fwdmsg ForwardMessageEnable3"
#define DCU_READ_FWDMSG_IP_3     "/usr/bin/conf_agent READ fwdmsg ForwardMessageIP3"
#define DCU_READ_FWDMSG_PORT_3   "/usr/bin/conf_agent READ fwdmsg ForwardMessagePort3"
#define DCU_READ_FWDMSG_MASK_3   "/usr/bin/conf_agent READ fwdmsg ForwardMessageMask3"

#define DCU_WRITE_FWDMSG_ENABLE_3 "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageEnable3 %d"
#define DCU_WRITE_FWDMSG_IP_3     "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageIP3 %s"
#define DCU_WRITE_FWDMSG_PORT_3   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessagePort3 %d"
#define DCU_WRITE_FWDMSG_MASK_3   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageMask3 0x%x"

#define DCU_READ_FWDMSG_ENABLE_4 "/usr/bin/conf_agent READ fwdmsg ForwardMessageEnable4"
#define DCU_READ_FWDMSG_IP_4     "/usr/bin/conf_agent READ fwdmsg ForwardMessageIP4"
#define DCU_READ_FWDMSG_PORT_4   "/usr/bin/conf_agent READ fwdmsg ForwardMessagePort4"
#define DCU_READ_FWDMSG_MASK_4   "/usr/bin/conf_agent READ fwdmsg ForwardMessageMask4"

#define DCU_WRITE_FWDMSG_ENABLE_4 "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageEnable4 %d"
#define DCU_WRITE_FWDMSG_IP_4     "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageIP4 %s"
#define DCU_WRITE_FWDMSG_PORT_4   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessagePort4 %d"
#define DCU_WRITE_FWDMSG_MASK_4   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageMask4 0x%x"

#define DCU_READ_FWDMSG_ENABLE_5 "/usr/bin/conf_agent READ fwdmsg ForwardMessageEnable5"
#define DCU_READ_FWDMSG_IP_5     "/usr/bin/conf_agent READ fwdmsg ForwardMessageIP5"
#define DCU_READ_FWDMSG_PORT_5   "/usr/bin/conf_agent READ fwdmsg ForwardMessagePort5"
#define DCU_READ_FWDMSG_MASK_5   "/usr/bin/conf_agent READ fwdmsg ForwardMessageMask5"

#define DCU_WRITE_FWDMSG_ENABLE_5 "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageEnable5 %d"
#define DCU_WRITE_FWDMSG_IP_5     "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageIP5 %s"
#define DCU_WRITE_FWDMSG_PORT_5   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessagePort5 %d"
#define DCU_WRITE_FWDMSG_MASK_5   "/usr/bin/conf_agent WRITE fwdmsg ForwardMessageMask5 0x%x"

/* DCU. These IP's we must not conflict with. */
#define DCU_READ_SCS_IP    "/usr/bin/conf_agent READ scs LocalSignalControllerIP" 
#define DCU_READ_RSU_IP    "/usr/bin/conf_agent READ RSU_INFO SC0IPADDRESS"

int32_t set_fwdmsgEnable(int32_t i, int32_t enable);
int32_t set_fwdmsgIP(int32_t i, uint8_t * IpAddr);
int32_t set_fwdmsgPort(int32_t i, uint16_t port);
int32_t set_fwdmsgMask(int32_t i, uint8_t mask);
/****************************************************************************** 
 * 5.7.1 - 5.7.11 GNSS Output: { rsu 6 }
 *
 *  int32_t rsuGnssOutputPort;
 *  uint8_t rsuGnssOutputAddress[RSU_DEST_IP_MAX];
 *  uint8_t rsuGnssOutputInterface[RSU_GNSS_OUTPUT_INTERFACE_MAX];
 *  int32_t rsuGnssOutputInterval;
 *  uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX];
 *  int32_t rsuGnssLat;
 *  int32_t rsuGnssLon;
 *  int32_t rsuGnssElv;
 *  int32_t rsuGnssMaxDeviation;
 *  int32_t rsuLocationDeviation;
 *  int32_t rsuGnssPositionError;
 ******************************************************************************/

/* 5.7.1 GNSS Output Port */
#define RSU_GNSS_OUTPUT_PORT_MIN     1024
#define RSU_GNSS_OUTPUT_PORT_MAX     RSU_DEST_PORT_MAX
#define RSU_GNSS_OUTPUT_PORT_DEFAULT 5115 /* MIB default. */

/* 5.7.2 GNSS Output Address: IPv6 or v4 */
#if 0
#define RSU_GNSS_OUTPUT_ADDRESS_DEFAULT "0A34:0B2F:AABB:CCDD:0:0:0:0"
#else
//#define RSU_GNSS_OUTPUT_ADDRESS_DEFAULT "10.52.11.47"
#define RSU_GNSS_OUTPUT_ADDRESS_DEFAULT "192.168.1.47"
#endif

/* 5.7.3 GNSS Output Interface Description */
#define RSU_GNSS_OUTPUT_INTERFACE_MIN     0
#define RSU_GNSS_OUTPUT_INTERFACE_MAX     100
#define RSU_GNSS_OUTPUT_INTERFACE_DEFAULT "eth0" /* 5940 + 5912 */ 

/* 5.7.4 GNSS Output Interval */
#define RSU_GNSS_OUTPUT_INTERVAL_MIN      0
#define RSU_GNSS_OUTPUT_INTERVAL_MAX      18000
#define RSU_GNSS_OUTPUT_INTERVAL_DEFAULT  0

/* 5.7.5 GNSS Data Output: NMEA 0183 GGA. */
#define RSU_GNSS_OUTPUT_STRING_MIN 0
#define RSU_GNSS_OUTPUT_STRING_MAX 100

/* 5.7.9 GNSS Allowable Location Deviation: max 2D deviation between (rsuGnssLat, rsuGnssLon) vs (rsuLocationLat, rsuLocationLon).*/
#define RSU_GNSS_OUTPUT_DEVIATION_MIN 0 /* default */
#define RSU_GNSS_OUTPUT_DEVIATION_MAX 20000

/* 5.7.10 GNSS Location Deviation: Current 2D deviation in metres of (rsuGnssLat, rsuGnssLon) vs (rsuLocationLat, rsuLocationLon).*/
#define RSU_GNSS_OUTPUT_DEVIATION_UNKNOWN 20001 /* metres */

/* 5.7.11 RSU Position Error: Average at 67% con. 1 std deviation: "tenth of a meter" */
#define RSU_GNSS_OUTPUT_POS_ERROR_MIN    0
#define RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN 200001 /* decimetre */
#define RSU_GNSS_OUTPUT_POS_ERROR_MAX    RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN

/* Internal version control. */
#define GNSS_OUTPUT_VERSION 1

/* struct only used to read+write to disk nicely. */
typedef struct {
    int32_t rsuGnssOutputPort;
    uint8_t rsuGnssOutputAddress[RSU_DEST_IP_MAX];
    int32_t rsuGnssOutputAddress_length;
    uint8_t rsuGnssOutputInterface[RSU_GNSS_OUTPUT_INTERFACE_MAX];
    int32_t rsuGnssOutputInterface_length;
    int32_t rsuGnssOutputInterval;
    int32_t rsuGnssMaxDeviation;
    /* Hidden from client. RSU internal use only: noAccess. */
    int32_t  version;      /* Internal revision control. */
    int32_t  status;
    uint32_t reserved;     /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;          /* Allow for 32 bit if needed. */
} __attribute__((packed)) GnssOutput_t; 

void install_rsuGnssOutput_handlers(void);
void destroy_rsuGnssOutput(void);

/* 5.7.1 */
int32_t get_rsuGnssOutputPort(void);
int32_t preserve_rsuGnssOutputPort(void);
int32_t action_rsuGnssOutputPort(int32_t data_in);
int32_t commit_rsuGnssOutputPort(void);
int32_t undo_rsuGnssOutputPort(void);
/* 5.7.2 */
int32_t get_rsuGnssOutputAddress(uint8_t * data_out);
int32_t preserve_rsuGnssOutputAddress(void);
int32_t action_rsuGnssOutputAddress(uint8_t * data_in, int32_t length);
int32_t commit_rsuGnssOutputAddress(void);
int32_t undo_rsuGnssOutputAddress(void);
/* 5.7.3 */
int32_t get_rsuGnssOutputInterface(uint8_t * data_out);
int32_t preserve_rsuGnssOutputInterface(void);
int32_t action_rsuGnssOutputInterface(uint8_t * data_in, int32_t length);
int32_t commit_rsuGnssOutputInterface(void);
int32_t undo_rsuGnssOutputInterface(void);
/* 5.7.4 */
int32_t get_rsuGnssOutputInterval(void);
int32_t preserve_rsuGnssOutputInterval(void);
int32_t action_rsuGnssOutputInterval(int32_t data_in);
int32_t commit_rsuGnssOutputInterval(void);
int32_t undo_rsuGnssOutputInterval(void);
/* 5.7.5 */
int32_t get_rsuGnssOutputString(uint8_t * data_out);
/* 5.7.6 */
int32_t get_rsuGnssLat(int32_t * data_out);
/* 5.7.7 */
int32_t get_rsuGnssLon(int32_t * data_out);
/* 5.7.8 */
int32_t get_rsuGnssElv(int32_t * data_out);
/* 5.7.9 */
int32_t get_rsuGnssMaxDeviation(int32_t * data_out);
int32_t preserve_rsuGnssMaxDeviation(void);
int32_t action_rsuGnssMaxDeviation(int32_t data_in);
int32_t commit_rsuGnssMaxDeviation(void);
int32_t undo_rsuGnssMaxDeviation(void);
/* 5.7.10 */
int32_t get_rsuLocationDeviation(int32_t * data_out);
/* 5.7.11 */
int32_t get_rsuGnssPositionError(int32_t * data_out);

/* Name of OID MIB data stored on disk. */
#define GNSS_OUTPUT_PATH         "/rwflash/I2V/ntcip-1218"
#define GNSS_OUTPUT_PATHFILENAME "/rwflash/I2V/ntcip-1218/gnssOutput.dat"

/* "skinny" means we are only preserving a subset of rsuGnssOutput to disk. */
int32_t commit_gnssOutput_to_disk(GnssOutput_t * gnssOutput);
int32_t copy_gnssOutput_to_skinny(GnssOutput_t * gnssOutput);

/* Handles TPS SHM, calls deviation test and sends notifcation message to client ip+port. */
void *gnssOutputThr(void __attribute__((unused)) *arg);
void update_gnssOutput(void);

/******************************************************************************
 * 5.8 Interface Log: { rsu 7 }: rsuInterfaceLogTable.
 * 
 * typedef struct {
 *     int32_t     rsuIfaceLogIndex;                     RO: 1 to maxRsuInterfaceLogs: 5940 has eth0, dsrc1 & cv2x1.
 *     onOff_e     rsuIfaceGenerate;                     RC: 0=off, 1=on.
 *     int32_t     rsuIfaceMaxFileSize;                  RC: 1..40 megabytes: 5 is default.
 *     int32_t     rsuIfaceMaxFileTime;                  RC: 1..48 hours: 24 is default.
 *     ifclogDir_e rsuIfaceLogByDir;                     RC: 1=inBound,2=outBound,3=biSeperate, 4=biTogether: default is 1?
 *     uint8_t     rsuIfaceName[RSU_INTERFACE_NAME_MAX]; RC: 0..127: "eth0", "dsrc0", "cv2x0". if its "" then what?
 *
 *     Storage path of ifclogs. Relative to the base directory (rsuSysDir). base dir is specified by  '/':  sub dir '/mysubdir'.
 *
 *     uint8_t rsuIfaceStoragePath[RSU_IFCLOG_STORE_PATH_MAX];  RC:1..255: path to ifclogs relative to rsuSysDir.
 * 
 * 
 *     Filename for ifclogs:
 *         <identifier>_<interface>_<direction>_<time> and contain only the following case-sensitive field names in chevrons (<>):
 *              <identifier>: Identifier of the RSU. See rsuID.
 *              <interface>: Identifier of the interface. See rsuIfaceName.
 *              <direction>: Direction of communications. In for Inbound, Out for Outbound), and Both for inbound and outbound.
 *              <time>: In UTC. The format is YYYYMMDD_hhmmss, where YYYY is the four-digit year, 
 *                              MM is the two-digit month, DD is the two-digit day, hh is the two-digit hour, 
 *                              mm is the two-digit minute, and ss is the two-digit seconds.
 *      
 *       Example: If the rsuID = rsu1, rsuIfaceName = dsrc1, both directions are collected in the same file, 
 *                and the date and time the file was created is January 3, 2019, at 01:35:03, 
 *                the filename would be rsu1_dsrc1_Both_20190103_013503.
 *      
 *     uint8_t rsuIfaceLogName[RSU_IFCLOG_NAME_SIZE_MAX]; RC: 12..172: filename for ifclogs ie rsu1_dsrc1_Both_20190103_013503.
 * 
 *     uint8_t rsuIfaceLogStart[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.           
 *     uint8_t rsuIfaceLogStop[MIB_DATEANDTIME_LENGTH];  RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC. 
 * 
 * 
 *     A bit-mapped value as defined below for configuring the message.
 *       Bit 0        0=Delete oldest if disk full, 1=Stop if disk full
 *       Bit 1        0=DoNotDelete, 1=Delete data log
 *     
 *       Bit 0 - Indicates if the RSU is to delete the oldest interface data log file if the disk space for 
 *             storing interface data logs is full, or to stop interface data logging. Default is to delete the oldest file.
 *       Bit 1 - Indicates if the RSU is to delete the corresponding interface data log file when an entry is destroyed in 
 *             the rsuInterfaceLogTable. Default is not to delete the log file.
 *     
 *     uint8_t rsuIfaceLogOptions; RC: bit 0 == diskFull, bit 1 == deleteentry: default is 0x00.
 * 
 *     int32_t rsuIfaceLogStatus;  RC: Create (4) & (6) destroy: notInService & notReady?
 * } RsuInterfaceLogEntry_t;
 *
 ******************************************************************************/

/* 5.8.1 Maximum Number of Interface Logs. For now only allow 1? */
#define RSU_IFCLOG_MIN               1 
#define RSU_IFCLOG_MAX               1  /* default: 5940 == eth0 + cv2x1 == 2. IFCLOG on does both directions in one file. */

/* 5.8.2 Interface Log Table. */
#define RSU_IFCLOG_FILE_SIZE_MIN     1
#define RSU_IFCLOG_FILE_SIZE_MAX     40
#define RSU_IFCLOG_FILE_SIZE_DEFAULT 5

#define RSU_IFCLOG_FILE_TIME_MIN     1
#define RSU_IFCLOG_FILE_TIME_MAX     48
#define RSU_IFCLOG_FILE_TIME_DEFAULT 24

#define RSU_INTERFACE_NAME_MIN       0   /* What does "" mean? should be invalid value for column. */
#define RSU_INTERFACE_NAME_MAX       127 

#define RSU_IFCLOG_STORE_PATH_MIN    1
#define RSU_IFCLOG_STORE_PATH_MAX    255 

#define RSU_IFCLOG_NAME_SIZE_MIN     12
#define RSU_IFCLOG_NAME_SIZE_MAX     172 

#define RSU_IFCLOG_OPTIONS_DEFAULT   0x00
#define RSU_IFCLOG_OPTIONS_DISK_FULL 0x80
#define RSU_IFCLOG_OPTIONS_DELETE_ENTRY 0x40
#define RSU_IFCLOG_OPTIONS_LENGTH    1    /* Bytes */

/* Internal version control. */
#define INTERFACE_LOG_VERSION 1

typedef struct {
    int32_t     rsuIfaceLogIndex;
    onOff_e     rsuIfaceGenerate;
    int32_t     rsuIfaceMaxFileSize;
    int32_t     rsuIfaceMaxFileTime;
    ifclogDir_e rsuIfaceLogByDir;
    uint8_t     rsuIfaceName[RSU_INTERFACE_NAME_MAX];
    int32_t     rsuIfaceName_length;
    uint8_t     rsuIfaceStoragePath[RSU_IFCLOG_STORE_PATH_MAX];
    int32_t     rsuIfaceStoragePath_length;
    uint8_t     rsuIfaceLogName[RSU_IFCLOG_NAME_SIZE_MAX];
    int32_t     rsuIfaceLogName_length;
    uint8_t     rsuIfaceLogStart[MIB_DATEANDTIME_LENGTH];
    uint8_t     rsuIfaceLogStop[MIB_DATEANDTIME_LENGTH];
    uint64_t    start_utc_dsec;
    uint64_t    stop_utc_dsec;
    uint8_t     rsuIfaceLogOptions;
    int32_t     rsuIfaceLogStatus;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuInterfaceLogEntry_t;

void install_rsuInterfaceLog_handlers(void);
void destroy_rsuInterfaceLog(void);
void set_default_row_interfaceLog(RsuInterfaceLogEntry_t * InterfaceLog, int32_t index); /* Set single instance only. */
int32_t rebuild_interfaceLog_live(RsuInterfaceLogEntry_t * InterfaceLog);
int32_t commit_interfaceLog_to_disk(RsuInterfaceLogEntry_t * InterfaceLog);
int32_t update_interfaceLog_shm(RsuInterfaceLogEntry_t * InterfaceLog);
/* 5.8.1 */
int32_t get_maxRsuInterfaceLogs(void);
/* 5.8.2.1 */
int32_t  get_rsuIfaceLogIndex(int32_t index, int32_t * data_out);
/* 5.8.2.2 */ 
int32_t get_rsuIfaceGenerate(int32_t index, int32_t * data_out);
int32_t preserve_rsuIfaceGenerate(int32_t index);
int32_t action_rsuIfaceGenerate(int32_t index, int32_t data_in);
int32_t commit_rsuIfaceGenerate(int32_t index);
int32_t undo_rsuIfaceGenerate(int32_t index);
/* 5.8.2.3 */ 
int32_t get_rsuIfaceMaxFileSize(int32_t index, int32_t * data_out);
int32_t preserve_rsuIfaceMaxFileSize(int32_t index);
int32_t action_rsuIfaceMaxFileSize(int32_t index, int32_t data_in);
int32_t commit_rsuIfaceMaxFileSize(int32_t index);
int32_t undo_rsuIfaceMaxFileSize(int32_t index);
/* 5.8.2.4 */ 
int32_t get_rsuIfaceMaxFileTime(int32_t index, int32_t * data_out);
int32_t preserve_rsuIfaceMaxFileTime(int32_t index);
int32_t action_rsuIfaceMaxFileTime(int32_t index, int32_t data_in);
int32_t commit_rsuIfaceMaxFileTime(int32_t index);
int32_t undo_rsuIfaceMaxFileTime(int32_t index);
/* 5.8.2.5 */ 
int32_t get_rsuIfaceLogByDir(int32_t index, int32_t * data_out);
int32_t preserve_rsuIfaceLogByDir(int32_t index);
int32_t action_rsuIfaceLogByDir(int32_t index, int32_t data_in);
int32_t commit_rsuIfaceLogByDir(int32_t index);
int32_t undo_rsuIfaceLogByDir(int32_t index);
/* 5.8.2.6 */
int32_t get_rsuIfaceName(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceName(int32_t index);
int32_t action_rsuIfaceName(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceName(int32_t index);
int32_t undo_rsuIfaceName(int32_t index);
/* 5.8.2.7 */
int32_t get_rsuIfaceStoragePath(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceStoragePath(int32_t index);
int32_t action_rsuIfaceStoragePath(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceStoragePath(int32_t index);
int32_t undo_rsuIfaceStoragePath(int32_t index);
/* 5.8.2.9 */
int32_t get_rsuIfaceLogName(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceLogName(int32_t index);
int32_t action_rsuIfaceLogName(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceLogName(int32_t index);
int32_t undo_rsuIfaceLogName(int32_t index);
/* 5.8.2.9 */
int32_t get_rsuIfaceLogStart(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceLogStart(int32_t index);
int32_t action_rsuIfaceLogStart(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceLogStart(int32_t index);
int32_t undo_rsuIfaceLogStart(int32_t index);
/* 5.8.2.10 */
int32_t get_rsuIfaceLogStop(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceLogStop(int32_t index);
int32_t action_rsuIfaceLogStop(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceLogStop(int32_t index);
int32_t undo_rsuIfaceLogStop(int32_t index);
/* 5.8.2.11 */
int32_t get_rsuIfaceLogOptions(int32_t index, uint8_t * data_out);
int32_t preserve_rsuIfaceLogOptions(int32_t index);
int32_t action_rsuIfaceLogOptions(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuIfaceLogOptions(int32_t index);
int32_t undo_rsuIfaceLogOptions(int32_t index);
/* 5.8.2.12 */
int32_t get_rsuIfaceLogStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuIfaceLogStatus(int32_t index);
int32_t action_rsuIfaceLogStatus(int32_t index, int32_t data_in);
int32_t commit_rsuIfaceLogStatus(int32_t index);
int32_t undo_rsuIfaceLogStatus(int32_t index);

/* RSU App support. Reality is tcpdump only supports single instance. */

/* SHM definition is modified version of RsuReceivedMsgEntry_t */
typedef struct {
    onOff_e     ifaceGenerate; /* logmgr will write back when done. */
    int32_t     ifaceMaxFileSize;
    int32_t     ifaceMaxFileTime;
    ifclogDir_e ifaceLogByDir;
    uint8_t     ifaceName[RSU_INTERFACE_NAME_MAX];
    int32_t     ifaceName_length;
    uint8_t     ifaceStoragePath[RSU_IFCLOG_STORE_PATH_MAX];
    int32_t     ifaceStoragePath_length;
    uint8_t     ifaceLogName[RSU_IFCLOG_NAME_SIZE_MAX]; /* ifclogger will write when file completed & moved. */
    int32_t     ifaceLogName_length;
    uint8_t     ifaceLogOptions;
    int32_t     ifaceLogStatus;
    uint8_t     rsuID[32]; /* really RSU_ID_LENGTH_MAX defined below */
    int32_t     rsuID_length;    
    uint8_t     dirty; /* Update from Client has occured. Clear when dirty handled. */
} interfaceLog_t;

/* Location of OID data stored on disk. */
#define INTERFACE_LOG_OUTPUT_PATH   "/rwflash/I2V/ntcip-1218/ifclog"

#define INTERFACE_LOG_SHM_PATH "/ntcip_ifclog_shm"
void *interfaceLogThr(void __attribute__((unused)) *arg);

/*Dupe definition from i2v_types.h.*/
#if defined(MY_UNIT_TEST)
#define IFC_COMPLETED_LOGS_DIR   "/tmp/rwflash/I2V/ifclogs"
#else
#define IFC_COMPLETED_LOGS_DIR   "/rwflash/I2V/ifclogs"
#endif

/******************************************************************************
 * 5.9.1 - 5.9.16: { rsu 8 }: Security: rsuSecAppCertTable and rsuSecProfileTable
 *   
 * typedef struct {
 *     int32_t  rsuSecAppCertIndex;                      RO: 1..maxRsuSecAppCerts.
 *     uint8_t  rsuSecAppCertPsid[RSU_SEC_APP_PSID_MAX]; RO: 1..255: "up to 64": should be 4..252?: 64 * 4 = 256? Padded to 4 octets.
 *     int32_t  rsuSecAppCertState;                      RO: 1..4: 1 = other, 2 = valid, 3 = notValid, 4 = future: validty= time + region.
 *     int32_t  rsuSecAppCertExpiration;                 RO: 0..255: hours: 0 = expire today, 255 = expired, 1..54 = hours till expire.
 *     int32_t  rsuSecAppCertReq;                        RO: 0..655535: hours before rsuSecAppCertExpiration RSU make req new creds.
 * } RsuSecAppCertEntry_t;
 * 
 * typedef struct {
 *     int32_t rsuSecProfileIndex;                                       RO: 1 to maxRsuSecProfiles: index must be 1 or greater. 
 *     uint8_t  rsuSecProfileName[RSU_SECURITY_PROFILE_NAME_LENGTH_MAX];  RO: 1..127: name of profile.
 *     uint8_t  rsuSecProfileDesc[RSU_SECURITY_PROFILE_DESC_LENGTH_MAX];  RO: 0..255: description. 
 * } RsuSecProfileEntry_t;
 * 
 * 5.9.1
 * int32_t                  rsuSecCredReq;
 * rsuSecEnrollCertStatus_e rsuSecEnrollCertStatus;
 * int32_t                  rsuSecEnrollCertValidRegion;
 *
 * 5.9.4 Enrollment Certificate Source Domain: URI-TC-MIB: Uri255: 0..255: OCTET STRING: Normalized(see below):
 *   A Uniform Resource Identifier (URI) as defined by STD 66. Objects using this TEXTUAL-CONVENTION MUST be in US-ASCII encoding, 
 *   and MUST be normalized as described by RFC 3986 Sections 6.2.1, 6.2.2.1, and 6.2.2.2. All unnecessary percent-encoding is removed, 
 *   and all case-insensitive characters are set to lowercase except for hexadecimal digits, which are normalized to uppercase 
 *   as described in Section 6.2.2.1. The purpose of this normalization is to help provide unique URIs. Note that this normalization 
 *   is not sufficient to provide uniqueness. Two URIs that are textually distinct after this normalization may still be equivalent. 
 *   Objects using this TEXTUAL-CONVENTION MAY restrict the schemes that they permit. For example, 'data:' and 'urn:' schemes might 
 *   not be appropriate. A zero-length URI is not a valid URI. This can be used to express 'URI absent' where required, for example 
 *   when used as an index field. STD 66 URIs are of unlimited length. Objects using this TEXTUAL-CONVENTION impose a length limit 
 *   on the URIs that they can represent. Where no length restriction is required, objects SHOULD use the 'Uri' TEXTUAL-CONVENTION instead.
 *   Objects used as indices SHOULD subtype the 'Uri' TEXTUAL-CONVENTION.
 *
 * uint8_t               rsuSecEnrollCertUrl[URI255_LENGTH_MAX];
 * uint8_t               rsuSecEnrollCertId[RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX];
 * uint8_t               rsuSecEnrollCertExpiration[MIB_DATEANDTIME_LENGTH];
 * rsuSecuritySource_e   rsuSecuritySource;
 * uint8_t               rsuSecAppCertUrl[URI1024_LENGTH_MAX];
 *
 * 5.9.9
 * int32_t               maxRsuSecAppCerts;
 * RsuSecAppCertEntry_t  rsuSecAppCertTable[RSU_SEC_APP_CERTS_MAX];
 *
 * 5.9.11
 * uint8_t               rsuSecCertRevocationUrl[URI255_LENGTH_MAX];
 * uint8_t               rsuSecCertRevocationTime[MIB_DATEANDTIME_LENGTH];
 * int32_t               rsuSecCertRevocationInterval;
 * int32_t               rsuSecCertRevocationUpdate;
 *
 * 5.9.15
 * int32_t               maxRsuSecProfiles;
 * RsuSecProfileEntry_t  rsuSecProfileTable[RSU_SECURITY_PROFILES_MAX];
 * 
 ******************************************************************************/

/* 5.9.1 Security Credentials Request. */
#define RSU_SECURITY_CRED_REQ_HOURS_MIN     0
#define RSU_SECURITY_CRED_REQ_HOURS_MAX     8736
#define RSU_SECURITY_CRED_REQ_HOURS_DEFAULT 168

/* 5.9.2 Enrollment Certificate Status */
typedef enum {
    enroll_cert_status_other       = 0,
    enroll_cert_status_unknown     = 1,
    enroll_cert_status_notEnrolled = 2,
    enroll_cert_status_enrolled    = 3,
}rsuSecEnrollCertStatus_e;

/* 5.9.3 Enrollment Certificate Valid Region. */
#define RSU_SECURITY_REGION_USA   840
#define RSU_SECURITY_REGION_OTHER 0

/* 5.9.5 Enrollment Certificate Identifier: hashID8. */
#define RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MIN 0
#define RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX 255

/* 5.9.7 Enrollment Certificate Source. */
typedef enum {
    security_source_other  = 1,
    security_source_scms   = 2,
    security_source_manual = 3,
}rsuSecuritySource_e;

/* 5.9.10.1 to 5.9.10.5 */

#define RSU_SEC_APP_CERTS_MIN  0
#define RSU_SEC_APP_CERTS_MAX  32  /* map,spat,tim,bsm,srm,ssm,psm...there's how many MSG ID's? */

#define RSU_SEC_APP_PSID_MIN  1 
#define RSU_SEC_APP_PSID_MAX  255  /* "upto 64" and including or not including? */

#define RSU_SEC_APP_CERT_STATE_MIN  1 /*1 = other, 2 = valid, 3 = notValid, 4 = future: validty= time + region.*/
#define RSU_SEC_APP_CERT_STATE_MAX  4

#define RSE_SEC_APP_CERT_EXP_MIN  0
#define RSE_SEC_APP_CERT_EXP_MAX  255

#define RSU_SEC_APP_CERT_REQ_MIN  0
#define RSU_SEC_APP_CERT_REQ_MAX  65535

typedef struct {
    int32_t  rsuSecAppCertIndex;
    uint8_t  rsuSecAppCertPsid[RSU_SEC_APP_PSID_MAX];
    int32_t  rsuSecAppCertPsid_length;
    int32_t  rsuSecAppCertState;
    int32_t  rsuSecAppCertExpiration;
    int32_t  rsuSecAppCertReq;
} __attribute__((packed)) RsuSecAppCertEntry_t;

/* 5.9.13 Certificate Revocation Update Interval in hours. */
#define RSU_SECURITY_CERT_REVOCATION_UPDATE_MIN     0
#define RSU_SECURITY_CERT_REVOCATION_UPDATE_MAX     255
#define RSU_SECURITY_CERT_REVOCATION_UPDATE_DEFAULT 24

/* 5.9.15 Max Security Profiles in rsuSecProfileTable. */
#define RSU_SECURITY_PROFILES_MIN  1
#define RSU_SECURITY_PROFILES_MAX  100 /* Can be upto 255. */

/* 5.9.16 Security Profiles: rsuSecProfileTable */
#define RSU_SECURITY_PROFILE_NAME_LENGTH_MIN 1
#define RSU_SECURITY_PROFILE_NAME_LENGTH_MAX 127

#define RSU_SECURITY_PROFILE_DESC_LENGTH_MIN 0
#define RSU_SECURITY_PROFILE_DESC_LENGTH_MAX 255

typedef struct {
    int32_t rsuSecProfileIndex;
    uint8_t rsuSecProfileName[RSU_SECURITY_PROFILE_NAME_LENGTH_MAX];
    int32_t rsuSecProfileName_length;
    uint8_t rsuSecProfileDesc[RSU_SECURITY_PROFILE_DESC_LENGTH_MAX];
    int32_t rsuSecProfileDesc_length;
} __attribute__((packed)) RsuSecProfileEntry_t;

void install_rsuSecurity_handlers(void);
void destroy_rsuSecurity(void);

/* 5.9.1 */
int32_t get_rsuSecCredReq(void);
int32_t preserve_rsuSecCredReq(void);
int32_t action_rsuSecCredReq(int32_t data_in);
int32_t commit_rsuSecCredReq(void);
int32_t undo_rsuSecCredReq(void);
/* 5.9.2 */
int32_t get_rsuSecEnrollCertStatus(void);
/* 5.9.3 */
int32_t get_rsuSecEnrollCertValidRegion(void);
/* 5.9.4 */
int32_t get_rsuSecEnrollCertUrl(uint8_t * data_out);
/* 5.9.5 */
int32_t get_rsuSecEnrollCertId(uint8_t * data_out);
/* 5.9.6 */
int32_t get_rsuSecEnrollCertExpiration(uint8_t * data_out);
/* 5.9.7 */
int32_t get_rsuSecuritySource(void);
/* 5.9.8 */
int32_t get_rsuSecAppCertUrl(uint8_t * data_out);
/* 5.9.9 */
int32_t get_maxRsuSecAppCerts(void);
/* 5.9.10.2 */
int32_t get_rsuSecAppCertPsid(int32_t index, uint8_t * data_out);
/* 5.9.10.3 */
int32_t get_rsuSecAppCertState(int32_t index);
/* 5.9.10.4 */
int32_t get_rsuSecAppCertExpiration(int32_t index);
/* 5.9.10.5 */ 
int32_t get_rsuSecAppCertReq(int32_t index);
int32_t preserve_rsuSecAppCertReq(int32_t index);
int32_t action_rsuSecAppCertReq(int32_t index, int32_t data_in);
int32_t commit_rsuSecAppCertReq(int32_t index);
int32_t undo_rsuSecAppCertReq(int32_t index);
/* 5.9.11 */
int32_t get_rsuSecCertRevocationUrl(uint8_t * data_out);
/* 5.9.12 */
int32_t get_rsuSecCertRevocationTime(uint8_t * data_out);
/* 5.9.13 */
int32_t get_rsuSecCertRevocationInterval(void);
int32_t preserve_rsuSecCertRevocationInterval(void);
int32_t action_rsuSecCertRevocationInterval(int32_t data_in);
int32_t commit_rsuSecCertRevocationInterval(void);
int32_t undo_rsuSecCertRevocationInterval(void);
/* 5.9.14 */
int32_t get_rsuSecCertRevocationUpdate(void);
int32_t preserve_rsuSecCertRevocationUpdate(void);
int32_t action_rsuSecCertRevocationUpdate(int32_t data_in);
int32_t commit_rsuSecCertRevocationUpdate(void);
int32_t undo_rsuSecCertRevocationUpdate(void);
/* 5.9.15 */
int32_t get_maxRsuSecProfiles(void);
/* 5.9.16.2 */
int32_t get_rsuSecProfileName(int32_t index, uint8_t * data_out);
/* 5.9.16.3 */
int32_t get_rsuSecProfileDesc(int32_t index, uint8_t * data_out);

/******************************************************************************
 * 5.10 WAVE Service Advertisement: { rsu 9 }: rsuWsaConfig.
 * 
 *   5.10.1               Max WAVE Service Ad: maxRsuWsaServices.
 *   5.10.2.2 - 5.10.2.16 WSA Table          : rsuWsaServiceTable.
 *   5.10.3.1 - 5.10.3.6  WSA Chan Table     : rsuWsaChannelTable.
 *   5.10.4               WSA Version        : rsuWsaVersion.  
 * 
 * typedef struct {
 *     int32_t rsuWsaIndex;                            RO: 1 to maxRsuWsaServices. Must be 1 or greater.
 *     uint8_t rsuWsaPsid[RSU_RADIO_PSID_SIZE];        RC: 1..4 OCTETS: not fixed length. 
 *     int32_t rsuWsaPriority;                         RC: 1..7: no default.
 *     uint8_t rsuWsaPSC[WSA_PSC_LENGTH_MAX];          RC: 0..31: 0 = omit from WSA.
 *     uint8_t rsuWsaIpAddress[RSU_DEST_IP_MAX];       RC: 0..64: ie '2031:0:130F::9C0:876A:130B':if empty then?
 *     int32_t rsuWsaPort;                             RC: 1024..65535
 *     int32_t rsuWsaChannel;                          RC: 0..255: dsrc is 172 to 184: cv2x is Table 7 of NTCIP 1218 v01.
 *     int32_t rsuWsaStatus;                           RC: Create (4) & (6) destroy: notInService & notReady?
 *     uint8_t rsuWsaMacAddress[RSU_RADIO_MAC_LENGTH]; RC: IEEE 1609.3-2016 dot3ProviderMacAddress
 * 
 *
 *     IEEE 1609.3-2016 dot3WsaType 
 *     IEEE 1609.3-2016 dot3ProviderIpService 
 *     IEEE 1609.3-2016 dot3ProviderRepeatRate 
 *     IEEE 1609.3-2016 dot3ProviderRcpiThreshold
 *     
 *     BIT 0: Security   : 0=no, 1=yes: Use Security.
 *     BIT 1: WRA        : 0=no, 1=yes: Tx WAVE Routing Advertisement with WSA.
 *     BIT 2: Repeat Rate: 0=no, 1=yes: Tx repeat rate sent with WSA.
 *     BIT 3: 2DLoc      : 0=no, 1=yes: Tx rsuLocationLat, rsuLocationLong: Only if Bit 4 not set. 
 *     BIT 4: 3DLoc      : 0=no, 1=yes: Tx rsuLocationLat, rsuLocationLong,rsuLocationElv.
 *     BIT 5: RcpiThresh : 0=no, 1=yes: Tx RcpiThreshold sent in WSA Header.
 *     BIT 6: Reserved
 *     BIT 7: Reserved
 *     
 *     uint8_t  rsuWsaOptions;        RC: bitmap 0 to 7
 * 
 *     int32_t  rsuWsaRcpiThreshold;  RC:0..255:coded per IEEE Std 802.11-2012/18.3.1  
 * 
 *     The recommended number of received WSAs within the number of 100 ms intervals defined by rsuWsaCountThresholdInterval 
 *     before accepting the advertised service.
 *        
 *     int32_t  rsuWsaCountThreshold; RC:0..255:msecs 
 * 
 *     The number of 100 millisecond intervals over which to count received WSAs before accepting the advertised service. 
 *     Used with rsuWsaCountThreshold. A value of 0 indicates the WSA count threshold and WSA count threshold interval 
 *     is not transmitted in the IEEE 1609.3 WSA header.
 *     
 *     int32_t  rsuWsaCountThresholdInterval;                    RC:0..255:msecs
 * 
 *     int32_t  rsuWsaRepeatRate;                                RC:0..255: wsa count per 5 seconds
 * 
 *     unit8_t  rsuWsaAdvertiserIdentifier[WSA_AD_ID_LENGTH_MAX]; RC:0..31: SIZE(0) = do not send.
 *  
 *     1 = RSU will do consistency check rsuWsaChannelTable (5.10.4), determined by PSID(rsuWsaChannelPsid). 
 *     If fails then set to 0.'genError' is returned, and an error is logged.
 *     
 *     int32_t rsuWsaEnable;  RC: 1|0 : 1 = row enabled, send WSA after validation, 0 = disabled.
 *
 * } RsuWsaServiceEntry_t;
 *
 * typedef struct {
 *     int32_t rsuWsaChannelIndex;                     RO: 1 to maxRsuWsaServices = RSU_WSA_SERVICES_MAX.
 *     uint8_t rsuWsaChannelPsid[RSU_RADIO_PSID_SIZE]; RC: 1..4 OCTETS: not fixed length. See 5.10.2 rsuWsaPsid.
 *     int32_t rsuWsaChannelNumber;                    RC: 0..255: dsrc 172 to 184: cv2x Table 7 of NTCIP 1218 v01.
 *     int32_t rsuWsaChannelTxPowerLevel;              RC: -127..128:dbm:IEEE 1609.3-2016 dot3ProviderChannelInfoTransmitPowerLevel.
 *     int32_t rsuWsaChannelAccess;                    RC: 0=both,1=slot0,2=slot1,3=notUsed:IEEE1609.3-2016 dot3ProviderChannelAccess.
 *     int32_t rsuWsaChannelStatus;                    RC: Create (4) & (6) destroy:
 * } RsuWsaChannelEntry_t;
 * 
 ******************************************************************************/

/* 5.10.1 Maximum Number of WSA Services. */
#define RSU_WSA_SERVICES_MIN 1
#define RSU_WSA_SERVICES_MAX 100

/* 5.10.2.1 - 5.10.2.16: WSA Table: rsuWsaServiceTable. */
#define WSA_PRIORITY_MIN 1
#define WSA_PRIORITY_MAX 7

#define WSA_PSC_LENGTH_MIN 0
#define WSA_PSC_LENGTH_MAX 31

#define WSA_IP_ADDR_LENGTH_MIN 0
#define WSA_IP_ADDR_LENGTH_MAX 64

#define WSA_PORT_MIN      1024
#define WSA_PORT_MAX      65535
#define WSA_PORT_DEFAULT  WSA_PORT_MIN 

#define WSA_RCPI_THRESHOLD_MIN 0
#define WSA_RCPI_THRESHOLD_MAX 255

#define WSA_AD_ID_LENGTH_MIN 0
#define WSA_AD_ID_LENGTH_MAX 31

#define WSA_OPTIONS_DEFAULT  0x00
#define WSA_OPTIONS_LENGTH   1    /* Bytes */

#define WSA_COUNT_THRESHOLD_MIN 0
#define WSA_COUNT_THRESHOLD_MAX 255

#define WSA_COUNT_THRESHOLD_INTERVAL_MIN 0
#define WSA_COUNT_THRESHOLD_INTERVAL_MAX 255

#define WSA_REPEAT_RATE_MIN 0
#define WSA_REPEAT_RATE_MAX 255

typedef struct {
    int32_t  rsuWsaIndex;
    uint8_t  rsuWsaPsid[RSU_RADIO_PSID_SIZE];
    int32_t  rsuWsaPsid_length;
    int32_t  rsuWsaPriority;
    uint8_t  rsuWsaPSC[WSA_PSC_LENGTH_MAX];
    int32_t  rsuWsaPSC_length;
    uint8_t  rsuWsaIpAddress[RSU_DEST_IP_MAX];
    int32_t  rsuWsaIpAddress_length;
    int32_t  rsuWsaPort;
    int32_t  rsuWsaChannel;
    int32_t  rsuWsaStatus;
    uint8_t  rsuWsaMacAddress[RSU_RADIO_MAC_LENGTH];
    uint8_t  rsuWsaOptions; 
    int32_t  rsuWsaRcpiThreshold;
    int32_t  rsuWsaCountThreshold;
    int32_t  rsuWsaCountThresholdInterval; 
    int32_t  rsuWsaRepeatRate; 
    uint8_t  rsuWsaAdvertiserIdentifier[WSA_AD_ID_LENGTH_MAX];
    int32_t  rsuWsaAdvertiserIdentifier_length;
    int32_t  rsuWsaEnable; 
} __attribute__((packed)) RsuWsaServiceEntry_t;

/*  5.10.3.1 to 5.10.3.6 */
#define WSA_CHANNEL_ACCESS_MIN 0
#define WSA_CHANNEL_ACCESS_MAX 3

typedef struct {
    int32_t rsuWsaChannelIndex;
    uint8_t rsuWsaChannelPsid[RSU_RADIO_PSID_SIZE];
    uint8_t rsuWsaChannelPsid_length;
    int32_t rsuWsaChannelNumber;
    int32_t rsuWsaChannelTxPowerLevel;
    int32_t rsuWsaChannelAccess;
    int32_t rsuWsaChannelStatus;
} __attribute__((packed)) RsuWsaChannelEntry_t;

/* 5.10.4 WSA Version. */
#define RSU_WSA_VERSION 3 /* 3 = IEEE 1609.3-2016 */

void install_rsuWsaConfig_handlers(void);
void destroy_rsuWsaConfig(void);
void set_default_row_rsuWsaService(RsuWsaServiceEntry_t * rsuWsaService, int32_t index); /* Set single instance only. */
void set_default_row_rsuWsaChannel(RsuWsaChannelEntry_t * rsuWsaChannel, int32_t index); /* Set single instance only. */
int32_t rebuild_rsuWsaService_live(RsuWsaServiceEntry_t * rsuWsaService);
int32_t rebuild_rsuWsaChannel_live(RsuWsaChannelEntry_t * rsuWsaChannel);

/* 5.10.1 */
int32_t get_maxRsuWsaServices(void);
/* 5.10.2.1 */
int32_t get_rsuWsaIndex(int32_t index, int32_t * data_out);
/* 5.10.2.2 */
int32_t get_rsuWsaPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaPsid(int32_t index);
int32_t action_rsuWsaPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaPsid(int32_t index);
int32_t undo_rsuWsaPsid(int32_t index);
/* 5.10.2.3 */
int32_t get_rsuWsaPriority(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaPriority(int32_t index);
int32_t action_rsuWsaPriority(int32_t index, int32_t data_in);
int32_t commit_rsuWsaPriority(int32_t index);
int32_t undo_rsuWsaPriority(int32_t index);
/* 5.10.2.4 */
int32_t get_rsuWsaPSC(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaPSC(int32_t index);
int32_t action_rsuWsaPSC(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaPSC(int32_t index);
int32_t undo_rsuWsaPSC(int32_t index);
/* 5.10.2.5 */
int32_t get_rsuWsaIpAddress(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaIpAddress(int32_t index);
int32_t action_rsuWsaIpAddress(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaIpAddress(int32_t index);
int32_t undo_rsuWsaIpAddress(int32_t index);
/* 5.10.2.6 */
int32_t get_rsuWsaPort(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaPort(int32_t index);
int32_t action_rsuWsaPort(int32_t index, int32_t data_in);
int32_t commit_rsuWsaPort(int32_t index);
int32_t undo_rsuWsaPort(int32_t index);
/* 5.10.2.7 */
int32_t get_rsuWsaChannel(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaChannel(int32_t index);
int32_t action_rsuWsaChannel(int32_t index, int32_t data_in);
int32_t commit_rsuWsaChannel(int32_t index);
int32_t undo_rsuWsaChannel(int32_t index);
/* 5.10.2.8 */
int32_t get_rsuWsaStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaStatus(int32_t index);
int32_t action_rsuWsaStatus(int32_t index, int32_t data_in);
int32_t commit_rsuWsaStatus(int32_t index);
int32_t undo_rsuWsaStatus(int32_t index);
/* 5.10.2.9 */
int32_t get_rsuWsaMacAddress(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaMacAddress(int32_t index);
int32_t action_rsuWsaMacAddress(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaMacAddress(int32_t index);
int32_t undo_rsuWsaMacAddress(int32_t index);
/* 5.10.2.10 */
int32_t get_rsuWsaOptions(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaOptions(int32_t index);
int32_t action_rsuWsaOptions(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaOptions(int32_t index);
int32_t undo_rsuWsaOptions(int32_t index);
/* 5.10.2.11 RW */
int32_t get_rsuWsaRcpiThreshold(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaRcpiThreshold(int32_t index);
int32_t action_rsuWsaRcpiThreshold(int32_t index, int32_t data_in);
int32_t commit_rsuWsaRcpiThreshold(int32_t index);
int32_t undo_rsuWsaRcpiThreshold(int32_t index);
/* 5.10.2.12 */
int32_t get_rsuWsaCountThreshold(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaCountThreshold(int32_t index);
int32_t action_rsuWsaCountThreshold(int32_t index, int32_t data_in);
int32_t commit_rsuWsaCountThreshold(int32_t index);
int32_t undo_rsuWsaCountThreshold(int32_t index);
/* 5.10.2.13 */
int32_t get_rsuWsaCountThresholdInterval(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaCountThresholdInterval(int32_t index);
int32_t action_rsuWsaCountThresholdInterval(int32_t index, int32_t data_in);
int32_t commit_rsuWsaCountThresholdInterval(int32_t index);
int32_t undo_rsuWsaCountThresholdInterval(int32_t index);
/* 5.10.2.14 */
int32_t get_rsuWsaRepeatRate(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaRepeatRate(int32_t index);
int32_t action_rsuWsaRepeatRate(int32_t index, int32_t data_in);
int32_t commit_rsuWsaRepeatRate(int32_t index);
int32_t undo_rsuWsaRepeatRate(int32_t index);
/* 5.10.2.15 */
int32_t get_rsuWsaAdvertiserIdentifier(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaAdvertiserIdentifier(int32_t index);
int32_t action_rsuWsaAdvertiserIdentifier(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaAdvertiserIdentifier(int32_t index);
int32_t undo_rsuWsaAdvertiserIdentifier(int32_t index);
/* 5.10.2.16 */
int32_t get_rsuWsaEnable(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaEnable(int32_t index);
int32_t action_rsuWsaEnable(int32_t index, int32_t data_in);
int32_t commit_rsuWsaEnable(int32_t index);
int32_t undo_rsuWsaEnable(int32_t index);
/* 5.10.3.1 */
int32_t get_rsuWsaChannelIndex(int32_t index, int32_t * data_out);
/* 5.10.3.2 */
int32_t get_rsuWsaChannelPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuWsaChannelPsid(int32_t index);
int32_t action_rsuWsaChannelPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuWsaChannelPsid(int32_t index);
int32_t undo_rsuWsaChannelPsid(int32_t index);
/* 5.10.3.3 */
int32_t get_rsuWsaChannelNumber(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaChannelNumber(int32_t index);
int32_t action_rsuWsaChannelNumber(int32_t index, int32_t data_in);
int32_t commit_rsuWsaChannelNumber(int32_t index);
int32_t undo_rsuWsaChannelNumber(int32_t index);
/* 5.10.3.4 */
int32_t get_rsuWsaChannelTxPowerLevel(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaChannelTxPowerLevel(int32_t index);
int32_t action_rsuWsaChannelTxPowerLevel(int32_t index, int32_t data_in);
int32_t commit_rsuWsaChannelTxPowerLevel(int32_t index);
int32_t undo_rsuWsaChannelTxPowerLevel(int32_t index);
/* 5.10.3.5 */
int32_t get_rsuWsaChannelAccess(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaChannelAccess(int32_t index);
int32_t action_rsuWsaChannelAccess(int32_t index, int32_t data_in);
int32_t commit_rsuWsaChannelAccess(int32_t index);
int32_t undo_rsuWsaChannelAccess(int32_t index);
/* 5.10.3.6 */
int32_t get_rsuWsaChannelStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuWsaChannelStatus(int32_t index);
int32_t action_rsuWsaChannelStatus(int32_t index, int32_t data_in);
int32_t commit_rsuWsaChannelStatus(int32_t index);
int32_t undo_rsuWsaChannelStatus(int32_t index);
/* 5.10.4 */
int32_t get_rsuWsaVersion(void);

/******************************************************************************
 * 5.11.1 - 5.11.7: WAVE Router AD Configuration: { rsu 10 }: IPv6 & MAC info
 *
 *  uint8_t rsuWraIpPrefix[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraIpPrefixLength;
 *  uint8_t rsuWraGateway[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraPrimaryDns[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraSecondaryDns[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraGatewayMacAddress[RSU_RADIO_MAC_LENGTH];
 *  int32_t rsuWraLifetime;
 *
 ******************************************************************************/

/* 5.11.7 WSA router Lifetime: IEEE 1609.3-2016 dot3ProviderWaveRoutingAdvertisementRouterLifetime:  IETF RFC 4861 */
#define RSU_WRA_LIFETIME_MIN  0      /* do not use 0 for default routers */
#define RSU_WRA_LIFETIME_MAX  65535

void install_rsuWraConfig_handlers(void);
void destroy_rsuWraConfig(void);

/* 5.11.1 */
int32_t get_rsuWraIpPrefix(uint8_t * data_out);
int32_t preserve_rsuWraIpPrefix(void);
int32_t action_rsuWraIpPrefix(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraIpPrefix(void);
int32_t undo_rsuWraIpPrefix(void);
/* 5.11.2 */
int32_t get_rsuWraIpPrefixLength(uint8_t * data_out);
int32_t preserve_rsuWraIpPrefixLength(void);
int32_t action_rsuWraIpPrefixLength(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraIpPrefixLength(void);
int32_t undo_rsuWraIpPrefixLength(void);
/* 5.11.3 */
int32_t get_rsuWraGateway(uint8_t * data_out);
int32_t preserve_rsuWraGateway(void);
int32_t action_rsuWraGateway(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraGateway(void);
int32_t undo_rsuWraGateway(void);
/* 5.11.4 */
int32_t get_rsuWraPrimaryDns(uint8_t * data_out);
int32_t preserve_rsuWraPrimaryDns(void);
int32_t action_rsuWraPrimaryDns(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraPrimaryDns(void);
int32_t undo_rsuWraPrimaryDns(void);
/* 5.11.5 */
int32_t get_rsuWraSecondaryDns(uint8_t * data_out);
int32_t preserve_rsuWraSecondaryDns(void);
int32_t action_rsuWraSecondaryDns(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraSecondaryDns(void);
int32_t undo_rsuWraSecondaryDns(void);
/* 5.11.6 */
int32_t get_rsuWraGatewayMacAddress(uint8_t * data_out);
int32_t preserve_rsuWraGatewayMacAddress(void);
int32_t action_rsuWraGatewayMacAddress(uint8_t * data_in, int32_t length);
int32_t commit_rsuWraGatewayMacAddress(void);
int32_t undo_rsuWraGatewayMacAddress(void);
/* 5.11.7 */
int32_t get_rsuWraLifetime(void);
int32_t preserve_rsuWraLifetime(void);
int32_t action_rsuWraLifetime(int32_t data_in);
int32_t commit_rsuWraLifetime(void);
int32_t undo_rsuWraLifetime(void);

/******************************************************************************
 * 5.12.1 - 5.12.2 Message Statistics: { rsu 11 }: rsuMessageCountsByPsidTable: tx&rx per PSID.
 *
 *   int32_t                       maxRsuMessageCountsByPsid;
 *   RsuMessageCountsByPsidEntry_t rsuMessageCountsByPsidTable[RSU_PSID_TRACKED_STATS_MAX];
 *
 *   typedef struct {
 *       int32_t  rsuMessageCountsByPsidIndex;                        RO: 1..maxRsuMessageCountsByPsid.
 *       uint8_t  rsuMessageCountsByPsidId[RSU_RADIO_PSID_SIZE];      RC: 0..4 OCTETS: 0x0 = any PSID.
 *       int32_t  rsuMessageCountsByChannel;                          RC: 0..255: dsrc is 172 to 184: 0 = all *used* channels.
 *       int32_t  rsuMessageCountsDirection;                          RC: 1 = inbound, 2 = outbound, 3 = bothdir.
 *       uint8_t  rsuMessageCountsByPsidTime[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *       uint32_t rsuMessageCountsByPsidCounts;                       RO: count since boot: Counter32 == rolls over.
 *       int32_t  rsuMessageCountsByPsidRowStatus;                    RC: Create (4) & (6) destroy:
 *   } RsuMessageCountsByPsidEntry_t;
 *
 ******************************************************************************/

/* 5.12.1 Max PSID's tracked in stats table. */
#define RSU_PSID_TRACKED_STATS_MIN 1
#define RSU_PSID_TRACKED_STATS_MAX 32

/* 5.12.2 RSU Message Count Table: 5.12.2.1 to 5.12.2.7. */
#define RSU_MESSAGE_COUNT_BY_CHANNEL_MIN 0
#define RSU_MESSAGE_COUNT_BY_CHANNEL_MAX 255

#define RSU_MESSAGE_COUNT_DIRECTION_MIN  1
#define RSU_MESSAGE_COUNT_DIRECTION_MAX  3

#define RSU_MESSAGE_COUNT_DIRECTION_IN   1
#define RSU_MESSAGE_COUNT_DIRECTION_OUT  2
#define RSU_MESSAGE_COUNT_DIRECTION_BOTH 3

/* Internal version control. */
#define MSG_STATS_VERSION 1

typedef struct {
    int32_t  rsuMessageCountsByPsidIndex;
    uint8_t  rsuMessageCountsByPsidId[RSU_RADIO_PSID_SIZE];
    int32_t  rsuMessageCountsByPsidId_length;
    int32_t  rsuMessageCountsByChannel;
    int32_t  rsuMessageCountsDirection;
    uint8_t  rsuMessageCountsByPsidTime[MIB_DATEANDTIME_LENGTH];
    uint32_t rsuMessageCountsByPsidCounts;
    int32_t  rsuMessageCountsByPsidRowStatus;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuMessageCountsByPsidEntry_t;

void install_rsuMessageStats_handlers(void);
void destroy_rsuMessageStats(void);
int32_t rebuild_rsuMessageStats_live(RsuMessageCountsByPsidEntry_t * MessageStats);
int32_t commit_rsuMessageStats_to_disk(RsuMessageCountsByPsidEntry_t * MessageStats);
int32_t update_messageStats_shm(RsuMessageCountsByPsidEntry_t * MessageStats);
int32_t get_mib_DateAndTime(uint8_t * DateAndTime);

/* 5.12.1 */
int32_t get_maxRsuMessageCountsByPsid(void);
/* 5.12.2.1 */
int32_t  get_rsuMessageCountsByPsidIndex(int32_t index, int32_t * data_out);
/* 5.12.2.2 */
int32_t get_rsuMessageCountsByPsidId(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMessageCountsByPsidId(int32_t index);
int32_t action_rsuMessageCountsByPsidId(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMessageCountsByPsidId(int32_t index);
int32_t undo_rsuMessageCountsByPsidId(int32_t index);
/* 5.12.2.3 */
int32_t get_rsuMessageCountsByChannel(int32_t index, int32_t * data_out);
int32_t preserve_rsuMessageCountsByChannel(int32_t index);
int32_t action_rsuMessageCountsByChannel(int32_t index, int32_t data_in);
int32_t commit_rsuMessageCountsByChannel(int32_t index);
int32_t undo_rsuMessageCountsByChannel(int32_t index);
/* 5.12.2.4 */
int32_t get_rsuMessageCountsDirection(int32_t index, int32_t * data_out);
int32_t preserve_rsuMessageCountsDirection(int32_t index);
int32_t action_rsuMessageCountsDirection(int32_t index, int32_t data_in);
int32_t commit_rsuMessageCountsDirection(int32_t index);
int32_t undo_rsuMessageCountsDirection(int32_t index);
/* 5.12.2.5 */
int32_t get_rsuMessageCountsByPsidTime(int32_t index, uint8_t * data_out);
int32_t preserve_rsuMessageCountsByPsidTime(int32_t index);
int32_t action_rsuMessageCountsByPsidTime(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuMessageCountsByPsidTime(int32_t index);
int32_t undo_rsuMessageCountsByPsidTime(int32_t index);
/* 5.12.2.6 */
int32_t get_rsuMessageCountsByPsidCounts(int32_t index, uint32_t * data_out);
/* 5.12.2.7 */
int32_t get_rsuMessageCountsByPsidRowStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuMessageCountsByPsidRowStatus(int32_t index);
int32_t action_rsuMessageCountsByPsidRowStatus(int32_t index, int32_t data_in);
int32_t commit_rsuMessageCountsByPsidRowStatus(int32_t index);
int32_t undo_rsuMessageCountsByPsidRowStatus(int32_t index);

/* RSU App support. */

/* SHM definition is modified version of RsuMessageStatsEntry_t */
typedef struct {
    uint32_t psid;
    int32_t  channel;
    int32_t  direction;
    uint64_t start_utc_dsec;
    uint32_t count;
    int32_t  rowStatus;
    uint8_t  dirty; /* Update from Client has occured. Clear when dirty handled. */
} messageStats_t;

/* Location of OID data stored on disk. */
#define MSG_STATS_OUTPUT_PATH   "/rwflash/I2V/ntcip-1218/msgStats"
/* Seconds: Rate app updates SHM for client. */
#define MSG_STATS_APP_SHM_UPDATE_RATE  1 /* Not instant update. */
#define MSG_STATS_SHM_PATH "/ntcip_msgStats_shm"
void *messageStatsThr(void __attribute__((unused)) *arg);

/******************************************************************************
 * 5.13 Systems Statistics: { rsu 12 }: rsuCommRangeTable
 * 
 * typedef struct {
 *     int32_t rsuCommRangeIndex;       RO: 1..maxRsuCommRange
 *     int32_t rsuCommRangeSector;      RC: 1..16 : sector = 22.5 deg: 16 sectors. 1 is north then east by 22.5 deg etc.
 *     int32_t rsuCommRangeMsgId;       RC: 0..32767: 0 = any: DE_DSRC_MessageID of J2735.
 *     int32_t rsuCommRangeFilterType;  RC: 1 = noFilter, 2 = vehicleType, 3 = vehicleClass: DE_VehicleType J2735.
 *     int32_t rsuCommRangeFilterValue; RC: 0..255: value of vehicleType or vehicleClass: addtional filter to rsuCommRangeMsgId.
 *     int32_t rsuCommRange1Min;        RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres:furtherest.
 *     int32_t rsuCommRange5Min;        RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres.furtherest.
 *     int32_t rsuCommRange15Min;       RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres.furtherest.
 *     int32_t rsuCommRangeAvg1Min;     RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeAvg5Min;     RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeAvg15Min;    RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeStatus;      RC: Create (4) & (6) destroy:   
 * } RsuCommRangeEntry_t;
 * 
 ******************************************************************************/

/* 5.13.2 RSU Internal Temperature. */
#define RSU_TEMP_CELSIUS_MIN -101 /* default */
#define RSU_TEMP_CELSIUS_MAX 100

/* 5.13.3 */
#define RSU_TEMP_LOW_CELSIUS_MIN -101
#define RSU_TEMP_LOW_CELSIUS_MAX 100
#define RSU_TEMP_LOW_CELSIUS_THRESHOLD  -40

/* 5.13.4 */
#define RSU_TEMP_HIGH_CELSIUS_MIN -101
#define RSU_TEMP_HIGH_CELSIUS_MAX 100
#define RSU_TEMP_HIGH_CELSIUS_THRESHOLD  85

/* 5.13.5 Maximum Number of Communications Range Entries. */
#define RSU_COMM_RANGE_MIN 1
#define RSU_COMM_RANGE_MAX 16 /* Enough to do all sectors. */

/* 5.13.6 RSU Communications Range Table. */
#define RSU_COMM_RANGE_SECTOR_MIN  1  /* Due north then east by 22.5 degrees. */
#define RSU_COMM_RANGE_SECTOR_MAX  16 /* Due north then west by 22.5 degrees. */

#define RSU_COMM_RANGE_MSG_ID_MIN  0
#define RSU_COMM_RANGE_MSG_ID_MAX  32767
#define RSU_COMM_RANGE_MSG_ID_DEFAULT  20 /* From DSRC.h ASN1V_basicSafetyMessage */

#define RSU_COMM_RANGE_FILTER_TYPE_MIN 1
#define RSU_COMM_RANGE_FILTER_TYPE_MAX 3

#define RSU_COMM_RANGE_FILTER_VALUE_MIN 0
#define RSU_COMM_RANGE_FILTER_VALUE_MAX 255

#define RSU_COMM_RANGE_MINUTES_MIN 0
#define RSU_COMM_RANGE_MINUTES_MAX 2001

/* Internal version control. */
#define COMM_RANGE_VERSION 1

typedef struct {
    int32_t rsuCommRangeIndex;
    int32_t rsuCommRangeSector;
 /* 
  * rsuCommRangeMsgId: SAE J2735 Message Identifier (DE_DSRC_MessageID).
  * See DSRC.h, i.e. "ASN1V_basicSafetyMessage 20".
  */
    int32_t rsuCommRangeMsgId;
    int32_t rsuCommRangeFilterType;
    int32_t rsuCommRangeFilterValue;
    int32_t rsuCommRange1Min;
    int32_t rsuCommRange5Min;
    int32_t rsuCommRange15Min;
    int32_t rsuCommRangeAvg1Min;
    int32_t rsuCommRangeAvg5Min;
    int32_t rsuCommRangeAvg15Min;
    int32_t rsuCommRangeStatus;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */  
} __attribute__((packed)) RsuCommRangeEntry_t;

void install_rsuSystemStats_handlers(void);
void destroy_rsuSystemStats(void);
void set_default_row_rsuCommRange(RsuCommRangeEntry_t * rsuCommRange, int32_t index); /* Set single instance only. */
int32_t rebuild_rsuCommRange_live(RsuCommRangeEntry_t * rsuCommRange);
/* 5.13.1 */
int32_t get_rsuTimeSincePowerOn(uint32_t * data_out);
/* 5.13.2 */
/* Ambient(internal) temp in C: Divide by 1000 then subtract 64, see /usr/src/wnc/scripts/temperature_get.sh. */
#define AMBIENT_TEMP_FILE "/sys/class/hwmon/hwmon0/temp1_input"
int32_t get_rsuIntTemp(int32_t * data_out);
/* 5.13.3 */
int32_t get_rsuIntTempLowThreshold(int32_t * data_out);
/* 5.13.4 */
int32_t get_rsuIntTempHighThreshold(int32_t * data_out);
/* 5.13.5 */
int32_t get_maxRsuCommRange(int32_t * data_out);
/* 5.13.6.1 */
int32_t get_rsuCommRangeIndex(int32_t index, int32_t * data_out);
/* 5.13.6.2 */
int32_t get_rsuCommRangeSector(int32_t index, int32_t * data_out);
int32_t preserve_rsuCommRangeSector(int32_t index);
int32_t action_rsuCommRangeSector(int32_t index, int32_t data_in);
int32_t commit_rsuCommRangeSector(int32_t index);
int32_t undo_rsuCommRangeSector(int32_t index);
/* 5.13.6.3 */
int32_t get_rsuCommRangeMsgId(int32_t index, int32_t * data_out);
int32_t preserve_rsuCommRangeMsgId(int32_t index);
int32_t action_rsuCommRangeMsgId(int32_t index, int32_t data_in);
int32_t commit_rsuCommRangeMsgId(int32_t index);
int32_t undo_rsuCommRangeMsgId(int32_t index);
/* 5.13.6.4 */
int32_t get_rsuCommRangeFilterType(int32_t index, int32_t * data_out);
int32_t preserve_rsuCommRangeFilterType(int32_t index);
int32_t action_rsuCommRangeFilterType(int32_t index, int32_t data_in);
int32_t commit_rsuCommRangeFilterType(int32_t index);
int32_t undo_rsuCommRangeFilterType(int32_t index);
/* 5.13.6.5 */
int32_t get_rsuCommRangeFilterValue(int32_t index, int32_t * data_out);
int32_t preserve_rsuCommRangeFilterValue(int32_t index);
int32_t action_rsuCommRangeFilterValue(int32_t index, int32_t data_in);
int32_t commit_rsuCommRangeFilterValue(int32_t index);
int32_t undo_rsuCommRangeFilterValue(int32_t index);
/* 5.13.6.6 */
int32_t get_rsuCommRange1Min(int32_t index, int32_t * data_out);
/* 5.13.6.7 */
int32_t get_rsuCommRange5Min(int32_t index, int32_t * data_out);
/* 5.13.6.8 */
int32_t get_rsuCommRange15Min(int32_t index, int32_t * data_out);
/* 5.13.6.9 */
int32_t get_rsuCommRangeAvg1Min(int32_t index, int32_t * data_out);
/* 5.13.6.10 */
int32_t get_rsuCommRangeAvg5Min(int32_t index, int32_t * data_out);
/* 5.13.6.11 */
int32_t get_rsuCommRangeAvg15Min(int32_t index, int32_t * data_out);
/* 5.13.6.12 */
int32_t get_rsuCommRangeStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuCommRangeStatus(int32_t index);
int32_t action_rsuCommRangeStatus(int32_t index, int32_t data_in);
int32_t commit_rsuCommRangeStatus(int32_t index);
int32_t undo_rsuCommRangeStatus(int32_t index);

int32_t commit_commRange_to_disk(RsuCommRangeEntry_t * commRange);

/* RSU App support. */

/* SHM definition. */

/* Slightly modified def of RsuCommRangeEntry_t */
typedef struct {
    int32_t CommRangeStatus;
    int32_t CommRangeSector;
    int32_t CommRangeMsgId;
    int32_t CommRangeFilterType;
    int32_t CommRangeFilterValue;
    int32_t CommRange1Min_max; /* Max seen this test epoch. */
    int32_t CommRange5Min_max;
    int32_t CommRange15Min_max;
    int32_t CommRangeAvg1Min; /* Average is total divided by count. */
    int32_t CommRangeAvg5Min;
    int32_t CommRangeAvg15Min;
    int32_t CommRange1Min_count;
    int32_t CommRange5Min_count;
    int32_t CommRange15Min_count;
    int32_t CommRange1Min_total;
    int32_t CommRange5Min_total;
    int32_t CommRange15Min_total;
    uint64_t CommRange1Min_time; /* Start time in UTC msec. */
    uint64_t CommRange5Min_time;
    uint64_t CommRange15Min_time;
} CommRangeStats_t;

/* Location of OID data stored on disk. */
#define COMM_RANGE_OUTPUT_PATH   "/rwflash/I2V/ntcip-1218/sysStats"

#define COMM_RANGE_SHM_PATH "/ntcip_commRange_shm"
void *commRangeThr(void __attribute__((unused)) *arg); /* Handles MIB <=> RSU via SHM. */

/******************************************************************************
 * 5.14.1 - 5.14.15: System Description: { rsu 13 }
 * 
 *  uint8_t            rsuMibVersion[RSUMIB_VERSION_LENGTH_MAX];
 *  uint8_t            rsuFirmwareVersion[RSU_FIRMWARE_VERSION_LENGTH_MAX];
 *  uint8_t            rsuLocationDesc[RSU_LOCATION_DESC_LENGTH_MAX];
 *  uint8_t            rsuID[RSU_ID_LENGTH_MAX];
 *  int32_t            rsuLocationLat;
 *  int32_t            rsuLocationLon;
 *  int32_t            rsuLocationElv;
 *  int32_t            rsuElevationOffset;
 *  int32_t            rsuInstallUpdate;
 *  uint8_t            rsuInstallFile[RSU_INSTALL_FILE_NAME_LENGTH_MAX];
 *  uint8_t            rsuInstallPath[RSU_INSTALL_PATH_LENGTH_MAX];
 *  rsuInstallStatus_e rsuInstallStatus;
 *  uint8_t            rsuInstallTime[MIB_DATEANDTIME_LENGTH]; 
 *  uint8_t            rsuInstallStatusMessage[RSU_INSTALL_STATUS_MSG_LENGTH_MAX]; 
 *  uint8_t            rsuScheduledInstallTime[MIB_DATEANDTIME_LENGTH]; 
 *
 ******************************************************************************/

/* 5.14.1 RSU MIB Version. */
#define RSUMIB_VERSION_LENGTH_MIN 0
#define RSUMIB_VERSION_LENGTH_MAX 32
 
/* 5.14.2 RSU Firmware Version. */
#define RSU_FIRMWARE_VERSION_LENGTH_MIN 0
#define RSU_FIRMWARE_VERSION_LENGTH_MAX 32

/* 5.14.3 RSU Location Description. */
#define RSU_LOCATION_DESC_LENGTH_MIN  0
#define RSU_LOCATION_DESC_LENGTH_MAX  140
#define RSU_LOCATION_DESC_DEFAULT     "DENSO RSU location site."

/* 5.14.4 RSU Identifier. */
#define RSU_ID_LENGTH_MIN  0
#define RSU_ID_LENGTH_MAX  32
#define RSU_ID_DEFAULT     "RSU1"

/* 5.14.8 RSU Ref Elv Offset. */
#define RSU_REF_ELV_OFFSET_MIN  0
#define RSU_REF_ELV_OFFSET_MAX  2001 /*default.*/

/* 5.14.10 RSU Install Filename. */
#define RSU_INSTALL_FILE_NAME_LENGTH_MIN  0
#define RSU_INSTALL_FILE_NAME_LENGTH_MAX  255
#define RSU_INSTALL_FILE_NAME_DEFAULT "densonor.bz2"

/* 5.14.11 RSU Installation Path. */
#define RSU_INSTALL_PATH_LENGTH_MIN  1
#define RSU_INSTALL_PATH_LENGTH_MAX  255
#define RSU_INSTALL_PATH_DEFAULT     "/rwflash/customer"

/* 5.14.12 RSU Install Status. */
typedef enum {
    installOther       = 1,
    installUnknown     = 2,
    installRejected    = 3,
    installRollback    = 4,
    installProcessing  = 5,
    installSuccess     = 6,    
} rsuInstallStatus_e;

/* 5.14.14 */
#define RSU_INSTALL_STATUS_MSG_LENGTH_MIN  0
#define RSU_INSTALL_STATUS_MSG_LENGTH_MAX  255
#define RSU_INSTALL_STATUS_MSG_DEFAULT "Factory installSuccess"

/* Internal version control. */
#define SYS_DESC_VERSION 1

/* struct only used to read+write to disk nicely.*/
typedef struct {
    /* 5.14.1 - 5.14.15 */
    uint8_t            rsuMibVersion[RSUMIB_VERSION_LENGTH_MAX];
    int32_t            rsuMibVersion_length;
    uint8_t            rsuFirmwareVersion[RSU_FIRMWARE_VERSION_LENGTH_MAX];
    int32_t            rsuFirmwareVersion_length;
    uint8_t            rsuLocationDesc[RSU_LOCATION_DESC_LENGTH_MAX];
    int32_t            rsuLocationDesc_length;
    uint8_t            rsuID[RSU_ID_LENGTH_MAX];
    int32_t            rsuID_length;
    int32_t            rsuLocationLat;
    int32_t            rsuLocationLon;
    int32_t            rsuLocationElv;
    int32_t            rsuElevationOffset;
    int32_t            rsuInstallUpdate;
    uint8_t            rsuInstallFile[RSU_INSTALL_FILE_NAME_LENGTH_MAX];
    int32_t            rsuInstallFile_length;
    uint8_t            rsuInstallPath[RSU_INSTALL_PATH_LENGTH_MAX];
    int32_t            rsuInstallPath_length;
    rsuInstallStatus_e rsuInstallStatus;
    uint8_t            rsuInstallTime[MIB_DATEANDTIME_LENGTH]; 
    uint8_t            rsuInstallStatusMessage[RSU_INSTALL_STATUS_MSG_LENGTH_MAX];
    int32_t            rsuInstallStatusMessage_length; 
    uint8_t            rsuScheduledInstallTime[MIB_DATEANDTIME_LENGTH];
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */
} __attribute__((packed)) SystemDescription_t; 

void install_rsuSysDescription_handlers(void);
void destroy_rsuSysDescription(void);
int32_t commit_sysDesc_to_disk(SystemDescription_t * sysDesc);
void installFirmware(void);

/* 5.14.1 */
int32_t get_rsuMibVersion(uint8_t * data_out);
/* 5.14.2 */
int32_t get_rsuFirmwareVersion(uint8_t * data_out);
/* 5.14.3 */
int32_t get_rsuLocationDesc(uint8_t * data_out);
int32_t preserve_rsuLocationDesc(void);
int32_t action_rsuLocationDesc(uint8_t * data_in, int32_t length);
int32_t commit_rsuLocationDesc(void);
int32_t undo_rsuLocationDesc(void);
/* 5.14.4 */
int32_t get_rsuID(uint8_t * data_out);
int32_t preserve_rsuID(void);
int32_t action_rsuID(uint8_t * data_in, int32_t length);
int32_t commit_rsuID(void);
int32_t undo_rsuID(void);
/* 5.14.5 */
int32_t get_rsuLocationLat(int32_t * data_out);
int32_t preserve_rsuLocationLat(void);
int32_t action_rsuLocationLat(int32_t data_in);
int32_t commit_rsuLocationLat(void);
int32_t undo_rsuLocationLat(void);
/* 5.14.6 */
int32_t get_rsuLocationLon(int32_t * data_out);
int32_t preserve_rsuLocationLon(void);
int32_t action_rsuLocationLon(int32_t data_in);
int32_t commit_rsuLocationLon(void);
int32_t undo_rsuLocationLon(void);
/* 5.14.7 */
int32_t get_rsuLocationElv(int32_t * data_out);
int32_t preserve_rsuLocationElv(void);
int32_t action_rsuLocationElv(int32_t data_in);
int32_t commit_rsuLocationElv(void);
int32_t undo_rsuLocationElv(void);
/* 5.14.8 */
int32_t get_rsuElevationOffset(int32_t * data_out);
int32_t preserve_rsuElevationOffset(void);
int32_t action_rsuElevationOffset(int32_t data_in);
int32_t commit_rsuElevationOffset(void);
int32_t undo_rsuElevationOffset(void);
/* 5.14.9 */
int32_t get_rsuInstallUpdate(int32_t * data_out);
int32_t preserve_rsuInstallUpdate(void);
int32_t action_rsuInstallUpdate(int32_t data_in);
int32_t commit_rsuInstallUpdate(void);
int32_t undo_rsuInstallUpdate(void);
/* 5.14.10 */
int32_t get_rsuInstallFile(uint8_t * data_out);
int32_t preserve_rsuInstallFile(void);
int32_t action_rsuInstallFile(uint8_t * data_in, int32_t length);
int32_t commit_rsuInstallFile(void);
int32_t undo_rsuInstallFile(void);
/* 5.14.11 */
int32_t get_rsuInstallPath(uint8_t * data_out);
int32_t preserve_rsuInstallPath(void);
int32_t action_rsuInstallPath(uint8_t * data_in, int32_t length);
int32_t commit_rsuInstallPath(void);
int32_t undo_rsuInstallPath(void);
/* 5.14.12 */
int32_t get_rsuInstallStatus(int32_t * data_out);
/* 5.14.13 */
int32_t get_rsuInstallTime(uint8_t * data_out);
/* 5.14.14 */
int32_t get_rsuInstallStatusMessage(uint8_t * data_out);
/* 5.14.15 */
int32_t get_rsuScheduledInstallTime(uint8_t * data_out);
int32_t preserve_rsuScheduledInstallTime(void);
int32_t action_rsuScheduledInstallTime(uint8_t * data_in, int32_t length);
int32_t commit_rsuScheduledInstallTime(void);
int32_t undo_rsuScheduledInstallTime(void);

/* Name of OID MIB data stored on disk. */
#define SYS_DESC_PATH         "/rwflash/I2V/ntcip-1218"
#define SYS_DESC_PATHFILENAME "/rwflash/I2V/ntcip-1218/sysDesc.dat"

/* Firmware Install. */
#define SYS_DESC_NOR_CMD "/usr/local/bin/densonor -F"
#define SYS_DESC_NOR_SUCCESS "NOR flashing complete...Reboot system to activate new firmware."
#define SYS_DESC_INSTALLUPDATE_OUTPUT "/tmp/installUpdate.txt"
#define SYS_DESC_INSTALLUPDATE_RESULT "/bin/cat /tmp/installUpdate.txt | grep \"NOR flashing complete\" "
#define SYS_DESC_INSTALLUPDATE_TIMEOUT  90 /* In ten's of seconds so 15 minutes. */

/* Versions and Dates. */
#define FIRMWARE_READ_SVN_REVISION "/bin/egrep 'SVN_REVISION' /usr/share/BuildInfo.txt"
#define FIRMWARE_READ_BUILD_DATE "/bin/egrep 'BUILD_DATE' /usr/share/BuildInfo.txt"

/* 32 values of binary data or ascii
 * ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 * ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 * 4d 65 61 74 79 42 65 61 74 79 42 69 67 41 6e 64
 * 42 6f 75 6e 63 79 43 61 6e 59 6f 75 44 69 67 31
 */
#define RSU_CUST_DEV_READ_ID       "/usr/local/dnutils/denso_nor_read | head -4 | cut -c 9-"
#define RSU_CUST_DEV_READ_ID_REPLY "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx"

/* csv file format: 
 * 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
 */
#define  RSU_CUST_DEV_WRITE_ID_FILE         "/tmp/mib_cust_dev_id_hex.csv"
#define  RSU_CUST_DEV_WRITE_ID              "/usr/bin/write_cust_devid /tmp/mib_cust_dev_id_hex.csv"
#define  RSU_CUST_DEV_WRITE_ID_FILE_STRING  "0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx,0x%hhx"
#define  RSU_CUST_DEV_WRITE_DONE            "Done.  Write successful!"
/******************************************************************************
 * 5.15 rsuSysSettings: {rsu 14}
 * 
 *     uint8_t         rsuNotifyIpAddress[RSU_DEST_IP_MAX];
 *     int32_t         rsuNotifyPort;
 *     uint8_t         rsuSysLogQueryStart[RFC2579_DATEANDTIME_LENGTH];
 *     uint8_t         rsuSysLogQueryStop[RFC2579_DATEANDTIME_LENGTH];
 *     syslog_level_e  rsuSysLogQueryPriority; 
 *     int32_t         rsuSysLogQueryGenerate; 
 *     syslog_status_e rsuSysLogQueryStatus;
 *     int32_t         rsuSysLogCloseCommand; 
 *     syslog_level_e  rsuSysLogSeverity;
 *     uint8_t         rsuSysConfigId[RSU_SYS_CONFIG_ID_LENGTH_MAX];
 *     int32_t         rsuSysRetries;
 *     int32_t         rsuSysRetryPeriod;
 *     int32_t         rsuShortCommLossTime;
 *     int32_t         rsuLongCommLossTime;
 *     uint8_t         rsuSysLogName[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];
 *     uint8_t         rsuSysDir[RSU_SYSDIR_NAME_LENGTH_MAX];
 *     int32_t         rsuLongCommLossReboot;
 *     uint8_t         rsuHostIpAddr[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostNetMask[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostGateway[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostDNS[RSU_DEST_IP_MAX];
 *     uint32_t        rsuHostDHCPEnable;
 * 
 ******************************************************************************/

/* 5.15.2 Notification Destination Port */
#define RSU_NOTIFY_PORT_DEFAULT 162

/* 5.15.5 Syslog Query Level: RFC 5424 Table 2 */
typedef enum {
    syslog_level_emergency = 0,
    syslog_level_alert     = 1,
    syslog_level_critcal   = 2,
    syslog_level_error     = 3,
    syslog_level_warning   = 4,
    syslog_level_notice    = 5,
    syslog_level_info      = 6,
    syslog_level_debug     = 7,
} syslog_level_e;

/* 5.15.7 Syslog Query Status */
typedef enum  { 
    syslog_status_other       = 1, /* not defined */
    syslog_status_unknown     = 2, /* not in progress */
    syslog_status_progressing = 3, /* query active */
    syslog_status_successful  = 4, /* query complete */
    syslog_status_outOfRange  = 5, /* bad start or stop times */
    syslog_status_badFilename = 6, /* bad file name */
}syslog_status_e;

/* 5.15.10 System Configuration File.*/
#define RSU_SYS_CONFIG_ID_LENGTH_MIN 0
#define RSU_SYS_CONFIG_ID_LENGTH_MAX 128

/* 5.15.11 System Startup Retries. */
#define RSU_SYS_RETRIES_MIN 0
#define RSU_SYS_RETRIES_MAX 15

/* 5.15.12 System Startup Retry Period */
#define RSU_SYS_RETRY_PERIOD_MIN 0   /* default.*/
#define RSU_SYS_RETRY_PERIOD_MAX 1440

/* 5.15.13 Short Comm Loss Time: Station may do GET on object to avoid loss event.*/
#define RSU_SHORT_COMM_LOSS_TIME_MIN 0     /* default.*/
#define RSU_SHORT_COMM_LOSS_TIME_MAX 65535

/* 5.15.14 Long Comm Loss Time: */
#define RSU_LONG_COMM_LOSS_TIME_MIN 0     /* default.*/
#define RSU_LONG_COMM_LOSS_TIME_MAX 65535

/* 5.15.15 System Log Name */
#define RSU_SYSLOG_NAME_AND_PATH_LENGTH_MIN  1                                           
#define RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX  255
#define RSU_SYSLOG_NAME_AND_PATH_DEFAULT     "LogNotReady"

/* 5.15.16 System Log Base Directory */
#define RSU_SYSDIR_NAME_LENGTH_MIN  0
#define RSU_SYSDIR_NAME_LENGTH_MAX  255
/* 
 * Probably should have logging folder and final folder that don't overlap. 
 * No way for user to change this so fixed for now. Probably best they dont.
 */
#if defined (MY_UNIT_TEST)
#define RSU_SYSDIR_DEFAULT  "/tmp/rwflash/I2V/syslogs/"
#else
#define RSU_SYSDIR_DEFAULT  "/rwflash/I2V/syslogs/"
#endif

/* Internal version control. */
#define SYSTEM_SETTINGS_VERSION 1

/* RSU SNMP IP is the RSU eth0 IP below. */
#define DCU_READ_RSU_SNMP_PORT  "/usr/bin/conf_agent READ snmpd agentaddress"
#define DCU_WRITE_RSU_SNMP_PORT "/usr/bin/conf_agent WRITE snmpd agentaddress %d"

typedef struct {

//TODO: SNMPTRAPD: Future work: snmptrapd.conf
    uint8_t         rsuNotifyIpAddress[RSU_DEST_IP_MAX];
    int32_t         rsuNotifyIpAddress_length;
    int32_t         rsuNotifyPort;

//LOGMGR: Superset of conf. Use SHM to support new functionality and give run time support.
//conf_mgr does not support LOGMGR conf so use MIB for now. 
//We can resolve using SHM and 1218 thread in LOGMGR at runtime.
    uint8_t         rsuSysLogQueryStart[RFC2579_DATEANDTIME_LENGTH];
    uint8_t         rsuSysLogQueryStop[RFC2579_DATEANDTIME_LENGTH];
    syslog_level_e  rsuSysLogQueryPriority;
    int32_t         rsuSysLogQueryGenerate;
    syslog_status_e rsuSysLogQueryStatus;
    int32_t         rsuSysLogCloseCommand;
    syslog_level_e  rsuSysLogSeverity;
//LOGMGR: read only
    uint8_t         rsuSysLogName[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];
    int32_t         rsuSysLogName_length;
    uint8_t         rsuSysDir[RSU_SYSDIR_NAME_LENGTH_MAX];
    int32_t         rsuSysDir_length;

//SYSCONF globalSetIDParameter.0?: Furure work. back up configs and have versions to restore.
    uint8_t         rsuSysConfigId[RSU_SYS_CONFIG_ID_LENGTH_MAX];
    int32_t         rsuSysConfigId_length;

//RSUHEALTH: Doesn't have conf. Future work. Impacts msgRepeat and IFM.
//maybe these?    {"RSU_HEALTH_ENABLE", "0", UNINIT}, {"RSU_SYSTEM_MONITOR_ENABLE",
    int32_t         rsuSysRetries;
    int32_t         rsuSysRetryPeriod;
    uint32_t        rsuSysRetryPeriodStart; /* UTC second */
    int32_t         rsuSysRetryAttempts; /* Recover from disk and seed RSUHEALTH SHM with. */
//TODO: SNMP: no activity from management station. future work.
    int32_t         rsuShortCommLossTime;
    int32_t         rsuLongCommLossTime;

//RSUHEALTH: Future work
    int32_t         rsuLongCommLossReboot;

//CONF_MGR domain: omit from disk backup & shm: Needs to be v4 and v6 so need smarts to detect
//TODO: i2v:I2VEnableEthIPv6 : Need to detect in addr prior and set.

#define DCU_READ_ETH0_IPV4_ADDR   "/usr/bin/conf_agent READ RSU_INFO SC0IPADDRESS"
#define DCU_WRITE_ETH0_IPV4_ADDR  "/usr/bin/conf_agent WRITE RSU_INFO SC0IPADDRESS %s"
#define DCU_READ_ETH0_IPV6_ADDR   "/usr/bin/conf_agent READ  i2v I2VEthIPv6Addr"
#define DCU_WRITE_ETH0_IPV6_ADDR  "/usr/bin/conf_agent WRITE i2v I2VEthIPv6Addr %s"
#define DCU_READ_ETH0_IPV6_SCOPE  "/usr/bin/conf_agent READ  i2v I2VEthIPv6Scope"
#define DCU_WRITE_ETH0_IPV6_SCOPE "/usr/bin/conf_agent WRITE i2v I2VEthIPv6Scope %s"
    uint8_t         rsuHostIpAddr[RSU_DEST_IP_MAX];
    int32_t         rsuHostIpAddr_length;

    //RSU_INFO:NETMASK
#define DCU_READ_ETH0_NETMASK  "/usr/bin/conf_agent READ RSU_INFO NETMASK"
#define DCU_WRITE_ETH0_NETMASK "/usr/bin/conf_agent WRITE RSU_INFO NETMASK %s"
    uint8_t         rsuHostNetMask[RSU_DEST_IP_MAX];
    int32_t         rsuHostNetMask_length;

    //access_cfg_ccu.c:    "I2VIPv6DefaultGateway",
    //i2v:I2VIPv6DefaultGateway"
#define DCU_READ_ETH0_GATEWAY  "/usr/bin/conf_agent READ i2v I2VIPv6DefaultGateway"
#define DCU_WRITE_ETH0_GATEWAY "/usr/bin/conf_agent WRITE i2v I2VIPv6DefaultGateway %s"
    uint8_t         rsuHostGateway[RSU_DEST_IP_MAX];
    int32_t         rsuHostGateway_length;

    //TODO:resolve.conf: create or update and invoke.
    //Can have multiple but we can only display one
    //search densoamericas.com
    //nameserver 10.74.76.34
    //nameserver 10.72.58.2
    //nameserver 10.72.33.46

    uint8_t         rsuHostDNS[RSU_DEST_IP_MAX];
    int32_t         rsuHostDNS_length;

    //RSU_INFO:NETWORK_MODE
#define DCU_READ_ETH0_DHCP  "/usr/bin/conf_agent READ RSU_INFO NETWORK_MODE"
#define DCU_WRITE_ETH0_DHCP "/usr/bin/conf_agent WRITE RSU_INFO NETWORK_MODE %d"
    uint32_t        rsuHostDHCPEnable;

    /* These are hidden from Client and for RSU internal use only: noAccess. */
    int32_t  version;       /* Internal revision control. */
    int32_t  status;        /* TBD. */
    uint32_t reserved;      /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;           /* Allow for 32 bit if needed. */
} __attribute__((packed)) RsuSysSetts_t;

void install_rsuSysSettings_handlers(void);
void destroy_rsuSysSettings(void);
int32_t commit_rsuSysSettings_to_disk(void);
int32_t update_rsuSysSettings_shm(void);

/* 5.15.1 */
int32_t get_rsuNotifyIpAddress(uint8_t * data_out);
int32_t preserve_rsuNotifyIpAddress(void);
int32_t action_rsuNotifyIpAddress(uint8_t * data_in, int32_t length);
int32_t commit_rsuNotifyIpAddress(void);
int32_t undo_rsuNotifyIpAddress(void);
/* 5.15.2 */
int32_t get_rsuNotifyPort(void);
int32_t preserve_rsuNotifyPort(void);
int32_t action_rsuNotifyPort(int32_t data_in);
int32_t commit_rsuNotifyPort(void);
int32_t undo_rsuNotifyPort(void);
/* 5.15.3 */
int32_t get_rsuSysLogQueryStart(uint8_t * data_out);
int32_t preserve_rsuSysLogQueryStart(void);
int32_t action_rsuSysLogQueryStart(uint8_t * data_in, int32_t length);
int32_t commit_rsuSysLogQueryStart(void);
int32_t undo_rsuSysLogQueryStart(void);
/* 5.15.4 */
int32_t get_rsuSysLogQueryStop(uint8_t * data_out);
int32_t preserve_rsuSysLogQueryStop(void);
int32_t action_rsuSysLogQueryStop(uint8_t * data_in, int32_t length);
int32_t commit_rsuSysLogQueryStop(void);
int32_t undo_rsuSysLogQueryStop(void);
/* 5.15.5 */
int32_t get_rsuSysLogQueryPriority(void);
int32_t preserve_rsuSysLogQueryPriority(void);
int32_t action_rsuSysLogQueryPriority(int32_t data_in);
int32_t commit_rsuSysLogQueryPriority(void);
int32_t undo_rsuSysLogQueryPriority(void);
/* 5.15.6 */
int32_t get_rsuSysLogQueryGenerate(void);
int32_t preserve_rsuSysLogQueryGenerate(void);
int32_t action_rsuSysLogQueryGenerate(int32_t data_in);
int32_t commit_rsuSysLogQueryGenerate(void);
int32_t undo_rsuSysLogQueryGenerate(void);
/* 5.15.7 */
int32_t get_rsuSysLogQueryStatus(void);
int32_t preserve_rsuSysLogQueryStatus(void);
int32_t action_rsuSysLogQueryStatus(int32_t data_in);
int32_t commit_rsuSysLogQueryStatus(void);
int32_t undo_rsuSysLogQueryStatus(void);
/* 5.15.8 */
int32_t get_rsuSysLogCloseCommand(void);
int32_t preserve_rsuSysLogCloseCommand(void);
int32_t action_rsuSysLogCloseCommand(int32_t data_in);
int32_t commit_rsuSysLogCloseCommand(void);
int32_t undo_rsuSysLogCloseCommand(void);
/* 5.15.9 */
int32_t get_rsuSysLogSeverity(void);
int32_t preserve_rsuSysLogSeverity(void);
int32_t action_rsuSysLogSeverity(int32_t data_in);
int32_t commit_rsuSysLogSeverity(void);
int32_t undo_rsuSysLogSeverity(void);
/* 5.15.10 */
int32_t get_rsuSysConfigId(uint8_t * data_out);
int32_t preserve_rsuSysConfigId(void);
int32_t action_rsuSysConfigId(uint8_t * data_in, int32_t length);
int32_t commit_rsuSysConfigId(void);
int32_t undo_rsuSysConfigId(void);
/* 5.15.11 */
int32_t get_rsuSysRetries(void);
int32_t preserve_rsuSysRetries(void);
int32_t action_rsuSysRetries(int32_t data_in);
int32_t commit_rsuSysRetries(void);
int32_t undo_rsuSysRetries(void);
/* 5.15.12 */
int32_t get_rsuSysRetryPeriod(void);
int32_t preserve_rsuSysRetryPeriod(void);
int32_t action_rsuSysRetryPeriod(int32_t data_in);
int32_t commit_rsuSysRetryPeriod(void);
int32_t undo_rsuSysRetryPeriod(void);
/* 5.15.13 */
int32_t get_rsuShortCommLossTime(void);
int32_t preserve_rsuShortCommLossTime(void);
int32_t action_rsuShortCommLossTime(int32_t data_in);
int32_t commit_rsuShortCommLossTime(void);
int32_t undo_rsuShortCommLossTime(void);
/* 5.15.14 */
int32_t get_rsuLongCommLossTime(void);
int32_t preserve_rsuLongCommLossTime(void);
int32_t action_rsuLongCommLossTime(int32_t data_in);
int32_t commit_rsuLongCommLossTime(void);
int32_t undo_rsuLongCommLossTime(void);
/* 5.15.15 */
int32_t get_rsuSysLogName(uint8_t * data_out);
/* 5.15.16 */
int32_t get_rsuSysDir(uint8_t * data_out);
/* 5.15.17 */  
int32_t get_rsuLongCommLossReboot(void);
int32_t preserve_rsuLongCommLossReboot(void);
int32_t action_rsuLongCommLossReboot(int32_t data_in);
int32_t commit_rsuLongCommLossReboot(void);
int32_t undo_rsuLongCommLossReboot(void);
/* 5.15.18 */
int32_t get_rsuHostIpAddr(uint8_t * data_out);
int32_t preserve_rsuHostIpAddr(void);
int32_t action_rsuHostIpAddr(uint8_t * data_in, int32_t length);
int32_t commit_rsuHostIpAddr(void);
int32_t undo_rsuHostIpAddr(void);
/* 5.15.19 */
int32_t get_rsuHostNetMask(uint8_t * data_out);
int32_t preserve_rsuHostNetMask(void);
int32_t action_rsuHostNetMask(uint8_t * data_in, int32_t length);
int32_t commit_rsuHostNetMask(void);
int32_t undo_rsuHostNetMask(void);
/* 5.15.20 */
int32_t get_rsuHostGateway(uint8_t * data_out);
int32_t preserve_rsuHostGateway(void);
int32_t action_rsuHostGateway(uint8_t * data_in, int32_t length);
int32_t commit_rsuHostGateway(void);
int32_t undo_rsuHostGateway(void);
/* 5.15.21 */
int32_t get_rsuHostDNS(uint8_t * data_out);
int32_t preserve_rsuHostDNS(void);
int32_t action_rsuHostDNS(uint8_t * data_in, int32_t length);
int32_t commit_rsuHostDNS(void);
int32_t undo_rsuHostDNS(void);
/* 5.15.22 */ 
int32_t get_rsuHostDHCPEnable(void);
int32_t preserve_rsuHostDHCPEnable(void);
int32_t action_rsuHostDHCPEnable(int32_t data_in);
int32_t commit_rsuHostDHCPEnable(void);
int32_t undo_rsuHostDHCPEnable(void);

/* RSU App support. */

/* SHM definition is modified version of RsuSysSettings_t */

/* 
 * App side can check value against theirs to see if update needed. 
 * Three RSU apps supported: LOGMGR, RSUHEALTH, SYSCONF.
 * Each has own dirty bit and assuming only one task will access, otherwise this may cause issues.
 */
typedef struct {
    /* LOGMGR handles syslog. Read only for MIB.*/
    uint64_t        start_utc_dsec;
    uint64_t        stop_utc_dsec;
    syslog_level_e  rsuSysLogQueryPriority; 
    int32_t         rsuSysLogQueryGenerate; 
    syslog_status_e rsuSysLogQueryStatus;
    int32_t         rsuSysLogCloseCommand; 
    syslog_level_e  rsuSysLogSeverity;
    uint8_t         rsuSysLogName[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];
    int32_t         rsuSysLogName_length;
    uint8_t         rsuSysDir[RSU_SYSDIR_NAME_LENGTH_MAX];
    int32_t         rsuSysDir_length;
    uint8_t         dirty_syslog;
} sysSetts_t;

/* Location of OID data stored on disk. */
#define SYS_SETTS_OUTPUT_LOCATION   "/rwflash/I2V/ntcip-1218"
#define SYS_SETTS_OUTPUT_LOCATION_NAME   "/rwflash/I2V/ntcip-1218/sysSetts.dat"

#define SYS_SETTS_SHM_PATH "/ntcip_sysSetts_shm"
void *sysSettingsThr(void __attribute__((unused)) *arg);
/* Updates MIB and returns current value. */
int32_t get_sysLogName(uint8_t * sysLogName, int32_t * length);
int32_t get_sysDir(uint8_t * sysDir, int32_t * length);
int32_t get_sysLogQueryStatus(void);
int32_t get_sysLogCloseCommand(void);
int32_t get_sysLogQueryGenerate(void);

/******************************************************************************
 * 5.16 Antenna Settings:{ rsu 15 }: rsuAntennaTable: really fixed scalars.
 *
 *  5.16.1 - 5.16.2:
 *
 *    int32_t           maxRsuAntennas;                     RO: RSU_ANTENNA_MAX
 *    RsuAntennaEntry_t rsuAntennaTable[RSU_ANTENNA_MAX];
 *
 * 5.16.2.1 - 5.16.2.6:
 *
 *    typedef struct {
 *        int32_t rsuAntennaIndex; RO: 1..maxRsuAntennas: Must be 1 or greater. 
 *        int32_t rsuAntLat;       RW: -900000000..900000001:   degrees: float scaled to int: 10^-7: 900000001  = unknown = default.
 *        int32_t rsuAntLong;      RW: -1800000000..1800000001: degrees: float scaled to int: 10^-7: 1800000001 = unknown = default.
 *        int32_t rsuAntElv;       RW: -100000..1000001: cm: 1000001 = unknown = default.
 *        int32_t rsuAntGain;      RW: -128..127: dbm 100th's: includes any cable loss: -128 = unknown =default.
 *        int32_t rsuAntDirection; RW: 0..361: deg: 0=north, 90=east etc: 360=uniform gain in horizontal plane: 361=unknown=default.
 *    } RsuAntennaEntry_t;
 *                                  
 ******************************************************************************/

/* 5.16.1: Maximum Antennas */
#define RSU_ANTENNA_MIN 1
#define RSU_ANTENNA_MAX 6 /* 2 for dsrc, 2 for cv2x, 2 for gnss. */

/* 5.16.2: Antenna Table. */
#define RSU_ANTENNA_GAIN_MIN     -128 /* default == unknown. */
#define RSU_ANTENNA_GAIN_MAX     127 
#define RSU_ANTENNA_GAIN_DEFAULT RSU_ANTENNA_GAIN_MIN

#define RSU_ANTENNA_DIRECTION_MIN      0
#define RSU_ANTENNA_DIRECTION_MAX      361 /* default == unknown. */
#define RSU_ANTENNA_DIRECTION_DEFAULT  RSU_ANTENNA_DIRECTION_MAX 

/* Internal version control. */
#define RSU_ANTENNA_VERSION 1

typedef struct {
    int32_t rsuAntennaIndex;
    int32_t rsuAntLat;
    int32_t rsuAntLong;
    int32_t rsuAntElv;
    int32_t rsuAntGain;
    int32_t rsuAntDirection;
    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;                                       /* Allow for 32 bit if needed. */  
} __attribute__((packed)) RsuAntennaEntry_t;

void install_rsuAntenna_handlers(void);
void destroy_rsuAntenna(void);
void set_default_row_rsuAntenna(RsuAntennaEntry_t * rsuAntenna, int32_t index);
int32_t commit_rsuAntenna_to_disk(RsuAntennaEntry_t * rsuAntenna); /* Will write single row to AMH_ACTIVE_DIR for AMH to accept or reject. */

/* 5.16.1 */
int32_t get_maxRsuAntennas(void);
/* 5.16.2.2 */
int32_t get_rsuAntLat(int32_t index, int32_t * data_out);
int32_t preserve_rsuAntLat(int32_t index);
int32_t action_rsuAntLat(int32_t index, int32_t data_in);
int32_t commit_rsuAntLat(int32_t index);
int32_t undo_rsuAntLat(int32_t index);
/* 5.16.2.3 */
int32_t get_rsuAntLong(int32_t index, int32_t * data_out);
int32_t preserve_rsuAntLong(int32_t index);
int32_t action_rsuAntLong(int32_t index, int32_t data_in);
int32_t commit_rsuAntLong(int32_t index);
int32_t undo_rsuAntLong(int32_t index);
/* 5.16.2.4 */
int32_t get_rsuAntElv(int32_t index, int32_t * data_out);
int32_t preserve_rsuAntElv(int32_t index);
int32_t action_rsuAntElv(int32_t index, int32_t data_in);
int32_t commit_rsuAntElv(int32_t index);
int32_t undo_rsuAntElv(int32_t index);
/* 5.16.2.5 */
int32_t get_rsuAntGain(int32_t index, int32_t * data_out);
int32_t preserve_rsuAntGain(int32_t index);
int32_t action_rsuAntGain(int32_t index, int32_t data_in);
int32_t commit_rsuAntGain(int32_t index);
int32_t undo_rsuAntGain(int32_t index);
/* 5.16.2.6 */
int32_t get_rsuAntDirection(int32_t index);
int32_t preserve_rsuAntDirection(int32_t index);
int32_t action_rsuAntDirection(int32_t index, int32_t data_in);
int32_t commit_rsuAntDirection(int32_t index);
int32_t undo_rsuAntDirection(int32_t index);

/* Location of OID data stored on disk. */
#define ANTENNA_OUTPUT_PATH   "/rwflash/I2V/ntcip-1218/antenna"

/******************************************************************************
 * 5.17.1 - 5.17.10: rsuSystemStatus : { rsu 16 }
 *
 *    rsuChanStatus_e     rsuChanStatus;
 *    rsuMode_e           rsuMode;
 *    rsuModeStatus_e     rsuModeStatus;
 *    int32_t             rsuReboot;
 *    clockSource_e       rsuClockSource;
 *    clockSourceStatus_e rsuClockSourceStatus;
 *    int32_t             rsuClockSourceTimeout;
 *    int32_t             rsuClockSourceFailedQuery;
 *    int32_t             rsuClockDeviationTolerance;
 *    rsuStatus_e         rsuStatus;
 *
 ******************************************************************************/

/* 5.17.1 RSU Radio Status: Have to check at runtime for status. Not just conf setup. */
typedef enum  {
    chanStat_bothOp = 0,
    chanStat_altOp  = 1,
    chanStat_contOp = 2,
    chanStat_noneOp = 3, /* default. */
}rsuChanStatus_e;

/* 5.17.2 RSU Mode. */
typedef enum  {
    rsuMode_other     = 1, /* SET to rsuMode_other is illegal. Return SNMP_ERR_BADVALUE. */
    rsuMode_standby   = 2, /* radios disabled and no ifclog. */
    rsuMode_operate   = 3, /* default: go. */
}rsuMode_e;

/* 5.17.3 RSU Mode Status: MIB will have to poll RSU till its up. Show status here. */
typedef enum  {
    rsuModeStatus_other   = 1, /* I'm sorry Dave, the RSU is non-functional. */
    rsuModeStatus_standby = 2, /* radios disabled and no ifclog. */
    rsuModeStatus_operate = 3,
    rsuModeStatus_fault   = 4, /* standby + some other faults but not enough to stop uploads and firmware upgrades. */  
}rsuModeStatus_e; 

/* 5.17.5 RSU Clock Source */
typedef enum  { 
    clockSource_other   = 1, 
    clockSource_crystal = 2,
    clockSource_gnss    = 3,
    clockSource_ntp     = 4, 
}clockSource_e;

/* 5.17.6 RSU Clock Status: Boot pending update? */
typedef enum  {  
    clockSourceStatus_other         = 1, /* is this prior to being active like boot time? */
    clockSourceStatus_active        = 2, /* TPS set to active. */
    clockSourceStatus_pendingUpdate = 3, /* default: boot pending update. */
}clockSourceStatus_e;

/* 5.17.7 RSU Clock Source Timeout */
#define RSU_CLOCK_SOURCE_TIMEOUT_MIN      0
#define RSU_CLOCK_SOURCE_TIMEOUT_MAX      3600
#define RSU_CLOCK_SOURCE_TIMEOUT_DEFAULT  0   /* default: disabled. */

/* 5.17.8 RSU Clock Source Queries: Doesn't really fit in with the concept of 1 PPS. Its solid till GNSS goes away. */
#define RSU_CLOCK_SOURCE_FAILED_QUERY_MIN  0  /* default = disabled. */
#define RSU_CLOCK_SOURCE_FAILED_QUERY_MAX  15

/* 5.17.9 RSU Clock Deviation Tolerance */
#define RSU_CLOCK_DEVIATION_MIN  0        /* default = disabled. */
#define RSU_CLOCK_DEVIATION_MAX  65535   

/* 5.17.10 RSU System Status. */
typedef enum  {  
    rsuStatus_other    = 1,  /* Boot state? */
    rsuStatus_okay     = 2,  /* Boot State? No detected errors in rsuServiceTable. */
    rsuStatus_warning  = 3,  /* RSU operational but conf issue potentially. */
    rsuStatus_critical = 4,  /* Loss of time source or one service critcal. */
    rsuStatus_unknown  = 5,  /* Boot state? */
}rsuStatus_e;

void install_rsuSysStatus_handlers(void);
void destroy_rsuSysStatus(void);

/* 5.17.1 */
int32_t get_rsuChanStatus(void);
/* 5.17.2 */
int32_t get_rsuMode(void);
int32_t preserve_rsuMode(void);
int32_t action_rsuMode(int32_t data_in);
int32_t commit_rsuMode(void);
int32_t undo_rsuMode(void);
/* 5.17.3 */
int32_t get_rsuModeStatus(void);
/* 5.17.4 */
int32_t get_rsuReboot(void);
int32_t preserve_rsuReboot(void);
int32_t action_rsuReboot(int32_t data_in);
int32_t commit_rsuReboot(void);
int32_t undo_rsuReboot(void);
/* 5.17.5 */
int32_t get_rsuClockSource(void);
/* 5.17.6 */
int32_t get_rsuClockSourceStatus(void);
/* 5.17.7 */
int32_t get_rsuClockSourceTimeout(void);
int32_t preserve_rsuClockSourceTimeout(void);
int32_t action_rsuClockSourceTimeout(int32_t data_in);
int32_t commit_rsuClockSourceTimeout(void);
int32_t undo_rsuClockSourceTimeout(void);
/* 5.17.8 */
int32_t get_rsuClockSourceFailedQuery(void);
int32_t preserve_rsuClockSourceFailedQuery(void);
int32_t action_rsuClockSourceFailedQuery(int32_t data_in);
int32_t commit_rsuClockSourceFailedQuery(void);
int32_t undo_rsuClockSourceFailedQuery(void);
/* 5.17.9 */
int32_t get_rsuClockDeviationTolerance(void);
int32_t preserve_rsuClockDeviationTolerance(void);
int32_t action_rsuClockDeviationTolerance(int32_t data_in);
int32_t commit_rsuClockDeviationTolerance(void);
int32_t undo_rsuClockDeviationTolerance(void);
/* 5.17.10 */
int32_t get_rsuStatus(void);

/*****************************************************************************
 * 5.18.1 - 5.18.4: Asynchronous Message: {rsu 17}: RO from customer POV.
 *
 *   rsuAsync_t Async;
 *   int32_t    rsuNotificationRepeatInterval;
 *   int32_t    rsuNotificationMaxRetries;
 ****************************************************************************/

/* 5.18.1 - 5. 18.2 */

#define RSU_ALERT_MSG_LENGTH_MIN  0
#define RSU_ALERT_MSG_LENGTH_MAX  255
#define RSU_ALERT_MSG_DEFAULT     "All systems nominal."

#define RSU_NOTIFY_INTERVAL_RATE_MIN  0
#define RSU_NOTIFY_INTERVAL_RATE_MAX  18000

/* Internal version control. */
#define RSU_ASYNC_VERSION 1

typedef enum  {  
    rsuAlertLevel_info     = 0,
    rsuAlertLevel_notice   = 1,
    rsuAlertLevel_warning  = 2,
    rsuAlertLevel_error    = 3,
    rsuAlertLevel_critical = 4,
    /* We could check msg length and if NULL but more work and what does _info and NULL mean? */  
    rsuAlertLevel_denso_empty = 5 /* For SHM only: So we can tell there is nothing to do i.e. no error or notifcation. */
}rsuAlertLevel_e;

#define RSU_ALERT_DEFAULT rsuAlertLevel_denso_empty; 

typedef struct  {

    int32_t rsuTrapEnable; /* hidden: DENSO: 1 = enable, 0 = disable. Not part of MIB: default = disable. */

    /* 5.18.1.1 File Integrity Check Error Message: AMH SAR & IMF: i.e. No valid message frame in UPER payload. */
    rsuAlertLevel_e messageFileIntegrityError;
    uint8_t         rsuMsgFileIntegrityMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuMsgFileIntegrity_length;
    int32_t         rsuMsgFileIntegrityAttempts;
    int32_t         rsuMsgFileIntegrityInterval;

    /* 5.18.1.2 Storage Integrity Error Message: Errors in file system. */
    rsuAlertLevel_e rsuSecStorageIntegrityError;
    uint8_t         rsuSecStorageIntegrityMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuSecStorageIntegrity_length;
    int32_t         rsuSecStorageIntegrityAttempts;
    int32_t         rsuSecStorageIntegrityInterval;

    /* 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one? */
    rsuAlertLevel_e rsuAuthError;
    uint8_t         rsuAuthMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuAuth_length;
    int32_t         rsuAuthAttempts;
    int32_t         rsuAuthInterval;

    /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
    rsuAlertLevel_e rsuSignatureVerifyError;
    uint8_t         rsuSignatureVerifyMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuSignatureVerify_length;
    int32_t         rsuSignatureVerifyAttempts;
    int32_t         rsuSignatureVerifyInterval;

    /* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
    rsuAlertLevel_e rsuAccessError;
    uint8_t         rsuAccessMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuAccess_length;
    int32_t         rsuAccessAttempts;
    int32_t         rsuAccessInterval;

    /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix. */
    rsuAlertLevel_e rsuTimeSourceLost;
    uint8_t         rsuTimeSourceLostMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuTimeSourceLost_length;
    int32_t         rsuTimeSourceLostAttempts;
    int32_t         rsuTimeSourceLostInterval;

    /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.*/
    rsuAlertLevel_e rsuTimeSourceMismatch;
    uint8_t         rsuTimeSourceMismatchMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuTimeSourceMismatch_length;
    int32_t         rsuTimeSourceMismatchAttempts;
    int32_t         rsuTimeSourceMismatchInterval;
    /*
     * 5.18.1.8 GNSS Anomaly Message: report any anomalous GNSS readings: Sky's the limit here.
     *
     *   ie: long TTFF, weak signals, antenna fault, moving above certain speed, old almanac, no leap second...
     *   Have to be careful because in the eyes of user this is seen as automatcally bad when could be no issue. 
     *     
     */
    rsuAlertLevel_e rsuGnssAnomaly;
    uint8_t         rsuGnssAnomalyMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuGnssAnomaly_length;
    int32_t         rsuGnssAnomalyAttempts;
    int32_t         rsuGnssAnomalyInterval;

    /* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed. */
    rsuAlertLevel_e rsuGnssDeviationError;
    uint8_t         rsuGnssDeviationMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuGnssDeviation_length;
    int32_t         rsuGnssDeviationAttempts;
    int32_t         rsuGnssDeviationInterval;
    /*
     * 5.18.1.10 GNSS NMEA Message:NMEA 0183 string (including the $ starting character and the ending <CR><LF>)
     *
     *  periodically send: uint8_t rsuGnssOutputString[RSU_ALERT_MSG_LENGTH_MAX];
     */
    rsuAlertLevel_e rsuGnssNmeaNotify;
    int32_t rsuGnssNmeaNotifyInterval; /* RW: 0..18000: seconds: 0 = off = default. */

    /* 5.18.1.11 Certificate Error Message */
    rsuAlertLevel_e rsuCertificateError;
    uint8_t         rsuCertificateMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuCertificate_length;
    int32_t         rsuCertificateAttempts;
    int32_t         rsuCertificateInterval;

    /* 5.18.1.12 Denial of Service Error Message */
    rsuAlertLevel_e rsuServiceDenialError;
    uint8_t         rsuServiceDenialMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuServiceDenial_length;
    int32_t         rsuServiceDenialAttempts;
    int32_t         rsuServiceDenialInterval;

    /* 5.18.1.13 Watchdog Error Message */
    rsuAlertLevel_e rsuWatchdogError;
    uint8_t         rsuWatchdogMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuWatchdog_length;
    int32_t         rsuWatchdogAttempts;
    int32_t         rsuWatchdogInterval;

    /* 5.18.1.14 Enclosure Environment Message: tamper detection */
    rsuAlertLevel_e rsuEnvironError;
    uint8_t         rsuEnvironMsg[RSU_ALERT_MSG_LENGTH_MAX];
    int32_t         rsuEnviron_length;
    int32_t         rsuEnvironAttempts;
    int32_t         rsuEnvironInterval;

    /* 5.18.2.11 Notification Alert Level */
    rsuAlertLevel_e rsuAlertLevel; /* TODO: Not used? */

    /* 5.18.3 */
    int32_t    rsuNotificationRepeatInterval;

    /* 5.18.4 */
    int32_t    rsuNotificationMaxRetries;

    /* These are hidden from Client and for RSU internal use only: noAccess. */
    char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX]; /* Path and Name to file on RSU. */
    int32_t  version;                                   /* Internal revision control. */
    int32_t  status;                                    /* TBD. */
    uint32_t reserved;                                  /* TBD. Allow some wiggle room if we forget something later. */
    uint32_t crc;
} __attribute__((packed)) rsuAsync_t;
 
/* 5.18.3 Notification Repeat Interval */
#define RSU_NOTIFY_REPEAT_RATE_MIN  0
#define RSU_NOTIFY_REPEAT_RATE_MAX  255

/* 5.18.4 Notification Maximum Retries */
#define RSU_NOTIFY_MSG_RETRY_MIN  0  /* default: send and forget: don't spew cause it could lead to cascade of problems and output.*/
#define RSU_NOTIFY_MSG_RETRY_MAX  255

void install_rsuAsync_handlers(void);
void destroy_rsuAsync(void);

/* 5.18.2.1 */
void set_messageFileIntegrity(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_messageFileIntegrityError(void);
int32_t get_rsuMsgFileIntegrityMsg(uint8_t * data_out);
 /* 5.18.2.2 */
void set_rsuSecStorageIntegrity(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuSecStorageIntegrityError(void);
int32_t get_rsuSecStorageIntegrityMsg(uint8_t * data_out);
 /* 5.18.2.3 */
void set_rsuAuth(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuAuthError(void);
int32_t get_rsuAuthMsg(uint8_t * data_out);
/* 5.18.2.4 */
void set_rsuSignatureVerify(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuSignatureVerifyError(void);
int32_t get_rsuSignatureVerifyMsg(uint8_t * data_out);
/* 5.18.2.5 */
void set_rsuAccess(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuAccessError(void);
int32_t get_rsuAccessMsg(uint8_t * data_out);
/* 5.18.2.6 */
void set_rsuTimeSourceLost(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuTimeSourceLost(void);
int32_t get_rsuTimeSourceLostMsg(uint8_t * data_out);
/* 5.18.2.7 */
void set_rsuTimeSourceMismatch(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuTimeSourceMismatch(void);
int32_t get_rsuTimeSourceMismatchMsg(uint8_t * data_out);
/* 5.18.2.8 */
void set_rsuGnssAnomaly(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuGnssAnomaly(void);
int32_t get_rsuGnssAnomalyMsg(uint8_t * data_out);
/* 5.18.2.9 */
void set_rsuGnssDeviation(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuGnssDeviationError(void);
int32_t get_rsuGnssDeviationMsg(uint8_t * data_out);
/* 5.18.1.10 */
int32_t get_rsuGnssNmeaNotify(void);
/* 5.18.2.10 */
int32_t get_rsuGnssNmeaNotifyInterval(void);
int32_t preserve_rsuGnssNmeaNotifyInterval(void);
int32_t action_rsuGnssNmeaNotifyInterval(int32_t data_in);
int32_t commit_rsuGnssNmeaNotifyInterval(void);
int32_t undo_rsuGnssNmeaNotifyInterval(void);
/* 5.18.2.11  Odd one out: error level but no error msg like the others: RO: 0..4: rsuAlertLevel_e */
int32_t get_rsuAlertLevel(void);
/* 5.18.2.12 */
void set_rsuCertificate(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuCertificateError(void);
int32_t get_rsuCertificateMsg(uint8_t * data_out);
/* 5.18.2.13 */
void set_rsuServiceDenial(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuServiceDenialError(void);
int32_t get_rsuServiceDenialMsg(uint8_t * data_out);
/* 5.18.2.14 */
void set_rsuWatchdog(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuWatchdogError(void);
int32_t get_rsuWatchdogMsg(uint8_t * data_out);
/* 5.18.2.15 */
void set_rsuEnviron(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length);
int32_t get_rsuEnvironError(void);
int32_t get_rsuEnvironMsg(uint8_t * data_out);
/* 5.18.3 Notification Repeat Interval */
int32_t get_rsuNotificationRepeatInterval(void);
int32_t preserve_rsuNotificationRepeatInterval(void);
int32_t action_rsuNotificationRepeatInterval(int32_t data_in);
int32_t commit_rsuNotificationRepeatInterval(void);
int32_t undo_rsuNotificationRepeatInterval(void);
/* 5.18.4 Notification Maximum Retries */
int32_t get_rsuNotificationMaxRetries(void);
int32_t preserve_rsuNotificationMaxRetries(void);
int32_t action_rsuNotificationMaxRetries(int32_t data_in);
int32_t commit_rsuNotificationMaxRetries(void);
int32_t undo_rsuNotificationMaxRetries(void);

/* RSU App support */
#define ASYNC_ACTIVE_DIR "/rwflash/I2V/ntcip-1218/async" /* To support retries need to store across reset & recover. */
#define ASYNC_ENABLE_TRAPS_FILE "dotraps" /* if ASYNC_ACTIVE_DIR/ASYNC_ENABLE_TRAPS_FILE exists then start thread for traps.*/

void *rsuAsyncThr(void __attribute__((unused)) *arg); /* Handles SHM that updates MIB and calls send_notifications(). */
void mib_send_notifications(uint64_t errorstates);    /* If alert ready send snmptrap. */

#if defined(ENABLE_ASYNC_BACKUP)
#define ASYNC_FILENAME  "1.dat" /* Stores mib data across reset if enabled. */
void commit_rsuAsync_to_disk(rsuAsync_t * rsuAsync);  /* Store across reset on disk. */
#endif

/* Makes code more readable */
typedef enum  {  
    messageFileIntegrityID     = 1,
    SecStorageIntegrityID      = 2,
    AuthID                     = 3,
    SignatureVerifyID          = 4,
    AccessID                   = 5,
    TimeSourceLostID           = 6,
    TimeSourceMismatchID       = 7,
    GnssAnomalyID              = 8,
    GnssDeviationID            = 9,
    GnssNmeaNotifyID           = 10,
    CertificateID              = 11,
    ServiceDenialID            = 12,
    WatchdogID                 = 13,
    EnvironID                  = 14,
} rsuTrapMsgID_e;

/******************************************************************************
 * 5.19 RSU Applications: { rsu 18 }: 32 DSRC MSG ID's.
 *
 *  5.19.1 - 5.19.2:
 * 
 *  int32_t maxRsuApps;
 *  RsuAppConfigEntry_t rsuAppConfigTable[RSU_APPS_RUNNING_MAX];
 *
 *  typedef enum  { 
 *    rsuAppConfigStartup_other      = 1,  When not ready? Like boot time?
 *    rsuAppConfigStartup_onStartup  = 2,  Start at boot automatically.
 *    rsuAppConfigStartup_notStartup = 3,  Do not start automatically at boot.
 *  }rsuAppConfigStartup_e; 
 *
 *  typedef struct  {
 *    int32_t               rsuAppConfigID;                                 RO: 1..RSU_APPS_RUNNING_MAX: index must be 1 or greater.
 *    uint8_t               rsuAppConfigName[RSU_APP_CONF_NAME_LENGTH_MAX]; RO: 1..127 char: name of app and version.
 *    rsuAppConfigStartup_e rsuAppConfigStartup;                            RW: SET to 1(rsuAppConfigStartup_other) returns badValue
 *    int32_t               rsuAppConfigState;                              RO: 0..1: 0 = started, 1 = stopped.
 *    int32_t               rsuAppConfigStart;                              RW: 0..1: SET 1 = start then return to 0. toggle.
 *    int32_t               rsuAppConfigStop;                               RW: 0..1: SET 1 = stop then return to 0. toggle.
 *  } RsuAppConfigEntry_t;
 *
 ******************************************************************************/

/* 5.19.1 Maximum Apps: */
#define RSU_APPS_RUNNING_MIN  1
#define RSU_APPS_RUNNING_MAX  32 /* 32 dsrc msg id's ie spat, map, bsm, tim, srmrx, ssm */

/* 5.19.2 RSU Application Configuration Table */
#define RSU_APP_CONF_NAME_LENGTH_MIN  1
#define RSU_APP_CONF_NAME_LENGTH_MAX  127

typedef enum  { 
    rsuAppConfigStartup_other      = 1, /* When not ready? Like boot time? */
    rsuAppConfigStartup_onStartup  = 2, /* Start at boot automatically. */
    rsuAppConfigStartup_notStartup = 3, /* Do not start automatically at boot. */
}rsuAppConfigStartup_e;

typedef struct  {
    int32_t               rsuAppConfigID;
    uint8_t               rsuAppConfigName[RSU_APP_CONF_NAME_LENGTH_MAX];
    int32_t               rsuAppConfigName_length;
    rsuAppConfigStartup_e rsuAppConfigStartup;
    int32_t               rsuAppConfigState;
    int32_t               rsuAppConfigStart;
    int32_t               rsuAppConfigStop;
} __attribute__((packed)) RsuAppConfigEntry_t;

void install_rsuAppConfig_handlers(void);
void destroy_rsuAppConfig(void);

/* 5.19.1 */
int32_t get_maxRsuApps(void);
/* 5.19.2.2 */
int32_t get_rsuAppConfigName(int32_t index, uint8_t * data_out);
/* 5.19.2.3 */
int32_t get_rsuAppConfigStartup(int32_t index, int32_t * data_out);
int32_t preserve_rsuAppConfigStartup(int32_t index);
int32_t action_rsuAppConfigStartup(int32_t index, int32_t data_in);
int32_t commit_rsuAppConfigStartup(int32_t index);
int32_t undo_rsuAppConfigStartup(int32_t index);
/* 5.19.2.4 */
int32_t get_rsuAppConfigState(int32_t index, int32_t * data_out);
/* 5.19.2.5 */
int32_t get_rsuAppConfigStart(int32_t index, int32_t * data_out);
int32_t preserve_rsuAppConfigStart(int32_t index);
int32_t action_rsuAppConfigStart(int32_t index, int32_t data_in);
int32_t commit_rsuAppConfigStart(int32_t index);
int32_t undo_rsuAppConfigStart(int32_t index);
/* 5.19.2.6 */
int32_t get_rsuAppConfigStop(int32_t index, int32_t * data_out);
int32_t preserve_rsuAppConfigStop(int32_t index);
int32_t action_rsuAppConfigStop(int32_t index, int32_t data_in);
int32_t commit_rsuAppConfigStop(int32_t index);
int32_t undo_rsuAppConfigStop(int32_t index);

/******************************************************************************
 * 5.20 RSU Services: { rsu 19 }:
 *
 * 5.20.1 - 5.20.2 
 * int32_t           maxRsuServices;
 * RsuServiceEntry_t rsuServiceTable[RSU_SERVICES_RUNNING_MAX];
 *
 * typedef enum  {  
 *     rsuServiceStatus_other    = 1, boot time?
 *     rsuServiceStatus_okay     = 2, fully operational with no errors
 *     rsuServiceStatus_warning  = 3, operating, but error detected which may(?) affect its operation, e.g., wrong config (how to know?)
 *     rsuServiceStatus_critical = 4, error is detected that has impact on the RSU operation e.g. you ignored the warning so now what?
 *     rsuServiceStatus_unknown  = 5, this is what comes after staying in critical for too long.
 * } rsuServiceStatus_e;
 * 
 * The name and assignments for the standard services are as follows:
 *         rsuServiceID   rsuServiceName   Description
 *               1        RSU system       system services on the RSU
 *               2        GNSS             GNSS services
 *               3        Time source      clock sources
 *               4        Storage          file and log storage available
 *               5        Firewall         access control services
 *               6        Network          network services
 *               7        Layers 1 and 2   radio access technology
 *               8        Layers 3 and 4   networking services
 *               9        Security         security services
 *               10       SCMS             SCMS services
 *             11-16      RESERVED         reserved for future services
 *             17-127     vendor-specific  for vendor-specific services
 *
 * typedef struct {
 *    int32_t            rsuServiceID;                                             RO: 1..maxRsuApps:
 *    uint8_t            rsuServiceName[RSU_SERVICE_NAME_LENGTH_MAX];              RW: 1..127 chars 
 *    rsuServiceStatus_e rsuServiceStatus;                                         RO: 1..5 enum: default is rsuServiceStatus_unknown
 *    uint8_t            rsuServiceStatusDesc[RSU_SERVICE_STATUS_DESC_LENGTH_MAX]; RO: 0..255 chars: details of status: ie "Fire,bad!"
 *    uint8_t            rsuServiceStatusTime[MIB_DATEANDTIME_LENGTH];             RO: 8 OCTETS:DateAndTime of status. 
 * } RsuServiceEntry_t;
 *
 ******************************************************************************/

/* 5.20.1 Maximum RSU Service Entries */
#define RSU_SERVICES_RUNNING_MIN 1   /* 1..RSU_SERVICE_RESERVED_INDEX_MAX are reserved or RO. */
#define RSU_SERVICES_RUNNING_MAX 32  /* 16 MIB defined like SCMS and 16 of our own vendor specific. */

#define RSU_SERVICE_RESERVED_INDEX_MIN 1  /* These are reserved and RO rows from clients perspective */
#define RSU_SERVICE_RESERVED_INDEX_MAX 16

/* 5.20.2 RSU Services Table */
#define RSU_SERVICE_NAME_LENGTH_MIN 1
#define RSU_SERVICE_NAME_LENGTH_MAX 127

#define RSU_SERVICE_STATUS_DESC_LENGTH_MIN 0
#define RSU_SERVICE_STATUS_DESC_LENGTH_MAX 255

typedef enum  {  
    rsuServiceStatus_other    = 1,
    rsuServiceStatus_okay     = 2,
    rsuServiceStatus_warning  = 3,
    rsuServiceStatus_critical = 4,
    rsuServiceStatus_unknown  = 5,
}rsuServiceStatus_e;

typedef struct {
   int32_t            rsuServiceID;
   uint8_t            rsuServiceName[RSU_SERVICE_NAME_LENGTH_MAX];
   int32_t            rsuServiceName_length;
   rsuServiceStatus_e rsuServiceStatus;
   uint8_t            rsuServiceStatusDesc[RSU_SERVICE_STATUS_DESC_LENGTH_MAX];
   uint32_t           rsuServiceStatusDesc_length;
   uint8_t            rsuServiceStatusTime[MIB_DATEANDTIME_LENGTH];
} __attribute__((packed)) RsuServiceEntry_t;

void install_rsuService_handlers(void);
void destroy_rsuService(void);

/* 5.20.1 */
int32_t get_maxRsuServices(void);
/* 5.20.2.2 */
int32_t get_rsuServiceName(int32_t index, uint8_t * data_out);
int32_t preserve_rsuServiceName(int32_t index);
int32_t action_rsuServiceName(int32_t index, uint8_t * data_in, int32_t length);
int32_t commit_rsuServiceName(int32_t index);
int32_t undo_rsuServiceName(int32_t index);
/* 5.20.2.3 */
int32_t get_rsuServiceStatus(int32_t index);
/* 5.20.2.4 */
int32_t get_rsuServiceStatusDesc(int32_t index, uint8_t * data_out);
/* 5.20.2.5 */
int32_t get_rsuServiceStatusTime(int32_t index, uint8_t * data_out);

/******************************************************************************
 * 5.21 Transmitted Messages For Forwarding: { rsu 20 }:
 * 
 * typedef struct {
 *     int32_t    rsuXmitMsgFwdingIndex;                                 RO: 1 to RSU_XMIT_MSG_COUNT_MAX
 *     uint8_t    rsuXmitMsgFwdingPsid[RSU_RADIO_PSID_SIZE];             RC: 1 to 4 OCTETS:
 *     uint8_t    rsuXmitMsgFwdingDestIpAddr[RSU_DEST_IP_MAX];           RC: 0 to 64: IPv4 or IPv6. simplified.
 *     int32_t    rsuXmitMsgFwdingDestPort;                              RC: 1024..65535.
 *     protocol_e rsuXmitMsgFwdingProtocol;                              RC: 1 = other, 2 = udp: default is udp.
 *     uint8_t    rsuXmitMsgFwdingDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC: 8 OCTETS:DateAndTime:SNMPv2-TC.
 *     uint8_t    rsuXmitMsgFwdingDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC: 8 OCTETS:DateAndTime:SNMPv2-TC.
 * 
 *     A value of 0 indicates the RSU is to forward only the WSM message payload without security headers. 
 *     Specifically this means that either of the following is forwarded, depending on whether the message is signed 
 *     (a) or unsigned (b): 
 *     (a) Ieee1609Dot2Data.signedData.tbsData.payload.data.unsecuredData 
 *     (b) Ieee1609Dot2Data.unsecuredData.
 *      
 *     A value of 1 indicates the RSU is to forward the entire WSM including the security headers. 
 *     Specifically this means that the entire Ieee1609Dot2Data frame is forwarded in COER format.
 *     
 *     int32_t rsuXmitMsgFwdingSecure; RC: 0|1.
 * 
 *     int32_t rsuXmitMsgFwdingStatus; RC: Create (4) & (6) destroy:
 *
 * } RsuXmitMsgFwdingEntry_t;
 * 
 ******************************************************************************/

/* 5.21.1 */
#define RSU_XMIT_MSG_COUNT_MIN 1
#define RSU_XMIT_MSG_COUNT_MAX 32 /* 32 DSRC MSG ID's */

/* 5.21.2 */
#define RSU_XMIT_MSG_PORT_MIN 1024
#define RSU_XMIT_MSG_PORT_MAX RSU_DEST_PORT_MAX

#define RSU_XMIT_MSG_SECURE_MIN  0
#define RSU_XMIT_MSG_SECURE_MAX  1

typedef struct {
    int32_t    rsuXmitMsgFwdingIndex;
    uint8_t    rsuXmitMsgFwdingPsid[RSU_RADIO_PSID_SIZE];
    int32_t    rsuXmitMsgFwdingPsid_length;
    uint8_t    rsuXmitMsgFwdingDestIpAddr[RSU_DEST_IP_MAX];
    int32_t    rsuXmitMsgFwdingDestIpAddr_length;
    int32_t    rsuXmitMsgFwdingDestPort;
    protocol_e rsuXmitMsgFwdingProtocol;
    uint8_t    rsuXmitMsgFwdingDeliveryStart[MIB_DATEANDTIME_LENGTH];
    uint8_t    rsuXmitMsgFwdingDeliveryStop[MIB_DATEANDTIME_LENGTH];
    int32_t    rsuXmitMsgFwdingSecure;
    int32_t    rsuXmitMsgFwdingStatus; 
} __attribute__((packed)) RsuXmitMsgFwdingEntry_t;

void install_rsuXmitMsgFwding_handlers(void);
void destroy_rsuXmitMsgFwding(void);
void set_default_row_rsuXmitMsgFwding(RsuXmitMsgFwdingEntry_t * rsuXmitMsgFwding, int32_t index); /* Set single instance only. */
int32_t rebuild_rsuXmitMsgFwding_live(RsuXmitMsgFwdingEntry_t * rsuXmitMsgFwding);

/* 5.21.1 */
int32_t get_maxXmitMsgFwding(void);
/* 5.21.2.1 */
int32_t get_rsuXmitMsgFwdingIndex(int32_t index, int32_t * data_out);
/* 5.21.2.2 */
int32_t get_rsuXmitMsgFwdingPsid(int32_t index, uint8_t * data_out);
int32_t preserve_rsuXmitMsgFwdingPsid(int32_t index);
int32_t action_rsuXmitMsgFwdingPsid(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuXmitMsgFwdingPsid(int32_t index);
int32_t undo_rsuXmitMsgFwdingPsid(int32_t index);
/* 5.21.2.3 */
int32_t get_rsuXmitMsgFwdingDestIpAddr(int32_t index, uint8_t * data_out);
int32_t preserve_rsuXmitMsgFwdingDestIpAddr(int32_t index);
int32_t action_rsuXmitMsgFwdingDestIpAddr(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuXmitMsgFwdingDestIpAddr(int32_t index);
int32_t undo_rsuXmitMsgFwdingDestIpAddr(int32_t index);
/* 5.21.2.4 */
int32_t get_rsuXmitMsgFwdingDestPort(int32_t index, int32_t * data_out);
int32_t preserve_rsuXmitMsgFwdingDestPort(int32_t index);
int32_t action_rsuXmitMsgFwdingDestPort(int32_t index, int32_t data_in);
int32_t commit_rsuXmitMsgFwdingDestPort(int32_t index);
int32_t undo_rsuXmitMsgFwdingDestPort(int32_t index);
/* 5.21.2.5 */
int32_t get_rsuXmitMsgFwdingProtocol(int32_t index, int32_t * data_out);
int32_t preserve_rsuXmitMsgFwdingProtocol(int32_t index);
int32_t action_rsuXmitMsgFwdingProtocol(int32_t index, int32_t data_in);
int32_t commit_rsuXmitMsgFwdingProtocol(int32_t index);
int32_t undo_rsuXmitMsgFwdingProtocol(int32_t index);
/* 5.21.2.6 */
int32_t get_rsuXmitMsgFwdingDeliveryStart(int32_t index, uint8_t * data_out);
int32_t preserve_rsuXmitMsgFwdingDeliveryStart(int32_t index);
int32_t action_rsuXmitMsgFwdingDeliveryStart(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuXmitMsgFwdingDeliveryStart(int32_t index);
int32_t undo_rsuXmitMsgFwdingDeliveryStart(int32_t index);
/* 5.21.2.7 */
int32_t get_rsuXmitMsgFwdingDeliveryStop(int32_t index, uint8_t * data_out);
int32_t preserve_rsuXmitMsgFwdingDeliveryStop(int32_t index);
int32_t action_rsuXmitMsgFwdingDeliveryStop(int32_t index,uint8_t * data_in, int32_t length);
int32_t commit_rsuXmitMsgFwdingDeliveryStop(int32_t index);
int32_t undo_rsuXmitMsgFwdingDeliveryStop(int32_t index);
/* 5.21.2.8 */
int32_t get_rsuXmitMsgFwdingSecure(int32_t index, int32_t * data_out);
int32_t preserve_rsuXmitMsgFwdingSecure(int32_t index);
int32_t action_rsuXmitMsgFwdingSecure(int32_t index, int32_t data_in);
int32_t commit_rsuXmitMsgFwdingSecure(int32_t index);
int32_t undo_rsuXmitMsgFwdingSecure(int32_t index);
/* 5.21.2.9 */
int32_t get_rsuXmitMsgFwdingStatus(int32_t index, int32_t * data_out);
int32_t preserve_rsuXmitMsgFwdingStatus(int32_t index);
int32_t action_rsuXmitMsgFwdingStatus(int32_t index, int32_t data_in);
int32_t commit_rsuXmitMsgFwdingStatus(int32_t index);
int32_t undo_rsuXmitMsgFwdingStatus(int32_t index);

/******************************************************************************
 * MIB Structure: TODO: Need smaller to disk only size i.e. drop msgRepeat and RO values.
 ******************************************************************************/

typedef struct {

    /* DENSO specifc version info. */
    uint32_t         dn_version;

    /* 5.2.1 - 5.2.1 */
    int32_t          maxRsuRadios;                 
    RsuRadioEntry_t  rsuRadioTable[RSU_RADIOS_MAX];

    /* 5.3.1 - 5.3.2 */
    int32_t               rsuGnssStatus;
    rsuGnssAugmentation_e rsuGnssAugmentation;

    /* 5.4.1 - 5.4.3 */ 
    int32_t                   maxRsuMsgRepeat; 
    RsuMsgRepeatStatusEntry_t rsuMsgRepeatStatusTable[RSU_SAR_MESSAGE_MAX];
    int32_t                   rsuMsgRepeatDeleteAll;

    /* 5.5.1 - 5.5.2 */
    int32_t             maxRsuIFMs;
    RsuIFMStatusEntry_t rsuIFMStatusTable[RSU_IFM_MESSAGE_MAX];

    /* 5.6.1 - 5.6.2 */
    int32_t               maxRsuReceivedMsgs;
    RsuReceivedMsgEntry_t rsuReceivedMsgTable[RSU_RX_MSG_MAX];

    /* 5.7.1 - 5.7.11 */
    int32_t rsuGnssOutputPort;
    uint8_t rsuGnssOutputAddress[RSU_DEST_IP_MAX];
    int32_t rsuGnssOutputAddress_length;
    uint8_t rsuGnssOutputInterface[RSU_GNSS_OUTPUT_INTERFACE_MAX];
    int32_t rsuGnssOutputInterface_length;
    int32_t rsuGnssOutputInterval;
    uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX]; /* RO: derived. */
    int32_t rsuGnssOutputString_length; /* RO: derived. */
    int32_t rsuGnssLat; /* RO: derived. */
    int32_t rsuGnssLon; /* RO: derived. */
    int32_t rsuGnssElv; /* RO: derived. */
    int32_t rsuGnssMaxDeviation;
    int32_t rsuLocationDeviation;
    int32_t rsuGnssPositionError; /* RO: derived. */

    /* 5.8.1 - 5.8.2 */
    int32_t                maxRsuInterfaceLogs;
    RsuInterfaceLogEntry_t rsuInterfaceLogTable[RSU_IFCLOG_MAX];
   
    /* 5.9.1 - 5.9.16 */
    int32_t                  rsuSecCredReq;
    rsuSecEnrollCertStatus_e rsuSecEnrollCertStatus;
    int32_t                  rsuSecEnrollCertValidRegion;
    uint8_t                  rsuSecEnrollCertUrl[URI255_LENGTH_MAX];
    int32_t                  rsuSecEnrollCertUrl_length;
    uint8_t                  rsuSecEnrollCertId[RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX];
    int32_t                  rsuSecEnrollCertId_length;
    uint8_t                  rsuSecEnrollCertExpiration[MIB_DATEANDTIME_LENGTH];
    rsuSecuritySource_e      rsuSecuritySource;
    uint8_t                  rsuSecAppCertUrl[URI1024_LENGTH_MAX];
    int32_t                  rsuSecAppCertUrl_length;
    int32_t                  maxRsuSecAppCerts;
    RsuSecAppCertEntry_t     rsuSecAppCertTable[RSU_SEC_APP_CERTS_MAX];
    uint8_t                  rsuSecCertRevocationUrl[URI255_LENGTH_MAX];
    int32_t                  rsuSecCertRevocationUrl_length;
    uint8_t                  rsuSecCertRevocationTime[MIB_DATEANDTIME_LENGTH];
    int32_t                  rsuSecCertRevocationInterval;
    int32_t                  rsuSecCertRevocationUpdate;
    int32_t                  maxRsuSecProfiles;
    RsuSecProfileEntry_t     rsuSecProfileTable[RSU_SECURITY_PROFILES_MAX];

    /* 5.10.1 - 5.10.4 */
    int32_t              maxRsuWsaServices;
    RsuWsaServiceEntry_t rsuWsaServiceTable[RSU_WSA_SERVICES_MAX]; 
    RsuWsaChannelEntry_t rsuWsaChannelTable[RSU_WSA_SERVICES_MAX];
    int32_t              rsuWsaVersion;

    /* 5.11.1 - 5.11.7 */
    uint8_t rsuWraIpPrefix[RSU_DEST_IP_MAX];
    uint8_t rsuWraIpPrefix_length;
    uint8_t rsuWraIpPrefixLength;
    uint8_t rsuWraGateway[RSU_DEST_IP_MAX];
    uint8_t rsuWraGateway_length;
    uint8_t rsuWraPrimaryDns[RSU_DEST_IP_MAX];
    uint8_t rsuWraPrimaryDns_length;
    uint8_t rsuWraSecondaryDns[RSU_DEST_IP_MAX];
    uint8_t rsuWraSecondaryDns_length;
    uint8_t rsuWraGatewayMacAddress[RSU_RADIO_MAC_LENGTH];
    int32_t rsuWraLifetime;

    /* 5.12.1 */
    int32_t                       maxRsuMessageCountsByPsid;
    RsuMessageCountsByPsidEntry_t rsuMessageCountsByPsidTable[RSU_PSID_TRACKED_STATS_MAX];

    /* 5.13.1 - 5.13.6 */
    uint32_t            rsuTimeSincePowerOn;
    int32_t             rsuIntTemp;
    int32_t             rsuIntTempLowThreshold;
    int32_t             rsuIntTempHighThreshold;
    int32_t             maxRsuCommRange;
    RsuCommRangeEntry_t rsuCommRangeTable[RSU_COMM_RANGE_MAX];

    /* 5.14.1 - 5.14.15 */
    uint8_t            rsuMibVersion[RSUMIB_VERSION_LENGTH_MAX];
    int32_t            rsuMibVersion_length;
    uint8_t            rsuFirmwareVersion[RSU_FIRMWARE_VERSION_LENGTH_MAX];
    int32_t            rsuFirmwareVersion_length;
    uint8_t            rsuLocationDesc[RSU_LOCATION_DESC_LENGTH_MAX];
    int32_t            rsuLocationDesc_length;
    uint8_t            rsuID[RSU_ID_LENGTH_MAX];
    int32_t            rsuID_length;
    int32_t            rsuLocationLat;
    int32_t            rsuLocationLon;
    int32_t            rsuLocationElv;
    int32_t            rsuElevationOffset;
    int32_t            rsuInstallUpdate;
    uint8_t            rsuInstallFile[RSU_INSTALL_FILE_NAME_LENGTH_MAX];
    int32_t            rsuInstallFile_length;
    uint8_t            rsuInstallPath[RSU_INSTALL_PATH_LENGTH_MAX];
    int32_t            rsuInstallPath_length;
    rsuInstallStatus_e rsuInstallStatus;
    uint8_t            rsuInstallTime[MIB_DATEANDTIME_LENGTH]; 
    uint8_t            rsuInstallStatusMessage[RSU_INSTALL_STATUS_MSG_LENGTH_MAX];
    int32_t            rsuInstallStatusMessage_length; 
    uint8_t            rsuScheduledInstallTime[MIB_DATEANDTIME_LENGTH]; 

    /* 5.15.1 - 5.15.22 */
    uint8_t         rsuNotifyIpAddress[RSU_DEST_IP_MAX];
    int32_t         rsuNotifyIpAddress_length;
    int32_t         rsuNotifyPort;
    uint8_t         rsuSysLogQueryStart[RFC2579_DATEANDTIME_LENGTH];
    uint8_t         rsuSysLogQueryStop[RFC2579_DATEANDTIME_LENGTH];
    syslog_level_e  rsuSysLogQueryPriority; 
    int32_t         rsuSysLogQueryGenerate; 
    syslog_status_e rsuSysLogQueryStatus;
    int32_t         rsuSysLogCloseCommand; 
    syslog_level_e  rsuSysLogSeverity;
    uint8_t         rsuSysConfigId[RSU_SYS_CONFIG_ID_LENGTH_MAX];
    int32_t         rsuSysConfigId_length;
    int32_t         rsuSysRetries;
    int32_t         rsuSysRetryPeriod;
    uint32_t        rsuSysRetryPeriodStart;
    int32_t         rsuSysRetryAttempts;  /* Store attempts to recover across reset. */
    int32_t         rsuShortCommLossTime;
    int32_t         rsuLongCommLossTime;
    uint8_t         rsuSysLogName[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];
    int32_t         rsuSysLogName_length;
    uint8_t         rsuSysDir[RSU_SYSDIR_NAME_LENGTH_MAX];
    int32_t         rsuSysDir_length;
    int32_t         rsuLongCommLossReboot;
    uint8_t         rsuHostIpAddr[RSU_DEST_IP_MAX];
    int32_t         rsuHostIpAddr_length;
    uint8_t         rsuHostNetMask[RSU_DEST_IP_MAX];
    int32_t         rsuHostNetMask_length;
    uint8_t         rsuHostGateway[RSU_DEST_IP_MAX];
    int32_t         rsuHostGateway_length;
    uint8_t         rsuHostDNS[RSU_DEST_IP_MAX];
    int32_t         rsuHostDNS_length;
    uint32_t        rsuHostDHCPEnable;

    /* 5.16.1 - 5.16.2  */
    int32_t           maxRsuAntennas;
    RsuAntennaEntry_t rsuAntennaTable[RSU_ANTENNA_MAX];

    /* 5.17.1 - 5.17.10 */
    rsuChanStatus_e     rsuChanStatus;
    rsuMode_e           rsuMode;
    rsuModeStatus_e     rsuModeStatus;
    int32_t             rsuReboot;
    clockSource_e       rsuClockSource;
    clockSourceStatus_e rsuClockSourceStatus;
    int32_t             rsuClockSourceTimeout;
    int32_t             rsuClockSourceFailedQuery;
    int32_t             rsuClockDeviationTolerance;
    rsuStatus_e         rsuStatus;

    /* 5.18.1 - 5.18.4 */
    rsuAsync_t Async;

    /* 5.19.1 - 5.19.2 */
    int32_t maxRsuApps;
    RsuAppConfigEntry_t rsuAppConfigTable[RSU_APPS_RUNNING_MAX];

    /* 5.20.1 - 5.20.2 */ 
    int32_t           maxRsuServices;
    RsuServiceEntry_t rsuServiceTable[RSU_SERVICES_RUNNING_MAX];

    /* 5.21.1 - 5.21.2 */
    int32_t                 maxXmitMsgFwding;
    RsuXmitMsgFwdingEntry_t rsuXmitMsgFwdingTable[RSU_XMIT_MSG_COUNT_MAX];
    
    /* Reserved */
    uint32_t reserved[DN_RESERVED_BLOCK_MAX];
    
    /* 
     * 1201 req: 
     * Update everytime MIB changes. Store across reset.
     * Applies to only static items that get stored across reset.
     * Therefore, inc whenever a disk commit occurs.
     */
    int32_t  globalSetIDParameter;

    /* CRC-32 */
    uint32_t mib_crc32;

} __attribute__((packed)) ntcip_1218_mib_t;

/******************************************************************************
 * General MIB Utils:
 ******************************************************************************/
#define MIB_ERROR_BASE_2   -33  /* error state for any given module is 32 bits. */

/* ntcip-1218.c related: Turn on debug messages when starting snmpd to see specific failure. */
#define RSEMIB_BAD_INPUT       -1     /* Null input on client request. */
#define RSEMIB_UNLOCK_FAIL     -2     /* Should not happen unless we did something wrong. */
#define RSEMIB_LOCK_FAIL       -3     /* Only happen under heavy load and busy. If not then issue. */
#define RSEMIB_ROW_EMPTY       -4     /* If this is a dynamic table with CREATE & DESTROY then this means row doens't exist. */
#define RSEMIB_BAD_DATA        -5     /* Data in MIB failed internal validation ie out of range value. */
#define RSEMIB_BAD_MODE        -6     /* Unexpected return value. */
#define RSEMIB_BAD_INDEX       -7     /* Client requested index in OID that is invalid ie non-existent row of table. */
#define RSEMIB_FOPEN_FAIL      -8     /* Somewhere fopen() or popen() failed. */
#define RSEMIB_MUTEX_INIT_FAIL -9     /* Failure to create mutex for MIB. FATAL. */
#define RSEMIB_SYSCALL_FAIL    -10      /* Failed system() call somewhere. */
#define RSEMIB_CREATE_DIR_FAIL      -11 /* Can't create folder on RSU. */
#define RSEMIB_BAD_RECORD_RESTORE   -12 /* Either wrong version or crc fail on recover from disk. */
#define RSEMIB_INSTALL_HANDLER_FAIL -13 /* Request handler install failed. Certain OIDs will not be available. */
#define RSEMIB_FWRITE_FAIL          -14 /* Somewhere fwrite() failed. */
#define RSEMIB_THREAD_FAIL          -15 /* Somewhere a thread failed to spawn. */
#define RSEMIB_FILE_STAT_FAIL       -16 /* File doesn't exist. */
#define RSEMIB_BAD_REQUEST_TABLE_INFO  -18
#define RSEMIB_REBUILD_FAIL            -19
#define RSEMIB_BAD_REQUEST_TABLE_ENTRY -20
#define RSEMIB_BAD_REQUEST_COLUMN      -21
#define RSEMIB_CREATE_ROW_FAIL         -22
#define RSEMIB_CREATE_TABLE_FAIL       -23
#define RSEMIB_OPTIONS_LENGTH_ERROR    -24
#define RSEMIB_MQ_FAIL                 -25
#define RSEMIB_ENQUEUE_FAIL            -26
#define RSEMIB_SHM_INIT_FAILED         -27
#define RSEMIB_SHM_SEED_FAILED         -28
#define RSEMIB_BAD_UTC                 -29
/* Limited to -32 */
/* Get module specfic error states. */
uint32_t get_ntcip_1218_error_states(void);
uint32_t get_rsuRadioTable_error_states(void);
uint32_t get_rsuGnssStatus_error_states(void);
uint32_t get_rsuGnssOutput_error_states(void);
uint32_t get_rsuWraConfig_error_states(void);
uint32_t get_rsuSysDescription_error_states(void);
uint32_t get_rsuSysSettings_error_states(void);
uint32_t get_rsuSysStatus_error_states(void);
uint32_t get_rsuAsync_error_states(void);
uint32_t get_rsuSecurity_error_states(void);
uint32_t get_rsuAntenna_error_states(void);
uint32_t get_rsuAppConfig_error_states(void);
uint32_t get_rsuService_error_states(void);
uint32_t get_rsuMessageStats_error_states(void);
uint32_t get_rsuMsgRepeat_error_states(void);
uint32_t get_rsuIFM_error_states(void);
uint32_t get_receivedMsg_error_states(void);
uint32_t get_interfaceLog_error_states(void);
uint32_t get_rsuWsaConfig_error_states(void);
uint32_t get_rsuSystemStats_error_states(void);
uint32_t get_rsuXmitMsgFwding_error_states(void);

/* Database Lock. */
int32_t ntcip1218_lock(void); 
void ntcip1218_unlock(void);

/* These are not locked so if you call its a good idea to lock first. Live pointer. */
RsuMsgRepeatStatusEntry_t * get_rsuMsgRepeatTable(void);
 
/* General utilities. */
void    update_ntcip_1218(ntcip_1218_mib_t * shmaddr); /* Could be replace by mq and threads in each module */
int32_t valid_table_row(int32_t data_in);
int32_t commit_to_disk(void); /* Will write entire MIB to RSU NVM to store across reset. Binary format. Not encrypted yet but can be. */

/* "skinny" means we are only preserving a subset of rsuSystemDescription to disk. */
int32_t copy_sysDesc_to_skinny(ntcip_1218_mib_t * ntcip_1218_mib , SystemDescription_t * sysDesc);

#endif /* NTCIP_1218_H */
