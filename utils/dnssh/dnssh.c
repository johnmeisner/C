/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: dnssh.c                                                          */
/*                                                                            */
/* Copyright (C) 2023 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SSHDCONF      "/mnt/rwflash/etc/ssh/sshd_config"
#define ETCSSHDCONF   "/etc/ssh/sshd_config"

int main(int argc, char* argv[]) {
    
    // Check if the new port parameter is provided
    if (argc != 2) {
        printf("Usage: %s <new_port>\n", argv[0]);
        return 1;
    }

    // New SSH port
    int new_port = atoi(argv[1]);

    // Check if the SSH port is already set to the new port
    FILE* fp = fopen(SSHDCONF, "r");
    if (fp == NULL) {
        perror("Error opening sshd_config file");
        return 1;
    }

    int current_port = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Port", 4) == 0) {
            current_port = atoi(strchr(line, ' ') + 1);
            break;
        }
    }

    fclose(fp);

    // Check if the SSH port is already set to the new port
    if (current_port == new_port) {
        printf("SSH port is already set to %d\n", new_port);
        return 0;
    }

    // Create the sed command to update the SSH port in the sshd_config file
    char sed_command[256];
    sprintf(sed_command, "sed -i 's/^#*\\s*Port .*/Port %d/' %s", new_port, SSHDCONF);

    if (setuid(0)) {
        printf("Error: Root privileges required for changing SSH port.\n");
        return 1;
    }
    
    // Use sed to update the SSH port in the sshd_config file
    if (system(sed_command) != 0) {
        printf("Error: Failed to update SSH port in sshd_config file.\n");
        return 1;
    }

    // Sync the file system to ensure changes are written to disk
    sync();

    /* 20231018: using sed to modify the sshd_config file on rwflash breaks the bind mount;
       need to unmount the etc version of the file and then remount */
    if (system("umount " ETCSSHDCONF)) {
        printf("Error: Failed to restore SSH configuration step 1.\n");
        return 1;
    } else {
        if (system("mount -o bind " SSHDCONF " " ETCSSHDCONF)) {
            printf("Error: Failed to restore SSH configuration step 1.\n");
            return 1;
        }
    }

    // Restart the SSH service
    system("pkill sshd");
    system("/usr/sbin/sshd");

    printf("SSH port has been changed to %d\n", new_port);
    return 0;
}
