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
 * @file nas_sec.h
 * 3GPP NAS security functions and definitions.
 */
#ifndef __RT3GPPNASSEC_H__
#define __RT3GPPNASSEC_H__

#include "rtxsrc/rtxContext.h"
#include "osCrypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enumeration of 3GPP System Architecture Evolution (SAE) integrity and
 * encryption algorithm identifiers.
 * Note that the values for each corresponds to the identifier values given in
 * TS 33.401 section 5.
 */
typedef enum {
   OS3GPPSecAlgorithm_NULL_ALG,   //for EIA0 and EEA0
   SNOW3G,     //for 128-EIA1 and 128-EEA1
   OS3GPPSecAlgorithm_AES,        //for 128-EIA2 and 128-EEA2
   ZUC         //for 128-EIA3 and 128-EEA3
} OS3GPPSecAlgorithm;


/**
 * Structure for passing security parameters. An instance of this structure
 * is available in the p3gppSec field of the OSCTXT struct. It can be
 * created using rtx3gppSecInitialize.
 *
 * This structure is used as input to the 3GPP SAE security functions, which
 * are used by NAS and others. The value to assign to the fields depends upon
 * the protocol/layer using the algorithms.
 *
 * Certain fields can be assigned directly while others are assigned via
 * function calls. Fields count, bearerId, and direction may be assigned
 * directly. The remaining fields (keys, algorithm identifiers, and precomputed
 * fields) are assigned using rtx3gppAssignAlgorithmKeys.
 */
typedef struct OS3GPPSecParams {
   OSUINT32 count;
   OSUINT8 bearerId;
   OSUINT8 direction;         //0 = uplink; 1 = downlink

   OSCrypt128Key* pIntegrityKey;
   OSCrypt128Key* pEncryptionKey;
   OS3GPPSecAlgorithm integrityAlgId;    //integrity algorithm identifier
   OS3GPPSecAlgorithm encryptionAlgId;   //encryption algorithm identifier

   /**
    * Precomputed subkeys for AES-128 CMAC mode.
    */
   OSCrypt128Key (*pAES128CMAC_subkeys)[2];

   /**
    * AES Key schedule. Index 0 is for integrity key schedule. Index 1 for
    * cipher key schedule.
    */
   OSCryptAESKeySchedule (*pAESKeySchedules)[2];
} OS3GPPSecParams;


/**
 * Cipher or decipher the given message in place.
 * Prerequisites:
 *    - invoked rtx3gppSecInitialize
 *    - invoked rtx3gppAssignAlgorithmKeys to set encryption key and algorithm
 *    - the security parameters are set:
 *          count, bearer, direction
 *
 * The encipher/decipher operations are identical, as both operations produce
 * a keystream which is XOR'd with the input data.
 *
 * The contents of msg will modified by XOR'ing them with a keystream generated
 * according to the security parameters contained by pctxt. The particular
 * algorithm depends on the encryptionAlgId in the OS3GPPSecParams.
 *
 * @param pctxt. Controls memory allocation and security parameters such as
 *    the encryption key and the chosen encryption algorithm.
 * @param msg Pointer the message contents. This data will be
 *    enciphered/deciphered.
 * @param msgLen Length of message in bytes.
 * @return completion status: 0 for success; < 0 is an error code.
 */
EXTERNRT int rtx3gppCipher(OSCTXT* pctxt, OSOCTET* msg, size_t msgLen);

/**
 * Create a new OS3GPPSecParams in the given context.
 * If you invoked this previously, you should free the memory for the
 * current OS3GPPSecParams object first, using rtx3gppSecFree.
 * @param pctxt Context to hold the OS3GPPSecParams object. This must have been
 *    initialized already.
 * @return completion status: 0 for success; < 0 is an error code.
 */
EXTERNRT int rtx3gppSecInitialize(OSCTXT* pctxt);

/**
 * Free the OS3GPPSecParams created using rtx3gppSecInitialize.
 * This will also free memory associated with fields that were precomputed using
 * rtx3gppAssignAlgorithmKeys.
 *
 */
EXTERNRT void rtx3gppSecFree(OSCTXT* pctxt);

/**
 * Assign the given integrity and cipher keys and algorithm identifiers into
 * the OS3GPPSecParams in the given context.
 *
 * This method allows for updating precomputed fields. Invoking this method
 * with OS3GPPSecAlgorithm_NULL_ALG for the algorithm identifier will clear precomputed fields.
 *
 * This method does not copy the key data; the key data must not be deleted
 * until the keys are no longer needed.
 *
 *
 * Prerequisite: invoked rtx3gppSecInitialize
 *
 * @param pIntegrityKey The integrity key. Can be null if integrityAlgId is
 *    OS3GPPSecAlgorithm_NULL_ALG.
 * @param pEncryptionKey The encryption key. Can be null if encryptionAlgId is
 *    OS3GPPSecAlgorithm_NULL_ALG.
 * @param integrityAlgId Identifier for the integrity algorithm.
 * @param encryptionAlgId Identifier for the encryption algorithm.
 * @return completion status: 0 for success; < 0 is an error code.
 */
EXTERNRT int rtx3gppAssignAlgorithmKeys(OSCTXT* pctxt,
                                       OSCrypt128Key* pIntegrityKey,
                                       OSCrypt128Key* pEncryptionKey,
                                       OS3GPPSecAlgorithm integrityAlgId,
                                       OS3GPPSecAlgorithm encryptionAlgId);

/**
 * Apply 3GPP Integrity Algorithm to compute a MAC.
 * The assumption in the design of this function is that you will have already
 * encoded the message to be protected, excepting the MAC which you must
 * now calculate.
 *
 * Prerequisites:
 *    - invoked rtx3gppSecInitialize
 *    - invoked rtx3gppAssignAlgorithmKeys to set integrity key and algorithm
 *    - the security parameters are set:
 *          count, bearer, direction
 *
 * @param pctxt. Controls memory allocation and contains 3GPP security
 *    parameters controlling the computation.
 * @param protectedData Pointer to the protected octets. For example, in the
 *    case of an NAS security protected message, this should point to the
 *    sequence number, which is followed by the (possibly encrypted) NAS message
 *    that is receiving security protection. In the case of a SERVICE REQUEST
 *    message, this should point to the start of that message (the protocol
 *    discriminator and security header type).
 *    NOTE: some algorithms, such as 128-EIA2, combine the protected data with
 *    other data (e.g. count, bearerId) prior to applying the underlying
 *    cryptographic function. This function handles this and so protectedData
 *    should NOT include such other data.
 * @param length The number of bytes protectedData points to.
 * @param mac Receives the computed MAC. For a SERVICE REQUEST message, the
 *    short MAC is the least significant 2 bytes of the mac.
 * @return completion status: 0 for success; < 0 is an error code.
 */
EXTERNRT int rtx3gppComputeMAC(OSCTXT* pctxt, OSOCTET protectedData[],
                              size_t length,  OSUINT32* mac);


#endif
