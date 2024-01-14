#ifndef PEERTOPEERC_H
#define PEERTOPEERC_H

/**
 * @file PeerToPeer.h
 * @brief C interface to the Peer Cert Distribution.
 */

#include <stdint.h>
#include "aerolink_api.h"
#include "ws_errno.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for Aerolink configuration.
 */

typedef uint8_t (*P2PCallback)(void     *userData,  // OUT
                               uint32_t  pduLength, // OUT
                               uint8_t  *pdu);      // OUT


/**
 * @brief Set the callback function for P2P CA cert replies.
 *
 * @param userData (IN) Pointer to user data returned in a callback
 * @param p2pCallback (IN) Pointer to the callback method
 *    Set pointer to NULL to disable callbacks
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
p2p_setCallback(
        void         *userData,
        P2PCallback   p2pCallback);

/**
 * @brief Process an incomming P2P Cert reply message
 *
 * @param pduLength (IN) Length of data unit
 * @param pdu (IN) Pointer to the data unit
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
p2p_process(
        uint32_t    pduLength,
        uint8_t    *pdu);

#ifdef __cplusplus
}
#endif

#endif /* PEER2PEERC_H */
