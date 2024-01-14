/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

/*
 * Demo program using the C API for generating/processing signed messages
 * asynchronously.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "viicsec.h"

#define LOG(text, result)                         \
    fprintf(stderr, "%s line %d: %s; result=%s\n", \
            __FILE__, __LINE__, text, ws_errid(result))

/*
 * Print a label and then the contents of the suppplied buffer to stdout
 */
static void printBytes(char *label, uint8_t *buffer, uint32_t length)
{
    uint32_t i = 0;

    printf("%s", label);

    for( i = 0; i < length; i++ ) {
        printf("%02x ", buffer[i] & 0xff);
    }

    printf("\n");
}


static volatile uint32_t    signCallbackCalled = 0;
static AEROLINK_RESULT      signCallbackStatus = WS_SUCCESS;
static void                *signCallbackUserData = NULL;
static uint8_t             *signCallbackData = NULL;
static uint32_t             signCallbackDataLen = 0;
static void                 signCallback(AEROLINK_RESULT returnCode,
                                         void           *userCallbackData,
                                         uint8_t        *cbSignedData,
                                         uint32_t        cbSignedDataLen)
{
    /* This code needs to be replaced with the user's desired actions.
     * A real application will have to be concerned with locking issues that
     * are not being addressed in this sample.
     * A real application may want to use a condition variable to signal that
     * the callback has occurred.
     */
    signCallbackStatus = returnCode;
    signCallbackUserData = userCallbackData;
    signCallbackData = cbSignedData;
    signCallbackDataLen = cbSignedDataLen;
    signCallbackCalled = 1;
}

static void setPermissions(
    SigningPermissions    *p,
    uint32_t               psid,
    uint8_t const * const  ssp,
    uint32_t               sspLen)
{
    p->psid = psid;
    p->isBitmappedSsp = 0;
    p->ssp = ssp;
    p->sspMask = NULL;
    p->sspLen = sspLen;
}

static AEROLINK_RESULT
generateSignedMessage(
        SecuredMessageGeneratorC    smg,
        uint32_t                    psidValue,
        uint8_t                     ssp[31],
        uint32_t                    sspLength,
        uint8_t                     payload[],
        uint32_t                    payloadLength,
        uint8_t const              *externalData,
        ExternalDataHashAlg         externalDataAlgorithm,
        uint8_t                     signedMessage[512],
        uint32_t                    signedMessageLength,
        void                       *userData,
        int                         printMessage)
{
    AEROLINK_RESULT result;

    /* This is a sample call of the async interface only.
     * This is not how a real application using a callback should be structured.
     */
    SigningPermissions permissions;
    setPermissions(&permissions, psidValue, ssp, sspLength);
    result = smg_signAsync(smg,
                           permissions,
                           STO_AUTO,
                           0,
                           payload, payloadLength,
                           0,
                           externalData, externalDataAlgorithm,
                           NULL,
                           signedMessage, signedMessageLength,
                           userData, signCallback);

    if (result != WS_SUCCESS)
    {
        LOG("Problem generating signed message", result);
        return result;
    }

    /* At this point your application will want to do other processing while
     * waiting for the callback to be fired.
     */
    while (!signCallbackCalled)
    {
        printf("Waiting for the sign callback to fire, taking a short nap\n");
        sleep(1);
    }
    signCallbackCalled = 0;

    if (signCallbackStatus != WS_SUCCESS)
    {
        printf("failed to generate the messaage error=%d(%s)\n", signCallbackStatus, ws_errid(signCallbackStatus));
        return signCallbackStatus;
    }
    else
    {
        printf("Signed message generated correctly\n");
        if (printMessage)
            printBytes("\nSigned message is:\n  ", signCallbackData, signCallbackDataLen);
    }

    return WS_SUCCESS;
}


static volatile uint32_t validateCallbackCalled = 0;
static AEROLINK_RESULT validateCallbackStatus = WS_SUCCESS;
static void            *validateCallbackUserData = NULL;
static void validateCallback(AEROLINK_RESULT returnCode, void *userCallbackData)
{
    /* This code needs to be replaced with the users desired actions.
     * A real application will have to be concerned with locking issues that
     * are not being addressed in this sample.
     * A real application may want to use a condition variable to signal that
     * the callback has occurred.
     */
    validateCallbackStatus = returnCode;
    validateCallbackUserData = userCallbackData;
    validateCallbackCalled = 1;
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
    ExternalDataHashAlg    *externalDataHashAlg,
    void                   *userData)
{
    AEROLINK_RESULT result;

    /*
     * Generation time "from" message payload.
     */
    int32_t genTimeLatitude;
    int32_t genTimeLongitude;
    uint16_t genTimeElevation;

    if ((smp == NULL) || (spduType == NULL) || (receivedMessageLength == NULL))
    {
        return WS_ERR_BAD_ARGS;
    }

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
     *
     * This is a sample call of the async interface only.
     * This is not how a real application using a callback should be structured.
     */
    printf("Verifying message and certificates' signatures\n");
    result = smp_verifySignaturesAsync(smp, userData, validateCallback);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    /* At this point your application will want to do other processing while
     * waiting for the callback to be fired.
     */
    while (!validateCallbackCalled)
    {
        printf("Waiting for the validate callback to fire, taking a short nap\n");
        sleep(1);
    }

    validateCallbackCalled = 0;
    if (validateCallbackStatus == WS_SUCCESS)
    {
        printf("Message is valid\n");
    }

    return validateCallbackStatus;
}

