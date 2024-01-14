/* This is single thread only! */
/* Do not include macro expansion. Include what macro calls */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <mqueue.h>
#include <CUnit/CUnit.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/msg.h>
#if defined(UNIT_NET_SNMP)
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#endif

#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "ris_struct.h"
#include "ris.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#endif
#if defined(J2735_2023)
#include "Common.h"
#include "AddGrpB.h"
#include "MessageFrame.h"
#include "SPAT.h"
#include "SignalRequestMessage.h"
#include "BasicSafetyMessage.h"
#include "MapData.h"
#include "TravelerInformation.h"
#include "PersonalSafetyMessage.h" /* 1 or 2? */
#endif
#include "i2v_riscapture.h"
#include "rs.h"
#include "stubs.h"
#include "i2v_snmp_client.h"
#include "conf_table.h"

#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "stubs"
/* Private: Don't expose */
#define RT_MH_FREEHEAPDESC (0x10000000);
#define OSCTXTINIT          0x1aa2a34a

typedef struct OSMemLink {
   struct OSMemLink* pnext;
   struct OSMemLink* pprev;
} OSMemLink;

typedef struct OSMemHeap {
   OSMemLink       rawBlkList;
   OSMemLink*      curMemBlk;
   OSUINT32        keepFreeUnits;
   OSUINT32        defBlkSize;   /* default block size in bytes */
   OSUINT32        refCnt;
   OSUINT32        flags;
#if 0
   OSMallocFunc  mallocFunc;     /* Memory allocation function pointer   */
   OSReallocFunc reallocFunc;    /* Memory reallocation function pointer */
   OSFreeFunc    memFreeFunc;    /* Memory free function pointer         */
#endif
} OSMemHeap;

static Stub_Control_Block_Type Stub_Control;

static wuint8        g_numberRadios    = 1; 
static MsgCount      my_sequenceNumber = 0x0;
i2vShmMasterT        shm_ptr_unit_test;
static OSRTDListNode my_list;
static OSMemHeap     my_pMemHeap;
static OSCTXT        my_pctxt;

void null_print(const char_t *format, ...) 
{
  if(NULL != format) {

  }
  return;
}
/****************************************************************************************************
 * CONTROL STUB FUNCTIONS
 ****************************************************************************************************/

void set_stub_signal(wint32 signal)
{
  if(signal < MAX_SIGNALS)
      Stub_Control.Signal[signal]    = 1;
      Stub_Control.Iteration[signal] = 0;
}

void set_stub_signal_iteration(wint32 signal,wint32 iteration)
{
  if(signal < MAX_SIGNALS) {
      Stub_Control.Signal[signal]    = 1;
      Stub_Control.Iteration[signal] = iteration;
  }
}

void clear_stub_signal(wint32 signal)
{
  if(signal < MAX_SIGNALS) {
      Stub_Control.Signal[signal]    = 0;
      Stub_Control.Iteration[signal] = 0;
  }
}

void clear_all_stub_signal(void)
{
  wuint32 i = 0;

  for(i=0;i<MAX_SIGNALS;i++) {
      Stub_Control.Signal[i]    = 0;
      Stub_Control.Iteration[i] = 0;
  }
}

wint32 get_stub_signal(wint32 signal)
{
  if(signal < MAX_SIGNALS) {
      return Stub_Control.Signal[signal];
  } else {
      return -1; //fail
  }
}

wint32 get_stub_iteration(wint32 signal)
{
  if(signal < MAX_SIGNALS) {
    return Stub_Control.Iteration[signal];    
  } else {
    return -1; //fail
  }
}

wint32 dec_stub_iteration(wint32 signal) 
{
  if((signal < MAX_SIGNALS) && (0 < Stub_Control.Iteration[signal])){
    Stub_Control.Iteration[signal]--;
    return 0;  
  } else {
    return -1; //fail
  }
}

wint32 check_stub_pending_signals(void)
{
  wint32 i = 0;

  for(i=0;(i<MAX_SIGNALS) && (0 == Stub_Control.Signal[i]) ; i++) 
  {};

  if(i<MAX_SIGNALS){
      
      //printf("\ncheck_stub_pending_signals:pending signal = %d.\n",i);
      return i+1;
  }
  else
      return 0;
}

//Not all these may relate to your test.
void init_stub_control(void)
{

  g_numberRadios    = 1; 
  my_sequenceNumber = 0x0;

  //memset(&shm_ptr_unit_test,0x0,sizeof(shm_ptr_unit_test));
  memset(&my_list,0x0,sizeof(my_list));
  memset(&my_pMemHeap,0x0,sizeof(my_pMemHeap));
  memset(&my_pctxt,0x0,sizeof(my_pctxt));
  memset(&Stub_Control,0x0,sizeof(Stub_Control));
}

/****************************************************************************************************
 * STUBS
 ****************************************************************************************************/
#if defined(UNIT_NET_SNMP)
/*
 * NET-SNMP
 */
static netsnmp_handler_registration my_netsnmp_handler_registration;
netsnmp_handler_registration * netsnmp_create_handler_registration(const char_t * name
           , Netsnmp_Node_Handler * handler_access_method
           , const oid *   reg_oid
           , size_t        reg_oid_len
           , int           modes) 
{
  name = name;
  handler_access_method = handler_access_method;
  reg_oid = reg_oid;
  reg_oid_len = reg_oid_len;
  modes = modes;

  return &my_netsnmp_handler_registration;

}


int  snmp_log( int priority, const char_t *format, ...)
{
  if((NULL != format) && (priority != 99))
      return 1;
  else
      return 0;
}
void debugmsgtoken(const char_t *token, const char_t *format,...)
{
    if((NULL != token) && (NULL != format)) {

    }    
    return;
}
void debugmsg(const char_t *token, const char_t *format, ...)
{
    if((NULL != token) && (NULL != format)) {

    }    
    return;
}

int snmp_get_do_debugging (void)
{
  return 0;
}

int snmp_set_var_typed_integer(netsnmp_variable_list * var, u_char type, long val)
{
  if(NULL != var) {
   if(0x0 == type) {
     if(0 == val) {
       return 1;
     } 
   }
  }

  return 0;
}

int  netsnmp_set_request_error(netsnmp_agent_request_info*reqinfo,netsnmp_request_info
                               *request, int error_value)
{
  reqinfo     = reqinfo;
  request     = request;
  error_value = error_value;
  return 0;
}

int snmp_set_var_typed_value(netsnmp_variable_list * var,
                             u_char type,
                             const void * value, size_t len)
{
  var = var;
  type = type;
  value = value;
  len = len;
  return 0;
}

int netsnmp_register_scalar(netsnmp_handler_registration *reginfo)
{
  int ret = -1;
  if (NULL != reginfo) {
    ret = 0;
  }
  if(1 == get_stub_signal(Signal_netsnmp_register_scalar)) {
      ret = -1;
      //clear_stub_signal(Signal_netsnmp_register_scalar);
  }

  return ret;
}
static netsnmp_table_request_info my_netsnmp_table_request_info;
netsnmp_table_request_info * netsnmp_extract_table_info(netsnmp_request_info * foo)
{
  if(NULL == foo) {
     return NULL;
  }
  memset(&my_netsnmp_table_request_info,0x0,sizeof(my_netsnmp_table_request_info));
  return &my_netsnmp_table_request_info;
}
static netsnmp_tdata_row my_tdata_row;
static int one_timerow_first = 0;
netsnmp_tdata_row *netsnmp_tdata_row_first(netsnmp_tdata * table)
{
  table = table;
  if(one_timerow_first < 2) {
      one_timerow_first++;
      return &my_tdata_row;
  } else {
      one_timerow_first = 0;
      return NULL;
  }
}
static netsnmp_tdata my_tdata;
netsnmp_tdata     *netsnmp_tdata_create_table(const char_t *name, long flags)
{
  /* silence warning */
  if(NULL == name) 
      flags +=1;
 
  return &my_tdata;
}


void netsnmp_table_helper_add_indexes(netsnmp_table_registration_info *tinfo, ...)
{
  tinfo = tinfo;
  return;
}


int netsnmp_tdata_register(  netsnmp_handler_registration    *reginfo,
                                 netsnmp_tdata                   *table,
                                 netsnmp_table_registration_info *table_info)
{

   if(   (NULL == reginfo)
      || (NULL == table)
      || (NULL == table_info)) {
     return -1;
   } 
   return 0;

}
static netsnmp_tdata_row  my_row;

static int one_time_next_row = 0;

netsnmp_tdata_row *netsnmp_tdata_row_next( netsnmp_tdata     *table,
                                           netsnmp_tdata_row *row)
{
  if((NULL == table) 
    || (NULL == row)) {
      return NULL;
  }
  if( one_time_next_row < 25) {
      one_time_next_row++;
      return &my_row;
  } else {
      return NULL;
  }

}

void * netsnmp_tdata_extract_entry(netsnmp_request_info *foo)
{
  foo = foo;
  return NULL;
}
static int one_time_netsnmp_tdata_add_row = 0;
int  netsnmp_tdata_add_row(netsnmp_tdata     *table,
                           netsnmp_tdata_row *row)
{

  if((NULL == row) || (NULL == table)) {
      return -1;
  }

  if(0 == one_time_netsnmp_tdata_add_row) {
    one_time_netsnmp_tdata_add_row = 1;
    return -1;
  } else {
     one_time_netsnmp_tdata_add_row = 0;
     return 0;
  }

}

void   *netsnmp_tdata_remove_and_delete_row(netsnmp_tdata    *table,
                                           netsnmp_tdata_row *row)
{
  table = table;
  row = row;
  return NULL;
}
netsnmp_tdata_row * netsnmp_tdata_create_row(void){

#if 0
  netsnmp_tdata_row * foo;
  
    foo = SNMP_MALLOC_TYPEDEF(netsnmp_tdata_row);
    return foo;
#else
    return NULL;
#endif 
}

void *netsnmp_tdata_delete_row(netsnmp_tdata_row *row)
{
  row = row;
  return NULL;
}

static netsnmp_variable_list my_varlist;

netsnmp_variable_list * snmp_varlist_add_variable(netsnmp_variable_list ** varlist,
                                 const oid * name, size_t name_length,
                                 u_char type,
                                 const void * value, size_t len) 
{
  varlist = varlist;
  name = name;
  name_length = name_length;
  type = type;
  value = value;
  len = len;
  return &my_varlist;
}


#if 0 //for easy reference


#define SNMP_ERR_NOERROR                (0)     /* XXX  Used only for PDUs? */
#define SNMP_ERR_TOOBIG	                (1)
#define SNMP_ERR_NOSUCHNAME             (2)
#define SNMP_ERR_BADVALUE               (3)
#define SNMP_ERR_READONLY               (4)
#define SNMP_ERR_GENERR	                (5)

    /*
     * in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs 
     */
#define SNMP_ERR_NOACCESS		(6)
#define SNMP_ERR_WRONGTYPE		(7)
#define SNMP_ERR_WRONGLENGTH		(8)
#define SNMP_ERR_WRONGENCODING		(9)
#define SNMP_ERR_WRONGVALUE		(10)
#define SNMP_ERR_NOCREATION		(11)
#define SNMP_ERR_INCONSISTENTVALUE	(12)
#define SNMP_ERR_RESOURCEUNAVAILABLE	(13)
#define SNMP_ERR_COMMITFAILED		(14)
#define SNMP_ERR_UNDOFAILED		(15)
#define SNMP_ERR_AUTHORIZATIONERROR	(16)
#define SNMP_ERR_NOTWRITABLE		(17)


typedef struct snmp_pdu {

#define non_repeaters	errstat
#define max_repetitions errindex

    /*
     * Protocol-version independent fields
     */
    /** snmp version */
    long            version;
    /** Type of this PDU */	
    int             command;
    /** Request id - note: not incremented on retries */
    long            reqid;  
    /** Message id for V3 messages note: incremented for each retry */
    long            msgid;
    /** Unique ID for incoming transactions */
    long            transid;
    /** Session id for AgentX messages */
    long            sessid;
    /** Error status (non_repeaters in GetBulk) */
    long            errstat;
    /** Error index (max_repetitions in GetBulk) */
    long            errindex;       
    /** Uptime */
    u_long          time;   
    u_long          flags;

    int             securityModel;
    /** noAuthNoPriv, authNoPriv, authPriv */
    int             securityLevel;  
    int             msgParseModel;

    /**
     * Transport-specific opaque data.  This replaces the IP-centric address
     * field.  
     */
    
    void           *transport_data;
    int             transport_data_length;

    /**
     * The actual transport domain.  This SHOULD NOT BE FREE()D.  
     */

    const oid      *tDomain;
    size_t          tDomainLen;

    netsnmp_variable_list *variables;


    /*
     * SNMPv1 & SNMPv2c fields
     */
    /** community for outgoing requests. */
    u_char         *community;
    /** length of community name. */
    size_t          community_len;  

    /*
     * Trap information
     */
    /** System OID */
    oid            *enterprise;     
    size_t          enterprise_length;
    /** trap type */
    long            trap_type;
    /** specific type */
    long            specific_type;
    /** This is ONLY used for v1 TRAPs  */
    unsigned char_t   agent_addr[4];  

    /*
     *  SNMPv3 fields
     */
    /** context snmpEngineID */
    u_char         *contextEngineID;
    /** Length of contextEngineID */
    size_t          contextEngineIDLen;     
    /** authoritative contextName */
    char_t           *contextName;
    /** Length of contextName */
    size_t          contextNameLen;
    /** authoritative snmpEngineID for security */
    u_char         *securityEngineID;
    /** Length of securityEngineID */
    size_t          securityEngineIDLen;    
    /** on behalf of this principal */
    char_t           *securityName;
    /** Length of securityName. */
    size_t          securityNameLen;        
    
    /*
     * AgentX fields
     *      (also uses SNMPv1 community field)
     */
    int             priority;
    int             range_subid;
    
    void           *securityStateRef;
} netsnmp_pdu;


pedef struct variable_list {
   /** NULL for last variable */
   struct variable_list *next_variable;    
   /** Object identifier of variable */
   oid            *name;   
   /** number of subid's in name */
   size_t          name_length;    
   /** ASN type of variable */
   u_char          type;   
   /** value of variable */
    netsnmp_vardata val;
   /** the length of the value to be copied into buf */
   size_t          val_len;
   /** buffer to hold the OID */
   oid             name_loc[MAX_OID_LEN];  
   /** 90 percentile < 40. */
   u_char          buf[40];
   /** (Opaque) hook for additional data */
   void           *data;
   /** callback to free above */
   void            (*dataFreeHook)(void *);    
   int             index;
} netsnmp_variable_list;


#endif

#define MAX_OID_LENGTH (128)
static oid  my_oid[MAX_OID_LENGTH];

netsnmp_variable_list * snmp_add_null_var(netsnmp_pdu * pdu,
                                          const oid   * name, 
                                          size_t        name_length)
{
  int i=0;

  if (  (NULL == pdu)
      ||(NULL == name)
      ||(0 == name_length)) {
    return NULL;
  } else {

    if(1 == get_stub_signal(Signal_snmp_add_null_var)) {
      clear_stub_signal(Signal_snmp_add_null_var);
      return NULL;
    }
    
    my_varlist.name        = my_oid;
    my_varlist.name_length = name_length;

    pdu->variables         = &my_varlist;

    for(i=0;(i<MAX_OID_LENGTH) && name_length;i++) {
      my_oid[i] = name[i];
    }

    return &my_varlist;
  }

}

int snmp_parse_args( int argc 
                , char_t **argv
                , netsnmp_session *session
                , const char_t *localOpts
                , void (*proc)(int, char_t *const *, int))
{
  int ret = NETSNMP_PARSE_ARGS_SUCCESS; //NETSNMP_PARSE_ARGS_ERROR_USAGE; 

    argc = argc;
    argv = argv;
    session = session;
    localOpts = localOpts;
    proc = proc;
    if(1 == get_stub_signal(Signal_snmp_parse_args)) {
      if(0 == get_stub_iteration(Signal_snmp_parse_args)) {
        clear_stub_signal(Signal_snmp_parse_args);
        ret = NETSNMP_PARSE_ARGS_ERROR;
      } else {
        dec_stub_iteration(Signal_snmp_parse_args);
      }
    }

    if(1 == get_stub_signal(Signal_snmp_parse_args_aok)) {
      if(0 == get_stub_iteration(Signal_snmp_parse_args_aok)) {
        clear_stub_signal(Signal_snmp_parse_args_aok);
        ret = NETSNMP_PARSE_ARGS_SUCCESS;
      } else {
        dec_stub_iteration(Signal_snmp_parse_args_aok);
      }
    }

    return ret;

}

static   netsnmp_session ss;
netsnmp_session * snmp_open ( netsnmp_session *ss_in)
{
    if(NULL == ss_in) {  
      return NULL;
    } else {

      if(1 == get_stub_signal(Signal_snmp_open)) {
        clear_stub_signal(Signal_snmp_open);
        return NULL;
      }
    
      return &ss; 
    }
} 
static   netsnmp_pdu my_pdu;
netsnmp_pdu * snmp_pdu_create(int type)
{

  if(type < 0) {

    return NULL;
  } else {

    if(1 == get_stub_signal(Signal_snmp_pdu_create)) {
      clear_stub_signal(Signal_snmp_pdu_create);
      return NULL;
    }
    return &my_pdu;
  }

}


oid  * snmp_parse_oid(const char_t * input, oid *objid, size_t *objidlen)
{
  int32_t i;
  
  if(   (NULL == input)
     && (NULL == objid)
     && (NULL == objidlen) ) {
      return NULL;
  }

  if(1 == get_stub_signal(Signal_snmp_parse_oid)) {
    if(0 == get_stub_iteration(Signal_snmp_parse_oid)) {
      clear_stub_signal(Signal_snmp_parse_oid);
      return NULL;
    } else {
      dec_stub_iteration(Signal_snmp_parse_oid);
    }
  }

  for(i=0; (i < MAX_OID_LENGTH) && (i < (int32_t)(*objidlen)); i++) {
    *(objid+i) = *(input+i);

    //printf("(0x%x,0x%x)",*(objid+i),*(input+i));
  }
  
  return objid;
}

void snmp_perror(const char_t * msg )
{
  msg = msg;
}

static netsnmp_pdu   my_pdu;
static netsnmp_pdu * my_pointer_pdu;
/* netsnmp_vardata is 64 bit opaque. */
static  int64_t my_int = 0;

//I could read these from config but should i? Maybe being fixed and specific is good?

//TM not sure about names?

//NTCIP_PHASE_OPTIONS                =  1.3.6.1.4.1.1206.4.2.1.1.2.1.21   # among other things provides phase enable
//NTCIP_PHASE_GREENS                 =  1.3.6.1.4.1.1206.4.2.1.1.4.1.4
//NTCIP_PHASE_YELLOWS                =  1.3.6.1.4.1.1206.4.2.1.1.4.1.3
//#NTCIP_PHASE_REDS                  =  1.3.6.1.4.1.1206.4.2.1.1.4.1.2
static char_t OID_YELLOW_PHASE[MAX_OID_LENGTH]   = "1.3.6.1.4.1.1206.4.2.1.1.4.1.4.2";
static char_t OID_GREEN_PHASE[MAX_OID_LENGTH]    = "1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1";
static char_t OID_PHASE_ENABLED[MAX_OID_LENGTH]  = "1.3.6.1.4.1.1206.4.2.1.1.4.1.3";
static char_t OID_PHASE_NUMBER[MAX_OID_LENGTH]   = "1.3.6.1.4.1.1206.4.2.1.1.2.1.21";

