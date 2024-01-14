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

/*
A stubbed version of this code can be produced by defining NAS_SEC_STUB.
This is done by rt3gppNasSecStub.c
*/

#ifndef NAS_SEC_STUB
//We want to include all of the crypto code so that the rt3gppNasSec object
//module contains that code, and we want to make those functions static since
//right now nothing else uses them.
#define STATIC_CRYPTO
#endif

#include "rt3gppNasSec.h"
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

#ifndef NAS_SEC_STUB
#include "osCrypto.c"
#endif

//see description in header
EXTRTMETHOD int rtx3gppSecInitialize(OSCTXT* pctxt)
{
   OS3GPPSecParams* pParams = rtxMemAllocType(pctxt, OS3GPPSecParams );

   if ( pParams == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);

   pctxt->p3gppSec = pParams;
   pParams->bearerId = 0;
   pParams->count = 0;
   pParams->pIntegrityKey = 0;
   pParams->pEncryptionKey = 0;
   pParams->direction = 0;
   pParams->encryptionAlgId = OS3GPPSecAlgorithm_NULL_ALG;
   pParams->integrityAlgId = OS3GPPSecAlgorithm_NULL_ALG;
   pParams->pAES128CMAC_subkeys = 0;
   pParams->pAESKeySchedules = 0;

   return 0;
}


//see description in header
EXTRTMETHOD void rtx3gppSecFree(OSCTXT* pctxt)
{
   //Clear precomputed fields
   rtx3gppAssignAlgorithmKeys(pctxt, NULL, NULL, OS3GPPSecAlgorithm_NULL_ALG, OS3GPPSecAlgorithm_NULL_ALG);

   rtxMemFreePtr(pctxt, pctxt->p3gppSec);
   pctxt->p3gppSec = NULL;
}


//see description in header
EXTRTMETHOD int rtx3gppAssignAlgorithmKeys(OSCTXT* pctxt,
                                       OSCrypt128Key* pIntegrityKey,
                                       OSCrypt128Key* pEncryptionKey,
                                       OS3GPPSecAlgorithm integrityAlgId,
                                       OS3GPPSecAlgorithm encryptionAlgId)
{
   OS3GPPSecParams* pParams = pctxt->p3gppSec;

   if ( pParams == 0 ) {
      return LOG_RTERRNEW(pctxt, RTERR_NOSECPARAMS);
   }

   if ( integrityAlgId != OS3GPPSecAlgorithm_AES ) {
      //No longer require CMAC subkeys if we have them
      if ( pParams->pAES128CMAC_subkeys != 0 ) {
         memset(pParams->pAES128CMAC_subkeys, 0,
            sizeof(OSCrypt128Key) * 2);

         rtxMemFreePtr(pctxt, pParams->pAES128CMAC_subkeys);
         pParams->pAES128CMAC_subkeys = 0;
      }

      if ( encryptionAlgId != OS3GPPSecAlgorithm_AES ) {
         //We also do not need the key schedules
         if ( pParams->pAESKeySchedules != 0 ) {
            memset(pParams->pAESKeySchedules, 0,
               sizeof(OSCryptAESKeySchedule) * 2);

            rtxMemFreePtr(pctxt, pParams->pAESKeySchedules);
            pParams->pAESKeySchedules = 0;
         }
      }
   }

   switch( integrityAlgId ) {
      case OS3GPPSecAlgorithm_NULL_ALG: break;
#ifndef NAS_SEC_STUB
      case OS3GPPSecAlgorithm_AES: {
         int ret = 0;
         //Allocate and precompute the key schedule
         if ( pParams->pAESKeySchedules == 0 ) {
            pParams->pAESKeySchedules =
               rtxMemAlloc(pctxt, sizeof(OSCryptAESKeySchedule)*2);

            if ( pParams->pAESKeySchedules == 0 )
               return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
         }

         rtxCryptAES128KeyExpansion((*pParams->pAESKeySchedules)[0],
            *pIntegrityKey);

         //allocate and precompute the CMAC subkeys
         if ( pParams->pAES128CMAC_subkeys == 0 ) {
            pParams->pAES128CMAC_subkeys =
               rtxMemAlloc(pctxt, sizeof(OSCrypt128Key)*2);

            if ( pParams->pAES128CMAC_subkeys == 0 )
               return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
         }

         ret = rtxCryptAES128CMACPrecompute(pctxt,
                     (*pParams->pAESKeySchedules)[0],
                     *pParams->pAES128CMAC_subkeys);

         if ( ret < 0 ) return LOG_RTERR(pctxt, ret);
         break;
      }
#endif
      default: {
         rtxErrAddStrParm(pctxt, "integrity algorithm");
         rtxErrAddIntParm(pctxt, integrityAlgId);
         return LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
      }
   }


   switch( encryptionAlgId ) {
      case OS3GPPSecAlgorithm_NULL_ALG: break;
#ifndef NAS_SEC_STUB
      case OS3GPPSecAlgorithm_AES: {
         //Allocate and precompute the key schedule
         if ( pParams->pAESKeySchedules == 0 ) {
            pParams->pAESKeySchedules =
               rtxMemAlloc(pctxt, sizeof(OSCryptAESKeySchedule)*2);

            if ( pParams->pAESKeySchedules == 0 )
               return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
         }

         rtxCryptAES128KeyExpansion((*pParams->pAESKeySchedules)[1],
            *pEncryptionKey);

         break;
      }
#endif
      default: {
         rtxErrAddStrParm(pctxt, "cipher algorithm");
         rtxErrAddIntParm(pctxt, encryptionAlgId);
         return LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
      }
   }

   pParams->pEncryptionKey = pEncryptionKey;
   pParams->pIntegrityKey = pIntegrityKey;
   pParams->encryptionAlgId = encryptionAlgId;
   pParams->integrityAlgId = integrityAlgId;
   return 0;
};


