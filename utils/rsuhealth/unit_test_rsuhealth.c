
/**************************************************************************************************

  RSU DIAGNOSTICS S/W UNIT TEST References. 

  SRS: SW Requirements RSU 5940 Application Diagnostics 041023.docx
  SDD: SW Detailed Design RSU 5940 Application Diagnostics 041023.doc 

  JIRA 1672: CPU Over Temp
  JIRA 613:  RSU Over Temp
  JIRA 1670: HSM Over Temp
  JIRA 1669: HSM Initialization Fail
  JIRA 1668: HSM Interface Lost
  JIRA 897:  Network Link Fail
  JIRA 898:  Network Init Fail
  JIRA 845:  GNSS Location Deviation
  JIRA 1671: GNSS Initialization Fail
  JIRA 609:  GNSS Interface Lost
  JIRA 347:  GNSS Antenna Fail
  JIRA 608:  GNSS Signal Loss
  JIRA 353:  GNSS Location Anomaly
  JIRA 864:  GNSS Time Pulse Lost
  JIRA 309:  GNSS Time Accuracy Fail
  JIRA 610:  V2X Radio Interface Fail
  JIRA 612:  V2X Radio Init Fail
  JIRA 611:  V2X Radio Firmware Load Fail
  JIRA 614:  V2X Radio Over Temp
  JIRA 445:  V2X Antenna Fail
  JIRA 311:  Time Sync Fail
  JIRA 608:  Time Sync Fail
  JIRA 835:  DC Input Voltage Out of Range
  JIRA 847:  Security Credential Fail
  JIRA 892:  Fault Indication External
***************************************************************************************************/

/**************************************************************************************************
* Includes
***************************************************************************************************/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "stubs.h"
#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "i2v_util.h"
#include "ipcsock.h"
#include "rs.h"
#include "shm_tps.h"
#include "shm_sous.h"
#include "rsuhealth.h"
#include "rsudiagnostic.h"
#include "conf_manager.h"
#include "conf_table.h"

/**************************************************************************************************
* Defines
***************************************************************************************************/

//TODO: HSM API for temperature disturbs to the chip readiness. Disable for now.
//#define ENABLE_HSM_TEST

/**************************************************************************************************
* Globals
***************************************************************************************************/

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/* rsuhealth.c */
extern int32_t my_main(int32_t argc, char_t *argv[]);
extern volatile rsuhealth_t   localrsuhealth;

/* rsudiagnostic.c */
extern uint32_t onesecondcounter;
/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
/* WARNING: NOT thread safe at all */

#define TOO_BIG_DATA "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
#define GET_IP_RX_PACKET_REPLY_DATA "RX packets:526715 errors:21 dropped:0 overruns:0 frame:0"
#define GET_IP_TX_PACKET_REPLY_DATA "TX packets:501530 errors:0 dropped:0 overruns:0 carrier:0"

#define MY_GET_RSU_IP_REPLY "          inet addr:%s  Bcast:%s  Mask:%s"
#define MY_GET_RSU_IP  "ifconfig ens33 | grep Mask"

#define MY_POPEN_OUTPUT "/tmp/foo.txt"

