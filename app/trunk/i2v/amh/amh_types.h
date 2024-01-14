/****************************************************************************
 *                                                                          *
 *  File Name: amh.h                                                        *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/
/* amh specific include */

#ifndef AMH_TYPES_H
#define AMH_TYPES_H

#include "i2v_types.h"
#include "i2v_general.h"


/* this is more than enough space allocation for amh files - but if
   the structure changes this macro needs to be modified - bear in
   mind this is allocated bytes that both imf and amh will allocate */
#define MAX_AMH_FILE_SIZE    8192


/* Max number of different PSID's AMH will register on-the-fly */
#define MAX_AMH_EXTRA_PSID   (32)

/* re:RSU_MIB_MAX_PAYLOAD_OCTET_STRING:
   Max number of 4-bit nibbles, which in ascii is 8-bit. 
   Need to be packed for WSM transmission.
*/
#define RSU_MIB_MAX_PAYLOAD_OCTET_STRING (MAX_WSM_DATA * 2)

#define AMH_NEED_SCH  0xFF

typedef struct {
    uint16_t count;
    uint8_t  payload[MAX_WSM_DATA];
} __attribute__((packed)) payloadType;

typedef struct {
    amhBitmaskType  type;
    uint32_t        psid;
    uint8_t         priority;
    uint8_t         channel;
    payloadType     data;
} __attribute__((packed)) rawImmediateType;

typedef struct {
    uint32_t      psid;
    uint8_t       priority;
    bool_t        useCCH;
    securityType  security;
    uint32_t      dsrcMsgId;
} __attribute__((packed)) amhSendParams;


/* 
 * Need to track all PSID's that register with Radio.
 * Store & Repat msgs are tracked easiliy in RAM list.
 * IMF messages register for radio but are NOT put in RAM list so easy to forget about
 *
 * Questioon: How many on the fly reg of different PSID's can the radio handle before problems?
 *            Should IMF psid be de-reg after MAX_PSID_TIMEOUT?
 *            Why not use preallocted circular buffer? We can only support fixed number anyways.
 *         
 */
typedef struct amcTrackPSIDRec_s {
    uint32_t    track_psid;
    uint64_t    track_lastSend;
} __attribute__((packed)) amcTrackPSIDRec;

/*track: Msg count, lagging client service, ...*/

