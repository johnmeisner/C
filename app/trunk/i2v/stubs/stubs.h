 
/* Pure Stub function only */
#ifndef STUBS_H
#define STUBS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "ipcsock.h"
#include "rtxsrc/rtxContext.h"
#include "i2v_shm_master.h"
typedef enum {
    Signal_wsuRisInitRadioComm = 0 /* Make sure starts at zero! */
   ,Signal_risUserServiceRequest_1
   ,Signal_risUserServiceRequest_2
   ,Signal_releaseRPS
   ,Signal_wsuRisGetCfgReq
   ,Signal_wsuRisUserServiceRequest
   ,Signal_wsuRisWsmServiceRequest
   ,Signal_wsuRisTerminateRadioComm
   ,Signal_WSU_SEM_LOCKR
   ,Signal_WSU_SEM_UNLOCKR
   ,Signal_WSU_SEM_LOCKW    
   ,Signal_WSU_SEM_UNLOCKW
   ,Signal_wsu_open_gate
   ,Signal_wsu_init_gate
   ,Signal_wsu_kill_gate
   ,Signal_wsu_share_init
   ,Signal_rtxDListFindByIndex
   ,Signal_rtxDListInit
   ,Signal_asn1Init_SignalRequestList
   ,Signal_rtxMemHeapCreate_1
   ,Signal_rtxMemHeapCreate_2
   ,Signal_rtxPreInitContext
   ,Signal_rtxInitContext
   ,Signal_rtxInitASN1Context
   ,Signal_asn1Init_VehicleID
   ,Signal_asn1Init_RegionalExtension
   ,Signal_asn1Init_RequestorType
   ,Signal_asn1Init_Position3D_regional
   ,Signal_asn1Init_Position3D
   ,Signal_asn1Init_RequestorPositionVector
   ,Signal_asn1Init_TransitVehicleStatus
   ,Signal_RequestorDescription_regional
   ,Signal_asn1Init_RequestorDescription
   ,Signal_asn1Init_SignalRequestMessage_regional
   ,Signal_asn1Init_SignalRequestMessage
   ,Signal_asn1Init_MessageFrame
   ,Signal_pu_setBuffer
   ,Signal_rtxInitContextBuffer
   ,Signal_asn1PD_MessageFrame
   ,Signal_asn1PE_MessageFrame
   ,Signal_asn1PD_SignalRequestMessage_regional
   ,Signal_asn1PD_SignalRequestMessage_1
   ,Signal_asn1PD_SignalRequestMessage_2
   ,Signal_asn1PE_SignalRequestMessage
   ,Signal_rtxErrReset
   ,Signal_pu_getMsgLen_1
   ,Signal_pu_getMsgLen_2
   ,Signal_i2vUtilParseConfFile
   ,Signal_asn1Init_RequestorDescription_regional
   ,Signal_asn1PE_SignalRequestMessage_regional
   ,Signal_wsu_share_kill
   ,Signal_i2vDirectoryListing
   ,Signal_i2vUtilCertOrDigest
   ,Signal_wsuRisSendWSMReq_1
   ,Signal_wsuRisSendWSMReq_2
   ,Signal_i2vUtilCreateNewLogFiles_1
   ,Signal_i2vUtilCreateNewLogFiles_2
   ,Signal_i2vUtilCreateNewLogFiles_3
   ,Signal_write_1
   ,Signal_pe_GetMsgPtr
   ,Signal_inet_aton
   ,Signal_socket
   ,Signal_wsu_shmlock_kill
   ,Signal_wsu_shmlock_init
   ,Signal_mq_open
   ,Signal_mq_close
   ,Signal_mq_receive
   ,Signal_mq_send
   ,Signal_mq_getattr
   ,Signal_mq_unlink
   ,Signal_wsuTpsInit
   ,Signal_wsuTpsRegister
   ,Signal_wsu_wait_at_gate
   ,Signal_timer_gettime
   ,Signal_timer_settime
   ,Signal_timer_create
   ,Signal_timer_delete
   ,Signal_wsuRisGetRadioMacaddress
   ,Signal_wsuRisSetCfgReq
   ,Signal_shm_open
   ,Signal_shm_unlink
   ,Signal_rtInitContextUsingKey //Signal_rtInitContext
   ,Signal_netsnmp_register_scalar
   ,Signal_dump_sdm_conf_to_file
   ,Signal_sem_open
   ,Signal_pthread_create
   ,Signal_wsuRisProviderServiceRequest
   ,Signal_wsuRisWsmServiceRequest_2
   ,Signal_spat_update_cfg
   ,Signal_asn1PE_SPAT
   ,Signal_isUserService
   ,Signal_rtxMemAlloc
   ,Signal_asn1EncodeUPER_MessageFrame
   ,Signal_spat_process_main_send_fail
   ,Signal_fopen_fail
   ,Signal_spat_init_radio_bsm_enable
   ,Signal_bsmFwdSock_error
   ,Signal_inet_aton_error
   ,Signal_connect_error
   ,Signal_getifaddrs
   ,Signal_ifa_addr_error
   ,Signal_getnameinfo_error
   ,Signal_snmp_parse_args
   ,Signal_snmp_open
   ,Signal_snmp_pdu_create
   ,Signal_snmp_add_null_var
   ,Signal_snmp_synch_response
   ,Signal_bind
   ,Signal_disable_scs
   ,Signal_enable_Snmpscs
   ,Signal_toggle_tomformat
   ,Signal_select_fail 
   ,Signal_select_aok
   ,Signal_select_busy
   ,Signal_DN_FD_ISSET_fail
   ,Signal_DN_FD_ISSET_force_fail
   ,Signal_recvfrom_aok
   ,Signal_recvfrom_fail
   ,Signal_inet_ntoa_aok
   ,Signal_icdParseSNMPSpat_aok
   ,Signal_icdParseSNMPSpat_missing_data
   ,Signal_icdParseSpat_aok 
   ,Signal_icdParseSpat_missing_data
   ,Signal_snmp_parse_args_aok
   ,Signal_oid_phase_number
   ,Signal_oid_phase_number_bad_type
   ,Signal_oid_phase_number_not_active
   ,Signal_oid_phase_min
   ,Signal_oid_phase_min_bad_type
   ,Signal_oid_phase_time_remain
   ,Signal_oid_phase_time_remain_bad_type
   ,Signal_oid_phase_max_count
   ,Signal_oid_phase_max_count_bad_type
   ,Signal_oid_yellow_duration
   ,Signal_oid_yellow_duration_bad_type
   ,Signal_oid_yellow_phase
   ,Signal_oid_phase_enable
   ,Signal_snmp_parse_oid
   ,Signal_setsockopt
   ,Signal_pthread_detach
   ,Signal_mkfifo
   ,Signal_msgget
   ,Signal_msgctl
   ,Signal_open
   ,Signal_read
   ,Signal_popen_fail
   ,Signal_popen_over
   ,Signal_popen_under
   ,Signal_popen_bogus
   ,Signal_popen_toobig
   ,Signal_popen_empty
   ,Signal_test_control_1
   ,Signal_test_control_2
   ,Signal_test_control_3
   ,Signal_test_control_4
}Stub_Signal_Types;
#define Signal_rtInitContext Signal_rtInitContextUsingKey

