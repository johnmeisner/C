/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ris.h                                                            */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Radio Interface Services (RIS API for Radio Services)         */
/*              include file                                                  */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/

#ifndef _RIS_H
#define _RIS_H

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
/* This file contains the API for Radio Interface Services (RIS) */
#include "ris_struct.h"

/***********************************************************************************
 * If an application has registered for it, this user defined callback function
 * will be called when a WSA is received indicating that a desired service is
 * available.
 *
 * See IEEE 2016-1609.3 WME-Notification.indication (Section 7.4.3.1)
 *
 * To register for this callback, use the function wsuRisInitRadioComm() and set
 * the corresponding field in appCredential.
 *
 * The callback function name can be any user defined name, but it must have
 * the following arguments and return type:
 *     void cbServiceAvailableFunction(serviceInfoType *);
 *
 **********************************************************************************/
typedef void (*cbServiceAvailableType)(serviceInfoType *);

/***********************************************************************************
 * If an application has registered for it, this user defined callback function
 * will be called when a WSM is received.
 *
 * See IEEE 2016-1609.3 WSM-WaveShortMessage.indication (Section 7.3.4)
 *
 * To register for this callback, use the function wsuRisInitRadioComm() and set
 * the corresponding field in appCredential.
 *
 * The callback function name can be any user defined name, but it must have
 * the following arguments and return type:
 *     void cbReceiveWSMFunction(inWSMType *);
 *
 **********************************************************************************/
typedef void (*cbReceiveWSMType)(inWSMType *);

/***********************************************************************************
 * If an application has registered for it, these user defined callback
 * functions will be called when a WSA sign or verify request fails.
 *
 * To register for these callbacks, use the function smiInitialize().
 *
 * These callback function names can be any user defined name, but it must have
 * the following arguments and return type:
 *     void cbSignWSARequestFunction(uint8_t *, uint16_t, uint8_t);
 *     void cbVerifyWSARequestFunction(uint8_t *, uint16_t, uint8_t);
 *
 **********************************************************************************/
typedef void (*cbSignWSARequestType)(uint8_t *unsignedWSA, uint16_t wsaLength,
              uint8_t wsaHandle);
typedef void (*cbVerifyWSARequestType)(uint8_t *signedWSA, uint16_t wsaLength,
              uint8_t wsaHandle);

//==========================================================================================
/* All the following definitions are specific for the Full use standard */
/* Type definition of application credentials defining application specific parameters */
#define MIN_USER_PRIORITY    0 // Minimum Application Priority
#define MAX_USER_PRIORITY    7 // Maximum Application Priority

typedef struct {
    cbServiceAvailableType cbServiceAvailable; // Service Available callback function
    cbReceiveWSMType       cbReceiveWSM;       // Receive WSM callback function
} appCredentialsType;

/***********************************************************************************
 * This function terminates Radio Communication for the calling application.
 * The application must call this API function as the final API function call before
 * exiting.
 *
 * Parameters:
 *   None
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisTerminateRadioComm (void);

/***********************************************************************************
 * This function gets the value of various Radio Services configuration items,
 * indicated by the identifier, cfgId
 *
 * See IEEE 2016-1609.3 WME-Get.request (Section 7.4.4.1)
 *
 * Parameters:
 *   cfgId - Configuration item identifier defined by cfgIdType
 *   cfg   - Configuration item value defined by cfgType. If appropriate,
 *           cfg->radioType must be set to indicate the radio type (RT_CV2X or
 *           RT_DSRC).
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetCfgReq(cfgIdType cfgId, cfgType *cfg);

/***********************************************************************************
 * This function sets the value of various Radio Services configuration items,
 * indicated by the identifier, cfgId
 *
 * See IEEE 2016-1609.3 WME-Set.request (Section 7.4.4.3)
 *
 * Parameters:
 *   cfgId - Configuration item identifier defined by cfgIdType
 *   cfg   - Configuration item value defined by cfgType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSetCfgReq(cfgIdType cfgId, cfgType *cfg);


/***********************************************************************************
 * This function sets the value of one or more of various Radio Services configuration
 * items in as single function call.
 *
 * Parameters:
 *   rsRadioCfg - structure containing multipel radio configuration items.
 *                (Any item with the valid flag set, will be modified by the
 *                Radio Stack.)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisRadioCfgReq(rsRadioCfgType *radioCfg);

/***********************************************************************************
 * This function returns the status of the Tx queue of the specified radio.
 * True is returned if any of the 4 Tx Queues is above the IsFull threshhold.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *
 * Returns - WTRUE, if Tx queue is full. WFALSE, if Tx queue is empty.
 **********************************************************************************/