//see description in header
EXTRTMETHOD int rtx3gppCipher(OSCTXT* pctxt, OSOCTET* msg, size_t msgLen)
{
   OS3GPPSecParams* pParams = pctxt->p3gppSec;

   if ( pParams == 0 ) {
      return LOG_RTERRNEW(pctxt, RTERR_NOSECPARAMS);
   }

   if ( msgLen == 0 ) return 0;  //just in case

   switch (pParams->encryptionAlgId) {
      case OS3GPPSecAlgorithm_NULL_ALG: {
         return 0;
      }
#ifndef NAS_SEC_STUB
      case OS3GPPSecAlgorithm_AES: {
         OSOCTET counter[16];
         OSOCTET op[16];
         int ret = 0;

         //counter is initialized to:
         // NAS count (32 bits), bearer (5 bits), direction (1 bit), 0 bits
         memset(counter, 0, 16);
         WORD_TO_BYTES(pParams->count, counter);
         counter[4] = pParams->bearerId << 3 | pParams->direction << 2;

         //process msg, one 16-byte block at a time, until we run out of data
         for(;;) {
            //Apply the cipher to the counter block
            ret = rtxCryptEncipherAES128(pctxt,
               (*pParams->pAESKeySchedules)[1], counter, op, 16);

            if ( ret < 0 ) return LOG_RTERR(pctxt, ret);

            //XOR the message with the ciphered counter block to yield ciphered
            //(or deciphered) message
            rtxCryptXOR(msg, op, msg, msgLen > 16 ? 16 : msgLen);

            if ( msgLen > 16 ) {
               //There are more bytes to cipher
               msgLen -= 16;
               msg += 16;

               //obtain the next counter block
               rtxCryptCTRIncr(counter, 16, 64, 1 /*lsb*/);
            }
            else break; //this was the last block
         }
         break;
      }
#endif
      default: {
         rtxErrAddStrParm(pctxt, "encryption algorithm");
         rtxErrAddIntParm(pctxt, pctxt->p3gppSec->encryptionAlgId);
         return LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
      }
   }
   return 0;
}




//see description in header
EXTRTMETHOD int rtx3gppComputeMAC(OSCTXT* pctxt, OSOCTET protectedData[],
                              size_t length,  OSUINT32* mac)
{
   size_t fullMsgLen;
   size_t offset;
   int ret = 0;
   OS3GPPSecParams* pParams = pctxt->p3gppSec;
   OSOCTET* msg = protectedData;

   if ( pParams == 0 ) {
      return LOG_RTERRNEW(pctxt, RTERR_NOSECPARAMS);
   }

   //Determine whether we need to combine protectedData into some larger
   //message on which we will calculate the MAC
   switch (pctxt->p3gppSec->integrityAlgId) {
      case OS3GPPSecAlgorithm_NULL_ALG: {
         fullMsgLen = length;
         offset = 0;
         break;
      }
#ifndef NAS_SEC_STUB
      case OS3GPPSecAlgorithm_AES: {
         fullMsgLen = length + 8;   //128-EIA2 adds leading 8 bytes
         offset = 8;
         break;
      }
#endif
      default: {
         fullMsgLen = length;
         offset = 0;
      }
   }

   if ( fullMsgLen > length ) {
      //we need to allocate a larger buffer and copy the message
      msg = rtxMemAllocArray(pctxt, fullMsgLen, OSOCTET);
      if ( msg == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
      memcpy(msg + offset, protectedData, length);
   }

   switch (pParams->integrityAlgId) {
      case OS3GPPSecAlgorithm_NULL_ALG: {
         *mac = 0;
         break;
      }
#ifndef NAS_SEC_STUB
      case OS3GPPSecAlgorithm_AES: {
         OSOCTET aesMAC[16];

         //Assign the leading 8 bytes of msg as per 128-EIA2.
         //first 32 bits are count, next 5 are bearerId, then direction,
         //then zeros.
         WORD_TO_BYTES(pctxt->p3gppSec->count, msg);
         msg[4] = pParams->bearerId << 3 | pParams->direction << 2;
         memset(msg+5, 0, 3);
         ret = rtxCryptAES128CMAC(pctxt, (*pParams->pAESKeySchedules)[0],
            *pParams->pAES128CMAC_subkeys, msg, fullMsgLen, aesMAC);
         if ( ret == 0 )
            *mac = WORD(aesMAC); //the most significant 4 bytes form the MAC.
         else
            ret = LOG_RTERR(pctxt, ret);

         break;
      }
#endif
      default: {
         rtxErrAddStrParm(pctxt, "integrity algorithm");
         rtxErrAddIntParm(pctxt, pctxt->p3gppSec->integrityAlgId);
         ret = LOG_RTERRNEW(pctxt, RTERR_NOTSUPP);
      }
   }

   if ( fullMsgLen > length ) {
      rtxMemFreePtr(pctxt, msg);
   }

   return ret;
}
