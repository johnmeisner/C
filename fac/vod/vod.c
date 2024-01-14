/**************************************************************************
 *                                                                        *
 *     File Name:  vod.c                                                  *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <string.h>         // For memset, memcpy
#include "ipcsock.h"
#include "wsu_sharedmem.h"  // for gates & shmlocks
#include "v2x_common.h"     // for V2X_LOG
#include "cfg_vod.h"        // For VOD's cfg struct
#include "rs.h"             // For rsVodMsgType and rsVodReplyType
#include "vod.h"
#include "vod_api.h"
#if !defined(HEAVY_DUTY)
#include "v2x_logging.h"
#include "cmm_api.h"
#include "cfp_api.h"
#include "prm_api.h"
#endif
#if defined(ENABLE_SR)
#include "shm_sr.h"
#endif
#include "type2str.h"
#include "wsu_util.h"

/* Main loop flag */
static bool_t mainLoop = TRUE;

/* Socket for communicating with radioServices */
static int radioServicesSock = -1;

/* Threads */                                        
pthread_t watchdog_thread;
bool_t    watchdog_thread_running = FALSE;

/* Shared memory */                                        
shm_vod_t * shm_vod_ptr = NULL;

// My config, to be used someday
cfg_vod_t cfg_vod;

#if defined(ENABLE_SR)
// SR Playback status
bool_t sr_in_playback_mode;
#endif
#if !defined(HEAVY_DUTY)
/* Structure to parse our config file */
cfp_local_t vod_config_parsing_setup[] =
{
    /* DSDHGPS config */
    {cfp_get_uint8,         "log_level",                (void *)&log_level                  },
    {cfp_get_uint16,        "log_facility",             (void *)&log_facility               },
    {cfp_get_uint8,         "log_options",              (void *)&log_options                },

    {cfp_get_bool,          "AppEnableFlag",            (void *)&cfg_vod.vodAppEnable},
    {cfp_get_bool,          "CsvLogFlag",               (void *)&cfg_vod.vodCsvLogEnable},
    {cfp_get_uint32,        "vodVerifyTimeoutMs",       (void *)&cfg_vod.vodVerifyTimeoutMs},
    {NULL},
};

/*
 * @brief   CMM reg response user handler portion.
 *          Just used to stage PRM reg after config received.
 */
bool_t cmm_reg_response_handler(cmm_msg_t *msg)
{
    V2X_LOG(LOG_DEBUG, "VOD: PRM Register");
    prm_register();
    return TRUE;
}
#endif

void *watchdog_thread_handler(void *arg)
{
    while (watchdog_thread_running) {
#if !defined(HEAVY_DUTY) /* What is RSU equivalent? */
        prm_wdt_kick();
#endif
        usleep(100000);
    }
    
    printf("VOD: %s exiting\n", __func__);
    pthread_exit(NULL);
    return NULL;
}

/**
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0;
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Blocks waiting  for areply via
**          reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    size_t      rcvSize;

    /* Open the socket to communicate with radioServices with if it isn't
     * already opened */
    if (radioServicesSock < 0) {
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("wsuConnectSocket() failed\n");
            return -1;
        }

    }

    /* Send the message to Radio Services */
    if (!wsuSendData(radioServicesSock, RS_RECV_PORT, msg, size)) {
        /* Send failed */
        printf("wsuSendData() failed\n");
        return -1;
    }

    /* Wait for a response */
    while (1) {
        /* Get the reply to the message */
        rcvData.available = FALSE;
        rcvData.fd        = radioServicesSock;
        rcvData.data      = reply;
        rcvData.size      = replySize;

        rcvSize = wsuReceiveData(/* Timeout ignored */0, &rcvData);

        if (rcvSize > 0) {
            /* We received data. In this case, there is no way
             * rcvData.available could have been set to FALSE. */
            break;
        }

        if (rcvSize < 0) {
            /* Receive failed */
            printf("wsuReceiveData() failed\n");
            return -1;
        }

        /* If rcvSize is 0, then continue in the loop and try again. A return
         * value of 0 is something that can occur normally with datagram
         * sockets; wsuConnectSocket() specifies SOCK_DGRAM on its call to
         * wsuSendData(). Also, a return value of 0 does NOT indicate the
         * socket was closed. wsuReceiveData() will also return 0 if errno is
         * set to EINTR upron return upon return from recvfrom(); in this case,
         * you should try again. */
    }

    return rcvSize;
}

