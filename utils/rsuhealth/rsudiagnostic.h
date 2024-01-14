#ifndef __RSUDIAGNOSTICS_H
#define __RSUDIAGNOSTICS_H

#define RSUHEALTH_BOOTUP_WAIT  20  /* seconds so system can stablize & get ready after boot */
//TODO: Should it be longer? Shorter seems problematic if user wants to intervene.
#define RSUHEALTH_FAULT_TIME_THRESHOLD  120 /* seconds: consecutive fails then issue reset if recovery enabled */
#define RSUHEALTH_AT_WAIT_TIME 30 /* seconds for AT radio to become ready */

#if defined(MY_UNIT_TEST) /* Under test these wait times are not needed */
#undef RSUHEALTH_FAULT_TIME_THRESHOLD
#define RSUHEALTH_FAULT_TIME_THRESHOLD  2
#undef  RSUHEALTH_AT_WAIT_TIME
#define RSUHEALTH_AT_WAIT_TIME 1
#undef  RSUHEALTH_BOOTUP_WAIT
#define RSUHEALTH_BOOTUP_WAIT  1
#endif

/* RSU-5940  DV Hardware Specification Version:  3.5
 *
 * https://globaldenso.sharepoint.com/:w:/r/sites/NA001147/Shared%20Documents/RSU_Development/RSU-5940%20MP/Hardware/Specification/DV/DENSO_RSU-5940_Hardware_Spec_DV_20230822.docx?d=w860747575cc44786b81d3e3d0f3edf68&csf=1&web=1&e=e9AOmi
 *
 * Ambient: warn/max (85 C)         min (-40 C)
 * iMX8:    warn/max (113/123 C)    min (-40 C)
 * Secton:  warn/max (124/150 C)    min (-40 C)
 * Pluton:  warn/max (109.5/150 C)  min (-40 C)
 * HSM:     warn/max (123 C)        min (-40 C)
 *
 * DCIN:    warn/max(13.4/16V       min (10.6V)
 */
#define AMBIENT_TEMP_LOW_CRITICAL   -40
#define AMBIENT_TEMP_LOW_WARN      -35
#define AMBIENT_TEMP_HIGH_WARN      75
#define AMBIENT_TEMP_HIGH_CRITICAL  85

#define IMX8_TEMP_LOW_CRITICAL   -40
#define IMX8_TEMP_LOW_WARN      -35
#define IMX8_TEMP_HIGH_WARN      113
#define IMX8_TEMP_HIGH_CRITICAL  123

#define SECTON_TEMP_LOW_CRITICAL   -40
#define SECTON_TEMP_LOW_WARN      -35
#define SECTON_TEMP_HIGH_WARN      124
#define SECTON_TEMP_HIGH_CRITICAL  150

#define PLUTON_TEMP_LOW_CRITICAL   -40
#define PLUTON_TEMP_LOW_WARN      -35
#define PLUTON_TEMP_HIGH_WARN      110  /* round up */
#define PLUTON_TEMP_HIGH_CRITICAL  150

#define HSM_TEMP_LOW_CRITICAL   -40
#define HSM_TEMP_LOW_WARN      -35
#define HSM_TEMP_HIGH_WARN      113
#define HSM_TEMP_HIGH_CRITICAL  123

#define DCIN_LOW_CRITICAL  10.6f
#define DCIN_LOW_WARN     11.0f
#define DCIN_HIGH_WARN    14.0f /* with power supply I see volts: dcin(13.86,13.83,13.91)*/
#define DCIN_HIGH_CRITICAL 16.0f

#define GET_HSM_TEMP_REPLY "%lf,"
#define GET_HSM_TEMP "v2xse-example-app hsm_temp"
#define HSM_TEMP_MIN         "-999.999999999,"
#define HSM_TEMP_MAX          "999.999999999,"
#define HSM_TEMP_MIN_LENGTH  strlen("0,")
#define HSM_TEMP_MAX_LENGTH  strlen(HSM_TEMP_MIN)

#define GET_CPU_TEMP_REPLY "%d"
#define GET_CPU_TEMP "cat /sys/class/thermal/thermal_zone0/temp"
#define CPU_TEMP_MIN         "-10000000"
#define CPU_TEMP_MAX          "10000000"
#define CPU_TEMP_MIN_LENGTH  strlen("0")
#define CPU_TEMP_MAX_LENGTH  strlen(CPU_TEMP_MIN)

#define GET_AMBIENT_TEMP_REPLY "%d"
#define GET_AMBIENT_TEMP "cat /sys/class/hwmon/hwmon0/temp2_input"
#define AMBIENT_TEMP_MIN         "-100000"
#define AMBIENT_TEMP_MAX         "100000"
#define AMBIENT_TEMP_MIN_LENGTH  strlen("0")
#define AMBIENT_TEMP_MAX_LENGTH  strlen(AMBIENT_TEMP_MIN)

/* check one time only. v2xse-se-info has reset at end */
#define CHECK_HSM_CONFIG_MAX  1

//TODO: verify spi setup? Does it matter cause the interface check will fail anyways?
//spi device node is /dev/spidev0.0
//dal_dav_pin is 406
//dal_reset_pin is 488