typedef struct {
  /* SAR Msg TX Count */
  uint32_t spat_sar_msg_tx_count;
  uint32_t map_sar_msg_tx_count;
  uint32_t tim_sar_msg_tx_count;
  uint32_t rtcm_sar_msg_tx_count;
  uint32_t ica_sar_msg_tx_count;
  uint32_t pdm_sar_msg_tx_count;
  uint32_t rsa_sar_msg_tx_count;
  uint32_t ssm_sar_msg_tx_count;
  uint32_t csr_sar_msg_tx_count;
  uint32_t eva_sar_msg_tx_count;
  uint32_t nmea_sar_msg_tx_count;
  uint32_t psm_sar_msg_tx_count;
  uint32_t pvd_sar_msg_tx_count;
  uint32_t srm_sar_msg_tx_count;
  uint32_t bsm_sar_msg_tx_count;
  uint32_t test00_sar_msg_tx_count;
  uint32_t test01_sar_msg_tx_count;
  uint32_t test02_sar_msg_tx_count;
  uint32_t test03_sar_msg_tx_count;
  uint32_t test04_sar_msg_tx_count;
  uint32_t test05_sar_msg_tx_count;
  uint32_t test06_sar_msg_tx_count;
  uint32_t test07_sar_msg_tx_count;
  uint32_t test08_sar_msg_tx_count;
  uint32_t test09_sar_msg_tx_count;
  uint32_t test10_sar_msg_tx_count;
  uint32_t test11_sar_msg_tx_count;
  uint32_t test12_sar_msg_tx_count;
  uint32_t test13_sar_msg_tx_count;
  uint32_t test14_sar_msg_tx_count;
  uint32_t test15_sar_msg_tx_count;
  uint32_t sar_tx_err;
  uint32_t sar_tx_cnt;
  uint32_t sar_tx_slip;
  uint64_t sar_tx_max_slip;


  /* AMH Client lagging Count: 
   * AMH will measure max lateness only
   */
  uint64_t spat_client_slip_count;
  uint64_t map_client_slip_count;
  uint64_t tim_client_slip_count;
  uint64_t rtcm_client_slip_count;
  uint64_t ica_client_slip_count;
  uint64_t pdm_client_slip_count;
  uint64_t rsa_client_slip_count;
  uint64_t ssm_client_slip_count;
  uint64_t csr_client_slip_count;
  uint64_t eva_client_slip_count;
  uint64_t nmea_client_slip_count;
  uint64_t psm_client_slip_count;
  uint64_t pvd_client_slip_count;
  uint64_t srm_client_slip_count;
  uint64_t bsm_client_slip_count;
  uint64_t test00_client_slip_count;
  uint64_t test01_client_slip_count;
  uint64_t test02_client_slip_count;
  uint64_t test03_client_slip_count;
  uint64_t test04_client_slip_count;
  uint64_t test05_client_slip_count;
  uint64_t test06_client_slip_count;
  uint64_t test07_client_slip_count;
  uint64_t test08_client_slip_count;
  uint64_t test09_client_slip_count;
  uint64_t test10_client_slip_count;
  uint64_t test11_client_slip_count;
  uint64_t test12_client_slip_count;
  uint64_t test13_client_slip_count;
  uint64_t test14_client_slip_count;
  uint64_t test15_client_slip_count;

  /* IMF Msg Count: From udp connection. */
  uint32_t spat_imf_msg_tx_count;
  uint32_t map_imf_msg_tx_count;
  uint32_t tim_imf_msg_tx_count;
  uint32_t rtcm_imf_msg_tx_count;
  uint32_t ica_imf_msg_tx_count;
  uint32_t pdm_imf_msg_tx_count;
  uint32_t rsa_imf_msg_tx_count;
  uint32_t ssm_imf_msg_tx_count;
  uint32_t csr_imf_msg_tx_count;
  uint32_t eva_imf_msg_tx_count;
  uint32_t nmea_imf_msg_tx_count;
  uint32_t psm_imf_msg_tx_count;
  uint32_t pvd_imf_msg_tx_count;
  uint32_t srm_imf_msg_tx_count;
  uint32_t bsm_imf_msg_tx_count;
  uint32_t test00_imf_msg_tx_count;
  uint32_t test01_imf_msg_tx_count;
  uint32_t test02_imf_msg_tx_count;
  uint32_t test03_imf_msg_tx_count;
  uint32_t test04_imf_msg_tx_count;
  uint32_t test05_imf_msg_tx_count;
  uint32_t test06_imf_msg_tx_count;
  uint32_t test07_imf_msg_tx_count;
  uint32_t test08_imf_msg_tx_count;
  uint32_t test09_imf_msg_tx_count;
  uint32_t test10_imf_msg_tx_count;
  uint32_t test11_imf_msg_tx_count;
  uint32_t test12_imf_msg_tx_count;
  uint32_t test13_imf_msg_tx_count;
  uint32_t test14_imf_msg_tx_count;
  uint32_t test15_imf_msg_tx_count;
  uint32_t imf_tx_err;
  uint32_t imf_tx_cnt;

  /* IMF drop Count: 
   * IMF will drop incoming msg if shm is full. 
   * No attempt to retransmit. Msg comes in via udp.
   */
  uint32_t spat_imf_drop_count;
  uint32_t map_imf_drop_count;
  uint32_t tim_imf_drop_count;
  uint32_t rtcm_imf_drop_count;
  uint32_t ica_imf_drop_count;
  uint32_t pdm_imf_drop_count;
  uint32_t rsa_imf_drop_count;
  uint32_t ssm_imf_drop_count;
  uint32_t csr_imf_drop_count;
  uint32_t eva_imf_drop_count;
  uint32_t nmea_imf_drop_count;
  uint32_t psm_imf_drop_count;
  uint32_t pvd_imf_drop_count;
  uint32_t srm_imf_drop_count;
  uint32_t bsm_imf_drop_count;
  uint32_t test00_imf_drop_count;
  uint32_t test01_imf_drop_count;
  uint32_t test02_imf_drop_count;
  uint32_t test03_imf_drop_count;
  uint32_t test04_imf_drop_count;
  uint32_t test05_imf_drop_count;
  uint32_t test06_imf_drop_count;
  uint32_t test07_imf_drop_count;
  uint32_t test08_imf_drop_count;
  uint32_t test09_imf_drop_count;
  uint32_t test10_imf_drop_count;
  uint32_t test11_imf_drop_count;
  uint32_t test12_imf_drop_count;
  uint32_t test13_imf_drop_count;
  uint32_t test14_imf_drop_count;
  uint32_t test15_imf_drop_count;

  /* SAR drop Count: 
   * Error of some sort causing drop. 
   */
  uint32_t spat_sar_drop_count;
  uint32_t map_sar_drop_count;
  uint32_t tim_sar_drop_count;
  uint32_t rtcm_sar_drop_count;
  uint32_t ica_sar_drop_count;
  uint32_t pdm_sar_drop_count;
  uint32_t rsa_sar_drop_count;
  uint32_t ssm_sar_drop_count;
  uint32_t csr_sar_drop_count;
  uint32_t eva_sar_drop_count;
  uint32_t nmea_sar_drop_count;
  uint32_t psm_sar_drop_count;
  uint32_t pvd_sar_drop_count;
  uint32_t srm_sar_drop_count;
  uint32_t bsm_sar_drop_count;
  uint32_t test00_sar_drop_count;
  uint32_t test01_sar_drop_count;
  uint32_t test02_sar_drop_count;
  uint32_t test03_sar_drop_count;
  uint32_t test04_sar_drop_count;
  uint32_t test05_sar_drop_count;
  uint32_t test06_sar_drop_count;
  uint32_t test07_sar_drop_count;
  uint32_t test08_sar_drop_count;
  uint32_t test09_sar_drop_count;
  uint32_t test10_sar_drop_count;
  uint32_t test11_sar_drop_count;
  uint32_t test12_sar_drop_count;
  uint32_t test13_sar_drop_count;
  uint32_t test14_sar_drop_count;
  uint32_t test15_sar_drop_count;
} __attribute__((packed)) amhManagerStatsT;

#endif /* AMH_TYPES_H */
