/**************************************************************************
 *                                                                        *
 *     File Name:   i2v.c                                                 *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Rd, Carlsbad                                        *
 *         CA 92008                                                       *
 *                                                                        *
 **************************************************************************/
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#if defined RSE_SIMULATOR
#include <mqueue.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "conf_table.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
#else
#define MAIN main
#define dn_mkfifo mkfifo
#define dn_msgget msgget
#define dn_msgctl msgctl
#endif

/* global defines */
#define UNDEFINED   "Undefined"
#define DEFAULT_CERT "root_ca"
#define GEN_CERT "msg_id_nlo"     /* TODO: assumption for Tx is 1 cert - need to understand if any more required */
#define KEY_EXTEN ".key"
#define CERT_EXTEN ".cert"
#define SECURITY_PROC "escsmpc"
#define DEFAULT_BIN_PATH "/usr/bin/"    /* FIXME: this define assumes programs like the SECURITY_PROC are always
                                           in the /usr/bin directory.  These programs are not part of i2v but i2v
                                           apps require them - fix this define if ever these programs move */

/* although currently no wsu supports 4 radios, DOT requires that number 
   - until hw available cfg files should disable extra radios */
#define NUM_RADIOS         4

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "i2v"
#define ETH_IFC        "eth0"

/* Minimum wait time in seconds between starting IFC(interface logs).*/
#define I2V_IFC_START_WAIT_SECS (2)

/* global variables */
static i2vShmMasterT *shmPtr = NULL;
STATIC bool_t mainloop = WTRUE;
STATIC cfgItemsT cfg;

static char_t *scs[]    = {(char_t *)"scs_app",  (char_t *)0};
static char_t *srm[]    = {(char_t *)"srmrx_app",  (char_t *)0};
static char_t *spat[]   = {(char_t *)"spat_app", (char_t *)0};
static char_t *ipb[]    = {(char_t *)"ipb_app", (char_t *)0};
static char_t *amh[]    = {(char_t *)"amh_app", (char_t *)0};
static char_t *fwdmsg[] = {(char_t *)"fwdmsg_app", (char_t *)0};

#ifdef RSE_SIMULATOR
static char_t *sps[] = {(char_t *)"sps", (char_t *)0};
static mqd_t mqSim;
#endif /* RSE_SIMULATOR */

static processPidsT i2vPids;
struct pcmd processes[] = {
#ifdef RSE_SIMULATOR
    {"sps",    (char_t **)&sps,    &i2vPids.sps,    0, &cfg.spsAppEnable},
#endif
    {"fwdmsg", (char_t **)&fwdmsg, &i2vPids.fwdmsg, 0, &cfg.fwdmsgAppEnable},  /* start before spat/amh/srm_rx */
    {"scs",    (char_t **)&scs,    &i2vPids.scs,    0, &cfg.scsAppEnable},
    {"spat",   (char_t **)&spat,   &i2vPids.spat,   0, &cfg.spatAppEnable},
    {"srm_rx", (char_t **)&srm,    &i2vPids.srm,    0, &cfg.srmAppEnable},
    {"ipb",    (char_t **)&ipb,    &i2vPids.ipb,    0, &cfg.ipbAppEnable},  /* start after map - uses map data if available */
    {"amh",    (char_t **)&amh,    &i2vPids.amh,    0, &cfg.amhAppEnable},  /* start after everything else */
};


/* dbg time support */
STATIC i2vUtilTimeParams dbgTime;

/* function protos */
static void i2vWaitFor(uint32_t secs);
STATIC void i2v_clear_statics(void);
/* this function exists in i2v because when the forwarding
   system file was touched by the child ipb process it causes
   a kernel panic every time - updating the system file can 
   not be done in the child process - not sure why */
/* editor's note: kernel panic was WSU1.0, no test on others;
   the most likely issue was the use of the system cmd in
   the child after registering for SIGCHLD - that was
   causing everything to terminate and the radio driver 
   seemingly doesn't handle quick termination well */
#if 0
static void i2vIPv6Forwarding(bool_t activate, int *savedval)
{
    char_t cmd[100] = {0};
    FILE  *f;
    char_t str[10] = {0};  /* up to 10 chars i.e. MAX_INT */

    if (activate) { 
        if ((f=fopen(PROC_IPV6_FORWARD_FNAME, "r")) == NULL) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to open forwarding file %s\n", PROC_IPV6_FORWARD_FNAME);
            return;
        }

        fread(str, sizeof(char), 10, f);
        fclose(f);
        *savedval = atoi(str);

        sprintf(cmd, "echo 1 > " PROC_IPV6_FORWARD_FNAME); /* magic value to enable forwarding in network stack */
    } else {
        sprintf(cmd, "echo %d > " PROC_IPV6_FORWARD_FNAME, *savedval);
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ipv6 forwarding new command: %s\n", cmd);
    system(cmd);
}
#endif

/* networking enable function for eth port 
   uses system calls - cannot be run after 
   registering for SIGCHLD unless shutting down */
/* 20191015: JJG - holy $h!t, we have been using QNX for
   4 years and never tested this! wow; fixing something
   that's been broken for 4 years */
static void i2vIPv6EthSet(bool_t activate)
{
    /* need to insert scope into activation commands */
    char_t cmd[300];    /* should be plenty */
    uint8_t origaddr[I2V_IP_MAX_STR_LEN] = {0};
    uint8_t prfxscope[I2V_IP_MAX_STR_LEN + 4];
    char_t  dummy,*tok;
    uint8_t i, numwrds;

    strcpy((char_t *)origaddr, (char_t *)cfg.i2vipv6addr);   /* will be used in strtok */
    memset(prfxscope, 0, sizeof(prfxscope));
    numwrds = cfg.ethIPv6Scope / 16;   /* scope is num bits - if scope not multiple of 8 the
                                          division will yield 1 less than the words for scope */
    if (cfg.ethIPv6Scope % 16) numwrds++;
    tok = strtok((char_t *)origaddr, ":");
    for (i=0; i<numwrds; i++) {
        if (NULL != tok) {
            strcat((char_t *)prfxscope, tok);
            strcat((char_t *)prfxscope, ":");
            if (i < (numwrds -1)) {
            /* tok is pointing to bytes in origaddr (strtok behavior); strtok
               will treat repetitive delimiters as single delim, manually check
               if "::" - could be 2000::1 as IP that means rest of prefix is 0 
               so just break from loop */
                dummy = tok[strlen(tok) + 1];
                if ((dummy == ':') || (dummy == 0)) {
                    break;   /* no more 'valid' tokens - rest of addr is 0; check
                                for 0 above is to see if val already set by strtok */
                } else {
                    tok = strtok(NULL, ":");
                }
            }
        } else if (i==0) {
            /* buggy condition - the address has invalid format - don't touch ip */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid IPv6 format - will not modify IP address settings\n");
            return;
        } else {
            /* should never reach here */
            break;
        }
    }
    strcat((char_t*)prfxscope, ":");   /* final ':' for a prefix */
    sprintf(cmd, "ifconfig " ETH_IFC " inet6 %s/%d", (char_t *)cfg.i2vipv6addr, cfg.ethIPv6Scope);
    if (!activate) {
        strcat(cmd, " delete");
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"setting IP: %s\n", cmd);
    if (0 != system(cmd)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"setting IP failed: %s: errno=%d\n", cmd, errno);
    }
