/****************************************************************************
 *                                                                          *
 *  File Name: amh.h                                                        *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      Rutherford rd, Carlsbad                                             *
 *      CA 92008                                                            *
 *                                                                          *
 ****************************************************************************/
#ifndef AMH_H
#define AMH_H

#include "i2v_general.h"
#include "amh_types.h"

/* AMH specifc function. */
/* MAX_WSM_DATA is 2302 so we need twice that for ascii UPER string in payload.*/
#define MEM_PARSE_CONF_GET_MAX_BUF   ((MAX_WSM_DATA * 2) + 100)
#define MEM_PARSE_CONF_MAX_BUF       MEM_PARSE_CONF_GET_MAX_BUF
#define MEM_PARSE_CONF_MAX_LEN       MEM_PARSE_CONF_GET_MAX_BUF

#define AMH_FILENAME_SIZE 108  //must be at least this big to handle conf input I2V_CFG_MAX_STR_LEN
#define AMH_PATH_SIZE     1024
#define AMH_CFG_FILE      "amh.conf"
/* 
 * Min file size allowed for SAR. 
 * Not exact just a rough check for bogus data.
 */
#define MIN_SAR_FILE_SIZE 50

#define AMH_AOK                            0
/* -1 to -32 : AMH specfic error states. Some are don't care and some are return values for main(). */
#define AMH_ERROR_BASE                   0
#define AMH_INIT_SHM_FAIL                  (AMH_ERROR_BASE - 1)
#define AMH_LOAD_CONFIG_FAIL               (AMH_ERROR_BASE - 2)
#define AMH_OPEN_MQ_FAIL                   (AMH_ERROR_BASE - 3)
#define AMH_CUSTOMER_DIGEST_FAIL           (AMH_ERROR_BASE - 4)
#define AMH_FWD_WRITE_ERROR                (AMH_ERROR_BASE - 5)
#define AMH_FWD_WRITE_IGNORE               (AMH_ERROR_BASE - 6)
#define AMH_RAMLIST_LOAD_TOO_SMALL         (AMH_ERROR_BASE - 7)
#define AMH_RAMLIST_LOAD_TOO_BIG           (AMH_ERROR_BASE - 8)
#define AMH_SIG_FAULT                      (AMH_ERROR_BASE - 9)
#define AMH_NEW_PSID_REG_FAIL              (AMH_ERROR_BASE - 10)
#define AMH_CONF_FOPEN_ERROR               (AMH_ERROR_BASE - 11)
#define AMH_RAMLIST_LOAD_NULL_INPUT_ERROR  (AMH_ERROR_BASE - 12)
#define AMH_RAMLIST_LOAD_FOPEN_ERROR       (AMH_ERROR_BASE - 13)
#define AMH_RAMLIST_PARSE_ERROR            (AMH_ERROR_BASE - 14)
#define AMH_RAMLIST_PRIORITY_ERROR         (AMH_ERROR_BASE - 15)
#define AMH_ACTIVE_MSG_DIR_ERROR           (AMH_ERROR_BASE - 16)
#define AMH_ACTIVE_MSG_DIR_READ_ERROR      (AMH_ERROR_BASE - 17)
#define AMH_PEER_LIST_FOPEN_ERROR          (AMH_ERROR_BASE - 18)
#define AMH_HEAL_FAIL                      (AMH_ERROR_BASE - 19)
#define AMH_HEAL_CFGITEM                   (AMH_ERROR_BASE - 20)
#define AMH_RADIO_REG_FULL                 (AMH_ERROR_BASE - 21)
#define AMH_RADIO_REG_NEGATIVE             (AMH_ERROR_BASE - 22)
#define AMH_FILENAME_TOO_LONG              (AMH_ERROR_BASE - 23)
#define AMH_TIME_FAULT                     (AMH_ERROR_BASE - 24)
#define AMH_TIMEOUT_WAITING_FOR_I2V_SHM    (AMH_ERROR_BASE - 25)
#define AMH_DELETE_SERVICE_FAILED          (AMH_ERROR_BASE - 26)
#define AMH_RAMLIST_LOAD_NOT_ASCII_DATA    (AMH_ERROR_BASE - 27)
#define AMH_RAMLIST_FILE_TOO_BIG           (AMH_ERROR_BASE - 28)
#define AMH_RAMLIST_FILE_TOO_SMALL         (AMH_ERROR_BASE - 29)
#define AMH_FWD_BAD_INPUT                  (AMH_ERROR_BASE - 30)
#define AMH_RAMLIST_LOAD_ODD_SIZE          (AMH_ERROR_BASE - 31)
#define AMH_CREATE_IMF_THREAD_FAIL         (AMH_ERROR_BASE - 32)

