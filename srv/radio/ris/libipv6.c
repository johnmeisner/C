/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: libipv6.c                                                        */
/*  Purpose: Functions added to set up and tear down the ipv6 connection.     */
/*                                                                            */
/* Copyright (C) 2020 DENSO International America, Inc.                       */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2020-09-02  VROLLINGER    Initial release.                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include "libipv6.h"
#include "ris.h"
#include "rs.h"
#include "type2str.h"

#define MAX_RADIOS 2
#define PRINT_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        fprintf(stderr, fmt, ##args); \
        counter++; \
    } \
}

/* Note that only the DSRC radio is supported. The C-V2X radio is not
 * supported. */
static char    IPv6Device[13] = "llc-sch-ipv6";
static bool_t  radioIPv6Configured[MAX_RADIOS] = {FALSE, FALSE};
static uint8_t IPv6Address[MAX_RADIOS][LENGTH_IPV6];
static uint8_t DefaultGateway[MAX_RADIOS][LENGTH_IPV6];
static uint8_t GatewayMacAddress[MAX_RADIOS][LENGTH_MAC];
static uint8_t prefixLen[MAX_RADIOS];
static bool_t  DefaultRouteSet = FALSE;
static uint8_t ProviderIPv6Address[MAX_RADIOS][LENGTH_IPV6];
static bool_t  IPv6AddressNTEntryAdded[MAX_RADIOS] = {FALSE, FALSE};

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/* Convert a MAC address into an equivalent IPv6 address. Has a lot of complex
 * (and perhaps unnecessary) logic to handle cases where the prefix length is
 * not 64.
 */
static void convertMacAddress2IpAddress(uint8_t *macAddress, uint8_t *ipv6Address,
    uint8_t *prefix, uint8_t prefixLen)
{
    uint8_t *lower8BytesPtr;
    uint8_t tmp[8];

    /*
     * 64 bits for the prefix length is probably what we will see most often.
     * If the prefix length is in fact 64, or if the prefix length is illegal,
     * convert the MAC address to the last 64 bits of the IPv6 address directly
     * into the ipv6Address buffer. Otherwise, convert it in a temporary buffer.
     */
    lower8BytesPtr = (prefixLen == 64 || prefixLen > 128) ? &ipv6Address[8] :
                                                            tmp;

    /* Do the MAC address to last 64 bits of IPv6 address conversion */
    memcpy(&lower8BytesPtr[0], &macAddress[0], 3);
    lower8BytesPtr[0] ^= 0x02; /* Meaning of local assignment bit inverted */
    lower8BytesPtr[3] = 0xff;
    lower8BytesPtr[4] = 0xfe;
    memcpy(&lower8BytesPtr[5], &macAddress[3], 3);

    if (prefixLen == 64 || prefixLen > 128) {
        /* If 64-bit prefix, copy prefix into first 64 bits */
        memcpy(ipv6Address, prefix, 8);
    }
    else {
        /*
         * The prefix is something other than 64 bits. We have to copy the
         * prefix into the first n bits. Then we have to copy the converted
         * stuff in the tmp buffer to the last 128 - n bits, or the last 64
         * bits, whichever is lower. Then we may possibly have to fiddle with
         * the byte that the bit count straddles (byte is part prefix, part
         * address; hopefully this will rarely happen).
         */
        int prefixBytes = prefixLen / 8;

        /* Start by zeroing out the address */
        bzero(ipv6Address, LENGTH_IPV6);
        /* Then copy the first (bitcount) / 8 bytes over */
        memcpy(ipv6Address, prefix, prefixBytes);

        if (prefixBytes < 8) {
            /* Copy the entire converted address to the end */
            memcpy(&ipv6Address[8], tmp, 8);
        }
        else {
            /* Copy as many bytes to the end as will fit */
            memcpy(&ipv6Address[prefixBytes], &tmp[prefixBytes - 8],
                   LENGTH_IPV6 - prefixBytes);

            /*
             * If one of the bytes straddles the prefix and the address, change
             * how ever many upper bits of that byte are appropriate from
             * address bytes to prefix bytes.
             */
            if (prefixLen % 8 != 0) {
                uint8_t mask = 0;
                int shift = 7;
                int i;

                for (i = 0; i < prefixLen % 8; i++) {
                    mask |= 1 << shift--;
                }

                ipv6Address[prefixBytes] =
                    (ipv6Address[prefixBytes] & ~mask) |
                    (prefix[prefixBytes] & mask);
            }
        }
    }
}