//NTCIP_YELLOW_DURATION              =  1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1
static char_t OID_YELLOW_DURATION[MAX_OID_LENGTH]  = "1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1";

//NTCIP_PHASE_NUMBER                      = 1.3.6.1.4.1.1206.3.47.1.1.1
//NTCIP_PHASE_MAX_COUNTDOWN               = 1.3.6.1.4.1.1206.3.47.1.1.3
//NTCIP_PHASE_MIN_COUNTDOWN               = 1.3.6.1.4.1.1206.3.47.1.1.2
static char_t OID_PHASE_MIN_CNTDOWN[MAX_OID_LENGTH]  = "1.3.6.1.4.1.1206.3.47.1.1.1";
static char_t OID_PHASE_TIME_REMAIN[MAX_OID_LENGTH]  = "1.3.6.1.4.1.1206.3.47.1.1.2";
static char_t OID_PHASE_MAX_CNTDOWN[MAX_OID_LENGTH]  = "1.3.6.1.4.1.1206.3.47.1.1.3";


//SNMPEnableSpatCommand         = 1.3.6.1.4.1.1206.3.5.2.9.44.1.1

//PEAGLE_GLE_SNMP                         = 1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1  # GLE snmp command
//PEAGLE_RLPE_SNMP                        = 1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1  # RLPE snmp command
//PEAGLE_PRIORITY_PREEMPT                 = 1.3.6.1.4.1.1206.4.2.1.6.3.1.2.5  # priority preempt
//PEAGLE_PRIORITY_CLEAR_PREEMPT           = 1.3.6.1.4.1.1206.4.2.1.6.3.1.2.5  # priority preempt

//TM based on OID and test signal we can return any desired data
int snmp_synch_response(netsnmp_session * session , netsnmp_pdu * pdu, netsnmp_pdu  ** response)
{
  int32_t  ret = SNMP_ERR_NOACCESS; //FAIL till proven SUCCESS
  int32_t  i   = 0;

  if (   (NULL == session)
      || (NULL == pdu)
      || (NULL == response)) {
    ret = SNMP_ERR_BADVALUE;
  } else {

    *response = NULL;

    if(1 == get_stub_signal(Signal_snmp_synch_response)) {
      if(0 == get_stub_iteration(Signal_snmp_synch_response)) {
        clear_stub_signal(Signal_snmp_synch_response);
        return SNMP_ERR_COMMITFAILED;
      } else {
        dec_stub_iteration(Signal_snmp_synch_response);
      }
    }

//OID: my_pdu.name_length = name_length = 128;
//   : Dont count on being NULL terminated

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_GREEN_PHASE));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_GREEN_PHASE[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_GREEN_PHASE)) {

      my_int                 = 0x6;
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;
    } 

    for(i=0;(i<MAX_OID_LENGTH) && (i < (int32_t)strlen(OID_YELLOW_PHASE));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_YELLOW_PHASE[i])) {
        break;
      }
    }
    if(   (i == (int32_t)strlen(OID_YELLOW_PHASE)) 
     //  && (strlen(OID_YELLOW_PHASE) == strlen(my_pdu.variables->name) ) 
      ) {

      my_int                 = 0x6;
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;


      if(1 == get_stub_signal(Signal_oid_yellow_phase)) {
        if(0 == get_stub_iteration(Signal_oid_yellow_phase)) {
          clear_stub_signal(Signal_oid_yellow_phase);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_yellow_phase);
        }
      }


    } 

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_PHASE_ENABLED));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_PHASE_ENABLED[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_PHASE_ENABLED)) {

      my_int                 = 0x1; //active
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_phase_enable)) {
        if(0 == get_stub_iteration(Signal_oid_phase_enable)) {
          clear_stub_signal(Signal_oid_phase_enable);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_phase_enable);
        }
      }


    }

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_PHASE_NUMBER));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_PHASE_NUMBER[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_PHASE_NUMBER)) {

      my_int                 = 0x1; //approach number = phase ?
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_phase_number)) {
        if(0 == get_stub_iteration(Signal_oid_phase_number)) {
          clear_stub_signal(Signal_oid_phase_number);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_phase_number);
        }
      }

      if(1 == get_stub_signal(Signal_oid_phase_number_bad_type)) {
        if(0 == get_stub_iteration(Signal_oid_phase_number_bad_type)) {
          clear_stub_signal(Signal_oid_phase_number_bad_type);
          my_varlist.type        = I2V_ASN_OCTET_STR;
        } else {
          dec_stub_iteration(Signal_oid_phase_number_bad_type);
        }
      }

      if(1 == get_stub_signal(Signal_oid_phase_number_not_active)) {
        if(0 == get_stub_iteration(Signal_oid_phase_number_not_active)) {
          clear_stub_signal(Signal_oid_phase_number_not_active);
          my_int = 0x0; //not active
        } else {
          dec_stub_iteration(Signal_oid_phase_number_not_active);
        }
      }
      
    }

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_PHASE_MIN_CNTDOWN));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_PHASE_MIN_CNTDOWN[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_PHASE_MIN_CNTDOWN)) {

      my_int                 = 0x1; //min count
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_phase_min)) {
        if(0 == get_stub_iteration(Signal_oid_phase_min)) {
          clear_stub_signal(Signal_oid_phase_min);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_phase_min);
        }
      }

      if(1 == get_stub_signal(Signal_oid_phase_min_bad_type)) {
        if(0 == get_stub_iteration(Signal_oid_phase_min_bad_type)) {
          clear_stub_signal(Signal_oid_phase_min_bad_type);
          my_varlist.type = I2V_ASN_OCTET_STR;
        } else {
          dec_stub_iteration(Signal_oid_phase_min_bad_type);
        }
      }

    }

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_PHASE_TIME_REMAIN));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_PHASE_TIME_REMAIN[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_PHASE_TIME_REMAIN)) {

      my_int                 = 15; //tt next phase seconds?
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_phase_time_remain)) {
        if(0 == get_stub_iteration(Signal_oid_phase_time_remain)) {
          clear_stub_signal(Signal_oid_phase_time_remain);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_phase_time_remain);
        }
      }

      if(1 == get_stub_signal(Signal_oid_phase_time_remain_bad_type)) {
        if(0 == get_stub_iteration(Signal_oid_phase_time_remain_bad_type)) {
          clear_stub_signal(Signal_oid_phase_time_remain_bad_type);
          my_varlist.type = I2V_ASN_OCTET_STR;
        } else {
          dec_stub_iteration(Signal_oid_phase_time_remain_bad_type);
        }
      }
    }

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_PHASE_MAX_CNTDOWN));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_PHASE_MAX_CNTDOWN[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_PHASE_MAX_CNTDOWN)) {

      my_int                 = 0x1; //max count
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_phase_max_count)) {
        if(0 == get_stub_iteration(Signal_oid_phase_max_count)) {
          clear_stub_signal(Signal_oid_phase_max_count);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_phase_max_count);
        }
      }

      if(1 == get_stub_signal(Signal_oid_phase_max_count_bad_type)) {
        if(0 == get_stub_iteration(Signal_oid_phase_max_count_bad_type)) {
          clear_stub_signal(Signal_oid_phase_max_count_bad_type);
          my_varlist.type = I2V_ASN_OCTET_STR;
        } else {
          dec_stub_iteration(Signal_oid_phase_max_count_bad_type);
        }
      }


    }

    for(i=0;(i<MAX_OID_LENGTH) && (i<(int32_t)strlen(OID_YELLOW_DURATION));i++) {
      if(my_pdu.variables->name[i] != (oid)(OID_YELLOW_DURATION[i])) {
        break;
      }
    }
    if(i == (int32_t)strlen(OID_YELLOW_DURATION)) {

      my_int                 = 0x1; //max count
      my_varlist.type        = I2V_ASN_INTEGER;
      my_varlist.val.integer = (void *)&my_int;

      my_pdu.variables       = &my_varlist;

      my_pdu.errstat   = SNMP_ERR_NOERROR;

      my_pointer_pdu   = &my_pdu;

      *response        = my_pointer_pdu;

      ret = SNMP_ERR_NOERROR;

      if(1 == get_stub_signal(Signal_oid_yellow_duration)) {
        if(0 == get_stub_iteration(Signal_oid_yellow_duration)) {
          clear_stub_signal( Signal_oid_yellow_duration);
          ret = SNMP_ERR_RESOURCEUNAVAILABLE;
        } else {
          dec_stub_iteration(Signal_oid_yellow_duration);
        }
      }

      if(1 == get_stub_signal(Signal_oid_yellow_duration_bad_type)) {
        if(0 == get_stub_iteration(Signal_oid_yellow_duration_bad_type)) {
          clear_stub_signal(Signal_oid_yellow_duration_bad_type);
          my_varlist.type = I2V_ASN_OCTET_STR;
        } else {
          dec_stub_iteration(Signal_oid_yellow_duration_bad_type);
        }
      }
    }


  }
  return ret;
}

void snmp_free_pdu( netsnmp_pdu *pdu)
{
  pdu = pdu;
  return;
}

void snmp_sess_perror(const char_t *msg, netsnmp_session *sess)
{
  msg = msg;
  sess = sess;
  return;
}
#endif /* UNIT_NET_SNMP */
/*
 * ASN1
 */

void rtFreeContext (OSCTXT* pctxt)
{
  pctxt = pctxt;
  return;
}

void rtxFreeContext (OSCTXT* pctxt)
{
  pctxt = pctxt;
  return;
}

int rtInitContextUsingKey (OSCTXT* pctxt, const OSOCTET* key, size_t keylen)
{

  int ret = 0; //ASN_OK;
  
  key = key;
  keylen = keylen;
  if(NULL == pctxt) {
      ret = -1;
  }  else {
      memset(&my_pctxt,0x0,sizeof(my_pctxt));
  }

  if(1 == get_stub_signal(Signal_rtInitContextUsingKey)) {
      ret = -1;
      clear_stub_signal(Signal_rtInitContextUsingKey);
  }

  return ret;

}

int dn_open(const char_t *pathname, int flags)
{
    if(NULL == pathname){
        return -1;
    }
    flags = flags;
    if(1 == get_stub_signal(Signal_open)) {

      if(0 == get_stub_iteration(Signal_open)) {

        clear_stub_signal(Signal_open);
        return -1;

      } else {
        dec_stub_iteration(Signal_open);
      }
    }
    return 1;
}

ssize_t dn_read(int fd, void *buf, size_t count)
{
    if((fd < 1) || (NULL == buf) || (count < 1)) {
        return -1;
    }

    if(1 == get_stub_signal(Signal_read)) {

      if(0 == get_stub_iteration(Signal_read)) {

        clear_stub_signal(Signal_read);
        return -1;

      } else {
        dec_stub_iteration(Signal_read);
      }
    }
    *((char_t *)buf) = '\0';
    return 1;

}

static mqd_t my_queue;

mqd_t mq_open(const char_t *name, int oflag, ...)
{
    if(1 == get_stub_signal(Signal_mq_open)) {
        clear_stub_signal(Signal_mq_open);
        return -1;
    }

    if((NULL == name) || (oflag < 0 )) {
        return -1;
    } else {
        return my_queue;
    }
}

int mq_close (mqd_t mqdes) 
{
    if(1 == get_stub_signal(Signal_mq_close)) {
        clear_stub_signal(Signal_mq_close);
        return -1;
    }

    if(0 < mqdes) {
        return 0;
    } else {
        return -1;
    }
    
}

ssize_t mq_receive (mqd_t          mqdes, 
                    char_t         * msg_ptr,
                    size_t         msg_len,
                    unsigned int * msg_prio) 
{

    mqdes = mqdes;
    msg_prio = msg_prio;
    if(1 == get_stub_signal(Signal_mq_receive)) {
        clear_stub_signal(Signal_mq_receive);
        return -1;
    }

    if((NULL == msg_ptr) ||  (msg_len < 1)) {
        return -1;
    } else {
        //memcpy(msg_ptr, 0x0,msg_len);
        return 0;
    }

}

int mq_send (mqd_t        mqdes,
             const char_t * msg_ptr,
             size_t       msg_len,
             unsigned int msg_prio)
{
    mqdes = mqdes;
    msg_prio = msg_prio;
    if(1 == get_stub_signal(Signal_mq_send)) {
        clear_stub_signal(Signal_mq_send);
        return -1;
    }

    if((NULL == msg_ptr) || (msg_len < 1)) {
        return -1;
    } else {
        return 0;
    }
}
int mq_getattr(mqd_t mqdes, struct mq_attr *attr)
{
    mqdes = mqdes;
    if(1 == get_stub_signal(Signal_mq_getattr)) {
        clear_stub_signal(Signal_mq_getattr);
        return -1;
    }

    if(NULL == attr) {
        return -1;
    } else {
        return 0;
    }
}

int mq_unlink(const char_t *name)
{
    if(1 == get_stub_signal(Signal_mq_unlink)) {
        clear_stub_signal(Signal_mq_unlink);
        return -1;
    }

    if(NULL == name) {
        return -1;
    } else {
        return 0;
    }
}


tpsResultCodeType wsuTpsInit(void)
{   
  tpsResultCodeType ret = TPS_SUCCESS;
  
  if(1 == get_stub_signal(Signal_wsuTpsInit)) {
      clear_stub_signal(Signal_wsuTpsInit);
      ret = TPS_FAILURE;
  }

  return ret;

}

tpsResultCodeType wsuTpsRegister(void (*registerCB)(tpsDataType *))
{
  tpsResultCodeType ret = TPS_SUCCESS;

  registerCB = registerCB;
  if(1 == get_stub_signal(Signal_wsuTpsRegister)) {
      clear_stub_signal(Signal_wsuTpsRegister);
      ret = TPS_FAILURE;
  }

  return ret;
}

rsResultCodeType wsuRisPcapEnable(rsPcapEnableType *pcapEnable)
{
  int32_t          size = 0;
  rsResultCodeType risRet = RS_SUCCESS;

  if(NULL == pcapEnable) {
      risRet = RS_BADPARAM;
  } else {
      pcapEnable->cmd = WSU_NS_PCAP_ENABLE;
#if 0
      /* Send the message to Radio Service and wait for a reply */
      size = communicateWithRadioServices(pcapEnable, sizeof(*pcapEnable),
                                        &risRet, sizeof(risRet));
#endif
      if (size < 0) {
          risRet = RS_EIOCTL;
      }
  }
  return risRet;
}
rsResultCodeType wsuRisPcapDisable(rsPcapDisableType *pcapDisable)
{
  int32_t          size = 0;
  rsResultCodeType risRet = RS_SUCCESS;

  if(NULL == pcapDisable) {
      risRet = RS_BADPARAM;
  } else {
      pcapDisable->cmd = WSU_NS_PCAP_DISABLE;
#if 0
      /* Send the message to Radio Service and wait for a reply */
      size = communicateWithRadioServices(pcapDisable, sizeof(*pcapDisable),
                                        &risRet, sizeof(risRet));
#endif

      if (size < 0) {
          risRet = RS_EIOCTL;
      }
  }
  return risRet;
}

rsResultCodeType wsuRisPcapRead(rsPcapReadType *pcapRead, uint8_t *pcapBuffer,
                                int pcapBufferLen, int *totalLength)
{
  rsPcapReadReplyType pcapReadReply;
  int                 offset = 0;
  int                 remainingLength = 0;
  int                 size = 0;

  memset(&pcapReadReply,0,sizeof(pcapReadReply));

  if((NULL == pcapRead) || (NULL == pcapBuffer) || (NULL == totalLength)) {
      pcapReadReply.risRet = RS_BADPARAM;
  } else {
      if (pcapRead->tx) {
          if (pcapBufferLen < PCAP_TX_LOG_BUF_SIZE) {
              return RS_ERANGE;
          }
      } else {
          if (pcapBufferLen < PCAP_LOG_BUF_SIZE) {
              return RS_ERANGE;
          }
      }

      pcapRead->cmd   = WSU_NS_PCAP_READ;
      pcapBufferLen   = (pcapRead->tx) ? PCAP_TX_LOG_BUF_SIZE : PCAP_LOG_BUF_SIZE;
      remainingLength = pcapBufferLen;
      offset          = 0;
      *totalLength    = 0;

    do {
#if 0
        /* Send the message to Radio Service and wait for a reply */
        size = communicateWithRadioServices(pcapRead, sizeof(*pcapRead),
                                            &pcapReadReply,
                                            sizeof(pcapReadReply));
#endif
//Stuff with baloney zeros.
        size = MAX_PCAP_READ_BYTES;
        pcapReadReply.size = size;
        pcapReadReply.done = WTRUE;

        if (size < 0) {
            pcapReadReply.risRet = RS_EIOCTL;
            return pcapReadReply.risRet;
        }

        if (pcapReadReply.risRet != RS_SUCCESS) {

            return pcapReadReply.risRet;
        }

        if (remainingLength < pcapReadReply.size) {
            pcapReadReply.size = remainingLength;
        }

        memcpy(&pcapBuffer[offset], pcapReadReply.data, pcapReadReply.size);
        offset          += pcapReadReply.size;
        *totalLength    += pcapReadReply.size;
        remainingLength -= pcapReadReply.size;

    } while ((!pcapReadReply.done) && (remainingLength > 0));
    
    if (!pcapReadReply.done) {
        pcapReadReply.risRet = RS_PCAPOVRFLOW;
    }
  }
  return pcapReadReply.risRet;
}

rsResultCodeType wsuRisInitRadioComm(char_t * app_abbrev, appCredentialsType *appCredential)
{
  rsResultCodeType ret = RS_SUCCESS;

  if((NULL == app_abbrev) || (NULL == appCredential)) {
      ret = RS_ERANGE;
  }

  if(1 == get_stub_signal(Signal_wsuRisInitRadioComm)) {
      ret = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisInitRadioComm);
  }

  return ret;       
}

