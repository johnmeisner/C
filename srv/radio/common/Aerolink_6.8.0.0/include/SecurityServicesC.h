#ifndef SECURITY_SERVICES_C_H
#define SECURITY_SERVICES_C_H

/**
 * @file SecurityServicesC.h
 * @brief C interface to the SecurityServices.
 */

#include <stdint.h>
#include <time.h>

#include "aerolink_api.h"
#include "SecurityContextC.h"
#include "IdChangeCallback.h"
#include "CustomInitialization.h"
#include "ws_errno.h"
#include "AerolinkReportData.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for Aerolink configuration.
 */
typedef struct {
    uint8_t TBD;
} AEROLINK_CONFIG;



/**
 * @brief Initialize the security services library from configuration files.
 * This routine must to be called once prior to any other library function.
 * This routine is an alternative to securityServices_initWithConfig() which
 * reads the configuration from a structure.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_INVALID_GLOBAL_CONFIG
 *     - global configuration file does not exist
 *     - global configuration file cannot be opened, is not properly formatted,
 *       or contains missing or invalid data
 *
 * @retval WS_ERR_INVALID_CONTEXT_CONFIG
 *     - any context configuration file cannot be opened, is not properly
 *       formatted, or contains missing or invalid data
 *     - no profile configuration file exists for any PSID/SSP listed in any
 *       context configuration file
 *
 * @retval WS_ERR_INVALID_PROFILE_CONFIG
 *     - any profile configuration file cannot be opened, is not properly
 *       formatted, or contains missing or invalid data
 *     - more than one profile configuration file contains the same PSID entry
 *
 * @retval WS_ERR_MEMORY_FAILURE
 *     - any problem allocating or accessing shared memory
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_initialize(void);



/**
 * @brief Initialize the security services library from a configuration
 * structure.
 * This routine must to be called once prior to any other library function.
 * This routine is an alternative to securityServices_initialize() which
 * reads the configuration from files.
 *
 * @param config (IN)  Configuration structure.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_INVALID_GLOBAL_CONFIG
 *     - global configuration does not exist
 *     - global configuration is not properly formatted,
 *       or contains missing or invalid data
 *
 * @retval WS_ERR_INVALID_CONTEXT_CONFIG
 *     - any context configuration is not properly
 *       formatted, or contains missing or invalid data
 *     - no profile configuration file exists for any PSID/SSP listed in any
 *       context configuration
 *
 * @retval WS_ERR_INVALID_PROFILE_CONFIG
 *     - any profile configuration is not properly
 *       formatted, or contains missing or invalid data
 *     - more than one profile configuration contains the same PSID entry
 *
 * @retval WS_ERR_MEMORY_FAILURE
 *     - any problem allocating or accessing shared memory
 */

/***
 * @brief Pass custom setting parameters to Aerolink for select services.
 *
 * @param service (IN) Name of service to handle the settings
 * @param numberOfSettings (IN) The number of entries in settings
 * @param settings  (IN) An array of strings specified as key value pairs
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - the library has not been previously initialized.
 *
 * @retval WS_ERR_UNSUPPORTED_CUSTOM_SERVICE
 *     - Unsupported Aerolink service.
 *
 * @retval WS_ERR_UNSUPPORTED_CUSTOM_SETTING
 *     - Unsupported Aerolink custom setting.
 *
 * @retval WS_ERR_INVALID_CUSTOM_SETTING_FORMAT
 *     - Invalid Aerolink custom setting format.
 *
 * @retval WS_ERR_SERVICE_DISABLED
 *     - Cannot accept the custom setting because the specified service is disabled.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_customSettings(
    CustomInitServiceType   service,
    uint32_t                numberOfSettings,
    uint8_t const          *settings[]);

/**
 * @brief Shut down the security services library.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - the library has not been previously initialized.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_shutdown(void);

/***
 * @brief Register the callback to receive Aerolink status or error report.
 *
 * @param reportIds (IN) contains one or more values of AerolinkReportIdBitmap, representing all
 *       reportIds the types of reports that should be provided using the reporting callback function.
 * @param reportCallbackFunction (IN) Function to be called when there are errors or status to report
 * @param userData (IN) user supplied data that will be returned in the callback.

 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - the library has not been previously initialized.
 *
 * @retval WS_ERR_ALREADY_REGISTERED
 *     - If a callback is already registered.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - If reportIds is 0 or contains any undefined bits set to 1.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_registerReportCallback(
        uint32_t                reportIds,
        AerolinkReportCallback  reportCallbackFunction,
        void                   *userData);

/***
 * @brief Register the callback to receive Aerolink status or error report.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - the library has not been previously initialized.
 *
 * @retval WS_ERR_NOT_REGISTERED
 *     - If a callback is not registered.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_unRegisterReportCallback (void);


/**
 * @brief Update the current geographic location.
 *
 * @param latitude (IN)  Latitude of the current location.
 * @param longitude (IN) Longitude of the current location.
 * @param elevation (IN) Elevation of the current location.
 * @param countryCode (IN) Country code of the current location.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - latitude is an invalid value
 *     - longitude is an invalid value
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_setCurrentLocation(
        int32_t  latitude,
        int32_t  longitude,
        uint16_t elevation,
        uint16_t countryCode);



/**
 * @brief Update the current time adjustment.
 *
 * @param seconds (IN)  Number of leap seconds since 00:00:00 1 Jan 2004 UTC
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_setTimeAdjustment(
        int32_t seconds);



/**
 * @brief Return the version of the library as a string.
 *
 * @return String of the library version.
 *         This pointer should not be freed by the caller.
 *
 */
AEROLINK_EXPORT char const *
securityServices_getVersion(void);



/**
 * @brief Register an LCM to participate in the ID-change protocol.
 *
 * @param scHandle (IN)  of the context containing the LCM to be registered
 * @param lcmName (IN) to be registered
 * @param userData (IN) user supplied data that will be returned in the callback.
 * @param initCallbackFunction (IN) callback function to be called at the start of an id change.
 * @param doneCallbackfunction (IN) callback function to be called at the completion of an id change.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle  is NULL
 *     - lcmName   is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle  does not refer to a valid security context
 *
 * @retval WS_ERR_ALREADY_REGISTERED
 *     -  the specified LCM has already been registered
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_idChangeRegister(
        SecurityContextC             scHandle,
        char const                  *lcmName,
        void                        *userData,
        IdChangeInitCallback         initCallbackFunction,
        IdChangeDoneCallback         doneCallbackfunction);


/**
 * @brief Unregister an LCM to participate in the ID Change protocol.
 *
 * @param scHandle (IN)  of the context containing the LCM to be unregistered
 * @param lcmName (IN) to be unregistered
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle  is NULL
 *     - lcmName   is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid security context
 *
 * @retval WS_ERR_NOT_REGISTERED
 *     -  the specified LCM is not registered
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_idChangeUnregister(
        SecurityContextC             scHandle,
        char const                  *lcmName);



/**
 * @brief Request an ID Change initiation.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_ID_CHANGE_BLOCKED
 *     - Id changes are blocked
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_idChangeInit(void);


/**
 * @brief Apply a lock to block ID Change initiations
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_idChangeLock(void);


/**
 * @brief Remove a lock to allow ID Change initiations.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_ID_CHANGE_NOT_LOCKED
 *     - There are no locks to be removed
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_idChangeUnlock(void);

#ifdef __cplusplus
}
#endif

#endif /* SECURITY_SERVICES_C_H */
