#ifndef PAYLOAD_TYPE_H
#define PAYLOAD_TYPE_H

/**
 * @file PayloadType.h
 * @brief Enumeration of payload types.
 */

typedef enum
{
    PLT_NOT_SET,
    PLT_UNSECURED_APP_PAYLOAD,
    PLT_UNSECURED_SPDU,
    PLT_SIGNED_SPDU,
    PLT_ENCRYPTED_SPDU,
    PLT_UNKNOWN,
} PayloadType;

#endif /* PAYLOAD_TYPE_H */