/* Deletes an entry from the neighbor table */
static void deleteNeighborTableEntryCmd(uint8_t *ipv6Addr)
{
    char cmd[80];

    sprintf(cmd, "ip -6 neigh delete "
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x "
                 "dev %s",
            ipv6Addr[0],  ipv6Addr[1],  ipv6Addr[2],  ipv6Addr[3],
            ipv6Addr[4],  ipv6Addr[5],  ipv6Addr[6],  ipv6Addr[7],
            ipv6Addr[8],  ipv6Addr[9],  ipv6Addr[10], ipv6Addr[11],
            ipv6Addr[12], ipv6Addr[13], ipv6Addr[14], ipv6Addr[15],
            IPv6Device);
#ifdef PRINT_CONSOLE_COMMANDS
    printf("RIS: %s\n", cmd);
#endif
    system(cmd);
}

/* Adds an entry to the neighbor table */
static void addNeighborTableEntryCmd(uint8_t *ipv6Addr, uint8_t *macAddr)
{
    char cmd[100];

    sprintf(cmd, "ip -6 neigh add "
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x "
                 "lladdr %02x:%02x:%02x:%02x:%02x:%02x "
                 "dev %s",
            ipv6Addr[0],  ipv6Addr[1],  ipv6Addr[2],  ipv6Addr[3],
            ipv6Addr[4],  ipv6Addr[5],  ipv6Addr[6],  ipv6Addr[7],
            ipv6Addr[8],  ipv6Addr[9],  ipv6Addr[10], ipv6Addr[11],
            ipv6Addr[12], ipv6Addr[13], ipv6Addr[14], ipv6Addr[15],
            macAddr[0], macAddr[1], macAddr[2],
            macAddr[3], macAddr[4], macAddr[5],
            IPv6Device);
#ifdef PRINT_CONSOLE_COMMANDS
    printf("RIS: %s\n", cmd);
#endif
    system(cmd);
}

/* Tears down the IPv6 setup */
rsResultCodeType wsuRisTeardownIPv6Info(uint8_t radio)
{
    if ((radio < MAX_RADIOS) && radioIPv6Configured[radio]) {
        char cmd[128];

        if (DefaultRouteSet &&
            !radioIPv6Configured[radio == 0 ? 1 : 0]) {
            /* Delete default route */
            sprintf(cmd, "ip -6 route delete default");

#ifdef PRINT_CONSOLE_COMMANDS
            printf("RIS: %s\n", cmd);
#endif
            system(cmd);
            DefaultRouteSet = FALSE;
        }

        /* If neighbor table entry was added for IPv6Address, delete it */
        if (IPv6AddressNTEntryAdded[radio]) {
            deleteNeighborTableEntryCmd(ProviderIPv6Address[radio]);
            IPv6AddressNTEntryAdded[radio] = FALSE;
        }

        /* Delete the global IPv6 address for our node */
        sprintf(cmd, "ip -6 addr delete %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d dev %s",
                IPv6Address[radio][0], IPv6Address[radio][1], IPv6Address[radio][2], IPv6Address[radio][3],
                IPv6Address[radio][4], IPv6Address[radio][5], IPv6Address[radio][6], IPv6Address[radio][7],
                IPv6Address[radio][8], IPv6Address[radio][9], IPv6Address[radio][10], IPv6Address[radio][11],
                IPv6Address[radio][12], IPv6Address[radio][13], IPv6Address[radio][14], IPv6Address[radio][15],
                prefixLen[radio], IPv6Device);
#ifdef PRINT_CONSOLE_COMMANDS
        printf("RIS: %s\n", cmd);
#endif
        system(cmd);

        /* Delete the neighbor table entry for the default gateway */
        /* Note: To avoid HUGE problems, you MUST issue this command AFTER the
         * above "ifconfig <device> inet6 <addr> delete" command!! */
        deleteNeighborTableEntryCmd(DefaultGateway[radio]);
        radioIPv6Configured[radio] = FALSE;
    }

    /* Disable IPv6 in the LLC driver */
    system("echo 0 > /sys/module/cw_llc/parameters/IPv6Enabled");
    return RS_SUCCESS;
}

/* This function deletes the old IPv6 information if present, and sets the IP
 * address and default gateway.
 */