void init_stub_control(void); /* Call first! */

void    set_stub_signal(wint32 signal);
void    set_stub_signal_iteration(wint32 signal,wint32 iteration);

wint32 get_stub_signal(wint32 signal);
void    clear_stub_signal(wint32 signal);

void    clear_all_stub_signal(void);
wint32 check_stub_pending_signals(void);

wint32 get_stub_iteration(wint32 signal);
wint32 dec_stub_iteration(wint32 signal);

#define MAX_CONFIG_ITEMS (1000)
#define MAX_SIGNALS      (1000)

typedef struct Stub_Control_Block_Type {

  wint32 Signal[MAX_SIGNALS];
  wint32 Iteration[MAX_SIGNALS];

} Stub_Control_Block_Type;

/* Wrapper Functions */
int dn_setsockopt(int socket, int level, int option_name,
const void *option_value, socklen_t option_len);

int dn_inet_aton(const char * cp, struct in_addr * addr);
int dn_socket(int domain,int type, int protocol);
int dn_bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
char * dn_inet_ntoa(struct in_addr in);
int dn_pthread_create (pthread_t *thread
                   , const pthread_attr_t *attr
                   , void *(*start_routine) (void *)
                   , void *arg);
int dn_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout);
int dn_pthread_detach(pthread_t thread);
ssize_t dn_recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

