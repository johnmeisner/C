/**************************************************************************
 *                                                                        *
 *     File Name:  fwdmsg.c  (Forward Messages)                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         2251 Rutherford Rd Ste 100                                     *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    The purpose of this process is to forward messages as a standalone 
    process.
 **************************************************************************/ 
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <poll.h>

#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "conf_table.h"
#include "fwdmsg.h"
#include "i2v_shm_master.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
#else
#define MAIN main
#define CONNECT    connect
#define INET_ATON  inet_aton
#define SOCKET     socket
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  3000  /* blobs rx. */
#else
#define OUTPUT_MODULUS  12300
#endif

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG     /* To serial & syslog. */
#else
#define MY_ERR_LEVEL   LEVEL_PRIV    /* To syslog only. */
#endif
#define MY_NAME        "fwdmsg"

/* Manage forwarding socket. Init and recover from failures. */
#define SOCKET_POLL_TIMEOUT  0       /* Zero wait. Don't hang around if not ready. */
#define FWD_SOCKET_MANAGEMENT_RATE 5 /* Seconds. */
#define FWD_WRITE_ERROR_THRESHOLD 10 /* Consecutive write fails to socket that triggers reset. */

int fwdmsgSock[MAX_FWD_MSG_IP] = {-1};
struct sockaddr_in dest[MAX_FWD_MSG_IP];
STATIC mqd_t fwdmsgfd;

STATIC i2vShmMasterT * shm_ptr = NULL;
WBOOL                  mainloop = WTRUE;
fwdmsgCfgItemsT fwdmsgCfg;          
cfgItemsT       cfg;               

/* Threads share socket so must lock. */
uint32_t fwdSocket_lock[MAX_FWD_MSG_IP] = {0};
uint32_t fwdSocket_lock_fail[MAX_FWD_MSG_IP] = {0}; /* Number of times we TO getting lock on socket. */

