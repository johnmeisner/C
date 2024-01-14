/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: wsu_sharedmem.c                                              */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Library of Shared Memory Functions                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/sem.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <fcntl.h>  // for O_CREAT, O_EXCL, O_RDWR
#include <sys/mman.h>   // for MAP_SHARED
#include "dn_types.h"

#include "wsu_sharedmem.h"
#include "v2x_common.h"

#define MAX_ERROR_PRINTS 10

/* Enable this to include the safety checks */
#define WANT_SHMLOCK_SAFETY_CHECKS 1


/* ================================================================================================== */
/* SHARED MEMORY BLOCKS                                                                               */
/* ================================================================================================== */


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
void * wsu_share_init ( size_t size, char_t *spath )
{
    void *shrm_start=NULL;
    int32_t fd=0;

    if(NULL == spath) {
        V2X_LOG(LOG_ERR, "wsu_share_init: NULL input fail.\n");
        return NULL;
    }
    fd = shm_open(spath, O_RDWR|O_CREAT|O_EXCL, 0664);
    if (fd >= 0) {
		    if(0 != ftruncate(fd, size)) {
            V2X_LOG(LOG_ERR, "wsu_share_init: ftruncate shm (%s) fail.\n", spath);
            return NULL;
        }
    } else {
    	  fd = shm_open(spath, O_RDWR, 0664);
    	  if (fd < 0) {
    		    V2X_LOG(LOG_ERR, "wsu_share_init: create shm (%s) fail.\n", spath);
    		    return NULL;
    	  }
    }
    shrm_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    // If you pass MAP_SHARED|MAP_ANON to mmap, it's an illegal combination, and it seems to
    //    return -1 as the error response.  So we test for -1 in addition to a null pointer,
    //    and return NULL so our caller doesn't have to test for two things.
    if ((int64_t) shrm_start == -1 || (int64_t) shrm_start == 0) {
    	V2X_LOG(LOG_ERR, "wsu_share_init: mmap for shm %s fd %d size %ld failed, returned %p.\n", spath, fd, size, shrm_start);
    	return NULL;
    }

    return shrm_start;
}


/**
 * Detaches a shared memory segment
 *
 * This function detaches the shared memory segment and
 * unmaps it from the process's memory. If no other processes are
 * using this segment, the user also needs to call wsu_share_delete()
 * to delete the shared memory.
 *
 * @param shrm_start - Address of the shared memory segment
 * @param size - Size of the shared memory segment
 */
void wsu_share_kill( void *shrm_start, size_t shm_size)
{
	if (munmap(shrm_start, shm_size) < 0)
	{
		V2X_LOG(LOG_WARNING, "wsu_share_kill: munmap failed, errno=%d\n", errno);
	}
}

/**
 * Removes the shared memory segment from memory
 *
 * This function removes the shared memory segment from memory.
 * Note that all processes using this segment must be detached first
 * with wsu_shm_kill().
 *
 * @param spath - The namespace path of the shared memory segment
 */
void wsu_share_delete( char_t * shm_path_str )
{
	if (shm_unlink(shm_path_str) < 0)
	{
		V2X_LOG(LOG_WARNING, "wsu_share_delete: shm_unlink %s failed, errno=%d\n", shm_path_str, errno);
	}
}







/* ================================================================================================== */
/* SHMLOCK - SHM LOCKS FOR SHARED MEMORY                                                              */
/* ================================================================================================== */


/**
 * Initializes a shared memory lock structure
 *
 * This function initializes a shared memory lock structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this lock will control.
 *
 * @param lock - Pointer to a wsu_shmlockv1_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_shmlock_init( wsu_shmlock_t * lock )
{
     // Is this shmlock already init'ed?
     if ( lock->marker == OBJ_VALID_MARKER ) {

        /* If init'ing a valid shmlock, we need to
            (1) just do no initing, just it as it
            (2) init it anyway, hope init'ing it twice doesn't lock us up
            (3) destroy it first
         */

        /*
        // Do nothing
        V2X_LOG(LOG_ERR, "wsu_shmlock_init: Asked to initialize wsu_shmlock_t 0x%lx but its marker is already set!\n", (unsigned long) lock);
        return FALSE;
        */

        // (3) Destroy it first
        wsu_shmlock_kill(lock);
    }

    // INITIALIZATION