rsResultCodeType risUserServiceRequest(UserServiceType *service)
{
  rsResultCodeType risRet = RS_SUCCESS; /* SUCCESS till FAIL */


  if(1 == get_stub_signal(Signal_risUserServiceRequest_1)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_risUserServiceRequest_1);
      return risRet; 
  }

  if(1 == get_stub_signal(Signal_risUserServiceRequest_2)) {
      risRet = RS_ENOROOM;
      clear_stub_signal(Signal_risUserServiceRequest_2);
      return risRet; 
  }

  if(NULL == service) {
      risRet = RS_ERANGE;
  }

  if(RS_SUCCESS == risRet) {
      if (service->radioNum > g_numberRadios - 1) {
          risRet = RS_ERANGE;
      }
  }

  if(RS_SUCCESS == risRet) {
      if (service->psid > PSID_4BYTE_MAX_VALUE ||
          (service->psid < PSID_4BYTE_MIN_VALUE &&
           service->psid > PSID_3BYTE_MAX_VALUE) ||
          (service->psid < PSID_3BYTE_MIN_VALUE &&
           service->psid > PSID_2BYTE_MAX_VALUE) ||
          (service->psid < PSID_2BYTE_MIN_VALUE &&
           service->psid > PSID_1BYTE_MAX_VALUE)) {

          risRet =  RS_ERANGE;
      }
  }
  if(RS_SUCCESS == risRet) {
      if (service->lengthPsc > LENGTH_PSC) {
          risRet = RS_ENOROOM;
      }
  }

  if(RS_SUCCESS == risRet) {
      if (service->lengthAdvertiseId > LENGTH_ADVERTISE) {
          risRet = RS_ENOROOM;
      }
  }

  return risRet; 

}
#if 0
rsResultCodeType releaseRPS(void)
{
  rsResultCodeType risRet = RS_SUCCESS;

  if(1 == get_stub_signal(Signal_releaseRPS)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_releaseRPS);
  }

  return risRet; 
}
#endif
rsResultCodeType wsuRisGetCfgReq(cfgIdType cfgId, cfgType *cfg)
{    
  rsResultCodeType risRet = RS_SUCCESS;
  int32_t size = 0;
  if(NULL == cfg) {
      risRet = RS_ERANGE;
  }
  if(RS_SUCCESS == risRet) {
    switch(cfgId){
    case CFG_ID_CV2X_ENABLE:
        cfg->u.CV2XEnable = WTRUE;
    break;
    case CFG_ID_DSRC_ENABLE:
        cfg->u.DSRCEnable = WFALSE;
    break;
    default:
    break;
    }
#if 0
    /* Fill out the fields not filled in by the caller */
    cfg->cmd   = WSU_NS_GET_NS_CFG;
    cfg->cfgId = cfgId;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(cfg, sizeof(*cfg), cfg, sizeof(*cfg));
#else
    size = sizeof(*cfg);
    cfg->risRet = RS_SUCCESS;
#endif
    if (size < 0) {
        cfg->risRet = RS_EIOCTL;
        //RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (cfg->risRet != RS_SUCCESS) {
        //RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(cfg->risRet));
    }

    return cfg->risRet;
  }

  if(1 == get_stub_signal(Signal_wsuRisGetCfgReq)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisGetCfgReq);
  }

  return risRet; 
}

rsResultCodeType wsuRisUserServiceRequest(UserServiceType *service)
{    
  rsResultCodeType risRet = RS_SUCCESS;

  if(NULL == service) {
      risRet = RS_ERANGE;
  }

  if(RS_SUCCESS == risRet) {
      risRet = risUserServiceRequest(service);
  }

  if(1 == get_stub_signal(Signal_wsuRisUserServiceRequest)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisUserServiceRequest);
  }

  return risRet; 
}

rsResultCodeType wsuRisWsmServiceRequest (WsmServiceType *service)
{
  rsResultCodeType risRet = RS_SUCCESS;

  if(NULL == service) {
      risRet = RS_ERANGE;
  }

  if(1 == get_stub_signal(Signal_wsuRisWsmServiceRequest)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisWsmServiceRequest);
  }

  if(1 == get_stub_signal(Signal_wsuRisWsmServiceRequest_2)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisWsmServiceRequest_2);
  }

  return risRet; 
}

rsResultCodeType wsuRisTerminateRadioComm(void)
{
  rsResultCodeType risRet = RS_SUCCESS;

  if(1 == get_stub_signal(Signal_wsuRisTerminateRadioComm)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisTerminateRadioComm);
  }

  return risRet; 
}

rsResultCodeType wsuRisGetRadioMacaddress(rsRadioType radioType, uint8_t radioNum, uint8_t *mac)
{
  rsResultCodeType risRet = RS_SUCCESS;

  radioType = radioType;
  radioNum = radioNum;
  mac = mac;
  if(1 == get_stub_signal(Signal_wsuRisGetRadioMacaddress)) {
      risRet = RS_EIOCTL;
      clear_stub_signal(Signal_wsuRisGetRadioMacaddress);
  }

  return risRet;
}

rsResultCodeType wsuRisSetCfgReq(cfgIdType cfgId, cfgType *cfg)
{
  rsResultCodeType risRet = RS_SUCCESS;

  cfgId = cfgId;
  cfg = cfg;
  if(1 == get_stub_signal(Signal_wsuRisSetCfgReq)) {
      risRet = RS_ERANGE;
      clear_stub_signal(Signal_wsuRisSetCfgReq);
  }

  return risRet;
}

void * WSU_SHARE_INIT( size_t size, char_t *spath)
{
  if(1 == get_stub_signal(Signal_wsu_share_init)) {
      clear_stub_signal(Signal_wsu_share_init);
      return NULL;
  }
  return wsu_share_init(size, spath);
}

bool_t WSU_INIT_GATE(wsu_gate_t * wsu_sem)
{
  if(1 == get_stub_signal((Signal_wsu_init_gate))) {
    if(0 == get_stub_iteration((Signal_wsu_init_gate))) {
      clear_stub_signal((Signal_wsu_init_gate));
      return WFALSE;
    } else {
      dec_stub_iteration((Signal_wsu_init_gate));
    }
  }
  return wsu_init_gate(wsu_sem);
}

bool_t WSU_KILL_GATE(wsu_gate_t * gate)
{
  if(1 == get_stub_signal((Signal_wsu_kill_gate))) {
    if(0 == get_stub_iteration((Signal_wsu_kill_gate))) {
      clear_stub_signal((Signal_wsu_kill_gate));
      return WFALSE;
    } else {
      dec_stub_iteration((Signal_wsu_kill_gate));
    }
  }
  return wsu_kill_gate(gate);
}

bool_t WSU_WAIT_AT_GATE(wsu_gate_t * wsu_sem)
{
  if(1 == get_stub_signal(Signal_wsu_wait_at_gate)) {
    if(0 == get_stub_iteration(Signal_wsu_wait_at_gate)) {
      clear_stub_signal(Signal_wsu_wait_at_gate);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_wsu_wait_at_gate);
    }
  }
  return wsu_wait_at_gate(wsu_sem);
}

bool_t WSU_OPEN_GATE(wsu_gate_t * wsu_sem)
{
  if(1 == get_stub_signal(Signal_wsu_open_gate)) {
    if(0 == get_stub_iteration(Signal_wsu_open_gate)) {
      clear_stub_signal(Signal_wsu_open_gate);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_wsu_open_gate);
    }
  }
  return wsu_open_gate(wsu_sem);
}

bool_t WSU_SHMLOCK_LOCKW(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_WSU_SEM_LOCKW)) {
    if(0 == get_stub_iteration(Signal_WSU_SEM_LOCKW)){
      clear_stub_signal(Signal_WSU_SEM_LOCKW);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_WSU_SEM_LOCKW);
    }
  }
  return wsu_shmlock_lockw(lock);
}

bool_t WSU_SHMLOCK_UNLOCKW(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_WSU_SEM_UNLOCKW)) {
    if(0 == get_stub_iteration(Signal_WSU_SEM_UNLOCKW)){
      clear_stub_signal(Signal_WSU_SEM_UNLOCKW);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_WSU_SEM_UNLOCKW);
    }
  }
  return wsu_shmlock_unlockw(lock);
}

bool_t WSU_SHMLOCK_KILL(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_wsu_shmlock_kill)) {
    if(0 == get_stub_iteration(Signal_wsu_shmlock_kill)){
      clear_stub_signal(Signal_wsu_shmlock_kill);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_wsu_shmlock_kill);
    }
  }
  return wsu_shmlock_kill(lock);
}

bool_t WSU_SHMLOCK_INIT(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_wsu_shmlock_init)) {
    if(0 == get_stub_iteration(Signal_wsu_shmlock_init)){
      clear_stub_signal(Signal_wsu_shmlock_init);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_wsu_shmlock_init);
    }
  }
  return wsu_shmlock_init(lock);
}

bool_t WSU_SHMLOCK_UNLOCKR(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_WSU_SEM_UNLOCKR)) {
    if(0 == get_stub_iteration(Signal_WSU_SEM_UNLOCKR)){
      clear_stub_signal(Signal_WSU_SEM_UNLOCKR);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_WSU_SEM_UNLOCKR);
    }
  }
  return wsu_shmlock_unlockr(lock);
}

bool_t WSU_SHMLOCK_LOCKR(wsu_shmlock_t * lock)
{
  if(1 == get_stub_signal(Signal_WSU_SEM_LOCKR)) {
    if(0 == get_stub_iteration(Signal_WSU_SEM_LOCKR)){
      clear_stub_signal(Signal_WSU_SEM_LOCKR);
      return WFALSE;
    } else {
      dec_stub_iteration(Signal_WSU_SEM_LOCKR);
    }
  }
  return wsu_shmlock_lockr(lock);
}

void WSU_SHARE_KILL( void * shrm_start, size_t size )
{
  if(1 == get_stub_signal(Signal_wsu_share_kill)) {
      clear_stub_signal(Signal_wsu_share_kill);
  }
  return wsu_share_kill(shrm_start,size );
}
void WSU_SHARE_DELETE( char_t * shm_path_str )
{
  return wsu_share_delete( shm_path_str );
}
#if 0
sem_t my_sema;

sem_t *sem_open(const char_t *name, int oflag,...)
{

  oflag = oflag;
  if(1 == get_stub_signal(Signal_sem_open)) {

      clear_stub_signal(Signal_sem_open);
      return SEM_FAILED;
  }
  if(NULL == name) {
      return SEM_FAILED;
  } else {
      return &my_sema;
  }

}

int sem_unlink(const char_t *name)
{
    if(NULL == name) {
        return -1;
    } else {
        return 0;
    }

}


int sem_post(sem_t *sem)
{
    if(NULL == sem) {
        return -1;
    } else {
        return 0;
    }
}

int sem_trywait(sem_t *sem)
{
    if(NULL == sem) {
        return -1;
    } else {
        return 0;
    }
}

int sem_close(sem_t *sem)
{

    if(NULL == sem) {
        return -1;
    } else {
        return 0;
    }
}
#endif
rsResultCodeType wsuRisGetNumRadios(rsRadioType radioType, uint8_t *num_radios)
{
  rsResultCodeType ret = RS_SUCCESS;

  radioType = radioType;
  if(NULL == num_radios) {
      ret = RS_ERANGE;
  } else {
      *num_radios=1;
  }

  return ret;
}

int timer_gettime(timer_t timerid, struct itimerspec *curr_value)
{
  int ret = 0;

  timerid = timerid;
  curr_value = curr_value;
  if(1 == get_stub_signal(Signal_timer_gettime)) {
      ret = -1;
      clear_stub_signal(Signal_timer_gettime);
  }

  return ret;

}

int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec * old_value)
{

  int ret = 0;

  timerid = timerid;
  flags = flags;
  new_value = new_value;
  old_value = old_value;
  if(1 == get_stub_signal(Signal_timer_settime)) {
      ret = -1;
      clear_stub_signal(Signal_timer_settime);
  }

  return ret;

}

int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
  int ret = 0;
  
  clockid = clockid;
  sevp = sevp;
  timerid = timerid;
  if(1 == get_stub_signal(Signal_timer_create)) {
      ret = -1;
      clear_stub_signal(Signal_timer_create);
  }

  return ret;
}

int timer_delete(timer_t timerid)
{
  int ret = 0;

  timerid = timerid;
  if(1 == get_stub_signal(Signal_timer_delete)) {
      ret = -1;
      clear_stub_signal(Signal_timer_delete);
  }

  return ret;
}

OSRTDListNode* rtxDListFindByIndex (const OSRTDList* pList, OSSIZE idx)
{
  OSRTDListNode * ret = NULL;
  idx = idx;
  if((NULL == pList) || (idx >= pList->count)) {
      // printf("\nrtxDListFindBy::ERROR: NULL inputs.");
      ret = NULL;
  } else {
      ret = &my_list;
  }

  if(1 == get_stub_signal(Signal_rtxDListFindByIndex)) {
      ret = NULL;
      clear_stub_signal(Signal_rtxDListFindByIndex);
  }
  return ret;
}

void rtxDListInit (OSRTDList* pList)
{
  if(1 == get_stub_signal(Signal_rtxDListInit)) {
      clear_stub_signal(Signal_rtxDListInit);
  } else {
      if (0 != pList) {
          pList->count = 0;
          pList->head = (OSRTDListNode*) 0;
          pList->tail = (OSRTDListNode*) 0;
       }
  }

}

void rtxDListFreeAll (struct OSCTXT* pctxt, OSRTDList* pList)
{
    pctxt = pctxt;
    pList = pList;
    return;
}

void* rtxMemHeapAlloc (void** ppvMemHeap, size_t nbytes)
{
    ppvMemHeap = ppvMemHeap;
    nbytes = nbytes;
    return (void*)&my_pMemHeap;
}

void asn1Free_SPAT (OSCTXT *pctxt, SPAT* pvalue)
{
    pctxt = pctxt;
    pvalue = pvalue;
    return;
}

OSRTDListNode* rtxDListAppend(struct OSCTXT* pctxt, OSRTDList* pList, void* pData)
{
    pctxt = pctxt;
    pList = pList;
    pData = pData;
    return NULL;
}

int asn1PE_SPAT (OSCTXT* pctxt, SPAT* pvalue)
{
  if((NULL == pctxt) || (NULL == pvalue)) {
    return -1;
  } else {
      if(1 == get_stub_signal(Signal_asn1PE_SPAT)) {
          clear_stub_signal(Signal_asn1PE_SPAT);
          return -1;
      }
      return 0;
  }
}
int asn1Init_SPAT (SPAT* pvalue)
{
    pvalue = pvalue;
    return 0;
}

int asn1Init_BasicSafetyMessage (BasicSafetyMessage* pvalue)
{
   pvalue = pvalue;
   return 0;
}
int asn1Init_PersonalSafetyMessage (PersonalSafetyMessage* pvalue)
{
   pvalue = pvalue;
   return 0;
}

static BasicSafetyMessage    local_bsm;
static PersonalSafetyMessage local_psm;

int asn1PD_BasicSafetyMessage (OSCTXT* pctxt, BasicSafetyMessage* pvalue)
{
   pctxt = pctxt;
   pvalue = pvalue;
   memset(&local_bsm,0x0,sizeof(local_bsm));
#if defined(J2735_2016)
   local_bsm.coreData.brakes.traction = 1;
   local_bsm.coreData.brakes.abs_ = 1; /* Nicely done ITE. */ 
   local_bsm.coreData.brakes.scs = 1;
   local_bsm.coreData.brakes.wheelBrakes.data[0] = 0x8;
#endif
#if defined(J2735_2023)
   if(NULL != local_bsm.coreData) {
       local_bsm.coreData->brakes.traction = 1;
       local_bsm.coreData->brakes.abs_ = 1; /* Nicely done ITE. */ 
       local_bsm.coreData->brakes.scs = 1;
       local_bsm.coreData->brakes.wheelBrakes.data[0] = 0x8;
   }
#endif

   memcpy(pvalue,&local_bsm,sizeof(local_bsm));
   return 0;
}
int asn1PD_PersonalSafetyMessage (OSCTXT* pctxt, PersonalSafetyMessage* pvalue)
{
   pctxt = pctxt;
   pvalue = pvalue;
   memset(&local_psm,0x0,sizeof(local_psm));
   local_psm.m.accelSetPresent = 1;
#if defined(J2735_2016)
   local_psm.position.m.elevationPresent = 1;
#endif
#if defined(J2735_2023)
   if(NULL != local_psm.position) {
       local_psm.position->m.elevationPresent = 1;
   }
#endif
   memcpy(pvalue,&local_psm,sizeof(local_psm));
   return 0;
}

static OSOCTET my_msg_pointer; 
//Not sure how to include ASN1 libs. Not compatible with CUNIT.
OSOCTET* pe_GetMsgPtr (OSCTXT* pctxt, OSINT32* pLength)
{
  if((NULL == pLength) || (NULL == pctxt)) {
      return NULL;
  }

  if(1 == get_stub_signal(Signal_pe_GetMsgPtr)) {
      clear_stub_signal(Signal_pe_GetMsgPtr);
      *pLength=0; /*pass nothing back.*/
      return NULL;
  }
  *pLength=100;
  return (OSOCTET *)&my_msg_pointer;
}

int asn1Init_SignalRequestList (SignalRequestList* pvalue)
{
  int ret = 0;

  rtxDListFastInit (pvalue);
  
  if(1 == get_stub_signal(Signal_asn1Init_SignalRequestList)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_SignalRequestList);
  }

  return ret;

}

int rtxMemHeapCreate (void** ppvMemHeap)
{
   int ret = 0;
  
   if (ppvMemHeap == 0) {
     ret = RTERR_INVPARAM;
   }

   if(0 == ret) {

       memset(&my_pMemHeap, 0, sizeof (OSMemHeap));

       my_pMemHeap.defBlkSize       = 1024; 
       my_pMemHeap.refCnt           = 1;
       my_pMemHeap.flags            = RT_MH_FREEHEAPDESC;
       my_pMemHeap.rawBlkList.pnext = my_pMemHeap.rawBlkList.pprev = &my_pMemHeap.rawBlkList;

       *ppvMemHeap = (void*)&my_pMemHeap;
  }
  
  if(1 == get_stub_signal(Signal_rtxMemHeapCreate_1)) {
      ret = RTERR_INVPARAM;
      clear_stub_signal(Signal_rtxMemHeapCreate_1);
  }

  if(1 == get_stub_signal(Signal_rtxMemHeapCreate_2)) {
      ret = RTERR_NOMEM;
      clear_stub_signal(Signal_rtxMemHeapCreate_2);
  }

  return ret;

}

void rtErrASN1Init (void) {}


int rtxPreInitContext (OSCTXT* pctxt)
{
  int ret = 0;

  if(NULL == pctxt) {
      ret = -1;
  }

  if(0 == ret) {
     memset(pctxt, 0, sizeof(OSCTXT));
   
     pctxt->initCode = OSCTXTINIT;
     pctxt->buffer.bitOffset = 8;
     pctxt->indent = 0;
     
  }

  if(1 == get_stub_signal(Signal_rtxPreInitContext)) {
      ret = -1;
      clear_stub_signal(Signal_rtxPreInitContext);
  }

  return ret;
  
}

int rtxInitContext (OSCTXT* pctxt)
{
  int ret = 0;

  if(NULL == pctxt) {
      ret = -1;
  }

  if(0 == ret) {
      rtxPreInitContext (pctxt);
      ret = rtxMemHeapCreate (&pctxt->pMemHeap);
  }

  if(1 == get_stub_signal(Signal_rtxInitContext)) {
      ret = -1;
      clear_stub_signal(Signal_rtxInitContext);
  }

  return ret;
}

int rtInitASN1Context (OSCTXT* pctxt)
{
  int ret = 0;

  if(NULL == pctxt) {
      ret = -1;
  }

  if(0 == ret) {
      ret = rtxInitContext (pctxt);

      if (0 == ret) {      
          rtErrASN1Init ();
          pctxt->indent = 3; /* for JSON */
      }
  }

  if(1 == get_stub_signal(Signal_rtxInitASN1Context)) {
      ret = -1;
      clear_stub_signal(Signal_rtxInitASN1Context);
  }

  return ret;
}

