#!/bin/sh
#
#---Variables
#
# DOWNLOAD_FROM_SCMS - true  = Certificates were, or will be,  downloaded from
#                              an SCMS server.
#                      false = Certificates were obtained elsewhere.
# DOWNLOAD_FROM_SCMS_IP_VERSION - IPv6 = SCMS were download from the SCMS
#                                        server using IPv6 over the air.
#                                 IPv4 = SCMS were download from the SCMS
#                                        server using IPv4 over Ethernet.
# DOWNLOAD_FROM_SCMS_IPV4_ADDRESS - If DOWNLOAD_FROM_SCMS is true and
#                                   DOWNLOAD_FROM_SCMS_IP_VERSION is IPv4, then
#                                   this is the default IPv4 address of the
#                                   Ethernet port. This could be overriden if
#                                   DHCP is used
# DOWNLOAD_FROM_SCMS_DEFAULT_GATEWAY - If DOWNLOAD_FROM_SCMS is true and
#                                      DOWNLOAD_FROM_SCMS_IP_VERSION is IPv4,
#                                      then this is the IPv4 address of the
#                                      default gateway. This could be overriden if
#                                      DHCP is used
# DOWNLOAD_FROM_SCMS_USE_DHCP - true  = Use DHCP to determine the Ethernet port
#                                       IPv4 address and default gateway IPv4
#                                       address.
#                               false = Use DOWNLOAD_FROM_SCMS_IPV4_ADDRESS to
#                                       determine the Ethernet port IPv4
#                                       address, and
#                                       DOWNLOAD_FROM_SCMS_DEFAULT_GATEWAY to
#                                       determine the default gateway IPv4
#                                       address.
# AEROLINK_LOGGING - true  = Enable Aerolink logging.
#                    false = Disable Aerolink logging.
# TLS_API_LOGGING - true = The TLS library logs minimal API information to
#                          the /mnt/rwflash/libaerolinkPKI_log directory.
# TLS_ADDITIONAL_LOGGING - true = The TLS library logs additional API
#                                 information to the
#                                 /mnt/rwflash/libaerolinkPKI_log directory.
#                                 TLS_API_LOGGING must also be set to true for
#                                 logging to occur.
# TLS_CONSOLE_LOGGING - true = The TLS library prints certain information to
#                              the console. This is a subset of the information
#                              that is written to the log file, controlled by
#                              TLS_API_LOGGING.
# TCP_BRIDGE - true  = Launch ipv6_provider and tcpBridge. *** NOT USED FOR RSU ***
#              false = Do not launch ipv6_provider and tcpBridge
# TCP_BRIDGE_EXECUTABLE_PATH - The path to the tcpBridge and
#                              tcpBridgeServerSide executables.
# TCP_BRIDGE_SCMS_IPV4_ADDRESS - The IPv4 address of the SCMS server you are
#                                bridging to.
# TCP_BRIDGE_SCMS_PORT - The port you are communicating to the SCMS server
#                        through
# TCP_BRIDGE_YAHOO_TEST - true  = Send SCMS commands to yahoo.com instead of
#                                 the SCMS server. Useful if you just want to
#                                 test communication without using up an SCMS
#                                 download.
# TCP_BRIDGE_YAHOO_IPV4_ADDRESS - The IPv4 address of yahoo.com.
# TCP_BRIDGE_YAHOO_PORT - The port number of yahoo.com's SSL server.
#
	
    export DOWNLOAD_FROM_SCMS=false
    export DOWNLOAD_FROM_SCMS_IP_VERSION=IPv4
    export DOWNLOAD_FROM_SCMS_IPV4_ADDRESS=10.121.20.77
    export DOWNLOAD_FROM_SCMS_DEFAULT_GATEWAY=192.168.1.1
    export DOWNLOAD_FROM_SCMS_USE_DHCP=true            
    export AEROLINK_LOGGING=false
    export TLS_API_LOGGING=false
    export TLS_ADDITIONAL_LOGGING=false
    export TLS_CONSOLE_LOGGING=false
    export ALSMI_DEBUG_LOG=false
    export AEROLINK_TRACE=none
    export AEROLINK_TRACE_LOGNAME=/mnt/rwflash/aerolink-1.log

    export AEROLINK_STATE_DIR=/rwflash/aerolink
    export AEROLINK_CONFIG_DIR=/rwflash/configs/aerolink_conf
