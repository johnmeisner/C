/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rsk.c                                                            */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Interface between Radio Services and the applications         */
/*              requesting service from it.                                   */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include "rpsi.h"
#include "wme.h"
#include "type2str.h"
#include "shm_rsk.h"
#include "wsu_sharedmem.h"
#include "ns_cv2x.h"
#include "nscfg.h"
#include "alsmi.h"
#include "ipcsock.h"
#include "rsu_defs.h"
#include "wsu_util.h"
#include "ns_pcap.h"
#include "i2v_util.h"
#include "nscfg.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN    rsk_main
#else
#define MAIN    main
#endif
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "rsk"

extern ApplicationStatusTableType   AST;
extern bool_t                       cv2x_started;

extern void initWme(uint32_t debugEnable);
extern void initNsCfg(uint32_t debugEnable);
extern void initCv2x(uint32_t debugEnable);
extern void initRpsi(uint32_t debugEnable);
#if defined(ENABLE_HSM_DAEMON)
extern void *densoDaemon(void *arg);
#endif
extern rskStatusType cv2xStatus;
/*----------------------------------------------------------------------------*/
/* Global Variables                                                           */
/*----------------------------------------------------------------------------*/
shm_rsk_t *shm_rsk_ptr           = NULL;
bool_t     mainLoop              = TRUE;
int        risSocket             = -1;
uint8_t    secmode               = 1;
bool_t     p2pEnabled            = TRUE;
bool_t     hdr1609p2             = TRUE;
bool_t     dummy_verify_succeeds = FALSE;

/*----------------------------------------------------------------------------*/
/* Local Variables                                                           */
/*----------------------------------------------------------------------------*/
bool_t cv2x_module_inited           = FALSE;
static bool_t wsa_status_table_initialized = FALSE;
static bool_t alsmiInitialized             = FALSE;
static bool_t receiveCommandsThreadCreated = FALSE;
static bool_t pcapInited                   = FALSE;
#if defined(ENABLE_HSM_DAEMON)
static pthread_t OneSecThrdID;
#endif
/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/**-----------------------------------------------------------------------------
** Function:  rsk_init_module
** @brief  Initialize radioServices at start-up.
** @return 0 if success, -1 if failure
**/
static int rsk_init_module(void)
{
    int              result;
    int              i;
    rsResultCodeType risRet;
    struct timeval   currTime;

    /* Initialize the random number seed */
    gettimeofday(&currTime, NULL);
    srandom(currTime.tv_usec);

    /* Create the socket for connecting to RIS */
    risSocket = wsuConnectSocket(-1 /* port ignored */);

    if (risSocket < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuConnectSocket() failed\n");
        goto rsk_init_module_err;
    }

    /* Parse the /rwflash/configs/nsconfig.conf file, except for the EDCA
     * parameters; must be done before DSRC is initialized */
    nsParseConfigFile1();

    /* Create the Radio Services shared memory region */
    shm_rsk_ptr = (shm_rsk_t *)wsu_share_init(sizeof(shm_rsk_t), RSK_SHM_PATH);

    if (shm_rsk_ptr == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to create Radio_ns shared memory area\n");
        goto rsk_init_module_err;
    }

    /* Clear out the shared memory counters */
    memset(shm_rsk_ptr, 0, sizeof(shm_rsk_t));
    shm_rsk_ptr->bootupState = 0x12340000;

    /* Initialize the wme.c global variables */
    for (i = 0; i < MAX_APPS; i++) {
        AST.ASTEntry[i].index = i;
    }

    /* Initialize the WSA status table */
    initWSAStatusTable();
    wsa_status_table_initialized = TRUE;
    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x1; // 1 -> WSA status table initialized
#ifndef NO_SECURITY

    /* If security is enabled, bring up Aerolink */
    if (secmode != 0) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling alsmiInitialize()\n");
#endif
        if (alsmiInitialize() != 0) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"alsmiInitialize() failed: verfication & signing off.\n");
#endif
            cv2xStatus.error_states |= CV2X_ALSMI_INIT_FAIL;
            secmode = 0;
        }
        else {
            alsmiInitialized = TRUE;
        }
    }

#else
    alsmiInitialized = FALSE;
    secmode = 0;
#endif
    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x2; // 2 -> ALSMI initialized
    /* Initialize PCAP */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pcap_init()\n");
#endif
    if (pcap_init() < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pcap_init() failed\n");
        cv2xStatus.error_states |= CV2X_PCAP_INIT_FAIL;
        goto rsk_init_module_err;
    }

    pcapInited = TRUE;
    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x3; // 3 -> PCAP initialized

    /* Launch the receive commands thread */
    risRet = createReceiveCommandsThread();

    if (risRet != RS_SUCCESS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error creating receive commands thread: %s\n",
                   rsResultCodeType2Str(risRet));
        cv2xStatus.error_states |= CV2X_COMM_THREAD_FAIL;
        goto rsk_init_module_err;
    }

    receiveCommandsThreadCreated = TRUE;
    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x4; // 4 -> WSA status table initialized

    /* Always initialize the C-V2X module */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling cv2x_module_init()\n");
