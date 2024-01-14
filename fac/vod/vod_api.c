/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: vod_api.c                                                        */
/*  Purpose: Verification On Demand API                                       */
/*                                                                            */
/* Copyright (C) 2016 DENSO International America, Inc.                       */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2016-03-02  KFRANKEL      First Implementation                             */
/*----------------------------------------------------------------------------*/


#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <string.h> 
#include <ctype.h>              //for isalnum
#include <netinet/in.h>
#include "dn_types.h"
#include "v2x_common.h"         // For V2X_LOG
#if !defined(HEAVY_DUTY)
#include "v2x_logging.h"        /*needed for extern log_level*/
#endif
#include "vod_api.h"


shm_vod_t * shm_vod_ptr;
uint8_t my_vod_clientid;

int8_t init_with_vod(char * app_abbrev)
{
    int ret;

    // Connect to the VOD shm
    shm_vod_ptr = (shm_vod_t *) wsu_share_init(sizeof(shm_vod_t), VOD_SHM_PATH);
    if (shm_vod_ptr == NULL) {
        V2X_LOG(LOG_ERR, "VOD_API: Unable to connect to vod shm!\n");
        ret = FALSE;
    } else {
        // For debugging, register ourselves as a client
        wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);
        if (shm_vod_ptr->num_registered_clients < MAX_VOD_CLIENTS) {
            my_vod_clientid = shm_vod_ptr->num_registered_clients;
            strcpy((char *)shm_vod_ptr->debug_clientAppAbbrev[my_vod_clientid], app_abbrev);
            ++shm_vod_ptr->num_registered_clients;
            ret = TRUE;
        } else {
            V2X_LOG(LOG_WARNING, "VOD_API: We have exceeded the max # of VOD clients. This is not a big, fatal error, but it is weird and should never happen!\n");
            ret = TRUE;
        }
        wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);
    }

    return ret;
}

void denit_with_vod()
{
    // For debugging, unregister ourselves.  But only if we are the last client.
    wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);
    if (shm_vod_ptr->num_registered_clients == my_vod_clientid + 1) {
        shm_vod_ptr->debug_clientAppAbbrev[my_vod_clientid][0] = 0; // Wipe out app abbrev
        shm_vod_ptr->num_registered_clients -= 1;   // Decrement client count
    }
    wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);

    // Release our shm ptr
    wsu_share_kill(shm_vod_ptr, sizeof(shm_vod_t));
}

int8_t request_msg_verification(uint32_t vod_msg_seq_num, uint8_t vod_priority)
{
    int i, req_index;

    // Lock
    wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);

    // Find an empty slot in the vod queue structures
    for (i=0, req_index=-1; i<MAX_VOD_REQUESTS && req_index < 0; i++) {
        if (shm_vod_ptr->request_status[i] == NOREQUEST) {
            req_index = i;
        }
    }

    // Fill the free slot
    if (req_index >= 0) {
        shm_vod_ptr->request_vod_msg_seq_num[req_index] = vod_msg_seq_num;
        shm_vod_ptr->request_priority[req_index] = vod_priority;
        shm_vod_ptr->request_status[req_index] = REQUESTED;
        shm_vod_ptr->request_clientid[req_index] = my_vod_clientid;
        ++shm_vod_ptr->cnt_client_requests;
    }

    // Notify VOD
    if (req_index >= 0) {
        wsu_open_gate(&shm_vod_ptr->request_added_gate);
    }

    // Unlock
    wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);

    // Request handle is the array index.
    return req_index;
}

bool_t wait_for_vod_result(uint8_t vod_request_handle)
{
    // Verify handle
    if (vod_request_handle < 0 || vod_request_handle >= MAX_VOD_REQUESTS) { 
        V2X_LOG(LOG_ERR, "VOD_API: ERROR: Invalid vod_request handle %d\n", vod_request_handle);
        return FALSE;
    }

    // Verify a request is there
    if (shm_vod_ptr->request_status[vod_request_handle] == NOREQUEST) {
        V2X_LOG(LOG_ERR, "VOD_API: ERROR: vod request # %d does not exist!\n", vod_request_handle);
        return FALSE;
    }

    // Don't wait for VOD msg seq num = 0 or 1; packet has been previously verified
    // by VTP.
    if (shm_vod_ptr->request_vod_msg_seq_num[vod_request_handle] >= 2) {
        // Life is waiting for the right moment to act - Paulo Coelho
        wsu_wait_at_gate(&shm_vod_ptr->request_completed_gate[vod_request_handle]);
    }

    return TRUE;
}


vod_vern_status_t retrieve_vod_result(uint8_t vod_request_handle)
{
    int8_t result;

    // Verify handle
    if (vod_request_handle < 0 || vod_request_handle >= MAX_VOD_REQUESTS) { 
        V2X_LOG(LOG_ERR, "VOD_API: ERROR: Invalid vod_request handle %d\n", vod_request_handle);
        return -1;
    }

    wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);

    // A VOD msg seq num value of 0 indicates the packet was successfully
    // verified by VTP
    if (shm_vod_ptr->request_vod_msg_seq_num[vod_request_handle] == 0) {
        result = SUCCESS;
    }
    // A VOD msg seq num value of 1 indicates the packet failed verification by
    // VTP
    else if (shm_vod_ptr->request_vod_msg_seq_num[vod_request_handle] == 1) {
        result = FAIL;
    }
    else {
        result = shm_vod_ptr->request_status[vod_request_handle];
    }

    shm_vod_ptr->request_status[vod_request_handle] = NOREQUEST;    // Mark slot available for reuse
    wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);

    return result;
}

void cancel_vod_request(uint8_t vod_request_handle)
{
    // Verify handle
    if (vod_request_handle < 0 || vod_request_handle >= MAX_VOD_REQUESTS) { 
        V2X_LOG(LOG_ERR, "VOD_API: ERROR: Invalid vod_request handle %d\n", vod_request_handle);
        return;
    }

    wsu_shmlock_lockw(&shm_vod_ptr->h.ch_lock);
    shm_vod_ptr->request_status[vod_request_handle] = NOREQUEST;    // Mark slot available for reuse
    wsu_open_gate(&shm_vod_ptr->request_completed_gate[vod_request_handle]); // Release anyone blocked
    wsu_shmlock_unlockw(&shm_vod_ptr->h.ch_lock);
}