#if 0   /* do we need to set this on QNX (don't seem to need this)?? if so, it needs to be route not ip -6 */
    sprintf(cmd, "ip -6 route %s %s/%d dev " ETH_IFC, action, prfxscope, cfg.ethIPv6Scope);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"setting route: %s\n", cmd);
    system(cmd);
#endif /* if 0 */
    /* just keeping this (too lazy to care how "NONE" was picked; yes i wrote the original code; YEARS AGO) */
    if (strcmp((char_t *)cfg.i2vipv6gwaddr, "NONE")) {
        sprintf(cmd, "route %s -inet6 default %s", (activate) ? "add" : "delete", (char_t *)cfg.i2vipv6gwaddr);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"setting default gateway: %s\n", cmd);
        if (0 != system(cmd)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"setting default gateway failed: %s: errno=%d\n", cmd, errno);
        }
    }
}

/* if cleanupOnly is WTRUE, will remove all queues
   and always returns WFALSE */
static bool_t i2vIwmhQSetup(bool_t cleanupOnly)
{
    /* note on System V msgqueues - only 16 supported
       and if 4 radios supported, 5 will exist leaving
       only 11 for the rest of all apps running */
    int32_t i, ret = -1;
    char_t qname[I2V_CFG_MAX_STR_LEN];
    iwmhVars *radio = &cfg.radio0wmh;

    if (cleanupOnly) {
        goto qCleanup;
    }

    for (i=0; i<NUM_RADIOS; radio++, i++) {     /* num supported radios */
        /* cchradio is always enabled despite cfg  */
        if ((cfg.cchradio == i) && (!radio->radioEnable)) {
            radio->radioEnable = WTRUE;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Config alert: cch radio (%d) disabled in config, activating nonetheless\n", i);
        }

        if (!radio->radioEnable) {
            continue;
        }
        if (cfg.nocflash) {
            sprintf(qname, "%s%d", I2V_WMH_BACKUP_QUEUE, i);
        } else {
            sprintf(qname, "%s%d", I2V_WMH_QUEUE, i);
        }
        if (((ret = dn_mkfifo(qname, DEFAULT_Q_MODE)) < 0) && (errno != EEXIST)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create message queue for broadcast messages. errno=%d.\n",errno);
            break;
        } else if (ret < 0) {
            remove(qname);   /* start fresh */
            if ((ret = dn_mkfifo(qname, DEFAULT_Q_MODE)) < 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create message queue for broadcast messages. errno=%d.\n",errno);
                break;
            }
        }

        /* now the msg queue from iwmh - using System V instead of POSIX mq 
           - 8k max msg size, 16k max bytes in queue (1.0/1.5/5001) */
        if ((radio->msgListener = dn_msgget(IPC_PRIVATE, IPC_CREAT)) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create message response queue for broadcast messages (%s %s)\n", qname, strerror(errno));
            ret = radio->msgListener;
            break;
        }
    }
    if (ret >= 0) {
        if ((cfg.iwmhSecQ = dn_msgget(IPC_PRIVATE, IPC_CREAT)) < 0) {
            /* unfortunately this queue may not be needed if no app enables VOD but since
               queue needs to exist before apps are started will fail startup */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create VOD queue for iwmh/apps: %s\n", strerror(errno));
            ret = cfg.iwmhSecQ;
        }
    }

qCleanup:
    if (ret < 0) {
        radio = &cfg.radio0wmh;
        for (i=0; i<NUM_RADIOS; radio++, i++) {
            if (!radio->radioEnable) {
                continue;
            }
            if (cfg.nocflash) {
                sprintf(qname, "%s%d", I2V_WMH_BACKUP_QUEUE, i);
            } else {
                sprintf(qname, "%s%d", I2V_WMH_QUEUE, i);
            }
            remove(qname);
            if (radio->msgListener < 0) {
                continue;
            }
            ret = dn_msgctl(radio->msgListener, IPC_RMID, NULL);
#ifdef EXTRA_DEBUG
            if (ret) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to cleanup broadcasting response queue (%s)\n", strerror(errno));
            }
#endif /* EXTRA_DEBUG */
        }
        if (cfg.iwmhSecQ >= 0) {
            ret = dn_msgctl(cfg.iwmhSecQ, IPC_RMID, NULL);
        }
#ifdef EXTRA_DEBUG
        if (ret) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to cleanup VOD iwmh queue (%s)\n", strerror(errno));
        }
#endif /* EXTRA_DEBUG */
        return WFALSE;
    }

    return WTRUE;
}

/* need to create an instance per enabled radio */
static void i2vInitWMH(const char_t *path)
{
    uint8_t i;
    union sigval sv;
    char_t proc[200];
    char_t radionum[2] = {(char_t)0,(char_t)0};
    char_t select1[3];
    char_t select2[3];
    char_t select3[3];
    char_t psid1[5];
    char_t psid2[5];
    char_t psid3[5];
    iwmhVars *radio = &cfg.radio0wmh;

    /* supporting up to 3 psids - need to pass all Rx secure psids 
       if security enabled -  which ones used doesn't matter just 
       make sure they are all passed since iwmh handles security 
       init for all bcast */
    char *iwmh[] = {"iwmh_app", "-r", radionum, select1, psid1, select2, 
                     psid2, select3, psid3, (char_t *)0};

    if (!cfg.iwmhAppEnable) {
        return;
    }

    memset(&sv, 0, sizeof(sv));
    sv.sival_int = 0;

    /* last string done so that name of process only needs to be changed in one place */
    sprintf(proc, "%s%s", path, *(char_t *const *)iwmh);

    /* Rx PSIDs - add all new Rx PSIDs accordingly */
    strcpy(select1, "-p");
    sprintf(psid1, "%x", cfg.bsmPsidDer);
    strcpy(select2, "-p");
    sprintf(psid2, "%x", cfg.bsmPsidPerA);
    strcpy(select3, "-p");
    sprintf(psid3, "%x", cfg.bsmPsidPerU);

    for (i=0; i<NUM_RADIOS; radio++, i++) {
        if (!radio->radioEnable) {
            continue;
        }
        sprintf(radionum, "%d", i);
#if !defined(MY_UNIT_TEST)
        if ((radio->pid = fork()) == -1) {
#else
        radio->pid = 1;
        if(radio->pid) {
#endif
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"iwmh %d fork failed: %s\n", i, strerror(errno));
            /* signal ourself so that proper cleanup is handled before termination */
            sigqueue(getpid(), SIGTERM, sv);
            return;
        } else if (radio->pid == 0) {
            if ((strlen(path) > 0) && (open(proc, O_RDONLY) == -1)) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cannot run cmd: %s (%s)\n", proc, strerror(errno));
                sigqueue(getppid(), SIGCHLD, sv);   /* do not confuse with getpid */
                exit(-1);
            }
#if !defined(MY_UNIT_TEST)
            if (execvp(proc, (char *const *)iwmh) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"iwmh %d exec failed: %s\n", i, strerror(errno));
                sigqueue(getppid(), SIGCHLD, sv);
                exit(-1);
            }
#endif
        } else {
            /* parent pid (i2v) wait for individual instances of iwmh */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v waiting for iwmh on radio%d to start up\n", i);
            wsu_wait_at_gate(&shmPtr->cfgData.iwmhGate);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v done waiting for iwmh\n");
        }
    }
}

