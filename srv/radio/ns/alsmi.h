/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: alsmi.h                                                          */
/*  Purpose: The WME module of radioServices                                  */
/*                                                                            */
/* Copyright (C) 2020 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#ifndef _ALSMI_H_
#define _ALSMI_H_

#include "alsmi_struct.h"
#include "ris_struct.h"

/*
 * This function sets debug_log equal to the value of the nsconfig
 * ALSMI_DEBUG_LOG_ENABLE value.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiSetDebugLogValue(void);

/*
 * This function should be called by radioServices during startup.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      Success(0) or failure(-1).
 */
int alsmiInitialize(void);

/*
 * Function to close and cleanup the resources. radioServices should call this
 * during shutdown.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiTerm(void);

/*
 * This function should be called by an application to register for signing
 * packets with security.
 * Input parameters:
 *        debug_flag_forsmi   : Debug flag. When enabled creates debug file in the path as specified in psmiConfigParams->debugLogFilename
 *        smi_vodEnabled : TRUE if VOD is enabled, FALSE otherwise
 *        vtp_milliseconds_per_verification : Millieconds per verification in VTP mode.
 *        smi_sign_results_options : Which sign results to return.
 *        smi_sign_wsa_results_options : Which sign WSA results to return.
 *        smi_verify_wsa_results_options : Which verify WSA results to return.
 *        lcmNameParam        : LCM name, obtained from the denso.wsc file.
 *        numSignResultsPSIDs : Number of PSID's to be signed.
 *        signResultsPSIDs    : PSID's to be signed. numSignResultsPSIDs is the length of the array.
 *        smi_sign_wsa_results_pid : Process ID of sign WSA results application.
 *        smi_verify_wsa_results_pid : Process ID of verify WSA results application.
 *        smi_cert_change_pid : Process ID of certificate change results application.
 *        pRegIdx             : Value passed back to the caller. This value is passed to the alsmiUnregister() function.
 *
 * Output parameters: None
 * Return Value:      Success(0) or failure(-1).
 */
int alsmiRegister(bool_t                        debug_flag_forsmi,
                  bool_t                        smi_vodEnabled,
                  uint32_t                      vtp_milliseconds_per_verification,
                  enum smi_results_e            smi_sign_results_options,
                  enum smi_results_e            smi_sign_wsa_results_options,
                  enum smi_results_e            smi_verify_wsa_results_options,
                  char                         *lcmNameParam,
                  uint8_t                       numSignResultsPSIDs,
                  uint32_t                     *signResultsPSIDs,
                  pid_t                         smi_sign_wsa_results_pid,
                  pid_t                         smi_verify_wsa_results_pid,
                  pid_t                         smi_cert_change_pid,
                  uint8_t                      *pRegIdx);

/*
 * This function should be called by an application to unregister from signing
 * packets with security.
 * Input parameters:
 *        regIdx : Value returned from alsmiRegister()
 *        sign_wsa_results   : Set to WTRUE if the process that called alsmiInitialize wanted sign WSA results
 *        verify_wsa_results : Set to WTRUE if the process that called alsmiInitialize wanted verify WSA results
 *        cert_change        : Set to WTRUE if the process that called alsmiInitialize wanted certificate change results
 * Output parameters:  None
 * Return Value:       Use count after terminating this instance.
 */
int alsmiUnregister(uint8_t regIdx, bool_t sign_wsa_results,
                    bool_t verify_wsa_results, bool_t cert_change);

