/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
 * @file rtxCppBitString.h
 * Contains utility function for sizing a bit string.
 */
#ifndef _RTXCPPBITSTRING_H_
#define _RTXCPPBITSTRING_H_

#include "rtxsrc/rtxContext.h"

/* Note: numocts is defined as OSUINT32 to match the definition of OSDynOctStr*/

/**
 * Check whether the given bit string is large enough, and expand it if
 * necessary.
 *
 * @param pBits pBits is a pointer to the bit string, or NULL if one has not
 *    been created.  If the string is expanded, pBits receives a pointer to
 *    the new bit string.
 * @param numocts is the current size of the bit string in octets.  If the bit
 *    string is expanded, numocts receives the new size.
 * @param minRequiredBits The minimum number of bits needed in the bit string.
 *    On return, pBits will point to a bit string with at least this many bits.
 * @param preferredLimitBits The number of bits over which we prefer not to go.
 *    If nonzero, no more bytes will be allocated than necessary for this many
 *    bits, unless explicitly required by minRequiredBits.
 * @return  If successful, 0.  Otherwise, an error code.
 */
EXTERNRT int rtxCppCheckBitBounds(OSOCTET*& pBits, OSUINT32& numocts,
   size_t minRequiredBits, size_t preferredLimitBits);


/**
 * This function is identical to rtxCppCheckBitBounds except that it supports
 * a 64-bit integer length on 64-bit systems.
 *
 * @param pBits pBits is a pointer to the bit string, or NULL if one has not
 *    been created.  If the string is expanded, pBits receives a pointer to
 *    the new bit string.
 * @param numocts is the current size of the bit string in octets.  If the bit
 *    string is expanded, numocts receives the new size.
 * @param minRequiredBits The minimum number of bits needed in the bit string.
 *    On return, pBits will point to a bit string with at least this many bits.
 * @param preferredLimitBits The number of bits over which we prefer not to go.
 *    If nonzero, no more bytes will be allocated than necessary for this many
 *    bits, unless explicitly required by minRequiredBits.
 * @return  If successful, 0.  Otherwise, an error code.
 */
EXTERNRT int rtxCppCheckBitBounds64(OSOCTET*& pBits, OSSIZE& numocts,
   size_t minRequiredBits, size_t preferredLimitBits);

#endif