static void i2vTermWMH(void)
{
    uint8_t i;
    iwmhVars *radio = &cfg.radio0wmh;
    union sigval sv;

    if (!cfg.iwmhAppEnable) {
        return;
    }

    memset(&sv, 0, sizeof(sv));
    sv.sival_int = 0;
    for (i=0; i<NUM_RADIOS; radio++, i++) {
        if (!radio->radioEnable) {
            continue;
        }
        if (radio->pid != 0) {
            sigqueue(radio->pid, SIGTERM, sv); /* terminate politely first... */
            i2vWaitFor(1);
            /* pid never updated so just run this just in case */
            kill(radio->pid, SIGKILL);
        }
    }
}

/* Function to set up signal handlers */
static void i2vSetupSigHandler(void (*handler)(int32_t, siginfo_t *, void *), int32_t sig)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    // MFM QNX port SA_RESTART not supported on QNX act.sa_flags = SA_SIGINFO|SA_RESTART;
    act.sa_flags = SA_SIGINFO;
    sigaction(sig, &act, NULL);
}

/* This is because sleep() was not waiting for the timeout period. 
 * The cause is unknown and unexplored. Possible cause was several 
 * signal handlers were in effect and catching signals.
 * Copied from v2v-i which copied from wsu_main (or maybe vsca - who knows)
 */
static void i2vWaitFor(uint32_t secs)
{
    struct timeval tt1, tt2;

    gettimeofday( &tt1, NULL );
    gettimeofday( &tt2, NULL );

    while ((tt2.tv_sec - tt1.tv_sec) < secs) {
        gettimeofday(&tt2, NULL);
        usleep(1000);
    }
}

/* function to terminate processes spawned from i2v */
static void i2vChildTerminator(void)
{
    union sigval val;
    uint32_t i, j;

    val.sival_int = 0;
    j = 0;
    for (i = 0; i < NUMITEMS(processes); i++) {
        if (*processes[i].pid != 0) {
            sigqueue(*processes[i].pid, SIGTERM, val); /* terminate politely first... */
            j++;
        }
    }
    i2vWaitFor(3);
    for (i = 0; i < NUMITEMS(processes); i++) {
        if (*processes[i].pid) {
            kill(*processes[i].pid, SIGKILL);
        }
    }
}

/* SIGCHLD handler */
static void i2vChildTermHandler(int32_t GCC_UNUSED_VAR a, siginfo_t *b, void GCC_UNUSED_VAR *c)
{
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"child gone (%d): (a) pid=%d, ex=%d\n", i2vUtilGetUTCTimeInSec(), b->si_pid, b->si_status);
#endif
    if (mainloop) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"child gone: (a) pid=%d, ex=%d\n", b->si_pid, b->si_status);
    }
    if (cfg.termOnChildExit) {
        mainloop = WFALSE;
    } else if (mainloop) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"child gone (pid %d) - config set up to keep operation - system DEGRADED\n", b->si_pid);
    }
}

/* 20140826 - this was copied from v2vi - unclear what external to the
   app sets this signal - no references in a search through svn - it
   may not be used ever besides the block in the term handler */
static volatile sig_atomic_t fatal_error_in_progress = 0;

/* SIGINT, SIGTERM handler */
static void i2vTermHandler(int32_t a, siginfo_t *b, void GCC_UNUSED_VAR *c)
{
    if (mainloop) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "i2v terminated: s(%d) %d\n", b->si_signo, b->si_pid);
#endif
        mainloop = WFALSE;
    }
    if (fatal_error_in_progress == 0) {
        fatal_error_in_progress = 1;
        raise(a);
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2v terminated: raise(%d)\n",a);
#endif
        mainloop = WFALSE;
    }
}