/*------------------------------------------------------------------------------
** Function:  vodExitHandler
** @brief  Terminate signal handler
** @param  signal -- [input]The signal that resulted in this function call
** @return 0 if success, -1 if failure
**
** Details: This routine is called when the SIGUSR1, SIGINT, SIGTERM, or
**          SIGKILL signal is received. It sets mainLoop to FALSE so that the
**          main loop will exit, except in the case of SIGUSR1, which is used
**          only to terminate threads, not vod itself.
**----------------------------------------------------------------------------*/
void vodExitHandler(int signal)
{
    /* Ignore SIGUSR1; it is only used to terminate threads, not vod itself. */
    if (signal != SIGUSR1) {
        if (mainLoop) {
            printf("VOD: %s called\n", __func__);
        }

        mainLoop = FALSE;
        /* Allow main loop to exit */
        wsu_open_gate(&shm_vod_ptr->request_added_gate);
    }
}

int main()
{
    int    i;
    int    req_index;
    int    cur_highest_priority;
    int    retVal = 0;
    int    result;
#if defined(ENABLE_SR)
    shm_sr_t *shm_sr_ptr;
#endif
#if !defined(HEAVY_DUTY)
    // Open a connection to syslog
    init_logging(VOD);
#endif
    V2X_LOG(LOG_DEBUG, "VOD Process Start");

    // Create our SHM
    shm_vod_ptr = (shm_vod_t *)wsu_share_init(sizeof(shm_vod_t), VOD_SHM_PATH);

    if (shm_vod_ptr == NULL) {
        V2X_LOG(LOG_ERR, "VOD: shmem init failed\n");
        retVal = -1;
        goto vodExit;
    }

    // Initialize stuff in our shm
    wsu_shmlock_init(&shm_vod_ptr->h.ch_lock);
    wsu_init_gate(&shm_vod_ptr->request_added_gate);

    for (i = 0; i < MAX_VOD_REQUESTS; i++) {
        wsu_init_gate(&shm_vod_ptr->request_completed_gate[i]);
        shm_vod_ptr->request_status[i] = NOREQUEST;
    }

    // Connect to radioServices
    radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

    if (radioServicesSock < 0) {
        /* Connect failed */
        V2X_LOG(LOG_ERR, "VOD: wsuConnectSocket() failed\n");
        retVal = -1;
        goto vodExit;
    }
#if defined(ENABLE_SR)
    // Connect to SR's shared memory
    if ((shm_sr_ptr = wsu_share_init(sizeof(shm_sr_t), SHM_SR_PATH)) == NULL) {
        V2X_LOG(LOG_ERR, "VOD: ERROR Could not connect to SR's shared memory.\n");
        retVal = -1;
        goto vodExit;
    }

    // Determine if SR is in playback
    sr_in_playback_mode = SR_IN_PLAYBACK_MODE(shm_sr_ptr);
    // Disconnect from SR's shm
    wsu_share_kill(shm_sr_ptr, sizeof(shm_sr_t));
#endif
    // Init the debugging stuff in our SHM
    shm_vod_ptr->num_registered_clients = 0;
    shm_vod_ptr->cnt_client_requests    = 0; // Requests slotted by clients
    shm_vod_ptr->cnt_stack_requests     = 0; // Requests sent to NS by app
#if defined(HEAVY_DUTY) 
    /* Only one conf value. Put in conf_manager or vod.conf == "100" */
    cfg_vod.vodVerifyTimeoutMs = 100;
#else
    // Initialize CMM API
    cmm_install_cb(CMM_RESP_REG, cmm_reg_response_handler);
    cmm_api_init(VOD);

    // Initialize PRM API
    prm_api_init(VOD);

    // Register for config files
    cmm_register(VOD_CONFIG_FILE, vod_config_parsing_setup);
    // Wait on PRM activation
    prm_wait();
#endif
    // Have to kick the watchdog...
    watchdog_thread_running = TRUE;
    result = pthread_create(&watchdog_thread, NULL,
                            watchdog_thread_handler, NULL);

    if (result != 0) {
        watchdog_thread_running = FALSE;
        V2X_LOG(LOG_ERR, "VOD: cv2xRxThread creation failed: %s\n",
                strerror(result));
    }

    // Intercept the termination signals
    wsuUtilSetupSignalHandler(vodExitHandler, SIGUSR1, 0);
    wsuUtilSetupSignalHandler(vodExitHandler, SIGINT, 0);
    wsuUtilSetupSignalHandler(vodExitHandler, SIGTERM, 0);
    wsuUtilSetupSignalHandler(vodExitHandler, SIGKILL, 0);

    while (mainLoop) {
        // Find the highest priority request that we have queued
        wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);
        cur_highest_priority = -1;
        req_index            = -1;

        for (i = 0; i < MAX_VOD_REQUESTS; i++) {
            if (shm_vod_ptr->request_status[i] == REQUESTED) {
                if (shm_vod_ptr->request_priority[i] > cur_highest_priority) {
                    cur_highest_priority = shm_vod_ptr->request_priority[i];
                    req_index            = i;
                }
            }
        }

        if (req_index < 0) {
            // If we don't have one, let's wait at our gate for someone to add
            // one
            wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);
            wsu_wait_at_gate(&shm_vod_ptr->request_added_gate);
        }
        else {
            // We do have one, so lets submit it!
            uint8_t result = ERROR;

            shm_vod_ptr->request_status[req_index] = REQUESTED;
            ++shm_vod_ptr->cnt_stack_requests;
            wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);