rsResultCodeType wsuRisSetIPv6Info(serviceInfoType *serviceInfo, uint8_t radio)
{
    char              cmd[128];
    cfgIdType         cfgId;
    cfgType           cfgR;
    rsResultCodeType  risRet;
    FILE             *fp;
    bool_t            match;
    uint8_t           prefixLenBytes;
    uint8_t           bits;
    uint8_t           bit;
    uint8_t           mask;

    if (radio >= MAX_RADIOS) {
        return RS_ERANGE;
    }

    /* If radio was previously configured, undo the results of last time */
    if (radioIPv6Configured[radio]) {
        wsuRisTeardownIPv6Info(radio);
    }

    prefixLen[radio] = serviceInfo->prefixLen;

    /* Set the IPv6 device name */
    strcpy(IPv6Device, (radio == 0) ? "llc-cch-ipv6" : "llc-sch-ipv6");

    /* Get the radio to configure's current MAC address */
    cfgId          = (radio == 0) ? CFG_ID_MACADDRESS_RADIO_0 :
                                    CFG_ID_MACADDRESS_RADIO_1;
    cfgR.radioType = RT_DSRC;
    risRet         = wsuRisGetCfgReq(cfgId, &cfgR);

    if (risRet != RS_SUCCESS) {
        printf("Get wsuRisGetCfgReq(CFG_ID_MACADDRESS_RADIO_%u) failed (%s)\n",
               radio, rsResultCodeType2Str(risRet));
        return RS_ERANGE;
    }

    /* Enable IPv6 in the LLC driver */
    system("echo 1 > /sys/module/cw_llc/parameters/IPv6Enabled");

    /* Convert the MAC address to an IPv6 address */
    convertMacAddress2IpAddress(cfgR.u.RadioMacAddress, IPv6Address[radio],
                                serviceInfo->ipPrefix, prefixLen[radio]);

    /* Save the default Gateway */
    memcpy(DefaultGateway[radio], serviceInfo->defaultGateway, LENGTH_IPV6);

    /* Save the gateway MAC address */
    if (serviceInfo->routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK) {
        /* If the gateway MAC address extension header exists, get the MAC
         * address from there */
        memcpy(GatewayMacAddress[radio], serviceInfo->gatewayMacAddress, LENGTH_MAC);
    }
    else {
        /* Else use the MAC address where the WSA was received from */
        memcpy(GatewayMacAddress[radio], serviceInfo->sourceMac, LENGTH_MAC);
    }

    /* Create global IPv6 address for our node */
    sprintf(cmd, "ip -6 addr add %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d dev %s",
            IPv6Address[radio][0], IPv6Address[radio][1], IPv6Address[radio][2], IPv6Address[radio][3],
            IPv6Address[radio][4], IPv6Address[radio][5], IPv6Address[radio][6], IPv6Address[radio][7],
            IPv6Address[radio][8], IPv6Address[radio][9], IPv6Address[radio][10], IPv6Address[radio][11],
            IPv6Address[radio][12], IPv6Address[radio][13], IPv6Address[radio][14], IPv6Address[radio][15],
            prefixLen[radio], IPv6Device);
#ifdef PRINT_CONSOLE_COMMANDS
    printf("RIS: %s\n", cmd);
#endif
    system(cmd);

    /* Add the default gateway to the neighbor table */
    addNeighborTableEntryCmd(serviceInfo->defaultGateway,
                             GatewayMacAddress[radio]);

    /* Remove the symbolic link if it exists, and write a new resolv.conf */
    unlink("/etc/resolv.conf");

    if ((fp = fopen("/etc/resolv.conf", "w")) == NULL) {
        printf("%s: Error opening /etc/resolv.conf for writing.\n", __func__);
    }
    else {
        fprintf(fp, "# Generated by wsuRisSetIPv6Info\n");
        fprintf(fp, "\n");
        fprintf(fp, "nameserver %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            serviceInfo->primaryDns[0], serviceInfo->primaryDns[1],
            serviceInfo->primaryDns[2], serviceInfo->primaryDns[3],
            serviceInfo->primaryDns[4], serviceInfo->primaryDns[5],
            serviceInfo->primaryDns[6], serviceInfo->primaryDns[7],
            serviceInfo->primaryDns[8], serviceInfo->primaryDns[9],
            serviceInfo->primaryDns[10], serviceInfo->primaryDns[11],
            serviceInfo->primaryDns[12], serviceInfo->primaryDns[13],
            serviceInfo->primaryDns[14], serviceInfo->primaryDns[15]);

        if (serviceInfo->routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK) {
            fprintf(fp, "nameserver %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                serviceInfo->secondaryDns[0], serviceInfo->secondaryDns[1],
                serviceInfo->secondaryDns[2], serviceInfo->secondaryDns[3],
                serviceInfo->secondaryDns[4], serviceInfo->secondaryDns[5],
                serviceInfo->secondaryDns[6], serviceInfo->secondaryDns[7],
                serviceInfo->secondaryDns[8], serviceInfo->secondaryDns[9],
                serviceInfo->secondaryDns[10], serviceInfo->secondaryDns[11],
                serviceInfo->secondaryDns[12], serviceInfo->secondaryDns[13],
                serviceInfo->secondaryDns[14], serviceInfo->secondaryDns[15]);
        }

        fprintf(fp, "nameserver 8.8.8.8\n");

        if (serviceInfo->routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK) {
            fprintf(fp, "# Too many DNS servers configured, the following entries may be ignored.\n");
        }

        fprintf(fp, "nameserver 8.8.4.4\n");
        fclose(fp);
    }

    /* Add a default route to the gateway */
    if (!DefaultRouteSet) {
        sprintf(cmd, "ip -6 route add default via %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                serviceInfo->defaultGateway[0], serviceInfo->defaultGateway[1],
                serviceInfo->defaultGateway[2], serviceInfo->defaultGateway[3],
                serviceInfo->defaultGateway[4], serviceInfo->defaultGateway[5],
                serviceInfo->defaultGateway[6], serviceInfo->defaultGateway[7],
                serviceInfo->defaultGateway[8], serviceInfo->defaultGateway[9],
                serviceInfo->defaultGateway[10], serviceInfo->defaultGateway[11],
                serviceInfo->defaultGateway[12], serviceInfo->defaultGateway[13],
                serviceInfo->defaultGateway[14], serviceInfo->defaultGateway[15]);
#ifdef PRINT_CONSOLE_COMMANDS
        printf("RIS: %s\n", cmd);
#endif
        system(cmd);
        DefaultRouteSet = TRUE;
    }

    /* If the ipv6Address field is on the same subnet as specified by the
     * ipPrefix and prefixLen fields, and the ipv6Address field is different
     * from the defaultGateway field, add a neighbor table entry for the
     * ipv6Address field and the providerMacAddress field. */
    if (serviceInfo->serviceBitmask & WSA_EXT_IPV6ADDRESS_BITMASK) {
        match = TRUE;
        prefixLenBytes = serviceInfo->prefixLen / 8;
        bits = serviceInfo->prefixLen % 8;

        if (memcmp(serviceInfo->ipv6Address, serviceInfo->ipPrefix,
                   prefixLenBytes) != 0) {
            match = FALSE;
        }
        else if (bits != 0) {
            bit  = 0x80;
            mask = 0;

            while (bits != 0) {
                mask |= bit;
                bit >>= 1;
                bits--;
            }

            if ((serviceInfo->ipv6Address[prefixLenBytes] & mask) !=
                (serviceInfo->ipPrefix[prefixLenBytes] & mask)) {
                match = FALSE;
            }
        }

        if (match && memcmp(serviceInfo->ipv6Address, serviceInfo->defaultGateway,
                            LENGTH_IPV6) != 0) {
            /* Add neighbor table entry only if IPv6Address and ipPrefix match,
             * and IPv6Address and defaultGateway are not equal */
            memcpy(ProviderIPv6Address[radio], serviceInfo->ipv6Address,
                   LENGTH_IPV6);
            addNeighborTableEntryCmd(serviceInfo->ipv6Address,
                                     (serviceInfo->serviceBitmask &
                                      WSA_EXT_PROVIDER_MACADDRESS_BITMASK) ?
                                     serviceInfo->providerMacAddress :
                                     GatewayMacAddress[radio]);
            IPv6AddressNTEntryAdded[radio] = TRUE;
        }
    } // if (serviceInfo->serviceBitmask & WSA_EXT_IPV6ADRESS_BITMASK)

    radioIPv6Configured[radio] = TRUE;
    return RS_SUCCESS;
}

int wsuRisSetRadioForIPv6(uint8_t radio)
{
    return RS_SUCCESS;
}


