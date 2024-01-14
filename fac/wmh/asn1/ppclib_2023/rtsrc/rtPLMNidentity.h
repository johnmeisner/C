/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/
/**
 * @file rtPLMNidentity.h
 * PLMNidentity utility functions.
 */
#ifndef _RTPLMNIDENTITY_H_
#define _RTPLMNIDENTITY_H_

#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxExternDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function converts MCC and MNC null-terminated string values to a
 * PLMNidentity.  The source strings should contain only characters '0' -
 * '9'.  Otherwise, an error will occur.
 *
 * The PLMNidentity format is described in (for example) 3GPP TS 25.413.
 * Each nibble represents a digit from 0 through 9 inclusive.  The first 3
 * nibbles are the digits of the MCC.  If the MNC is 2 digits long, the
 * fourth nibble is a filler of 0xf.  Otherwise, the fourth nibble is the
 * first digit of the MNC.  Then the remaining 2 nibbles are the remaining
 * digits of the MNC.
 *
 * @param mcc          Null-terminated string for the MCC.
 * @param mnc          Null-terminated string for the MNC.
 * @param plmnBuffer   The destination buffer.  The size of this buffer must
 *                     be at least 3 octets.
 * @param bufsiz       The size of the destination buffer (in octets, must be
 *                     at least 3).  If larger than 3, octets after the third
 *                     one will not be changed.
 * @return             0 for success or a negative value if an error occurs.
 */
EXTERNRT int rtStringToPLMNidentity
   (const char* mcc, const char* mnc, OSOCTET* plmnBuffer, size_t bufsiz);

/**
 * This function retrieves an MCC value from a PLMNidentity and returns it
 * as a null-terminated string.  Refer to the description of PLMNidentity in
 * rtStringToPLMNidentity().  This function will allocate the number of bytes
 * required to hold the MCC string.  This memory can be released via the
 * free() function.
 *
 * @param plmnIdentity   Pointer to the PLMNidentity octet array.  The function
 *                       assumes this array is at least 3 bytes long.
 * @return               Null-terminated string for the MCC, or 0 if an error
 *                       occurs.
 */
EXTERNRT const char* rtGetMCCFromPLMNidentity
   (OSOCTET* plmnIdentity);

/**
 * This function retrieves an MNC value from a PLMNidentity and returns it
 * as a null-terminated string.  Refer to the description of PLMNidentity in
 * rtStringToPLMNidentity().  This function will allocate the number of bytes
 * required to hold the MNC string.  This memory can be released via the
 * free() function.
 *
 * @param plmnIdentity   Pointer to the PLMNidentity octet array.  The function
 *                       assumes this array is at least 3 bytes long.
 * @return               Null-terminated string for the MNC, or 0 if an error
 *                       occurs.
 */
EXTERNRT const char* rtGetMNCFromPLMNidentity
   (OSOCTET* plmnIdentity);

/**
 * @} PLMNidentityHelper
 */

#ifdef __cplusplus
}
#endif

#endif
