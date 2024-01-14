/**************************************************************************
 *                                                                        *
 *     File Name:  defaults.h                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#ifndef DEFAULTS_H
#define DEFAULTS_H

#define ONE_SECOND       1000000

#define MAX_STR_LEN      101
#define MAX_BYTE_SIZE    100
#define MAX_BYTE_STR     2 * MAX_BYTE_SIZE + 1
#define MAX_KEY_LEN      26
#define MAX_TCP_DATA     0xFFFF

/* has to be long enough for byte string + key name and '=' 
   with only a single 'null' the MAX_KEY_LEN covers the '=' 
   need to add room for spaces (MAX_BYTE_SIZE) */
#define MAX_LINE_LEN     MAX_KEY_LEN + MAX_BYTE_STR + MAX_BYTE_SIZE

/* default values */
#define DEFAULT_PORT          16094
#define DEFAULT_IPVER         6
#define DEFAULT_NUMCLIENTS    10

#define ACK_LENGTH            16

typedef enum {
    FALSE = 0,
    TRUE 
} __attribute__((packed)) BOOLEAN;

#endif /* DEFAULTS_H */