int main (int argc, char **argv)
{
    /* General variables */
    int printMessage = 0;
    int i;

    SecurityContextC context;
    SecuredMessageGeneratorC smg = NULL;
    SecuredMessageParserC smp = NULL;

    char *contextName = "denso.wsc";

    /* Time adjustment: number of leap seconds since 1/1/2004. */
    int32_t leapSeconds;

    /* Position variables */
    int32_t latitude;
    int32_t longitude;
    uint16_t elevation;
    uint16_t countryCode;

    /* Signing variables */
    uint32_t  psidValue = 0x20;
    uint8_t   sspValue [31] = {0};
    uint32_t  sspLength = 0;

    uint32_t  userProvidedPsidValue = 0;
    uint8_t   userProvidedSspValue [31] = {0};
    uint32_t  userProvidedSspLength = 0;

    uint8_t payload[] = "some payload";
    uint32_t payloadLength = sizeof payload;

    uint8_t signedMessage[512];
    uint32_t signedMessageLength = sizeof signedMessage;

    uint8_t externalDataHash[32];
    uint8_t const *receivedExternalDataHash = NULL;
    ExternalDataHashAlg receivedExternalDataHashAlg;

    uint32_t userData[] = {1, 2, 3, 4};

    /* Parsing variables */
    PayloadType  spduType;
    PayloadType  payloadType;
    uint8_t const *receivedMessage;
    uint32_t receivedMessageLength = 0;
    AEROLINK_RESULT result;


    /*
     * Command line argument processing.
     */
    for( i = 1; i < argc; ++i )
    {
        if (strcmp("-p", argv[i]) == 0)
        {
            printMessage = 1;
        }

        if (strcmp("-psid", argv[i]) == 0)
        {
            char *end;
            userProvidedPsidValue = (uint32_t) strtol(argv[++i], &end, 0);
        }

        if( strcmp("-ssp", argv[i]) == 0 )
        {
            char *inputSsp = argv[++i];
            printf ("ssp = %s\n", inputSsp);
            char *rest = inputSsp;
            char *end;

            char *token = strtok_r(rest, "-", &rest);
            while (token && (userProvidedSspLength < sizeof userProvidedSspValue))
            {
                userProvidedSspValue[userProvidedSspLength++] = (uint8_t)strtol(token, &end,  16);
                token = strtok_r(rest, "-", &rest);
            }
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
    elevation = 0x20;
    countryCode = 0x348;

    result = securityServices_setCurrentLocation(latitude, longitude, elevation, countryCode);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_setPosition() failed", result);
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
        return result;
    }

    /*
     * Get the filename of the security context
     */
    {
        char const * name = NULL;
        result = sc_getName(context, &name);
        if (result != WS_SUCCESS || !name)
        {
            LOG("Failed to get the filename of the security context", result);
            smg_delete(smg);
            smp_delete(smp);
            (void)sc_close(context);
            (void)securityServices_shutdown();
            return result;
        }

        printf("The name of the security context is '%s'\n", name);
    }

    /*
     * Create Secured Message Generator
     */
    printf ("Creating a secured message generator\n");
    result = smg_new(context, &smg);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a signed message generator", result);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Create Secured Message Parser
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
     * Generate signed message
     */
    printf("\nSigning the message\n");


    if (userProvidedPsidValue > 0)
    {
        psidValue = userProvidedPsidValue;
    }

    if (userProvidedSspLength > 0)
    {
        sspLength = userProvidedSspLength;
        memcpy(sspValue, userProvidedSspValue, userProvidedSspLength);
    }

    result = generateSignedMessage(smg, psidValue, sspValue, sspLength,
                                   payload, payloadLength, externalDataHash, EDHA_NONE,
                                   signedMessage, signedMessageLength, userData,
                                   printMessage);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to generate the signed message", result);
        smg_delete(smg);
        smp_delete(smp);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
    }

    /*
     * Process signed message.
     */
    result = processSignedMessage(smp,
                                  signCallbackData, signCallbackDataLen, &spduType,
                                  &receivedMessage, &receivedMessageLength, &payloadType,
                                  &receivedExternalDataHash, &receivedExternalDataHashAlg, userData);
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
    if (receivedMessageLength != payloadLength ||
            strncmp((char *) payload, (char *) receivedMessage, receivedMessageLength) != 0)
    {
        LOG( "The returned payload does not match the payload passed to smg_sign()", result);
        smg_delete(smg);
        smp_delete(smp);
        (void)sc_close(context);
        (void)securityServices_shutdown();
        return result;
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