/*
 * alsmiVerifyThisOne: API get a WSM of interest verified in the case of VOD.
 *
 * A call to this function may eventually result in a call to the verification
 * result callback (alsmiValidateVODCallback()), either immediately or at a
 * later time.
 *
 * Input parameters:
 *             vodMsgSeqNum: The VOD message sequence number of the message to be verified.
 *             recvCmdSock: Socket to send the results to.
 *             sender_port: Port to send the results to.
 * Output parameters:
 *             radioType: The radio type the packet being verified was
 *                        originally received on.
 * Return Value: RS_SUCCESS on Success, error code on failure. If RS_SUCCESS is
 *               returned, then either alsmiVerifyThisOne() or
 *               alsmiValidateVODCallback() will respond to the
 *               WSU_NS_SECURITY_VERIFY_WSM_REQ command, so
 *               radioServicesReceiveCommandsThread() should not. If something
 *               other than RS_SUCCESS is returned, then the
 *               WSU_NS_SECURITY_VERIFY_WSM_REQ command will not be responded
 *               to, so radioServicesReceiveCommandsThread() should respond to
 *               it.
 */
rsResultCodeType alsmiVerifyThisOne(uint32_t vodMsgSeqNum, int recvCmdSock,
                                    uint16_t sender_port, rsRadioType *radioType);

/*
 * alsmiSendCertChangeReq: API to send certificate change request to on-board/off-board Ubuntu SM/esBOX.
 *
 * Input parameters:
 *             None.
 * Output parameters:
 *             None.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiSendCertChangeReq(void);

/******************************************************************************
 *           NEW FUNCTIONS FOR ALSMI ARE IMPLEMENTED IN THIS SECTION          *
 ******************************************************************************/

/*
 * alsmiGetVersion
 *
 * Returns a pointer to a string containing the Aerolink software version.
 *
 * Input parameters:
 *             None
 * Output parameters:
 *             None
 * Return Value:  pointer to a string containing the Aerolink software version
 */
const char *alsmiGetVersion(void);

/*
 * alsmiUpdatePositionAndTime
 *
 * Updates the current position and leap seconds since 2004. Called from TPS
 * when new GPS data is received.
 *
 * Input parameters:
 *             latitude, longitude, elevation - Current GPS position
 *             leapSecondsSince2004 - Leap seconds since 1/1/2004, 00:00:00 UTC
 *             countryCode - field representing the country the device is
 *                           currently in
 * Output parameters:
 *             None
 * Return Value:  None
 */
void alsmiUpdatePositionAndTime(double latitude, double longitude,
                                double elevation, int16_t leapSecondsSince2004,
                                uint16_t countryCode);

/*
 * alsmiGetLeapSecondsSince2004
 *
 * Gets the number of leap seconds since 2004. alsmiUpdatePositionAndTime()
 * must have been called some time previous to this function being called.
 *
 * Input parameters:
 *             None.
 * Output parameters:
 *             None
 * Return Value:  Number of leap seconds since 2004.
 */
int16_t alsmiGetLeapSecondsSince2004(void);

/*
 * alsmiSign
 *
 * Signs a packet. When signing is complete, a callback routine (generally in
 * radioServices) is called to transmit the packet. Also, the sign result
 * callback is called if desired.
 *
 * Input parameters:
 *             unsignedData:       Pointer to the data to be signed
 *             unsignedDataLength: Length of the data to be signed
 *             additionalData:     Pointer to additional data outside of the
 *                                 signed data area.
 *                                 This is the basic header for an EU packet.
 *             additionalDataLen:  Additional data length
 *             pktType:            Set to ALSMI_PKT_TYPE_EU if EU data packet,
 *                                 ALSMI_PKT_TYPE_WSMP if BSM WSMP packet,
 *                                 ALSMI_PKT_TYPE_WSA if WSA WSMP packet
 *             additionalSecurityParameters:
 *                                 Additional security parameters. This is a
 *                                 pointer to a sendWSMStruct_s for BSM WSMP
 *                                 packet. This is a pointer to a
 *                                 risTxMetaEUPktType for an EU packet. This is
 *                                 a pointer to a wsaTxSecurityType for a WSA
 *                                 WSMP packet.
 *
 * Output parameters:
 *             None
 * Return Value:  WS_SUCCESS on Success; Error code on failure.
 */
