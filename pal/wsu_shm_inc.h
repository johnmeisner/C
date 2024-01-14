/**************************************************************************
 *                                                                        *
 *     File Name:  wsu_shm_inc.h                                          *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#ifndef WSU_SHM_INC_H
#define WSU_SHM_INC_H

#include "dn_types.h"
#include "wsu_sharedmem.h"

/**
* @brief Definition of some state ???
*
* Contains ...
*/
enum wsu_state_e {
    wsu_disabled,  		  			/*!< Disabled	*/
    wsu_enabled  		  			/*!< Enabled	*/
};


/**
* @brief Definition of Result Codes ???, returned by the APIs ???
*
* Contains Result codes.
*/
enum wsu_valid_e {
    wsu_invalid = 0,  		  			/*!< Invalid	*/
    wsu_valid  		  				/*!< Valid	*/
};

/**
* @brief Definition of flags structure for shared memory
*/
typedef struct wsu_shm_hdr_s {
    wsu_shmlock_t       ch_lock;            /*!< Using shmlock_t now instead of old wsu_sem_t [201504]*/
    uint64_t            ch_time;            /*!< time of last shrmem update 		*/
    enum wsu_state_e    ch_state;           /*!< indicates if the process is enabled 	*/
    uint32_t            ch_sequence_number; /*!< incremented each shrmem update 	*/
    uint32_t            ch_keepalive;       /*!< incremented each process execution     */
    bool_t              ch_data_valid;      /*!< data is valid for use			*/
} wsu_shm_hdr_t;

#endif