#endif
    result = rs_cv2x_module_init();

    if (result < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_module_init() failed\n");
        goto rsk_init_module_err;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_module_init() succeeded\n");
#endif
    cv2x_module_inited = TRUE;

    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x7; // 7 -> C-V2x module initialized

    /* Start the C-V2X subsystem if enabled */
    if (getCV2XEnable()) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling start_cv2x()\n");
#endif
        result = start_cv2x();

        if (result < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"start_cv2x() failed\n");
            goto rsk_init_module_err;
        }
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"start_cv2x() succeeded\n");
#endif
        cv2x_started = TRUE;
    }

    shm_rsk_ptr->bootupState &= ~0xf;
    shm_rsk_ptr->bootupState |= 0x8; // 8 -> C-V2X started
    return 0;

rsk_init_module_err:
    I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"rsk_init_module() failed: bootupState(0x%x)\n",shm_rsk_ptr->bootupState);
    if (receiveCommandsThreadCreated) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling terminateReceiveCommandsThread()\n");
#endif
        terminateReceiveCommandsThread();
        receiveCommandsThreadCreated = FALSE;
    }

    if (pcapInited) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pcap_term()\n");
#endif
        pcap_term();
    }

#ifndef NO_SECURITY
    if (alsmiInitialized) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling alsmiTerm()\n");
#endif
        alsmiTerm();
    }

#endif
    if (wsa_status_table_initialized) {
        deinitWSAStatusTable();
    }

    if (cv2x_started) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling stop_cv2x()\n");
#endif
        stop_cv2x();
        cv2x_started = FALSE;
    }

    /* Terminate the C-V2X module */
    if (cv2x_module_inited) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling cv2x_module_term()\n");
#endif
        rs_cv2x_module_term();
        cv2x_module_inited = FALSE;
    }

    if (shm_rsk_ptr != NULL) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling wsu_share_kill(shm_rsk_ptr, sizeof(*shm_rsk_ptr))\n");
#endif
        wsu_share_kill(shm_rsk_ptr, sizeof(*shm_rsk_ptr));
        shm_rsk_ptr = NULL;
    }

    if (risSocket >= 0) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Closing risSocket\n");
#endif
        close(risSocket);
    }
    cv2xStatus.error_states |= CV2X_RSK_INIT_FAIL;
    return -1;
}

/**-----------------------------------------------------------------------------
** Function:  rsk_exit_module
** @brief  Do radioServices termination at shutdown.
** @return 0 if success, -1 if failure
**/
static int rsk_exit_module(void)
{
    /* Terminate the receive commands thread. Don't bother looking at the
     * return code, since if an error occurs the routine will print an error
     * message, and there is nothing we could do about it anyway. */
    if (receiveCommandsThreadCreated) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling terminateReceiveCommandsThread(DEV_ID0)\n");
#endif
        terminateReceiveCommandsThread();
    }

#ifndef NO_SECURITY
    /* Kill off security */
    if (alsmiInitialized) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling alsmiTerm()\n");
#endif
        alsmiTerm();
    }

#endif
    if (wsa_status_table_initialized) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling deinitWSAStatusTable()\n");
#endif
        deinitWSAStatusTable();
    }

    /* Stop the C-V2X subsystem */
    if (cv2x_started) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling stop_cv2x()\n");
#endif
        stop_cv2x();
    }

    /* Terminate the C-V2X module */
    if (cv2x_module_inited) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling cv2x_module_term()\n");
#endif
        rs_cv2x_module_term();
    }

    /* Kill off the Radio Services shared memory region */
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling wsu_share_kill(shm_rsk_ptr, sizeof(shm_rsk_t))\n");
#endif
    wsu_share_kill(shm_rsk_ptr, sizeof(shm_rsk_t));

    /* Close the RIS socket */
    if (risSocket >= 0) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Closing risSocket\n");
#endif
        close(risSocket);
    }
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    return 0;
}

/**
** Function:  rskExitHandler
** @brief  Terminate signal handler
** @param  signal -- [input]The signal that resulted in this function call
** @return 0 if success, -1 if failure
**
** Details: This routine is called when the SIGUSR1, SIGINT, SIGTERM, or
**          SIGKILL signal is received. It sets mainLoop to FALSE so that the
**          main loop will exit, except in the case of SIGUSR1, which is used
**          only to terminate threads, not radioServices itself.
**/
void rskExitHandler(int signal)
{
    /* Ignore SIGUSR1; it is only used to terminate threads, not radioServices
     * itself. */
    if (signal != SIGUSR1) {
#if defined(EXTRA_DEBUG)
        if (mainLoop) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Called\n");
        }
#endif
        mainLoop = FALSE;
    }
}