AEROLINK_RESULT alsmiSign(uint8_t  *unsignedData,
                          uint16_t  unsignedDataLength,
                          uint8_t  *additionalData,
                          uint8_t   additionalDataLen,
                          uint8_t   pktType,
                          void     *additionalSecurityParameters);

/*
 * alsmiAddUnsecured1609p2Hdr
 *
 * Adds a 1609.2 header to a packet. When complete, a callback routine
 * (generally in Radio_ns) is called to transmit the packet. Also, the sign
 * result callback is called if desired. The packet isn't really signed, but
 * we want to call the sign results callback to minimize the amount of special
 * case code that has to be written.
 *
 * Note that if the 1609 version is 3, then the sendWSMStruct.wsmpHdrLen
 * assumes that the data length will require 2 bytes. If it turns out that,
 * after adding the 1609.2 header the data length only requires 1 byte, then
 * the sendWSMStruct->wsmpHdrLen will need to be decremented by 1, and the
 * data will be shifted to the left one byte before sendWSMActual() is called.
 *
 * Input parameters:
 *             WSA:                    TRUE if this is a WSA, FALSE otherwise
 *             inData:                 Pointer to the data to have the header
 *                                     added
 *             inDataLength:           Length of that data
 *             additionalSecurityData: Pointer to additional security data.
 *                                     This is a pointer to a wsaTxSecurityType
 *                                     if WSA is TRUE. This is a pointer to a
 *                                     sendWSMStruct_S if WSA is FALSE.
 * Output parameters:
 *             None
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiAddUnsecured1609p2Hdr(bool_t    WSA,
                               uint8_t  *inData,
                               uint16_t  inDataLength,
                               void     *additionalSecurityData);

/*
 * alsmiVerifyThenProcess: Strips the security information off of a packet and,
 * if the packet was signed, and also verifies it periodically.
 *
 * Input parameters:
 *             data: Pointer to the data to be verified. Note: points into the
 *                   structure pointed to be pPkb below.
 *             dataLength: Length of the data to be verified
 *             pPayload: Pointer to a pointer to the payload. This value is
 *                       written when the security is stripped.
 *             pPayloadLength: Pointer to to the payload length. This value is
 *                             written when the security is stripped.
 *             pktType: ALSMI_PKT_TYPE_WSMP, or ALSMI_PKT_TYPE_EU.
 *             pSecurity: Points to the Security field of the rsReceiveDataType
 *                        structure (for ALSMI_PKT_TYPE_WSMP) or to the
 *                        security field of the euPacketInfo structure (for
 *                        ALSMI_PKT_TYPE_EU).
 *             pSecurityFlags: Points to the SecurityFlags field of the
 *                             rsReceiveDataType structure (for
 *                             ALSMI_PKT_TYPE_WSMP) or to the securityFlags
 *                             field of the euPacketInfo structure (for
 *                             ALSMI_PKT_TYPE_EU).
 *                             securedVerifiedSuccess, unsecured,
 *                             securedButNotVerified, or securedVerifiedFail is
 *                             eventually stored in the location where this
 *                             points to.
 *             offsetToBasicHeader: Offset from pPkb->data to the start of the
 *                                  basic header. Used by ALSMI_PKT_TYPE_EU to
 *                                  calculate the offset to the start of the
 *                                  common header. Not used by
 *                                  ALSMI_PKT_TYPE_WSMP.
 *             pOffsetToCommonHeader: Points to the offsetToCommonHeader field
 *                                    of the euPacketInfo structure for
 *                                    ALSMI_PKT_TYPE_EU. Not used by
 *                                    ALSMI_PKT_TYPE_WSMP and can be set to
 *                                    NULL.
 *             pSspLen: Pointer to where to store the the SSP data length. Not
 *                      used by ALSMI_PKT_TYPE_EU.
 *             ssp: Pointer to where to store the SSP data. Not used by
 *                  ALSMI_PKT_TYPE_EU.
 *             pVodMsgSeqNum: Pointer to where to write the VOD message sequence number.
 *             psid: Needed in case the packet is unsecured. The WSMP header
 *                   will already have been stripped off, and the PSID will not
 *                   be in the 1609.2 header (if present). But the PSID will be
 *                   needed in order to determine which queue to eventually put
 *                   the packet on.
 *             radioType: The radio type the packet was received on.
 * Output parameters:
 *             The data pointed to by pSecurity and pSecurityFlags variables
 *             will be updated.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiVerifyThenProcess(uint8_t        *data,
                           uint16_t        dataLength,
                           uint8_t const **pPayload,
                           uint16_t       *pPayloadLength, 
                           uint8_t         pktType,
                           securityType   *pSecurity,
                           uint32_t       *pSecurityFlags,
                           uint32_t        offsetToBasicHeader,
                           uint8_t        *pOffsetToCommonHeader,
                           uint8_t        *pSspLen,
                           uint8_t        *ssp,
                           uint32_t       *pVodMsgSeqNum,
                           uint32_t        psid,
                           rsRadioType     radioType);

/*
 * alsmiVerifyThenProcessIeee: Strips the security information off of an IEEE
 * packet and, if the packet was signed, and also verifies it periodically.
 *
 * Input parameters:
 *             recv:             Pointer to the rsReceiveDataType structure
 *                               associated with this packet.
 *             packet: Pointer to the data to be verified. Note: points into the
 *                   structure pointed to be pPkb below.
 *             packetLength: Length of the data to be verified
 *             pPayload: Pointer to a pointer to the payload. This value is
 *                       written when the security is stripped.
 *             pPayloadLenget: Pointer to to the payload length. This value is
 *                             written when the security is stripped.
 *             radioType: The radio type the packet was received on.
 * Output parameters:
 *             The data pointed to by pSecurity and pSecurityFlags variables
 *             will be updated.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiVerifyThenProcessIeee(rsReceiveDataType *recv, uint8_t *packet,
                               uint16_t packetLength, uint8_t const **pPayload,
                               uint16_t *pPayloadLength, rsRadioType radioType);

/*
 * alsmiVerifyWSA: Strips the security information off of a WSA and verifies
 * it. The message does not go through the message buffer.
 *
 * A call to this function may eventually result in a call to
 * alsmiVerifyWSACallback1() (perhaps before alsmiVerifyWSA() returns.)
 *
 * Input parameters:
 *             wsaData: Pointer to the WSA data to be verified
 *             wsaLen: Length of the WSA data to be verified
 *             signedWsaHandle: Handle to the signed WSA
 * Output parameters:
 *             vodResult: The Aerolink result code is written to here
 * Return Value:  RS_SUCCESS on success, RS_EVERWSAFAIL on failure
 */
rsResultCodeType alsmiVerifyWSA(uint8_t *wsaData, uint16_t wsaLen,
                                uint32_t signedWsaHandle,
                                AEROLINK_RESULT *vodResult);

/*
 * This function sets the certChangeTimeoutTestCount variable to the specified
 * amount. In wint32 alsmiSendCertChangeReq(), the call to
 * securityServices_idChangeInit() will be suppressed this many times, causing
 * the certificate change to time out.
 * Input parameters:  count - The value to set certChangeTimeoutTestCount to.
 * Output parameters: None
 * Return Value:      None
 */
void alsmiSetCertChangeTimeoutTestCount(uint16_t count);

/*
 * This function simulates an unsolicited certificate change from Aerolink.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiUnsolicitedCertChangeTest(void);

/*
 * This function processes P2P data.
 * Input parameters:
 *             data: P2P data length.
 *             len: P2P data length.
 * Output parameters:
 *             None
 * Return Value:  0 if success. Error code if error.
 */
int alsmiProcessP2PData(uint8_t *data, uint16_t len);

#endif // _ALSMI_H_