/* -33 to -65: SAR specfic error states. */
#define AMH_SAR_ERROR_BASE                 -33
#define AMH_STAT_FAIL                      AMH_SAR_ERROR_BASE
#define AMH_STAT_SIZE_FAIL                 (AMH_SAR_ERROR_BASE - 1)
#define AMH_STAT_NULL_INPUT                (AMH_SAR_ERROR_BASE - 2)
#define AMH_STAT_BAD_DATA                  (AMH_SAR_ERROR_BASE - 3)
#define AMH_STAT_BAD_MODE                  (AMH_SAR_ERROR_BASE - 4)
#define AMH_STAT_BAD_FORM                  (AMH_SAR_ERROR_BASE - 5)
#define AMH_STAT_FOPEN_FAIL                (AMH_SAR_ERROR_BASE - 6)
#define AMH_STAT_FREAD_FAIL                (AMH_SAR_ERROR_BASE - 7)
//#define AMH_STAT_FWRITE_FAIL               (AMH_SAR_ERROR_BASE - 8)
#define AMH_SAR_TOO_MANY_WARNING           (AMH_SAR_ERROR_BASE - 9)      /* RSU storage has reached limit of allowed SAR Msgs */
#define AMH_SAR_VALIDATION_NOT_ENABLED     (AMH_SAR_ERROR_BASE - 10)
#define AMH_SAR_DIR_LIST_TOO_LARGE_WARNING (AMH_SAR_ERROR_BASE - 11)
#define AMH_SAR_JUNK_FILES_WARNING         (AMH_SAR_ERROR_BASE - 12)
#define AMH_SAR_WSM_REQ_ERROR              (AMH_SAR_ERROR_BASE - 13)
#define AMH_STAT_CRC_FAIL                  (AMH_SAR_ERROR_BASE - 14)
#define AMH_SAR_PARSE_CONF_NULL_INPUT      (AMH_SAR_ERROR_BASE - 15)
//#define AMH_SAR_DELETE_ERROR               (AMH_SAR_ERROR_BASE - 16)
#define AMH_MAIN_SAR_NULL_INPUT_ERROR      (AMH_SAR_ERROR_BASE - 17)
#define AMH_NEW_SAR_PSID_REG_FAIL          (AMH_SAR_ERROR_BASE - 18)
#define AMH_PROCESS_MSG_WRONG_VERSION      (AMH_SAR_ERROR_BASE - 19)
#define AMH_PROCESS_BAD_MSG_TYPE           (AMH_SAR_ERROR_BASE - 20)
#define AMH_MSG_STATS_MIB_THREAD_FAIL      (AMH_SAR_ERROR_BASE - 21)
#define AMH_SAR_FULL_WARNING               (AMH_SAR_ERROR_BASE - 22)
#define AMH_SAR_REJECTED                   (AMH_SAR_ERROR_BASE - 23)
#define AMH_SAR_BAD_START_TIME             (AMH_SAR_ERROR_BASE - 24)
#define AMH_SAR_FILENAME_TOO_LONG          (AMH_SAR_ERROR_BASE - 25)
#define AMH_SAR_BAD_STOP_TIME              (AMH_SAR_ERROR_BASE - 26)
#define AMH_SAR_STOP_TIME_BEFORE_START     (AMH_SAR_ERROR_BASE - 27)
#define AMH_SAR_FILE_OPEN_ERROR            (AMH_SAR_ERROR_BASE - 28)
#define AMH_SAR_PARSE_CONF_BLANK_INPUT     (AMH_SAR_ERROR_BASE - 29)
#define AMH_SAR_PARSE_CONF_VALIDATE_FAIL   (AMH_SAR_ERROR_BASE - 30)
#define AMH_RSUHEALTH_NOT_READY            (AMH_SAR_ERROR_BASE - 31)

