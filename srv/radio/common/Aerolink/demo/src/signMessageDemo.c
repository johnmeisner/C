/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

/*
 * Demo program using the C API for generating/processing signed messages
 * synchronously.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "viicsec.h"

#define LOG(text, result)                         \
    fprintf(stderr, "%s line %d: %s; result=%s\n", \
            __FILE__, __LINE__, text, ws_errid(result))

#if defined(_WIN32) || defined(_WIN64)
#define strtok_r strtok_s
#endif

static void updateCallbackFunction(
    AerolinkReportIdBitmap    id,
    void const*               reportData,
    void                    * userData)
{
    printf("\nupdateCallbackFunction called\n");

    if (reportData)
    {
        if (ARIB_CERTIFICATE_STATUS == id)
        {
            printf("  report id = ARIB_CERTIFICATE_STATUS\n");
            CertificateStatusReport const* data = (CertificateStatusReport const*)reportData;
            printf("    status:          0x%08x\n", data->status);
            printf("    lcm path length: %u\n", data->lcmDirLength);
            printf("    lcm path:        %s\n", (char const*)&data->lcmDir[0]);
        }
        else
        {
            printf("  Unknown report id = %u\n", id);
        }
    }

    printf("\n");
}

/*
 * Print a label and then the contents of the supplied buffer to stdout.
 */
static void printBytes(char *label, uint8_t buffer[], uint32_t length)
{
    uint32_t i = 0;

    printf("%s (%d bytes): ", label, length);

    for( i = 0; i < length; i++ ) {
        printf("%02x ", buffer[i] & 0xffU);
    }

    printf("\n");
}

static AEROLINK_RESULT
processSignedMessage(
        SecuredMessageParserC   smp,
        uint8_t                 signedMessage[512],
        uint32_t                signedMessageLength,
        PayloadType            *spduType,
        uint8_t const         **receivedMessage,
        uint32_t               *receivedMessageLength,
        PayloadType            *payloadType,
        uint8_t  const        **externalDataHash,
        ExternalDataHashAlg    *externalDataHashAlg)
{
    AEROLINK_RESULT result;

    /*
     * Generation time "from" message payload.
     */
    int32_t genTimeLatitude;
    int32_t genTimeLongitude;
    uint16_t genTimeElevation;

    /*
     * Process secured message.
     */
    printf("\nProcessing \"received\" secured message\n");
    printf("Parsing secured message\n");
    result = smp_extract(smp,
                         signedMessage, signedMessageLength, spduType,
                         receivedMessage, receivedMessageLength, payloadType,
                         externalDataHash, externalDataHashAlg);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to parse the message", result);
        return result;
    }

    /*
     * Set generation location "from" the message payload since it's not
     * in the security headers.
     */
    printf("Setting generation location\n");
    genTimeLatitude = 0x323c4650;
    genTimeLongitude = 0x01020304;
    genTimeElevation = 0x0020;
    result = smp_setGenerationLocation(smp, genTimeLatitude, genTimeLongitude,
            genTimeElevation);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    /*
     * Check relevance.
     */
    printf("Checking message relevance\n");
    result = smp_checkRelevance(smp);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    /*
     * Check consistency.
     */
    printf("Checking message and certificates' consistency\n");
    result = smp_checkConsistency(smp);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    /*
     * Verify signatures.
     */
    printf("Verifying message and certificates' signatures\n");
    result = smp_verifySignatures(smp);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    printf("Message is valid\n");
    return WS_SUCCESS;
}

static void setPermissions(
    SigningPermissions    *p,
    uint32_t               psid,
    uint8_t const * const  ssp,
    uint8_t const * const  sspMask,
    uint32_t               sspLen)
{
    p->psid = psid;
    p->ssp = ssp;
    p->sspMask = sspMask;
    p->isBitmappedSsp = (NULL == sspMask) ? 0 : 1;
    p->sspLen = sspLen;
}