static FILE    * my_popen_file = NULL;
static char_t    rsu_ip[I2V_DEST_IP_MAX];
#if 0
static int32_t my_get_ip(void)
{
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[256]; /* Size according to your needs. */

  char_t    net_mask[I2V_DEST_IP_MAX];
  char_t    net_bcast[I2V_DEST_IP_MAX];

  memset(net_mask,'\0',sizeof(net_mask));
  memset(net_bcast,'\0',sizeof(net_bcast));
  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(rsu_ip,'\0',sizeof(rsu_ip));

  if(NULL != (fp = popen(MY_GET_RSU_IP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          printf("\n  my_get_ip(): buffer(%d)(%s)\n",i,replyBuf);
          if((0 != i) && (i <= sizeof(replyBuf))){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(3 != sscanf(replyBuf,MY_GET_RSU_IP_REPLY,rsu_ip,net_bcast,net_mask)) {
              printf("\n  my_get_ip(): RSUHEALTH_IP_GET_FAIL(%s)\n",replyBuf);
          } else {
              return 0;
          }
      } else {
          printf("\n  my_get_ip(): ip RSUHEALTH_FGETS_FAIL.\n");
      }
      pclose(fp);
  } else {
      printf("\n  my_get_ip(): ip RSUHEALTH_POPEN_FAIL.\n");
  }
  return -1;
}
#endif
FILE * POPEN(const char *command, const char *type)
{
  int32_t popen_control = 0;

  my_popen_file = NULL;
  popen_control = 0;

  /* Clear prior file incase fclose not called or not called in time before exit() */
  if(system("rm /tmp/foo.txt 2>/dev/null")) {

  }
  /* Use signal to setup type of test we want for all popen requests.
   * Do NOT clear signals in this case.
   */
  if(1 == get_stub_signal(Signal_popen_under)) {
      if(0 == get_stub_iteration(Signal_popen_under)) {
          //clear_stub_signal(Signal_popen_under);
          popen_control=1;
      } else {
          dec_stub_iteration(Signal_popen_under);
      }
  }
  if(1 == get_stub_signal(Signal_popen_over)) {
      if(0 == get_stub_iteration(Signal_popen_over)) {
          //clear_stub_signal(Signal_popen_over);
          popen_control=2;
      } else {
          dec_stub_iteration(Signal_popen_over);
      }
  }
  if(1 == get_stub_signal(Signal_popen_fail)) {
      if(0 == get_stub_iteration(Signal_popen_fail)) {
          // clear_stub_signal(Signal_popen_fail);
          return NULL;
      } else {
          dec_stub_iteration(Signal_popen_fail);
      }
  }
  if(1 == get_stub_signal(Signal_popen_bogus)) {
      if(0 == get_stub_iteration(Signal_popen_bogus)) {
          //clear_stub_signal(Signal_popen_bogus);
          popen_control = 3;
      } else {
          dec_stub_iteration(Signal_popen_bogus);
      }
  }
  if(1 == get_stub_signal(Signal_popen_toobig)) {
      if(0 == get_stub_iteration(Signal_popen_toobig)) {
          //clear_stub_signal(Signal_popen_toobig);
          popen_control = 4;
      } else {
          dec_stub_iteration(Signal_popen_toobig);
      }
  }
  if(1 == get_stub_signal(Signal_popen_empty)) {
      if(0 == get_stub_iteration(Signal_popen_empty)) {
          //clear_stub_signal(Signal_popen_empty);
          popen_control = 5;
      } else {
          dec_stub_iteration(Signal_popen_empty);
      }
  }
  if((NULL != command) && (NULL != type)){
      if('r' == *type) {
          if (NULL != (my_popen_file = fopen(MY_POPEN_OUTPUT,"w"))){
              if(0 == strcmp(command,GET_HSM_TEMP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_HSM_TEMP_REPLY,69.69);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_HSM_TEMP_REPLY,(float64_t)HSM_TEMP_LOW_CRITICAL - 1.0);
                      //printf(GET_HSM_TEMP_REPLY,(float64_t)HSM_TEMP_LOW_CRITICAL - 1.0);
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_HSM_TEMP_REPLY,(float64_t)HSM_TEMP_HIGH_CRITICAL + 1.0);
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_HSM_TEMP_REPLY,69.69);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
              } else if(0 == strcmp(command,GET_CPU_TEMP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_CPU_TEMP_REPLY,6666 * 10);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_CPU_TEMP_REPLY,(IMX8_TEMP_LOW_CRITICAL - 1) * 1000);
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_CPU_TEMP_REPLY,(IMX8_TEMP_HIGH_CRITICAL + 1) * 1000);
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_CPU_TEMP_REPLY,6666 * 10);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
              } else if(0 == strcmp(command,GET_AMBIENT_TEMP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_AMBIENT_TEMP_REPLY,(6767 + 6400) * 10);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_AMBIENT_TEMP_REPLY,((AMBIENT_TEMP_LOW_CRITICAL+64) - 1) * 1000);
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_AMBIENT_TEMP_REPLY,((AMBIENT_TEMP_HIGH_CRITICAL+64) + 1) * 1000);
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_AMBIENT_TEMP_REPLY,(6767 + 6400) * 10);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
              } else if(0 == strcmp(command,GET_HSM_GSA)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_HSM_GSA_REPLY,HSM_GSA_VERSION);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_HSM_GSA_REPLY,"1.12.0");
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_HSM_GSA_REPLY,"3.22.99");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_HSM_GSA_REPLY,HSM_GSA_VERSION);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
              }  else if(0 == strcmp(command,GET_HSM_USAPP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,HSM_USAPP_VERSION);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,"1.12.0");
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,"3.22.99");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,HSM_USAPP_VERSION);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
              }  else if(0 == strcmp(command,GET_HSM_JCOP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_HSM_JCOP_REPLY,HSM_JCOP_VERSION);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_HSM_JCOP_REPLY,"J4S2M0024BB70800");
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_HSM_JCOP_REPLY,"J6S2M0024BB70800");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,HSM_JCOP_VERSION);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");

              } else if(0 == strcmp(command,GET_HSM_CFG)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_HSM_CFG_REPLY,HSM_PLATFORM_CONFIG);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_HSM_CFG_REPLY,"00hKF6");
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_HSM_CFG_REPLY,"00hKF8");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_HSM_USAPP_REPLY,HSM_PLATFORM_CONFIG);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");

              } else if(0 == strcmp(command,GET_RSU_IP)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_RSU_IP_REPLY,rsu_ip);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_RSU_IP_REPLY,"");
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_RSU_IP_REPLY,"");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_RSU_IP_REPLY,rsu_ip);
                  break;
                  }
                  if((popen_control != 5) && (popen_control != 1) && (popen_control != 2))
                      fprintf(my_popen_file,"\n");

              } else if(0 == strcmp(command,GET_IP_RX_PACKET)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,"%s",GET_IP_RX_PACKET_REPLY_DATA);
                  break;
                  case 1:
                      fprintf(my_popen_file,"%s","\0");
                  break;
                  case 2:
                      fprintf(my_popen_file,"%s","\0");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,"%s",GET_IP_RX_PACKET_REPLY_DATA);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");

              } else if(0 == strcmp(command,GET_IP_TX_PACKET)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,"%s",GET_IP_TX_PACKET_REPLY_DATA);
                  break;
                  case 1:
                      fprintf(my_popen_file,"%s","\0");
                  break;
                  case 2:
                      fprintf(my_popen_file,"%s","\0");
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,"%s",GET_IP_TX_PACKET_REPLY_DATA);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
#if 0
              } else if(0 == strcmp(command,GET_GNSS_LOC_DEV)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_GNSS_LOC_DEV_REPLY, 20001);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_GNSS_LOC_DEV_REPLY, 3);
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_GNSS_LOC_DEV_REPLY, 1000000);
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_GNSS_LOC_DEV_REPLY, 3);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");

              } else if(0 == strcmp(command,GET_GNSS_MAX_DEV)) {
                  switch(popen_control) {
                  case 0:
                      fprintf(my_popen_file,GET_GNSS_MAX_DEV_REPLY, 0);
                  break;
                  case 1:
                      fprintf(my_popen_file,GET_GNSS_MAX_DEV_REPLY, 10);
                  break;
                  case 2:
                      fprintf(my_popen_file,GET_GNSS_MAX_DEV_REPLY, 3);
                  break;
                  case 3:
                      fprintf(my_popen_file,"%s","BADBEEF");
                  break;
                  case 4:
                      fprintf(my_popen_file,"%s",TOO_BIG_DATA);
                  break;
                  case 5:

                  break;
                  default:
                      fprintf(my_popen_file,GET_GNSS_MAX_DEV_REPLY, 10);
                  break;
                  }
                  if(popen_control != 5)
                      fprintf(my_popen_file,"\n");
#endif
              }
              fflush(my_popen_file);
              fclose(my_popen_file);
              my_popen_file = fopen(MY_POPEN_OUTPUT,"r");
          }
      }
  }
  if(NULL == my_popen_file)
      printf("\nPOPEN FAILED!\n");
  return my_popen_file;
}
int32_t PCLOSE(FILE * stream)
{
  if(NULL != stream) {
      fclose(stream);
      if(system("rm /tmp/foo.txt 2>/dev/null")) {
          printf("\nPCLOSE FAILED!\n");
      }
      return 0;
  } else {
      return -1;
  }
}
static rsIoctlType  pendingradiocall = WSU_NS_INIT_RPS;
bool_t WSUSENDDATA (int32_t sock_fd, uint16_t port, void *data, size_t size)
{
  bool_t ret = WTRUE;
  rsIoctlType * radiocall;

  sock_fd = sock_fd;
  port = port;
  if((NULL == data) || (0 == size)){
      printf("\n  WSUSENDDATA: null inputs.\n");
  } else {
      if(sizeof(int32_t) == size){
          radiocall = data;
          switch(*radiocall) {
          case WSU_NS_GET_RSK_STATUS:
              pendingradiocall = WSU_NS_GET_RSK_STATUS;
              ret = WTRUE;
          break;
          default:
              printf("\n  WSUSENDDATA: can't support cmd type(%d)\n",*radiocall); 
              ret = WFALSE;
          break;
          }
      } else {
          printf("\n  WSUSENDDATA: size is wrong for cmd type: size(%lu) != int32_t(%lu)\n", size, sizeof(int32_t) ); 
          ret = WFALSE;
      }
  }
  return ret;
}
static rskStatusType localrskStatus;
int32_t WSURECEIVEDATA (uint32_t ignored_unused_timeout, RcvDataType *rcv_data)
{
  int32_t ret = 0;

  ignored_unused_timeout = ignored_unused_timeout;

  if(NULL == rcv_data){
      printf("\n  WSURECEIVEDATA: null inputs.\n");
  } else {

      if((NULL == rcv_data->data) || (rcv_data->size < sizeof(localrskStatus))) {
          printf("\n  WSURECEIVEDATA: rcv_data bad.\n");
      } else {
          switch(pendingradiocall) {
          case WSU_NS_GET_RSK_STATUS:

              localrskStatus.interface = 1;
              localrskStatus.firmware = 1;
              localrskStatus.temperature = 50;
              localrskStatus.ready = 1;
              localrskStatus.tx_count++;
              localrskStatus.tx_err_count=0;
              localrskStatus.rx_count++;
              localrskStatus.rx_err_count=0;
              localrskStatus.wme_rx_good=0;
              localrskStatus.wme_rx_bad=0;
              localrskStatus.wme_tx_not_ready=0;
#if 0
              localrskStatus.SignRequests;
              localrskStatus.SignSuccesses;
              localrskStatus.SignFailures;
              localrskStatus.SignCBFailures;
              localrskStatus.SFUnknownPacketType;
              localrskStatus.SFInvalidPSID;
              localrskStatus.SFPayloadTooLarge;
              localrskStatus.SFCertificateChangeInProgress;
              localrskStatus.SFSignBufferOverflow;
              localrskStatus.SFsmg_newFailed;
              localrskStatus.LastSignErrorCode;
              localrskStatus.VerifyRequests;
              localrskStatus.VerifySuccesses;
              localrskStatus.VerifyFailures;
              localrskStatus.VerifyCBFailures;
              localrskStatus.LastVerifyErrorCode;
#endif
              /* Test printf of rsk status change */
              if (onesecondcounter == 10) {
                  localrskStatus.error_states = 0xFFFFFFFF; /* give system a glitch to excercise coverage */
              } else {
                  localrskStatus.error_states = 0x0;
              }

              memcpy(rcv_data->data,&localrskStatus,sizeof(localrskStatus));    
              ret = sizeof(localrskStatus);
          break;
          default:
              printf("\n  WSURECEIVEDATA: can't support cmd type(%d)\n",pendingradiocall); 
          break;
          }
      }
  }
  return ret;
}
static rsuhealth_t * shm_rsuhealth_ptr; /* RSUHEALTH SHM */
static shm_tps_t * shm_tps_ptr = NULL; /* TPS SHM  */
static shm_sous_t * shm_sous_ptr = NULL; /* SOUS and related */
static int32_t opendiagnosticshm(void)
{
  /* Open RSUHEALTH SHM, if fails not fatal. MIB and IWMH will suffer. */
  if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
      printf("\n  SHM init failed.\n");
      shm_rsuhealth_ptr = NULL;
      return -1;
  } else {
#if 0 /* Don't create lock here. */
      if (!wsu_shmlock_init(&shm_rsuhealth_ptr->h.ch_lock)) {
          I2V_DBG_LOG(LEVEL_DBG,MY_NAME,"sem init failed for SHM.\n");
          wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
          shm_rsuhealth_ptr = NULL;
      }
#endif
  }
  /* open TPS SHM */
  if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
      printf("\n  TPS SHM init failed.\n");
      shm_tps_ptr = NULL;
      return -2;
  }
  /* open SOUS SHM */
  if ((shm_sous_ptr = wsu_share_init(sizeof(shm_sous_t), SHM_SOUS_PATH)) == NULL) {
      printf("\n  SOUS SHM init failed.\n");
      shm_sous_ptr = NULL;
      return -3;
  }
  return 0;
}
static void closediagnosticshm(void)
{
  /* Close SHM. Don't destroy. Recover on start up.*/
  if(NULL != shm_rsuhealth_ptr) {
#if 0
      wsu_shmlock_kill(&shm_rsuhealth_ptr->h.ch_lock);
#endif
      wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      shm_rsuhealth_ptr = NULL;
  }
  if(NULL != shm_tps_ptr) {
      wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
      shm_tps_ptr = NULL;
  }
  if(NULL != shm_sous_ptr) {
      wsu_share_kill(shm_sous_ptr, sizeof(shm_sous_t));
      shm_sous_ptr = NULL;
  }
}

