/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps_api.h                                                    */
/*     Purpose: Library of functions to communicate with the TPS process     */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Contains functions to communicate with the TPS process       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/** @file tps_api.h
 *  @brief Time Position Services API
 *
 *  @addtogroup TPS
 *  @{
 */


#ifndef TPS_API_H
#define TPS_API_H

#include "tps_types.h"


/**
 * This function converts a tpsResultCodeType to a string.
 *
 * @param result   The tpsResultCodeType to convert.
 *
 * @return The string representation of "result"
 */
char *tpsResultCodeType2Str(tpsResultCodeType result);

/**
 * This function initializes the interface to TPS for the calling application.
 *        The application must call this API function before any other API function call.
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
tpsResultCodeType wsuTpsInit(void);

/**
 * This function registers the User Application to TPS to receive the TPS data
 *         once per second.
 *
 * @param registerCB   User Application's callback function for the TPS data.
 *                     TPS data is passed to this callback function.
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
tpsResultCodeType wsuTpsRegister(void (*registerCB)(tpsDataType *));

/**
 * This function de-registers the User Application from TPS.
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
tpsResultCodeType wsuTpsDeregister(void);

/**
 * This function requests to TPS to receive one TPS data (one time request only).
 *
 * @param requestCB   User Application's callback function for the TPS data.
 *                    TPS data is passed to this callback function.
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
tpsResultCodeType wsuTpsRequest(void (*requestCB)(tpsDataType *));

/**
 * This function terminates TPS service for the calling application.
 *        The application must call this API function as the final API function
 *        call before exiting.
 *
 * @return None
 */
void wsuTpsTerm(void);

/**
 * This function requests TPS to send GPS Correction data to GPS receiver 
 *        through serial port of WSU. 
 *
 * @param p_data   Pointer to a unsigned character buffer containing 
 *  	       GPS correction data. 
 * @param length   Size of the data in the character buffer. Note the 
 *  	      data size should be between 1 and 500.
 * @return   Size of the GPS correction data sent to the serial port(GPS Receiver), or a value of -1 to indicate an error. 
 */
int32_t wsuTpsSndGpsOutput(char_t * p_data, int32_t length);

/**
 * By calling this function with a specific debug output type as a parameter 
 * will enables TPS to show  all debug messages related to that specific type.
 *
 * @param OutputType : Bit mask to switch on/off various types of debug messages from TPS.
 *         - Valid Bit masks are:
 *             - DBG_OUTPUT_DISABLE  0x00 --> disables all debug msgs.
 *             - DBG_OUTPUT_NMEA     0x01 --> shows NMEA data from serial.
 *             - DBG_OUTPUT_TIME     0x02 --> Show time synchronization data.
 *             - DBG_OUTPUT_ALL      0xffffffff --> Turns on all debug msgs.
 *             .
 *         Note: Do bitwise-OR, to turn on more than one type of debug message.
 *         .
 * 
 * @return None 
 */
void  wsuTpsShowDebugMsg(uint32_t OutputType);

/**
 * TPS will call the callback function with error codes. 
 *
 * @param errHandlerCB : Error handler callback function 
 *
 * @return None
 */
void wsuTpsRegisterErrorHandler(void (*errHandlerCB)(tpsResultCodeType));

/**
 * TPS will call the callback function with RTCM data and its length. 
 *
 * @param handleRTCMCB RTCM data callback function 
 *
 * @return None
 */
void wsuTpsRegisterRTCMHandler(void (*handleRTCMCB)(char_t *, int32_t));

/**
 * receiving raw messages which terminates with a CR-LF (0x0d,0x0a) combination. 
 * Once the message with the specified header is received on GPS serial port, then
 * the call back function will be called with the received data.
 * The following parameters should be passed while calling this function.
 *
 * @param handleRawMsgCb   A pointer to a callback function that takes two 
 *            parameters, 1 - pointer to Raw data received and
 *            2 - Length of the data. Type void (*handleRawMsgCB)(int8_t *, int32_t).
 * 
 * @param msgHeader    A null terminated raw data prefix/header string 
 *
 * @param raw_hdrlen   Length of the raw data prefix/header string. Maximum 
 *                     string length supported is 20 bytes
 * 
 * @param msgUpdIntvl  Data update interval (in ms), which indicates how often the 
 *                     call back function gets called with the latest data from TPS.
 *                     A value of 0 means, call back function will be called only
 *                     when the data arrives at GPS serial port. 
 *                     Currently, only the 0 option is supported.
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 *
 * @note Some example Raw message header/prefix:  BESTUTMA, $GPRMC
 *
 */
int32_t wsuTpsRegisterRawMsgHandler(void (*handleRawMsgCB)(char_t *, int32_t), 
        char_t *msgHeader, int32_t raw_hdrlen, int32_t msgUpdIntvl);

/**
 * This function composes a message to send to the U-Blox module in UBX format
 * to set the wheel tick count.
 *
 * @param WheelTickData - tpsWheelTickData_t struct containing the wheel tick data
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
#if defined(ENABLE_WHEELTIX)
int32_t wsuTpsSendWheelTicks(WheelTickData_t WheelTickData);
#endif /* ENABLE_WHEELTIX */
/**
 * This function deregisters a specific RAW message from TPS registration. 
 *    When called with a Raw message header, TPS stops calling the call back 
 *    function with the specified RAW message data. Following parameters 
 *    should be specified while calling this function.
 * 
 * @param msgHeader   A null terminated raw data prefix/header string.
 *
 * @param raw_hdrlen  Length of the raw data header string. maximum string length 
 *                     supported is 20 bytes
 *
 * @return Result code defined by tpsResultCodeType (TPS_SUCCESS or TPS_FAILURE)
 */
int32_t wsuTpsDeregisterRawMsgHandler(char_t *msgHeader, int32_t raw_hdrlen);


/**
 * This function queries and returns the current gpsDebugOutput mask.
 *
 * @return a uint32_t containing the bitmask controlling gps's debug output
 */
uint32_t wsuTpsGetDebugOutputMask(void);

/**
 * Cold start Ublox GNSS chipset.
 */
void  wsuTpsColdStartuBlox(void);

/**
 * Toggle Stop/Start GNSS chipset. No Reset.
 */
void  wsuTpsStopuBlox(void);

/**
 * Dump SPI stats.
 */
void wsuTpsDumpSPIstats(void);

/**
 * Dump GNSS HW Status (UBX-MON-HW).
 */
void wsuTpsDumpGNSSHWstatus(void); 

/**
 * Dump Ublox CFG (UBX-MON-VER).
 */
void wsuTpsDumpUbloxConfigs(void);

/**
 * Set Ublox CFG (UBX-XXX-YYY).
 */
void wsuTpsSendDefaultUbloxCfgs(void);


#endif
/**@}*/