/* shared memory cleanup when process terminated */
static void i2vCleanShm(void)
{
    /* semaphore cleanup */
    wsu_shmlock_kill(&shmPtr->i2vPids.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->cfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->scsSpatData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->scsCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->fwdmsgCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->spatCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->srmCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->ipbCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->amhCfgData.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->amhImmediatePsm.h.ch_lock);
    wsu_shmlock_kill(&shmPtr->amhImmediateBsm.h.ch_lock);
    wsu_kill_gate(&shmPtr->scsSpatData.spatTriggerGate);
    wsu_kill_gate(&shmPtr->cfgData.iwmhGate);
    wsu_kill_gate(&shmPtr->scsSRMData.srmTriggerGate);

    /* remove shared mem */
    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));
    shmPtr = NULL;
    wsu_share_delete(I2V_SHM_PATH);
}
STATIC uint32_t i2vSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
       I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"i2vSetConfDefault: NULL input.\n");
       ret = I2V_RETURN_NULL_PTR;
  } else {
      if(0 == strcmp("I2VUnifiedChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTransmitPower",tag)) {
          *(float32_t *)itemToUpdate = I2V_TX_POWER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%f)\n",tag,*(float32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRSUID",tag)) {
          *(uint16_t *)itemToUpdate = I2VRSUID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSCSAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_SCS_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSRMAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_SRM_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VIPBAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_IPB_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VAMHAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_AMH_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VIWMHAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_IWMH_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VFWDMSGAppEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_FWDMSG_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGlobalDebugFlag",tag)) {
          *(uint8_t *)itemToUpdate = I2V_GLOBAL_DEBUG_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VDebugFlag",tag)) {
         *(uint8_t *)itemToUpdate = I2V_DEBUG_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGPSOverrideEnable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_GPS_OVERRIDE_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGPSOverrideLat",tag)) {
         *(float64_t *)itemToUpdate = I2V_GPS_LAT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%f)\n",tag,*(float64_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGPSOverrideLong",tag)) {
         *(float64_t *)itemToUpdate = I2V_GPS_LON_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%f)\n",tag,*(float64_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGPSOverrideElev",tag)) {
         *(uint16_t *)itemToUpdate = I2V_GPS_ELEV_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("I2VEnableEthIPv6",tag)) {
          *(uint8_t *)itemToUpdate = I2V_ENABLE_IPV6_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VEthIPv6Addr",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_ETH_IPV6_ADDR_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VEthIPv6Scope",tag)) {
          *(uint8_t *)itemToUpdate = I2V_ETH_IPV6_SCOPE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VIPv6DefaultGateway",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_IPV6_GATEWAY_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VForceAlive",tag)) {
          *(bool_t *)itemToUpdate = I2V_FORCE_ALIVE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("I2VControlRadio",tag)) {
          *(uint8_t *)itemToUpdate = I2V_CONTROL_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadioSecVerify",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_SEC_VERIFY_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio0Enable",tag)) {
           *(bool_t *)itemToUpdate = I2V_RADIO0_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio0Mode",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO0_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio0Channel",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio1Enable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO1_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio1Mode",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO1_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio1Channel",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio2Enable",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO2_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio2Mode",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO2_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio2Channel",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio3Enable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO3_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio3Mode",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO3_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadio3Channel",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VBroadcastUnified",tag)) {
          *(uint8_t *)itemToUpdate = I2V_BROADCAST_UNIFIED_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VUnifiedRadioNumber",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VUnifiedWSAPriority",tag)) {
          *(uint8_t *)itemToUpdate = I2V_WSA_UNIFIED_PRIORITY_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VEnableVOD",tag)) {
         *(uint8_t *)itemToUpdate = I2V_ENABLE_VOD_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSecurityVTPMsgRateMs",tag)) {
         *(uint32_t *)itemToUpdate = I2V_SECURITY_VTP_MSG_RATE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSecurityEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_SECURITY_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSecurityTimeBuffer",tag)) {
         *(uint8_t *)itemToUpdate = I2V_SEC_TIME_BUFFER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VCertAttachRate",tag)) {
         *(uint8_t *)itemToUpdate = I2V_CERT_ATTACH_RATE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VGenDefaultCert",tag)) {
         *(uint8_t *)itemToUpdate = I2V_GEN_DEFAULT_CERTS_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VAutoGenCertRestart",tag)) {
         *(uint8_t *)itemToUpdate = I2V_AUTO_GEN_CERT_RESTART_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VStorageBypass",tag)) {
         *(uint8_t *)itemToUpdate = I2V_STORAGE_BYPASS_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VDisableSysLog",tag)) {
         *(uint8_t *)itemToUpdate = I2V_DISABLE_SYSLOG_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VConsoleCleanup",tag)) {
         *(uint8_t *)itemToUpdate = I2V_CONSOLE_CLEANUP_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSPATSecPSID",tag)) {
         *(uint32_t *)itemToUpdate = I2V_SPAT_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VMAPSecPSID",tag)) {
         *(uint32_t *)itemToUpdate = I2V_MAP_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTIMSecPSID",tag)) {
         *(uint32_t *)itemToUpdate = I2V_TIM_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VIPBSecPSID",tag)) {
         *(uint32_t *)itemToUpdate = I2V_IPB_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSRMPSID",tag)) {
         *(uint32_t *)itemToUpdate = SRMRX_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("BSMRxPSIDDER",tag)) {
         *(uint32_t *)itemToUpdate = I2V_BSM_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("BSMRxPSIDPERA",tag)) {
         *(uint32_t *)itemToUpdate = I2V_BSM_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("BSMRxPSIDPERU",tag)) {
         *(uint32_t *)itemToUpdate = I2V_BSM_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("I2VDirectASCSNMP",tag)) {
         *(uint8_t *)itemToUpdate = I2V_SNMP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VWSAEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_WSA_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VRadioType",tag)) {
         *(uint8_t *)itemToUpdate = I2V_RADIO_TYPE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSPATSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_SPAT_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSPATBITMAPPEDSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSPATSspString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_SPAT_SSP_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSPATSspMaskString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_SPAT_SSP_MASK_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VMAPSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_MAP_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VMAPBITMAPPEDSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VMAPSspString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_MAP_SSP_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VMAPSspMaskString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_MAP_SSP_MASK_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTIMSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_TIM_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTIMBITMAPPEDSSPEnable",tag)) {
         *(uint8_t *)itemToUpdate = I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTIMSspString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_TIM_SSP_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VTIMSspMaskString",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_TIM_SSP_MASK_STRING_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VUseDeviceID",tag)) {
         *(bool_t *)itemToUpdate = I2V_USE_DEVICE_ID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("I2VUseGPSTXControl",tag)) {
         *(bool_t *)itemToUpdate = I2V_USE_GPS_TX_CONTROL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("I2VBroadcastingTimeShift",tag)) {
           *(bool_t *)itemToUpdate = I2V_BROADCAST_TIMESHIFT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("I2VInterfaceLogMask",tag)) {
         *(uint8_t *)itemToUpdate = I2V_IFACE_LOG_MASK_DEFAULT ;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("I2VSecurityDirectory",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_SECURITY_DIR_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("I2VConfigTimeSetting",tag)) {
          dbgTime.epochsecs = I2V_DEFAULT_UTC_SECONDS;
          dbgTime.epochusecs = I2V_DEFAULT_UTC_USECONDS;
      } else if(0 == strcmp("I2VConfigTimeMode",tag)) {
         *(uint8_t *)itemToUpdate = I2V_CONFIG_TIME_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (0x%x)\n",tag,*(uint8_t *)itemToUpdate);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) item is not known, ignoring.\n",tag);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
static uint32_t i2vUpdateConfig(char_t * directory, char_t *fname, char_t *path,  pid_t *pid)
{
#define CFG_FILE_STR_LEN  (I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN + 1) /* filename + directory + null term */
    cfgItemsTypeT cfgItems[] = {
        {"I2VRSUID",                (void *)i2vUtilUpdateUint16Value,&cfg.i2vrsuID, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSCSAppEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.scsAppEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSRMAppEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.srmAppEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VIPBAppEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.ipbAppEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VAMHAppEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.amhAppEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VIWMHAppEnable",        (void *)i2vUtilUpdateWBOOLValue, &cfg.iwmhAppEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VFWDMSGAppEnable",      (void *)i2vUtilUpdateWBOOLValue, &cfg.fwdmsgAppEnable, NULL,(ITEM_VALID| UNINIT)},
#ifdef RSE_SIMULATOR
        {"I2VSPSAppEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.spsAppEnable, NULL,(ITEM_VALID| UNINIT)},
#endif /* RSE_SIMULATOR */
        {"I2VGlobalDebugFlag",      (void *)i2vUtilUpdateWBOOLValue, &cfg.globalDebugFlag, NULL,(ITEM_VALID| UNINIT)},
        {"I2VDebugFlag",            (void *)i2vUtilUpdateWBOOLValue, &cfg.i2vDebugEnableFlag, NULL,(ITEM_VALID| UNINIT)},
        {"I2VGPSOverrideEnable",    (void *)i2vUtilUpdateWBOOLValue, &cfg.enableGPSOverride, NULL,(ITEM_VALID| UNINIT)},
        {"I2VGPSOverrideLat",       (void *)i2vUtilUpdateDoubleValue,&cfg.gpsOvrLat, NULL,(ITEM_VALID| UNINIT)},
        {"I2VGPSOverrideLong",      (void *)i2vUtilUpdateDoubleValue,&cfg.gpsOvrLong, NULL,(ITEM_VALID| UNINIT)},
        {"I2VGPSOverrideElev",      (void *)i2vUtilUpdateInt16Value, &cfg.gpsOvrElev, NULL,(ITEM_VALID| UNINIT)},
        {"I2VConfigTimeMode",       (void *)i2vUtilUpdateUint8Value, &cfg.cfgTimeMode, NULL,(ITEM_VALID| UNINIT)},
        {"I2VConfigTimeSetting",    (void *)NULL,                    &dbgTime, (void *)i2vUtilUpdateTimeStr,(ITEM_VALID| UNINIT)},
        {"I2VEnableEthIPv6",        (void *)i2vUtilUpdateWBOOLValue, &cfg.ethIPv6Enable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VEthIPv6Scope",         (void *)i2vUtilUpdateUint8Value, &cfg.ethIPv6Scope, NULL,(ITEM_VALID| UNINIT)},
        {"I2VEthIPv6Addr",          (void *)i2vUtilUpdateStrValue,   &cfg.i2vipv6addr, NULL,(ITEM_VALID| UNINIT)},
        {"I2VIPv6DefaultGateway",   (void *)i2vUtilUpdateStrValue,   &cfg.i2vipv6gwaddr, NULL,(ITEM_VALID| UNINIT)},
        {"I2VForceAlive",           (void *)i2vUtilUpdateWBOOLValue, &cfg.termOnChildExit, NULL,(ITEM_VALID| UNINIT)},
        {"I2VBroadcastingTimeShift",(void *)i2vUtilUpdateWBOOLValue, &cfg.bcastTimeShift, NULL,(ITEM_VALID| UNINIT)},
        {"I2VWSAEnable",            (void *)i2vUtilUpdateWBOOLValue, &cfg.wsaEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VControlRadio",         (void *)i2vUtilUpdateUint8Value, &cfg.cchradio, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadioSecVerify",       (void *)i2vUtilUpdateWBOOLValue, &cfg.iwmhVerifyOD, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio0Enable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.radio0wmh.radioEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio0Mode",           (void *)i2vUtilUpdateWBOOLValue, &cfg.radio0wmh.radioMode, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio0Channel",        (void *)i2vUtilUpdateUint8Value, &cfg.radio0wmh.radioChannel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio1Enable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.radio1wmh.radioEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio1Mode",           (void *)i2vUtilUpdateWBOOLValue, &cfg.radio1wmh.radioMode, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio1Channel",        (void *)i2vUtilUpdateUint8Value, &cfg.radio1wmh.radioChannel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio2Enable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.radio2wmh.radioEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio2Mode",           (void *)i2vUtilUpdateWBOOLValue, &cfg.radio2wmh.radioMode, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio2Channel",        (void *)i2vUtilUpdateUint8Value, &cfg.radio2wmh.radioChannel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio3Enable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.radio3wmh.radioEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio3Mode",           (void *)i2vUtilUpdateWBOOLValue, &cfg.radio3wmh.radioMode, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadio3Channel",        (void *)i2vUtilUpdateUint8Value, &cfg.radio3wmh.radioChannel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VBroadcastUnified",     (void *)i2vUtilUpdateWBOOLValue, &cfg.bcastLockStep, NULL,(ITEM_VALID| UNINIT)},
        {"I2VUnifiedChannelNumber", (void *)i2vUtilUpdateUint8Value, &cfg.uchannel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VUnifiedRadioNumber",   (void *)i2vUtilUpdateUint8Value, &cfg.uradioNum, NULL,(ITEM_VALID| UNINIT)},
        {"I2VUnifiedWSAPriority",   (void *)i2vUtilUpdateUint8Value, &cfg.uwsaPriority, NULL,(ITEM_VALID| UNINIT)},
        {"I2VTransmitPower",        (void *)i2vUtilUpdateFloatValue, &cfg.txPwrLevel, NULL,(ITEM_VALID| UNINIT)},
        {"I2VEnableVOD",       (void *)i2vUtilUpdateWBOOLValue, &cfg.vodEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSecurityVTPMsgRateMs",   (void *)i2vUtilUpdateUint32Value, &cfg.securityVtPMsgRateMs, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSecurityEnable",       (void *)i2vUtilUpdateWBOOLValue, &cfg.security, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSecurityTimeBuffer",   (void *)i2vUtilUpdateUint8Value, &cfg.secStartTO, NULL,(ITEM_VALID| UNINIT)},
        {"I2VCertAttachRate",       (void *)i2vUtilUpdateUint8Value, &cfg.certAttach, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSecurityDirectory",    (void *)i2vUtilUpdateStrValue,   &cfg.securityDir, NULL,(ITEM_VALID| UNINIT)},
        {"I2VGenDefaultCert",       (void *)i2vUtilUpdateWBOOLValue, &cfg.dfltCert, NULL,(ITEM_VALID| UNINIT)},
        {"I2VAutoGenCertRestart",   (void *)i2vUtilUpdateWBOOLValue, &cfg.regenCert, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSPATSecPSID",          (void *)i2vUtilUpdatePsidValue,&cfg.secSpatPsid, NULL,(ITEM_VALID| UNINIT)},
        {"I2VMAPSecPSID",           (void *)i2vUtilUpdatePsidValue,&cfg.secMapPsid, NULL,(ITEM_VALID| UNINIT)},
        {"I2VTIMSecPSID",           (void *)i2vUtilUpdatePsidValue,&cfg.secTimPsid, NULL,(ITEM_VALID| UNINIT)},
        {"I2VIPBSecPSID",           (void *)i2vUtilUpdatePsidValue,&cfg.secIpbPsid, NULL,(ITEM_VALID| UNINIT)},
        {"BSMRxPSIDDER",            (void *)i2vUtilUpdatePsidValue,&cfg.bsmPsidDer, NULL,(ITEM_VALID| UNINIT)},
        {"BSMRxPSIDPERA",           (void *)i2vUtilUpdatePsidValue,&cfg.bsmPsidPerA, NULL,(ITEM_VALID| UNINIT)},
        {"BSMRxPSIDPERU",           (void *)i2vUtilUpdatePsidValue,&cfg.bsmPsidPerU, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSRMPSID",           (void *)i2vUtilUpdatePsidValue,&cfg.srmPsid, NULL,(ITEM_VALID| UNINIT)},
        {"I2VStorageBypass",        (void *)i2vUtilUpdateWBOOLValue, &cfg.nocflash, NULL,(ITEM_VALID| UNINIT)},
        {"I2VDisableSysLog",        (void *)i2vUtilUpdateWBOOLValue, &cfg.nosyslog, NULL,(ITEM_VALID| UNINIT)},
        {"I2VConsoleCleanup",       (void *)i2vUtilUpdateWBOOLValue, &cfg.limitconsoleprints, NULL,(ITEM_VALID| UNINIT)},
#ifdef HSM
        {"I2VSPATSSPEnable",        (void *)i2vUtilUpdateWBOOLValue, &cfg.spatSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSPATBITMAPPEDSSPEnable",(void *)i2vUtilUpdateWBOOLValue, &cfg.spatBitmappedSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSPATSspString",        (void *)i2vUtilUpdateStrValue  , &cfg.spatSsp, NULL,(ITEM_VALID| UNINIT)},
        {"I2VSPATSspMaskString",    (void *)i2vUtilUpdateStrValue  , &cfg.spatSspMask, NULL,(ITEM_VALID| UNINIT)},

        {"I2VMAPSSPEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.mapSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VMAPBITMAPPEDSSPEnable",(void *)i2vUtilUpdateWBOOLValue, &cfg.mapBitmappedSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VMAPSspString",         (void *)i2vUtilUpdateStrValue  , &cfg.mapSsp, NULL,(ITEM_VALID| UNINIT)},
        {"I2VMAPSspMaskString",     (void *)i2vUtilUpdateStrValue  , &cfg.mapSspMask, NULL,(ITEM_VALID| UNINIT)},

        {"I2VTIMSSPEnable",         (void *)i2vUtilUpdateWBOOLValue, &cfg.timSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VTIMBITMAPPEDSSPEnable",(void *)i2vUtilUpdateWBOOLValue, &cfg.timBitmappedSspEnable, NULL,(ITEM_VALID| UNINIT)},
        {"I2VTIMSspString",         (void *)i2vUtilUpdateStrValue  , &cfg.timSsp, NULL,(ITEM_VALID| UNINIT)},
        {"I2VTIMSspMaskString",     (void *)i2vUtilUpdateStrValue  , &cfg.timSspMask, NULL,(ITEM_VALID| UNINIT)},
#endif /* HSM */
        {"I2VDirectASCSNMP",        (void *)i2vUtilUpdateWBOOLValue, &cfg.enableASCSNMP, NULL,(ITEM_VALID| UNINIT)},
        {"I2VRadioType",            (void *)i2vUtilUpdateUint32Value, &cfg.RadioType, NULL,(ITEM_VALID| UNINIT)},
        {"I2VInterfaceLogMask",     (void *)i2vUtilUpdateUint8Value,&cfg.InterfaceLogMask, NULL,(ITEM_VALID| UNINIT)},
        {"I2VUseDeviceID",          (void *)i2vUtilUpdateWBOOLValue, &cfg.i2vUseDeviceID, NULL,(ITEM_VALID| UNINIT)},
        {"I2VUseGPSTXControl",      (void *)i2vUtilUpdateWBOOLValue, &cfg.gpsCtlTx, NULL,(ITEM_VALID| UNINIT)},
    };

    char_t filepath[CFG_FILE_STR_LEN];
    FILE *f;
    uint32_t retVal;
    uint32_t i = 0;
    bool_t overrideFlag = WFALSE;
#ifdef RSE_SIMULATOR
    struct mq_attr mqattr;
#endif

    path = path; /* Silence of the Warnings. */
    pid = pid;

    if ((NULL == directory) || (NULL == fname)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: Null file pointer.\n", __FUNCTION__);
        return I2V_RETURN_NULL_PTR;
    }

    if ((strlen(directory) > I2V_CFG_MAX_STR_LEN) || (strlen(fname) > I2V_CFG_MAX_STR_LEN)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: directory name or filename too long.\n", __FUNCTION__);
        return I2V_RETURN_INVALID_PARAM;
    }

    strcpy(filepath, directory);
    strcat(filepath, fname);
    memset(&dbgTime, 0, sizeof(dbgTime));

    if ((f = fopen(filepath, "r")) == NULL) {
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"I2V_RETURN_INV_FILE_OR_DIR: (%s)\n",filepath);
        return I2V_RETURN_INV_FILE_OR_DIR;
    } 

    if(I2V_RETURN_OK != (retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), overrideFlag, NULL))){
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: not happy i2vRet=%u.\n",retVal);
        //set_my_error_state(I2V_READ_CFG_FAIL);
    }
    fclose(f);

    /* Check IP's or anything else for extra wellness above what I2V_UTIL does. */
    if(I2V_RETURN_OK != i2vCheckIPAddr(cfg.i2vipv6addr, I2V_IP_ADDR_V6)) {
        cfgItems[17].state = (ITEM_VALID|BAD_VAL);
        retVal = I2V_RETURN_INVALID_CONFIG;
    }
    if(I2V_RETURN_OK != i2vCheckIPAddr(cfg.i2vipv6gwaddr, I2V_IP_ADDR_V6)) {
        cfgItems[18].state = (ITEM_VALID|BAD_VAL);
        retVal = I2V_RETURN_INVALID_CONFIG;
    }
    /* Check if conf was missing needed values or had bogus values. Recover and keep going. */      
    for (i = 0; i < NUMITEMS(cfgItems); i++) {
        if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
            || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
            if(I2V_RETURN_OK == (retVal = i2vSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                cfgItems[i].state = (ITEM_VALID|INIT);
            } else {
                break; /* Heal has failed. FATAL. */
            }
        }
    }

    /* cleanup items */
    cfg.ipbAppEnable = (!cfg.wsaEnable) ? cfg.wsaEnable : cfg.ipbAppEnable;

    if (cfg.txPwrLevel > MAX_TX_PWR_LEVEL) {    /* max radio setting */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: power setting is invalid [%.1f]; system cannot start.\n", cfg.txPwrLevel);
        return I2V_RETURN_FAIL;
    } else if (cfg.txPwrLevel < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: power setting is invalid [%.1f]; system cannot start.\n", cfg.txPwrLevel);
        return I2V_RETURN_FAIL;
    } else if (((cfg.txPwrLevel - ((int)cfg.txPwrLevel)) != 0) &&
     ((cfg.txPwrLevel - ((int)cfg.txPwrLevel)) != 0.5)) {
        /* verification decimal is 0.5 increment */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: power setting is not correct increments [%.1f]; rounding down.\n", cfg.txPwrLevel);
        cfg.txPwrLevel = ((int)cfg.txPwrLevel);
    }

    cfg.spatAppEnable = cfg.scsAppEnable;   /* scs controls if spat is enabled or not */

    cfg.dfltCert = (cfg.i2vDebugEnableFlag) ? cfg.dfltCert : cfg.i2vDebugEnableFlag;
    cfg.regenCert = (cfg.dfltCert) ? cfg.regenCert : cfg.dfltCert;
    cfg.termOnChildExit = !cfg.termOnChildExit;   /* config file has the inverse setting */

    /* Check I2V SHM to see if globaldebug enabled.*/
    if(cfg.globalDebugFlag)
        i2vUtilEnableDebug(MY_NAME);
    else
        i2vUtilDisableDebug();

    /* special case */
    if (cfg.i2vUseDeviceID) {
        i2vUtilLoadDeviceID(&cfg);
    }

#ifdef RSE_SIMULATOR
    if ((cfg.spsAppEnable) && (cfg.nocflash)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid config for rse simulator - not enabling simulator\n");
        cfg.spsAppEnable = WFALSE;
    }

    if (cfg.spsAppEnable) {
        /* prepare attr */
        memset(&mqattr, 0, sizeof(mqattr));
        mqattr.mq_flags = DEFAULT_MQ_FLAGS;
        mqattr.mq_maxmsg = DEFAULT_MQ_MAX_MSG;
        mqattr.mq_msgsize = sizeof(sdmMQMsgData);

        /* preparation first (copied from TM); ignoring errno */
        mqSim = mq_open(I2V_SDM_QUEUE, O_WRONLY);
        mq_unlink(I2V_SDM_QUEUE);
        mq_close(mqSim);

        /* make simulator queue now */
        if ((mqSim = mq_open(I2V_SIM_SPAT_QUEUE, DEFAULT_MQ_DESCR_FLAGS, DEFAULT_MQ_PERMS, &attr)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unable to initialize rse simulator - not enabling simulator\n");
            cfg.spsAppEnable = WFALSE;
        }
    }
#endif /* RSE_SIMULATOR */

    /* 
     * Don't run any tasks till I2V CFG SHM valid. SHM not locked here.
     * All tasks are waiting on I2V CFG SHM to become valid.
     */
    memcpy(&shmPtr->cfgData,&cfg,sizeof(cfg));
    shmPtr->cfgData.h.ch_data_valid = WTRUE;

    if (cfg.iwmhAppEnable) {
        if (!i2vIwmhQSetup(WFALSE)) {
            if (!cfg.bcastFB) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"fallback broadcasting not enabled - failing startup\n");
#ifdef RSE_SIMULATOR
                if (cfg.spsAppenable) {
                    mq_unlink(I2V_SIM_SPAT_QUEUE);
                    mq_close(mqSim);
                }
#endif /* RSE_SIMULATOR */
                return I2V_RETURN_FAIL;
            }
            cfg.iwmhAppEnable = WFALSE;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"switching to fallback broadcasting - apps broadcast for themselves\n");
        }
    }
    return retVal;
#undef CFG_FILE_STR_LEN
}
#ifdef RSE_SIMULATOR
static void i2vCloseSystemLogging(pid_t *pid)
{
    pid = pid; /* Silence of the Warnings. */
    if (cfg.nosyslog) {
        /* nothing to do */
        /* see RSE_SIMULATOR note below - this block is newer */
        if (cfg.spsAppenable) {
            mq_unlink(I2V_SIM_SPAT_QUEUE);
            mq_close(mqSim);
        }
        return;
    }

    /* this isn't the most intuitive place to put this code but
       it is the same operation so doing it here */
    if (cfg.spsAppenable) {
        mq_unlink(I2V_SIM_SPAT_QUEUE);
        mq_close(mqSim);
    }
}
#endif

STATIC void i2v_clear_statics(void)
{
  mainloop = WTRUE; 
  return;
}
/* 
 * TRAC #2782: RSU:HERC:IFCLOGGER: can only log one interface at a time.
 * cfg.InterfaceLogMask is a bitmask. We should be able to launch any combination of interfaces to log.
 */
#if defined(ENABLE_PEER_IFC_REQUESTS)
STATIC void i2v_start_ifclog(void)
{
  /* Start any combo of IFCLOGS based off bit field from i2v.conf.*/
  if(IFCLOG_ETH0_ENABLE_BIT & cfg.InterfaceLogMask) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"eth0 IFCLOG enabled.\n");
      ACTIVATE_ETH0_LOGGING;
      sleep(I2V_IFC_START_WAIT_SECS);     
  }
  if(IFCLOG_ETH1_ENABLE_BIT & cfg.InterfaceLogMask) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"eth1 IFCLOG not supported.\n");
  }
  if((IFCLOG_DSRC0_ENABLE_BIT & cfg.InterfaceLogMask) && (0 == cfg.RadioType)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dsrc0 IFCLOG enabled.\n");
      ACTIVATE_DSRC0_LOGGING;
      sleep(I2V_IFC_START_WAIT_SECS);
  }
  if((IFCLOG_DSRC1_ENABLE_BIT & cfg.InterfaceLogMask) && (0 == cfg.RadioType)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dsrc1 IFCLOG not supported.\n");
  }
  if((IFCLOG_CV2X0_ENABLE_BIT & cfg.InterfaceLogMask) && (1 == cfg.RadioType)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x0 IFCLOG enabled.\n");
      ACTIVATE_CV2X0_LOGGING;
      sleep(I2V_IFC_START_WAIT_SECS);
  }
  if((IFCLOG_CV2X1_ENABLE_BIT & cfg.InterfaceLogMask) && (1 == cfg.RadioType)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x1 IFCLOG not supported.\n");
  }
  return;
}

STATIC void i2v_stop_ifclog(void)
{
  if(IFCLOG_ETH0_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_ETH0_LOGGING;
  }
  if(IFCLOG_ETH1_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_ETH1_LOGGING;
  }
  if(IFCLOG_DSRC0_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_DSRC0_LOGGING;
  }
  if(IFCLOG_DSRC1_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_DSRC1_LOGGING;
  }
  if(IFCLOG_CV2X0_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_CV2X0_LOGGING;
  }
  if(IFCLOG_CV2X1_ENABLE_BIT & cfg.InterfaceLogMask) {
      DEACTIVATE_CV2X1_LOGGING;
  }
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IFCLOG closed.\n");
  return;
}
#endif
/* i2v main */
int32_t MAIN(int32_t argc, char_t *argv[])
{
    enum cmdOptionBits {
        MAIN_CFG_PATH = 0x01,
        MAIN_CFG_FILE = 0x02,
        MAIN_CFG_UNUSED = 0x04, /* this val can be hijacked for future use */
    };

    uint8_t i, cmdopts = 0;
    bool_t ignoreCmdlineErr = WFALSE;
    int8_t retVal = I2V_RETURN_OK; /* Only returns 8 bits unsigned. */
    uint32_t numChildrenAlive = 0;
    pid_t pid = -1, loggingPid = -1;
    char_t str1[200];
    char_t path[100];
    int32_t fd = -1, stat1 = -1;
    int32_t dummy = 0;

    i2v_clear_statics();
#if !defined(MY_UNIT_TEST)
    memset(&cfg, 0, sizeof(cfg));
#endif
    memset(str1,'\0',sizeof(str1));
    memset(path,'\0',sizeof(path));

    /* 
     * Pause to allow system to come up and serial debug to come out. 
     */
    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    if ((shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Main shared memory init failed\n");
        return I2V_RETURN_SHM_FAIL;
    }

    if((argc) && (NULL == argv)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"argc==%d but argv == NULL\n",argc);
        return I2V_RETURN_NULL_PTR;
    }

    /* cmdopts is a bitmask - each bit set matches the order of args listed below
       update data type to longer field if more options needed */
    for (i=0; i<argc; i++) {
        if (*argv[i] == '-') {
            switch (argv[i][1]) {
                case 'd': /* config directory */
                    if (strlen(argv[i]) > I2V_CFG_MAX_STR_LEN + 2) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"directory path too long\n");
                        retVal = I2V_RETURN_INVALID_PARAM;
                        break;
                    }
                    strncpy(cfg.config_directory, argv[i] + 2, I2V_CFG_MAX_STR_LEN);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"using config directory: %s\n", cfg.config_directory);
                    cmdopts |= MAIN_CFG_PATH;
                    break;
                case 'f': /* config file name parameter */
                    if (strlen(argv[i]) > I2V_CFG_MAX_STR_LEN + 2) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"filename too long\n");
                        retVal = I2V_RETURN_INVALID_PARAM;
                        break;
                    }
                    strncpy(cfg.config_filename, argv[i] + 2, I2V_CFG_MAX_STR_LEN);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"using config file: %s\n", cfg.config_filename);
                    cmdopts |= MAIN_CFG_FILE;
                    break;
                case 'i': /* ignore command line errors */
                    if (argv[i][2] == 0) {
                        ignoreCmdlineErr = WTRUE;
                    }
                    break;
                default:
                    break;
            }
        }
        if ((!ignoreCmdlineErr) && (retVal != I2V_RETURN_OK)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"command line options invalid, not starting\n");
            return retVal;
        }
    }