int asn1Init_VehicleID (VehicleID* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
     pvalue->t = 0;
     memset(&pvalue->u, 0, sizeof(pvalue->u));
     ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_VehicleID)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_VehicleID);
  }

  return ret;
}


int asn1Init_RegionalExtension (RegionalExtension* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
     pvalue->regExtValue.numocts = 0;
     pvalue->regExtValue.data = 0;
     ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_RegionalExtension)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_RegionalExtension);
  }

  return ret;
}

int asn1Init_RequestorType (RequestorType* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
     memset(&pvalue->m, 0, sizeof (pvalue->m));
     ret = asn1Init_RegionalExtension(&pvalue->regional);

     if(0 == ret)
       rtxDListFastInit (&pvalue->extElem1);
  }

  if(1 == get_stub_signal(Signal_asn1Init_RequestorType)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_RequestorType);
  }

  return ret;
}

int asn1Init_Position3D_regional (Position3D_regional* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      rtxDListFastInit (pvalue);
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_Position3D_regional)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_Position3D_regional);
  }

  return ret;
}

int asn1Init_Position3D (Position3D* pvalue)
{ 
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
     memset(&pvalue->m, 0, sizeof (pvalue->m));
     ret = asn1Init_Position3D_regional (&pvalue->regional);

     if(0 == ret)
       rtxDListFastInit (&pvalue->extElem1);
  }

  if(1 == get_stub_signal(Signal_asn1Init_Position3D)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_Position3D);
  }

  return ret;

}
#if defined(J2735_2023)
static Position3D  my_pos3d;
#endif
int asn1Init_RequestorPositionVector (RequestorPositionVector* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
     memset(&pvalue->m, 0, sizeof (pvalue->m));
#if defined(J2735_2016)
     ret = asn1Init_Position3D(&pvalue->position);
#endif
#if defined(J2735_2023)
     pvalue->position = &my_pos3d;
     ret = asn1Init_Position3D(pvalue->position);
#endif

printf("\nasn1Init_RequestorPositionVector ret2=%d\n",ret);
     if(0 == ret)
        rtxDListFastInit (&pvalue->extElem1);
  }

  if(1 == get_stub_signal(Signal_asn1Init_RequestorPositionVector)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_RequestorPositionVector);
  }

  return ret;
}

int asn1Init_TransitVehicleStatus (TransitVehicleStatus* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      pvalue->numbits = 0;
      memset(pvalue->data, 0, sizeof (pvalue->data));
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_TransitVehicleStatus)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_TransitVehicleStatus);
  }

  return ret;
}


int asn1Init_RequestorDescription_regional (RequestorDescription_regional* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      rtxDListFastInit (pvalue);
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_RequestorDescription_regional)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_RequestorDescription_regional);
  }

  return ret;
}
#if defined(J2735_2023)
static VehicleID my_vehicle_id;
#endif
int asn1Init_RequestorDescription (RequestorDescription* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      memset(&pvalue->m, 0, sizeof (pvalue->m));
      #if defined(J2735_2016)
      ret = asn1Init_VehicleID (&pvalue->id);
      #endif
      #if defined(J2735_2023)
      pvalue->id = &my_vehicle_id;
      ret = asn1Init_VehicleID (pvalue->id);
      #endif
      if(0 == ret) {
          #if defined(J2735_2016)
          ret = asn1Init_RequestorType(&pvalue->type);
          #endif
          #if defined(J2735_2022)
          ret = asn1Init_RequestorType(pvalue->type);
          #endif
      }
      if(0 == ret) {
          ret = asn1Init_RequestorPositionVector(&pvalue->position);
      }
      if(0 == ret) {
          pvalue->name = 0;
          pvalue->routeName = 0;

          ret = asn1Init_TransitVehicleStatus (&pvalue->transitStatus);
      }
      if(0 == ret) {
          ret = asn1Init_RequestorDescription_regional (&pvalue->regional);
      }
      if(0 == ret) {
          rtxDListFastInit (&pvalue->extElem1);
      }      
  }

  if(1 == get_stub_signal(Signal_asn1Init_RequestorDescription)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_RequestorDescription);
  }
  return ret;
}

int asn1Init_SignalRequestMessage_regional (SignalRequestMessage_regional* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      rtxDListFastInit (pvalue);
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1Init_SignalRequestMessage_regional)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_SignalRequestMessage_regional);
  }
  return ret;
}

int asn1Init_SignalRequestMessage (SignalRequestMessage* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      memset(&pvalue->m, 0, sizeof (pvalue->m));

      ret = asn1Init_SignalRequestList(&pvalue->requests);

      if(0 == ret) {
          ret = asn1Init_RequestorDescription(&pvalue->requestor);
      }
      if(0 == ret) {
          ret = asn1Init_SignalRequestMessage_regional(&pvalue->regional);
      }
      if(0 == ret) {
          rtxDListFastInit(&pvalue->extElem1);
      }
  }

  if(1 == get_stub_signal(Signal_asn1Init_SignalRequestMessage)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_SignalRequestMessage);
  }

  return ret;
}


int asn1Init_MessageFrame (MessageFrame* pvalue)
{
  int ret = 0;

  if(NULL == pvalue) {
      ret = -1;
  } else {
      pvalue->value.numocts = 0;
      pvalue->value.data = 0;
      rtxDListFastInit (&pvalue->extElem1);
  }

  if(1 == get_stub_signal(Signal_asn1Init_MessageFrame)) {
      ret = -1;
      clear_stub_signal(Signal_asn1Init_MessageFrame);
  }

  return ret;
}


int pu_setBuffer(OSCTXT* pctxt, OSOCTET* bufaddr, size_t bufsiz, OSBOOL aligned)
{
  int ret = 0;

  if(    (NULL == pctxt) 
      || (NULL == bufaddr)
      || (bufsiz < 1)
      || !((WTRUE == aligned) || (WFALSE == aligned))) 
  {
      ret = -1;
  } else {

      if(1 == get_stub_signal(Signal_pu_setBuffer)) {
          ret = -1;
          clear_stub_signal(Signal_pu_setBuffer);
      } else {
          ret = rtxInitContextBuffer (pctxt, bufaddr, bufsiz);

          if(0 == ret)
            pctxt->buffer.aligned = aligned;

      }
  }

  return ret;
}


int rtxInitContextBuffer(OSCTXT* pctxt, OSOCTET* bufaddr, OSSIZE bufsiz)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == bufaddr) || (0 == bufsiz)) {
      ret = RTERR_NOTINIT;
  } else {
      pctxt->buffer.data = bufaddr; 
      pctxt->buffer.size = bufsiz;
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_rtxInitContextBuffer)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_rtxInitContextBuffer);
  }

  return ret;

}


int asn1PD_MessageFrame (OSCTXT* pctxt, MessageFrame* pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      pvalue->value.data     = pctxt->buffer.data; 
      pvalue->value.numocts  = pctxt->buffer.size;
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1PD_MessageFrame)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_asn1PD_MessageFrame);
  }

  return ret;
}

int asn1PE_MessageFrame(OSCTXT * pctxt, MessageFrame * pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1PE_MessageFrame)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_asn1PE_MessageFrame);
  }

  return ret;
}

int asn1PD_SignalRequestMessage_regional (OSCTXT* pctxt, SignalRequestMessage_regional* pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1PD_SignalRequestMessage_regional)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_asn1PD_SignalRequestMessage_regional);
  }

  return ret;
}


int asn1PD_SignalRequestMessage (OSCTXT* pctxt, SignalRequestMessage* pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
      pvalue->sequenceNumber = my_sequenceNumber++;
      pvalue->requests.count = 1;
  }

  if(1 == get_stub_signal(Signal_asn1PD_SignalRequestMessage_1)) {
      ret = -1;
      clear_stub_signal(Signal_asn1PD_SignalRequestMessage_1);
  }

  if(1 == get_stub_signal(Signal_asn1PD_SignalRequestMessage_2)) {
      my_sequenceNumber--;
      pvalue->sequenceNumber = my_sequenceNumber;
      clear_stub_signal(Signal_asn1PD_SignalRequestMessage_2);
  }

  return ret;
}

int asn1PE_SignalRequestMessage_regional (OSCTXT* pctxt, SignalRequestMessage_regional* pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1PE_SignalRequestMessage_regional)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_asn1PE_SignalRequestMessage_regional);
  }

  return ret;
}


int asn1PE_SignalRequestMessage (OSCTXT* pctxt, SignalRequestMessage* pvalue)
{
  int ret = 0;

  if((NULL == pctxt) || (NULL == pvalue)) {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_asn1PE_SignalRequestMessage)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_asn1PE_SignalRequestMessage);
  }

  return ret;
}


void rtxErrPrint (OSCTXT* pctxt)
{
  if(NULL != pctxt) {
    pctxt = pctxt; 
  } 
  return;
}


void rtxMemReset (OSCTXT* pctxt)
{
  if(NULL != pctxt) {
    pctxt = pctxt; 
  } 
  return;
}


void rtxMemFree (OSCTXT* pctxt)
{

  if(NULL != pctxt) {
    pctxt = pctxt; 
  } 
  return;
}


int rtxErrReset (OSCTXT* pctxt)
{
  int ret = 0;

  if(NULL == pctxt)  {
      ret = RTERR_NOTINIT;
  } else {
      ret = 0;
  }

  if(1 == get_stub_signal(Signal_rtxErrReset)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_rtxErrReset);
  }

  return ret;
}

size_t pu_getMsgLen (OSCTXT* pctxt)
{

  size_t ret = 0;

  if(NULL == pctxt)  {
      ret = RTERR_NOTINIT;
  } else {
      //ret = (pctxt->buffer.bitOffset == 8) ? pctxt->buffer.byteIndex : pctxt->buffer.byteIndex + 1;
      ret = pctxt->buffer.size;
  }

  if(1 == get_stub_signal(Signal_pu_getMsgLen_1)) {
      ret = RTERR_NOTINIT;
      clear_stub_signal(Signal_pu_getMsgLen_1);
  }
  if(1 == get_stub_signal(Signal_pu_getMsgLen_2)) {
      ret = 0x7FFFFFFF; /*real big*/
      clear_stub_signal(Signal_pu_getMsgLen_2);
  }
  return ret;
}

void asn1Free_MessageFrame (OSCTXT *pctxt, MessageFrame* pvalue)
{
    pctxt = pctxt;
    pvalue = pvalue;
    return;
}
void asn1Free_SignalRequestMessage (OSCTXT *pctxt, SignalRequestMessage* pvalue)
{
    pctxt = pctxt;
    pvalue = pvalue;
    return;
}

rsResultCodeType wsuRisProviderServiceRequest(ProviderServiceType *service)
{
  rsResultCodeType  risRet = RS_SUCCESS;

  if(NULL == service) {
    risRet = RS_ERANGE;
  }

  if(1 == get_stub_signal(Signal_wsuRisProviderServiceRequest)) { 
      clear_stub_signal(Signal_wsuRisProviderServiceRequest);
      return RS_ERANGE; 
  }
  return (risRet == RS_ESOCKSEND) ? risRet : RS_SUCCESS;
}

rsResultCodeType wsuRisSendWSMReq(outWSMType *WSM)
{

  rsResultCodeType  risRet = RS_SUCCESS;

  if(NULL == WSM) {
      risRet = RS_ERANGE;  
  }


  if(1 == get_stub_signal(Signal_wsuRisSendWSMReq_1)) {
      risRet = RS_ERANGE;  
      clear_stub_signal(Signal_wsuRisSendWSMReq_1);
  }

  if(1 == get_stub_signal(Signal_wsuRisSendWSMReq_2)) {
      risRet = RS_ESOCKSEND;  
      clear_stub_signal(Signal_wsuRisSendWSMReq_2);
  }

  //RIS_MESSAGE_BUFFER_SIZE

  return risRet;

}

int dn_setsockopt(  int socket
                  , int level
                  , int option_name
                  , const void *option_value
                  , socklen_t option_len)
{
  if(1 == get_stub_signal(Signal_setsockopt)) {

    if(0 == get_stub_iteration(Signal_setsockopt)) {
      clear_stub_signal(Signal_setsockopt);
      return -1;

    } else {
      dec_stub_iteration(Signal_setsockopt);
    }
  }

  return setsockopt(socket, level,option_name,option_value, option_len);
}
#if defined (UNIT_PTHREAD)
int dn_pthread_detach(pthread_t thread)
{
  if(1 == get_stub_signal(Signal_pthread_detach)) {

    if(0 == get_stub_iteration(Signal_pthread_detach)) {
      clear_stub_signal(Signal_pthread_detach);
      return EINVAL;

    } else {
      dec_stub_iteration(Signal_pthread_detach);
    }
  }

  return pthread_detach(thread);

}

int dn_pthread_create (pthread_t *thread
                   , const pthread_attr_t *attr
                   , void *(*start_routine) (void *)
                   , void *arg)
{
  if(1 == get_stub_signal(Signal_pthread_create)) {

    if(0 == get_stub_iteration(Signal_pthread_create)) {
      clear_stub_signal(Signal_pthread_create);
      return -1;

    } else {
      dec_stub_iteration(Signal_pthread_create);
    }
  }
  return pthread_create (thread,attr,start_routine,arg);
}
#endif /* UNIT_PTHREAD */
/*do not stub sockets. Use wrappers. CUnit will barf otherwise. */
int CONNECT(int s, const struct sockaddr * name, socklen_t namelen)
{
  if(1 == get_stub_signal(Signal_connect_error)) {
      if(0 == get_stub_iteration(Signal_connect_error)) {
          clear_stub_signal(Signal_connect_error);
          return -1;
      } else {
          dec_stub_iteration(Signal_connect_error);
      }
  }
  return connect(s, name, namelen);
}

int SOCKET(int domain,int type, int protocol)
{
    if(1 == get_stub_signal(Signal_socket)) {

      if(0 == get_stub_iteration(Signal_socket)) {

        clear_stub_signal(Signal_socket);
        return -1;

      } else {
        dec_stub_iteration(Signal_socket);
      }
    }

    return socket(domain, type, protocol);
}

int dn_bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
    if(1 == get_stub_signal(Signal_bind)) {
        clear_stub_signal(Signal_bind);
        return -1;
    }
    return bind(sockfd,addr,addrlen);
}

void DN_FD_ZERO(fd_set *set)
{
  if(NULL == set)
    return;

  //FD_ZERO(set);
  return;
}

void DN_FD_SET(int fd, fd_set *set)
{
  if((fd < 0) || (NULL == set))
    return;

  //FD_SET(fd,set);
  return;
}

void DN_FD_CLR(int fd, fd_set *set)
{
  if((fd < 0) || (NULL == set))
    return;

  //FD_CLR(fd, set);
  return;
}

int  DN_FD_ISSET(int fd, fd_set *set)
{
  if((fd < 0) || (NULL == set)) {
    printf("\nDN_FD_ISSET ERROR!.\n"); 
    return WFALSE;
  }

    if(1 == get_stub_signal(Signal_DN_FD_ISSET_fail)) {
      if(0 == get_stub_iteration(Signal_DN_FD_ISSET_fail)) {
        clear_stub_signal(Signal_DN_FD_ISSET_fail);
        return WFALSE;
      } else {
        dec_stub_iteration(Signal_DN_FD_ISSET_fail);
      }
    }

    if(1 == get_stub_signal(Signal_DN_FD_ISSET_force_fail)) {
      return WFALSE;
    }

  return WTRUE;
}

int dn_select( int      nfds 
              ,fd_set * readfds
              ,fd_set * writefds
              ,fd_set * exceptfds
              ,struct timeval * timeout)
{
    if(1 == get_stub_signal(Signal_select_fail)) {
      if(0 == get_stub_iteration(Signal_select_fail)) {
        clear_stub_signal(Signal_select_fail);
        return -1;
      } else {
        dec_stub_iteration(Signal_select_fail);
      }
    }

    if(1 == get_stub_signal(Signal_select_busy)) {
      if(0 == get_stub_iteration(Signal_select_busy)) {
        clear_stub_signal(Signal_select_busy);
        return 0;
      } else {
        dec_stub_iteration(Signal_select_busy);
      }
    }

    if(1 == get_stub_signal(Signal_select_aok)) {
      if(0 == get_stub_iteration(Signal_select_aok)) {
        clear_stub_signal(Signal_select_aok);
        return 1;
      } else {
        dec_stub_iteration(Signal_select_aok);
      }
    }

  return select(nfds,readfds,writefds,exceptfds,timeout);
}
uint8_t my_spat_buf[SPAT_BUF_SIZE];
ssize_t dn_recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen)
{
  int32_t i;
  uint8_t * foo;
    if(NULL == buf) {
      return -1;
    }

    sockfd = sockfd;
    flags = flags;
    src_addr = src_addr;
    addrlen = addrlen;

    foo = (uint8_t *)buf;

    if(1 == get_stub_signal(Signal_recvfrom_aok)) {
      if(0 == get_stub_iteration(Signal_recvfrom_aok)) {
        clear_stub_signal(Signal_recvfrom_aok);

        for(i=0;(i < SPAT_BUF_SIZE) && (i < (int32_t)len) ;i++) {
          foo[i] = my_spat_buf[i];
        }

        return i; //magic length of blob
      } else {
        dec_stub_iteration(Signal_recvfrom_aok);
      }
    }
    if(1 == get_stub_signal(Signal_recvfrom_fail)) {
      if(0 == get_stub_iteration(Signal_recvfrom_fail)) {
        clear_stub_signal(Signal_recvfrom_fail);
        return -1;
      } else {
        dec_stub_iteration(Signal_recvfrom_fail);
      }
    }

    return 0;
}

int INET_ATON(const char_t * cp, struct in_addr * addr)
{
    if(1 == get_stub_signal(Signal_inet_aton)) {
        if(0 == get_stub_iteration(Signal_inet_aton)) {
            clear_stub_signal(Signal_inet_aton);
            return 0;
        } else {
            dec_stub_iteration(Signal_inet_aton);
        }
    }

    return inet_aton(cp, addr);
}

char_t * dn_inet_ntoa(struct in_addr in)
{
  in = in;
  if(1 == get_stub_signal(Signal_inet_ntoa_aok)) {
      if(0 == get_stub_iteration(Signal_inet_ntoa_aok)) {
        clear_stub_signal(Signal_inet_ntoa_aok);
        return "192.168.1.47"; //cfg.localSignalControllerIP;
      } else {
        dec_stub_iteration(Signal_inet_ntoa_aok);
      }
  }

  return "192.168.1.1";
}

int dn_mkfifo(const char_t *pathname, mode_t mode)
{
  if(NULL == pathname) {
      return -1;
  }
  mode = mode;
  if(1 == get_stub_signal(Signal_mkfifo)) {
      if(0 == get_stub_iteration(Signal_mkfifo)) {
        clear_stub_signal(Signal_mkfifo);
        return -1;
      } else {
        dec_stub_iteration(Signal_mkfifo);
      }
  }
  return 0;
}

