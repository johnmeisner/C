
/*
 * Demo program using the C API for encrypting data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "viicsec.h"

#define LOG(text, result)                         \
    printf("%s line %d: %s; result=%s\n", \
            __FILE__, __LINE__, text, ws_errid(result))


/*
 * Print a label and then the contents of the supplied buffer to stdout.
 */
static void printBytes(char *label, uint8_t const buffer[], uint32_t length)
{
    uint32_t i = 0;

    printf("%s", label);

    for( i = 0; i < length; i++ ) {
        printf("%02x ", buffer[i] & 0xffU);
    }

    printf("\n");
}


int main (int argc, char *argv[])
{
    /* General variables */
    int32_t printMessage = 0;
    int32_t printKey = 0;
    int32_t i;

    char *contextName = "denso.wsc";
    SecurityContextC context;

    /* Parsing variables */
    SecuredMessageParserC smp = NULL;
    uint8_t const *receivedMessage = NULL;
    uint32_t receivedMessageLength = 0;

    /* Generation variables */
    SecuredMessageGeneratorC smg = NULL;

    uint8_t plainText[256];
    uint32_t plainTextLength;

    /* Encryption variables */
    AerolinkEncryptionKey encryptionKey;
    uint8_t encryptedData[512];
    uint32_t encryptedDataLength = sizeof encryptedData;

    PayloadType     spduType;
    PayloadType     payloadType;
    uint8_t const *receivedExternalDataHash = NULL;
    ExternalDataHashAlg receivedExternalDataHashAlg = EDHA_NONE;

    SymmetricKeyType symmetricKeyType = SKT_DEK;
    AEROLINK_RESULT result;

    /* Spdu data */
    uint8_t signedPayload[] = {
                               0x03, 0x81, 0x00, 0x40, 0x03, 0x80, 0x0d, 0x73, 0x6f, 0x6d, 0x65, 0x20, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x00,
                               0x40, 0x01, 0x20, 0x00, 0x01, 0xca, 0x45, 0xa5, 0x4c, 0xe5, 0xf5, 0x81, 0x01, 0x01, 0x80, 0x03, 0x00, 0x80, 0xed, 0x88,
                               0x43, 0x43, 0xc4, 0xa8, 0xd4, 0x96, 0x10, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xb1, 0x2b, 0x05, 0x86, 0x00, 0x0c,
                               0x01, 0x01, 0x00, 0x01, 0x20, 0x80, 0x80, 0x83, 0x07, 0x37, 0x3a, 0x9f, 0xbc, 0xb8, 0xb1, 0x1c, 0x34, 0x92, 0x08, 0x75,
                               0x6b, 0x8f, 0x8d, 0xc5, 0x4a, 0x94, 0x96, 0xb8, 0x53, 0xd1, 0x1e, 0x69, 0x86, 0x45, 0x9c, 0x73, 0x95, 0x4d, 0xfc, 0xd0,
                               0x80, 0x82, 0x9e, 0xbc, 0x94, 0x84, 0x8d, 0xd9, 0xda, 0x68, 0xf6, 0x55, 0x05, 0xeb, 0xa3, 0x69, 0x94, 0xab, 0x5e, 0x2d,
                               0xfa, 0x9d, 0xe3, 0x99, 0xe7, 0xe3, 0xd5, 0x79, 0x57, 0xfe, 0x37, 0xcd, 0xb9, 0xaf, 0xb5, 0xe4, 0x7b, 0x89, 0xfa, 0x95,
                               0x36, 0x95, 0xd4, 0xcd, 0x5b, 0x10, 0xc6, 0x63, 0x0e, 0x10, 0xea, 0xd1, 0x29, 0xef, 0x39, 0xd5, 0x8f, 0x86, 0x4f, 0x31,
                               0xd6, 0xc0, 0x84, 0x86, 0x72, 0xff, 0x80, 0x82, 0x58, 0xb5, 0xb3, 0x80, 0xc8, 0xf8, 0xbd, 0x2b, 0x77, 0x40, 0xe3, 0x48,
                               0x7e, 0xbb, 0x2c, 0x93, 0x0e, 0xae, 0x21, 0x2d, 0xa3, 0xe2, 0xd8, 0xb7, 0x60, 0xf8, 0x55, 0xb6, 0x03, 0x38, 0x3f, 0x3f,
                               0x43, 0x39, 0xf1, 0x6c, 0x09, 0x23, 0xcd, 0x8a, 0xe6, 0x7b, 0xbc, 0x8b, 0x48, 0xf6, 0x05, 0xc0, 0x2d, 0xc3, 0xa4, 0xa1,
                               0x41, 0x71, 0x5e, 0xc9, 0x98, 0x49, 0x38, 0x5b, 0xf6, 0x30, 0x2f, 0x72};

    uint32_t signedPayloadLength = sizeof signedPayload;

    /*
     * Command line argument processing.
     */
    for( i = 1; i < argc; ++i )
    {
        if( strcmp("-pm", argv[i]) == 0 )
        {
            printMessage = 1;
        }

        else if( strcmp("-pk", argv[i]) == 0 )
        {
            printKey = 1;
        }
        else
        {
            printf("Invalid argument: %s is not supported\n", argv[i]);
            return 2;
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
     * Open application's security context.
     */
    printf("Opening security context\n");
    result = sc_open(contextName, &context);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to open wsc", result);
        (void) securityServices_shutdown();
        return result;
    }


    /*
     * Create Secured Message Generator.
     */
    printf("Creating a secured message generator\n");
    result = smg_new(context, &smg);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a secured message generator", result);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }


    /*
     * Create Secured Message Generator.
     */
    printf("Creating a secured message parser\n");
    result = smp_new(context, &smp);
    if (result != WS_SUCCESS)
    {
        LOG("Unable to create a secured message parser", result);
        (void) smg_delete(smg);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    /*
     * Generate encryption
     */
    printf ("Creating a symmetric encryption key\n");
    result = securityServices_generateSymmetricEncryptionKey(SEA_AES128CCM, symmetricKeyType, &encryptionKey);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_generateSymmetricEncryptionKey() failed", result);
        (void) smg_delete(smg);
        (void) smp_delete(smp);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    if (printKey)
    {
        printBytes("returned encryption key: ", encryptionKey.blob, encryptionKey.blobLen);
    }

    /*
     * Generate encrypted data.
     */
    AerolinkEncryptionKey const * recipients[] =  {&encryptionKey};
    size_t numRecipients = sizeof recipients / sizeof recipients[0];
    printf("\nEncrypting spdu data\n");
    memcpy(plainText, signedPayload, sizeof signedPayload);
    plainTextLength = signedPayloadLength;
    encryptedDataLength = sizeof encryptedData;
    result = smg_encrypt(smg, recipients, numRecipients,
                         plainText, plainTextLength, 1, encryptedData, &encryptedDataLength);
    if (result != WS_SUCCESS)
    {
        LOG("smg_encrypt() failed", result);
        (void) securityServices_deleteEncryptionKey(&encryptionKey);
        free((uint8_t *)encryptionKey.blob);
        (void) smg_delete(smg);
        (void) smp_delete(smp);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    printf("Encrypted data generated correctly\n");

    if (printMessage)
    {
        printBytes("\nEncrypted data is:\n  ",
                   encryptedData, encryptedDataLength);
    }

    /*
     * Process the encrypted data
     */
    printf("\nProcessing \"received\" encrypted message\n");
    printf("Parsing encrypted message\n");
    result = smp_extract(smp,
                         encryptedData, encryptedDataLength, &spduType,
                         &receivedMessage, &receivedMessageLength, &payloadType,
                         &receivedExternalDataHash, &receivedExternalDataHashAlg);
    if (result != WS_SUCCESS)
    {
        LOG("Failed to parse the encrypted data", result);
        (void) securityServices_deleteEncryptionKey(&encryptionKey);
        free((uint8_t *)encryptionKey.blob);
        (void) smg_delete(smg);
        (void) smp_delete(smp);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    else
    {
        printf("Message decrypted\n");

        if ((receivedMessageLength != plainTextLength) ||
            (memcmp(plainText, receivedMessage, receivedMessageLength) != 0))
        {
            LOG( "Returned payload does not match the payload passed to smg_encrypt()", result);
            (void) securityServices_deleteEncryptionKey(&encryptionKey);
            free((uint8_t *)encryptionKey.blob);
            (void) smg_delete(smg);
            (void) smp_delete(smp);
            (void) sc_close(context);
            (void) securityServices_shutdown();
            return result;
        }
    }

    /*
     * Delete encryption key
    */
    printf ("\nDeleting encryption key\n");
    result = securityServices_deleteEncryptionKey(&encryptionKey);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_deleteEncryptionKey() failed", result);
        free((uint8_t *)encryptionKey.blob);
        (void) smg_delete(smg);
        (void) smp_delete(smp);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    /*
     * Delete the blob pointer
     */
    if (encryptionKey.blob)
    {
        free((uint8_t *)encryptionKey.blob);
    }

    /* Delete secured message generator. */
    printf("Deleting secured message generator\n");
    result = smg_delete(smg);
    if (result != WS_SUCCESS)
    {
        LOG("Error deleting smg", result);
        (void) smp_delete(smp);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }

    /* Delete secured message parser. */
    printf("Deleting secured message parser\n");
    result = smp_delete(smp);
    if (result != WS_SUCCESS)
    {
        LOG("Error deleting smp", result);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }


    /* Close security context. */
    printf("Closing security context\n");
    result = sc_close(context);
    if (result != WS_SUCCESS)
    {
        LOG("Error closing wsc", result);
        (void) securityServices_shutdown();
        return result;
    }

    /* Shutdown security services. */
    printf("Shutting down security services\n");
    result = securityServices_shutdown();
    if (result != WS_SUCCESS)
    {
        LOG("Error shutting down security services", result);
    }

    printf("\n");
    return result;
}