/**
** Function: rskParseArgs
** @brief    Parse the command line arguments.
** @brief    Currently, only --ver is supported
** @param    argc from main
** @param    argv from main
** @return   TRUE for success. FALSE for error.
**/
static bool_t rskParseArgs(int argc, char *argv[])
{
    int32_t  i = 1; 
    uint32_t j;
    uint32_t temp;
    uint16_t index;

    const char *pCommandArray[] =
    {
        "SECMODE",
        "P2P",
        "HDR1609P2",
        "DUMMY_VERIFY_SUCCEEDS",
    };

    /* Have MAX_CMD_BUF be the sze of the longest entry in pCommandArray */
    #define MAX_CMD_BUF sizeof("DUMMY_VERIFY_SUCCEEDS")
    #define MAX_CMDS    (sizeof(pCommandArray) / sizeof(pCommandArray[0]))
    char tempBuf[MAX_CMD_BUF];

    while (i < argc) {
        /* Be sure parameter begins with "--" */
        if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
            return FALSE;
        }

        memset(tempBuf, 0, MAX_CMD_BUF);
        /* 
         * TRAC 2711: [rsk.c:402]: (error) Array 'tempBuf[22]' accessed at index 22
         * Get the parameter name, converted to all upper case, in tempBuf 
         */
        for (j = 0; (j < (MAX_CMD_BUF-1)) && (j < strlen(&argv[i][2])); j++) {
            tempBuf[j] = (char)toupper((int)(argv[i][j + 2]));
        }

        tempBuf[j] = '\0';

        /* Find where the command is in pCommandArray */
        for (index = 0; index < MAX_CMDS; index++) {
            if (strncmp((const char *)pCommandArray[index],
                        (const char *)tempBuf,
                        strlen(pCommandArray[index])) == 0) {
                break;
            }
        }

        /* Error if not found */
        if (index == MAX_CMDS) {
            return FALSE;
        }

        /* Point to the value; error if command specified without a value */
        if (++i >= argc) {
            return FALSE;
        }

        /* Convert the value to an unsigned long integer */
        temp = strtoul(argv[i], NULL, 0);

        /* Process the parameter */
        switch (index) {
        case 0:     // SECMODE
            if (temp > 1) {
                I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"--secmode must be from 0 to 1 (%u specified)\n", temp);
                return FALSE;
            }

            secmode = temp;
            break;

        case 1:     // P2P
            p2pEnabled = (temp == 0) ? FALSE : TRUE;
            break;

        case 2:     // HDR1609P2
            hdr1609p2 = (temp == 0) ? FALSE : TRUE;
            break;

        case 3:     // DUMMY_VERIFY_SUCCEEDS
            dummy_verify_succeeds = (temp == 0) ? FALSE : TRUE;
            break;

        default:
            return FALSE;
        } // switch (index)

        /* Move past the value to the next parameter, if any */
        i++;
    } // while (i < argc)

//todo: runtime check that isn't ready in time here.
    /* See nsconfig.conf::I2V_SYSLOG_ENABLE to enable/disable output at runtime */
    if (0x1 == getI2vSyslogEnable()) {
        i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME);
        i2vUtilEnableDebug(MY_NAME);
    } else {
        i2vUtilDisableSyslog();
        i2vUtilDisableDebug();
    }

    initWme(getI2vSyslogEnable());
    initNsCfg(getI2vSyslogEnable());
    initCv2x(getI2vSyslogEnable());
    initRpsi(getI2vSyslogEnable());
    return TRUE;
}

/**
** Function:  main
** @brief  The main routine for Radio Services
** @param  argc, argv -- [input]Command line arguments
** @return 0 if success, -1 if failure
**
** Details: This function calls rsk_init_module() to start up. Then it waits in
**          a loop for radioServices to be terminated. Then it calls
**          rsk_exit_module() to shut down.
**/
int MAIN(int argc, char **argv)
{
    int result;

    /* Parse the command line arguments */
    rskParseArgs(argc, argv);

    /* Do once-at-startup initialization */
    result = rsk_init_module();

    if (result < 0) {
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"rsk_init_module() failed\n");
        return -1;
    }

    /* Intercept the termination signals */
    wsuUtilSetupSignalHandler(rskExitHandler, SIGUSR1, 0);
    wsuUtilSetupSignalHandler(rskExitHandler, SIGINT, 0);
    wsuUtilSetupSignalHandler(rskExitHandler, SIGTERM, 0);
    wsuUtilSetupSignalHandler(rskExitHandler, SIGKILL, 0);

#if defined(ENABLE_HSM_DAEMON)
    /* create 1 second and greater thread */
    if (0 == pthread_create(&OneSecThrdID, NULL, densoDaemon, NULL)) {
        pthread_detach(OneSecThrdID);
    } else {
        I2V_DBG_LOG(LEVEL_DBG,MY_NAME,"Failed to start denosDaemon.\n");
    }
#endif

    /* Keep running forever */
    while (mainLoop) {
        sleep(1);
#if defined(MY_UNIT_TEST)
        break;
#endif
    }

    /* Do once-at-termination de-initialization */
    rsk_exit_module();
    return 0;
}

