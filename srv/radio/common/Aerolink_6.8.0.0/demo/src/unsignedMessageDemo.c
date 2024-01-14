/*
 * Demo program for the new C API for generating/processing unsigned messages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "viicsec.h"

#define LOG(text, result)                         \
    fprintf(stderr, "%s line %d: %s; result=%s\n", \
            __FILE__, __LINE__, text, ws_errid(result))


/*
 * Print a label and then the contents of the supplied buffer to stdout
 */
static void printBytes(char *label, uint8_t buffer[], uint32_t length)
{
    uint32_t i = 0;

    printf("%s", label);

    for( i = 0; i < length; i++ ) {
        printf("%02x ", buffer[i] & 0xffU);
    }

    printf("\n");
}

static AEROLINK_RESULT
generateUnsignedMessage(
        SecuredMessageGeneratorC    smg,
        uint8_t                     payload[],
        uint32_t                    payloadLength,
        uint8_t                     unsignedMessage[],
        uint32_t                   *unsignedMessageLength)
{
    /*
     * Generate unsigned message
     */
    return smg_createUnsecuredData(smg,
                                   payload, payloadLength,
                                   unsignedMessage, unsignedMessageLength);
}

static AEROLINK_RESULT
processUnsignedMessage(
        SecuredMessageParserC   smp,
        uint8_t                 unsignedMessage[512],
        uint32_t                unsignedMessageLength,
        PayloadType            *spduType,
        uint8_t const         **receivedMessage,
        uint32_t               *receivedMessageLength,
        PayloadType            *payloadType,
        uint8_t  const        **externalDataHash,
        ExternalDataHashAlg    *externalDataHashAlg)
{
    AEROLINK_RESULT result;

    /*
     * Process unsigned message
     */
    printf("\nChecking unsigned message\n");
    printf("Parsing unsigned message\n");
    result = smp_extract(smp,
                         unsignedMessage, unsignedMessageLength, spduType,
                         receivedMessage, receivedMessageLength, payloadType,
                         externalDataHash, externalDataHashAlg);
    if (result != WS_SUCCESS)
    {
        return result;
    }

    printf("Message is valid\n");
    return WS_SUCCESS;
}

int main (int argc, char *argv[])
{
    /* General variables */
    int32_t printMessage = 0;
    int32_t i;

    SecurityContextC  context;
    SecuredMessageGeneratorC smg = NULL;
    SecuredMessageParserC smp = NULL;

    char *contextName = "denso.wsc";

    /* Position variables */
    int32_t latitude;
    int32_t longitude;
    uint16_t elevation;
    uint16_t countryCode;

    uint8_t payload[] = "some payload";
    uint32_t payloadLength = sizeof payload;

    uint8_t unsignedMessage[512];
    uint32_t unsignedMessageLength = sizeof unsignedMessage;

    uint8_t const *receivedExternalDataHash = NULL;
    ExternalDataHashAlg externalDataHashAlg;

    /* Parsing variable */
    PayloadType  spduType;
    PayloadType  payloadType;
    uint8_t const *receivedMessage;
    uint32_t receivedMessageLength = 0;
    AEROLINK_RESULT result;

    
    /*
     * Command line argument processing
     */
    for( i = 1; i < argc; ++i )
    {
        if( strcmp("-p", argv[i]) == 0 )
        {
            printMessage = 1;
        }
    }

    /*
     * Get the library version
     */
    printf("Library version is '%s'\n", securityServices_getVersion());

    /*
     * Initialize security services
     */
    result = securityServices_initialize();
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_initialize() failed", result);
        return result;
    }


    /*
     * Set the current position
    */
    latitude = 0x323c4650;
    longitude = 0x01020304;
    elevation = 0x20;
    countryCode = 0x348;
    result = securityServices_setCurrentLocation(latitude, longitude, elevation, countryCode);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_setPosition() failed", result);
        securityServices_shutdown();
        return result;
    }


    /*
     * Open applications security context
     */
    printf ("Open security context\n");
    result = sc_open(contextName, &context);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to open wsc", result);
        securityServices_shutdown();
        return result;
    }

    /*
     * Create Secured Message Generator
     */
    result = smg_new(context, &smg);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a signed message generator", result);
        sc_close(context);
        securityServices_shutdown();
        return result;
    }

    /*
     * Create Secured Message Parser
     */
    result = smp_new(context, &smp);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a secured message parser", result);
        smg_delete(smg);
        sc_close(context);
        securityServices_shutdown();
        return result;
    }

    /*
     * Get the filename of the security context
     */
    {
        char const * name = NULL;
        result = sc_getName(context, &name);
        if ((result != WS_SUCCESS) || !name)
        {
            LOG("Failed to get the filename of the security context", result);
            smg_delete(smg);
            smp_delete(smp);
            sc_close(context);
            securityServices_shutdown();
            return result;
        }

        printf("Security context name is '%s'\n", name);
    }


    /*
     * Generate unsigned message
     */
    printf("Generating Unsigned message\n");
    result = generateUnsignedMessage(smg,
                                     payload, payloadLength,
                                     unsignedMessage, &unsignedMessageLength);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to generate the unsigned message", result);
        smg_delete(smg);
        smp_delete(smp);
        sc_close(context);
        securityServices_shutdown();
        return result;
    }

    if (printMessage)
    {
        printf("Unsigned message generated correctly\n");
        printBytes("\nUnsigned message is:\n  ",
                   unsignedMessage, unsignedMessageLength);
    }
    
    printf("unsignedMessageLength = %d\n", unsignedMessageLength);

    /*
     * Process unsigned message
     */
    result = processUnsignedMessage(smp, unsignedMessage, unsignedMessageLength,
                                    &spduType, &receivedMessage, &receivedMessageLength, &payloadType,
                                    &receivedExternalDataHash, &externalDataHashAlg);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to process the signed message", result);
        smg_delete(smg);
        smp_delete(smp);
        sc_close(context);
        securityServices_shutdown();
        return result;
    }

    printf("spduType = %d; payloadType = %d; receivedMessageLength = %d\n", spduType, payloadType, receivedMessageLength);

    /* Check that the signed message contains payload */
    if ((receivedMessageLength != payloadLength) ||
            (strncmp((char *) payload, (char *) receivedMessage, receivedMessageLength) != 0))
    {
        LOG( "Returned payload does not match the payload passed to smg_sign()", result);
        smg_delete(smg);
        smp_delete(smp);
        sc_close(context);
        securityServices_shutdown();
        return result;
    }


    if (! ((receivedMessage >= unsignedMessage) &&
           (receivedMessage <= (unsignedMessage+unsignedMessageLength))))
    {
        printf("Returned payload is not with in the original message");
    }



    printf("\nClosing security context\n");
    smg_delete(smg);
    smp_delete(smp);
    sc_close(context);
    securityServices_shutdown();

    return WS_SUCCESS;
}