int dn_msgget(key_t key, int msgflg)
{
  key =  key;
  msgflg = msgflg;
  if(1 == get_stub_signal(Signal_msgget)) {
      if(0 == get_stub_iteration(Signal_msgget)) {
        clear_stub_signal(Signal_msgget);
        return -1;
      } else {
        dec_stub_iteration(Signal_msgget);
      }
  }
  return 1; /* Only one queue is created and thats 1 */
}

int dn_msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
  msqid = msqid;
  cmd = cmd;
  buf = buf;
  if(1 == get_stub_signal(Signal_msgctl)) {
      if(0 == get_stub_iteration(Signal_msgctl)) {
        clear_stub_signal(Signal_msgctl);
        return -1;
      } else {
        dec_stub_iteration(Signal_msgctl);
      }
  }
  return 1; /* Only one queue is created and thats 1 */
}


int dn_getifaddrs(struct ifaddrs **ifap)
{
  if(1 == get_stub_signal(Signal_getifaddrs)) {
      if(0 == get_stub_iteration(Signal_getifaddrs)) {
        clear_stub_signal(Signal_getifaddrs);
        return -1;
      } else {
        dec_stub_iteration(Signal_getifaddrs);
      }
  }
  return getifaddrs(ifap);
}
static char_t blob[100];
void * RTXMEMALLOC(OSCTXT * ctxt, size_t blobLength)
{
  ctxt = ctxt;
  if(1 == get_stub_signal(Signal_rtxMemAlloc)) {
      if(0 == get_stub_iteration(Signal_rtxMemAlloc)) {
        clear_stub_signal(Signal_rtxMemAlloc);
        return NULL;
      } else {
        dec_stub_iteration(Signal_rtxMemAlloc);
      }
  }
  //Still a stub. Dont call without having first properly setup the ctxt.
  if(blobLength < 100)
      return blob; //rtxMemAlloc(ctxt, blobLength);
  else
      return NULL;

}
#if defined (NO_SECURITY)
#include "ws_errno.h"
static char_t my_alsmi_ver[] = "Alsmi unit test 1.0";
const char_t *alsmiGetVersion(void)
{
  return my_alsmi_ver;
}

int32_t alsmiAddUnsecured1609p2Hdr(bool_t    WSA,
                               uint8_t  *inData,
                               uint16_t  inDataLength,
                               void     *additionalSecurityData)
{
  return 0;
}

AEROLINK_RESULT alsmiSign(uint8_t  *unsignedData,
                          uint16_t  unsignedDataLength,
                          uint8_t  *additionalData,
                          uint8_t   additionalDataLen,
                          uint8_t   pktType,
                          void     *additionalSecurityParameters)
{
  return WS_SUCCESS;
}
void alsmiSetDebugLogValue(void)
{

}
typedef uint32_t atlk_rc_t;
/** API function return code constructor */
#define ATLK_RC(code) ((atlk_rc_t)(code))

/** Operation successful */
#define ATLK_OK ATLK_RC(0)


/** CV2X function out parameter */
#define CV2X_OUT
/** CV2X function in parameter */
#define CV2X_IN
/** CV2X function in and out parameter */
#define CV2X_INOUT


atlk_rc_t v2x_sockets_passthrough_set(int on_off)
{
  return ATLK_OK;
}


/** CV2X Modulation Cosing Scheme (MCS) enum, i.e. MCS values from 0 to 31 */
typedef int8_t cv2x_mcs_t;

/** Maximum number of elements in TBS-MCS arrays for US SAE profile */
#define CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS            15
/** Default configuration for US SAE */
#define CV2X_US_SAE_WIN_ADJUSTMENT_ENABLE_DEFAULT    0
#define CV2X_US_SAE_CBR_THRESHOLD_1_DEFAULT          25
#define CV2X_US_SAE_T2_1_DEFAULT                     20
#define CV2X_US_SAE_CBR_THRESHOLD_2_DEFAULT          65
#define CV2X_US_SAE_T2_2_DEFAULT                     50
#define CV2X_US_SAE_PDB_REDUCTION_DEFAULT            10

/** max number of carriers */
#define CV2X_CARRIER_POOL_MAX_NUM_CARRIERS 2
/** Max number of Rx pools (within a carrier) */
#define CV2X_CARRIER_POOL_MAX_NUM_RX_POOLS 4
/** Max number of Tx pools (within a carrier) */
#define CV2X_CARRIER_POOL_MAX_NUM_TX_POOLS 4

/** CV2X Priority enum, i.e. PPPP values from 0 to 7 */
typedef enum {
  CV2X_PPPP_0 = 0,
  CV2X_PPPP_1,
  CV2X_PPPP_2,
  CV2X_PPPP_3,
  CV2X_PPPP_4,
  CV2X_PPPP_5,
  CV2X_PPPP_6,
  CV2X_PPPP_7
} cv2x_pppp_t;

/** Min priority */
#define CV2X_PPPP_MIN         CV2X_PPPP_0
/** Max priority */
#define CV2X_PPPP_MAX         CV2X_PPPP_7
/** CV2X priority validation */
#define CV2X_PPPP_IS_VALID(p) (((p) <= CV2X_PPPP_MAX)) // ((p)>=CV2X_PPPP_MIN) &&
/** Invalid priority */
#define CV2X_INVALID_PPPP     (-1)

/** Min MCS value */
#define CV2X_MCS_MIN         0
/** Max MCS value */
#define CV2X_MCS_MAX         31
/** CV2X MCS validation */
#define CV2X_MCS_IS_VALID(p) (((p) <= CV2X_MCS_MAX) && ((p) >= CV2X_MCS_MIN))
/** Invalid MCS */
#define CV2X_INVALID_MCS     (-1)

/* Lists minimum & maximum length */

#define CV2X_RRC_MIN_FREQ_V2X_R14                         1
#define CV2X_RRC_MAX_FREQ_V2X_R14                         8

#define CV2X_RRC_MIN_SL_V2X_TX_CONFIG2_R14                1
#define CV2X_RRC_MAX_SL_V2X_TX_CONFIG2_R14                128

#define CV2X_RRC_MIN_SL_V2X_CBR_CONFIG2_R14               1
#define CV2X_RRC_MAX_SL_V2X_CBR_CONFIG2_R14               8

#define CV2X_RRC_MIN_SL_V2X_RX_POOL_PRECONF_R14           1
#define CV2X_RRC_MAX_SL_V2X_RX_POOL_PRECONF_R14           16

#define CV2X_RRC_MIN_SL_V2X_TX_POOL_PRECONF_R14           1
#define CV2X_RRC_MAX_SL_V2X_TX_POOL_PRECONF_R14           8

#define CV2X_RRC_MIN_RESERVATION_PERIOD_R14               1
#define CV2X_RRC_MAX_RESERVATION_PERIOD_R14               16

#define CV2X_RRC_MIN_PSSCH_TX_CONFIG_R14                  1
#define CV2X_RRC_MAX_PSSCH_TX_CONFIG_R14                  16

#define CV2X_RRC_MIN_CBR_PSSCH_TX_CONFIG                  1
#define CV2X_RRC_MAX_CBR_PSSCH_TX_CONFIG                  8

#define CV2X_RRC_MIN_CBR_LEVEL_R14                        1
#define CV2X_RRC_MAX_CBR_LEVEL_R14                        16

#define CV2X_RRC_THRES_PSSCH_RSRP_LIST_LEN                64

#define CV2X_RRC_POOL_BMP_LEN                             13

/* Variables' minimum & maximum values */
#define CV2X_RRC_MAX_EARFCN2                              262143
#define CV2X_RRC_MAX_MCS_PSSCH_R14                        31
#define CV2X_RRC_MAX_SUB_CHANNEL_NUMBER_PSSCH_R14         20
#define CV2X_RRC_MAX_CR_LIMIT_R14                         10000
#define CV2X_RRC_MIN_TX_POWER_R14                         (-41)
#define CV2X_RRC_MAX_TX_POWER_R14                         31
#define CV2X_RRC_TX_POWER_R14_MINUSINFINITY               0
#define CV2X_RRC_MAX_CBR_LEVEL_VALUE_R14                  100
#define CV2X_RRC_MIN_PRIORITY_R13                         1
#define CV2X_RRC_MAX_PRIORITY_R13                         8
#define CV2X_RRC_MAX_OFFSET_DFN                           1000
#define CV2X_RRC_MIN_P_MAX                                (-30)
#define CV2X_RRC_MAX_P_MAX                                33
#define CV2X_RRC_MIN_ADDITIONAL_SPECTRUM_EMISSION_R12     1
#define CV2X_RRC_MAX_ADDITIONAL_SPECTRUM_EMISSION_R12     32
#define CV2X_RRC_MIN_ADDITIONAL_SPECTRUM_EMISSION_V10L0   33
#define CV2X_RRC_MAX_ADDITIONAL_SPECTRUM_EMISSION_V10L0   288
#define CV2X_RRC_MIN_ZONE_ID_LONGI_MOD_R14                1
#define CV2X_RRC_MAX_ZONE_ID_LONGI_MOD_R14                4
#define CV2X_RRC_MIN_ZONE_ID_LATI_MOD_R14                 1
#define CV2X_RRC_MAX_ZONE_ID_LATI_MOD_R14                 4
#define CV2X_RRC_MAX_SYNC_OFFSET_INDICATOR_R14            159
#define CV2X_RRC_MIN_TX_PARAMETERS_R14                    (-126)
#define CV2X_RRC_MAX_TX_PARAMETERS_R14                    31
#define CV2X_RRC_MAX_SYNC_TX_THRESH_OOC_R14               11
#define CV2X_RRC_MIN_MIN_NUM_CANDIDATE_SF_R14             1
#define CV2X_RRC_MAX_MIN_NUM_CANDIDATE_SF_R14             13
#define CV2X_RRC_MAX_THRES_PSSCH_RSRP                     66
#define CV2X_RRC_MAX_OFFSET_INDICATOR_SMALL_R12           319
#define CV2X_RRC_MAX_OFFSET_INDICATOR_LARGE_R12           10239
#define CV2X_RRC_MAX_START_RB_SUBCHANNEL_R14              99
#define CV2X_RRC_MAX_START_RB_PSCCH_POOL_R14              99
#define CV2X_RRC_MAX_ZONE_ID_R14                          7
#define CV2X_RRC_MAX_THRESH_S_RSSI_CBR_R14                45
#define CV2X_RRC_MAX_CBR_LEVEL_1_R14                      15
#define CV2X_RRC_MAX_SL_V2X_CBRCONFIG2_1_R14              7
#define CV2X_RRC_MIN_PRIORITY_THRESHOLD_R14               1
#define CV2X_RRC_MAX_PRIORITY_THRESHOLD_R14               8
#define CV2X_RRC_MAX_SL_V2X_TXCONFIG2_1_R14               127

/* List index by frequency */
#define CV2X_RRC_V2X_DEFAULT_FREQ_LIST_INDEX              0

/* Values needed for converting frequency */
#define CV2X_RRC_FREQ_LOWEST_BAND46                       5105
#define CV2X_RRC_EARFCN_CALC_OFFSET_BAND46                46790

#define CV2X_RRC_FREQ_LOWEST_BAND47                       5855
#define CV2X_RRC_EARFCN_CALC_OFFSET_BAND47                54540

#define CV2X_RRC_FREQ_LOWEST                              CV2X_RRC_FREQ_LOWEST_BAND46

/* Currently we support two consecutive bands*/
/* 46790 - 54539 ==> Band 46, 54540 - 55239 ==> Band 47 */
#define CV2X_RRC_EARFCN_IS_BAND_46(rrc_freq)              ((rrc_freq) < CV2X_RRC_EARFCN_CALC_OFFSET_BAND47)
#define CV2X_RRC_FREQ_IS_BAND_46(cv2x_freq)               ((cv2x_freq) < CV2X_RRC_FREQ_LOWEST_BAND47)

/* Converting RRC frequency value to MHz units (and vice versa) is done according to TS136.101@5.7.3.*/
/* May be used after validating values. */
#define RRC_FREQ_TO_CV2X_FREQ(rrc_freq)                  (CV2X_RRC_EARFCN_IS_BAND_46(rrc_freq) ? \
  	                                                     (CV2X_RRC_FREQ_LOWEST_BAND46 + (((rrc_freq) - CV2X_RRC_EARFCN_CALC_OFFSET_BAND46) / 10)) : \
  	                                                     (CV2X_RRC_FREQ_LOWEST_BAND47 + (((rrc_freq) - CV2X_RRC_EARFCN_CALC_OFFSET_BAND47) / 10)) )

#define CV2X_FREQ_TO_RRC_FREQ(cv2x_freq)                 (CV2X_RRC_FREQ_IS_BAND_46(cv2x_freq) ? \
                                                         ((10 * ((cv2x_freq) - CV2X_RRC_FREQ_LOWEST_BAND46)) + CV2X_RRC_EARFCN_CALC_OFFSET_BAND46) : \
                                                         ((10 * ((cv2x_freq) - CV2X_RRC_FREQ_LOWEST_BAND47)) + CV2X_RRC_EARFCN_CALC_OFFSET_BAND47) )

/* Converting RRC threshold for RSSI CBR to DBM units is done according to TS 36.214 [48]. */
/* Value 0 corresponds to -112 dBm, value 1 to -110 dBm, value n to (-112 + n*2) dBm, and so on. */
#define RRC_RSSI_THRESHOLD_TO_DBM(rrc_rssi_threshold)    (-112 + ((rrc_rssi_threshold) * 2))


/* ------------------------ Enumerations ------------------------- */


/** Bandwidth in number of Resource Blocks */
typedef enum {
  CV2X_RRC_BANDWIDTH_N6 = 0,
  CV2X_RRC_BANDWIDTH_N15,
  CV2X_RRC_BANDWIDTH_N25,
  CV2X_RRC_BANDWIDTH_N50,
  CV2X_RRC_BANDWIDTH_N75,
  CV2X_RRC_BANDWIDTH_N100,

  CV2X_RRC_BANDWIDTH_MAX = CV2X_RRC_BANDWIDTH_N100

} cv2x_rrc_sl_bandwidth_t;


/** Sub-channel size in number of Resource Blocks */
typedef enum {
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N4 = 0,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N5,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N6,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N8,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N9,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N10,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N12,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N15,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N16,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N18,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N20,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N25,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N30,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N48,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N50,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N72,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N75,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N96,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_N100,

  CV2X_RRC_SUBCHANNEL_SIZE_R14_MAX = CV2X_RRC_SUBCHANNEL_SIZE_R14_N100,

  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE13,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE12,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE11,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE10,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE9,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE8,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE7,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE6,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE5,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE4,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE3,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE2,
  CV2X_RRC_SUBCHANNEL_SIZE_R14_SPARE1,

} cv2x_rrc_subchannel_size_r14_t;


/** Number of sub-channels */
typedef enum {
  CV2X_RRC_SUBCHANNEL_NUM_R14_N1 = 0,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N3,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N5,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N8,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N10,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N15,
  CV2X_RRC_SUBCHANNEL_NUM_R14_N20,

  CV2X_RRC_SUBCHANNEL_NUM_R14_MAX = CV2X_RRC_SUBCHANNEL_NUM_R14_N20,

  CV2X_RRC_SUBCHANNEL_NUM_R14_SPARE1

} cv2x_rrc_subchannel_num_r14_t;


/** TDD ( Time-Division Duplex) physical channel configuration */
typedef enum {
  CV2X_RRC_SFRAME_ASSIGNMENT_NONE = 0,
  CV2X_RRC_SFRAME_ASSIGNMENT_0,
  CV2X_RRC_SFRAME_ASSIGNMENT_1,
  CV2X_RRC_SFRAME_ASSIGNMENT_2,
  CV2X_RRC_SFRAME_ASSIGNMENT_3,
  CV2X_RRC_SFRAME_ASSIGNMENT_4,
  CV2X_RRC_SFRAME_ASSIGNMENT_5,
  CV2X_RRC_SFRAME_ASSIGNMENT_6,

  CV2X_RRC_SFRAME_ASSIGNMENT_MAX = CV2X_RRC_SFRAME_ASSIGNMENT_6

} cv2x_rrc_tdd_config_sl_r12_t;


/** Measurement filtering coefficient */
typedef enum {
  CV2X_RRC_FILTER_COEFF_0 = 0,
  CV2X_RRC_FILTER_COEFF_1,
  CV2X_RRC_FILTER_COEFF_2,
  CV2X_RRC_FILTER_COEFF_3,
  CV2X_RRC_FILTER_COEFF_4,
  CV2X_RRC_FILTER_COEFF_5,
  CV2X_RRC_FILTER_COEFF_6,
  CV2X_RRC_FILTER_COEFF_7,
  CV2X_RRC_FILTER_COEFF_8,
  CV2X_RRC_FILTER_COEFF_9,
  CV2X_RRC_FILTER_COEFF_11,
  CV2X_RRC_FILTER_COEFF_13,
  CV2X_RRC_FILTER_COEFF_15,
  CV2X_RRC_FILTER_COEFF_17,
  CV2X_RRC_FILTER_COEFF_19,

  CV2X_RRC_FILTER_COEFF_MAX = CV2X_RRC_FILTER_COEFF_19,

  CV2X_RRC_FILTER_COEFF_SPARE1

} cv2x_rrc_filter_coefficient_t;


/** Hysteresis when evaluating a sync reference UE using absolute comparison */
typedef enum {
  CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB0 = 0,
  CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB3,
  CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB6,
  CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB9,
  CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB12,

  CV2X_RRC_SYNC_REF_MIN_HYST_R14_MAX = CV2X_RRC_SYNC_REF_MIN_HYST_R14_dB12

} cv2x_rrc_sync_ref_min_hyst_r14_t;


/** Hysteresis when evaluating a sync reference UE using relative comparison */
typedef enum {
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dB0 = 0,
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dB3,
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dB6,
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dB9,
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dB12,
  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dBinf,

  CV2X_RRC_SYNC_REF_DIFF_HYST_R14_MAX = CV2X_RRC_SYNC_REF_DIFF_HYST_R14_dBinf

} cv2x_rrc_sync_ref_diff_hyst_r14_t;


/** RSVP value */
typedef enum {
  /** 20 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V0DOT2 = 0,

  /** 50 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V0DOT5,

  /** 100 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V1,

  /** 200 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V2,

  /** 300 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V3,

  /** 400 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V4,

  /** 500 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V5,

  /** 600 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V6,

  /** 700 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V7,

  /** 800 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V8,

  /** 900 ms */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V9,

  /** 1 second */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_V10,

  /** Maximum enumeration value */
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_MAX = CV2X_RRC_RESTRICT_RR_PERIOD_R14_V10,

  CV2X_RRC_RESTRICT_RR_PERIOD_R14_SPARE4,
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_SPARE3,
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_SPARE2,
  CV2X_RRC_RESTRICT_RR_PERIOD_R14_SPARE1,

} cv2x_rrc_sl_restrict_rr_period_r14_t;


/** Synchronization reference type */
typedef enum {
  CV2X_RRC_TYPE_TX_SYNC_R14_GNSS = 0,
  CV2X_RRC_TYPE_TX_SYNC_R14_ENB,
  CV2X_RRC_TYPE_TX_SYNC_R14_UE,

  CV2X_RRC_TYPE_TX_SYNC_R14_MAX = CV2X_RRC_TYPE_TX_SYNC_R14_UE

} cv2x_rrc_type_tx_sync_r14_t;


