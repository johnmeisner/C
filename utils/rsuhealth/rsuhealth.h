#ifndef RSUHEALTH_H
#define RSUHEALTH_H

/* 
 NOTE: signals are separated *just in case* down the road other
 utilities need to issue them to the health script
*/
#include <signal.h>
#include "wsu_sharedmem.h"
#include "wsu_shm_inc.h"

#define RSUSIG_TERMI2V    SIGRTMIN+1
#define RSUSIG_RESTOREI2V SIGRTMIN+2
#define RSUSIG_LEDMGMT    SIGRTMIN+3
#define RSUSIG_LEDQUERY   SIGRTMIN+4
#define RSUSIG_STATE      SIGRTMIN+5
#define RSUSIG_UTIL       SIGRTMIN+6

/* shared with dnnetcfg */
#define GWRST             "gatewayreset"

/* state values */
typedef enum {
    RSUHSTATE_UNKNOWN,
    RSUHSTATE_FWUPSTART,
    RSUHSTATE_FWUPEND,
    RSUHSTATE_LEDTESTSTART,
    RSUHSTATE_LEDTESTEND,
    RSUHSTATE_FAULTSET,
    RSUHSTATE_FAULTCLEAR,
    RSUHSTATE_NORMAL,
} rsustate_e;

/* special state case - hopefully we don't have more than 0xFF states;
   state will go in upper byte of upper word, pid (hopefully 0x7FFFFF is enough for pid)
   goes in lower word and low 7 bits of upper word) */
#define RSUHSTATE_QUERY   0x00800000
#define QUERY_PID_MASK    0x007FFFFF
#define QUERY_RESP_SHIFT  24


/* this is for any utility query issued using rsuhealth in command line mode
   to the service; add any new generic queries here */
typedef enum {
    RSUH_IPCHECK,
}  rsuhealth_e;

//TODO: maybe belong somewhere else? i2v_general.h or ?
#define RSUHEALTH_MIN_GPS_SV_COUNT 4  /* satellite vehicles */
#define RSU_TIME_TO_FIRST_FIX_MAX  60 /* seconds */

/* diagnostic results go here for apps to see. */
#define RSUHEALTH_SHM_PATH "/rsuhealth_shm"
#define FAULTLED_SHM_PATH "/faultled_shm"

/* 
 * be kind: If you add new values be sure to update print functions that use them.
 */