#define HSM_GSA_VERSION      "2.12.1"
#define GET_HSM_GSA_REPLY " Version of GSA is:       %s"
#define GET_HSM_GSA "v2xse-se-info | grep 'Version of GSA'"

#define HSM_USAPP_VERSION    "2.12.3"
#define GET_HSM_USAPP_REPLY " Version of US Applet is: %s"
#define GET_HSM_USAPP "v2xse-se-info | grep 'Version of US Applet'"

#define HSM_JCOP_VERSION     "J5S2M0024BB70800"
#define GET_HSM_JCOP_REPLY "  JCOP version:                                         %s"
#define GET_HSM_JCOP "v2xse-se-info | grep 'JCOP version'"

#define HSM_PLATFORM_CONFIG  "00hKF7"
#define GET_HSM_CFG_REPLY " Platform configuration:                               %s"
#define GET_HSM_CFG "v2xse-se-info | grep 'Platform configuration'"


#define GET_RSU_IP_REPLY "%s"
#define GET_RSU_IP  "/usr/bin/conf_agent READ RSU_INFO SC0IPADDRESS"

#if defined(MY_UNIT_TEST)
#define DUMP_NETWORK_INIT "ifconfig ens33 > /tmp/network_init.txt"
#else
#define DUMP_NETWORK_INIT "ifconfig eth0 > /tmp/network_init.txt"
#endif

#define GET_IP_RX_PACKET_REPLY "          RX packets:%lu errors:%lu dropped:%lu overruns:%lu frame:%lu"
#define GET_IP_RX_PACKET "cat /tmp/network_init.txt | grep frame"

#define GET_IP_TX_PACKET_REPLY "          TX packets:%lu errors:%lu dropped:%lu overruns:%lu carrier:%lu"
#define GET_IP_TX_PACKET "cat /tmp/network_init.txt | grep carrier"

#define IP_PACKET_REPLY_MAX_SIZE 512

#if defined(MY_UNIT_TEST)
#define RSU_AEROLINK_CERTS_LOCATION "../../app/trunk/i2v/stubs/rwflash/aerolink/certificates/"
#define RSU_AEROLINK_CERTS_ACF      "../../app/trunk/i2v/stubs/rwflash/aerolink/certificates/%s.acf"
/* pipe stderr to NULL to avoid serial spluge */
#define RSU_AEROLINK_GET_CERTS  "ls -w1 ../../app/trunk/i2v/stubs/rwflash/aerolink/certificates/*.acf 2>/dev/null"
#else
#define RSU_AEROLINK_CERTS_LOCATION "/rwflash/aerolink/certificates/"
#define RSU_AEROLINK_CERTS_ACF      "/rwflash/aerolink/certificates/%s.acf"
/* pipe stderr to NULL to avoid serial spluge */
#define RSU_AEROLINK_GET_CERTS  "ls -w1 /rwflash/aerolink/certificates/*.acf 2>/dev/null"
#endif

#define RSU_ACF_TIMEOUT_COUNT        300 /* seconds before cert expration to start warning */
#define RSU_ACF_FILENAME_MAX_LENGTH  64  /* make future proof by extra large sizes */
#define RSU_ACF_MAX_COUNT            16  /* old are suppossed to be deleted and only 2 weeks allowed but extra large for future */

#if defined(MY_UNIT_TEST)
#define RSUHEALTH_GNSS_DEVIATION_WAIT_TIME 1
#else
#define RSUHEALTH_GNSS_DEVIATION_WAIT_TIME 45
#endif

#define EMMC_CHECK_MODULUS 3600 /* Check once an hour for now. */

/* For A & B: 0x1 to 0xA is 0 to 100%. 0xB is exceeded max. */
/* For EOL: 0x1 == Normal, 0x2 == warning 80%, 0x3 == urgent 90% done. */

#define GET_EMMC_HEALTH_A         "/usr/bin/mmc extcsd read /dev/mmcblk0 | grep TYP_A"
#define GET_EMMC_HEALTH_A_REPLY   "eMMC Life Time Estimation A [EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_A]: 0x%X"
#define GET_EMMC_HEALTH_B         "/usr/bin/mmc extcsd read /dev/mmcblk0 | grep TYP_B"
#define GET_EMMC_HEALTH_B_REPLY   "eMMC Life Time Estimation B [EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_B]: 0x%X"
#define GET_EMMC_HEALTH_EOL       "/usr/bin/mmc extcsd read /dev/mmcblk0 | grep EOL_INFO"
#define GET_EMMC_HEALTH_EOL_REPLY "eMMC Pre EOL information [EXT_CSD_PRE_EOL_INFO]: 0x%X"

#define EMMC_USE_MAX_THRESHOLD 95.0 /* Percent full */

#define GET_EMMC_USAGE         "df -h | grep mnt.emmc"
/* /dev/mmcblk0p3  6.9G  1.1G  5.5G  16% /mnt/emmc */
#define GET_EMMC_USAGE_REPLY   "/dev/mmcblk0p3  %s  %s  %s  %s /mnt/emmc"

/* Seconds to wait for shm update, else fail. RH850 only samples every 5 seconds. */
#define RH850_SHM_UPDATE_RATE 10
/* Give it this many seconds to boot. */
#define RH850_BOOT_WAIT 10
#endif /* RSUDIAGNOSTICS */