/** Speed threshold */
typedef enum {
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH60 = 0,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH80,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH100,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH120,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH140,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH160,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH180,
  CV2X_RRC_THRESH_UE_SPEED_R14_KMPH200,

  CV2X_RRC_THRESH_UE_SPEED_R14_MAX = CV2X_RRC_THRESH_UE_SPEED_R14_KMPH200

} cv2x_rrc_thres_ue_speed_r14_t;


/** Re-transmissions mode */
typedef enum {
  /** No re-transmission */
  CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_N0 = 0,

  /** One re-transmission */
  CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_N1,

  /** UE may autonomously select no re-transmission or one re-transmission */
  CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_BOTH,

  /** Maximum enumeration value */
  CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_MAX = CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_BOTH,

  CV2X_RRC_ALLOWED_RETX_NUMBER_PSSCH_R14_SPARE1,

} cv2x_rrc_allowed_retx_number_pssch_r14_t;


/** Probability for keeping selected resources */
typedef enum {
  /** 0%: No reservation keep */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0 = 0,

  /** 20% */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0DOT2,

  /** 40% */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0DOT4,

  /** 60% */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0DOT6,

  /** 80% */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0DOT8,

  /** Maximum enumeration value */
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_MAX = CV2X_RRC_PROB_RESOURCE_KEEP_R14_V0DOT8,

  CV2X_RRC_PROB_RESOURCE_KEEP_R14_SPARE3,
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_SPARE2,
  CV2X_RRC_PROB_RESOURCE_KEEP_R14_SPARE1,

} cv2x_rrc_prob_resource_keep_r14_t;


/** Number of skipped transmissions that will trigger resource re-selection */
typedef enum {
  CV2X_RRC_RESELECT_AFTER_R14_N1 = 0,
  CV2X_RRC_RESELECT_AFTER_R14_N2,
  CV2X_RRC_RESELECT_AFTER_R14_N3,
  CV2X_RRC_RESELECT_AFTER_R14_N4,
  CV2X_RRC_RESELECT_AFTER_R14_N5,
  CV2X_RRC_RESELECT_AFTER_R14_N6,
  CV2X_RRC_RESELECT_AFTER_R14_N7,
  CV2X_RRC_RESELECT_AFTER_R14_N8,
  CV2X_RRC_RESELECT_AFTER_R14_N9,

  CV2X_RRC_RESELECT_AFTER_R14_MAX = CV2X_RRC_RESELECT_AFTER_R14_N9,

  CV2X_RRC_RESELECT_AFTER_R14_SPARE7,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE6,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE5,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE4,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE3,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE2,
  CV2X_RRC_RESELECT_AFTER_R14_SPARE1,

} cv2x_rrc_sl_reselect_after_r14_t;


/** Synchronization source (used for sync source priority) */
typedef enum {
  CV2X_RRC_SYNC_PRIORITY_R14_GNSS = 0,
  CV2X_RRC_SYNC_PRIORITY_R14_ENB,

  CV2X_RRC_SYNC_PRIORITY_R14_MAX = CV2X_RRC_SYNC_PRIORITY_R14_ENB

} cv2x_rrc_sync_priority_r14_t;


/** Geographic zone length / width in meters */
typedef enum {
  CV2X_RRC_ZONE_LEN_WID_M5 = 0,
  CV2X_RRC_ZONE_LEN_WID_M10,
  CV2X_RRC_ZONE_LEN_WID_M20,
  CV2X_RRC_ZONE_LEN_WID_M50,
  CV2X_RRC_ZONE_LEN_WID_M100,
  CV2X_RRC_ZONE_LEN_WID_M200,
  CV2X_RRC_ZONE_LEN_WID_M500,

  CV2X_RRC_ZONE_LEN_WID_MAX = CV2X_RRC_ZONE_LEN_WID_M500,

  CV2X_RRC_ZONE_LEN_WID_SPARE1,

} cv2x_rrc_zone_len_wid_t;


/** Length of resource pool bitmap in number of bits */
typedef enum {
  CV2X_RRC_BIT_STRING_BITS_10 = 0,
  CV2X_RRC_BIT_STRING_BITS_16,
  CV2X_RRC_BIT_STRING_BITS_20,
  CV2X_RRC_BIT_STRING_BITS_30,
  CV2X_RRC_BIT_STRING_BITS_40,
  CV2X_RRC_BIT_STRING_BITS_50,
  CV2X_RRC_BIT_STRING_BITS_60,
  CV2X_RRC_BIT_STRING_BITS_100,

  CV2X_RRC_BIT_STRING_BITS_MAX = CV2X_RRC_BIT_STRING_BITS_100

} cv2x_rrc_subframe_bitmap_sl_t;


/** Used as a flag to mark max_tx_power_r14 value interpretation */
typedef enum {
  /** Ignore max_tx_power_r14 value as it is not available in RRC Pre-Configuration */
  CV2X_RRC_MAX_TX_POWER_STATUS_ABSENT = 0,

  /** Value of -Infinity */
  CV2X_RRC_MAX_TX_POWER_STATUS_MINUS_INFINITY,

  /** Valid power value in dBm */
  CV2X_RRC_MAX_TX_POWER_STATUS_VALUE,

  /** Maximum enumeration value */
  CV2X_RRC_MAX_TX_POWER_STATUS_MAX = CV2X_RRC_MAX_TX_POWER_STATUS_VALUE

} cv2x_rrc_max_tx_power_status_t;


/** Used as a flag to mark sl_offset_indicator_r14 value interpretation */
typedef enum {
  /** Ignore sl_offset_indicator_r14 value as it is not available in RRC Pre-Configuration */
  CV2X_SL_OFFSET_INDICATOR_STATUS_ABSENT = 0,

  /** sl_offset_indicator_r14 value is in range of 0 - 319 */
  CV2X_SL_OFFSET_INDICATOR_STATUS_SMALL,

  /** sl_offset_indicator_r14 value is in range of 0 - 10239 */
  CV2X_SL_OFFSET_INDICATOR_STATUS_LARGE,

  /** Maximum enumeration value */
  CV2X_SL_OFFSET_INDICATOR_STATUS_MAX = CV2X_SL_OFFSET_INDICATOR_STATUS_LARGE

} cv2x_sl_offset_indicator_status_t;


/* -------------------------- Structures ------------------------- */


/* Level-4 IE structures */
/* --------------------- */

/** Tx parameters */
typedef struct cv2x_rrc_sl_pssch_tx_parameters_r14 {
  uint8_t                                  min_mcs_pssch_r14;                // Mandatory | Range: 0 - 31
  uint8_t                                  max_mcs_pssch_r14;                // Mandatory | Range: 0 - 31
  uint8_t                                  min_sub_channel_number_pssch_r14; // Mandatory | Range: 0 - 20
  uint8_t                                  max_sub_channel_number_pssch_r14; // Mandatory | Range: 0 - 20
  cv2x_rrc_allowed_retx_number_pssch_r14_t allowed_retx_number_pssch_r14;    // Mandatory
  cv2x_rrc_max_tx_power_status_t           max_tx_power_status;              // Flag to mark the next field interpretation
  int8_t                                   max_tx_power_r14;                 // Optional  | Range: -41 - 31

} cv2x_rrc_sl_pssch_tx_parameters_r14_t;


/* Level-3 IE structures */
/* --------------------- */

/** PDCP ROHC (Robust Header Compression) Profiles-r12 */
typedef struct cv2x_rrc_rohc_profiles_r12 {
  bool_t profile0x0001_r12; // Mandatory
  bool_t profile0x0002_r12; // Mandatory
  bool_t profile0x0004_r12; // Mandatory
  bool_t profile0x0006_r12; // Mandatory
  bool_t profile0x0101_r12; // Mandatory
  bool_t profile0x0102_r12; // Mandatory
  bool_t profile0x0104_r12; // Mandatory

} cv2x_rrc_rohc_profiles_r12_t;


/** Sync offset indicators */
typedef struct cv2x_rrc_sync_offset_indicators_r14 {
  uint8_t sync_offset_indicator1_r14;           // Mandatory | Range: 0 - 159
  uint8_t sync_offset_indicator2_r14;           // Mandatory | Range: 0 - 159
  bool_t    is_sync_offset_indicator3_available;  // Flag to mark whether next field is available
  uint8_t sync_offset_indicator3_r14;           // Optional  | Range: 0 - 159

} cv2x_rrc_sync_offset_indicators_r14_t;


/** P2X related resource selection mechanism */
typedef struct cv2x_rrc_sl_p2x_resource_selection_config_r14 {
  bool_t is_partial_sensing_available;  // Flag to mark whether next field is available
  bool_t partial_sensing_r14;           // Optional
  bool_t is_random_selection_available; // Flag to mark whether next field is available
  bool_t random_selection_r14;          // Optional

} cv2x_rrc_sl_p2x_resource_selection_config_r14_t;


/** Sync references allowed to use the associated pool */
typedef struct cv2x_rrc_sl_sync_allowed_r14 {
  bool_t is_gnss_sync_available;  // Flag to mark whether next field is available
  bool_t gnss_sync_r14;           // Optional
  bool_t is_enb_sync_available;   // Flag to mark whether next field is available
  bool_t enb_sync_r14;            // Optional
  bool_t is_ue_sync_available;    // Flag to mark whether next field is available
  bool_t ue_sync_r14;             // Optional

} cv2x_rrc_sl_sync_allowed_r14_t;


/** CBR-to-PPPP mappings */
typedef struct cv2x_rrc_sl_cbr_pppp_tx_pre_config_list_r14 {
  uint8_t   priority_threshold_r14;        // Mandatory | Range: 1 - 8
  uint32_t  default_tx_config_index_r14;   // Mandatory | Range: 0 - 15
  uint32_t  cbr_config_index_r14;          // Mandatory | Range: 0 - 7

  uint8_t   *tx_config_index_list_r14_ptr; // Mandatory
  uint32_t  tx_config_index_list_r14_len;  // Length: 1 - 16 [CBR_LEVEL_R14]

} cv2x_rrc_sl_cbr_pppp_tx_pre_config_list_r14_t;


/** Tx configuration per Tx sync source */
typedef struct sl_pssch_tx_config_list_r14_t
{
  bool_t                                  is_type_tx_sync_available;  // Flag to mark whether next field is available
  cv2x_rrc_type_tx_sync_r14_t           type_tx_sync_r14;           // Optional
  cv2x_rrc_thres_ue_speed_r14_t         thres_ue_speed_r14;         // Mandatory
  cv2x_rrc_sl_pssch_tx_parameters_r14_t parameters_above_thres_r14; // Mandatory
  cv2x_rrc_sl_pssch_tx_parameters_r14_t parameters_below_thres_r14; // Mandatory

} cv2x_rrc_sl_pssch_tx_config_list_r14_t;


/** P2X sensing configuration */
typedef struct cv2x_rrc_p2x_sensing_config_r14 {
  uint8_t  min_num_candidate_sf_r14;   // Mandatory | Range: 1 - 13
  uint16_t gap_candidate_sensing_r14;  // Mandatory | Valid bits: 0 - 9

} cv2x_rrc_p2x_sensing_config_r14_t;


/* Level-2 IE structures */
/* --------------------- */

/** General configuration */
typedef struct cv2x_rrc_sl_pre_config_general_r12 {

  /* PDCP ROHC profiles configuration */
  cv2x_rrc_rohc_profiles_r12_t rohc_profiles_r12;                   // Mandatory

  /* Physical configuration */
  uint32_t                     carrier_freq_r12;                    // Mandatory | Range: 0 - 262143
  int8_t                       max_tx_power_r12;                    // Mandatory | Range: -30 - 33
  uint8_t                      additional_spectrum_emission_r12;    // Mandatory | Range: 1 - 32
  cv2x_rrc_sl_bandwidth_t      sl_bandwidth_r12;                    // Mandatory
  cv2x_rrc_tdd_config_sl_r12_t tdd_config_sl_r12;                   // Mandatory
  uint32_t                     reserved_r12;                        // Mandatory | Valid bits: 0 - 18
  bool_t                         is_add_spec_emiss_v10l0_available;   // Flag to mark whether next field is available
  uint16_t                     additional_spectrum_emission_v10l0;  // Optional  | Range: 33 - 288

} cv2x_rrc_sl_pre_config_general_r12_t;


/** Sync configuration */
typedef struct cv2x_rrc_sl_pre_config_v2x_sync_r14 {
  cv2x_rrc_sync_offset_indicators_r14_t sl_v2x_sync_offset_indicators_r14;  // Mandatory
  int32_t                               sync_tx_parameters_r14;             // Mandatory | Range: -126 - 31
  uint8_t                               sync_tx_thresh_ooc_r14;             // Mandatory | Range: 0 - 11
  cv2x_rrc_filter_coefficient_t         filter_coefficient_r14;             // Mandatory
  cv2x_rrc_sync_ref_min_hyst_r14_t      sync_ref_min_hyst_r14;              // Mandatory
  cv2x_rrc_sync_ref_diff_hyst_r14_t     sync_ref_diff_hyst_r14;             // Mandatory

} cv2x_rrc_sl_pre_config_v2x_sync_r14_t;


/** Pool configuration (common for V2X-Tx / V2X-Rx / P2X-Tx pools) */
typedef struct cv2x_rrc_sl_pre_config_common_pool_r14 {
  cv2x_sl_offset_indicator_status_t               sl_offset_indicator_status;             // Flag to mark the next field interpretation
  uint16_t                                        sl_offset_indicator_r14;                // Optional | Range: 0 - 319 for small; 0 - 10239 for large
  cv2x_rrc_subframe_bitmap_sl_t                   sl_subframe_size;                       // Flag to mark the next field interpretation
  uint8_t                                         sl_subframe_r14[CV2X_RRC_POOL_BMP_LEN]; // Mandatory
  bool_t                                            adjacency_pscch_pscch_r14;              // Mandatory
  cv2x_rrc_subchannel_size_r14_t                  size_subchannel_r14;                    // Mandatory
  cv2x_rrc_subchannel_num_r14_t                   num_subchannel_r14;                     // Mandatory
  uint8_t                                         start_rb_subchannel_r14;                // Mandatory | Range: 0 - 99
  bool_t                                            is_start_rb_pscch_pool_available;       // Flag to mark whether next field is available
  uint8_t                                         start_rb_pscch_pool_r14;                // Optional  | Range: 0 - 99
  int8_t                                          data_tx_params_r14;                     // Mandatory | Range: -126 - 31
  bool_t                                            is_zone_id_available;                   // Flag to mark whether next field is available
  uint8_t                                         zone_id_r14;                            // Optional  | Range: 0 - 7
  bool_t                                            is_thresh_s_rssi_cbr_available;         // Flag to mark whether next field is available
  uint8_t                                         thresh_s_rssi_cbr_r14;                  // Optional  | Range: 0 - 45

  cv2x_rrc_sl_cbr_pppp_tx_pre_config_list_r14_t   *cbr_pssch_tx_config_list_r14_ptr;      // Optional
  uint32_t                                        cbr_pssch_tx_config_list_r14_len;       // Length: 1 - 8 [CBR_PSSCH_TX_CONFIG]

  bool_t                                            is_p2x_rsrc_select_cfg_available;       // Flag to mark whether next field is available
  cv2x_rrc_sl_p2x_resource_selection_config_r14_t sl_p2x_resource_selection_config_r14;   // Optional
  bool_t                                            is_sync_allowed_available;              // Flag to mark whether next field is available
  cv2x_rrc_sl_sync_allowed_r14_t                  sl_sync_allowed_r14;                    // Optional

  cv2x_rrc_sl_restrict_rr_period_r14_t            *sl_restrict_rr_period_list_r14_ptr;    // Optional
  uint32_t                                        sl_restrict_rr_period_list_r14_len;     // Length: 1 - 16 [RESERVATION_PERIOD_R14]

} cv2x_rrc_sl_pre_config_common_pool_r14_t;


/** Pool sensing configuration used for resource selection */
typedef struct cv2x_rrc_sl_comm_tx_pool_sensing_config_r14 {

  cv2x_rrc_sl_pssch_tx_config_list_r14_t *pssch_tx_config_list_r14_ptr;     // Mandatory
  uint32_t                               pssch_tx_config_list_r14_len;      // Length: 1 - 16 [PSSCH_TX_CONFIG_R14]

  uint8_t                                thres_pssch_rsrp_list_r14_arr[CV2X_RRC_THRES_PSSCH_RSRP_LIST_LEN]; // Mandatory

  cv2x_rrc_sl_restrict_rr_period_r14_t   *restrict_rr_period_list_r14_ptr;  // Optional
  uint32_t                               restrict_rr_period_list_r14_len;   // Length: 1 - 16 [RESERVATION_PERIOD_R14]

  cv2x_rrc_prob_resource_keep_r14_t      prob_resource_keep_r14;            // Mandatory
  bool_t                                   is_p2x_sensing_config_available;   // Flag to mark whether next field is available
  cv2x_rrc_p2x_sensing_config_r14_t      p2x_sensing_config_r14;            // Optional
  bool_t                                   is_reselect_after_available;       // Flag to mark whether next field is available
  cv2x_rrc_sl_reselect_after_r14_t       reselect_after_r14;                // Optional

} cv2x_rrc_sl_comm_tx_pool_sensing_config_r14_t;


/** Zone configuration used for resource selection */
typedef struct cv2x_rrc_sl_zone_config_r14 {

  cv2x_rrc_zone_len_wid_t zone_length_r14;        // Mandatory
  cv2x_rrc_zone_len_wid_t zone_width_r14;         // Mandatory
  uint8_t                 zone_id_longi_mod_r14;  // Mandatory | Range: 1 - 4
  uint8_t                 zone_id_lati_mod_r14;   // Mandatory | Range: 1 - 4

} cv2x_rrc_sl_zone_config_r14_t;


/** CBR ranges */
typedef struct cv2x_rrc_sl_cbr_levels_config_list_r14 {
  uint8_t  *cbr_level_list_r14_ptr; // Mandatory
  uint32_t cbr_level_list_r14_len;  // Length: 1 - 16 [CBR_LEVEL_R14]

} cv2x_rrc_sl_cbr_levels_config_list_r14_t;


/** Tx parameters per CBR range */
typedef struct cv2x_rrc_sl_cbr_pssch_tx_config_r14 {
  uint16_t                              cr_limit_r14;             // Mandatory | Range: 0 - 10000
  cv2x_rrc_sl_pssch_tx_parameters_r14_t pssch_tx_parameters_r14;  // Mandatory

} cv2x_rrc_sl_cbr_pssch_tx_config_r14_t;



/* Level-1 IE structures */
/* --------------------- */