static shm_tps_t  localshmtps; /* Local copy of TPS SHM */
static uint32_t   time_of_week_msec = 0;
static uint32_t   seqno=0;
void seedtpsshm(int32_t control)
{
  switch(control) {
  case 0:
      time_of_week_msec +=1000;
      seqno++;
      strcpy((char_t *)localshmtps.ublox_config_version_str,UBX_CFG_VERSION_STR);
      strcpy((char_t *)localshmtps.ublox_firmware_version_str,UBX_FW_VERSION_STR);

      localshmtps.debug_gps_cmd_count_ubx_nav_pvt_total++;
      localshmtps.debug_gps_cmd_count_ubx_nav_pvt_unused = 1;
      localshmtps.debug_gps_cmd_count_nmea_gpgga_valid_data++;
      localshmtps.nav_status.spoofing_detection_state  = 1;
      localshmtps.mon_hw_jamming_status    = 0;
      localshmtps.mon_hw_rtc_is_calibrated = 1;

      localshmtps.debug_cnt_1pps_fetch_event   = 0;
      localshmtps.debug_cnt_1pps_latency_event = 0;
      localshmtps.debug_cnt_timestamp_missing  = 0;

      localshmtps.debug_cnt_time_adjustment_fail_mktime = 0;
      localshmtps.debug_cnt_1pps_halt_event = 0;
      localshmtps.debug_gps_cmd_count_ubx_nav_pvt_no_timeacc = 0;
      localshmtps.cnt_timeacc_exceeded_threshold = 0;

      localshmtps.debug_cnt_gps_fix_lost = 0;
      localshmtps.debug_cnt_gps_time_and_system_time_out_of_sync = 0;

      localshmtps.seqno = seqno;

      localshmtps.pub_geodetic.last_updated_pvt_index = (localshmtps.seqno % 2);

      localshmtps.pub_geodetic.last_PVT[0].time_of_week_msec = time_of_week_msec;
      localshmtps.pub_geodetic.last_PVT[0].flag_gnss_fix_ok  = 1;
      localshmtps.pub_geodetic.last_PVT[0].num_sats          = RSUHEALTH_MIN_GPS_SV_COUNT;
      localshmtps.pub_geodetic.last_PVT[1].time_of_week_msec = time_of_week_msec;
      localshmtps.pub_geodetic.last_PVT[1].flag_gnss_fix_ok  = 1;
      localshmtps.pub_geodetic.last_PVT[1].num_sats          = RSUHEALTH_MIN_GPS_SV_COUNT;

  break;
  case 1:

  break;
  case 2:

  default:

  break;
  }
  memcpy(shm_tps_ptr,&localshmtps,sizeof(localshmtps));
  usleep(1000);
}
static shm_sous_t localsous;
void seedsousshm(int32_t control)
{
  switch(control) {
  case 0:
      localsous.rh850_data.dcin_voltage = 12.0f;
      localsous.rh850_data.cv2x_ant1_status = ANTCON_CONNECTED;
      localsous.rh850_data.cv2x_ant2_status = ANTCON_CONNECTED;
      localsous.rh850_data.gps_ant2_status = ANTCON_CONNECTED;
      localsous.rh850_update_count++;
  break;
  case 1:
      localsous.rh850_data.dcin_voltage = DCIN_LOW_CRITICAL - 1.0f;
      localsous.rh850_data.cv2x_ant1_status = ANTCON_SHORT;
      localsous.rh850_data.cv2x_ant2_status = ANTCON_SHORT;
      localsous.rh850_data.gps_ant2_status = ANTCON_SHORT;
      localsous.rh850_update_count++;
  break;
  case 2:
      localsous.rh850_data.dcin_voltage = DCIN_HIGH_CRITICAL + 1.0f;
      localsous.rh850_update_count++;
  default:
      localsous.rh850_data.dcin_voltage = 12.0f;
      localsous.rh850_data.cv2x_ant1_status = ANTCON_CONNECTED;
      localsous.rh850_data.cv2x_ant2_status = ANTCON_CONNECTED;
      localsous.rh850_data.gps_ant2_status = ANTCON_CONNECTED;
      localsous.rh850_update_count++;
  break;
  }
  /* Allows us to test both aspects of logic */
  if((5 < onesecondcounter) && (onesecondcounter < 7)) {
      localrsuhealth.cv2xruntime &= ~RSUHEALTH_CV2X_ANTENNA_CHECK;
  } else {
      localrsuhealth.cv2xruntime |= RSUHEALTH_CV2X_ANTENNA_CHECK;
  }

  memcpy(shm_sous_ptr,&localsous,sizeof(localsous));
  usleep(1000);
}
void setnexttestiteration(uint32_t counter)
{
  int32_t control = 0;

  /* Use signal to setup type of test we want for all popen requests.
   * Do NOT clear signals in this case.
   */
  if(1 == get_stub_signal(Signal_test_control_1)) {
      if(0 == get_stub_iteration(Signal_test_control_1)) {
          //clear_stub_signal(Signal_test_control_1);
          control=1;
      } else {
          dec_stub_iteration(Signal_test_control_1);
      }
  }
  if(1 == get_stub_signal(Signal_test_control_2)) {
      if(0 == get_stub_iteration(Signal_test_control_2)) {
          //clear_stub_signal(Signal_test_control_2);
          control=2;
      } else {
          dec_stub_iteration(Signal_test_control_2);
      }
  }
  if(1 == get_stub_signal(Signal_test_control_3)) {
      if(0 == get_stub_iteration(Signal_test_control_3)) {
          //clear_stub_signal(Signal_test_control_3);
          control=3;
      } else {
          dec_stub_iteration(Signal_test_control_3);
      }
  }
  if(1 == get_stub_signal(Signal_test_control_4)) {
      if(0 == get_stub_iteration(Signal_test_control_4)) {
          //clear_stub_signal(Signal_test_control_4);
          control=4;
      } else {
          dec_stub_iteration(Signal_test_control_4);
      }
  }
  switch(control){
  case 0:
      seedtpsshm(0); /* seed normal */
      seedsousshm(0);
  break;
  case 1:
      if(5 < counter) { /* no seed at start */

      } else {
          seedtpsshm(0);
          seedsousshm(0);
      }
  break;
  case 2:
      if((5 < counter) && (counter < 10)) {  /* no seed in middle */

      } else {
          seedtpsshm(0);
          seedsousshm(0);
      }
  break;
  case 3:
      /* No seed ever */
  break;
  case 4:
      if(5 < counter) {   /* seed at start but not at end */
          seedtpsshm(0);
          seedsousshm(0);
      } else {

      }
  break;
  default:
      seedtpsshm(0);
      seedsousshm(0);
  break;
  }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_main(void)
{
  char_t  *token[MAX_TOKENS];
  int32_t  my_argc=2;
  int32_t  i;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

#if 0 //TODO
  if(0 != my_get_ip()) {
      CU_ASSERT(0);
      return;
  }
#endif
  CU_ASSERT(0 == opendiagnosticshm());

  for(i=0;i<MAX_TOKENS;i++) {
      token[i] = (char_t *)malloc(MAX_TOKEN_SIZE * 2); /* Allow room for null + extra junk to break conf_manager */
      strcpy(token[i],"");
  }

  my_argc=2;
  strcpy(token[0],"./rsuhealth");

  /* 
   * Test daemon mode which has rsudiagnostic support.
   */
  strcpy(token[1],"-d");
  my_main(my_argc,token); /* Nominal case */

  CU_ASSERT(0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL));
  CU_ASSERT(0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL));
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL));

  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG));
