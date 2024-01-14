/* Copyright 2017 OnBoard Security, Inc. 
   Version 0.6
*/

#ifndef AEROLINK_PKI_CONN_H_
#define AEROLINK_PKI_CONN_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief High-level error code returned by the connection APIs.
 */
#define AerolinkPKI_ERR_NONE                            0    /* No error */
#define AerolinkPKI_ERR_CONNECTION_CLOSED              -1    /* Connection was closed, e.g. the peer server has closed the connection, or network service becomes unavailable */
#define AerolinkPKI_ERR_HOST_NOT_RESOLVABLE            -2    /* Host address cannot be resolved */
#define AerolinkPKI_ERR_HOST_UNREACHABLE               -3    /* Unable to connect to host address */
#define AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED   -4    /* Host address family is unsupported, i.e. IPv6 */
#define AerolinkPKI_ERR_TLS_INIT_FAILURE               -5    /* TLS initialization failure */
#define AerolinkPKI_ERR_TLS_INTERNAL_FAILURE           -6    /* TLS internal failure, e.g. TLS library failure (protocol), SYSCALL failure (I/O), etc. */
#define AerolinkPKI_ERR_TLS_AUTHENTICATION_FAILURE     -7    /* TLS authentication failure detected during TLS handshake. SCMS-979, 980 */
#define AerolinkPKI_ERR_BAD_CONNECTION_HANDLE          -8    /* Bad connection handle */
#define AerolinkPKI_ERR_NO_MEMORY                      -9    /* Out of memory */
#define AerolinkPKI_ERR_TRY_AGAIN                      -10   /* Try the operation again */


/**
 * @brief Callback function prototype for asynchronous report of network availability.
 * If a callback is registered, whenever the network availability changes, the callback is invoked to report the new status.
 * @param bNetworkAvailable (IN) If the network service is available, true is used. Otherwise, false is used.
 * @param userData (IN) Custom data passed in from the AerolinkPKI_registerNetworkAvailCallback() function.
 */
typedef void(*NetworkAvailCallback)(bool bNetworkAvailable, void* userData);

/**
 * @brief Callback function prototype for asynchronous report of data received/error detected on a connection while receiving data.
 * @param result (IN) If data is received successfully, AerolinkPKI_ERR_NONE is used. Otherwise, an error code corresponding 
 * to the error condition is used, such as:
 *    - AerolinkPKI_ERR_CONNECTION_CLOSED 
 *    - AerolinkPKI_ERR_NO_MEMORY
 * @param data (IN) Pointer to a buffer that stores the data received from the connection.
 * @param dataLen (IN) Size of the data buffer in bytes.
 * @param userData (IN) Custom data passed in from the AerolinkPKI_connect() function. It is for the callback to process the result.
 */
typedef void(*ReceiveDataCallback)(int32_t result, uint8_t const * data, size_t dataLen, void * userData);

/**
 * @brief Register the callback function to receive network availability status.
 * When the callback is registered, the system will invoke the callback to report the current network availability.
 * Whenever the network availability changes, the system will invoke the registered callback to report the new status.
 * When the registered callback no longer needs to receive notification, it should be removed by calling the function
 * AerolinkPKI_unregisterNetworkAvailCallback(). 
 * @param networkCallback  (IN) callback function to report the network availability.
 * @param userData (IN) Custom data that will be used in the callback function.
 */
extern void AerolinkPKI_registerNetworkAvailCallback(NetworkAvailCallback networkCallback, void* userData);

/**
 * @brief Remove the callback registered by the AerolinkPKI_registerNetworkAvailCallback().
 */
extern void AerolinkPKI_unregisterNetworkAvailCallback(void);