/* 64 bit error states for RSUHEALTH outside of diagnostic stuff. */
#define RSUHEALTH_SEC_CERT_FAIL     0x0000000000000001 /* no valid certs */
#define RSUHEALTH_SEC_CERT_EXPIRED  0x0000000000000002 /* cert about to expire */
#define RSUHEALTH_SEC_CERT_BOGUS    0x0000000000000004 /* acf file name wrong format */
#define RSUHEALTH_SEC_CERT_FULL     0x0000000000000008 /* acf files found exceed max */
#define RSUHEALTH_BOGUS_PVT_INDEX   0x0000000000000010
#define RSUHEALTH_STALE_SOUS        0x0000000000000020
#define RSUHEALTH_TPS_SHM_FAIL      0x0000000000000040
#define RSUHEALTH_SOUS_SHM_FAIL     0x0000000000000080
#define RSUHEALTH_SHM_FAIL          0x0000000000000100
#define RSUHEALTH_SSCANF_FAIL       0x0000000000000200
#define RSUHEALTH_GUI_SCRIPT_FAIL   0x0000000000000400
#define RSUHEALTH_READY_SET_FAIL    0x0000000000000800
#define RSUHEALTH_I2V_RESET         0x0000000000001000
#define RSUHEALTH_BOGUS_LED_STATE   0x0000000000002000
#define RSUHEALTH_READY_FILE_FAIL   0x0000000000004000
#define RSUHEALTH_SYSSETTS_SHM_FAIL 0x0000000000008000
#define RSUHEALTH_SHM_SEMA_FAIL     0x0000000000010000
#define RSUHEALTH_NSSTATS_FAILED    0x0000000000020000
#define RSUHEALTH_MANAGE_FOLDER_OFF 0x0000000000040000
#define RSUHEALTH_FGETS_FAIL        0x0000000000080000
#define RSUHEALTH_POPEN_FAIL        0x0000000000100000
#define RSUHEALTH_STALE_PVT         0x0000000000200000
#define RSUHEALTH_SYSCALL_FAIL      0x0000000000400000 /* system call failed somewhere, see syslog for details ie thread failed to start. */
#define RSUHEALTH_NETWORK_LINK_FAIL 0x0000000000800000
#define RSUHEALTH_NETWORK_INIT_FAIL 0x0000000001000000
#define RSUHEALTH_IP_GET_FAIL       0x0000000002000000
#define RSUHEALTH_IP_RX_GET_FAIL    0x0000000004000000
#define RSUHEALTH_IP_TX_GET_FAIL    0x0000000008000000
#define RSUHEALTH_SYS_CALL_FAIL     0x0000000010000000
#define RSUHEALTH_DUMP_REPORT_FAIL  0x0000000020000000
#define RSUHEALTH_IP_INFO_FAIL      0x0000000040000000
#define RSUHEALTH_NETMASK_GET_FAIL  0x0000000080000000 /* 32 */
#define RSUHEALTH_DCIN_FAIL         0x0000000100000000
#define RSUHEALTH_CPU_TEMP_FAIL     0x0000000200000000
#define RSUHEALTH_AMBIENT_TEMP_FAIL 0x0000000400000000
#define RSUHEALTH_DCIN_WARN         0x0000000800000000
#define RSUHEALTH_CPU_TEMP_WARN     0x0000001000000000
#define RSUHEALTH_AMBIENT_TEMP_WARN 0x0000002000000000
#define RSUHEALTH_MONITOR_FILE_FAIL 0x0000004000000000
#define RSUHEALTH_SEC_CERT_NEAR_EXP 0x0000008000000000

#define RSUHEALTH_BAD_MODE          0x0100000000000000 /* Unexpected case 'default' in switch statement */
#define RSUHEALTH_TRAP_TRUCATED     0x0200000000000000 /* snmp trao notify had to be truncated for size. */
#define RSUHEALTH_TRAP_DROPPED      0x0400000000000000 /* snmp trap notify had to drop lower level alert. No space left. */
#define RSUHEALTH_BAD_INPUT         0x0800000000000000 /* somebody passed in a NULL or bad paramter. */
#define RSUHEALTH_REALTIME_THR_TO   0x1000000000000000 /* realtime thread not responding */
#define RSUHEALTH_DIAG_THR_TO       0x2000000000000000 /* rsudiag thread not responding */
#define RSUHEALTH_STANDBY_MODE      0x4000000000000000 /* go to standby mode: no txrx, no ifclog, no fwding */
#define RSUHEALTH_FAULT_MODE        0x8000000000000000 /* 64: reboot if recovery enabled else go to standby */