/* -66 to -98 IMF specific error states. */
#define AMH_IMF_ERROR_BASE                 -66
#define AMH_IMF_BAD_INPUT                  AMH_IMF_ERROR_BASE
#define AMH_IMF_SOCKET_FAIL                (AMH_IMF_ERROR_BASE - 1)
#define AMH_IMF_BIND_FAIL                  (AMH_IMF_ERROR_BASE - 2)
#define AMH_IMF_CHANNEL_ERROR              (AMH_IMF_ERROR_BASE - 3)

#define AMH_IFM_PARSE_CONF_BLANK_INPUT     (AMH_IMF_ERROR_BASE - 4)
//#define AMH_IFM_PARSE_CONF_VALIDATE_FAIL   (AMH_IMF_ERROR_BASE - 5)
//#define AMH_IFM_PARSE_CONF_COMMENT_INPUT   (AMH_IMF_ERROR_BASE - 6)
#define AMH_IFM_MIB_WSM_SEND_FAIL          (AMH_IMF_ERROR_BASE - 7)

#define AMH_IMF_PAYLOAD_NOT_HEX_DATA       (AMH_IMF_ERROR_BASE - 8)
//#define AMH_IMF_FILESIZE_TOO_SMALL         (AMH_IMF_ERROR_BASE - 9)
#define AMH_IMF_BAD_TXINTERVAL             (AMH_IMF_ERROR_BASE - 10)
#define AMH_IMF_ENCRYPT_REJECT             (AMH_IMF_ERROR_BASE - 11)

#define AMH_IMF_SECURITY_REJECT            (AMH_IMF_ERROR_BASE - 12)
#define AMH_IMF_MSGTYPE_REJECT             (AMH_IMF_ERROR_BASE - 13)
#define AMH_IMF_INVALID_PSID               (AMH_IMF_ERROR_BASE - 14)
#define AMH_IMF_RECVFROM_ERROR             (AMH_IMF_ERROR_BASE - 15)

//#define AMH_IMF_PAYLOAD_NOT_ASCII          (AMH_IMF_ERROR_BASE - 16)
#define AMH_IMF_MIB_THREAD_FAIL            (AMH_IMF_ERROR_BASE - 17)
#define AMH_IFM_QUEUE_MSG_SIZE_FAIL        (AMH_IMF_ERROR_BASE - 18) 
#define AMH_IFM_STAT_BAD_FORM              (AMH_IMF_ERROR_BASE - 19)

#define AMH_IMF_CONF_ITEM_MISSING          (AMH_IMF_ERROR_BASE - 20)
#define AMH_IMF_PAYLOAD_TOO_SMALL          (AMH_IMF_ERROR_BASE - 21)
#define AMH_IMF_PAYLOAD_TOO_BIG            (AMH_IMF_ERROR_BASE - 22)
#define AMH_IMF_PAYLOAD_ODD_LENGTH         (AMH_IMF_ERROR_BASE - 23)

#define AMH_IFM_LEGACY_WSM_SEND_FAIL       (AMH_IMF_ERROR_BASE - 24)
#define AMH_IFM_MQ_FAIL                    (AMH_IMF_ERROR_BASE - 25)
//cheating, ran out of bits
//#define AMH_FWD_WRITE_NOT_READY            (AMH_IMF_ERROR_BASE - 26)
//#define AMH_FWD_WRITE_POLL_TIMEOUT         (AMH_IMF_ERROR_BASE - 27)

//#define AMH_FWD_WRITE_POLL_FAIL            (AMH_IMF_ERROR_BASE - 28)
#define AMH_IMF_READ_NOT_READY             (AMH_IMF_ERROR_BASE - 29)
#define AMH_IMF_BAD_OPTIONS                (AMH_IMF_ERROR_BASE - 30)
#define AMH_IMF_RESTRICT_IP                (AMH_IMF_ERROR_BASE - 31)

