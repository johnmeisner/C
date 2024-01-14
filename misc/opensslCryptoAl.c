/******************************************************************************
 * Copyright (C) NXP Semiconductors, 2016
 * This software is property of NXP Semiconductors. Unauthorized
 * duplication and disclosure to third parties is prohibited.
 *****************************************************************************/
/*******************************************************************************
 *
 * \file cryptoAl.c
 *
 * \author RajeshKumar R
 *
 *
 * \brief   cryptoAl.h implementation for openssl
 *          provides platform independent abstract interface for openssl
 *          crypto functions
 *
 *******************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <errno.h>
#include "v2xCryptoPal.h"
#include "trustAl.h"
#include "cryptoAl.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
#define HOST_ENCRYPT (AES_ENCRYPT) //!< Request encrypt operation
#define HOST_DECRYPT (AES_DECRYPT) //!< Request decrypt operation

#define KEY_SET0_FILE "keyset0.txt"
#define KEY_SET1_FILE "keyset1.txt"
#define MIN_LINE_DATA 6
#define INSTALL_TOKEN_P1 0x0Cu
#define INSTALL_TOKEN_P2 0x00u
#define DELETE_TOKEN_P1 0x00u
#define DELETE_TOKEN_P2 0x80u
#define KEY_INJECTION_P1 0x00u
#define KEY_INJECTION_P2 0x00u


/******************************************************************************
 * INTERNAL TYPE DEFINITIONS
 *****************************************************************************/
typedef CMAC_CTX axHcCmacCtx_t;

typedef void *cryptoAlCmacInst_t;

/**
 * Contains information required to resume an SCP03 channel with the Security Module.
 * The content of this data structure is only to be interpreted by the Host Library.
 */
typedef struct
{
    uint8_t sEnc[SCP_KEY_SIZE];  //!< SCP03 session channel encryption key
    uint8_t sMac[SCP_KEY_SIZE];  //!< SCP03 session command authentication key
    uint8_t sRMac[SCP_KEY_SIZE]; //!< SCP03 session response authentication key
    uint8_t mcv[SCP_MCV_LEN];    //!< SCP03 MAC chaining value
    uint8_t cCounter[16];        //!< SCP03 command counter
} Scp03SessionState_t;

/**
 * Stores the security sensitive static SCP03 keyset.
 */
typedef struct
{
    Scp03SessionState_t session; //!< SCP03 session state
} ScpState_t;

/******************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/******************************************************************************
 * INTERNAL VARIABLES
 *****************************************************************************/
static ScpState_t scpState[3];
static scpKeySet_t gChAuthStatekeySetType = e_scpKeySet_NORMAL_OP;

static uint8_t aStoreBuff[V2XSE_TRUST_AL_CHALLENGE_LEN];
static randSMCmd_t curState = e_RandSMCmdReset;
/******************************************************************************
 * INTERNAL FUNCTION DECLARATIONS
 *****************************************************************************/
static int32_t v2xCryptoAl_initMastSesData(scpKeySet_t keySetType,
        uint8_t *pscp03Enc,
        uint8_t *pscp03Mac,
        uint8_t *pscp03Dek);

static int32_t v2xCryptoAl_cmacInit(cryptoAlCmacInst_t *ctx, const uint8_t *pKey, size_t keySizeInBytes);

static int32_t v2xCryptoAl_cmacGet(const uint8_t *pKey,
                                   uint8_t keySizeInBytes,
                                   const uint8_t *pMsg,
                                   int32_t msgLen,
                                   uint8_t *pMac);

static int32_t v2xCryptoAl_cmacUpdate(cryptoAlCmacInst_t ctx, const uint8_t *pMsg, size_t msgLen);

static int32_t v2xCryptoAl_cmacFinish(cryptoAlCmacInst_t ctx, uint8_t *pMac);

static SM_Error_t v2xCryptoAl_getCmdICV(channel_t channelNum, uint8_t *pIcv);

static SM_Error_t v2xCryptoAl_getRspICV(channel_t channelNum, uint8_t *pIcv);

static SM_Error_t v2xCryptoAl_procAesCbc(const uint8_t *pKey,
        int32_t keyLen,
        const uint8_t *pIv,
        int8_t dir,
        const uint8_t *pIn,
        int32_t inLen,
        uint8_t *pOut);

static SM_Error_t v2xCryptoAl_rmvPadding(uint8_t *pRxBuff,
        int32_t *pRspLen,
        uint8_t *pPlainResponse,
        int32_t plainResponseLen,
        uint8_t *pSw);

#if TST_SECURITY_APP
/*CAUTION : This macro(TST_SECURITY_APP) is to be enabled only for local testing. It is not meant for production builds
Please do not set it to 1 for customer builds*/

#define SIZE_COMMAND 261
#define SIZE_SESSION_KEYS 16

#define GLOBAL_FILE "global.txt"

static int32_t readLine(int lineNum, char * lineData, char * fileName);
static int32_t writeToFile(char * lineData, char * fileName);
static int32_t stringToInteger(char *str, int32_t *i);
static int32_t stringToHex(char *lineData, uint8_t * lineDataHex);
static int32_t hexToString(uint8_t * lineDataHex,char *lineData,int32_t size);
static int32_t readGlobal();
static int32_t writeGlobal();




#endif


/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_initMastSesData
 *
 * DESCRIPTION  :   Load the Master Key from Custom Key File.
 *
 * NOTES:       :   The implementation of this function assumes the keys are stored in a file.
 *                  The main functionality is to return the three master keys to the caller of this function in plain.
 *                  If the pointer provided as input is NULL then only the integrity check is performed and keys
 *                  are not returned. The current implementation, is not a secured way of handling the keys for
 *                  production release. Customer can implement it in more secured way if required.
 *-----------------------------------------------------------------------------
 */