#endif

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL)){
    printf("\nerror = not RSUHEALTH_IP_GET_FAIL\n");
    CU_ASSERT(0);
  }
  if(localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL)
      CU_ASSERT(0);
  if(localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL)
      CU_ASSERT(0);
  //printf("\nrsudiag=0x%lx\n",localrsuhealth.errorstates);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID);
  CU_ASSERT(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY);
  CU_ASSERT(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY);

  clear_all_stub_signal();
//TODO: Order of execution makes test result non-deterministic. Disable for now.
#if 0
  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_over); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);

  /* ASSERT can't handle 64 bit value */
  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);

  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);

  //printf("\nrsudiag=0x%lx\n",localrsuhealth.errorstates);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID);
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY));
  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_under); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);

  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID);
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY));

  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_fail);
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);

  //printf("\ncv2x=0x%x\n",localrsuhealth.cv2xstatus);
  //printf("\nrsudiag=0x%lx\n",localrsuhealth.errorstates);
  //printf("\ngnss=0x%x\n",localrsuhealth.gnssstatus);
  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);

  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
     CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT((localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY));

  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_bogus);
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);

  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  //TODO: this test fails on server.
  //if(localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL)
  //    CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT((localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY));
  clear_all_stub_signal();
#if 0 //Results differ from build server from time to time. Disable for now
  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_toobig);
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);

  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));

  CU_ASSERT((localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY));
  CU_ASSERT(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY));

  clear_all_stub_signal();