/* Management of fordwarding socket: These counts persist. */
uint32_t fwd_socket_fail[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_socket_invalid_ip[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_socket_connect_fail[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_socket_reset_count[MAX_FWD_MSG_IP] = {0};

/* Writing to forwarding socket: These counts reach FWD_WRITE_ERROR_THRESHOLD and socket reset occurs.*/
uint32_t fwd_write_count[MAX_FWD_MSG_IP] = {0};
uint32_t total_fwd_write_error_count[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_write_poll_revents[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_write_error_count[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_write_poll_to_count[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_write_poll_notready_count[MAX_FWD_MSG_IP] = {0};
uint32_t fwd_write_poll_fail_count[MAX_FWD_MSG_IP] = {0};

/* Limits forwarding debug output. */
int32_t busyLogger[MAX_FWD_MSG_IP] = {20}; 
static pthread_mutex_t fwdmsg_sock_mutex[MAX_FWD_MSG_IP] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};
STATIC uint32_t debug_counter = 0; /* for output control */
STATIC uint64_t my_error_states = 0x0;

static pthread_t monitorThreadID;
void *monitorThread(void __attribute__((unused)) *arg);
STATIC void set_my_error_state(int32_t my_error);  
 
STATIC pthread_t manageSocketThreadID;
STATIC void * manageSocketThread(void __attribute__((unused)) *arg);

void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);

  if((dummy <= 64) && (0 < dummy)) {
      my_error_states |= (uint64_t)(0x1) << (dummy - 1);
  }
}
void clear_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);
  if((dummy <= 64) && (0 < dummy)) {
      my_error_states &= ~((uint64_t)(0x1) << (dummy - 1));
  }
}
int32_t is_my_error_set(int32_t my_error)
{
  int32_t dummy = 0;
  int32_t ret = 0; /* FALSE till proven TRUE */

  dummy = abs(my_error);
  if((dummy <= 64) && (0 < dummy)) {
      if (my_error_states & ((uint64_t)(0x1) << (dummy - 1))) {
          ret = 1;
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only actionable items for user.              */
/*----------------------------------------------------------------------------*/
STATIC void dump_msgfwd_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/msgfwd.txt", "w")) == NULL){
      if(0x0 == is_my_error_set(FWDMSG_CUSTOMER_DIGEST_FAIL)) { /* Report fail one time. */
          set_my_error_state(FWDMSG_CUSTOMER_DIGEST_FAIL);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Unable to create user FWDMSG report.\n"); /* Not fatal */
      }
  } else {
      clear_my_error_state(FWDMSG_CUSTOMER_DIGEST_FAIL);
      if(is_my_error_set(FWDMSG_FWD_IP_ERROR)){
          fprintf(file_out,"FWDMSG_FWD_IP_ERROR: Invalid IP addr. Go to Web Gui->Configuration Settings->Forward Message and verify IP+Port used.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_CONNECT_ERROR)){
          fprintf(file_out,"FWDMSG_FWD_IP_ERROR: Unable to connect to IP+Port. Ensure host IP+Port is ready. Verify Web Gui->Configuration Settings->Forward Message IP+Port are correct.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_SOCKET_BUSY)){
          fprintf(file_out,"FWDMSG_FWD_SOCKET_BUSY: Host IP+Port is busy and attempt to send failed.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_SOCKET_NOT_READY)){
          fprintf(file_out,"FWDMSG_FWD_SOCKET_NOT_READ: Host IP+Port is not ready yet and data can not be forwarded.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_WRITE_ERROR)){
          fprintf(file_out,"FWDMSG_FWD_WRITE_ERROR: Send to Host IP+Port failed. Is RSU still connected to network? Is Host still available.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_WRITE_ERROR)){
          fprintf(file_out,"FWDMSG_FWD_WRITE_ERROR: Send to Host IP+Port failed because connection is busy. Potentially too much data being sent.\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_WRITE_POLL_FAIL)){
          fprintf(file_out,"FWDMSG_FWD_WRITE_POLL_FAIL: Unable to poll host IP+Port. Is RSU still connected to network? Is Host still available?\n"); 
      }
      if(is_my_error_set(FWDMSG_FWD_WRITE_POLL_TIMEOUT)){
          fprintf(file_out,"FWDMSG_FWD_WRITE_POLL_TIMEOUT: Timed out polling host IP+Port. Is RSU still connected to network? Is Host still available?\n"); 
      }
      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
  }
}
STATIC void init_statics(void)
{
    memset(fwdmsgSock,0x0,sizeof(fwdmsgSock));
    memset(dest,0x0,sizeof(dest));
    memset(&fwdmsgfd,0x0,sizeof(fwdmsgfd));
    shm_ptr = NULL;
    mainloop = WTRUE;
    memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfg));          
    memset(&cfg,0x0,sizeof(cfg));             
    memset(fwdSocket_lock,0x0,sizeof(fwdSocket_lock));
    memset(fwdSocket_lock_fail,0x0,sizeof(fwdSocket_lock_fail));

    memset(fwd_socket_fail,0x0,sizeof(fwd_socket_fail));
    memset(fwd_socket_invalid_ip,0x0,sizeof(fwd_socket_invalid_ip));
    memset(fwd_socket_connect_fail,0x0,sizeof(fwd_socket_connect_fail));
    memset(fwd_socket_reset_count,0x0,sizeof(fwd_socket_reset_count));

    memset(fwd_write_count,0x0,sizeof(fwd_write_count));
    memset(total_fwd_write_error_count,0x0,sizeof(total_fwd_write_error_count));
    memset(fwd_write_poll_revents,0x0,sizeof(fwd_write_poll_revents));
    memset(fwd_write_error_count,0x0,sizeof(fwd_write_error_count));
    memset(fwd_write_poll_to_count,0x0,sizeof(fwd_write_poll_to_count));
    memset(fwd_write_poll_notready_count,0x0,sizeof(fwd_write_poll_notready_count));
    memset(fwd_write_poll_fail_count,0x0,sizeof(fwd_write_poll_fail_count));

    memset(busyLogger,0x0,sizeof(busyLogger));
    memset(fwdmsg_sock_mutex,0x0,sizeof(fwdmsg_sock_mutex));
    debug_counter = 0; /* for output control */
    my_error_states = 0x0;
}
#if 0 // Wait for C Unit Tests to be Complete to cross check config values 
STATIC int32_t fwdmsgSetConfDefault(char_t * tag, void * itemToUpdate)
{

    return WTRUE;
}
#endif


/* fwdmsg_update_cfg()
 *    Reads both configs into fwdmsg_cfg
 * Returns TRUE on success, FALSE on failures
 */
STATIC WBOOL fwdmsg_update_cfg()
{
    cfgItemsTypeT cfgItems[] = {
        {"ForwardMessageEnable1",   (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[0].fwdmsgEnable,      NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageIP1",       (void *)i2vUtilUpdateStrValue,      &fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,       NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessagePort1",     (void *)i2vUtilUpdateUint16Value,   &fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdPort,     NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageMask1",     (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[0].fwdmsgMask,        NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageEnable2",   (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[1].fwdmsgEnable,      NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageIP2",       (void *)i2vUtilUpdateStrValue,      &fwdmsgCfg.fwdmsgAddr[1].fwdmsgFwdIp,       NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessagePort2",     (void *)i2vUtilUpdateUint16Value,   &fwdmsgCfg.fwdmsgAddr[1].fwdmsgFwdPort,     NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageMask2",     (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[1].fwdmsgMask,        NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageEnable3",   (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[2].fwdmsgEnable,      NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageIP3",       (void *)i2vUtilUpdateStrValue,      &fwdmsgCfg.fwdmsgAddr[2].fwdmsgFwdIp,       NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessagePort3",     (void *)i2vUtilUpdateUint16Value,   &fwdmsgCfg.fwdmsgAddr[2].fwdmsgFwdPort,     NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageMask3",     (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[2].fwdmsgMask,        NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageEnable4",   (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[3].fwdmsgEnable,      NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageIP4",       (void *)i2vUtilUpdateStrValue,      &fwdmsgCfg.fwdmsgAddr[3].fwdmsgFwdIp,       NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessagePort4",     (void *)i2vUtilUpdateUint16Value,   &fwdmsgCfg.fwdmsgAddr[3].fwdmsgFwdPort,     NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageMask4",     (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[3].fwdmsgMask,        NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageEnable5",   (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[4].fwdmsgEnable,      NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageIP5",       (void *)i2vUtilUpdateStrValue,      &fwdmsgCfg.fwdmsgAddr[4].fwdmsgFwdIp,       NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessagePort5",     (void *)i2vUtilUpdateUint16Value,   &fwdmsgCfg.fwdmsgAddr[4].fwdmsgFwdPort,     NULL,(ITEM_VALID| UNINIT)},
        {"ForwardMessageMask5",     (void *)i2vUtilUpdateUint8Value,    &fwdmsgCfg.fwdmsgAddr[4].fwdmsgMask,        NULL,(ITEM_VALID| UNINIT)},
    };

    FILE *f;
    char_t fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
    uint32_t retVal = I2V_RETURN_OK;
    uint32_t i;

    /* Parse fwdmsg.conf */
    strncpy(fileloc, I2V_CONF_DIR, I2V_CFG_MAX_STR_LEN);
    strcat(fileloc, FWDMSG_CONF_FILE);
    f = fopen(fileloc, "r");

    /* If not in a unit test, then failing to open FWDMSG_CFG_FILE is fatal */
    if (f == NULL) {
      set_my_error_state(FWDMSG_OPEN_CONF_FAIL);
      return WFALSE; 
    }

    retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL);
    if (retVal != I2V_RETURN_OK)   {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: not happy retVal=%u.\n",retVal);
      retVal = FWDMSG_LOAD_CONF_FAIL;
      set_my_error_state(FWDMSG_LOAD_CONF_FAIL); /* Note for user. */

      if(NULL != f) {
        fclose(f);
      }

      return WFALSE; 
    }

    if(NULL != f) {
      fclose(f);
    }

#if 0 // Wait for C Unit Tests to be Complete to cross check config values 
    /* Check IP Address */
    for ( i = 0; i < MAX_FWD_MSG_IP; i++)   {
        if(I2V_RETURN_OK != i2vCheckIPAddr(fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdIp[i], I2V_IP_ADDR_V4)) {
            /* Remember to check this if any item is added of deleted from cfgItems */
            cfgItems[i+4].state = (ITEM_VALID|BAD_VAL);
            retVal = FWDMSG_LOAD_CONF_FAIL;
            set_my_error_state(FWDMSG_LOAD_CONF_FAIL); /* Note for user. */
        }
    }

    /* Regardless of i2vRet, check. */
    for (i = 0; i < NUMITEMS(cfgItems); i++) {
        if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
            || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
            if(I2V_RETURN_OK == (retVal = fwdmsgSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                cfgItems[i].state = (ITEM_VALID|INIT);
                set_my_error_state(FWDMSG_HEAL_CONF_ITEM); /* Note for user. */
                retVal = I2V_RETURN_OK;
            } else {
                break; /* Heal has failed. FATAL. */
            }
        }
    }
#endif

    /* I2V SHM is ready now. Copy the fwdmsgCfg to the shared memory */
    WSU_SEM_LOCKW(&shm_ptr->fwdmsgCfgData.h.ch_lock);
    shm_ptr->fwdmsgCfgData.h.ch_data_valid = WTRUE;
    memcpy(&shm_ptr->fwdmsgCfgData.fwdmsgAddr, &fwdmsgCfg.fwdmsgAddr, sizeof(fwdmsgCfg.fwdmsgAddr));  /* save config for later use */
    WSU_SEM_UNLOCKW(&shm_ptr->fwdmsgCfgData.h.ch_lock);

    /* Wait on I2V SHM to get I2V CFG.*/
    for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
      WSU_SEM_LOCKR(&shm_ptr->cfgData.h.ch_lock);
      if(WTRUE == shm_ptr->cfgData.h.ch_data_valid) {
          memcpy(&cfg, &shm_ptr->cfgData, sizeof(cfgItemsT));  /* save config for later use */
          WSU_SEM_UNLOCKR(&shm_ptr->cfgData.h.ch_lock);
          break;
      }
      WSU_SEM_UNLOCKR(&shm_ptr->cfgData.h.ch_lock);
      usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    return (I2V_RETURN_OK == retVal) ? WTRUE : WFALSE;
}


int32_t fwdmsgSocketInit(void)
{
  int32_t retval = FWDMSG_AOK; /* TRUE till proven FALSE. */
  int i = 0;

    for ( i = 0; i < MAX_FWD_MSG_IP; i++ )  {

        if (fwdmsgCfg.fwdmsgAddr[i].fwdmsgEnable)   {

            if ((fwdmsgSock[i] = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unable to create forwarding sock[i] \n",i);
                retval = FWDMSG_FWD_SOCKET_ERROR;
            }

            if((FWDMSG_AOK == retval) && (0 == INET_ATON(fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdIp, &dest[i].sin_addr))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Invalid fwdmsg IP[%d] \n",i);     
                retval = FWDMSG_FWD_IP_ERROR;
            }

            if(FWDMSG_AOK == retval) {
                dest[i].sin_family = AF_INET;
                dest[i].sin_port = htons(fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdPort);
                if(CONNECT(fwdmsgSock[i], (struct sockaddr *)&dest[i], sizeof(dest[i])) < 0) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Connect error for fwdmsg IP[%d] \n",i);
                    retval = FWDMSG_FWD_CONNECT_ERROR;
                }
            }

            if(FWDMSG_AOK != retval) {
                if(-1 != fwdmsgSock[i]) {
                   close(fwdmsgSock[i]);
                }
                fwdmsgSock[i] = -1; /* On fail make sure it stays -1. */
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"fwdmsgSocketInit: socket fail[%d]: \n",i);
            }
        }
    }

    return retval;
}
static uint32_t max_report_errors = 0x0;
void FordwardSocketInit(int index)
{
  int32_t retval = FWDMSG_AOK; /* TRUE till proven FALSE. */

    /* set up spat,bsm, psm and map(tc legacy) forwarding sock. */
    if ((fwdmsgSock[index] = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        if(max_report_errors < 50) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unable to create forwarding sock[i] \n",index);
            max_report_errors++;
        }
        retval = FWDMSG_FWD_SOCKET_ERROR;
        fwd_socket_fail[index]++;
    }
    if((FWDMSG_AOK == retval) && (0 == INET_ATON((const char_t *)fwdmsgCfg.fwdmsgAddr[index].fwdmsgFwdIp, &dest[index].sin_addr))) {
        if(max_report_errors < 50) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Invalid fwdmsg IP[%d] \n",index);
            max_report_errors++;
        }   
        retval = FWDMSG_FWD_IP_ERROR;
        fwd_socket_invalid_ip[index]++;
    }

    if(FWDMSG_AOK == retval) {
        dest[index].sin_family = AF_INET;
        dest[index].sin_port = htons(fwdmsgCfg.fwdmsgAddr[index].fwdmsgFwdPort);
        if(CONNECT(fwdmsgSock[index], (struct sockaddr *)&dest, sizeof(dest[index])) < 0) {
            if(max_report_errors < 50) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Connect error for fwdmsg IP[%d] \n",index);
            }
            retval = FWDMSG_FWD_CONNECT_ERROR;
        }
    }

    if(FWDMSG_AOK != retval) {
        if(-1 != fwdmsgSock[index] ) {
           close(fwdmsgSock[index]);
        }
        fwdmsgSock[index] = -1; /* On fail make sure it stays -1. */
        if(max_report_errors < 50) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"fwdmsgSocketInit: socket fail[i]: \n",index);
            max_report_errors++;
        }
    }
    if(FWDMSG_AOK == retval) {
        max_report_errors = 0;
    }
}

/* Assumes you have locked socket for use. Do not allow debug output under lock. */
int32_t forwardWrite(uint8_t * buf, int32_t length, int32_t index)
{
  int32_t ret = FWDMSG_AOK;
  struct pollfd write_poll;
  uint8_t *data;
  uint32_t realepoch;
  wtime epoch;

  if((NULL == buf) || (length < 1) || (MAX_FWD_MSG_DATA < length)) {
      ret = FWDMSG_BAD_INPUT;
  } else {
      if (-1 == fwdmsgSock[index]) {
          /* Socket is not ready yet. Don't care, carry on. */
          ret = FWDMSG_FWD_SOCKET_NOT_READY;
      } else {
          /*
           * poll() socket to see if Clear to Send.
           */
          write_poll.fd      = fwdmsgSock[index];
          write_poll.events  = POLLOUT|POLLPRI;
          write_poll.revents = 0;
          errno = 0;
          if(0 < (ret = poll(&write_poll,1,SOCKET_POLL_TIMEOUT))) {
              if(write_poll.revents & (POLLHUP|POLLPRI|POLLERR|POLLNVAL)) {
                  fwd_write_poll_revents[index]++;
              }
              if(write_poll.revents & POLLOUT) { /* If same subnet then connect works but every other write fails. Odd. */
                  /* CTS */
                  if (!cfg.i2vUseDeviceID) {
                    ret = sendto(fwdmsgSock[index], buf, length, 0, (struct sockaddr *)&dest[index],sizeof(dest[index]));
                  }
                  else {

                    if (NULL == (data = calloc(1, length + I2VRSUIDSIZE + sizeof(unsigned int)))) {
                        ret = FWDMSG_CALLOC_ERROR;
                    }

                    memcpy(data, &cfg.i2vDeviceID, I2VRSUIDSIZE);
                    epoch = (wtime)(i2vUtilGetTimeInMs()/1000); /* wtime tracks msecs as well; don't ship that; just epoch */
                    realepoch = (unsigned int)epoch;   /* real epoch fits in 4 bytes */
                    memcpy(data + I2VRSUIDSIZE, &realepoch, sizeof(unsigned int));
                    memcpy(data + I2VRSUIDSIZE + sizeof(unsigned int), buf, length);
                    /* New length: */
                    length = length +  I2VRSUIDSIZE + sizeof(unsigned int);

                    ret = sendto(fwdmsgSock[index], data, length, 0, (struct sockaddr *)&dest[index],sizeof(dest[index]));
                    free(data);
                  }
                  if (ret != -1)    {
                      fwd_write_count[index]++;
                      fwd_write_error_count[index] = 0; /* Reset consecutive fail threshold. */
                      fwd_write_poll_notready_count[index] = 0;
                      fwd_write_poll_to_count[index]   = 0;
                      fwd_write_poll_fail_count[index] = 0;
                      ret = FWDMSG_AOK;
                  } else {
                      total_fwd_write_error_count[index]++;
                      fwd_write_error_count[index]++;
                      ret = FWDMSG_FWD_WRITE_ERROR;
                  }
              } else {
                  /* !CTS */
                  fwd_write_poll_notready_count[index]++;
                  ret = FWDMSG_FWD_WRITE_NOT_READY;
              }
          } else {
              if(0 == ret) {
                  /* TO: Is this likely to occur once we have connected? */
                  fwd_write_poll_to_count[index]++;
                  ret = FWDMSG_FWD_WRITE_POLL_TIMEOUT;
              } else {
                  /* poll() malfunction, output errno. */
                  fwd_write_poll_fail_count[index]++;
                  ret = FWDMSG_FWD_WRITE_POLL_FAIL;
              }
          }
      }
      if(ret < FWDMSG_AOK) {
          set_my_error_state(ret);
      }
  }
  return ret;
}

STATIC void * manageSocketThread(void __attribute__((unused)) *arg)
{
  uint32_t rolling_counter[MAX_FWD_MSG_IP] = {0};
  uint32_t i,j;

  while(mainloop) {
      for (i = 0; i < MAX_FWD_MSG_IP; i++) {

          pthread_mutex_lock(&fwdmsg_sock_mutex[i]);

          if (-1 == fwdmsgSock[i]) { /* If sock is -1 then no need to wait for lock. Nobody using socket. */
              fwdSocket_lock[i] = 1; /* Lock it. Not really needed but to be strict do it. */
              FordwardSocketInit(i);
              fwdSocket_lock[i] = 0; /* Unlock. If socket isn't ready that will block other fwd threads anyways. */
          } else {
              /*
               * If thresholds met then reset for the next interation to re-open.
               * Fails given equal weight today but that could be fined tuned.
               * Key is it has to be consecutive fails to avoid reset on just intermittent fails.
               * There's lots of event syslog output for intermittent fails if user cares.
               */
              if(   (FWD_WRITE_ERROR_THRESHOLD < fwd_write_error_count[i])
                 || (FWD_WRITE_ERROR_THRESHOLD < fwd_write_poll_notready_count[i])
                 || (FWD_WRITE_ERROR_THRESHOLD < fwd_write_poll_to_count[i] )
                 || (FWD_WRITE_ERROR_THRESHOLD < fwd_write_poll_fail_count[i])) {

                  for(j=0;(j < FWD_SOCKET_LOCK_ATTEMPT_MAX);j++){ /* Play nice, and get lock. */
                      if(0 == fwdSocket_lock[i]) {
                          fwdSocket_lock[i] = 1;
                          break;
                      } else {
                          usleep(FWD_SOCKET_LOCK_ATTEMPT_MAX); /* Wait a bit then force the issue. */
                      }
                  }
                  if(FWD_SOCKET_LOCK_ATTEMPT_MAX <= j){
                      fwdSocket_lock_fail[i]++; /* It's a don't care but track. */
                  }

                  /* Reset all now. */
                  close(fwdmsgSock[i]);
                  fwdmsgSock[i] = -1;
                  fwd_socket_reset_count[i]++;
                  I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"fwd socket: Resetting socket for %u time, We:!RDY:TO:TOe[%u,%u,%u,%u]\n",
                      fwd_socket_reset_count[i],fwd_write_error_count[i], fwd_write_poll_notready_count[i], fwd_write_poll_to_count[i], fwd_write_poll_fail_count[i]);
                  fwd_write_error_count[i] = 0;
                  fwd_write_poll_notready_count[i] = 0;
                  fwd_write_poll_to_count[i]   = 0;
                  fwd_write_poll_fail_count[i] = 0;
                  fwdSocket_lock[i] = 0; /* Unlock for use. */
              }
          }
          sleep(FWD_SOCKET_MANAGEMENT_RATE); /* No rush. */
          #if defined(MY_UNIT_TEST)
          if(15 < rolling_counter[i]) {
              mainloop=WFALSE;
              break;
          }
          #endif
          if(0 == (rolling_counter[i] % (OUTPUT_MODULUS/50))) {
              I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"fwd socket[%d]: TX:TXe:Le[%u,%u,%u] We:!RDY:TO:TOe[%u,%u,%u,%u] Sf:IPf:Cf:R#:Pe[%u,%u,%u,%u,%u]\n",
                  i,fwd_write_count[i],total_fwd_write_error_count[i],fwdSocket_lock_fail[i],fwd_write_error_count[i],fwd_write_poll_notready_count[i],fwd_write_poll_to_count[i]
                  ,fwd_write_poll_fail_count[i],fwd_socket_fail[i],fwd_socket_invalid_ip[i],fwd_socket_connect_fail[i],fwd_socket_reset_count[i],fwd_write_poll_revents[i]);
          }
          rolling_counter[i]++; 

          pthread_mutex_unlock(&fwdmsg_sock_mutex[i]);
      } //for
  } //while

  for (i = 0; i < MAX_FWD_MSG_IP; i++)  {
      /* Close socket. */
      if(-1 != fwdmsgSock[i]) {
          close(fwdmsgSock[i]);
          fwdmsgSock[i] = -1;
      }

  }
  pthread_exit(NULL);
}


/* This thread monitors the messages from other processes. Once received, it sends the packets
IP addresses that were enabled to receive the messages in config file */
void *monitorThread(void __attribute__((unused)) *arg)
{
    fwdmsgData  rcvfwdmsgData; 
    uint8_t  buf[sizeof(fwdmsgData) + 1] = {0};
    int32_t  ret;
    int32_t i,j;

    /* Monitor the thread at 25ms */
    while (mainloop)
    {
        memset(&rcvfwdmsgData,0,sizeof(fwdmsgData));
        memset(buf,0,sizeof(buf));
        if ( -1 == (mq_receive(fwdmsgfd, (char_t *)&rcvfwdmsgData, sizeof(fwdmsgData) + 1, NULL)))
        {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," mq_receive failed : errno = %d(%s)\n",errno,strerror(errno));
#endif
        }
        else    {

            for (i = 0; i < MAX_FWD_MSG_IP; i++)
            {
                /* Get the message type */
                if ((fwdmsgSock[i] != -1) && (fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask & rcvfwdmsgData.fwdmsgType) && (fwdmsgCfg.fwdmsgAddr[i].fwdmsgEnable))  {
                    /* Copy the data to the buffer */
                    memcpy(&buf,rcvfwdmsgData.fwdmsgData,rcvfwdmsgData.fwdmsgDataLen);

                    for(j=0;(j < FWD_SOCKET_LOCK_ATTEMPT_MAX);j++){ /* Lock it. */
                      if(0 == fwdSocket_lock[i]) {
                          fwdSocket_lock[i] = 1;
                          ret = forwardWrite(buf,rcvfwdmsgData.fwdmsgDataLen,i);
                          fwdSocket_lock[i] = 0;
                          if(FWDMSG_AOK != ret){
                              if (busyLogger[i]) {
                                  I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Forward FAILED %d bytes to %s %d: ret = %d.\n", rcvfwdmsgData.fwdmsgDataLen, fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdIp, fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdPort,ret);
                                  busyLogger[i]--;
                              }
                          }
                          break;
                      } else {
                          usleep(FWD_SOCKET_LOCK_WAIT_MAX); /* wait a bit. */
                      }
                    }
                    if(FWD_SOCKET_LOCK_ATTEMPT_MAX <= i) {
                      if (busyLogger[i]) {
                          I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Dropped data, busy: %d bytes to %s %d\n", rcvfwdmsgData.fwdmsgDataLen, fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdIp, fwdmsgCfg.fwdmsgAddr[i].fwdmsgFwdPort,ret);
                          busyLogger[i]--;
                      }
                      fwdSocket_lock_fail[i]++;
                      set_my_error_state(FWDMSG_FWD_SOCKET_BUSY);
                    }
                }
            }
        }
#if defined(MY_UNIT_TEST)
        static uint32_t rolling_counter = 0;
        if(15 < rolling_counter) {
            mainloop=WFALSE;
            break;
        }
        sleep(1);
        rolling_counter++;
#else
        usleep(5000);
#endif
    }
    return 0;
}

STATIC void fwdmsg_sighandler(int __attribute__((unused)) sig)
{
#if !defined(MY_UNIT_TEST)
  mainloop = WFALSE;
#endif
    set_my_error_state(FWDMSG_SIG_FAULT);   
    /* otherwise fwdmsg may get stuck waiting for scs which may already be killed */
}

int MAIN(void)
{
    int32_t ret = FWDMSG_AOK;
    struct sigaction sa;
    int32_t i;

    init_statics();

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    shm_ptr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shm_ptr == NULL) {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME," critical error: Main Shared memory init failed\n");
      ret = FWDMSG_SHM_FAIL;
      goto out;
    }

    // Read config files into our shm for later
    if (fwdmsg_update_cfg() != WTRUE) {
        ret = FWDMSG_LOAD_CONF_FAIL;
        goto out_shm;
    }

    /* catch SIGINT/SIGTERM */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = fwdmsg_sighandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Init the sockets here */
    if (fwdmsgSocketInit() == -1)   {
        goto out_shm;
    }

    if ((fwdmsgfd = mq_open(I2V_FWDMSG_QUEUE, O_RDWR| O_NONBLOCK)) == -1) {
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME," mq() failed to open: errno = %d(%s)\n",errno,strerror(errno)); 
        ret = FWDMSG_OPEN_MQ_FAIL;
        goto out_shm;
    }


    if (0 == (ret = pthread_create(&manageSocketThreadID, NULL, manageSocketThread, NULL))) {
      pthread_detach(manageSocketThreadID);
    } else {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"manageSocketThread: Failed. Not Fatal: errno=%d.\n",ret, errno);
      set_my_error_state(FWDMSG_CREATE_FWD_THREAD_FAIL);
      /* If this thread creation fails, don't start the mainloop */
      goto out_queue;
    }

    /* Create the thread to monitor message queue for forward messages */
    if (-1 == pthread_create(&monitorThreadID, NULL, monitorThread, NULL)) {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME," Failed to start Forwading threads\n");
      set_my_error_state(FWDMSG_CREATE_FWD_THREAD_FAIL);    
      /* If this thread creation fails, don't start the mainloop */
      goto out_queue;
    }

    pthread_detach(monitorThreadID);

    while (mainloop)    {

        /* Wait here */
#if defined(MY_UNIT_TEST)
        sleep(1);
        /* If child threads don't exit then do it for them. Ideally child exit first and no race on exit. */
        if(30 < debug_counter) {
            mainloop=WFALSE;
        }
#else
        usleep(100000);
#endif
        if(0 == (debug_counter % OUTPUT_MODULUS)){
            I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%x)\n",my_error_states);
        }
        if(0 == (debug_counter % 600)){
            dump_msgfwd_report();
        }
        debug_counter++;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"EXIT:(0x%x) debug_counter=%u\n",my_error_states,debug_counter);
#endif
    /* Check if pthread_wait is required here */

out_queue:
    mq_unlink(I2V_FWDMSG_QUEUE);
    mq_close(fwdmsgfd);

out_shm:
    if(NULL != shm_ptr) {
        wsu_share_kill(shm_ptr, sizeof(i2vShmMasterT));
    }

    for (i = 0; i < MAX_FWD_MSG_IP; i++)    {
        if (fwdmsgSock[i] != -1)
            close(fwdmsgSock[i]);
    }

out:
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    return ret;
} /* END OF MAIN */
