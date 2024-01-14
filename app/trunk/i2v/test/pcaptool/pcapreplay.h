/**************************************************************************
 *                                                                        *
 *     File Name:  pcapreplay.h                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#ifndef PCAPREPLAY_H
#define PCAPREPLAY_H

#define ONE_SECOND       1000000

#define MAX_STR_LEN      101
#define MAX_BYTE_SIZE    100
#define MAX_BYTE_STR     2 * MAX_BYTE_SIZE + 1
#define MAX_KEY_LEN      26
#define MAX_WSM          1440

/* has to be long enough for byte string + key name and '=' 
   with only a single 'null' the MAX_KEY_LEN covers the '=' 
   need to add room for spaces (MAX_BYTE_SIZE) */
#define MAX_LINE_LEN     MAX_KEY_LEN + MAX_BYTE_STR + MAX_BYTE_SIZE

#define WSM_MSG_TYPE     0x88dc

typedef enum {
    FALSE = 0,
    TRUE 
} __attribute__((packed)) BOOLEAN;

/* for future support, add new types here - these
   are used in switch statements so if multiple 
   types are supported that will require its own
   type - i.e. BSM_SRM */
typedef enum {
    BSM = 0,
} wsmTypes;

/* courtesy wireshark */
typedef struct pcap_hdr_s {
    unsigned int magicNum;
    unsigned short versionMaj;
    unsigned short versionMin;
    signed int thiszone;
    unsigned int sigfigs;
    unsigned int snaplen;
    unsigned int network;
} __attribute__((packed)) pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    unsigned int tsSec;
    unsigned int tsUsec;
    unsigned int capLen;
    unsigned int origLen;
} __attribute__((packed)) pcaprec_hdr_t;

#endif /* PCAPREPLAY_H */