/** RRC Pre-Configuration per frequency */
typedef struct cv2x_rrc_sl_v2x_pre_config_freq_list_r14 {
  cv2x_rrc_sl_pre_config_general_r12_t          v2x_comm_pre_config_general_r14;   // Mandatory
  bool_t                                          is_v2x_pcfg_v2x_sync_available;    // Flag to mark whether next field is available
  cv2x_rrc_sl_pre_config_v2x_sync_r14_t         v2x_comm_pre_config_v2x_sync_r14;  // Optional

  cv2x_rrc_sl_pre_config_common_pool_r14_t      *v2x_common_rx_pool_list_r14_ptr;  // Mandatory
  uint32_t                                      v2x_common_rx_pool_list_r14_len;   // Length: 1 - 16 [SL_V2X_RX_POOL_PRECONF_R14]

  cv2x_rrc_sl_pre_config_common_pool_r14_t      *v2x_common_tx_pool_list_r14_ptr;  // Mandatory
  uint32_t                                      v2x_common_tx_pool_list_r14_len;   // Length: 1 - 8 [SL_V2X_TX_POOL_PRECONF_R14]

  cv2x_rrc_sl_pre_config_common_pool_r14_t      *p2x_common_tx_pool_list_r14_ptr;  // Mandatory
  uint32_t                                      p2x_common_tx_pool_list_r14_len;   // Length: 1 - 8 [SL_V2X_TX_POOL_PRECONF_R14]

  bool_t                                          is_v2x_rsrc_select_cfg_available;  // Flag to mark whether next field is available
  cv2x_rrc_sl_comm_tx_pool_sensing_config_r14_t v2x_resource_selection_config_r14; // Optional [by standard]
                                                                                   // Forcing its presence in our solution [Autonomous Resource Selection (Mode4)].

  bool_t                                          is_zone_config_available;          // Flag to mark whether next field is available
  cv2x_rrc_sl_zone_config_r14_t                 zone_config_r14;                   // Optional
  cv2x_rrc_sync_priority_r14_t                  sync_priority_r14;                 // Mandatory
  bool_t                                          is_thres_sl_tx_prio_available;     // Flag to mark whether next field is available
  uint8_t                                       thres_sl_tx_prioritization_r14;    // Optional | Range: 1 - 8
  bool_t                                          is_offset_dfn_available;           // Flag to mark whether next field is available
  uint16_t                                      offset_dfn_r14;                    // Optional | Range: 0 - 1000

} cv2x_rrc_sl_v2x_pre_config_freq_list_r14_t;


/** CBR ranges and Tx configuration */
typedef struct cv2x_rrc_sl_cbr_pre_config_tx_config_list_r14 {
  cv2x_rrc_sl_cbr_levels_config_list_r14_t *cbr_range_common_config_list_r14_ptr; // Mandatory
  uint32_t                                 cbr_range_common_config_list_r14_len;  // Length: 1 - 8 [SL_V2X_CBR_CONFIG2_R14]

  cv2x_rrc_sl_cbr_pssch_tx_config_r14_t    *sl_cbr_pssch_tx_config_list_r14_ptr;  // Mandatory
  uint32_t                                 sl_cbr_pssch_tx_config_list_r14_len;   // Length: 1 - 128 [SL_V2X_TX_CONFIG2_R14]

} cv2x_rrc_sl_cbr_pre_config_tx_config_list_r14_t;


/* Level-0 IE structures */
/* --------------------- */

/** RRC Pre-Configuration root structure */
typedef struct cv2x_rrc_pre_config {
  cv2x_rrc_sl_v2x_pre_config_freq_list_r14_t      *v2x_pre_config_freq_list_r14_ptr;  // Mandatory
  uint32_t                                        v2x_pre_config_freq_list_r14_len;   // Length: 1 - 8 [FREQ_V2X_R14]

  uint32_t                                        *anchor_carrier_freq_list_r14_ptr;  // Optional
  uint32_t                                        anchor_carrier_freq_list_r14_len;   // Length: 1 - 8 [FREQ_V2X_R14]

  bool_t                                            is_cbr_pre_config_available;        // Flag to mark whether next field is available
  cv2x_rrc_sl_cbr_pre_config_tx_config_list_r14_t cbr_pre_config_list_r14;            // Optional
                                                                                      // (This is not a list. The "list" in the
                                                                                      // type / name is taken from the standard)

} cv2x_rrc_pre_config_t;

/** CV2X US SAE configuration parameters */
typedef struct cv2x_mcs_tbs_us_sae_config_st {
  /** TBS thresholds for SPS transmission below speed threshold for US SAE profile */
  uint16_t   mcs_tbs_us_sae_sps_below_thresh_tbs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

  /** MCS values per TBS for SPS transmission below speed threshold for US SAE profile */
  cv2x_mcs_t mcs_tbs_us_sae_sps_below_thresh_mcs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

  /** TBS thresholds for AD-HOC transmission below speed threshold for US SAE profile */
  uint16_t   mcs_tbs_us_sae_adhoc_below_thresh_tbs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

  /** MCS values per TBS for AD-HOC transmission below speed threshold for US SAE profile */
  cv2x_mcs_t mcs_tbs_us_sae_adhoc_below_thresh_mcs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

  /** TBS thresholds for SPS / AD-HOC transmission above speed threshold for US SAE profile */
  uint16_t   mcs_tbs_us_sae_above_thresh_tbs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

  /** MCS values per TBS for SPS / AD-HOC transmission above speed threshold for US SAE profile */
  cv2x_mcs_t mcs_tbs_us_sae_above_thresh_mcs[CV2X_US_SAE_TBS_MCS_MAX_NUM_ELEMS];

} cv2x_mcs_tbs_us_sae_config_t;

/** CV2X configuration parameters */
typedef struct cv2x_configuration_st {
  /** {0,1,2,3,4} cyclic shift, in samples */
  int32_t                      lmac_tx_csd;

  /** {0,1} default value 0. */
  int32_t                      lmac_pass_crc_fail_packets;

  /** lmac threshold for passing measurements. units - DB. only measurements with higher narrow band rssi shall pass to host */
  int32_t                      lmac_measurements_rssi_threshold_for_sci;

  /** {0 - 10239} Milliseconds to add to RRC offset_dfn when calculating DFN / SFN */
  uint32_t                     cv2x_hack_offset_dfn_ms;

  /** Message lease time in milliesecs */
  uint32_t                     message_tx_lease_time_ms;

  /** CV2X RR configuration */
  int32_t                      cv2x_rr_prob_resource_keep_always;

  /** CV2X MAC packet processing time */
  int32_t                      cv2x_rr_mac_time_ahead_usec;

  /** Resource reservation processing time */
  int32_t                      cv2x_rr_min_time_ahead_usec;

  /** Maximum gap (in milliesec) allowed for HARQ. 0 to disable HARQ */
  int32_t                      cv2x_rr_harq_window_subframes;

  /** The minimum gap allowed between Tx subframes */
  int32_t                      cv2x_rr_dsp_min_subframes_diff;

  /**  */
  int32_t                      cv2x_rr_missing_measurements_due_tx;

  /** One Shot range configuration */
  int32_t                      cv2x_rr_min_one_shot_counter;

  int32_t                      cv2x_rr_max_one_shot_counter;

  /** Single Measurement Processing Time (SMPT) in msec */
  uint32_t                     smpt_msec;

  /** CV2X dispatcher configuration */
  int32_t                      cv2x_dispatcher_time_ahead_usec;

  /** Maximum drift time allowed before resetting the RR, in milliesec */
  uint32_t                     cv2x_time_sync_rr_reset_gap_ms;

  /** MAC-internal clock sync interval in milliesec */
  uint32_t                     cv2x_time_sync_polling_interval_ms;

  /** DSP packet processing time */
  int32_t                      cv2x_lmac_time_ahead_usec;

  /** Enable/Disable measurements indication from device to host (0 / 1) */
  int32_t                      cv2x_lmac_measurements;

  /**
     Device logging flags:
     0x001 - Tx application level
     0x002 - Tx device confirmation
     0x004 - Tx device LMAC
     0x008 - Tx device IRQ
     0x010 - Tx / Rx host status
     0x100 - Rx application level
     0x200 - Rx device reception
     0x400 - Rx device measurement
   */
  uint32_t                     cv2x_lmac_verbose;

  /**
     DSP flags:
     0x040 - Force subframe number zero on Rx
     0x080 - Force subframe number zero on Tx
     0x100 - Tx only mode
     0x200 - Disable RSSI sorting feature
     0x400 - Decode HARQ second frame as well, if first was decoded successfully
     0x800 - Disable High Doppler feature
  */
  uint32_t                     cv2x_lmac_flags;

  /** Disable Tx in MAC layer (1 disable, 0 to enable) */
  int32_t                      cv2x_rx_only_mode;

  /** Enable Non-IP header in case of Non-IP traffic (0 - disabled, non 0 - enabled) */
  uint32_t                     cv2x_enable_non_ip_header;

  /**
     Subchannel bitmap MCS for single transmission (no HARQ).
     Will be used if no value different than 0x0 will be supplied by user via cv2x_sw_config.txt file.
     If even one value is supplied by user, it will be used, and the rest of the values (that were not supplied)
     will be set to 0x0.
  */
  uint32_t                     cv2x_subchannel_bitmap_mcs_single[CV2X_MCS_MAX + 1];

  /**
     Subchannel bitmap MCS for HARQ transmission.
     Will be used if no value different than 0x0 will be supplied by user via cv2x_sw_config.txt file
     for HARQ transmission OR for single transmission.
     If even one value is supplied by user, it will be used, and the rest of the values (that were not supplied)
     will be set to 0x0.
     If no value was supplied by user, but values for cv2x_subchannel_bitmap_mcs_single were supplied, values for
     cv2x_subchannel_bitmap_mcs_single shall be used for cv2x_subchannel_bitmap_mcs_harq as well.
  */
  uint32_t                     cv2x_subchannel_bitmap_mcs_harq[CV2X_MCS_MAX + 1];

  /** WDM diversity mode */
  int32_t                      cv2x_diversity_mode;

  /** Packet Delay Budget (PDB) by priority */
  uint32_t                     cv2x_pdb_by_pppp_array[CV2X_PPPP_MAX + 1];

  /**  */
  uint32_t                     max_cr_future_subframes;

  /** Logs related configuration */
  uint32_t                     cv2x_customer_cbr_log_gap_ms;

  /**
     Rx L2ID filtering mode:
        0 - unfiltered (default), all L2IDs are forwarded to the application layer
        1 - filtered, only device's own source L2ID and broadcast L2ID are forwarded
  */
  int32_t                      rx_l2id_filtering_mode;

  /** CV2X max allowed Tx sockets */
  uint32_t                     cv2x_max_tx_sockets;

  /** CV2X max allowed SPS Tx sockets */
  uint32_t                     cv2x_max_tx_sps_sockets;

  /** RR max allowed policies */
  uint32_t                     cv2x_rr_max_policies;

  /** RR minimum measurements required before operation */
  uint32_t                     cv2x_rr_measurements_threshold_time_usec;

  /** Codebook index according to precoding matrix. {0,1,2,3,4,5,6,7}. 6, 7 are combinations of the 0-5. per carrier per pool */
  uint8_t                      cv2x_tx_codebook_idx_per_carrier_per_pool[CV2X_CARRIER_POOL_MAX_NUM_CARRIERS][CV2X_CARRIER_POOL_MAX_NUM_TX_POOLS];

  /** log statistics dump period */
  uint32_t                     log_stats_period_ms;

  /** Max Tx power offset in dBm8, {-5 ... +5 dB} in 0.5dB steps. Default value: 0dB. Will be translated into dBm2 for DSP use later on. */
  int8_t                       max_tx_power_offset_dbm8;

  /**
     Limit log prints to avoid log flooding.
     0 - disable.
     Any other positive number - number of past prints (records) to keep for rate limiting.
     Default - 2 records
  */
  uint16_t                     logger_rate_limit;

  /**
     Logger rate limit time threshold in milliseconds. Max threshold between last print and current time.
     0 - disable.
     Default value is 1,000 ms (1 second).
  */
  uint32_t                     logger_rate_limit_time_threshold_ms;

  /**
     Maximal log_level_t for which to apply logger rate limit.
     3 - LOG_LEVEL_ERROR
     4 - LOG_LEVEL_WARNING
     6 - LOG_LEVEL_INFO
     Default value is 3 (LOG_LEVEL_ERROR)
  */
  uint8_t                      logger_rate_limit_max_log_level;

  /**
     Cyclic-shift score threshold for PSCCH decoding.
     Default is 0 which means the DSP which will set its own default value
  */
  uint32_t                     cv2x_dsp_cs_score_threshold;

  /**
     Rx users selection configuration:
     Bits [0:7]   - ID of the V2I pool (the other is V2V): 0 - Not initialized; 1 - Pool-0; 2 - Pool-1
     Bits [8:15]  - V2I score factor (for prioritizing V2I over V2V)
     Bits [16:23] - RV2-combine score factor (for prioritizing HARQ copy)
     Bits [24:31] - Dense mode threshold (number of users before we start to operate in "dense mode")
     Default is 0 which means the DSP which will set its own default value
  */
  uint32_t                     cv2x_dsp_rx_users_selection_config;

  /**
     Adjustment to RX processing time budget.
     Default is 0 which means the DSP which will set its own default value
  */
  int32_t                      cv2x_dsp_rx_proc_time_offset;

  /**
     If we detect more UEs (PSCCH) in a SF than this threshold, we will decode only PSCCH and skip the PSSCH decoding.
     Default is 0 which means the DSP which will set its own default value
  */
  uint32_t                     cv2x_dsp_num_of_pscch_above_threshold;

  /**
     In sps, exceeding this gap from last successful message, will invoke reselection.
  */
  uint32_t                     cv2x_reselect_after_gap_from_last_msg_msec;

  /**
     In sps, exceeding this number of unused reservations, will invoke reselection.
     This value will be used by RR when reselect_after_r14 is not configured in RRC.
  */
  uint32_t                     cv2x_reselect_after_unused_slots;

  uint32_t                     rx_duplication_pps;

  /** Is MCS-TBS tables for US SAE profile enabled (0 - disabled, 1 - enabled) */
  uint32_t                     cv2x_mcs_tbs_us_sae_mode;

  /** US SAE TBS-MCS configuration */
  cv2x_mcs_tbs_us_sae_config_t cv2x_mcs_tbs_us_sae_config;
  /**
     US SAE Profile Allocation Window adjustment configuration.
     When operating in US SAE mode, the system shall set the resource selection window ([n+T1, n+T2]) as follows:

    T1 <= 4
    T2 = cv2x_us_sae_t2_1 if CBR <= cv2x_us_sae_cbr_threshold_1
    T2 = min(PDB-10, cv2x_us_sae_t2_2) if cv2x_us_sae_cbr_threshold_1 < CBR < cv2x_us_sae_cbr_threshold_2
    T2 = PDB - cv2x_us_sae_pdb_reduction if CBR >= cv2x_us_sae_cbr_threshold_2
  */
  uint32_t              cv2x_us_sae_win_adjustment_enable;
  uint32_t              cv2x_us_sae_cbr_threshold_1;
  uint32_t              cv2x_us_sae_t2_1;
  uint32_t              cv2x_us_sae_cbr_threshold_2;
  uint32_t              cv2x_us_sae_t2_2;
  uint32_t              cv2x_us_sae_pdb_reduction;

  /** Application SPS send jitter, in milliseconds */
  uint32_t              cv2x_rr_sps_send_jitter_msec;

  /** Maximum number of opperations(loops) during the reselection. */
  uint32_t              cv2x_rr_reselection_max_iterations;

  /**
     MLT Payload Length Threshold. Segments with the payload less or equal to the
     threshold will use selection window with dispatcher_ahead reduced by
     dispatcher_and_lmac_ahead_reduction_us.
   */
  uint32_t              cv2x_mlt_payload_len_threshold_bytes;

  /**
     Dispatcher ahead reduction when the segment payload length is less or equal
     to mlt_payload_len_threshold_bytes.
  */
  uint32_t              cv2x_dispatcher_ahead_reduction_usec;

  /** Skip Sensing Based Algorithm and allocate the first available slot */
  uint32_t              cv2x_rr_skip_sba;

  /**
     Flag to indicate if Rx promiscuous mode is enabled or not.
     When enabled - validity check of PDCP Rx header's fields (SN, PGK and PTK) is disabled.
     0 - Mode disabled, validity checks active (default)
     1 - Mode enabled, validy checked not active
  */
  uint8_t               rx_promiscuous_mode;
} cv2x_configuration_t;
atlk_rc_t ref_cv2x_sys_init(CV2X_IN const char *interface_name,
                  CV2X_IN char *application_role_str,
                  CV2X_OUT cv2x_configuration_t *config_ptr,
                  CV2X_OUT cv2x_rrc_pre_config_t *rrc_config_ptr)
{
  return ATLK_OK;
}

int32_t ref_cv2x_ue_id_get(void)
{
  return 0;
}
#define atlk_must_check
/** WDM service instance */
typedef struct wdm_service wdm_service_t;
//static wdm_service_t my_wdm_service;
atlk_rc_t atlk_must_check wdm_service_get(const char *service_name,wdm_service_t **service_pptr)
{
//  **service_pptr = &my_wdm_service;
  return ATLK_OK;
}
/** DDM service instance */
typedef struct ddm_service ddm_service_t;
atlk_rc_t atlk_must_check ddm_service_get(const char *service_name, ddm_service_t **service_pptr)
{
  return ATLK_OK;
}
/** DDM state */
typedef enum {
  /** Device disconnected */
  DDM_STATE_DISCONNECTED = 0,

  /** Device not initialized */
  DDM_STATE_UNINITIALIZED,

  /** Device ready */
  DDM_STATE_READY,

} ddm_state_t;

/** Invalid argument */
#define ATLK_E_INVALID_ARG ATLK_RC(2)

atlk_rc_t atlk_must_check ddm_state_get(ddm_service_t *service_ptr, ddm_state_t *ddm_state_ptr)
{
  atlk_rc_t ret = ATLK_E_INVALID_ARG;
  if((NULL != service_ptr) && (NULL != ddm_state_ptr)){
      *ddm_state_ptr = DDM_STATE_READY;
      ret = ATLK_OK;
  }
  return ret;
}
atlk_rc_t atlk_must_check ddm_version_get(ddm_service_t *service_ptr,char_t *version, size_t *version_size_ptr)
{
  return ATLK_OK;
}
/** TSF lock status */
typedef enum {
  /** TIME SYNC is not synchronized*/
  DDM_TSF_LOCK_STATUS_UNLOCKED = 0,

  /** TIME SYNC PPS is locked*/
  DDM_TSF_LOCK_STATUS_LOCKED,

  /** TIME SYNC is synchronized */
  DDM_TSF_LOCK_STATUS_UTC_SYNCED

} ddm_tsf_lock_status_t;
atlk_rc_t atlk_must_check ddm_tsf_get(ddm_service_t *service_ptr,uint64_t *time_usec_ptr,uint64_t *accuracy_usec_ptr,ddm_tsf_lock_status_t *ddm_tsf_lock_status_ptr)
{
  return ATLK_OK;
}
/** DDM baseband V2X HW revision */
typedef struct {
  /** Baseband V2X HW revision major */
  uint32_t bb_v2x_hw_rev_major;

  /** Baseband V2X HW revision minor */
  uint32_t bb_v2x_hw_rev_minor;

} ddm_baseband_v2x_hw_revision_t;
atlk_rc_t atlk_must_check ddm_baseband_v2x_hw_revision_get(ddm_service_t *service_ptr,ddm_baseband_v2x_hw_revision_t *revision_ptr)
{
  return ATLK_OK;
}
atlk_rc_t cv2x_version_get(CV2X_OUT uint32_t * const major_ver_ptr,CV2X_OUT uint32_t * const minor_ver_ptr,CV2X_OUT uint32_t * const revision_ver_ptr)
{
  return ATLK_OK;
}
/** CV2X service instance obscure data type */
typedef struct cv2x_service_st cv2x_service_t;
atlk_rc_t atlk_must_check cv2x_service_get(CV2X_IN  const char *         const service_name_ptr,
                 CV2X_OUT       cv2x_service_t       **service_pptr)
{
  return ATLK_OK;
}

