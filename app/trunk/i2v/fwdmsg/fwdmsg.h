/**************************************************************************
 *                                                                        *
 *     File Name:  fwdmsg.h  (Forward Messages)                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         2251 Rutherford Rd Ste 100                                     *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    Header file related to forward messages as a standalone 
    process.
 **************************************************************************/ 

#define FWDMSG_CONF_FILE        "fwdmsg.conf"

/* Manage forwarding socket. Init and recover from failures. */
#define FWD_SOCKET_LOCK_WAIT_MAX    1000  /* Wait for lock in usecs. */
#define FWD_SOCKET_LOCK_ATTEMPT_MAX 10    /* Attempts to lock before giving up. */

/* Basic return values. */
/* Only 32 errors max otherwise need bigger data type. */
#define FWDMSG_AOK                     (0)
#define FWDMSG_SHM_FAIL                (-1)  /* FATAL: nothing can save us. */
#define FWDMSG_LOAD_CONF_FAIL          (-2)
#define FWDMSG_FWD_SOCKET_ERROR        (-3)
#define FWDMSG_FWD_IP_ERROR            (-4)
#define FWDMSG_FWD_CONNECT_ERROR       (-5)
#define FWDMSG_CREATE_FWD_THREAD_FAIL  (-6)
#define FWDMSG_SIG_FAULT               (-7)
#define FWDMSG_OPEN_MQ_FAIL            (-8)
#define FWDMSG_OPEN_CONF_FAIL          (-9)
#define FWDMSG_HEAL_CONF_ITEM          (-10)
#define FWDMSG_FWD_SOCKET_BUSY         (-11)
#define FWDMSG_FWD_SOCKET_NOT_READY    (-12)
#define FWDMSG_FWD_WRITE_ERROR         (-13)
#define FWDMSG_FWD_WRITE_NOT_READY     (-14)
#define FWDMSG_FWD_WRITE_POLL_FAIL     (-15)
#define FWDMSG_FWD_WRITE_POLL_TIMEOUT  (-16)
#define FWDMSG_BAD_INPUT               (-17)
#define FWDMSG_CALLOC_ERROR            (-18)
#define FWDMSG_CUSTOMER_DIGEST_FAIL    (-19)
