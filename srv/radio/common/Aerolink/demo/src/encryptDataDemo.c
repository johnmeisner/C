/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

/*
 * Demo program using the C API for encrypting data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "viicsec.h"

#define LOG(text, result)                         \
    printf("%s line %d: %s; result = %s\n", \
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
    uint8_t plainText[] = "some payload";
    uint32_t plainTextLength = sizeof plainText;

    /* Encryption variables */
    AerolinkEncryptionKey encryptionKey;
    uint8_t encryptedData[512];
    uint32_t encryptedDataLength = sizeof encryptedData;

    PayloadType     spduType = PLT_NOT_SET;
    PayloadType     payloadType = PLT_NOT_SET;
    uint8_t const *receivedExternalDataHash = NULL;
    ExternalDataHashAlg receivedExternalDataHashAlg = EDHA_NONE;

    PublicEncryptionAlgorithm publicEncryptionAlg = PEA_ECIES_NISTP256;

    AEROLINK_RESULT result;

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

        else if ( strcmp("-curveChoice", argv[i]) == 0 )
        {
            if (i+1 < argc)
            {
                if ( strcmp("nistp256", argv[i+1]) == 0 )
                {
                    publicEncryptionAlg = PEA_ECIES_NISTP256;
                }
                else if ( strcmp("brainpoolp256r1", argv[i+1]) == 0 )
                {
                    publicEncryptionAlg = PEA_ECIES_BRAINPOOL256R1;
                }
                else if ( strcmp("sm2", argv[i+1]) == 0 )
                {
                    publicEncryptionAlg = PEA_ECENC_SM2;
                }
                else
                {
                    printf("Invalid argument provided: -curveChoice requires one of the following strings: nistp256, brainpoolp256r1, or sm2.\n");
                    return 2;
                }
                i++;
            }
            else
            {
                printf("Invalid argument: -curveChoice requires one of the following strings: nistp256, brainpoolp256r1, or sm2.\n");
                return 3;
            }
        }

        else
        {
            printf("Invalid argument: %s is not supported\n", argv[i]);
            return 4;
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
     * Create Secured Message Parser.
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
     * Generate encryption keypair.
     */
    printf ("Creating public encryption key pair\n");
    result = securityServices_generatePublicEncryptionKeyPair(publicEncryptionAlg, &encryptionKey);
    if (result != WS_SUCCESS)
    {
        LOG("securityServices_generatePublicEncryptionKeyPair() failed", result);
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
    printf("\nEncrypting data\n");
    AerolinkEncryptionKey const * recipients[] =  {&encryptionKey};
    size_t numRecipients = sizeof recipients / sizeof recipients[0];
    result = smg_encrypt(smg, recipients, numRecipients,
                         plainText, plainTextLength, 0, encryptedData, &encryptedDataLength);
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
     * Process the encrypted data.
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
            (strncmp((char *) plainText, (char *) receivedMessage, receivedMessageLength) != 0))
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
    if (spduType != PLT_ENCRYPTED_SPDU)
    {
        printf("Returned input SPDU type incorrect %d\n", spduType);
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
        printf("Input SPDU type = PLT_ENCRYPTED_SPDU\n");
    }
    if (payloadType != PLT_UNSECURED_APP_PAYLOAD)
    {
        printf("Returned output payload type incorrect %d\n", payloadType);
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
        printf("Output payload type = PLT_UNSECURED_APP_PAYLOAD\n");
    }


    /*
     * Delete encryption key.
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
     * Delete the blob pointer.
     */
    if (encryptionKey.blob)
    {
        free((uint8_t *)encryptionKey.blob);
    }

    /*
     * Delete secured message generator.
     */
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

    /*
     * Delete secured message parser.
     */
    printf("Deleting secured message parser\n");
    result = smp_delete(smp);
    if (result != WS_SUCCESS)
    {
        LOG("Error deleting smp", result);
        (void) sc_close(context);
        (void) securityServices_shutdown();
        return result;
    }


    /*
     * Close security context.
     */
    printf("Closing security context\n");
    result = sc_close(context);
    if (result != WS_SUCCESS)
    {
        LOG("Error closing wsc", result);
        (void) securityServices_shutdown();
        return result;
    }

    /*
     * Shutdown security services.
     */
    printf("Shutting down security services\n");
    result = securityServices_shutdown();
    if (result != WS_SUCCESS)
    {
        LOG("Error shutting down security services", result);
    }

    printf("\n");
    return result;
}
