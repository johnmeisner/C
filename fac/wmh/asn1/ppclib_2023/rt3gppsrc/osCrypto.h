/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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
 * @file oscrypto.h
 * Cryptographic functions and definitions.
 */
#ifndef __OSCRYPTO_H__
#define __OSCRYPTO_H__

/*
The functions in this file will be declared static if STATIC_CRYPTO is defined.
This is done by rt3gppNasSec.c, just to avoid making these functions exported
when it is not necessary.
*/

//In this file, use OSCRYPTO_RT in place of EXTERNRT
#ifdef STATIC_CRYPTO
#define OSCRYPTO_RT static
#define OSCRYPTO_RTMETHOD static
#else
#define OSCRYPTO_RT EXTERNRT
#define OSCRYPTO_RTMETHOD EXTRTMETHOD
#endif


#include "rtxsrc/rtxContext.h"

#ifdef __cplusplus
extern "C" {
#endif

//Assign a 32-bit word to the 4 bytes located at b.
//w is a pointer to the 32-bit word.
//b is a pointer to the byte which shall receive the MSB of w.
#define WORD_TO_BYTES(w,b) \
      b[0] = w >> 24; \
      b[1] = w >> 16 & 0xFF; \
      b[2] = w >> 8 & 0xFF; \
      b[3] = w & 0xFF;


//Form a 32-bit word out of the 4 bytes located at x, where x is a pointer
//to an OSOCTET and will become the MSB of the result.
#define WORD(x) ( ((x)[0] << 24) ^ ((x)[1] << 16) ^ ((x)[2] << 8) ^ (x)[3] )

/*
Representation of AES-128 key schedule.
We might change this to a struct if we add support for other size keys,
as the size of the key schedule depends upon the size of the key.
*/
typedef OSUINT32 OSCryptAESKeySchedule[44];

typedef OSOCTET OSCrypt128Key[16];    //A 128-bit key

/*
The AES state is a 4 x 4 table of bytes. The chosen representation here is
as 4 32-bit words, each representing a column of the state.
When the state is filled with the plaintext, the first four bytes form the first
column, the next four bytes the second column and so on.
*/
typedef OSUINT32 OSCryptAESState[4];


/**
 * Function pointer type for functions that implement a counter increment, where
 * one 128-bit counter block is derived from another.
 */
typedef int (* CTR_INCR) (OSOCTET counter[16], OSOCTET nextCounter[16]);


/**
 * Perform the AES Key expansion of key into schedule.
 * For a given key, the result of the expansion is fixed, so this method only
 * needs to be invoked once per key.
 *
 * @param schedule Receives the key schedule, the result of key expansion.
 * @param key The 128 bit key.
 */
OSCRYPTO_RT void rtxCryptAES128KeyExpansion(OSCryptAESKeySchedule schedule,
                             OSCrypt128Key key);

/**
 * Encipher data using block cipher AES-128 (AES with 128-bit key).\
 * AES is specified in NIST FIPS PUB 197, November 2001.
 *
 * @param pctxt context to use for any memory allocations.
 * @param keySchedule The key schedule produced by rtxCryptAES128KeyExpansion
 * @param plaintext The plaintext message to be enciphered.
 * @param ciphertext The resulting ciphertext. This may point to the same
 *    memory as plaintext.
 * @param msgBytes The size of both plaintext and ciphertext, in bytes.
 *    This must be a multiple of 16; AES operates on 128-bit blocks.
 * @return completion status: 0 for success; < 0 is an error code.
 */
OSCRYPTO_RT int rtxCryptEncipherAES128(OSCTXT* pctxt,
   OSCryptAESKeySchedule keySchedule,
   OSOCTET plaintext[], OSOCTET ciphertext[], size_t msgBytes );


/**
 * Calculate a CMAC (Cipher-based Message Authentication Code) using AES-128 as
 * the block cipher. CMAC mode is specified in NIST PUB 800-38B, May 2005.
 *
 * @param pctxt context to use for any memory allocations
 * @param keySchedule The key schedule obtained using rtxCryptAES128KeyExpansion
 * @param subkeys Subkeys previously derived from the keySchedule (and thus,
 *    ultimately derived from the same key as keySchedule)
 * @param message The message on which the CMAC is to be calculated.
 * @param msgBytes The size of the message, in bytes.\
 * @param mac Receives the MAC. If your application of CMAC calls for fewer
 *    than 16 bytes for the MAC, as many of the most significant bytes of T
 *    as required should be used.
 * @return completion status: 0 for success; < 0 is an error code.
 */
OSCRYPTO_RT int rtxCryptAES128CMAC(OSCTXT* pctxt,
   OSCryptAESKeySchedule keySchedule, OSCrypt128Key subkeys[2],
   OSOCTET message[], size_t msgBytes, OSOCTET mac[16] );


/**
 * Precompute the CMAC subkeys used for calculating a CMAC, using AES-128 as the
 * block cipher.  The subkeys are fixed for a given input key, so this method
 * may be invoked once for a given key and the results cached, or else invoked
 * each time the subkeys are needed.
 *
 * @param pctxt context to use for any memory allocations
 * @param keySchedule The key schedule obtained using rtxCryptAES128KeyExpansion
 * @param subkeys Receives the subkeys. Keep this as secret as key.
 * @return completion status: 0 for success; < 0 is an error code.
 */
OSCRYPTO_RT int rtxCryptAES128CMACPrecompute(OSCTXT* pctxt,
   OSCryptAESKeySchedule keySchedule,
   OSCrypt128Key subkeys[2]);


/**
 * This implements the counter increment function described for the
 * Counter Mode of operation in NIST PUB 800-38A, December 2001.
 *
 * The increment function adds one to a bit field. This bit field consists of
 * either the most significant or the least significant bits (depending on lsb)
 * of the bytes consisting of bytes[0]..bytes[numBytes-1].
 * For example, if numBytes = 2, bitsWide = 3, lsb = 1:
 *    { 1000 0000 0000 0111 } becomes { 1000 0000 0000 0000 }
 *                      ^-^----- the bit field is these three bits
 *
 * @param bytes Pointer to the bytes on which to operate.
 * @param numBytes Number of bytes pointed to by bytes.
 * @param bitsWide The number of bits in which to do the increment.
 *    bitsWide > 0.
 * @param lsb If true, the bit field is found in the least significant bits;
 *    otherwise, in the most significant.
 */
OSCRYPTO_RT void rtxCryptCTRIncr(OSOCTET* bytes, OSUINT32 numBytes,
                             OSUINT32 bitsWide, OSBOOL lsb);


/**
 * Encipher or Decipher the given input message by applying CTR block cipher
 * mode using AES-128 as the block cipher and using the given CTR increment
 * function to derive counter blocks.
 *
 * @param pctxt context to use for any memory allocation
 * @param key The key to use with the block cipher
 * @param counter The first counter block
 * @param pfIncrement The CTR increment function. This is used to derive each
 *    subsequent counter block from the previous one. It should return the
 *    error status.
 * @param message The message to by enciphered/deciphered. It will be modified
 *    by application of the decipher/encipher operation according to CTR mode.
 * @param msgBytes The number of bytes in message.
 * @return completion status: 0 for success; < 0 is an error code.
 */
/*
OSCRYPTO_RT int rtxCryptAES128CTR(OSCTXT* pctxt, OSOCTET key[16],
         OSOCTET counter[16],
         CTR_INCR pfIncrement,
         OSOCTET message[],
         size_t msgBytes
        );
*/

/**
 * Considering the input arguments as bit strings, XOR
 * op1 with op2. Store the results in result (possibly, the same as op1 or op2).
 *
 * @param op1 First XOR operand
 * @param op2 Second XOR operand
 * @param result Where to store the result. Optionally, this may be op1 or op2.
 *    It should otherwise not overlap with op1 and op2.
 * @octets The number of octets to operate on. Each of op1, op2, and result
 *    must be at least this large.
 */
OSCRYPTO_RT void rtxCryptXOR (OSOCTET op1[], OSOCTET op2[], OSOCTET result[],
                         size_t octets);

#endif