static int32_t v2xCryptoAl_initMastSesData(scpKeySet_t keySetType,
        uint8_t *pscp03Enc,
        uint8_t *pscp03Mac,
        uint8_t *pscp03Dek )
{
    int32_t ret;
    int32_t rc;
    FILE *hFileSesData;
    char *fopRet;
    int32_t buffMax = V2XSE_TRUST_AL_SCP_AES_KEY_LEN * 3;
    int32_t lineSize;
    int32_t lineRead = 0x00;
    uint8_t *pNumBuff;
    uint32_t numval;
    int32_t parRetVal;
    bool bBreakLoop = false;
    char asesDataBuff[V2XSE_TRUST_AL_SCP_AES_KEY_LEN * 3];

    /* While replacing keyset1 , corresponding file used for authentication is v2xscppalutil-ks1.bin
       While replacing keyset0 , corresponding file used for authentication is v2xscppalutil.bin

       While adding keyset1, corresponding file used for authentication is v2xscppalutil.bin

       If the file v2xscppalutil-ks1.bin is not present while replacing keyset1, the function will report error
       Once the keyset1 is added successfully, v2xscppalutil-ks1.bin should be placed with same keys so that the functionality
       related to keyset1 works correctly without error
       */

#define V2XSCPPALUUTIL_KSL_PATH "/usr/bin/v2xscppalutil-ks1.bin"
#define V2XSCPPALUUTIL_PATH     "/usr/bin/v2xscppalutil.bin"
//TODO: path should be passed in instead of assuming right there.
    if( keySetType == e_scpKeySet_KEYINJECTION_OP )
    {
        hFileSesData = fopen(V2XSCPPALUUTIL_KSL_PATH, "r");
    }
    else
    {
        hFileSesData = fopen(V2XSCPPALUUTIL_PATH, "r");
    }

    if (hFileSesData == NULL)
    {
        if( keySetType == e_scpKeySet_KEYINJECTION_OP )
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "ERROR: File v2xscppalutil-ks1.bin Not Found/Opened\n");
        }
        else
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "ERROR: File v2xscppalutil.bin Not Found/Opened\n");
        }
        ret = HOST_CRYPTO_UNDEFINED_ERROR;
    }
    else
    {
        ret = HOST_CRYPTO_OK;
    }

    if (ret == HOST_CRYPTO_OK)
    {
        while (bBreakLoop == false)
        {
            // Read Single APDU Command line ....
            fopRet = fgets(&asesDataBuff[0x00], buffMax, hFileSesData);

            if (fopRet == NULL)
            {
                //EOF Reached or Error ...
                bBreakLoop = true;
                continue;
            }

            //Skipping the comment Line in the File.
            if (asesDataBuff[0x00] == '#')
            {
                continue;
            }

            switch (lineRead)
            {
            case 0x00:
            {
                pNumBuff = (pscp03Enc != NULL) ? pscp03Enc : NULL;
            }
            break;

            case 0x01:
            {
                pNumBuff = (pscp03Mac != NULL) ? pscp03Mac : NULL;
            }
            break;

            case 0x02:
            {
                pNumBuff = (pscp03Dek != NULL) ? pscp03Dek : NULL;
            }
            break;

            default:
            {
                v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err Invalid Line Offset (%d) \n!!!", lineRead);
                pNumBuff = NULL;
                bBreakLoop = true;
                ret = HOST_CRYPTO_UNDEFINED_ERROR;
            }
            break;
            }

            // No valid destinaton Buff, Skipping the Line;
            if (pNumBuff == NULL)
            {
                lineRead++;
                continue;
            }

            lineSize = 0x00;

            // Convert line from ASCII to Binary ....
            do
            {
                parRetVal = sscanf(&asesDataBuff[lineSize * 0x02], "%02X", &numval);

                if (parRetVal != EOF)
                {
                    //Copy upto 16 bytes, for remaining bytes do not copy but
                    //increment the lineSize to validate correctness of key length later on
                    if(lineSize < V2XSE_TRUST_AL_SCP_AES_KEY_LEN)
                    {
                        pNumBuff[lineSize] = (uint8_t)numval;
                    }
                    lineSize++;
                }

            }
            while ( parRetVal != EOF);  // Read till end of line and later check if the size matches or not

            if (lineSize == V2XSE_TRUST_AL_SCP_AES_KEY_LEN)
            {
                lineRead++;
            }
            else
            {
                v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err Invalid Line Size(%d) \n", lineSize);
                ret = HOST_CRYPTO_UNDEFINED_ERROR;
                bBreakLoop = true;
            }
        }

        if (lineRead != 0x03)
        {
            ret = HOST_CRYPTO_UNDEFINED_ERROR;
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err lineRead Expect(3) Actu(%d) \n", lineRead);
        }

        rc = fclose(hFileSesData);

        if (rc != 0)
        {
            v2xDbg_msg(DBG_LEVEL_1, DBG_APDU_LEVEL, "Err fclose() failed errno(%d)%s \n",
                       errno,
                       strerror(errno));
            ret = HOST_CRYPTO_UNDEFINED_ERROR;
        }
    }

    return (ret);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_cmacInit
 *
 * DESCRIPTION  :   Create openssl cmac Instance.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static int32_t v2xCryptoAl_cmacInit(cryptoAlCmacInst_t *ctx, const uint8_t *pKey, size_t keySizeInBytes)
{
    int32_t ret;
    axHcCmacCtx_t *cmac_ctx;

    *ctx = NULL;
    cmac_ctx = CMAC_CTX_new();

    if (cmac_ctx == NULL)
    {
        ret = HOST_CRYPTO_ERROR_MEMORY;
    }
    else
    {
        ret = HOST_CRYPTO_OK;
    }

    if (ret == HOST_CRYPTO_OK)
    {
        // CMAC_Init() returns
        //      1 = success
        //      0 = failure
        ret = CMAC_Init(cmac_ctx, pKey, keySizeInBytes, EVP_aes_128_cbc(), NULL);
        *ctx = cmac_ctx;

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err CMAC_Init fails !!!\n");
        }
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_cmacGet
 *
 * DESCRIPTION  :   Create & update openssl cmac Instance.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static int32_t v2xCryptoAl_cmacGet(const uint8_t *pKey,
                                   uint8_t keySizeInBytes,
                                   const uint8_t *pMsg,
                                   int32_t msgLen,
                                   uint8_t *pMac)
{
    int32_t ret;
    size_t size;
    cryptoAlCmacInst_t ctx = NULL;

    ret = v2xCryptoAl_cmacInit(&ctx, pKey, keySizeInBytes);
    if (ret == HOST_CRYPTO_OK)
    {
        ret = CMAC_Update(ctx, pMsg, (size_t)msgLen);
        if (ret == HOST_CRYPTO_OK)
        {
            ret = CMAC_Final(ctx, pMac, &size);
        }
    }

    if (ctx != NULL)
    {
        CMAC_CTX_free(ctx);
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_cmacUpdate
 *
 * DESCRIPTION  :   Wrapper function for openssl CMAC_Update
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static int32_t v2xCryptoAl_cmacUpdate(cryptoAlCmacInst_t ctx, const uint8_t *pMsg, size_t msgLen)
{
    int32_t ret;
    axHcCmacCtx_t *cmac_ctx = (axHcCmacCtx_t *)ctx;

    // CMAC_Update() returns
    //      1 = success
    //      0 = failure
    ret = CMAC_Update(cmac_ctx, pMsg, msgLen);

    if (ret != HOST_CRYPTO_OK)
    {
        v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err CMAC_Update fails !!!\n");
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_cmacFinish
 *
 * DESCRIPTION  :   Wrapper function for openssl CMAC_Final
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static int32_t v2xCryptoAl_cmacFinish(cryptoAlCmacInst_t ctx, uint8_t *pMac)
{
    int32_t ret;
    size_t size;
    axHcCmacCtx_t *cmac_ctx = (axHcCmacCtx_t *)ctx;

    if( pMac != NULL )
    {
        // CMAC_Final() returns
        //      1 = success
        //      0 = failure
        ret = CMAC_Final(cmac_ctx, pMac, &size);

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err CMAC_Final fails !!!\n");
        }
    }
    else
    {
        ret = HOST_CRYPTO_OK;
    }

    if( ( ret == HOST_CRYPTO_OK ) && ( cmac_ctx != NULL ) )
    {
        CMAC_CTX_free(cmac_ctx);
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_procAesCbc
 *
 * DESCRIPTION  :   openssl wrapper function for encrypt & decrypt.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static SM_Error_t v2xCryptoAl_procAesCbc(const uint8_t *pKey,
        int32_t keyLen,
        const uint8_t *pIv,
        int8_t dir,
        const uint8_t *pIn,
        int32_t inLen,
        uint8_t *pOut)
{
    SM_Error_t ret_err = SW_OK;
    int32_t outLen = 0;
    int32_t nRet;
#ifdef V2XSE_CFG_OPENSSL_1_1_0 // openSSL 1.1.0 version
    EVP_CIPHER_CTX *aesCtx;
#else // openSSL 1.0.2 version
    EVP_CIPHER_CTX aesCtx;
#endif

    if ((pIn == NULL) || (pOut == NULL))
    {
        ret_err = ERR_API_ERROR;
    }
#ifdef V2XSE_CFG_OPENSSL_1_1_0
    aesCtx = EVP_CIPHER_CTX_new();
#else
    EVP_CIPHER_CTX_init(&aesCtx);
#endif
    if (ret_err == SW_OK)
    {
        if (keyLen != AES_BLOCK_SIZE)
        {
            v2xDbg_msg(DBG_LEVEL_3,DBG_APDU_LEVEL,"Unsupported key length for HOST_AES_CBC_Process\n");
            ret_err = ERR_API_ERROR;
        }

        if ((inLen % AES_BLOCK_SIZE) != 0)
        {
            v2xDbg_msg(DBG_LEVEL_3,DBG_APDU_LEVEL,"Input data are not block aligned for HOST_AES_CBC_Process\n");
            ret_err = ERR_API_ERROR;
        }
    }

    if (ret_err == SW_OK)
    {
        if (dir == HOST_ENCRYPT)
        {
            // EVP_EncryptInit_ex returns 0 on failure and 1 upon success
#ifdef V2XSE_CFG_OPENSSL_1_1_0
            nRet = EVP_EncryptInit_ex(aesCtx, EVP_aes_128_cbc(), NULL, pKey, pIv);
#else
            nRet = EVP_EncryptInit_ex(&aesCtx, EVP_aes_128_cbc(), NULL, pKey, pIv);
#endif
            if (nRet == 0x00)
            {
                ret_err = HOST_CRYPTO_UNDEFINED_ERROR;
            }

            if (ret_err == SW_OK)
            {
#ifdef V2XSE_CFG_OPENSSL_1_1_0
                nRet = EVP_EncryptUpdate(aesCtx, pOut, &outLen, pIn, inLen);
#else
                nRet = EVP_EncryptUpdate(&aesCtx, pOut, &outLen, pIn, inLen);
#endif
                if (nRet == 0x00)
                {
                    ret_err = HOST_CRYPTO_UNDEFINED_ERROR;
                }
            }
        }
        else if (dir == HOST_DECRYPT)
        {
            // EVP_DecryptInit_ex returns 0 on failure and 1 upon success
#ifdef V2XSE_CFG_OPENSSL_1_1_0
            nRet = EVP_DecryptInit_ex(aesCtx, EVP_aes_128_cbc(), NULL, pKey, pIv);
#else
            nRet = EVP_DecryptInit_ex(&aesCtx, EVP_aes_128_cbc(), NULL, pKey, pIv);
#endif
            if (nRet == 0x00)
            {
                ret_err = HOST_CRYPTO_UNDEFINED_ERROR;
            }

            if (ret_err == SW_OK)
            {
#ifdef V2XSE_CFG_OPENSSL_1_1_0
                nRet = EVP_DecryptUpdate(aesCtx, pOut, &outLen, pIn, inLen);
#else
                nRet = EVP_DecryptUpdate(&aesCtx, pOut, &outLen, pIn, inLen);
#endif
                if (nRet == 0x00)
                {
                    ret_err = HOST_CRYPTO_UNDEFINED_ERROR;
                }
            }
        }
        else
        {
            v2xDbg_msg(DBG_LEVEL_3,DBG_APDU_LEVEL,"Unsupported direction for HOST_AES_CBC_Process\n");
            ret_err = ERR_API_ERROR;
        }
    }

    //EVP_CIPHER_CTX_cleanup() returns 1 for success and 0 for failure.
#ifdef V2XSE_CFG_OPENSSL_1_1_0
    EVP_CIPHER_CTX_free(aesCtx);
#else
    nRet = EVP_CIPHER_CTX_cleanup(&aesCtx);
#endif
    if (nRet == 0x00)
    {
        ret_err = ERR_API_ERROR;
    }

    return (ret_err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getCmdICV
 *
 * DESCRIPTION  :   Get Command ICV
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static SM_Error_t v2xCryptoAl_getCmdICV(channel_t channelNum, uint8_t *pIcv)
{
    SM_Error_t ret_err;
    uint8_t ivZero[SCP_KEY_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "getCommandICV SndCmdCounter", scpState[channelNum].session.cCounter, (uint32_t)SCP_KEY_SIZE);

    ret_err = v2xCryptoAl_procAesCbc(scpState[channelNum].session.sEnc,
                                     SCP_KEY_SIZE,
                                     ivZero,
                                     HOST_ENCRYPT,
                                     scpState[channelNum].session.cCounter,
                                     SCP_KEY_SIZE,
                                     pIcv);

    return ret_err;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getRspICV
 *
 * DESCRIPTION  :   Get responce ICV
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static SM_Error_t v2xCryptoAl_getRspICV(channel_t channelNum, uint8_t *pIcv)
{
    SM_Error_t ret_err;
    uint8_t ivZero[SCP_KEY_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t commandCounter[SCP_KEY_SIZE];

    // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
    // Here return value is ignoured, since not interested in return value of
    // the destination address.
    (void)memcpy(commandCounter, scpState[channelNum].session.cCounter, (size_t)SCP_KEY_SIZE);

    commandCounter[0] = V2XSE_TRUST_AL_SCP_SECLVL_OFF; // Section 6.2.7 of SCP03 spec
    v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "getResponseICV RCmdCounter", commandCounter, 16U);

    ret_err = v2xCryptoAl_procAesCbc(scpState[channelNum].session.sEnc,
                                     SCP_KEY_SIZE,
                                     ivZero,
                                     HOST_ENCRYPT,
                                     commandCounter,
                                     (int32_t)(sizeof(commandCounter)),
                                     pIcv);
    v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "getResponseICV pIcv", pIcv, 16U);

    return (ret_err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_rmvPadding
 *
 * DESCRIPTION  :   Remove padding from R-APDU.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static SM_Error_t v2xCryptoAl_rmvPadding(uint8_t *pRxBuff,
        int32_t *pRspLen,
        uint8_t *pPlainResponse,
        int32_t plainResponseLen,
        uint8_t *pSw)
{
    SM_Error_t err = SW_OK;
    int32_t inx = plainResponseLen;

    while ((inx > 1) && (inx > (int32_t)(plainResponseLen - SCP_KEY_SIZE)))
    {
        if (pPlainResponse[inx - 1] == 0x00U)
        {
            inx--;
        }
        else if (pPlainResponse[inx - 1] == 0x80U)
        {
            // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
            // Here return value is ignoured, since not interested in return value of
            // the destination address.
            // We have found padding delimitor
            (void)memcpy(&pPlainResponse[inx - 1], pSw, (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN);
            (void)memcpy(pRxBuff, pPlainResponse, (size_t)inx + (size_t)(1));
            *pRspLen = inx + 1;
            v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "SCP: plainResponseStripped+SW", pRxBuff, (uint32_t)(*pRspLen));
            inx = 0x00;
            continue;
        }
        else
        {
            // We've found a non-padding character while removing padding
            // Most likely the cipher text was not properly decoded.
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Info %s@%d No Padding data !!!\n", __FUNCTION__, __LINE__);
            inx = 0x00;
            err = SCP_RSP_MAC_FAIL;
            continue;
        }
    }

    return err;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getChAuthStateKeySetType
 *
 * DESCRIPTION  :   Global State Key Set Type used for Channel Auth
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
scpKeySet_t v2xCryptoAl_getChAuthStateKeySetType( void )
{
    return( gChAuthStatekeySetType );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_incICVCmdCntr
 *
 * DESCRIPTION  :   Increment ICV Counter
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void v2xCryptoAl_incICVCmdCntr(channel_t channelNum)
{
    int32_t inx = 15;

    while (inx > 0)
    {
        if (scpState[channelNum].session.cCounter[inx] < 255U)
        {
            scpState[channelNum].session.cCounter[inx] += 1U;
            break;
        }
        else
        {
            scpState[channelNum].session.cCounter[inx] = 0U;
            inx--;
        }
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_deriveDataUsingCmac
 *
 * DESCRIPTION  :   Derive Data using CMAC
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_deriveDataUsingCmac(channel_t channelNum, uint8_t *pDda, int32_t ddaLen, uint8_t *pCryptogram)
{
    SM_Error_t err;
    int32_t ret;

    v2xDbg_msg(DBG_LEVEL_2, DBG_APDU_LEVEL, "Info HOST: Calculate session keys\n");

    ret = v2xCryptoAl_cmacGet(scpState[channelNum].session.sMac,
                              V2XSE_TRUST_AL_SCP_AES_KEY_LEN,
                              pDda,
                              ddaLen,
                              pCryptogram);
    if (ret != HOST_CRYPTO_OK)
    {
        err = ERR_CRYPTO_ENGINE_FAILED;
    }
    else
    {
        err = SW_OK;
    }

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_deriveSessionKey
 *
 * DESCRIPTION  :   derive session key.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_deriveSessionKey(channel_t channelNum, scpKeySet_t keySetType, uint8_t *pDda, int32_t ddaLen,
                                        sessionKeys_t *pDerivedSessionKeys)
{
    SM_Error_t err;

    uint8_t scp03Enc[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];
    uint8_t scp03Mac[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];

    uint8_t sessionEncKey[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];
    uint8_t sessionMacKey[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];
    uint8_t sessionRmacKey[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];
    int32_t ret;

    ret = v2xCryptoAl_initMastSesData(keySetType, scp03Enc, scp03Mac, NULL);
    if (ret != HOST_CRYPTO_OK)
    {
        err = ERR_WRONG_RESPONSE;
    }
    else
    {
        err = SW_OK;
    }

    if (err == SW_OK)
    {
        // Calculate the S-ENC key
        pDda[DD_LABEL_LEN - 1] = DATA_DERIVATION_SENC;
        ret = v2xCryptoAl_cmacGet(scp03Enc, V2XSE_TRUST_AL_SCP_AES_KEY_LEN, pDda, ddaLen, sessionEncKey);
        if (ret != HOST_CRYPTO_OK)
        {
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
        else
        {
            err = SW_OK;
        }
    }


    // Calculate the S-MAC key
    if (err == SW_OK)
    {
        pDda[DD_LABEL_LEN - 1] = DATA_DERIVATION_SMAC;
        ret = v2xCryptoAl_cmacGet(scp03Mac, V2XSE_TRUST_AL_SCP_AES_KEY_LEN, pDda, ddaLen, sessionMacKey);
        if (ret != HOST_CRYPTO_OK)
        {
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
    }


    // Calculate the S-RMAC key
    if (err == SW_OK)
    {
        pDda[DD_LABEL_LEN - 1] = DATA_DERIVATION_SRMAC;
        ret = v2xCryptoAl_cmacGet(scp03Mac, V2XSE_TRUST_AL_SCP_AES_KEY_LEN, pDda, ddaLen, sessionRmacKey);
        if (ret != HOST_CRYPTO_OK)
        {
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
    }


    if (err == SW_OK)
    {
        // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
        // Here return value is ignoured, since not interested in return value of
        // the destination address.
        // Store the Session Keys in the appropriate Channel Session State
        (void)memcpy(scpState[channelNum].session.sEnc, sessionEncKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
        (void)memcpy(scpState[channelNum].session.sMac, sessionMacKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
        (void)memcpy(scpState[channelNum].session.sRMac, sessionRmacKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);

        if ((keySetType == e_scpKeySet_NORMAL_OP) &&
                (pDerivedSessionKeys != NULL))
        {
            (void)memcpy(pDerivedSessionKeys->sessionKeys_ENC, sessionEncKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
            (void)memcpy(pDerivedSessionKeys->sessionKeys_MAC, sessionMacKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
            (void)memcpy(pDerivedSessionKeys->sessionKeys_RMAC, sessionRmacKey, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
        }

        gChAuthStatekeySetType = keySetType;
    }

    return (err);
}

SM_Error_t v2xCryptoAl_updateSessionKey(channel_t channelNum, scpKeySet_t keySetType, sessionKeys_t *pDerivedSessionKeys )
{
    SM_Error_t err;

    if (pDerivedSessionKeys != NULL)
    {
        (void)memcpy(scpState[channelNum].session.sEnc, pDerivedSessionKeys->sessionKeys_ENC, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
        (void)memcpy(scpState[channelNum].session.sMac, pDerivedSessionKeys->sessionKeys_MAC, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
        (void)memcpy(scpState[channelNum].session.sRMac, pDerivedSessionKeys->sessionKeys_RMAC, (size_t)V2XSE_TRUST_AL_SCP_AES_KEY_LEN);
    }

    err = SW_OK;

    gChAuthStatekeySetType = keySetType;

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_calMac
 *
 * DESCRIPTION  :   Calculate MAC.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_calMac(channel_t channelNum, const uint8_t *pMsg, size_t msgLen, uint8_t *pMac)
{
    SM_Error_t err;
    int32_t ret;
    cryptoAlCmacInst_t ctx = NULL;
    uint8_t aMac[SCP_MCV_LEN] = {0};

    ret = v2xCryptoAl_cmacInit(&ctx, scpState[channelNum].session.sMac, SCP_KEY_SIZE);

    if (ret != HOST_CRYPTO_OK)
    {
        v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
        err = ERR_CRYPTO_ENGINE_FAILED;
    }
    else
    {
        err = SW_OK;
    }

    if (err == SW_OK)
    {
        ret = v2xCryptoAl_cmacUpdate(ctx, scpState[channelNum].session.mcv, SCP_KEY_SIZE);

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
    }

    if (err == SW_OK)
    {
        ret = v2xCryptoAl_cmacUpdate(ctx, pMsg, msgLen);

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
    }

    if (err == SW_OK)
    {
        ret = v2xCryptoAl_cmacFinish(ctx, &aMac[0x00]);

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
        else
        {
            ctx = NULL;
        }
    }

    // Store updated mcv!
    if (err == SW_OK)
    {
        v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Info %s@%d !!!\n", __FUNCTION__, __LINE__);
        v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "Info MCV", scpState[channelNum].session.mcv, (uint32_t)SCP_MCV_LEN);
        v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "Info Comp Mac", &aMac[0x00], (uint32_t)SCP_MCV_LEN);

        // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
        // Here return value is ignoured, since not interested in return value of
        // the destination address.
        (void)memcpy(scpState[channelNum].session.mcv, &aMac[0x00], (size_t)SCP_MCV_LEN);

        if( gChAuthStatekeySetType == e_scpKeySet_NORMAL_OP )
        {
            (void)memcpy(pMac, &aMac[0x00], (size_t)V2XSE_TRUST_AL_SCP03_KEY_LEN);
        }
        else
        {
            (void)memcpy(pMac, &aMac[0x00], (size_t)V2XSE_TRUST_AL_SCP_GP_IU_CARD_CRYPTOGRAM_LEN);
        }

    }
    else
    {
        // Free the CMAC Context resource,
        // if not released via v2xCryptoAl_cmacFinish()
        // due to failure in control flow
        if( ctx != NULL )
        {
            // return value of CMAC_CTX_free() is void.
            // its ok to ignore the return value here.
            (void)v2xCryptoAl_cmacFinish( ctx, NULL );
        }
    }

    return err;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_verifyMac
 *
 * DESCRIPTION  :   Verify Mac
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_verifyMac(channel_t channelNum, uint8_t *pRxBuff, int32_t *pRspLen, bool rspDec)
{
    SM_Error_t err;
    int32_t ret;
    uint8_t iv[16];
    uint8_t *pIv = (uint8_t *)iv;
    uint8_t response[SCP_BUFFER_SIZE];
    uint8_t plaintextResponse[SCP_BUFFER_SIZE];
    uint8_t sw[V2XSE_TRUST_AL_SCP_GP_SW_LEN];
    uint8_t pMac[16] = {0};
    cryptoAlCmacInst_t ctx = NULL;
    int32_t rxLen = *pRspLen;

    if (rxLen >= 10)
    {
        // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
        // Here return value is ignoured, since not interested in return value of
        // the destination address.
        (void)memcpy(sw, &(pRxBuff[rxLen - (int32_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN]), (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN);

        ret = v2xCryptoAl_cmacInit(&ctx, scpState[channelNum].session.sRMac, SCP_KEY_SIZE);

        if (ret != HOST_CRYPTO_OK)
        {
            v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
            err = ERR_CRYPTO_ENGINE_FAILED;
        }
        else
        {
            err = SW_OK;
        }

        if (err == SW_OK)
        {
            ret = v2xCryptoAl_cmacUpdate(ctx, scpState[channelNum].session.mcv, SCP_CMAC_SIZE);

            if (ret != HOST_CRYPTO_OK)
            {
                v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
                err = ERR_CRYPTO_ENGINE_FAILED;
            }
        }

        if (err == SW_OK)
        {
            if (rxLen > 10)
            {
                ret = v2xCryptoAl_cmacUpdate(ctx, pRxBuff,
                                             ((size_t)rxLen - (size_t)SCP_COMMAND_MAC_SIZE - (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN));

                if (ret != HOST_CRYPTO_OK)
                {
                    v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
                    err = ERR_CRYPTO_ENGINE_FAILED;
                }
            }
        }

        if (err == SW_OK)
        {
            ret = v2xCryptoAl_cmacUpdate(ctx, sw, V2XSE_TRUST_AL_SCP_GP_SW_LEN);

            if (ret != HOST_CRYPTO_OK)
            {
                v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
                err = ERR_CRYPTO_ENGINE_FAILED;
            }
        }

        if (err == SW_OK)
        {
            ret = v2xCryptoAl_cmacFinish(ctx, pMac);

            if (ret != HOST_CRYPTO_OK)
            {
                v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d !!!\n", __FUNCTION__, __LINE__);
                err = ERR_CRYPTO_ENGINE_FAILED;
            }
            else
            {
                ctx = NULL;
            }
        }

        if (err == SW_OK)
        {
            v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "SCP: Calculated Response Mac", pMac, (uint32_t)SCP_CMAC_SIZE);

            // Do a comparison of the received and the calculated mac
            if (memcmp(pMac, &pRxBuff[rxLen - (int32_t)SCP_COMMAND_MAC_SIZE - (int32_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN], SCP_COMMAND_MAC_SIZE) != 0)
            {
                v2xDbg_msg(DBG_LEVEL_2, DBG_APDU_LEVEL, "Err Response Mac Did Not Verify !!!\n");
                err = SCP_RSP_MAC_FAIL;
            }
        }
        else
        {
            // Free the CMAC Context resource,
            // if not released via v2xCryptoAl_cmacFinish()
            // due to failure in control flow
            if( ctx != NULL )
            {
                // return value of CMAC_CTX_free() is void.
                // its ok to ignore the return value here.
                (void)v2xCryptoAl_cmacFinish( ctx, NULL );
            }
        }
    }
    else
    {
        v2xDbg_msg(DBG_LEVEL_2, DBG_APDU_LEVEL, "Err %s:%d Invalid rxLen(%d)!!!\n", __FUNCTION__, __LINE__, rxLen);
        err = SCP_RSP_MAC_FAIL;
    }

    if (err == SW_OK)
    {
        if (rxLen > 10)
        {
            if (rspDec == true)
            {
                // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
                // Here return value is ignoured, since not interested in return value of
                // the destination address.
                (void)memcpy(response, pRxBuff, (size_t)rxLen - 10U);

                // Decrypt Response Data Field in case Reponse Mac verified OK

                (void)memcpy(sw, &(pRxBuff[rxLen - (int32_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN]), (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN);

                err = v2xCryptoAl_getRspICV(channelNum, pIv);

                if (err == SW_OK)
                {
                    err = v2xCryptoAl_procAesCbc(scpState[channelNum].session.sEnc,
                                                 SCP_KEY_SIZE,
                                                 pIv,
                                                 HOST_DECRYPT,
                                                 response,
                                                 rxLen - 10,
                                                 plaintextResponse);
                }

                if (err == SW_OK)
                {

                    //Remove the padding from the plaintextResponse
                    err = v2xCryptoAl_rmvPadding(pRxBuff, pRspLen, plaintextResponse, (rxLen - 10), sw);
                }


            }
            else
            {
                // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
                // Here return value is ignoured, since not interested in return value of
                // the destination address.
                //Remove the R-Mac from RxBuff ...
                (void)memcpy(&pRxBuff[rxLen - (int32_t)SCP_COMMAND_MAC_SIZE - (int32_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN], sw, (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN);
                *pRspLen -= SCP_COMMAND_MAC_SIZE;
            }
        }
        else if (rxLen == 10)
        {
            // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
            // Here return value is ignoured, since not interested in return value of
            // the destination address.
            // There's no data payload in response
            (void)memcpy(pRxBuff, sw, (size_t)V2XSE_TRUST_AL_SCP_GP_SW_LEN);
            *pRspLen = V2XSE_TRUST_AL_SCP_GP_SW_LEN;
        }
        else
        {
            // We're receiving a response with an unexpected response length
            v2xDbg_msg(DBG_LEVEL_2, DBG_APDU_LEVEL, "Err %s:%d Invalid rxLen(%d)!!!\n", __FUNCTION__, __LINE__, rxLen);
            err = SCP_RSP_MAC_FAIL;
        }
    }

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_encrypt
 *
 * DESCRIPTION  :   Encrypt Payload
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_encrypt(channel_t channelNum, uint8_t *pBuffToEncrypt, int32_t buffLen, uint8_t *popbuffEnc)
{
    SM_Error_t err;
    uint8_t iv[SCP_KEY_SIZE];

    err = v2xCryptoAl_getCmdICV(channelNum, &iv[0x00]);

    if (err == SW_OK)
    {
        err = v2xCryptoAl_procAesCbc(scpState[channelNum].session.sEnc, SCP_KEY_SIZE, &iv[0x00],
                                     HOST_ENCRYPT, pBuffToEncrypt,
                                     buffLen, popbuffEnc);
    }

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_stateMaciRandom
 *
 * DESCRIPTION  :   Load the Master Key from Custom Key File.
 *
 * NOTES:       :   The implementation of this function assumes the keys are stored in a file.
 *                  The main functionality is to return the three master keys to the caller of this function in plain.
 *                  If the pointer provided as input is NULL then only the integrity check is performed and keys
 *                  are not returned. The current implementation, is not a secured way of handling the keys for
 *                  production release.
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_stateMaciRandomCtrl(randSMCmd_t funId, uint8_t *pRandom)
{
    SM_Error_t err = SW_OK;

    switch (funId)
    {
    case e_RandSMCmdInit:
    {
        (void)memcpy(aStoreBuff, pRandom, V2XSE_TRUST_AL_CHALLENGE_LEN);
        curState = e_RandSMCmdInit;
    }
    break;

    case e_RandSMCmdReset:
    {
        curState = e_RandSMCmdReset;
    }
    break;

    case e_RandSMCmdGetStoreVal:
    {
        if (curState == e_RandSMCmdInit)
        {
            (void)memcpy(pRandom, aStoreBuff, V2XSE_TRUST_AL_CHALLENGE_LEN);
        }
        else
        {
            err = ERR_WRONG_RESPONSE;
        }
    }
    break;

    default:
    {
        err = ERR_WRONG_RESPONSE;
    }
    break;
    }

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getRandom
 *
 * DESCRIPTION  :   generate cryptographically strong pseudo-random bytes.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_getRandom(int32_t inLen, uint8_t *pRandom)
{
    int32_t ret;
    SM_Error_t err;

    ret = RAND_bytes(pRandom, inLen);

    if (ret != HOST_CRYPTO_OK)
    {
        v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Err %s@%d openssl ErrCode(%d) \n",
                   __FUNCTION__,
                   __LINE__,
                   ERR_get_error());
        err = ERR_CRYPTO_ENGINE_FAILED;
    }
    else
    {
        err = v2xCryptoAl_stateMaciRandomCtrl(e_RandSMCmdInit, pRandom);
    }

    return (err);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_rstMacChainVal
 *
 * DESCRIPTION  :   Reset MAC Chaining value
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void v2xCryptoAl_rstMacChainVal(channel_t channelNum)
{
    // From Linux/Posix Manpage - "The memset() function returns a pointer to the memory area s"
    // Here return value is ignoured, since not interested in return value of
    // the memory address.
    (void)memset(scpState[channelNum].session.mcv, 0, SCP_MCV_LEN);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_updateMacChainVal
 *
 * DESCRIPTION  :   Update MAC Chaining value
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void v2xCryptoAl_updateMacChainVal(channel_t channelNum, uint8_t * pMcvUpdate )
{
    // From Linux/Posix Manpage - "The memset() function returns a pointer to the memory area s"
    // Here return value is ignoured, since not interested in return value of
    // the memory address.
    (void)memcpy(scpState[channelNum].session.mcv, pMcvUpdate, SCP_MCV_LEN);

    // Explicit reset is required to sync global state between REE & TEE
    v2xCryptoAl_rstICVCmdCntr( channelNum );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_rstICVCmdCntr
 *
 * DESCRIPTION  :   reset ICV Command counter.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void v2xCryptoAl_rstICVCmdCntr(channel_t channelNum)
{
    uint8_t commandCounter[SCP_KEY_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // From Linux/Posix Manpage - "The memcpy() function returns a pointer to dest"
    // Here return value is ignoured, since not interested in return value of
    // the destination address.
    (void)memcpy(scpState[channelNum].session.cCounter, commandCounter, (size_t)SCP_KEY_SIZE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_encryptForPutKey
 *
 * DESCRIPTION  :   .
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */

int32_t v2xCryptoAl_encryptForPutKey(scpKeySet_t keySetType, uint8_t operation, scpKeys_t keyTypeToBeEnc, uint8_t *pOutVal)
{
    int32_t int_err;
    SM_Error_t ret_err = SW_OK;
    uint8_t dataForKeyCheck[] =  {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    uint8_t ivZero[SCP_KEY_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t keyDek[SCP_KEY_SIZE];

    uint8_t plainKey[V2XSE_TRUST_AL_SCP03_KEY_LEN];
    FILE *fptr = NULL;
    bool fileOpen = false;

    char *buff;
    uint32_t lineNum = 0, lineNumToPutKey = 0;
    char lineData[V2XSE_TRUST_AL_CMD_RSP_LEN * 2];
    int32_t lineSize; //commands
    int32_t rv_sscan;
    int32_t val;

    /*
     Depending on the keySetType and the operation, open appropriate files
         1. If keySetType is e_scpKeySet_NORMAL_OP and operation is to replace then use file
              corresponding to KEY_SET0_FILE as input file(containing new keys) and  v2xScpUtils.bin as master key file
        2. If keySetType is e_scpKeySet_KEYINJECTION_OP and operation is to replace then use file
              corresponding to KEY_SET1_FILE as input file(containing new keys) and  v2xscppalutil-ks1.bin as master key file
        3. If keySetType is e_scpKeySet_KEYINJECTION_OP and operation is to add then use file
              corresponding to KEY_SET1_FILE as input file(containing new keys) and  v2xscppalutil.bin as master key file
    */

    if(operation>V2XSE_AES_ENC_KEYSET1_ADDITION)
    {
        ret_err = ERR_API_ERROR;
    }


    if (ret_err == SW_OK)
    {
        if (keySetType == e_scpKeySet_NORMAL_OP)
        {
            fptr = fopen(KEY_SET0_FILE, "r");
        }
        else if (keySetType == e_scpKeySet_KEYINJECTION_OP)
        {
            fptr = fopen(KEY_SET1_FILE, "r");
            if(operation == V2XSE_KCV_CALC_KEYSET1_ADDITION || operation == V2XSE_AES_ENC_KEYSET1_ADDITION)
            {
                //If keyset 1 addition is requested use v2xScpUtils.bin for encrypting keys in KEY_SET1_FILE
                keySetType = e_scpKeySet_NORMAL_OP;

            }
        }
        else
        {
            ret_err = ERR_API_ERROR;
        }
    }



    if (fptr == NULL)
    {
        ret_err = ERR_API_ERROR;

        v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Keyset File Open Failed  \n", __FUNCTION__);
    }
    else
    {
        fileOpen = true;
    }

    if (ret_err == SW_OK)
    {
        // based on key type to be encrypted, get the line number of key
        switch (keyTypeToBeEnc)
        {
        case e_scpKeys_ENC:
            lineNumToPutKey = 1;
            break;
        case e_scpKeys_MAC:
            lineNumToPutKey = 2;
            break;
        case e_scpKeys_DEK:
            lineNumToPutKey = 3;
            break;
        default:
            ret_err = ERR_API_ERROR;
            break;
        }
    }

    if (ret_err == SW_OK)
    {
        do
        {
            buff = fgets(lineData, 128, fptr);
            if (buff == NULL)
            {
                ret_err = ERR_API_ERROR;
                break;
            }
            lineNum++;
            // read untill you reach decided key (line number)
        }
        while (lineNum != lineNumToPutKey);
    }

    if (ret_err == SW_OK)
    {
        lineSize = 0;
        do
        {
            rv_sscan = 0x00;
            // pack the key data in hex values of 1 byte each
            rv_sscan = sscanf(&lineData[(lineSize * 2)], "%02X", &val);
            if (rv_sscan != EOF)
            {
                plainKey[lineSize] = (uint8_t)val;
                lineSize++;
            }
        }
        while (rv_sscan != EOF);
    }

    if (fileOpen == true)
    {
        (void)fclose(fptr);
    }

    if (ret_err == SW_OK)
    {
        v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "Input key", plainKey, (uint32_t)SCP_KEY_SIZE);

        int_err = v2xCryptoAl_initMastSesData(keySetType, NULL, NULL, keyDek);

        if (int_err != HOST_CRYPTO_OK)
        {
            ret_err = ERR_WRONG_RESPONSE;
        }
        else
        {
            ret_err = SW_OK;
        }
    }

    if (ret_err == SW_OK)
    {
        if ((operation == V2XSE_KCV_CALC)|| (operation == V2XSE_KCV_CALC_KEYSET1_ADDITION))
        {
            ret_err = v2xCryptoAl_procAesCbc(plainKey, SCP_KEY_SIZE, ivZero,
                                             HOST_ENCRYPT, dataForKeyCheck, SCP_KEY_SIZE, pOutVal);
        }
        if ((operation == V2XSE_AES_ENC)||(operation == V2XSE_AES_ENC_KEYSET1_ADDITION))
        {
            v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "DEK Key", keyDek, (uint32_t)SCP_KEY_SIZE);
            ret_err = v2xCryptoAl_procAesCbc(keyDek, SCP_KEY_SIZE, ivZero,
                                             HOST_ENCRYPT, plainKey, SCP_KEY_SIZE, pOutVal);
        }

        v2xDbg_printByteString(DBG_LEVEL_3, DBG_APDU_LEVEL, "Output key", pOutVal, (uint32_t)SCP_KEY_SIZE);
    }

    v2xDbg_msg(DBG_LEVEL_3, DBG_APDU_LEVEL, "Info: v2xCryptoAl_encryptForPutKey  %04X\n", ret_err);

    return ret_err;
}


/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_checkKey
 *
 * DESCRIPTION  :   Check for Master Key file integrity
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
int32_t v2xCryptoAl_checkKey( scpKeySet_t keySetType )
{

    uint8_t keyBuff[V2XSE_TRUST_AL_SCP_AES_KEY_LEN];
    int32_t ret;
    SM_Error_t ret_err;

    ret = v2xCryptoAl_initMastSesData( keySetType, keyBuff, keyBuff, keyBuff);
    if (ret != HOST_CRYPTO_OK)
    {
        ret_err = ERR_WRONG_RESPONSE;
    }
    else
    {
        ret_err = SW_OK;
    }

    (void)keyBuff;

    return ret_err;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getCommand
 *
 * DESCRIPTION  :   Get the Command from Trust zone database
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
SM_Error_t v2xCryptoAl_getCommand(channel_t channelNum, cmdInfo_t cmd, uint8_t *pCommandApdu, int32_t *ptxlen)
{
    FILE *fptr;
    char *buff;
    uint32_t lineNum = 0;
    char lineData[V2XSE_TRUST_AL_CMD_RSP_LEN * 2];
    uint8_t lineDataHex[V2XSE_TRUST_AL_CMD_RSP_LEN];
    int32_t lineSize; //commands
    int32_t rv_sscan;
    int32_t val;
    bool fileOpen = false;
    uint8_t curveIdTz, keyTypeTz;
    uint16_t keyIdTz;
    appletSelection_t appletIdTz;
    int32_t commandLen = 0;
    int32_t status = V2XSE_SUCCESS;
    int index = 0;
    uint8_t privateKeyZero[256];
    int retVal;
    /*v2xCryptoAl_getCommand() is to be implemented by customer based on their requirement of trust zone
      For demonstration , it is implemented using a text file
      Input parameter source ID is treated as line number from where the required plain command is fetched.
      CLA,INS, P1, P2, Lc, Le are added to make complete APDU and the function returns APDU
      The function works only when keySetType is set to Key injection */

    /*v2xScp_transposeCmd() is used to encrypt the plain command obtained by v2xCryptoAl_getCommand.
      Security level used for encryption is 5
      It encrypts the command, calculates MAC and appends MAC to the encrypted command and returns encrypted APDU.
      Finally CLA is added with channel number and then command is ready to be sent to SE */

    // return if keyset Type is not set for Key Injection mode
    if( v2xCryptoAl_getChAuthStateKeySetType() == e_scpKeySet_NORMAL_OP )
    {
        status = V2XSE_FAILURE;
    }

    if (status == V2XSE_SUCCESS)
    {
        /*
            The format of the file is described in "SXF1800 CLI utilities user manual" (under v2xse-key-injection chapter)".

            Based on sourceId and command type parameter (passed under structure cmd as input parameter), a particular line is read from keydata.txt
            and command APDU containing the fields "CLA", "INS", "P1", "P2", "Lc" and "Data" "Le" is created
          */
        if ((fptr = fopen("keydata.txt", "r")) == NULL)
        {
            status = V2XSE_FAILURE;
            v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s File Open Failed  \n", __FUNCTION__);
        }
        else
        {
            fileOpen = true;
        }
    }

    if (status == V2XSE_SUCCESS)
    {
        do
        {
            // read a line from keydata.txt
            buff = fgets(lineData, V2XSE_TRUST_AL_CMD_RSP_LEN * 2u, fptr);
            if (buff == NULL)
            {
                status = V2XSE_FAILURE;
                v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Reached End of file but could not find data of interest \n", __FUNCTION__);
                break;
            }
            lineNum++;
            // keep reading until you reach the desired line number for desired command
        }
        while (lineNum != cmd.sourceId);
    }
    if (status == V2XSE_SUCCESS)
    {
        // the required line is read correctly, now pack it in hex format and copy it  in an array
        lineSize = 0;
        do
        {
            rv_sscan = 0x00;
            // each value in the line is packed as a one byte hex value. do this until end of line
            rv_sscan = sscanf(&lineData[(lineSize * 2)], "%02X", &val);
            if (rv_sscan != EOF)
            {
                lineDataHex[lineSize] = (uint8_t)val;
                lineSize++;
            }
        }
        while (rv_sscan != EOF);
        // Each line shall contain atleast 5 bytes as mentioned in the line format.
        // if it does not even have 5 bytes , then data is not sufficient
        if (lineSize < MIN_LINE_DATA)
        {
            status = V2XSE_FAILURE;
            v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Too less data on the line.   \n", __FUNCTION__);
        }
        else
        {
            // first byte on each line is applet ID. compare the applet Id obtained from the data read  with the applet Id that is passed to this function.
            // if the applet ID does not match, report error
            // The applet ID check is required to ensure that the key meant for one applet is injected for that applet only

            // applet Id check will be skipped when both the applets are being deleted and installed

            if (cmd.type > e_cmdType_DELETE_TOKEN)
            {
                appletIdTz = (appletSelection_t)lineDataHex[0];
                if (appletIdTz != cmd.appletId)
                {
                    status = V2XSE_FAILURE;
                    v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Applet ID Mismatch/Incorrect, appletIdTz = %d , cmd.appletId = %d \n", __FUNCTION__, appletIdTz,
                               cmd.appletId);
                }
            }
            // second byte on each line is command type. This should match the command type passed as argument to this function through cmd
            // if there is mismatch in command type, report error
            /* Value of valid command types
                e_cmdType_INSTALL_TOKEN - 0x00,
                e_cmdType_DELETE_TOKEN - 0x01,
                e_cmdType_INJECT_MA_KEY - 0x02,
                e_cmdType_INJECT_BA_KEY - 0x03,
                e_cmdType_INJECT_RT_KEY 0x04
             */
            if (lineDataHex[1] != (uint8_t)cmd.type)
            {
                status = V2XSE_FAILURE;
                v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Command type Mismatch  \n", __FUNCTION__);
            }
        }
    }

    if (fileOpen == true)
    {
        //the required data is read, so close the file now
        (void)fclose(fptr);
    }

    if (status == V2XSE_SUCCESS)
    {
        /*
            1) APDU format for Delete/Install token:
                 CLA(1byte)INS(1byte)P1(1byte)P2(1byte)Lc(1byte)COMMAND(Lc bytes)Le(1byte)
               Lc is length of token command
            2) APDU format for MA key injection
                 CLA(1byte)INS(1byte)P1(1byte)P2(1byte)Lc(1 byte)keytype(1byte)curveId(1byte)Keydata(32/48 bytes)Le(1byte)
            3) APDU format for BA/RT key injection
                 CLA(1byte)INS(1byte)P1(1byte)P2(1byte)Lc(1 byte)keytype(1byte)keyId(2bytes)curveId(1byte)Keydata(32/48 bytes)Le(1byte)

         */
        // if all fine , then start constructing the command
        // first byte is CLA . The command is always encrypted with security level 5, so value of CLA as per spec id 0x84.
        // channel number is to be added to CLA value, but that is to be done post encryption of data just before sending the encrypted command to SE
        // So The CLA value considered here is for default channel 0. If different logical channel is used , then the value of channel number should be added post encryption
        pCommandApdu[index++] = V2XSE_CLA_GLOBAL_PLATFORM + 4u;
        switch (cmd.type)
        {
        case e_cmdType_INSTALL_TOKEN:
            // next byte is INS value for install token command
            pCommandApdu[index++] = e_insInstall;
            // next byte is P1 value for install token command
            pCommandApdu[index++] = INSTALL_TOKEN_P1;
            // next byte is P2 value for install token command
            pCommandApdu[index++] = INSTALL_TOKEN_P2;
            // so we added CLA, INS, P1, P2 as of now
            //next byte is Lc value , length of the install token command followed by the actual command of same length.
            //the data from here on can come from trust zone

            pCommandApdu[index++] = lineDataHex[2]; //  Lc value
            if (lineSize == (int32_t)(lineDataHex[2]) + (int32_t)3 ) // Explicit individual type casting required as per 10.8
            {
                // copy the actual command .
                (void)memcpy(&pCommandApdu[index], &lineDataHex[3], lineDataHex[2]);
                //update the buffer index according to the length of data
                index = (int)lineDataHex[2] + index;
            }
            else
            {
                // if the data read from the text file  is not in sync with expected length, report Error
                status = V2XSE_FAILURE;
                v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Install Token Command Size is incorrect \n", __FUNCTION__);
            }
            break;
        case e_cmdType_DELETE_TOKEN:
            // next byte is INS value for delete token command
            pCommandApdu[index++] = e_insDeleteToken;
            // next byte is P2 value for delete token command
            pCommandApdu[index++] = DELETE_TOKEN_P1;
            // next byte is P2 value for delete token command
            pCommandApdu[index++] = DELETE_TOKEN_P2;
            // so we added CLA, INS, P1, P2 as of now
            //next byte is Lc value, length of the delete token command followed by the actual command of same length.
            //the data from here on can come from trust zone
            pCommandApdu[index++] = lineDataHex[2];// Lc value
            if (lineSize == (int32_t)(lineDataHex[2]) + (int32_t)3U )//Individual typecasting required as per MISRA rule 10.8
            {
                // copy the actual command
                (void)memcpy(&pCommandApdu[index], &lineDataHex[3], lineDataHex[2]);

                //update the buffer index according to the length of data
                index = (int)lineDataHex[2] + index;
            }
            else
            {
                // if the data read from the text file  is not in sync with expected length, report Error
                status = V2XSE_FAILURE;
                v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Delete Token Command Size is incorrect \n", __FUNCTION__);
            }
            break;
        case e_cmdType_INJECT_BA_KEY:
        case e_cmdType_INJECT_MA_KEY:
        case e_cmdType_INJECT_RT_KEY:
            // read the key type , key Id and curve Id from the line data that is read from the text file
            // key type valid values : 00 - MA key, 02 - BA key, 03 - RT key
            keyTypeTz = lineDataHex[2];
            //represents key id or key index or key slot where the key will be injected. Max value allowed is 0x270F(decimal 9999), min value is 0x0000
            keyIdTz = (((uint16_t)lineDataHex[3] << 8) | (uint16_t)lineDataHex[4]);
            // curve ID. Valid values : 00 to 05, Key length will be based on curve ID
            curveIdTz = lineDataHex[5];

            //if the key Id value read from the text file is same as the key Id passed as parameter then proceed ahead.
            // Else return error for key Id mismatch
            if (keyIdTz == cmd.keyId)
            {
                // we have already added CLA as first byte of the APDU .
                // next byte in the APDU is INS value for inject key
                pCommandApdu[index++] = e_insInjectKeys;
                // next 2 bytes are P1 , P2 values which are  always 0 for key injection command
                pCommandApdu[index++] = KEY_INJECTION_P1;
                pCommandApdu[index++] = KEY_INJECTION_P2;
            }
            else
            {
                //ERROR: key ID mismatch
                status = V2XSE_FAILURE;
                v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Key ID Mismatch found while injecting key \n", __FUNCTION__);
            }

            if (status == V2XSE_SUCCESS)
            {
                // find out what should be the length of key based on the curve ID(that is read from text file)
                status = v2xCryptoAl_getKeyLenFromCurveID(curveIdTz, &commandLen);
                if (commandLen == 0x00)
                {
                    // since the curve ID is invalid, length of key is reported as 0, so report Error
                    status = V2XSE_FAILURE;
                }
            }

            if (status == V2XSE_SUCCESS)
            {
                // check if the total data read from text file is in sync with the expected length (based on key length/curveId)
                // 6 is for applet Id(1) + cmd type(1)+ keytype(1)+ keyid(2)+curveID(1)
                if (lineSize == (6 + commandLen))
                {
                    // we have added CLA, INS, P1, P2 as of now and the key length is also correct. so proceed ahead
                    if (keyTypeTz == 0u)
                    {
                        // For MA key
                        // next byte id Lc value. Lc value is the length of key + 1 byte for key type + 1 byte for curve Id
                        // key Id value is ignored for MA key injection, since it is always 0 and does not form part of APDU. so it is not embedded  while constructng APDU
                        pCommandApdu[index++] = (uint8_t)commandLen + 2u; //4
                        // next byte is key type
                        pCommandApdu[index++] = lineDataHex[2];
                        // next byte is curve ID
                        pCommandApdu[index++] = lineDataHex[5];
                    }
                    else
                    {
                        // For Rt and Ba key

                        // next byte id Lc value. Lc value is the length of key + 1 byte for key type + 1 byte for curve Id + 2 bytes of key Id
                        pCommandApdu[index++] = (uint8_t)commandLen + 4u;
                        // next byte is key type
                        pCommandApdu[index++] = lineDataHex[2];
                        // MSB of key ID
                        pCommandApdu[index++] = lineDataHex[3];
                        // LSB of key ID
                        pCommandApdu[index++] = lineDataHex[4];
                        // next byte is curve ID value
                        pCommandApdu[index++] = lineDataHex[5];
                    }
                    // copy the actual key into APDU
                    (void)memcpy(&pCommandApdu[index], &lineDataHex[6], commandLen);
                    (void)memset(privateKeyZero,0,commandLen);
                    retVal = memcmp(privateKeyZero,&pCommandApdu[index],commandLen);
                    if(retVal == 0)
                    {
                        status = V2XSE_FAILURE;
                        v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Private Key with all 0s  \n", __FUNCTION__);
                    }
                    //update the buffer index according to the length of data
                    index = commandLen + index;
                }
                else
                {
                    // if the data present is not in sync with expectation, report error
                    status = V2XSE_FAILURE;
                    v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Inject Key Command Size is incorrect  \n", __FUNCTION__);
                }
            }
            break;
        default:
            status = V2XSE_FAILURE;
            v2xDbg_msg(DBG_LEVEL_1, DBG_COMMUNICATION_LEVEL, "%s Incorrect Command type \n", __FUNCTION__);
            break;
        }
    }
    if (status == V2XSE_SUCCESS)
    {
        // if all fine, then add Le value . it is 0 for both token commands and key injection command
        pCommandApdu[index++] = 0x00U; // adding Le as 0
        // return the length of APDU to the caller function
        *ptxlen = index;
    }

    return status;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_getKeyLenFromCurveID
 *
 * DESCRIPTION  :   calculate key length based on curve id
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
int32_t v2xCryptoAl_getKeyLenFromCurveID(TypeCurveId_t curveID, int32_t *pKeyLen)
{
    int32_t status = V2XSE_FAILURE;
    switch (curveID)
    {
    case V2XSE_CURVE_NISTP256:
    case V2XSE_CURVE_BP256R1:
    case V2XSE_CURVE_BP256T1:
        *pKeyLen = 32;
        status = V2XSE_SUCCESS;
        break;
    case V2XSE_CURVE_NISTP384:
    case V2XSE_CURVE_BP384R1:
    case V2XSE_CURVE_BP384T1:
        *pKeyLen = 48;
        status = V2XSE_SUCCESS;
        break;
    default:
        status = V2XSE_FAILURE;
        break;
    }

    return status;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_open
 *
 * DESCRIPTION  :
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
int32_t v2xCryptoAl_open( void )
{

#if TST_SECURITY_APP
    /*CAUTION : This macro(TST_SECURITY_APP) is to be enabled only for local testing. It is not meant for production builds
    Please do not set it to 1 for customer builds*/

    return readGlobal();
#else    // TBD :: Right now stub

    return (V2XSE_SUCCESS);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xCryptoAl_close
 *
 * DESCRIPTION  :
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
int32_t v2xCryptoAl_close( void )
{
#if TST_SECURITY_APP
    /*CAUTION : This macro(TST_SECURITY_APP) is to be enabled only for local testing. It is not meant for production builds
    Please do not set it to 1 for customer builds*/

    return writeGlobal();
#else
    // TBD :: Right now stub
    return (V2XSE_SUCCESS);
#endif
}

#if TST_SECURITY_APP
/*CAUTION : This macro(TST_SECURITY_APP) is to be enabled only for local testing. It is not meant for production builds
Please do not set it to 1 for customer builds*/

static int32_t readGlobal()
{
    int32_t status = -1;

    char line[SIZE_COMMAND*2];

    /*global.txt file contains the global structure data of session state for all channel */
    // line 1 to 5 contains session data for channel 0
    status = readLine(1,line,GLOBAL_FILE);
    if(status == 0)
    {
        stringToHex(line,scpState[0].session.sEnc);
        status = readLine(2,line,GLOBAL_FILE);
        stringToHex(line,scpState[0].session.sMac);
        status = readLine(3,line,GLOBAL_FILE);
        stringToHex(line,scpState[0].session.sRMac);
        status = readLine(4,line,GLOBAL_FILE);
        stringToHex(line,scpState[0].session.mcv);
        status = readLine(5,line,GLOBAL_FILE);
        stringToHex(line,scpState[0].session.cCounter);

        // line 6 to 10 contains session data for channel 1
        status = readLine(6,line,GLOBAL_FILE);
        stringToHex(line,scpState[1].session.sEnc);
        status = readLine(7,line,GLOBAL_FILE);
        stringToHex(line,scpState[1].session.sMac);
        status = readLine(8,line,GLOBAL_FILE);
        stringToHex(line,scpState[1].session.sRMac);
        status = readLine(9,line,GLOBAL_FILE);
        stringToHex(line,scpState[1].session.mcv);
        status = readLine(10,line,GLOBAL_FILE);
        stringToHex(line,scpState[1].session.cCounter);

        // line 11 to 15 contains session data for channel 2
        status = readLine(11,line,GLOBAL_FILE);
        stringToHex(line,scpState[2].session.sEnc);
        status = readLine(12,line,GLOBAL_FILE);
        stringToHex(line,scpState[2].session.sMac);
        status = readLine(13,line,GLOBAL_FILE);
        stringToHex(line,scpState[2].session.sRMac);
        status = readLine(14,line,GLOBAL_FILE);
        stringToHex(line,scpState[2].session.mcv);
        status = readLine(15,line,GLOBAL_FILE);
        stringToHex(line,scpState[2].session.cCounter);

        //current key set type
        readLine(16,line,GLOBAL_FILE);
        stringToInteger((char *)line,(int32_t *)&gChAuthStatekeySetType);
        // last random number
        readLine(17,line,GLOBAL_FILE);
        stringToHex(line,aStoreBuff);
        readLine(18,line,GLOBAL_FILE);
        // current state machine state
        stringToInteger((char *)line,(int32_t *)&curState);
    }
    return V2XSE_SUCCESS;

}

static int32_t writeGlobal()
{

    FILE *file = NULL;
    char line[SIZE_COMMAND*2];

    file = fopen(GLOBAL_FILE, "w");
    fclose(file);


    /*global.txt file contains the global structure data of session state for all channel */
    // line 1 to 5 contains session data for channel 0
    hexToString(scpState[0].session.sEnc,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[0].session.sMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[0].session.sRMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[0].session.mcv,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[0].session.cCounter,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);

    // line 6 to 10 contains session data for channel 1
    hexToString(scpState[1].session.sEnc,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[1].session.sMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[1].session.sRMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[1].session.mcv,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[1].session.cCounter,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);

    // line 11 to 15 contains session data for channel 2
    hexToString(scpState[2].session.sEnc,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[2].session.sMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[2].session.sRMac,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[2].session.mcv,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);
    hexToString(scpState[2].session.cCounter,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);

    //current key set type
    sprintf(line,"%d",gChAuthStatekeySetType);
    writeToFile(line,GLOBAL_FILE);

    // last random number
    hexToString(aStoreBuff,line,SIZE_SESSION_KEYS);
    writeToFile(line,GLOBAL_FILE);

    // current state machine state
    sprintf(line,"%d",curState);
    writeToFile(line,GLOBAL_FILE);

    return V2XSE_SUCCESS;

}


static int32_t readLine(int lineNum, char * lineData, char * fileName)
{
    FILE* file = NULL;
    int i = 0;
    file = fopen(fileName, "r");

    if (file == NULL)
    {
        return -1;
    }
    while (fgets(lineData, 256, file))
    {
        i++;
        if(i == lineNum )
        {
            break;
        }
    }
    fclose(file);
    return 0;
}

static int32_t writeToFile(char * lineData, char * fileName)
{

    FILE *file = NULL;
    file = fopen(fileName, "a");
    if (file == NULL)
    {
        return -1;
    }

    fprintf(file, "%s\n", lineData);
    fclose(file);
    return 0;
}


static int32_t stringToInteger(char *str, int32_t *i)
{
    char *endPtr;
    errno = 0;
    int32_t result = strtol(str, &endPtr, 10);
    *i = result;

    if (errno != 0)
    {
        // Some conversion error
        return V2XSE_FAILURE;
    }
    if (str[0] == '\0')
    {
        // Empty string
        return V2XSE_FAILURE;
    }

    if (endPtr[0] != '\0')
    {
        // String contains remaining characters that cannot be converted
        return V2XSE_FAILURE;
    }

    return V2XSE_SUCCESS;
}

static int32_t stringToHex(char *lineData, uint8_t * lineDataHex)
{
    int32_t lineSize;
    int32_t rv_sscan;
    int32_t val;
    lineSize = 0;

    do
    {
        rv_sscan = 0x00;
        // each value in the line is packed as a one byte hex value. do this until end of line
        rv_sscan = sscanf(&lineData[(lineSize * 0x02u)], "%02X", &val);
        if (rv_sscan != EOF)
        {
            lineDataHex[lineSize] = (uint8_t)val;
            lineSize++;
        }
    }
    while (rv_sscan != EOF);
    return 0;
}

static int32_t hexToString(uint8_t * lineDataHex,char *lineData,int32_t size)
{
    int i = 0;
    for ( i=0; i < size; i++ )
    {
        sprintf ( &lineData[i*2], "%02x", lineDataHex[i] );
    }
    return 0;
}



#endif
