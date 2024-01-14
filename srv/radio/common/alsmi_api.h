/**************************************************************************
 *                                                                        *
 *     File Name:  alsmi_api.h                                            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#ifndef _ALSMI_API_H_
#define _ALSMI_API_H_
#include "alsmi_struct.h"
#include "rs.h"

/*
 * smiInitialize: This function should be called by application during the
 *                application startup when security is enabled.
 *
 * Input parameters:
 *        smi_init : Pointer to structure containing smi initialization
 *                   parameters.
 * Output parameters:
 *        None
 * Return Value:
 *        Success(0) or failure(-1).
 */
int smiInitialize(rsSecurityInitializeType *smi_init);

/*
 * smiTerm: Function to close and cleanup the resources. Application should
 *          call this during application shutdown.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        None.
 */
void smiTerm(void);

/*
 * smiUpdatePositionAndTime: API to update the GPS position and the number of
 *          leap seconds since 2004 for security.
 *
 * Input parameters:
 *        latitude
 *        longitude
 *        altitude
 *        leapSecondsSince2004
 *        countryCode
 * Output parameters:
 *        None.
 * Return Value:
 *        None.
 */
void smiUpdatePositionAndTime(float64_t latitude, float64_t longitude,
                              float64_t altitude, int16_t leapSecondsSince2004,
                              uint16_t countryCode);

/*
 * smiGetLeapSecondsSince2004: API to get number of leap seconds since 2004.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        The number of leap seconds since 2004.
 */
int16_t smiGetLeapSecondsSince2004(void);

/*
 * smiSendCertChangeReq: API to send certificate change request.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        0 on Success -1 on failure.
 */
int smiSendCertChangeReq(void);

/*
 * smiGetAerolinkVersion: Get the current Aerolink version
 *
 * Input parameters:
 *        len - The length of buf
 * Output parameters:
 *        buf - Where to put the Aerolink version string
 * Return Value:
 *        0 on Success -1 on failure.
 */
int smiGetAerolinkVersion(char *buf, int len);

/*
 * processSmiSignResults: Process sign results. This function should only be
 *                        called if recv->indType equals IND_SIGN_WSM_RESULTS
 *                        or IND_SIGN_WSA_RESULTS. An smiSighResults_t
 *                        structure is filled out and sent back to either
 *                        the sign results callback function or the sign WSA
 *                        results callback.
 *
 * Input parameters:
 *        recv: The IND_SIGN_WSM_RESULTS or IND_SIGN_WSM_RESULTS received from
 *              radioServices.
 * Output parameters:
 *        void
 * Return Value:
 *        NULL
 */
void processSmiSignResults(rsReceiveDataType *recv);

/*
 * processSmiVerifyWSAResults: Process verify WSA results. This function should
 *                             only be called if recv->indType equals
 *                             IND_VERIFY_WSA_RESULTS
 *
 * Input parameters:
 *        recv: The IND_VERIFY_WSA_RESULTS received from radioServices.
 * Output parameters:
 *        none
 * Return Value:
 *        void
 */
void processSmiVerifyWSAResults(rsReceiveDataType *recv);

/*
 * processCertChangeResults: Process certificate change results. This function
 *                           should only be called if recv->indType equals
 *                           IND_CERT_CHANGE_RESULTS. Calls the certificate
 *                           change results callback function.
 *
 * Input parameters:
 *        recv: The IND_CERT_CHANGE_RESULTS received from radioServices.
 * Output parameters:
 *        none
 * Return Value:
 *        void
 */
void processCertChangeResults(rsReceiveDataType *recv);

#endif // _ALSMI_API_H_