//TODO need seperate register to manage ports

/* The end. No errors beyond here are handled. */
#define AMH_ERROR_TOP                      -99

/* amhValidator.c & amhlib.c return bitmask(32 bit max). */
#define AMH_SAR_FILE_TOO_BIG              0x1
#define AMHL_SAR_NO_TYPE                  0x2
#define AMHL_SAR_NO_PSID                  0x4
#define AMHL_SAR_NO_PRIORITY              0x8
#define AMHL_SAR_NO_TXMODE                0x10
#define AMHL_SAR_NO_TXCHANNEL             0x20
#define AMHL_SAR_NO_TXINTERVAL            0x40
#define AMHL_SAR_NO_DELIVERYSTART         0x80
#define AMHL_SAR_NO_DELIVERYSTOP          0x100
#define AMHL_SAR_NO_SIGNATURE             0x200
#define AMHL_SAR_NO_PAYLOAD               0x400
#define AMHL_SAR_NO_VERSION               0x800
#define AMHL_SAR_NULL_INPUT               0x1000
#define AMHL_SAR_BAD_MSG_TYPE             0x2000
#define AMV_NULL_INPUT                    0x4000
#define AMV_SAR_DOS2UNIX_FAIL             0x8000
#define AMV_SAR_FILE_NOT_ASCII            0x10000
#define AMV_SAR_FILENAME_RSEMIB_CONFLICT  0x20000
#define AMV_SAR_FILE_OPEN_ERROR           0x40000
#define AMV_SAR_FILE_OPEN_ERROR2          0x80000

/* For imf.c */
#define SPAT_EXTENSION    "SPAT"
#define MAP_EXTENSION     "MAP"
#define TIM_EXTENSION     "TIM"
#define RTCM_EXTENSION    "RTCM"
#define ICA_EXTENSION     "ICA"
#define PDM_EXTENSION     "PDM"
#define RSA_EXTENSION     "RSA"
#define SSM_EXTENSION     "SSM"
#define CSR_EXTENSION     "CSR"
#define EVA_EXTENSION     "EVA"
#define NMEA_EXTENSION    "NMEA"
#define PSM_EXTENSION     "PSM"
#define PVD_EXTENSION     "PVD"
#define SRM_EXTENSION     "SRM"
#define BSM_EXTENSION     "BSM"
#define TEST00_EXTENSION  "TEST00"
#define TEST01_EXTENSION  "TEST01"
#define TEST02_EXTENSION  "TEST02"
#define TEST03_EXTENSION  "TEST03"
#define TEST04_EXTENSION  "TEST04"
#define TEST05_EXTENSION  "TEST05"
#define TEST06_EXTENSION  "TEST06"
#define TEST07_EXTENSION  "TEST07"
#define TEST08_EXTENSION  "TEST08"
#define TEST09_EXTENSION  "TEST09"
#define TEST10_EXTENSION  "TEST10"
#define TEST11_EXTENSION  "TEST11"
#define TEST12_EXTENSION  "TEST12"
#define TEST13_EXTENSION  "TEST13"
#define TEST14_EXTENSION  "TEST14"
#define TEST15_EXTENSION  "TEST15"

/* amhlib.c */
uint32_t amvParseSARFile(FILE *f, cfgItemsTypeT *cfgItems, uint32_t numCfgItems);
void amvUpdateType(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
void amvUpdateTxMode(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
void amvUpdateTxChannel(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
void amvUpdateMinute(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
void amvUpdateBool(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
void amvUpdatePayload(char_t GCC_UNUSED_VAR * tag, char_t GCC_UNUSED_VAR * parsedItem, char_t * value, void  * configItem, uint32_t * status);
void amvUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger);

/* amh_stats.c */
void amh_handle_diagnostics(amhBitmaskType encoding, uint64_t delta);
void update_sar_count(uint32_t ret, amhBitmaskType type);
void update_imf_count(uint32_t ret, amhBitmaskType type);

#endif /* AMH_H */
