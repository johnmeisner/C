/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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
/**
 * @file rtxPcapFile.h
 * Data structures and functions for processing PCAP files.
 */
#ifndef _RTXPCAPFILE_H_
#define _RTXPCAPFILE_H_

#include "rtxsrc/rtxContext.h"

typedef struct pcap_hdr_s {
   OSOCTET magic_number[4]; /* magic number */
   OSUINT16 version_major;  /* major version number */
   OSUINT16 version_minor;  /* minor version number */
   OSINT32  thiszone;       /* GMT to local correction */
   OSUINT32 sigfigs;        /* accuracy of timestamps */
   OSUINT32 snaplen;        /* max length of captured packets, in octets */
   OSUINT32 network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
   OSUINT32 ts_sec;         /* timestamp seconds */
   OSUINT32 ts_usec;        /* timestamp microseconds */
   OSUINT32 incl_len;       /* number of octets of packet saved in file */
   OSUINT32 orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

typedef struct ether_header_s {
   OSUINT8 ether_dhost[6];  /* destination host MAC address */
   OSUINT8 ether_shost[6];  /* source host MAC address */
   OSUINT16 ether_type;
} ether_header_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function decodes a PCAP global header structure.
 *
 * @param pctxt  Pointer to context holding message to decode.
 * @param pPcapHdr Pointer to structure to receive decoded header.
 *    If pointer is NULL, header record will be skipped.
 * @param pswapped Boolean to receive results of magic number analysis
 *    to determine bytes need to be swapped when decoding integer values.
 * @return Status of decode operation: 0 = success, negative value = error
 */
EXTERNRT int rtxDecPcapHdr
(OSCTXT* pctxt, pcap_hdr_t* pPcapHdr, OSBOOL* pswapped);

/**
 * This function decodes a PCAP record header structure.
 *
 * @param pctxt Pointer to context holding message to decode.
 * @param pPcapRecHdr Pointer to structure to receive decoded header.
 *    If pointer is NULL, header record will be skipped.
 * @param swapped Boolean indicating if byte swapping needs to be done
 *    when decoding integer values.
 * @return Status of decode operation: 0 = success, negative value = error
 */
EXTERNRT int rtxDecPcapRecHdr
(OSCTXT* pctxt, pcaprec_hdr_t* pPcapRecHdr, OSBOOL swapped);

/**
 * This function decodes an Ethernet header structure.
 *
 * @param pctxt Pointer to context holding message to decode.
 * @param pEtherHdr Pointer to structure to receive decoded header.
 *    If pointer is NULL, header record will be skipped.
 * @param swapped Boolean indicating if byte swapping needs to be done
 *    when decoding integer values.
 * @return Status of decode operation: 0 = success, negative value = error
 */
EXTERNRT int rtxDecEtherHdr
(OSCTXT* pctxt, ether_header_t* pEtherHdr, OSBOOL swapped);

/**
 * This function prints the contents of a PCAP global header structure
 * to stdout.
 *
 * @param pPcapHdr Pointer to structure to print.
 */
EXTERNRT void rtxPrintPcapHdr(const pcap_hdr_t* pPcapHdr);

/**
 * This function prints the contents of a PCAP record header structure
 * to stdout.
 *
 * @param pPcapRecHdr Pointer to structure to print.
 */
EXTERNRT void rtxPrintPcapRecHdr(const pcaprec_hdr_t* pPcapRecHdr);

#ifdef __cplusplus
}
#endif

#endif
