/**************************************************************************
 *                                                                        *
 *     File Name:  vod_api.h                                              *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************
 *      CAMP - Vehicle Safety Communications 3 Consortium Proprietary     *
 **************************************************************************/

/** @file
 *  @brief VOD API
 *
 *  @addtogroup VOD
 *  @{
 */

#ifndef _VOD_API_H_
#define _VOD_API_H_

#include "wsu_shm_inc.h"
#include "v2x_common.h"  // For APP_ABBREV_SIZE

/// @cond

#define VOD_SHM_PATH "/shm_vod"

#define  MAX_VOD_REQUESTS 19
#define  MAX_VOD_CLIENTS 10

/// @endcond

/** @brief Status values for VOD Requests
 *
 *  @details This enum contains the various stages and result values for a VOD request.
 */
typedef enum
{
    NOREQUEST = 0, 
    REQUESTED,
    INPROCESS,
    SUCCESS,
    FAIL,
    ERROR
} vod_vern_status_t;

/** @brief Priority values for VOD Requests
 *
 *  @details This enum contains the priority levels to assign to a vod request
 */
typedef enum
{
    PRIORITY_LOW = 1, 
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
    PRIORITY_EMERGENCY
} vod_req_priority_t;

/** @brief VOD Request structures
 *
 *  @details This structure maintains communcation between the VOD server
 *      application and the VOD client apps.  The vod_api library
 *      contains functions that interface with and abstract these elements.
 *
 * @cond
 */
typedef struct
{
    wsu_shm_hdr_t          h;  // replaceable with simple mutex since everybody wants write access
    wsu_gate_t             request_added_gate;                         //!< VOD application waits for this to be opened, then looks for new requests
    uint32_t               request_vod_msg_seq_num[MAX_VOD_REQUESTS];  //!< Msg seq num of WSM message that the vod client wishes to have verified
    uint8_t                request_priority[MAX_VOD_REQUESTS];         //!< Set by the vod client.  Requests with higher priority are sent first to VOD engine
    vod_vern_status_t request_status[MAX_VOD_REQUESTS];           //!< Current stage of a request.  See vod_vern_status_t for enums.
    wsu_gate_t             request_completed_gate[MAX_VOD_REQUESTS];   //!< Clients wait for these to be opened, then they can collect vod results

// FIELDS USEFUL FOR DEBUGGING
    uint8_t                num_registered_clients;                     //!< For debugging - Counts vod clients, used to assign a unique clientid to the vod clients.
    uint32_t               cnt_client_requests;                        //!< For debugging - Counts requests slotted by clients
    uint32_t               cnt_stack_requests;                         //!< For debugging - Counts requests sent to NS by app
    uint8_t                debug_clientAppAbbrev[MAX_VOD_CLIENTS][APP_ABBREV_SIZE];   //!< For debugging - Allows debug messages to mention a vod client by its app_abbrev
    uint8_t                request_clientid[MAX_VOD_REQUESTS];         //!< For debugging - tracks which client created each request
} shm_vod_t;

/// @endcond


/**
 * @brief  Initialization
 * @details Initiates communication with the VOD application
 * @param app_abbrev Pointer to containing the App Abbreviation
 * @return Returns the vod client id for this application, or a negative value on error.
 */
int8_t init_with_vod(char * app_abbrev);

/**
 * @brief Denit
 * @details Ends communication with the VOD application
 */
void denit_with_vod();

/**
 * @brief VOD Request
 * @details Queues a message verification
 * @param vod_msg_seq_num The unique vod_msg_seq_num identifier of the message
 * @param vod_priority The prioritization value of this request (higher means more important)
 * @return Returns the vod request handle for this request.  Pass this to wait_for_vod_result
 *         and retrieve_vod_result.  Returns negative value on error.
 */
int8_t request_msg_verification(uint32_t vod_msg_seq_num, uint8_t vod_priority);

/**
 * @brief Wait For Verification Completion
 * @details Waits on a msg verification's completion gate.  Will not return until message has been processed.
 * @param vod_request_handle The unique vod_request identifier of the verification request.
 * @return Returns true if the requested vod request was valid and wait was available.  Return false on a request
 *         that no longer exists.
 */
bool_t wait_for_vod_result(uint8_t vod_request_handle);


/**
 * @brief Get Verification Results
 * @details Retrieves the status for msg's verification.  Clears the msg verification slot so it can be reused. Do not call
 *        this function twice for the same vod_request_handle.  Make sure you call this function, or cancel_vod_request, for
 *        every request made.
 * @param vod_request_handle The unique vod_request identifier of the verification request.
 * @return Returns the verification result of the vod_request.
 */
vod_vern_status_t retrieve_vod_result(uint8_t vod_request_handle);

/**
 * @brief Cancels Verification
 * @details Cancels the request for a msg's verification.  Allows the request slot to be reused without waiting 
 *        for verification and retrieving the result.  This function should be used when a client has multiple
 *        messages to verify that have a dependency relationship -- for example if message A is invalid, then
 *        there's no need to verify messages B & C, and the vod client is coded to submit messages A-C with A
 *        at higher priority, and then wait for A's result.  If A is invalid, the client wants to abandon B & C,
 *        but the requests for B & C are still pending and taking up vod api space.  If the client simply never
 *        waits on B & C's gates and never calls retrieve_vod_result() for B & C, the B & C requests will stay
 *        forever in the api array taking up space.  Instead, the client should call this function, which
 *        will mark their request slots as available.  On the other hand, the client could be coded to request
 *        verification on A alone, and if A is valid, then separately request verification on B & C.  If that
 *        is always the case, this function is unnecessary.
 * @param vod_request_handle The unique vod_request identifier of the verification request.
 */
void cancel_vod_request(uint8_t vod_request_handle);

#endif

/**@}*/