/**
 * @brief This function establishes a connection to the server.
 * @param address (IN) C style string of IP address or domain name of a server.
 * @param port (IN) Port number on the server.
 * @param networkPreference  (IN) Preferred network number. 0 indicates to connect to any available network; 
 * Others indicate OEM-defined preference numbers corresponding to the network preferences in their configuration.
 * @param dataCallback  (IN) callback function to be called when the connection receives data from server.
 * @param userCallbackData  (IN) user supplied data that will be used in the callback function.
 *
 * @return int32_t
       - When the connection is established, a valid connection handle that is greater than or equal to 0 is returned,
 *       It uniquely identifies this connection. Otherwise, one of the following error codes is returned.
 *
 * @retval AerolinkPKI_ERR_HOST_NOT_RESOLVABLE
 *     - Host address cannot be resolved.
 * 
 * @retval AerolinkPKI_ERR_HOST_UNREACHABLE
 *     - Unable to connect to host address.
 *
 * @retval AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED
 *     - Host address family is unsupported.
 *
 * @retval AerolinkPKI_ERR_TLS_INIT_FAILURE
 *     - TLS initialization failure.
 *
 * @retval AerolinkPKI_ERR_TLS_INTERNAL_FAILURE
 *     - TLS internal failure, e.g. TLS library failure (protocol).

 * @retval AerolinkPKI_ERR_TLS_AUTHENTICATION_FAILURE
 *     - TLS authentication failure detected during TLS handshake.
 *
 * @retval AerolinkPKI_ERR_NO_MEMORY
 *     - Cannot allocate memory for the connection.
 */
extern int32_t AerolinkPKI_connect(
        char const*         address,
        uint16_t            port,
        uint8_t             networkPreference,
        ReceiveDataCallback dataCallback,
        void*               userCallbackData);

/**
 * @brief This function establishes a connection to the server without TLS.
 * @param address (IN) C style string of IP address or domain name of a server.
 * @param port (IN) Port number on the server.
 * @param networkPreference  (IN) Preferred network number. 0 indicates to connect to any available network;
 * Others indicate OEM-defined preference numbers corresponding to the network preferences in their configuration.
 * @param dataCallback  (IN) callback function to be called when the connection receives data from server.
 * @param userCallbackData  (IN) user supplied data that will be used in the callback function.
 *
 * @return int32_t
       - When the connection is established, a valid connection handle that is greater than or equal to 0 is returned,
 *       It uniquely identifies this connection. Otherwise, one of the following error codes is returned.
 *
 * @retval AerolinkPKI_ERR_HOST_NOT_RESOLVABLE
 *     - Host address cannot be resolved.
 *
 * @retval AerolinkPKI_ERR_HOST_UNREACHABLE
 *     - Unable to connect to host address.
 *
 * @retval AerolinkPKI_ERR_HOST_ADDR_FAMILY_UNSUPPORTED
 *     - Host address family is unsupported.
 *
 * @retval AerolinkPKI_ERR_NO_MEMORY
 *     - Cannot allocate memory for the connection.
 */
extern int32_t AerolinkPKI_unsecure_connect(
        char const*         address,
        uint16_t            port,
        uint8_t             networkPreference,
        ReceiveDataCallback dataCallback,
        void*               userCallbackData);

/**
 * @brief Send data to the server. 
 * @param connectionHandle  (IN) Connection handle that identifies the connection.
 * @param data (IN) Pointer to a buffer to be sent to the server. It will not be valid after the function returns.
 * @param dataLen  (IN) Number of bytes in the buffer to be sent.
 *
 * @return int32_t
 *     - Number of bytes sent to the server; Otherwise, one of the following error codes is returned when the operation fails.
 *
 * @retval AerolinkPKI_ERR_CONNECTION_CLOSED
 *     - Disconnected by the server.
 *
 * @retval AerolinkPKI_ERR_BAD_CONNECTION_HANDLE
 *     - The handle doesn't identify a connection instance.
 *
 * @retval AerolinkPKI_ERR_TRY_AGAIN
 *     - System might be busy, try to send data again later.
 *
 * @retval AerolinkPKI_ERR_NO_MEMORY
 *     - Cannot allocate memory for the operation.
 */
extern int32_t AerolinkPKI_send(int32_t connectionHandle, uint8_t const * data, int32_t dataLen);

/**
 * @brief Close the connection identified by the given handle and release resources associated with this connection.
 * @param connectionHandle  (IN) Connection handle that identifies the connection instance.
**/
extern void AerolinkPKI_close(int32_t connectionHandle);

#ifdef __cplusplus
}
#endif

#endif