#ifdef WANT_SHMLOCK_SAFETY_CHECKS
    // Tells QNX that this needs to be an inter-process, sharedmem mutex
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared( &mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&lock->mutex_for_cnts, &mutex_attr);
#endif

    lock->rcnt = 0;
    lock->wcnt = 0;
    lock->marker = OBJ_VALID_MARKER;

    // Setup rwlock attributes
    pthread_rwlockattr_t rwlock_attr;
    pthread_rwlockattr_init(&rwlock_attr);
    // Tell OS that this needs to be an inter-process, sharedmem rwlock
    if (pthread_rwlockattr_setpshared(&rwlock_attr, PTHREAD_PROCESS_SHARED) != 0) {
        V2X_LOG(LOG_ERR, "wsu_shmlock_init: rwlockattr_setpshared failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
        return FALSE;
    }

    // Tell OS that writers need to be prioritized over readers (not the default)
    if (pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP) != 0) {
        // PTHREAD_RWLOCK_PREFER_WRITER_NP
        V2X_LOG(LOG_ERR, "wsu_shmlock_init: rwlockattr_setkind failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
        return FALSE;
    }

    // Init our shmlock2's rwlock var
    if (pthread_rwlock_init(&lock->rwlock, &rwlock_attr) != EOK) {
        V2X_LOG(LOG_ERR, "wsu_shmlock_init: rwlockvar init failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
        return FALSE;
    }

    return TRUE;
}

/**
 * Destroys a shared memory lock
 *
 * This function destroys the components that represent a shared
 * memory lock, preparing it for deallocation.
 * Note that it does not de-allocate the structure's memory itself.
 *
 * @param lock - Pointer to a wsu_shmlockv1_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_shmlock_kill( wsu_shmlock_t *lock )
{
	int ret = TRUE;
#ifdef WANT_SHMLOCK_SAFETY_CHECKS
	if (pthread_mutex_destroy(&lock->mutex_for_cnts) != EOK)
		ret = FALSE;
#endif
	if (pthread_rwlock_destroy(&lock->rwlock) != EOK)
		ret = FALSE;
    lock->marker = OBJ_DESTROYED_MARKER;
	return ret;
}

/**
 * Read-lock a shared memory segment
 *
 * This function is used to do a read lock.
 * Many readers can be present but will block on a single writer.
 * When this function returns, the shmlock has been read-locked
 * by the caller.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_shmlock_lockr(wsu_shmlock_t * lock)
{
    if (pthread_rwlock_rdlock(&lock->rwlock) != EOK)
        return FALSE;

#ifdef WANT_SHMLOCK_SAFETY_CHECKS
    pthread_mutex_lock(&lock->mutex_for_cnts);
    lock->rcnt++;
    pthread_mutex_unlock(&lock->mutex_for_cnts);
#endif

    return TRUE;
}


/**
 * Write-lock a shared memory segment
 *
 * This function is used to do a write lock.  It will block if there are any readers or writers.
 *
 * Code give writes priority over reads. This was done by having writes
 * pre-increment the wcnt. All reads that were in progress continued while any new
 * reads will block waiting on the wcnt to go to zero. That way every request for
 * a write lock is guaranteed to proceed.

 DESIGN NOTES:
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

 @return Returns TRUE on success, or FALSE on failure
*/
bool_t wsu_shmlock_lockw(wsu_shmlock_t * lock)
{
    if (pthread_rwlock_wrlock(&lock->rwlock) != EOK)
        return FALSE;

#ifdef WANT_SHMLOCK_SAFETY_CHECKS
    pthread_mutex_lock(&lock->mutex_for_cnts);
    if (lock->wcnt > 0) {
        V2X_LOG(LOG_WARNING, "WSU_SHMLOCK2: in a lockw, wcnt = %u (want 0), pid = %u\n", (unsigned int) lock->wcnt, (unsigned int) getpid());
#if defined(EXTRA_DEBUG)
        printf("\nWSU_SHMLOCK2: in a lockw, wcnt = %u (want 0), pid = %u\n", (unsigned int) lock->wcnt, (unsigned int) getpid());
#endif
    }
    if (lock->rcnt > 0) {
        V2X_LOG(LOG_WARNING, "WSU_SHMLOCK2: in a lockw, rcnt = %u (want 0), pid = %u\n", (unsigned int) lock->rcnt, (unsigned int) getpid());
#if defined(EXTRA_DEBUG)
        printf("\nWSU_SHMLOCK2: in a lockw, rcnt = %u (want 0), pid = %u\n", (unsigned int) lock->rcnt, (unsigned int) getpid());
#endif
    }
    lock->wcnt++;
    pthread_mutex_unlock(&lock->mutex_for_cnts);
#endif

    return TRUE;
}
bool_t wsu_shmlock_trylockw(wsu_shmlock_t * lock) {
    if (pthread_rwlock_trywrlock(&lock->rwlock) != EOK)
        return FALSE;

    return TRUE;
}
bool_t wsu_shmlock_trylockr(wsu_shmlock_t * lock) {
    if (pthread_rwlock_trywrlock(&lock->rwlock) != EOK)
        return FALSE;

    return TRUE;
}
/**
 * Write-unlock a shared memory segment
 *
 * This function is used to release a write-lock on a shared memory segment.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */

bool_t wsu_shmlock_unlockw(wsu_shmlock_t * lock)
{
#ifdef WANT_SHMLOCK_SAFETY_CHECKS
    pthread_mutex_lock(&lock->mutex_for_cnts);
    if (lock->wcnt !=  1) {
#if defined(EXTRA_DEBUG)
        printf("\nWSU_SHMLOCK2: in an unlockw, wcnt = %u (want 1), pid = %u\n", (unsigned int) lock->wcnt, (unsigned int) getpid());
#endif
        V2X_LOG(LOG_WARNING, "WSU_SHMLOCK2: in an unlockw, wcnt = %u (want 1), pid = %u\n", (unsigned int) lock->wcnt, (unsigned int) getpid());
    }
    lock->wcnt--;
    pthread_mutex_unlock(&lock->mutex_for_cnts);
#endif
    if (pthread_rwlock_unlock(&lock->rwlock) != EOK)
        return FALSE;
    else
        return TRUE;
}

//TODO: There is no diff between w or r lock?
/**
 * Read-unlock a shared memory segment
 *
 * This function is used to release a read-lock on a shared memory segment.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_shmlock_unlockr(wsu_shmlock_t * lock)
{
#ifdef WANT_SHMLOCK_SAFETY_CHECKS
    pthread_mutex_lock(&lock->mutex_for_cnts);
    lock->rcnt--;
    pthread_mutex_unlock(&lock->mutex_for_cnts);
#endif
    if (pthread_rwlock_unlock(&lock->rwlock) != EOK)
        return FALSE;
    else
        return TRUE;
}


/* ================================================================================================== */
/* GATES                                                                                              */
/* ================================================================================================== */


/**
 * Initializes a shared memory gate
 *
 * This function initializes a shared memory gate structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this gate resides in.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_init_gate(wsu_gate_t * gate)
{
    if (gate == NULL)
    {
        return FALSE;
    }

    if (gate->marker == OBJ_VALID_MARKER) {
        wsu_kill_gate(gate);
    }

	// Let's start with gate closed
	gate->gate_is_open = 0;

    // Mark this gate as needing to be destroyed first if we try re-initing again
    gate->marker = OBJ_VALID_MARKER;

	// Tells QNX that this needs to be an inter-process, sharedmem mutex
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_setpshared( &mutex_attr, PTHREAD_PROCESS_SHARED);

	// Init our gate's mutex.
	if (pthread_mutex_init(&gate->mutex, &mutex_attr) != EOK)
	{
		V2X_LOG(LOG_WARNING, "wsu_init_gate: Mutex init failed! error=%d=%s (2=already inited),pid=%d\n", errno, strerror( errno ), getpid() );
		return FALSE;
	}

	// Tell QNX that this needs to be an inter-process, sharedmem condvar
	pthread_condattr_t cond_attr;
	pthread_condattr_init(&cond_attr);
	pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);

	// Init our gate's condvar
	if (pthread_cond_init(&gate->condvar, &cond_attr) != EOK) {
		V2X_LOG(LOG_WARNING, "wsu_init_gate: Condvar init failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
		return FALSE;
	}

    return TRUE;
}


/**
 * Waits at shared memory gate
 *
 * This function waits a shared memory gate structure.
 * Note that it does not allocate the structure, that is done
 * elsewhere, typically by the process that allocates
 * the shared memory this gate resides in.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_wait_at_gate(wsu_gate_t * gate)
{
    if (gate == NULL)
    {
        return FALSE;
    }

    if (pthread_mutex_lock(&gate->mutex) != EOK)
    {
		V2X_LOG(LOG_ERR, "wsu_wait_at_gate: Mutex lock failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
		return FALSE;
    }

    while ( ! gate->gate_is_open ) {
    	if (pthread_cond_wait(&gate->condvar, &gate->mutex) != EOK)
    	{
    		V2X_LOG(LOG_ERR, "wsu_wait_at_gate: cond_wait failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
    		pthread_mutex_unlock(&gate->mutex);
    		return FALSE;
    	}
    }

    // Close the gate behind us on our way out
    // We could signal the locking processes as well, but we're not doing that now
    gate->gate_is_open = 0;

    if (pthread_mutex_unlock(&gate->mutex) != EOK)
    {
		V2X_LOG(LOG_ERR, "wsu_wait_at_gate: Mutex unlock failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
		return FALSE;
    }

	return TRUE;
}

/**
 * Opens a shared memory gate
 *
 * This function opens a shared memory gate structure.
 * If a process is blocked, waiting for this gate to be opened, it will be unblocked.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_open_gate(wsu_gate_t * gate)
{
    bool_t ret;
    if (gate == NULL)
    {
        return FALSE;
    }
    ret = TRUE;

	if (pthread_mutex_lock(&gate->mutex) != EOK)
	{
		V2X_LOG(LOG_ERR, "wsu_open_gate: Mutex lock failed! error=%d=%s,pid=%d,gate=%p\n", errno, strerror(errno), getpid(), gate);
		return FALSE;
	}

	// Design question: Do we want a condvar signal when opening an open gate?  Currently NO.
	if (! gate->gate_is_open) {
		gate->gate_is_open = 1;
		// Signal the guy on the other side of the gate
		if (pthread_cond_signal(&gate->condvar) != EOK) {
			V2X_LOG(LOG_ERR, "wsu_open_gate: Condvar Signal failed! error=%d=%s,pid=%d,gate=%p\n", errno, strerror(errno), getpid(), gate);
			ret = FALSE;
		}
	}
	if (pthread_mutex_unlock(&gate->mutex) != EOK)
	{
		V2X_LOG(LOG_ERR, "wsu_open_gate: Mutex unlock failed! error=%d=%s,pid=%d,gate=%p\n", errno, strerror(errno), getpid(), gate);
		ret = FALSE;
	}
    return ret;
}

/**
 * Destroys a shared memory gate
 *
 * This function destroys the components that represent a shared
 * memory gate, preparing it for deallocation.
 * Note that it does not de-allocate the structure's memory itself.
 *
 * @param lock - Pointer to a wsu_gate_t structure.
 *
 * @return Returns TRUE on success, or FALSE on failure
 */
bool_t wsu_kill_gate(wsu_gate_t * gate)
{
	int ret = TRUE;
	if (pthread_mutex_destroy(&gate->mutex) != EOK)
		ret = FALSE;
	if (pthread_cond_destroy(&gate->condvar) != EOK)
		ret = FALSE;
    gate->marker = OBJ_DESTROYED_MARKER;
	return ret;
}



/**
 * Gets Data from Shared Memory
 *
 * @param lock - Pointer to a wsu_shmlock_t structure.
 * @param shm_data- Pointer to data within Shared Memory.
 * @param local_data- Pointer to a local copy of data structure.
 * @param size - Size of data to be copied.
 *
 * @return TRUE/FALSE
 */
bool_t get_shm_data(wsu_shmlock_t* lock, void* shm_data, void* local_data, size_t size)
{
    if (shm_data == NULL || local_data == NULL)
    {
        V2X_LOG(LOG_ERR, "Null Pointer");
        return FALSE;
    }

    if (lock != NULL)
    {
        if (wsu_shmlock_lockr(lock) == FALSE)
        {
            V2X_LOG(LOG_ERR, "SHM Lock Failed");
            return FALSE;
        }
    }

    memcpy(local_data, shm_data, size);

    if (lock != NULL)
    {
        if (wsu_shmlock_unlockr(lock) == FALSE)
        {
            V2X_LOG(LOG_ERR, "SHM Unlock Failed");
            return FALSE;
        }
    }

    return TRUE;
}