#if !defined(MY_UNIT_TEST)
    if ((cmdopts & MAIN_CFG_PATH) != MAIN_CFG_PATH) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"using default config directory\n");
        #endif
        strcpy(cfg.config_directory, I2V_CONF_DIR);
    }
    if ((cmdopts & MAIN_CFG_FILE) != MAIN_CFG_FILE) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"using default config filename\n");
        #endif
        strcpy(cfg.config_filename, I2V_CONF_FILE);
    }
#endif

    /* TODO: wsu vs wsu1.5 - /mnt/rwflash or /mnt/usbdrv - validate below finding if wsu1.5 required */
    /* searching it appears that on wsu1.5 /mnt/usbdrv is also mounted as /mnt/rwflash - so this *should* work as is */
    /* the check was made in mount_externals - init script that calls mount_usb */
    if ((retVal = i2vUpdateConfig(cfg.config_directory, cfg.config_filename, path, &loggingPid)) != I2V_RETURN_OK) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failure loading config, not starting %s%s: ret=%d\n",cfg.config_directory,cfg.config_filename,retVal);
        return retVal;
    }

#if 0
    if (cfg.ipbAppEnable) {
        i2vIPv6Forwarding(WTRUE, &dummy);
    }
#endif
    if (cfg.ethIPv6Enable) {
        i2vIPv6EthSet(WTRUE);   /* activate */
    }

    /* setup SIGTERM, SIGINT, SIGCHLD handlers */
    i2vSetupSigHandler(i2vTermHandler, SIGTERM);
    i2vSetupSigHandler(i2vTermHandler, SIGINT);
    i2vSetupSigHandler(i2vTermHandler, SIGKILL);  /* adding for 5912; masked off by systemd startup.sh call? */
    i2vSetupSigHandler(i2vChildTermHandler, SIGCHLD);

    /* if any of these inits fail should we consider not starting? */
    if (!wsu_shmlock_init(&shmPtr->i2vPids.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for i2vpids %d %s\n", errno, strerror(errno)); 
    }
    if (!wsu_shmlock_init(&shmPtr->cfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for cfgdata \n");
    }
    if (cfg.enableTimeOverride) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"debug config time enabled - overriding system time\n");
        /* this call creates a new thread - needs cfgData sem inited
           and does not generate SIGCHLD */
        timeMgrActivate(I2V_SHM_PATH, &dbgTime, cfg.cfgTimeMode);
    }
    if (!wsu_shmlock_init(&shmPtr->scsSpatData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for scsSpatdata \n");
    }
    if (!wsu_shmlock_init(&shmPtr->scsCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for scsCfgdata \n");
    }
    if (!wsu_shmlock_init(&shmPtr->fwdmsgCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," sem init failed for fwdmsgCfgdata \n");
    }
    if (!wsu_shmlock_init(&shmPtr->spatCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for spatCfgdata %d %s\n", errno, strerror(errno)); 
    }
    if (!wsu_shmlock_init(&shmPtr->ipbCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for ipbCfgData \n");
    }
    if (!wsu_shmlock_init(&shmPtr->amhCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for amhCfgData \n");
    }
    if (!wsu_shmlock_init(&shmPtr->amhImmediatePsm.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for amhImmediatePsr \n");
    }
    if (!wsu_shmlock_init(&shmPtr->amhImmediateBsm.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for amhImmediateBsm \n");
    }
    if (!wsu_shmlock_init(&shmPtr->srmCfgData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for srm16_CfgData.\n");
        return I2V_RETURN_SHM_FAIL;
    }
    if (!wsu_shmlock_init(&shmPtr->scsSRMData.h.ch_lock)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sem init failed for scsSRMData \n");
    }
    wsu_init_gate(&shmPtr->scsSpatData.spatTriggerGate);
    wsu_init_gate(&shmPtr->cfgData.iwmhGate);
    wsu_init_gate(&shmPtr->scsSRMData.srmTriggerGate);
    i2vPids.i2v = getpid();
    if(0x0 != i2v_Util_Open_Queue()){/* Open queues in one place. */
        I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "i2v_Util_Open_Queue had issue: ret(0x%x).\n");
    }
//TODO: In lieu of rsudiag shm handshake , wait. If we launch iwmh too soon then CV2X not ready in rsudiag shm.
    sleep(10);

    /* Start the iwmh irrespective of the security status */
    i2vInitWMH(path);
    /* Start the individual applications */
    for (i = 0,errno=0; i < NUMITEMS(processes); i++) {
        if (*processes[i].appEnableFlag == WTRUE) {
            numChildrenAlive++;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, " Start process(%d).\n",i);
            sleep(5); /* Slow launch so we can see debug output */
#endif

#if !defined(MY_UNIT_TEST)
            if ((pid = fork()) == -1) {
#else
            if((pid = i+1) == -1) {
#endif
                I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"fork() failed.\n");
                return 1;
            } else if (pid == 0) {
                I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"i2v (%d) executing %s (%d). errno=%d.\n", getppid(), processes[i].pname, getpid(),errno);
                if (!strcmp(processes[i].pname, SECURITY_PROC)) {     /* FIXME: there could be other programs called that aren't
                                                                        local i2v apps - may need a flag in the struct for this */
                    sprintf(str1, "%s%s", DEFAULT_BIN_PATH, *(char *const *)processes[i].cmd);
                } else {
                    sprintf(str1, "%s%s", path, *(char *const *)processes[i].cmd);
                }
                if ((strlen(path) > 0) && (fd = open(str1, O_RDONLY)) == -1) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Cannot run cmd: %s (%s/%d)\n", str1, strerror(errno), errno);
                    return 1;
                }
#if !defined(MY_UNIT_TEST)
                if (execvp(str1, (char *const *)processes[i].cmd) == -1) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s)\n",processes[i].pname);
                    return 1;
                }
#endif
                return 0;       /* this code is never reached. */
            } else {
                *processes[i].pid = pid;
#if 0
/* FIXME (210309 JJG): I have no clue why i originally wrote starting iwmh as a follow up to launching
   a child process (and doing it for each task ???); but for now, disabling this block of code.
   This implementation is at least 6 years old and i don't think applies to the 5912.
*/
                i2vInitWMH(path);
                /* this block could also go in !WSU_5001 but no harm on 5001 */
                if (!strcmp(processes[i].pname, SECURITY_PROC) && cfg.secStartTO) {
                    /* give it time to start up */
                    usleep(cfg.secStartTO * 100000);
                }
                if (cfg.iwmhAppEnable) {
                    /* stagger time hard coded in for child apps - don't want them
                       all making service requests at the same time (if possible) */
                    usleep(200000);   /* will be more than 2 deciseconds */
                }
#endif
            }
        }
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"number of processes loaded is %d.\n", numChildrenAlive);
    /* for SDK file completion */
    dummy = (void *)&shmPtr->scsSpatData.spatSelector - (void *)shmPtr;
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"S: %d \n", dummy);
    dummy = (void *)shmPtr - (void *)&shmPtr->ipbCfgData;
    dummy += sizeof(i2vShmMasterT);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d\n", dummy);

    WSU_SEM_LOCKW(&shmPtr->i2vPids.h.ch_lock);
    shmPtr->i2vPids.i2v = i2vPids.i2v;
    shmPtr->i2vPids.h.ch_data_valid = WTRUE;
    WSU_SEM_UNLOCKW(&shmPtr->i2vPids.h.ch_lock);

#if defined(ENABLE_PEER_IFC_REQUESTS)
    /* Start IFCLOGS. If it fails we don't care. Not critical system. */
    if((WFALSE == cfg.nosyslog) && cfg.InterfaceLogMask) {
        i2v_start_ifclog();
    }
#endif

#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Enter mainloop = %d.\n",mainloop);
#endif

    /* wait here until terminated */
    while (mainloop) {
        usleep(500000);    /* 500 ms */
        #if defined(MY_UNIT_TEST)
        break; /* Make sure child threads exit first if any */
        #endif
    }
#if defined(ENABLE_PEER_IFC_REQUESTS)
    /* Close IFCLOGS. */
    if((WFALSE == cfg.nosyslog) && cfg.InterfaceLogMask) {
        i2v_stop_ifclog();
    }
#endif
    i2vChildTerminator();
    for (i = 0; i < NUMITEMS(processes); i++) {
        if (*processes[i].pid > 0) {
#if defined(EXYTA_DEBUG)
            I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"waiting for process %d (%s) ... \n", *processes[i].pid, processes[i].pname);
#endif
            waitpid(*processes[i].pid, &stat1, 0);
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,">>>> %s\n", processes[i].pname);
#endif
        }
    }

    i2v_Util_Close_Queue(); /* Unlink & close queues in one place. */

    /* if this is enabled, turn it off BEFORE stopping system log */
    if (cfg.enableTimeOverride) {
        timeMgrDeactive();
    }

#ifdef RSE_SIMULATOR
    /* Checks cfg.nosyslog itself. */
    i2vCloseSystemLogging(&loggingPid);
#endif

#if 0
    if (cfg.ipbAppEnable) {
        i2vIPv6Forwarding(WFALSE, &dummy);
    }
#endif
    if (cfg.ethIPv6Enable) {
        i2vIPv6EthSet(WFALSE);   /* deactivate */
    }
    if (cfg.iwmhAppEnable) {
        i2vTermWMH();
        i2vIwmhQSetup(WTRUE);   /* cleanup - don't care about return */
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%d) exiting: ret=%d\n", getpid(),retVal);
#endif
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    i2vCleanShm();
    sync();
    return abs(retVal);
}
