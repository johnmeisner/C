/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Filename: wsu_sharedmem.h                                                 */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifndef WSU_SHAREDMEM_H
#define WSU_SHAREDMEM_H

#include <sys/time.h>
#include <semaphore.h>
#include "dn_types.h"
#include <pthread.h>


/**
* @brief Internal values to mark sharedmem structures as valid, or destroyed
*/
#define OBJ_VALID_MARKER     0x31323334                /*!< Marker for valid */
#define OBJ_DESTROYED_MARKER 0x11DEAD11                /*!< Marker for destroyed.  Funner to find in memory dumps than all zeroes */


/**
* @brief Definition of flags structure for shared memory
*/
typedef struct wsu_shmlock_t {
	pthread_rwlock_t  rwlock;
	pthread_mutex_t mutex_for_cnts;
    uint8_t volatile rcnt;
    uint8_t volatile wcnt;
    uint32_t marker;
} wsu_shmlock_t;

/**
* @brief Definition of flags structure for shared memory
*/
typedef struct wsu_gate_t {
	pthread_mutex_t mutex;   		  	/*!< Mutex	*/
	pthread_cond_t  condvar;   		 	/*!< Cond variable*/
    bool_t gate_is_open;  		 	    /*!< is gate opened?  0=false 1=true */
    uint32_t marker;
} wsu_gate_t;

/**
 *@brief Function creates and attaches a shared memory segment
 *
 * This function creates and attaches a shared memory segment,
 * named by spath. If a shared segment by that name already exists,
 * then the user is simply attached to it.
 *
 * @param size - Desired size of the shared memory segment
 * @param spath - Name to identify this segment so other processes
 *                can attach to it using this name.  Segment name
 *                must start with a slash, and have no other slashes.
 *
 * @return Returns pointer to shared memory segment, or NULL on failure
 */
void *wsu_share_init( size_t size, char_t * spath );

/**
 *@brief Function detaches a shared memory segment
 *
 * This function detaches the shared memory segment and
 * unmaps it from the process's memory. If no other processes are
 * using this segment, the user also needs to call wsu_share_delete()
 * to delete the shared memory.
 *
 * @param shrm_start - Address of the shared memory segment
 * @param size - Size of the shared memory segment
 */
void wsu_share_kill( void * shrm_start, size_t size );

/**
 *@brief Function removes the shared memory segment from memory
 *
 * This function removes the shared memory segment from memory.
 * Note that all processes using this segment must be detached first
 * with wsu_shm_kill().
 *
 * @param spath - The namespace path of the shared memory segment
 */
void wsu_share_delete( char_t * shm_path_str );




/**
 *@brief Function waits at shared memory gate
 *
 * This function waits a shared memory gate structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this gate resides in.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_wait_at_gate(wsu_gate_t * wsu_sem);

/**
 *@brief Function opens a shared memory gate
 *
 * This function opens a shared memory gate structure.
 * If a process is blocked, waiting for this gate to be opened, it will be unblocked.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_open_gate(wsu_gate_t * wsu_sem);

/**
 *@brief Function initializes a shared memory gate
 *
 * This function initializes a shared memory gate structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this gate resides in.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_init_gate(wsu_gate_t * wsu_sem);

/**
 *@brief Function destroys a shared memory gate
 *
 * This function destroys the components that represent a shared
 * memory gate, preparing it for deallocation.
 * Note that it does not de-allocate the structure's memory itself.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_kill_gate(wsu_gate_t * gate);

/**
 *@brief Function initializes a shared memory lock structure
 *
 * This function initializes a shared memory lock structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this lock will control.
 *
 * @param lock - Pointer to a wsu_shmlock_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_shmlock_init(wsu_shmlock_t * lock);

/**
 *@brief Function destroys a shared memory lock
 *
 * This function destroys the components that represent a shared
 * memory lock, preparing it for deallocation.
 * Note that it does not de-allocate the structure's memory itself.
 *
 * @param lock - Pointer to a wsu_shmlock_t structure.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_shmlock_kill(wsu_shmlock_t * lock);

/**
 *@brief Write-lock a shared memory segment
 *
 * This function is used to do a write lock.  It will block if there are any readers or writers.
 *
 * Code give writes priority over reads. This was done by having writes
 * pre-increment the wcnt. All reads that were in progress continued while any new
 * reads will block waiting on the wcnt to go to zero. That way every request for
 * a write lock is guaranteed to proceed.

 @note DESIGN NOTES:
    Because there are a lot of programs read locking each area often, we found
    that it is possible to freeze out the writelock by the read count never returning to zero.

    That has been fixed with the flag
        lock->write_lock_request_pending = 1;
    When this flag is > 0, all new attempts to start a readlock will block.  Eventually, all currently
    held readlocks will finish and the pending writelock will gain the mutex.
    Following the writelock unlock, all waiting reads will be allowed to proceed.

    If second writelock is waiting, it will set lock->write_lock_request_pending flag before it blocks,
        for its prioritized turn at the mutex when the first writelock is released.

    When this function returns, the shared memory segment has been write-locked by the caller.

 @return Returns WTRUE on success, or WFALSE on failure
*/
bool_t wsu_shmlock_lockw(wsu_shmlock_t * lock);
bool_t wsu_shmlock_trylockw(wsu_shmlock_t * lock);
/**
 * @brief Read-lock a shared memory segment
 *
 * This function is used to do a read lock.
 * Many readers can be present but will block on a single writer.
 * When this function returns, the shmlock has been read-locked
 * by the caller.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_shmlock_lockr(wsu_shmlock_t * lock);
bool_t wsu_shmlock_trylockr(wsu_shmlock_t * lock);

/**
 * @brief Read-unlock a shared memory segment
 *
 * This function is used to release a read-lock on a shared memory segment.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_shmlock_unlockr(wsu_shmlock_t * lock);

/**
 * @brief Write-unlock a shared memory segment
 *
 * This function is used to release a write-lock on a shared memory segment.
 *
 * @return Returns WTRUE on success, or WFALSE on failure
 */
bool_t wsu_shmlock_unlockw(wsu_shmlock_t * lock);

/// @cond DEV
// SHM Locking Macros -- Perhaps these need to go away too, just use function names!  [20190305]
#define WSU_SHM_LOCKR(a) wsu_shmlock_lockr((a))
#define WSU_SHM_LOCKW(a) wsu_shmlock_lockw((a))
#define WSU_SHM_UNLOCKR(a) wsu_shmlock_unlockr((a))
#define WSU_SHM_UNLOCKW(a) wsu_shmlock_unlockw((a))


// Per: https://stackoverflow.com/questions/23686494/marking-functions-from-a-library-as-deprecated
#ifdef __GNUC__
#define DEPRECATED(X) X __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(X) __declspec(deprecated) X
#else
#define DEPRECATED(X) X
#endif

/**
 * Gets Data from Shared Memory
 *
 * @param lock - Pointer to a wsu_shmlock_t structure.
 * @param shm_data- Pointer to data within Shared Memory.
 * @param local_data- Pointer to a local copy of data structure.
 * @param size - Size of data to be copied.
 *
 * @return 0, -1
 */
bool_t get_shm_data(wsu_shmlock_t* lock, void* shm_data, void* local_data, size_t size);

#endif