/* 32 bit gnss error states */
#define RSUHEALTH_GNSS_FIX_VALID         0x00000001 /* If set we have a valid fix. AOK */
#define RSUHEALTH_GNSS_BAD_ANTENNA       0x00000002 /* These are off nominal events to consider for recovery. FAIL */
#define RSUHEALTH_GNSS_BAD_LOC_DEV       0x00000004
#define RSUHEALTH_GNSS_BAD_TTFF          0x00000008
//free, undefined                        0x00000010
#define RSUHEALTH_GNSS_BAD_FW            0x00000020
#define RSUHEALTH_GNSS_BAD_CFG           0x00000040
#define RSUHEALTH_1PPS_BAD_FETCH         0x00000080
#define RSUHEALTH_1PPS_BAD_LATENCY       0x00000100
#define RSUHEALTH_1PPS_TPS_BAD_LATENCY   0x00000200
#define RSUHEALTH_1PPS_BAD_TIMESTAMP     0x00000400
#define RSUHEALTH_1PPS_BAD_MKTIME        0x00000800
#define RSUHEALTH_1PPS_HALTED            0x00001000
#define RSUHEALTH_1PPS_INIT_FAIL         0x00002000
#define RSUHEALTH_GNSS_TPS_SEQNO_STUCK   0x00004000
#define RSUHEALTH_GNSS_PVT_STUCK         0x00008000
#define RSUHEALTH_GNSS_PVT_UNUSED        0x00010000
#define RSUHEALTH_GNSS_GGA_STUCK         0x00020000
#define RSUHEALTH_GNSS_NO_TIME_ACCURACY  0x00040000
#define RSUHEALTH_GNSS_BAD_TIME_ACCURACY 0x00080000
#define RSUHEALTH_GNSS_RTC_BAD_CAL       0x00100000
#define RSUHEALTH_GNSS_BAD_LEAPSEC       0x00200000
#define RSUHEALTH_GNSS_SPOOF_DETECT      0x00400000
#define RSUHEALTH_GNSS_JAM_PRESENT       0x00800000
#define RSUHEALTH_GNSS_JAM_NOFIX         0x01000000
#define RSUHEALTH_GNSS_SSCANF_FAIL       0x02000000
#define RSUHEALTH_GNSS_FGETS_FAIL        0x04000000
#define RSUHEALTH_GNSS_POPEN_FAIL        0x08000000

typedef struct {
  uint32_t badfix;
  uint32_t badantenna;
  uint32_t badlocdeviation;
  uint32_t badanomaly;
  uint32_t badfw;
  uint32_t badcfg;
  uint32_t badfetch;
  uint32_t bad1ppslatency;
  uint32_t bad1ppstpslatency;
  uint32_t badtimestamp;
  uint32_t badmktime;
  uint32_t bad1ppshalt;
} gnsserrorcounts_t;

/* 32 bit cv2x + hsm + rsk error states */
#define RSUHEALTH_CV2X_TX_READY         0x00000001
#define RSUHEALTH_CV2X_RX_READY         0x00000002
#define RSUHEALTH_CV2X1_BAD_ANTENNA     0x00000004
#define RSUHEALTH_CV2X2_BAD_ANTENNA     0x00000008
#define RSUHEALTH_CV2X_TEMP_FAIL        0x00000010
#define RSUHEALTH_CV2X_BAD_FW_REVISION  0x00000020
#define RSUHEALTH_CV2X_BAD_INTERFACE    0x00000040
#define RSUHEALTH_CV2X_INIT_FAIL        0x00000080
#define RSUHEALTH_HSM_TEMP_FAIL         0x00000100
#define RSUHEALTH_HSM_BAD_CFG           0x00000200
#define RSUHEALTH_HSM_BAD_JCOP          0x00000400
#define RSUHEALTH_HSM_BAD_USAPP         0x00000800
#define RSUHEALTH_HSM_BAD_GSA           0x00001000
#define RSUHEALTH_SECURITY_SIGN_FAIL    0x00002000
#define RSUHEALTH_SECURITY_VERIFY_FAIL  0x00004000
#define RSUHEALTH_AEROLINK_DIR_FAIL     0x00008000
#define RSUHEALTH_RSK_CONNECT_FAIL      0x00010000
#define RSUHEALTH_RSK_SEND_FAIL         0x00020000
#define RSUHEALTH_RSK_RECEIVE_FAIL      0x00040000
#define RSUHEALTH_HSM_SSCANF_FAIL       0x00080000
#define RSUHEALTH_HSM_FGETS_FAIL        0x00100000
#define RSUHEALTH_HSM_POPEN_FAIL        0x00200000
#define RSUHEALTH_ALSMI_SIGN_FAIL       0x00400000
#define RSUHEALTH_ALSMI_VERIFY_FAIL     0x00800000
#define RSUHEALTH_CV2X_TEMP_WARN        0x01000000
#define RSUHEALTH_HSM_TEMP_WARN         0x02000000
#define RSUHEALTH_SECURITY_FOPEN_FAIL   0x04000000