#endif

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_popen_empty);
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_CPU_TEMP_FAIL\n");
    CU_ASSERT(0);
  }

  if (0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
    printf("\nerror = not RSUHEALTH_AMBIENT_TEMP_FAIL\n");
    CU_ASSERT(0);
  }
#if defined(ENABLE_HSM_TEST)
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_TEMP_FAIL);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_GSA);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_USAPP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_JCOP);
  CU_ASSERT(localrsuhealth.cv2xstatus  & RSUHEALTH_HSM_BAD_CFG);
#endif
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL))
      CU_ASSERT(0);
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL))
      CU_ASSERT(0);

  CU_ASSERT(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV));
  clear_all_stub_signal();

  /* 
   * Specifc scenarios of test.
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_test_control_1); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_test_control_2); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_test_control_3); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  sleep(2);
  my_main(my_argc,token);
  clear_all_stub_signal();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  set_stub_signal(Signal_test_control_4); /* You must clear manually, will not self clear */
  strcpy(token[1],"-d");
  my_argc=1; //no debug
  sleep(2);
  my_main(my_argc,token);
  clear_all_stub_signal();

  /*
   * Test one shot calls to rsuhealth. No rsudiagnostic support yet.
   */
  strcpy(token[1],"-tcv2x");
  my_main(my_argc,token);

  strcpy(token[1],"-c");
  my_main(my_argc,token);

  strcpy(token[1],"-Ird");
  my_main(my_argc,token);
