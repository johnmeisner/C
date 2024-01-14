/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: mdm9150update.c                                                  */
/* Purpose: updates the MDM9150 firmware                                      */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20220106 SB initial revision                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>


#define FW_DIR_SIZE 100 
#define BUFFSIZE 100
#define CMDSIZE 100
#define DEFAULT_FW_DIR "/rwflash/customer/v00.03.01.00_py3"


static void usage()
{
    printf("mdm9150update programs the MDM9150 module. The default location for the firmware image is in %s. This can be overwritten. \n\n", DEFAULT_FW_DIR);
    printf("Usage: mdm9150update [-d <dir> | -r | -h ]\n\n");
    printf("All parameters are OPTIONAL, where: \n");
    printf("\t-d <dir> = Directory of firmware package. \n");
    printf("\t-r       = Reboot if the programming process sucessfully completes. \n");
    printf("\t-h       = Prints this help message.\n\n");
}


/*----------------------------------------------------------------------------*/
/* Explanation: Updates the MDM9150 firmware. Uses the firmware in the        */
/* default directory, unless directory is supplied by the user.               */
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char fwdir[FW_DIR_SIZE];
    char buf[BUFFSIZE];
    char cmd[CMDSIZE];
    FILE *pf;
    char *adb_id;
    int i;
    signed char c;
    unsigned char flashing_successful = 0;
    unsigned char doreboot = 0;

    /* Get firmware directory and determine if reboot is automatic*/
    snprintf(fwdir, FW_DIR_SIZE, DEFAULT_FW_DIR);
    while ((c=getopt(argc, argv, "d:rh")) != -1) {
        switch (c) {
            case 'd': /* directory */
                strncpy(fwdir, optarg, FW_DIR_SIZE);
                break;
            case 'r': /* when specified do automatic reboot */
                doreboot = 1;
                break;
            case 'h': /* help */
                usage();
                return 0;
            default:
                break;
        }
    }
    
    
    /* Verify the directory exists: TODO: verify other files or permissions? */
    if (access(fwdir, F_OK)) {
        printf("Error: firmware directory %s does not exist. \n", fwdir);
        return -1;
    }
    

    /* Stop RSU Health and CV2X Services */
    if (system("rsuhealth -It")) {
        printf("Error: unknown error pausing rsuhealth. Unable to flash MDM9150. \n");
        return -1;
    }
    if (system("systemctl stop cv2x")) {
        printf("Error: unknown error stopping cv2x operation. Unable to flash MDM9150. \n");
        return -1;
    }
   

    /* Start ADB */
    if (system("adb devices")) {
        printf("Error: unknown error starting adb. Unable to flash MDM9150. \n");
        return -1;
    }
    

    /* Get the ADB ID of the device */
    if (NULL == (pf = popen("adb devices", "r"))) {
        printf("Error: unable to find any adb devices. \n");
        return -1;
    }
    for (i=0; i<=1; i++) {  //row 0 is the header, get row 1
        fgets(buf, BUFFSIZE, pf);
    } 
    pclose(pf); 
    adb_id = strtok(buf, " \t"); // the id is the first substr separated by whitespace delimiter      
    

    /* Add the device to test.txt */
    snprintf(cmd, CMDSIZE, "adb -s %s shell \"uname -a \" > %s/test.txt", adb_id, fwdir);
    if (system(cmd)) {
        printf("Error: Unable to write the adb device information to device list. \n");
        return -1;
    }


    /* Verify the adb device in the device list is the MDM9150 */
    snprintf(cmd, CMDSIZE, "cat %s/test.txt | grep mdm9150", fwdir);
    if (NULL == (pf = popen(cmd, "r"))) {
        printf("Error: Unable to read adb device list. \n");
        return -1;
    }
    if (fgetc(pf) == EOF) {
        printf("Error: unable to verify a valid MDM9150 device. \n");
        pclose(pf);
        return -1;
    }
    pclose(pf);
    printf("Device verified. \n");
    

    /* Put MDM9150 into fastboot "bootloader" mode */
    /* If there are any errors after this point, the device MUST be rebooted. */
    snprintf(cmd, CMDSIZE, "adb -s %s reboot bootloader", adb_id);
    if (system(cmd)) {
        printf("Error: Unable to put MDM9150 into fastboot \"bootloader\" mode. \n");
        return -1;
    }


    /* Unbind PCI link */
    if (system("echo 1 > /sys/bus/pci/devices/0000\\:04\\:00.0/remove")) {
        printf("Error: Unable to bind PCI link. \n");
        printf("Reboot RSU now to resume regular operation. \n");
        return -1;
    }


    /* Execute python scripts */
    snprintf(cmd, CMDSIZE, "python3 %s/common/build/fastboot_complete.py --pf=asic", fwdir);
    printf("Updating MDM9150 firmware... \n");
    if (NULL == (pf = popen(cmd, "r"))) {
        printf("Error: Unable to complete steps needed to update firmware. \n "
               "Reboot RSU now to resume regular operation. \n"); 
        return -1;
    }
    while (fgets(buf, BUFFSIZE, pf)) {
        printf("%s", buf);
        if (strstr(buf, "FLASHING SUCCESSFUL!")){
            flashing_successful = 1;
        }
    }
    pclose(pf);


    /* Print sucess/failure message */
    if (flashing_successful) {
        printf("Update Succesful! \n");
        if (doreboot) {
            printf("Rebooting system now...\n\n");
            sleep(2); /* two seconds to view message */
            system("reboot");
        }
        else {
            printf("Reboot device to resume operation. \n\n");
            return 0;
        }
    }
    else {
        printf("ERROR: Unable to update MDM9150. \nReboot device to resume operation. \n\n");
        return -1;
    }

}