#define RSUHEALTH_CV2X_READY_MASK      (RSUHEALTH_CV2X_TX_READY|RSUHEALTH_CV2X_RX_READY)
#define RSUHEALTH_HSM_BAD_CFG_MASK     (RSUHEALTH_HSM_BAD_CFG|RSUHEALTH_HSM_BAD_JCOP|RSUHEALTH_HSM_BAD_USAPP|RSUHEALTH_HSM_BAD_GSA)

//TODO: tracked but not sent to log. needs work.
typedef struct {
  uint32_t badantenna1;
  uint32_t badantenna2;
  uint32_t badcputemperature;
  uint32_t badambienttemperature;
  uint32_t badcv2xtemperature;
  uint32_t badhsmtemperature;
} cv2xerrorcounts_t;

typedef struct { /* celcius */
  int32_t            cv2x;
  int32_t            cv2xmin;
  int32_t            cv2xmax;
  float64_t          hsm;
  float64_t          hsmmin;
  float64_t          hsmmax;
  float64_t          cpu;
  float64_t          cpumin;
  float64_t          cpumax;
  float64_t          ambient;
  float64_t          ambientmin;
  float64_t          ambientmax;
} temperatures_t;

typedef struct {
  uint64_t rxpackets; /* 64 bit ensures we are always big enough for incoming */
  uint64_t rxerrors;
  uint64_t rxdropped;
  uint64_t rxoverruns;
  uint64_t rxframe;
  uint64_t txpackets;
  uint64_t txerrors;
  uint64_t txdropped;
  uint64_t txoverruns;
  uint64_t txcarrier;
} ipcounts_t;

typedef struct {
  float32_t  dcin;   /* volts */
  float32_t  dcinmin;
  float32_t  dcinmax;
} volts_t;

/* 32 bit error states for system. */
#define RSUHEALTH_EMMCA_SSCANF_FAIL       0x1
#define RSUHEALTH_EMMCA_FGETS_FAIL        0x2
#define RSUHEALTH_EMMCA_POPEN_FAIL        0x4
#define RSUHEALTH_EMMCB_SSCANF_FAIL       0x8
#define RSUHEALTH_EMMCB_FGETS_FAIL        0x10
#define RSUHEALTH_EMMCB_POPEN_FAIL        0x20
#define RSUHEALTH_EMMCEOL_SSCANF_FAIL     0x40
#define RSUHEALTH_EMMCEOL_FGETS_FAIL      0x80
#define RSUHEALTH_EMMCEOL_POPEN_FAIL      0x100
#define RSUHEALTH_EMMCEOL_RANGE_FAIL      0x200
#define RSUHEALTH_EMMCUSE_SSCANF_FAIL     0x400
#define RSUHEALTH_EMMCUSE_FGETS_FAIL      0x800
#define RSUHEALTH_EMMCUSE_POPEN_FAIL      0x1000
#define RSUHEALTH_EMMCUSE_RANGE_FAIL      0x2000

#define RSUHEALTH_EMMCUSE_FAULT           0x40000000
#define RSUHEALTH_EMMCEOL_FAULT           0x80000000

typedef struct {
  float64_t  emmcusemin;   /* percent of space used */
  float64_t  emmcusemax;
} systems_t;

/* cv2xruntime bitfields */
#define RSUHEALTH_CV2X_ANTENNA_CHECK 0x1 /* If set then CV2X antenna check enabled */

#define RSUHEALTH_ALERT_MSG_LENGTH_MAX 255 /* dupe of RSUHEALTH_ALERT_MSG_LENGTH_MAX */