bool_t wsuRisIsRadioTxQFull(rsRadioType radioType, uint8_t radioNum);

/***********************************************************************************
 * This function returns the status of the specified Tx queue of the specified radio.
 * CCH/SCH channel type and priority queue.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *CHANType channelType: 0: CHANTYPE_CCH, 1: CHANTYPE_SCH, 2:CHANTYPE_ALL
 *             *txPriority 0-7 (TxPriority is the same as WsmOut struct and 0xff=ALL)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
bool_t wsuRisIsRadioTxQueueFull(rsRadioType radioType, uint8_t radioNum, CHANType channelType, uint8_t txPriority);

/***********************************************************************************
 * This function flushes the transmission queue by specified radio interface and
 * CCH/SCH channel type and priority queue.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *CHANType channelType: 0: CHANTYPE_CCH, 1: CHANTYPE_SCH, 2:CHANTYPE_ALL
 *             *txPriority 0-7 (TxPriority is the same as WsmOut struct and 0xff=ALL)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisFlushTxQueue(rsRadioType radioType, uint8_t radioNum, CHANType channelType, uint8_t txPriority);

/***********************************************************************************
 * This function inquires the tallies of specified radio
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *tallies: returns radio statistics
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetRadioTallies(rsRadioType radioType, uint8_t radioNum, DeviceTallyType *tallies);

/***********************************************************************************
 * This function flushes the transmission queue by specified radio interface and
 * CCH/SCH channel type and priority queue.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *CHANType channelType: 0: CHANTYPE_CCH, 1: CHANTYPE_SCH, 2:CHANTYPE_ALL
 *             *txPriority 0-7 (TxPriority is the same as WsmOut struct and 0xff=ALL)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisFlushTxQueue(rsRadioType radioType, uint8_t radioNum, CHANType channelType, uint8_t txPriority);

/***********************************************************************************
 * This function inquires the number of radios of a particular type are
 * available in the WSU devices.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             *radios: returns number of radios (1 or 2)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetNumRadios(rsRadioType radioType, uint8_t *num_radios);

/***********************************************************************************
 * This function initializes Radio Communication for the calling application.
 * It must be called when using the Full-Use version of the WAVE protocol stack.
 * The application must call this API function before any other API function call.
 *
 * Parameters:
 *   appCredential - Application specific parameters defined by appCredentialsType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisInitRadioComm (char * app_abbrev, appCredentialsType *appCredential);

/***********************************************************************************
 * This function sends a WSM.
 *
 * See IEEE 2016-1609.3 WSM-WaveShortMessage.Request (Section 7.3.2)
 *
 * Parameters:
 *   WSM - WSM parameters defined by outWSMType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendWSMReq (outWSMType *WSM);

/***********************************************************************************
 * This function registers the WSM service request functions.  It is used with the
 * Full-Use version of the WAVE radio stack.  This function is used to allow an
 * application to receive WSMs.
 *
 * See IEEE 2016-1609.3 WME-WSMService.request (Section 7.4.2.6)
 *
 * Parameters:
 *   service - definition of WSM service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisWsmServiceRequest (WsmServiceType *service);

/***********************************************************************************
 * This function registers the user service's request functions.  It is used with the
 * Full-Use version of the WAVE radio stack.  This function is used to allow an
 * application to register for services and access the SCH as a user.
 *
 * See IEEE 2016-1609.3 WME-UserService.request (Section 7.4.2.4)
 *
 * Parameters:
 *   service - definition of User service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisUserServiceRequest (UserServiceType *service);

/***********************************************************************************
 * This function changes the parameters of an existing user service request that has
 * already been registered.  It is used with the Full-Use version of the WAVE radio
 * stack.  This function is intended to be used when accepting or ending a service
 * that was advertised in a WSA, allowing access to the advertised service channel
 * and other designated parameters.
 *
 * Parameters:
 *   psid - psid Service
 *   UserServiceChangeType - currently only support service access.
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisChangeUserServiceRequest(UserServiceChangeType *change);

/***********************************************************************************
 * This function registers the EU service request functions.  It is used with the
 * Full-Use version of the WAVE radio stack for Registering for receiving non-WSM
 * (Currently EU) packets.  This function is used to allow an application to
 * supported non- ETH_P_WSMP packets based on EtherType.    This function cannot
 * be used to receive ETH_P_WSMP packets.
 *
 * Parameters:
 *   service - definition of EU service parameters for V2 messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisEUServiceRequest(EUServiceType *service);

/***********************************************************************************
 * This function requests a previously received signed WSA be verified.
 *
 * If the verification of the signed WSA is successful, the application's
 * service available callback will be called, and the serviceBitmask field in
 * serviceInfoType structure sent to the callback will have both the
 * WSA_SIGNED and WSA_VERIFIED bits set.
 *
 * Parameters:
 *   signedWsaHandle - The handle to the signed WSA that was received in the
 *                     serviceInfoType structure sent to the service available
 *                     callback in the signedWsaHandle field.
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisVerifyWsaRequest(uint32_t signedWsaHandle);

/***********************************************************************************
 * This function process provider service requests from application and sends to NS layer *
 * It is used with the Full-Use version of the WAVE radio stack.  This function is used
 * by an application to register for a provider service and transmit WSAs.
 *
 * See IEEE 2016-1609.3 WME-ProviderService.request (Section 7.4.2.2)
 *
 * Parameters:
 *   service - definition of provider service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisProviderServiceRequest(ProviderServiceType *service);

/***********************************************************************************
 * This function processes delete all services requests from the application
 * and sends them to NS layer. This function is used to delete all services for
 * a given radio number, either for all PID's or only for the calling
 * application's PID.
 *
 * Parameters:
 *   service - definition of delete all services parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisDeleteAllServicesRequest(DeleteAllServicesType *service);

/***********************************************************************************
 * This function gets the MAC address for the specified radio number.
 *
 * This function may be deprecated in the future.  It is recommended to use
 * the wsuRisGetCfg function instead.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             mac: Where to store the MAC address
 *
 **********************************************************************************/