static uint32_t validHexToken(char *tokenString)
{
    return (tokenString == NULL || strlen(tokenString) != 2 ||
            !isxdigit(tokenString[0]) || !isxdigit(tokenString[1])) ?  0 : 1;
}

static AEROLINK_RESULT splitSspTokens(
    char *tokenString,
    uint8_t *tokenArray,
    size_t *numTokens)
{
    AEROLINK_RESULT result;

    if(tokenString == NULL)
    {
        result = WS_ERR_BAD_ARGS;
        LOG("Tried to parse tokens from empty token string", result);
        return result;
    }

    char *end;
    char *savptr;
    char *token = strtok_r(tokenString, "-", &savptr);

    size_t tokensCounted = 0;
    while (token && tokensCounted < *numTokens)
    {
        if(!validHexToken(token))
        {
            result = WS_ERR_BAD_ARGS;
            LOG("Input string contains invalid hex byte", result);
            return result;
        }
        tokenArray[tokensCounted++] = (uint8_t)strtol(token, &end,  16);
        token = strtok_r(NULL, "-", &savptr);
    }

    if(token != NULL)
    {
        result = WS_ERR_BAD_ARGS;
        LOG("Input string more permissions than can fit into input array", result);
        return result;
    }

    *numTokens = tokensCounted;
    return WS_SUCCESS;
}