void DN_FD_ZERO(fd_set *set);
void DN_FD_SET(int fd, fd_set *set);
void DN_FD_CLR(int fd, fd_set *set);
int  DN_FD_ISSET(int fd, fd_set *set);

int dn_mkfifo(const char *pathname, mode_t mode);
int dn_msgget(key_t key, int msgflg);
int dn_msgctl(int msqid, int cmd, struct msqid_ds *buf);

int dn_open(const char *pathname, int flags);
ssize_t dn_read(int fd, void *buf, size_t count);

int SOCKET(int domain,int type, int protocol); 
int BIND(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int CONNECT(int s, const struct sockaddr * name, socklen_t namelen);
int INET_ATON(const char * cp, struct in_addr * addr);

void * WSU_SHARE_INIT( size_t size, char *spath);
void   WSU_SHARE_KILL( void * shrm_start, size_t size );
void   WSU_SHARE_DELETE( char_t * shm_path_str );
bool_t WSU_INIT_GATE( wsu_gate_t * wsu_sem);
bool_t WSU_KILL_GATE(wsu_gate_t * gate);
bool_t WSU_WAIT_AT_GATE(wsu_gate_t * wsu_sem);
bool_t WSU_OPEN_GATE(wsu_gate_t * wsu_sem);
bool_t WSU_SHMLOCK_LOCKW(wsu_shmlock_t * lock);
bool_t WSU_SHMLOCK_UNLOCKW(wsu_shmlock_t * lock);
bool_t WSU_SHMLOCK_KILL(wsu_shmlock_t * lock);
bool_t WSU_SHMLOCK_INIT(wsu_shmlock_t * lock);
bool_t WSU_SHMLOCK_UNLOCKR(wsu_shmlock_t * lock);
bool_t WSU_SHMLOCK_LOCKR(wsu_shmlock_t * lock);
void * RTXMEMALLOC(OSCTXT * ctxt, size_t blobLength);

FILE * POPEN(const char *command, const char *type);
int32_t PCLOSE(FILE * stream);

bool_t WSUSENDDATA (int32_t sock_fd, uint16_t port, void *data, size_t size);
int32_t WSURECEIVEDATA (uint32_t ignored_unused_timeout, RcvDataType *rcv_data);

void setnexttestiteration(uint32_t counter);

int32_t i2v_setupSHM_Default(i2vShmMasterT * shmPtr);
void i2v_setupSHM_Clean(i2vShmMasterT * shmPtr);

//deprecate
#define WSU_SEM_UNLOCKW WSU_SHMLOCK_UNLOCKW
#define WSU_SEM_UNLOCKR WSU_SHMLOCK_UNLOCKR
#define WSU_SEM_LOCKW   WSU_SHMLOCK_LOCKW
#define WSU_SEM_LOCKR   WSU_SHMLOCK_LOCKR

/* Test configs used. */
#define DEFAULT_CONFIG  "./"
#define NO_CONFIG       "../stubs/"
#define HOST_PC_IP      "10.52.11.47"  //Your PC IP here. Intention this is a valid IP.

#endif  /* STUBS_H */