rsResultCodeType wsuRisGetRadioMacaddress(rsRadioType radioType, uint8_t radioNum, uint8_t *mac);

/***********************************************************************************
 * This function sends an EU Packet, to be used with the WAVE Radio Stack, when
 * it is operating in EU mode.
 *
 * Parameters:
 *   EUPkt     - EU Packet defined by risEUPktType
 *   EUPktMeta - EU Packet metadata defined by risTxMetaEUPktType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendEUPacketReq(risEUPktType *EUPkt, risTxMetaEUPktType *EUPktMeta);

/***********************************************************************************
 * This function receives an EU Packet, to be used with the WAVE Radio Stack when
 * it is operating in EU mode.
 *
 * IF the WAVE Radio Stack queue of recevied EU packets is empty, this function
 * will block until a packet is received, which will then be returned.  Otherwise,
 * this function will return the next EU packet in the WAVE Radio Stack queue of
 * received EU packets.
 *
 * Parameters:
 *   EUPkt     - EU Packet defined by risEUPktType
 *   EUPktMeta - EU Packet metadata defined by risRxMetaEUPktType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisReceiveEUPacketReq(risEUPktType *EUPkt, risRxMetaEUPktType *EUPktMeta);

/***********************************************************************************
 * This function sends a raw packet, to be used with the WAVE Radio Stack.  The
 * exact data provided to this function is transmitted * over-the-air, nothing
 * more and nothing less.
 *
 * Parameters:
 *   raw - Raw packet parameters defined by outRawType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendRawPacketReq(outRawType *raw);

/***********************************************************************************
 * This function enables PCAP capture for a particualr radio.
 *
 * Parameters: pcapEnable: Parameters for the PCAP enable command
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapEnable(rsPcapEnableType *pcapEnable);

/***********************************************************************************
 * This function disables PCAP capture for a particualr radio.
 *
 * Parameters: pcapDisable: Parameters for the PCAP disable command
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapDisable(rsPcapDisableType *pcapDisable);

/***********************************************************************************
 * This function reads bytes from the PCAP buffer.
 *
 * Parameters: pcapRead: Parameters for the PCAP read command
 *             pcapBuffer: The buffer to hold the PCAP data bytes. Should be
 *                         PCAP_TX_LOG_BUF_SIZE (for tx only) or PCAP_LOG_BUF_SIZE
 *                         (in all other cases) in length.
 *             pcapBufferLen: The length of pcapBuffer. See above description for
 *                            what the size should be. Returns RS_ERANGE if the
 *                            size is less than minimum.
 *             totalLength: The acutal number of bytes read is written to the
 *                          int pointed to by totalLength.
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapRead(rsPcapReadType *pcapRead, uint8_t *pcapBuffer,
                                int pcapBufferLen, int *totalLength);

#endif // _RIS_H