int main (int argc, char *argv[])
{
    /* General variables */
    int32_t printMessage = 0;
    int32_t i;

    SecurityContextC context;
    SecuredMessageGeneratorC smg = NULL;
    SecuredMessageParserC smp = NULL;

    char *contextName = "denso.wsc";

    /* Time adjustment: number of leap seconds since 1/1/2004 */
    int32_t leapSeconds;

    /* Position variables */
    int32_t latitude;
    int32_t longitude;
    uint16_t elevation;
    uint16_t countryCode;

    uint16_t userProvidedCountryCodeValue = 0;

    /* Signing variables */
    uint32_t  ieeePsidValue = 0x20;
    uint8_t   ieeeSsp [31] = {0};
    uint32_t  ieeeSspLength = 0;
    uint8_t   ieeeSspMask [31] = {0};
    uint32_t  ieeeSspMaskLength = 0;

    uint32_t  psidValue;
    uint8_t   sspValue [31] = {0};
    uint32_t  sspLength = 0;
    uint8_t   sspMaskValue[31] = {0};
    uint32_t  sspMaskLength = 0;

    uint32_t  userProvidedPsidValue = 0;
    uint8_t   userProvidedSspValue [31] = {0};
    uint8_t   userProvidedSspMaskValue [31] = {0};
    size_t  userProvidedSspLength = 0;
    size_t  userProvidedSspMaskLength = 0;

    uint8_t payload[] = "some payload";
    uint32_t payloadLength = sizeof payload;

    uint8_t signedMessage[512];
    uint32_t signedMessageLength = sizeof signedMessage;

    uint8_t externalDataHash[32];
    uint8_t const *receivedExternalDataHash = NULL;
    ExternalDataHashAlg externalDataHashAlg;

    /* Parsing variables */
    PayloadType  spduType;
    PayloadType  payloadType;
    uint8_t const *receivedMessage;
    uint32_t receivedMessageLength = 0;
    AEROLINK_RESULT result;
    uint32_t testUserData;

    /*
     * Command line argument processing.
     */

    for( i = 1; i < argc; ++i )
    {
        if( strcmp("-p", argv[i]) == 0 )
        {
            printMessage = 1;
        }

        if( strcmp("-psid", argv[i]) == 0 )
        {
            char *end;
            userProvidedPsidValue = (uint32_t)strtol(argv[++i], &end,  0);
        }

        if( strcmp("-ssp", argv[i]) == 0 )
        {
            char *line = argv[++i];
            char *savePtr;

            char *bitMapToken = strtok_r(line, ":", &savePtr);

            char *sspMaskTokens = NULL;
            char *sspTokens = NULL;

            // Checking if ssp is bitmapped
            if(strcmp("b", bitMapToken) == 0)
            {
                sspTokens = strtok_r(NULL, ":", &savePtr);
                sspMaskTokens = strtok_r(NULL, ":", &savePtr);

                if(sspTokens == NULL || sspMaskTokens ==  NULL)
                {
                    result = WS_ERR_BAD_ARGS;
                    LOG("Expected format is b:ssp:sspMask", result);
                    return result;
                }
            }

            // Parsing plain ssp
            else
            {
                sspTokens = line;

                // Looking for extraneous separators
                sspMaskTokens = strtok_r(NULL, ":", &savePtr);

                if(sspMaskTokens != NULL)
                {
                    result = WS_ERR_BAD_ARGS;
                    LOG("Expected format is ssp bytes or b:ssp:sspMask", result);
                    return result;
                }
            }

            userProvidedSspLength = sizeof userProvidedSspValue;
            result = splitSspTokens(sspTokens, userProvidedSspValue, &userProvidedSspLength);

            if (result != WS_SUCCESS)
            {
                return result;
            }

            if( sspMaskTokens != NULL)
            {
                userProvidedSspMaskLength = sizeof userProvidedSspMaskValue;
                result = splitSspTokens(sspMaskTokens, userProvidedSspMaskValue, &userProvidedSspMaskLength);

                if (result != WS_SUCCESS)
                {
                    return result;
                }

                if( userProvidedSspMaskLength != userProvidedSspLength )
                {
                    result = WS_ERR_BAD_ARGS;
                    LOG("Parsed bitmap and bitmask are not the same length", result);
                    return result;
                }
            }
        }

        if( strcmp("-countryCode", argv[i]) == 0 )
        {
            char *end;
            userProvidedCountryCodeValue = (uint16_t)strtol(argv[++i], &end,  0);
        }
    }

    /*
     * Get the library version.
     */
    printf("\nLibrary version is '%s'\n", securityServices_getVersion());

    /*
     * Initialize security services.
     */
    printf ("\nInitializing security services\n");
    result = securityServices_initialize();
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_initialize() failed", result);
        return result;
    }

    /*
     * Register to receive error and/or status updates
     */
    printf("Register status callback\n");
    result = securityServices_registerReportCallback((uint32_t)ARIB_CERTIFICATE_STATUS, updateCallbackFunction, &testUserData);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_registerReportCallback() failed", result);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Set the number of leap seconds since 1/1/2004.
     */
    printf ("Setting leap seconds\n");
    leapSeconds = 5;
    result = securityServices_setTimeAdjustment(leapSeconds);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_setTimeAdjustment() failed", result);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Set the current position.
     */
    printf ("Setting current position\n");
    latitude = 0x323c4650;
    longitude = 0x01020304;
    elevation = 0x0020;

    countryCode = (userProvidedCountryCodeValue > 0) ? userProvidedCountryCodeValue : 0x348;

    result = securityServices_setCurrentLocation(latitude, longitude, elevation, countryCode);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_setPosition() failed", result);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Open application's security context.
     */
    printf ("Opening security context\n");
    result = sc_open(contextName, &context);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to open wsc", result);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Get the filename of the security context.
     */
    {
        char const * name = NULL;
        result = sc_getName(context, &name);
        if ((result != WS_SUCCESS) || !name)
        {
            LOG("Failed to get the filename of the security context", result);
            smg_delete(smg);
            smp_delete(smp);
            (void)sc_close(context);
            (void)securityServices_shutdown();
            return result;
        }

        printf("Security context name is '%s'\n", name);
    }

    /*
     * Create Secured Message Generator.
     */
    printf ("Creating a secured message generator\n");
    result = smg_new(context, &smg);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a secured message generator", result);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Create Secured Message Parser.
     */
    printf ("Creating a secured message parser\n");
    result = smp_new(context, &smp);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a secured message parser", result);
        smg_delete(smg);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Set signing permissions.
     */

    psidValue = (userProvidedPsidValue > 0) ? userProvidedPsidValue : ieeePsidValue;
    if (userProvidedSspLength > 0)
    {
        sspLength = userProvidedSspLength;
        memcpy(sspValue, userProvidedSspValue, userProvidedSspLength);
    }

    else
    {
        sspLength = ieeeSspLength;
        memcpy(sspValue, ieeeSsp, ieeeSspLength);
    }

    if (userProvidedSspMaskLength > 0)
    {
        sspMaskLength = userProvidedSspMaskLength;
        memcpy(sspMaskValue, userProvidedSspMaskValue, userProvidedSspMaskLength);
    }

    else
    {
        sspMaskLength = ieeeSspMaskLength;
        memcpy(sspMaskValue, ieeeSspMask, ieeeSspMaskLength);
    }

    printf("\nSigning permissions: psid = 0x%02x, ssp = ", psidValue);
    for (size_t j = 0; j < sspLength; ++j)
    {
        printf("0x%02x ", sspValue[j]);
    }

    if(sspMaskLength > 0)
    {
        printf("sspMask = ");

        for (size_t k = 0; k < sspMaskLength; ++k)
        {
            printf("0x%02x ", sspMaskValue[k]);
        }
    }

    /*
     * Generate signed message.
    */

    printf("\nSigning the message\n");
    SigningPermissions permissions;
    setPermissions(&permissions, psidValue, sspValue, (sspMaskLength > 0) ? sspMaskValue : NULL, sspLength);
    result = smg_sign(smg,
                      permissions,
                      STO_AUTO,
                      0,
                      payload, payloadLength,
                      0,                          // isPayloadSPDU, no
                      externalDataHash,
                      EDHA_NONE,                  // External DataHash Algorithm
                      NULL,                       // Certificate to be included in message, none
                      signedMessage, &signedMessageLength);

    if (result != WS_SUCCESS)
    {
        LOG("Failed to generate the signed message", result);
        smg_delete(smg);
        smp_delete(smp);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }
    else
    {
        printf("Signed message generated correctly\n");
    }

    if (printMessage)
    {
        printBytes("\nSigned message is:\n  ",
                   signedMessage, signedMessageLength);
    }

    /*
     * Process signed message.
     */
    result = processSignedMessage(smp, signedMessage, signedMessageLength,
                                  &spduType, &receivedMessage, &receivedMessageLength, &payloadType,
                                  &receivedExternalDataHash, &externalDataHashAlg);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to process the signed message", result);
        smg_delete(smg);
        smp_delete(smp);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }

    /* Check that the signed message contains payload. */
    if ((receivedMessageLength != payloadLength) ||
            (strncmp((char *) payload, (char *) receivedMessage, receivedMessageLength) != 0))
    {
        LOG( "Returned payload does not match the payload passed to smg_sign()", result);
        smg_delete(smg);
        smp_delete(smp);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }


    if (! ((receivedMessage >= signedMessage) &&
           (receivedMessage <= (signedMessage+signedMessageLength))))
    {
        printf("Returned payload is not within the original message\n");
    }

    printf("\nCleaning up\n");

    /* Delete secured message generator. */
    printf("Deleting secured message generator\n");
    smg_delete(smg);

    /* Delete secured message parser. */
    printf("Deleting secured message parser\n");
    smp_delete(smp);

    /* Close security context. */
    printf("Closing security context\n");
    result = sc_close(context);
    if (result != WS_SUCCESS)
    {
        LOG("Error closing wsc", result);
        (void)securityServices_shutdown();
        return result;
    }


    /* Unregister status callback. */
    printf("unregister status callback\n");
    result = securityServices_unRegisterReportCallback();
    if (result != WS_SUCCESS)
    {
        LOG("Error shutting down security services", result);
        return result;
    }

    /* Shutdown security services. */
    printf("Shutting down security services\n");
    result = securityServices_shutdown();
    if (result != WS_SUCCESS)
    {
        LOG("Error shutting down security services", result);
        return result;
    }
    printf("\n");

    return WS_SUCCESS;
}
