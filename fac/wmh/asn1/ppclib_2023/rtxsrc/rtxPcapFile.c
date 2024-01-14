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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxPcapFile.h"

static int decodePcapUInt
(OSCTXT* pctxt, OSUINT32 nbytes, OSBOOL swapped, OSUINT32* pvalue)
{
   int ret = 0;
   OSOCTET numbuf[8];
   OSUINT32 i, value = 0;

   if (nbytes != 4 && nbytes != 2) {
      return LOG_RTERR(pctxt, RTERR_INVPARAM);
   }
   ret = rtxReadBytes(pctxt, numbuf, nbytes);
   if (0 != ret) return LOG_RTERR(pctxt, ret);

   if (swapped) {
      for (i = nbytes; i > 0; i--) {
         value = value * 256 + numbuf[i-1];
      }
   }
   else {
      for (i = 0; i < nbytes; i++) {
         value = value * 256 + numbuf[i];
      }
   }
   if (pvalue) *pvalue = value;

   return 0;
}

int rtxDecPcapHdr(OSCTXT* pctxt, pcap_hdr_t* pPcapHdr, OSBOOL* pswapped)
{
   int ret;
   if (pPcapHdr) {
      OSBOOL swapped;
      OSUINT32 ui32 = 0;

      /* magic number */
      ret = rtxReadBytes(pctxt, pPcapHdr->magic_number, 4);
      if (0 != ret) return LOG_RTERR(pctxt, ret);

      /* Check magic number */
      swapped = (pPcapHdr->magic_number[0] == 0xd4);
      if (pswapped) *pswapped = swapped;

      /* Decode other header fields */
      ret = decodePcapUInt(pctxt, 2, swapped, &ui32);
      if (0 == ret) {
         pPcapHdr->version_major = (OSUINT16)ui32;
         ret = decodePcapUInt(pctxt, 2, swapped, &ui32);
      }
      if (0 == ret) {
         pPcapHdr->version_minor = (OSUINT16)ui32;
         ret = decodePcapUInt(pctxt, 4, swapped, &ui32);
      }
      if (0 == ret) {
         pPcapHdr->thiszone = (OSINT32)ui32;
         ret = decodePcapUInt(pctxt, 4, swapped, &pPcapHdr->sigfigs);
      }
      if (0 == ret) {
         ret = decodePcapUInt(pctxt, 4, swapped, &pPcapHdr->snaplen);
      }
      if (0 == ret) {
         ret = decodePcapUInt (pctxt, 4, swapped, &pPcapHdr->network);
      }
   }
   else {
      ret = rtxReadBytes(pctxt, 0, sizeof(pcap_hdr_t));
   }

   return (0 == ret) ? 0 : LOG_RTERR(pctxt, ret);
}

int rtxDecPcapRecHdr(OSCTXT* pctxt, pcaprec_hdr_t* pPcapRecHdr, OSBOOL swapped)
{
   int ret;
   if (pPcapRecHdr) {
      ret = decodePcapUInt (pctxt, 4, swapped, &pPcapRecHdr->ts_sec);
      if (0 == ret) {
         ret = decodePcapUInt(pctxt, 4, swapped, &pPcapRecHdr->ts_usec);
      }
      if (0 == ret) {
         ret = decodePcapUInt(pctxt, 4, swapped, &pPcapRecHdr->incl_len);
      }
      if (0 == ret) {
         ret = decodePcapUInt (pctxt, 4, swapped, &pPcapRecHdr->orig_len);
      }
   }
   else {
      ret = rtxReadBytes(pctxt, 0, sizeof(pcaprec_hdr_t));
   }

   return (0 == ret) ? 0 : LOG_RTERR(pctxt, ret);
}

int rtxDecEtherHdr(OSCTXT* pctxt, ether_header_t* pEtherHdr, OSBOOL swapped)
{
   int ret;
   if (pEtherHdr) {
      ret = rtxReadBytes(pctxt, pEtherHdr->ether_dhost,
                         sizeof(pEtherHdr->ether_dhost));
      if (0 == ret) {
         ret = rtxReadBytes(pctxt, pEtherHdr->ether_shost,
                            sizeof(pEtherHdr->ether_shost));
      }
      if (0 == ret) {
         OSUINT32 ui32 = 0;
         ret = decodePcapUInt (pctxt, 2, swapped, &ui32);
         pEtherHdr->ether_type = (OSUINT16)ui32;
      }
   }
   else {
      ret = rtxReadBytes(pctxt, 0, sizeof(ether_header_t));
   }

   return (0 == ret) ? 0 : LOG_RTERR(pctxt, ret);
}

void rtxPrintPcapHdr(const pcap_hdr_t* pPcapHdr)
{
   printf ("magic number = [%02x, %02x, %02x, %02x]\n",
           pPcapHdr->magic_number[0], pPcapHdr->magic_number[1],
           pPcapHdr->magic_number[2], pPcapHdr->magic_number[3]);
   printf ("major version = %d\n", pPcapHdr->version_major);
   printf ("minor version = %d\n", pPcapHdr->version_minor);
   printf ("this zone = %d\n", pPcapHdr->thiszone);
   printf ("sigfigs = %d\n", pPcapHdr->sigfigs);
   printf ("snaplen = %d\n", pPcapHdr->snaplen);
   printf ("network = %d\n", pPcapHdr->network);
}

void rtxPrintPcapRecHdr(const pcaprec_hdr_t* pPcapRecHdr)
{
   printf("ts_sec = %d\n", pPcapRecHdr->ts_sec);
   printf("ts_usec = %d\n", pPcapRecHdr->ts_usec);
   printf("incl_len = %d\n", pPcapRecHdr->incl_len);
   printf("orig_len = %d\n", pPcapRecHdr->orig_len);
}