typedef struct {
  wsu_shm_hdr_t      h;                /* header to preserve */
  uint64_t           errorstates;      /* bitmask */
  uint32_t           gnssstatus;       /* bitmask */
  gnsserrorcounts_t  gnsserrorcounts;  /* error condition counters */
  uint32_t           cv2xstatus;       /* bitmask */
  cv2xerrorcounts_t  cv2xerrorcounts;  /* error condition counters */
  uint32_t           cv2xruntime;      /* bitmask */
  temperatures_t     temperature;      /* celcius */
  ipcounts_t         ipcounts;         /* nic statistics */
  volts_t            volts;            /* voltages */
  uint32_t           systemstates;     /* bitmask */
  systems_t          systems;          /* system stats */

/*
 * rsuAsync.c will clear Msg as ack that its has been serviced.
 * So if not "empty", you know the message has not gone out yet.
 * In that case, all you can do is throw your new message on the floor and set error state.
 *   5.18.1.1  done
 *         .2  done
 *         .3  SNMP auth fail (bad creds)
 *   5.18.1.4  done
 *         .5  Access Conrol List (done in rsuAccessMgr)
 *   5.18.1.6  done
 *         .7  time source mismatch
 *         .8  done
 *         .9  done
 *   5.18.1.10 done
 *   5.18.1.11 done
 *         .12 DoS
 *         .13 wd
 *         .14 done
 */
  int32_t trapsenabled; /* handshake: async thread will set to 1 when open for business */
  /* 5.18.1.1 File Integrity Check Error Message: AMH SAR& IMF: i.e. No valid message frame in UPER payload. */
  uint32_t messageFileIntegrityError;
  char_t  rsuMsgFileIntegrityMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.2 Storage Integrity Error Message: Errors in file system. */
  uint32_t rsuSecStorageIntegrityError;
  char_t  rsuSecStorageIntegrityMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.3 Authorization Error Message: invalid security creds in query: Is failed login attempt one? */
  uint32_t rsuAuthError;
  char_t  rsuAuthMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
  uint32_t rsuSignatureVerifyError;
  char_t  rsuSignatureVerifyMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
  uint32_t rsuAccessError;
  char_t  rsuAccessMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix. */
  uint32_t rsuTimeSourceLost;
  char_t  rsuTimeSourceLostMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.*/
  uint32_t rsuTimeSourceMismatch;
  char_t  rsuTimeSourceMismatchMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.8 GNSS Anomaly Message */
  uint32_t rsuGnssAnomaly;
  char_t  rsuGnssAnomalyMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* NOTE: Done by rsuGnssOutput in MIB, not by RSUDIAG */
  /* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed. */
  uint32_t rsuGnssDeviationError;
  char_t  rsuGnssDeviationMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* NOTE: Done by rsuGnssOutput in MIB, not by RSUDIAG */
  /* 5.18.1.10 */
  /* 5.18.1.11 Certificate Error Message */
  uint32_t rsuCertificateError;
  char_t  rsuCertificateMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.12 Denial of Service Error Message */
  uint32_t rsuServiceDenialError;
  char_t  rsuServiceDenialMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.13 Watchdog Error Message */
  uint32_t rsuWatchdogError;
  char_t  rsuWatchdogMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
  /* 5.18.1.14 Enclosure Environment Message: tamper detection */
  uint32_t rsuEnvironError;
  char_t  rsuEnvironMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];
} rsuhealth_t;

/* Customer folder status 32-bit */
#define CUSTOMER_CLEAN_STATE    0x00000001 /* Folder too full and had to be pruned. */
#define CUSTOMER_CLEAN_SYSFAIL  0x00000002 /* A system call failed. Check syslog for details. */

/* For Fault LED */ 

#define LED_NO_ERR 0
#define LED_ERR_1  1
#define LED_ERR_2  2
#define LED_ERR_3  3

typedef struct {
    int error_level;
    int start_time;
    char name[100];
} led_diagnostic_t;

typedef struct {
    wsu_shm_hdr_t    h; /* for shm */
    led_diagnostic_t diag_thread;
    led_diagnostic_t gps_fix;
    led_diagnostic_t time_accuracy;
    led_diagnostic_t gps_position;
    led_diagnostic_t gnss_antenna;
    led_diagnostic_t cv2x_antenna;
    led_diagnostic_t battery_voltage;
    led_diagnostic_t network;
    led_diagnostic_t temperature;
    led_diagnostic_t hsm;
    led_diagnostic_t cv2x_interface;
    led_diagnostic_t max_error;
    int              faultledstate;
} led_diagnostics_t;


#endif /* RSUHEALTH_H */