#if defined(ENABLE_SR)
            if (sr_in_playback_mode) {
                // If SR is in playback mode, none of the messages are in the
                // radio, and we can't verify anything. Rather than force the
                // user to turn off VOD in the configs, we'll mark every verify
                // request as successful.
                result = SUCCESS;
            }
            else
#endif
            {
                // SR in Record or None mode, we can verify messages
                rsVodMsgType   vodMsg;
                rsVodReplyType vodReply;
                int32_t        size;

                vodMsg.cmd          = WSU_NS_SECURITY_VERIFY_WSM_REQ;
                vodMsg.vodMsgSeqNum = shm_vod_ptr->request_vod_msg_seq_num[req_index];
                /* Send the message to radioServices and wait for a reply */
                size = communicateWithRadioServices(&vodMsg, sizeof(vodMsg),
                                                    &vodReply, sizeof(vodReply));

                if (size < 0) {
                    V2X_LOG(LOG_ERR, "VOD: communicateWithRadioServices() failed\n");
                    result = ERROR;
                }
                else if (vodReply.risRet != RS_SUCCESS) {
                    V2X_LOG(LOG_ERR, "VOD: Error vodReply.risRet = %s\n",
                            rsResultCodeType2Str(vodReply.risRet));
                    result = ERROR;
                }
                else  {
                    // Evaluate the result
                    result = (vodReply.vodResult == WS_ERR_NOT_FOUND) ? ERROR   :
                             (vodReply.vodResult == WS_SUCCESS)       ? SUCCESS :
                                                                        FAIL;
                }
            }

            // Save the result
            wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);
            shm_vod_ptr->request_status[req_index] = result;
            wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);

            // notify the client
            wsu_open_gate(&shm_vod_ptr->request_completed_gate[req_index]);
        }
    }

vodExit:
    // Release resources
    if (watchdog_thread_running) {
        watchdog_thread_running = FALSE;
        result = pthread_join(watchdog_thread, NULL);

        if (result != 0) {
            V2X_LOG(LOG_ERR, "VOD: pthread_join() failed: %s\n",
                    strerror(result));
        }
    }

    if (radioServicesSock >= 0) {
        close(radioServicesSock);
    }

    if (shm_vod_ptr != NULL) {
        for (i = 0; i < MAX_VOD_REQUESTS; i++) {
            wsu_kill_gate(&shm_vod_ptr->request_completed_gate[i]);
        }

        wsu_kill_gate(&shm_vod_ptr->request_added_gate);
        wsu_shmlock_kill(&shm_vod_ptr->h.ch_lock);
        wsu_share_kill(shm_vod_ptr, sizeof(shm_vod_t));
        wsu_share_delete(VOD_SHM_PATH);
    }
#if !defined(HEAVY_DUTY)
    cmm_terminate();
    prm_terminate();
#endif
    return retVal;
}
