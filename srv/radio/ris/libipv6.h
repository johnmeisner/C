/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: libipv6.h                                                        */
/*  Purpose: Functions to set up and tear down the IPv6 connection include    */
/*           file                                                             */
/*                                                                            */
/* Copyright (C) 2020 DENSO International America, Inc.                       */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2020-09-02  VROLLINGER    Initial release.                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef _LIBIPV6_H
#define _LIBIPV6_H

#include "ris_struct.h"

/***********************************************************************************
 * This function deletes the existing IPv6 information if present, and sets the
 * IP address and default gateway.
 *
 * Note: Use of this functions requires you to compile and link in the
 * ndp_lib.c file into your program.
 *
 * Parameters:
 *   serviceInfo - Pointer to information about the V2 service.
 *   radio - The radio to configure.
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSetIPv6Info(serviceInfoType *serviceInfo, uint8_t radio);

/***********************************************************************************
 * This function deletes the existing IPv6 information if present.
 *
 * Note: Use of this functions requires you to compile and link in the
 * ndp_lib.c file into your program.
 *
 * Parameters:
 *   radio - The radio to de-configure.
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisTeardownIPv6Info(uint8_t radio);

/***********************************************************************************
 * This function sets the radio number for use with IPv6. It is generally used
 * only by applications that start a provider service.
 *
 * Note: Use of this functions requires you to compile and link in the
 * ndp_lib.c file into your program.
 *
 * Parameters:
 *   radio - The radio number to use for IPv6 communication.
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSetRadioForIPv6(uint8_t radio);

#endif // _LIBIPV6_H