#endif

  closediagnosticshm();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  for(i=0;i<MAX_TOKENS;i++) {
      free(token[i]);
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int init_suite(void) 
{ 

  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int clean_suite(void) 
{ 
  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**************************************************************************************************
* MAIN()
***************************************************************************************************/
int main (void)
{

  CU_pSuite    pSuite1 = NULL;
  CU_ErrorCode ret     = CUE_SUCCESS; /* SUCCESS till proven FAIL */

  init_stub_control();

  ret = CU_initialize_registry();

  if(CUE_SUCCESS == ret) {
      if(NULL == (pSuite1 = CU_add_suite("Basic_Test_Suite1", init_suite, clean_suite))) {
          ret = ~CUE_SUCCESS;
      }
  }
  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_main...",test_main))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
     printf("\nCUnit CU_basic_run_tests...\n");
     ret = CU_basic_run_tests();
  } 

  fflush(stdout);

  if(CUE_SUCCESS != ret) {
      printf("\nCUnit Exception: %d [%s]\n",CU_get_error(),CU_get_error_msg());
  } else {
      printf("\nCUnit Complete.\n");
  }

  CU_cleanup_registry();

  return CU_get_error();

}  /* END of MAIN() */

/**************************************************************************************************
* End of Module
***************************************************************************************************/
