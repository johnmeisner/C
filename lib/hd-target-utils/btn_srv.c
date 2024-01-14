/*
 *  Filename: btn_srv.c
 *  Purpose: Reset button service
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  05/16/19    BDOBRYANSKI   Initial release.
 *  08/29/19    BDOBRYANSKI   CfgMgr release.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <gpio/gpio.h>
#include "led_common.h"
#include "cfgmgrapi.c"

/* Defines */
#define IMX6_GPIO_PORT    "GPIO6_IO05"  /* RSU button */
#define GPIO_OUT_EN       "GPIO_OUT_EN"

/* usleep */
#define HALF_SECOND       500000   
#define THREE_SECOND      3000000

/* Globals */
static wulong64             gpio_output;
static gpio_connection_t    *gpio_connection;
static gpio_port_t          *gpio_out_en;

const char* options[] = {"reboot", "reset_ip", "reset", "factory_reset"};
static char* defaultIP = "192.168.2.77";


/*
 * Usage()
 * Print command-line usage instructions
 */
void Usage(void)
{
    printf("=== USAGE ===\n");
    printf("<path/to/>btn_srv -h -o <option>\n\n");
    printf("Options: \'reboot\' \'reset_ip\' \'reset\' \'factory_reset\'\n");
    printf("=== OPTIONS ===\n");
    printf("reboot: reboots the RSU.\n");
    printf("reset_ip: resets IP to default (%s).\n", defaultIP);
    printf("reset: resets /rwflash/configs/ to default, including IP.\n");
    printf("factory_reset: resets /rwflash/ and /mnt/cflash/ to default, including IP.\n");
    exit(0);
}

/*
 * Exit()
 */
void Exit(void)
{
    printf("Reset failed.\n");
    system("led_cmd red on");
    exit(1);
}

void Kill_rsuhealth(void)
{
    system("pid=$(pidin -f Aa | grep rsuhealth | grep -v grep | awk '{print $NF}'); [[ ! -z $pid ]] && kill -9 $pid");
}

/*
 * Check_Validity()
 * Compares option to valid options
 * Returns 0 if valid
 */