atlk_rc_t atlk_must_check cv2x_configuration_set(CV2X_IN cv2x_configuration_t *configuration_ptr, CV2X_IN cv2x_rrc_pre_config_t *rrc_config_ptr)
{
  return ATLK_OK;
}
atlk_rc_t atlk_must_check cv2x_service_enable(cv2x_service_t *service_ptr)
{
  return ATLK_OK;
}
atlk_rc_t atlk_must_check cv2x_src_l2id_set(CV2X_IN  cv2x_service_t *service_ptr, CV2X_OUT uint32_t       src_l2id)
{
  return ATLK_OK;
}
/** CV2X service instance obscure data type */
typedef struct cv2x_service_st cv2x_service_t;

/** CV2X socket obscure type */
typedef struct cv2x_socket cv2x_socket_t;


/** CV2X socket types enum */
typedef enum {
  /** No type */
  CV2X_SOCKET_TYPE_NONE = 0,

  /** Socket to transmit semi persistent messages */
  CV2X_SOCKET_TYPE_SEMI_PERSISTENT_TX,

  /** Socket to transmit ad hoc messages */
  CV2X_SOCKET_TYPE_AD_HOC_TX,

  /** Socket to receive messages */
  CV2X_SOCKET_TYPE_RX,

  /** Maximum allowed value */
  CV2X_SOCKET_TYPE_MAX = CV2X_SOCKET_TYPE_RX
} cv2x_socket_type_t;

/** CV2X socket traffic type */
typedef enum {
  /** Not initialized */
  CV2X_SOCKET_TRAFFIC_NONE = 0,

  /** IP Traffic */
  CV2X_SOCKET_TRAFFIC_IP,

  /** NON-IP Traffic */
  CV2X_SOCKET_TRAFFIC_NON_IP,

  /** Maximum allowed value */
  CV2X_SOCKET_TRAFFIC_MAX = CV2X_SOCKET_TRAFFIC_NON_IP
} cv2x_socket_traffic_t;

/** CV2X socket V2X family ID */
typedef enum {
  /** Not initialized */
  CV2X_SOCKET_V2X_FAMILY_ID_NONE = 0,

  /** IEEE_1609 */
  CV2X_SOCKET_V2X_FAMILY_ID_IEEE_1609,

  /** ISO */
  CV2X_SOCKET_V2X_FAMILY_ID_ISO,

  /** ETSI_ITS */
  CV2X_SOCKET_V2X_FAMILY_ID_ETSI_ITS,

  /** Maximum allowed value */
  CV2X_SOCKET_V2X_FAMILY_ID_MAX = CV2X_SOCKET_V2X_FAMILY_ID_ETSI_ITS
} cv2x_socket_v2x_family_id_t;

/** CV2X socket configuration */
typedef struct cv2x_socket_config_st {
  /** Socket traffic type: IP / NON_IP */
  cv2x_socket_traffic_t       traffic_type;

  /** Family ID: IEEE_1609 / ISO / ETSI_ITS [Relevant only for traffic_type of NON_IP] */
  cv2x_socket_v2x_family_id_t v2x_family_id;
} cv2x_socket_config_t;

atlk_rc_t atlk_must_check cv2x_socket_create(cv2x_service_t *service_ptr,
                   cv2x_socket_type_t           socket_type,
                   const cv2x_socket_config_t  *socket_config_ptr,
                   cv2x_socket_t               **socket_pptr)
{
  return ATLK_OK;
}

atlk_rc_t cv2x_build_version_get(CV2X_OUT uint32_t * const build_num_ptr)
{
  return ATLK_OK;
}
/** CV2X policy / socket settings */
typedef struct cv2x_socket_policy_st {
  /** Policy message priority, i.e PPPP */
  cv2x_pppp_t priority;

  /** Policy message size (in bytes) */
  uint32_t    size;

  /** Policy control interval in millisec */
  uint32_t    control_interval_ms;
} cv2x_socket_policy_t;
atlk_rc_t atlk_must_check cv2x_socket_policy_set(CV2X_IN       cv2x_socket_t        *socket_ptr,
                       CV2X_IN const cv2x_socket_policy_t *socket_policy_ptr)
{
  return ATLK_OK;
}
bool_t cv2x_tx_is_ready(void)
{
  return(WTRUE);
}
/** Power in units of dBm */
typedef int8_t power_dbm_t;

/** Value indicating that power in units of dBm is N/A */
#define POWER_DBM_NA INT8_MIN

/** Power in units of 1/8 dBm */
typedef int16_t power_dbm8_t;

#define ATLK_INTERFACES_MAX 2
#define RF_INDEX_MAX 2


/** Maximum size of compensator data */
#define COMPENSATOR_DATA_SIZE_MAX 8

/** Generic compensator data */
typedef struct compensator_data {
  uint8_t data[COMPENSATOR_DATA_SIZE_MAX];
  uint8_t len;
  uint8_t reserved;
} compensator_data_t;


typedef uint64_t cv2x_time_t;

/** CV2X send parameters */
typedef struct cv2x_send_params_st {
  /** Message ID */
  uint32_t      message_id;

  /** Source layer 2 ID */
  uint32_t      src_l2id;

  /** Destination layer 2 ID */
  uint32_t      dst_l2id;

  /** Message generation time by application */
  cv2x_time_t   generation_time;

  /** Transmission power level in units of 1/8 dBm Per Antenna */
  power_dbm8_t  power_dbm8[ATLK_INTERFACES_MAX];

  /** Compensator data per antenna.  */
  compensator_data_t comp_data[RF_INDEX_MAX];
} cv2x_send_params_t;



atlk_rc_t atlk_must_check cv2x_sps_send(CV2X_IN cv2x_socket_t            *sps_socket_ptr,
              CV2X_IN const uint8_t            *data_ptr,
              CV2X_IN size_t                   data_size,
              CV2X_IN const cv2x_send_params_t *params_ptr)
{
  return ATLK_OK;
}

/** Default policy descriptor initializer */
#define CV2X_SOCKET_POLICY_INIT CV2X_SOCKET_POLICY_ST(0, CV2X_PPPP_0, 0)

/** CV2X receive parameters */
typedef struct cv2x_receive_params_st {
  /** Receive time */
  cv2x_time_t                 receive_time;

  /** Measured total received power at antenna port. */
  power_dbm8_t                rssi[ATLK_INTERFACES_MAX];

  /** Priority */
  cv2x_pppp_t                 pppp;

  /** Source Layer-2 ID field, Identity of the source == ProSe UE ID */
  uint32_t                    l2id_src;

  /** Destination Layer-2 ID field, Identity of the source == ProSe UE ID */
  uint32_t                    l2id_dst;

  /** V2X family ID - relevant only for traffic_type of NON_IP */
  cv2x_socket_v2x_family_id_t v2x_family_id;

} cv2x_receive_params_t;

/** Wait option type */
typedef enum {
  /** Wait a time interval */
  ATLK_WAIT_TYPE_INTERVAL = 0,

  /** Wait indefinitely */
  ATLK_WAIT_TYPE_FOREVER = 1

} atlk_wait_type_t;
typedef struct {
  /** Wait option type */
  atlk_wait_type_t wait_type;

  /** Number of microseconds */
  uint32_t wait_usec;

} atlk_wait_t;
atlk_rc_t atlk_must_check cv2x_receive(CV2X_IN          cv2x_socket_t         *socket_ptr,
             CV2X_OUT         uint8_t               *data_ptr,
             CV2X_INOUT       size_t                *data_size_ptr,
             CV2X_OUT         cv2x_receive_params_t *params_ptr,
             CV2X_IN    const atlk_wait_t           *wait_ptr)
{
  return ATLK_OK;
}
atlk_rc_t cv2x_socket_delete(cv2x_socket_t *socket_ptr)
{
  return ATLK_OK;
}
typedef struct {
  /** L2 */
  uint32_t l2_messages_received_successfully;
  uint32_t l2_messages_dropped_crc_errors;
  uint32_t l2_messages_dropped_queue_full;
  uint32_t l2_messages_dropped_exceeding_max_rx_size;
  uint32_t l2_messages_received_per_mcs[CV2X_MCS_MAX + 1];

  /** RLC */
  uint32_t rlc_messages_reassembled_successfully;
  uint32_t rlc_messages_reassembly_timed_out;
  uint32_t rlc_messages_duplicated;

  /** PDCP */
  uint32_t pdcp_messages_received_successfully;
  uint32_t pdcp_messages_dropped_parsing_error;

  /** SVC */
  uint32_t svc_messages_received_successfully;
  uint32_t svc_messages_dropped;

  /** MAC */
  uint32_t mac_messages_received_successfully;
  uint32_t mac_messages_dropped_invalid_destination_l2id;
  uint32_t mac_messages_dropped_queue_full;
  uint32_t mac_messages_dropped_invalid_struct;
  uint32_t mac_messages_dropped_invalid_version;
  uint32_t mac_messages_pending;
  uint32_t mac_messages_concatenated; // (PDUs which contain multiple SDUs)
} cv2x_rx_stats_t;
atlk_rc_t atlk_must_check cv2x_rx_stats_get(CV2X_IN cv2x_service_t *service_ptr, CV2X_OUT cv2x_rx_stats_t *rx_stats_ptr)
{
  return ATLK_OK;
}

/** Number of RSSI values returned prior calibration */
#define WDM_RSSI_VALUES_COUNT 8

/** WDM DSP revision ID length */
#define WDM_DSP_REV_ID_LEN 20

/** WDM DSP version */
typedef struct {
  /** Major */
  uint8_t major;

  /** Minor */
  uint8_t minor;

  /** SW revision */
  uint8_t sw_revision;

  /** Revision id (19 Bytes + Null terminated) */
  char dsp_rev_id[WDM_DSP_REV_ID_LEN];
} wdm_dsp_version_t;
atlk_rc_t atlk_must_check wdm_dsp_version_get(wdm_service_t *service_ptr,
                    wdm_dsp_version_t *dsp_version_ptr)
{
  return ATLK_OK;
}
/** WDM interface states */
typedef enum {
  /** Detached interface state */
  WDM_INTERFACE_STATE_DETACHED = 0,

  /** Attached interface state */
  WDM_INTERFACE_STATE_ATTACHED,
} wdm_interface_state_t;
/** WDM interface modes */
typedef enum {
  /** CV2X mode */
  WDM_INTERFACE_MODE_CV2X = 0,

  /** DSRC mode */
  WDM_INTERFACE_MODE_DSRC,

  /** WIFI mode */
  WDM_INTERFACE_MODE_WIFI,

  /** Special mode for diag-cli */
  WDM_INTERFACE_MODE_SERVICE,

  /** Max number of interface modes */
  WDM_INTERFACE_MODE_MAX,
} wdm_interface_mode_t;
/** Interface status */
typedef struct {
  /** Interface state */
  wdm_interface_state_t state;

  /** Interface mode */
  wdm_interface_mode_t mode;
} wdm_interface_status_t;
/** MAC interface index */
typedef uint8_t if_index_t;
atlk_rc_t wdm_interface_status_get(wdm_service_t *service_ptr,
                         if_index_t if_index,
                         wdm_interface_status_t *status_ptr)
{
  return ATLK_OK;
}

/** WDM status */
typedef enum {
  /** Status is OK */
  WDM_STATUS_OK = 0,

  /** Status is not OK */
  WDM_STATUS_FAILED
} wdm_status_t;

/** RF status */
typedef struct {
  /** Status of AFE PLL */
  wdm_status_t afe_pll;

  /** Status of RF PLL */
  wdm_status_t rf_pll[RF_INDEX_MAX];

  /** Status of calibration status */
  wdm_status_t calibration_status[RF_INDEX_MAX];

  /** RFIC chip temperature  */
  int32_t temperature_celsius;

  /** Modem status */
  wdm_status_t modem_status;

  /** Modem recovery counter */
  uint32_t modem_recovery_counter;
} wdm_rf_status_t;

atlk_rc_t atlk_must_check wdm_rf_status_get(wdm_service_t *service_ptr, wdm_rf_status_t *status_ptr)
{
  return ATLK_OK;
}


typedef struct cv2x_socket_stats_st {
  /** Number of messages for tx */
  uint32_t    messages_count;

  /** Number of messages successfully sent */
  uint32_t    messages_sent_success;

  /** Number of messages which failed to be sent after reaching MAC Layer */
  uint32_t    messages_sent_failures;

  /** Number of messages which were dropped before reaching MAC Layer */
  uint32_t    messages_dropped;

  /** Number of skipped transmission slots due to missing tx message */
  uint32_t    skipped_resources;
} cv2x_socket_stats_t;
atlk_rc_t atlk_must_check
cv2x_socket_stats_get(CV2X_IN  cv2x_service_t      *service_ptr,
                      CV2X_IN  cv2x_socket_t       *socket_ptr,
                      CV2X_IN  uint32_t            millisec,
                      CV2X_OUT cv2x_socket_stats_t *socket_stats_ptr)
{
  return ATLK_OK;
}
static char_t my_rc_string[] = "ATLK_OK";
const char_t *atlk_rc_to_str(atlk_rc_t rc)
{
  return my_rc_string;
}
atlk_rc_t atlk_must_check time_sync_init(ddm_service_t* ddm_service_ptr)
{
  return ATLK_OK;
}
#endif /* NO_SECURITY */

/* ASSUMPTION: No other tasks are using I2V SHM yet so no locks needed. */

/* Default SHM flavor. Unit Test and IWMH will open SHM PATH*/
int i2v_setupSHM_Default(i2vShmMasterT * shmPtr)
{
    if(NULL == shmPtr) {
        return I2V_RETURN_SHM_FAIL;
    }

    if (WFALSE == wsu_shmlock_init(&shmPtr->cfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for cfgdata.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->scsCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for scsCfgdata.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->scsSpatData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for scsSpatData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->spatCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for spatCfgdata %d %s.\n", errno, strerror(errno)); 
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->ipbCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for ipbCfgData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->amhCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for amhCfgData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->scsSRMData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for scsSRMData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (WFALSE == wsu_shmlock_init(&shmPtr->fwdmsgCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for fwdmsgCfgData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    /* Init gates. */
    if(WFALSE == wsu_init_gate(&shmPtr->scsSpatData.spatTriggerGate)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," gate init failed for spatTriggerGate.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if(WFALSE == wsu_init_gate(&shmPtr->cfgData.iwmhGate)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," gate init failed for iwmhGate.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if(WFALSE == wsu_init_gate(&shmPtr->scsSRMData.srmTriggerGate)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," gate init failed for srmTriggerGate.\n");
        return I2V_RETURN_SHM_FAIL;
    }

    /* Setup as i2v.conf would. */
    strncpy(shmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN); /* Default conf used in install. */

    shmPtr->cfgData.scsAppEnable  = 1;
    shmPtr->cfgData.srmAppEnable  = 1;
    shmPtr->cfgData.spatAppEnable = 1;
    shmPtr->cfgData.ipbAppEnable  = 0;
    shmPtr->cfgData.amhAppEnable  = 1;
    shmPtr->cfgData.iwmhAppEnable = 1;

    shmPtr->cfgData.wsaEnable = 1;
    shmPtr->cfgData.radio0wmh.radioChannel=I2V_RADIO_CHANNEL_DEFAULT;
    shmPtr->cfgData.radio1wmh.radioChannel=I2V_RADIO_CHANNEL_DEFAULT;
    shmPtr->cfgData.radio2wmh.radioChannel=I2V_RADIO_CHANNEL_DEFAULT;
    shmPtr->cfgData.radio3wmh.radioChannel=I2V_RADIO_CHANNEL_DEFAULT;
    shmPtr->cfgData.bcastLockStep =1;
    shmPtr->cfgData.uchannel= I2V_RADIO_CHANNEL_DEFAULT;
    shmPtr->cfgData.uradioNum = 0;
    shmPtr->cfgData.uwsaPriority = 20;
    shmPtr->cfgData.security = 0;

    shmPtr->cfgData.secSpatPsid = 0x8002;
    shmPtr->cfgData.secMapPsid = 0x8002;
    shmPtr->cfgData.secTimPsid = 0x8003;

    shmPtr->cfgData.RadioType               = 0;
    //shmPtr->cfgData.globalApplogsEnableFlag = 1;
    shmPtr->cfgData.globalDebugFlag         = 1;
    //shmPtr->cfgData.i2vDebugEnableFlag      = 1;
    //shmPtr->cfgData.i2vApplogEnableFlag     = 1;
    //shmPtr->cfgData.bcastLockStep           = WTRUE;
    shmPtr->cfgData.security                = WFALSE;

    wsu_shmlock_lockr(&shmPtr->cfgData.h.ch_lock); //test, what does it do if not init? 
    wsu_shmlock_unlockr(&shmPtr->cfgData.h.ch_lock);

    wsu_shmlock_lockw(&shmPtr->cfgData.h.ch_lock); //test 
    wsu_shmlock_unlockw(&shmPtr->cfgData.h.ch_lock);

    wsu_shmlock_lockr(&shmPtr->amhCfgData.h.ch_lock);
    wsu_shmlock_unlockr(&shmPtr->amhCfgData.h.ch_lock);

    wsu_shmlock_lockw(&shmPtr->amhCfgData.h.ch_lock);
    wsu_shmlock_unlockw(&shmPtr->amhCfgData.h.ch_lock);

    shmPtr->cfgData.h.ch_data_valid = WTRUE;
    shmPtr->fwdmsgCfgData.h.ch_data_valid = WTRUE;
    return I2V_RETURN_OK;
}
void i2v_setupSHM_Clean(i2vShmMasterT * shmPtr)
{
    if(NULL == shmPtr) {
        return;
    }
    /* semaphore cleanup */
    wsu_shmlock_kill(&shmPtr->i2vPids.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->cfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->scsSpatData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->scsCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->spatCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->ipbCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->amhCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->fwdmsgCfgData.h.ch_lock);

    wsu_kill_gate(&shmPtr->scsSpatData.spatTriggerGate);
    wsu_kill_gate(&shmPtr->cfgData.iwmhGate);
    wsu_kill_gate(&shmPtr->scsSRMData.srmTriggerGate);
    //wsu_shmlock_kill(&shmPtr->logmgrShmData.h.ch_lock);
    
    /* remove I2V shared mem */
    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));
    wsu_share_delete(I2V_SHM_PATH);
}