int Check_Validity(char* option)
{
    int i = 0;
    for (i=0; i < sizeof(options) / sizeof(options[0]); i++) {
        if (strcmp(option, options[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

/*
 * Reboot()
 * Executes shutdown call
 */
void Reboot(void)
{
    printf("System will reboot in 3 seconds.\n");
    system("led_cmd amber off; slay -Qf led_srv");  /* will turn off any led regardless of color */
    usleep(THREE_SECOND);
    system("shutdown");
}

/*
 * ResetIP()
 * Resets IP to default RSU IP
 */
void ResetIP(void)
{
    printf("Reset IP option selected.\n");
    Kill_rsuhealth(); /* Kill rsuhealth so led do not return to green during update */
    
    int systype = 0;  /* 0 for RSU and OBU 5910; nonzero for 5900 */
    int boolval;
    
    boolval = cfgUnlock();
    if (boolval != 0) { /* fail */
        Exit();
    }
    
    boolval = cfgNetworkStaticIP(defaultIP, systype);
    if (boolval != 0) { /* fail */
        Exit();
    }
    
    boolval = cfgCommit();
    if (boolval != 0) { /* fail */
        Exit();
    } else {
        printf("Reset IP successful. New IP is %s.\n", defaultIP);
        printf("System will reboot for settings to take effect.\n");
        Reboot();
    }
}

/*
 * Reset()
 * Resets /rwflash/configs to default, including IP
 */
void Reset(void)
{
    printf("Reset rwflash/configs/ option selected.\n");
    Kill_rsuhealth(); /* Kill rsuhealth so led does not return to green during update */
    system("led_cmd amber on");
    
    int boolval;
    
    boolval = cfgResetToDefault();
    if (boolval != 0) {
        Exit();
    } else {
        printf("Reset successful. New IP is %s.\n", defaultIP);
        printf("System will reboot for settings to take effect.\n");
        Reboot();
    }
}

/*
 * Reset()
 * Resets /rwflash/configs to default, including IP
 */
void FactoryReset(void)
{
    printf("Factory reset option selected.\n");
    Kill_rsuhealth(); /* Kill rsuhealth so led do not return to green during update */
    system("led_cmd amber on");
    
    int boolval;
    FILE *stream;
    char buf[1024];
    
    /* check if /mnt/cflash/factorysettings/ exists and is not empty */
    stream = popen("[ \"$(ls -A /mnt/cflash/factorysettings/mnt/cflash)\" ] && [ \"$(ls -A /mnt/cflash/factorysettings/rwflash)\" ] && echo true", "r");
    fgets(buf, sizeof(buf), stream);
    pclose(stream);
    if (strstr(buf, "true") == NULL) { /* fail */
        printf("/mnt/cflash/factorysettings/ does not have a backup stored.\n");
        Exit();
    }
    
    /* reset /mnt/cflash/      */
    /* delete everything from cflash except factorysettings directory */
    printf("Resetting /mnt/cflash/ ...\n");
    system("rm -r `find /mnt/cflash/* | grep -v factorysettings` > /dev/null 2>&1");
    system("cp -rpf /mnt/cflash/factorysettings/mnt/cflash/. /mnt/cflash > /dev/null 2>&1");
    
    /* reset /rwflash/ */
    printf("Resetting /rwflash/ ...\n");
    
    boolval = cfgResetToDefault();
    if (boolval != 0) { /* fail */
        Exit();
    } 
    printf("RSU IP has been reset to %s.\n", defaultIP);
    
    system("rm -r `find /rwflash/* | grep -v configs` > /dev/null 2>&1");
    system("cp -rpf /mnt/cflash/factorysettings/rwflash/. /rwflash > /dev/null 2>&1");
    
    /* ensure id_rsa permissions are set or unit will be inaccessible */
    system("chmod 600 /rwflash/root/.ssh/id_rsa");
    stream = popen("ls -l /rwflash/root/.ssh/id_rsa", "r");
    fgets(buf, sizeof(buf), stream);
    pclose(stream);
    if (strstr(buf, "-rw-------") == NULL) { /* fail */
        printf("id_rsa permissions failed to set to 600.\n");
        Exit();
    }
    
    printf("System will NOR flash for settings to take effect.\n");
    system("flash_nor_wsu5900a.sh -q -v -r -b");
}


wint32 main(wint32 argc, wint8 *argv[])
{
    int counter = 0;  	/* Polling time counter  */
    wsint8 c_arg;   	/* Flag arg      */
    
    /* 
     * Flag handling
     */
    while ((c_arg = getopt(argc, argv, "o:h")) != -1) {
        switch (c_arg) {
            case 'o': /* Option input */
                if (Check_Validity(optarg) == 0) {
                    if (strcmp(optarg, options[0]) == 0) {
                        Reboot();
                    } else if (strcmp(optarg, options[1]) == 0) {
                        ResetIP();
                    } else if (strcmp(optarg, options[2]) == 0) { 
                        Reset();
                    } else if (strcmp(optarg, options[3]) == 0) {
                        FactoryReset();
                    }
                    break;
                } else {
                    printf("WARNING: Invalid <option> entered. Check \'Options\' below.\n\n");
                    Usage();
                }
            case 'h':
                Usage();
            default:
                break;  /* continue to GPIO polling */
        }
    }
    
    /*
     * GPIO connection
     */
    
    gpio_connection = gpio_connect(NULL, 0);  /* Establish GPIO connection */ 
    
    if(gpio_connection == NULL) {
        fprintf(stderr, "gpio_connection failed.\n");
        gpio_disconnect(gpio_connection);
        exit(1);
    }
    
    gpio_out_en = gpio_open_named_port(gpio_connection, IMX6_GPIO_PORT, 0); /* Connect to named port */
    
    if(gpio_out_en == NULL) {
        fprintf(stderr, "gpio_open_named_port failed.\n");
        gpio_close(gpio_out_en);
        exit(1);
    }
    
    /* 
     * GPIO polling 
     */
    while(1) {
        gpio_in(gpio_out_en, &gpio_output, 0);  /* Grab GPIO output */
        
        if (gpio_output == 0) {     /* Increase counter for each half second button is pressed */
            counter++;
        }
        else if (gpio_output == 1 && counter > 0) {
            if (counter >= 3 && counter <= 5) {   /* REBOOT: Between 1.5 and 2.5 seconds */
                gpio_close(gpio_out_en);
                gpio_disconnect(gpio_connection);
                Reboot(); 
            }
            else if (counter >= 9 && counter <= 11) { /* RESET IP: Between 4.5 and 5.5 seconds */
                gpio_close(gpio_out_en);
                gpio_disconnect(gpio_connection);
                ResetIP();
            }   
            /* Longer counter time frame given to account for potential inaccuracy */
            else if (counter >= 18 && counter <= 23) {    /* RESET RWFLASH: Between 9 and 11.5 seconds */
                gpio_close(gpio_out_en);
                gpio_disconnect(gpio_connection);
                Reset();
            } 
            else if (counter >= 28 && counter <= 33) {    /* FACTORY RESET: Between 14 and 16.5 seconds */
                gpio_close(gpio_out_en);
                gpio_disconnect(gpio_connection);
                FactoryReset();
            }  
            else {
                counter = 0;
            }
        }
        
        usleep(HALF_SECOND);    /* Check GPIO output every 0.5 seconds */
    } 
}
