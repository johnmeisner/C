/*
*  Filename: custctl.c
*  Purpose: Utility to allow control of select apps
*
*  Copyright (C) 2020 DENSO International America, Inc.
*
*  The purpose of this utility is to provide a 
*  customer with the capability of turning on or off
*  select features.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  10-17-20    JJG        Initial release.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define QPARC "/mnt/qparc"
#define BLOBFILE   "/mnt/cflash/.ihr"

/* requires root permission to execute */

int rwunlock(void)
{
    /* check if this is a cfgmgr platform */
    if (!system("whence dc >/dev/null")) {
        /* magic commands -- NEVER share */
        if (!system("umount " QPARC)) {
            return system("mount -tdos " BLOBFILE " " QPARC);
        }
    }

    return -1;
}

int rwlock(void)
{
    /* check if this is a cfgmgr platform */
    if (!system("whence dc >/dev/null")) {
        if (!system("umount " QPARC)) {
            return system("mount -r -tdos " BLOBFILE " " QPARC);
        }
    }

    return -1;
}

/* enableDisable = 0 to disable */
int security(int enableDisable, int force)
{
    /* i2v.conf:
        I2VSecurityEnable = 1 (enable)
        I2VMAPSecPSID = 0xE0000017 (for Florida DOT security); 0x8002 = no security

      ONLY if not present AND no force (otherwise customer needs to manage)
      scms-enabled.sh becomes /rwflash/customer/scms.sh (enabled)
      scms-disabled.sh becomes /rwflash/customer/scms.sh (disabled)
    */

    if (enableDisable) {
        if (!system("sed -i 's/\\(I2VSecurityEnable.*\\)[0-9]\\(;.*\\)/\\11\\2/' /rwflash/configs/i2v.conf")) {
            if (!system("sed -i 's/\\(I2VMAPSecPSID.*\\)0x[A-F0-9]*\\(;.*\\)/\\10xE0000017\\2/' /rwflash/configs/i2v.conf")) {
                if (force) {
                    return system("cp /scripts/scms-enabled.sh /rwflash/customer/scms.sh");
                } else {
                    return system("if [ ! -e /rwflash/customer/scms.sh ]; then cp /scripts/scms-enabled.sh /rwflash/customer/scms.sh; fi");
                }
            }
        }
    } else {
        if (!system("sed -i 's/\\(I2VSecurityEnable.*\\)[0-9]\\(;.*\\)/\\10\\2/' /rwflash/configs/i2v.conf")) {
            if (!system("sed -i 's/\\(I2VMAPSecPSID.*\\)0x[A-F0-9]*\\(;.*\\)/\\10x8002\\2/' /rwflash/configs/i2v.conf")) {
                if (force) {
                    return system("cp /scripts/scms-disabled.sh /rwflash/customer/scms.sh");
                } else {
                    return system("if [ ! -e /rwflash/customer/scms.sh ]; then cp /scripts/scms-disabled.sh /rwflash/customer/scms.sh; fi");
                }
            }
        }
    }
    return -1;
}

/* enableDisable = 0 to disable */
int advsnmp(int enableDisable)
{
    if (enableDisable) {
        return system("sed -i '/bin\\/sh/a\\ADVANCED_SNMP=1' /rwflash/configs/startup.sh");
    } else {
        return system("sed -i '/ADVANCED_SNMP=/d' /rwflash/configs/startup.sh");
    }

    return -1;
}

/* enableDisable = 0 to disable */
int gui(int enableDisable)
{
    if (enableDisable) {
        return system("sed -i '/bin\\/sh/a\\ENABLE_GUI=1' /rwflash/configs/startup.sh");
    } else {
        return system("sed -i '/ENABLE_GUI=/d' /rwflash/configs/startup.sh");
    }

    return -1;
}

void rungui(void)
{
    if (system("pidin | grep bozo >> /dev/null")) {
        system("rm -f /tmp/guistart.sh && echo #!/bin/sh >> /tmp/guistart.sh");
        system("echo ENABLE_GUI=1 >> /tmp/guistart.sh");
        system("sed -ne '/Web config/,$ p' /rwflash/configs/startup.sh >> /tmp/guistart.sh");
        system("chmod +x /tmp/guistart.sh");
        system("/tmp/guistart.sh &");
        system("rm /tmp/guistart.sh");
    }
}

void termgui(void)
{
    system("kill -15 `pidin -f Aa | grep conf_ | grep -v grep | awk '{print $NF}'` >>/dev/null");
}

/**
* main()
*
* Main function
*
* returns 0 (success); -1 (failure)
*
*/
int main(int argc, char *argv[])
{ 
#define TERMGUI         0x01
#define RUNGUI          0x02
#define ENABLEGUI       0x04
#define DISABLEGUI      0x08
#define ENABLESECURITY  0x10
#define DISABLESECURITY 0x20
#define ENABLESNMP      0x40
#define DISABLESNMP     0x80

    signed char c;
    int force = 0, option = 0;

    /* check if I'm root; if not exit */
    if (system("id -un | grep \"^root$\" >> /dev/null")) {
        /* not the root user */
        return -1;
    }

    /* now check args */
    while ((c = getopt(argc, argv, "DdfGgSsWw")) != -1) {
        switch (c) {
            case 'D':
                if (!(option & DISABLESECURITY)) {
                    option |= ENABLESECURITY;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'd':
                if (!(option & ENABLESECURITY)) {
                    option |= DISABLESECURITY;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'f':
                force = 1;
                break;
            case 'G':
                if (!(option & DISABLEGUI)) {
                    option |= ENABLEGUI;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'g':
                if (!(option & ENABLEGUI)) {
                    option |= DISABLEGUI;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'S':
                if (!(option & DISABLESNMP)) {
                    option |= ENABLESNMP;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 's':
                if (!(option & ENABLESNMP)) {
                    option |= DISABLESNMP;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'W':
                if (!(option & TERMGUI)) {
                    option |= RUNGUI;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            case 'w':
                if (!(option & RUNGUI)) {
                    option |= TERMGUI;
                } else {
                    printf("ERROR: conflicting input received; please try again\n");
                }
                break;
            default:
                break;
        }
    }

    /* now do crap */
    if (option & RUNGUI) {
        rungui();
        option &= ~RUNGUI;
    }
    if (option & TERMGUI) {
        termgui();
        option &= ~TERMGUI;
    }
    if (option && !rwunlock()) {
        if (option & ENABLEGUI) {
            gui(1);
        }
        if (option & DISABLEGUI) {
            gui(0);
        }
        if (option & ENABLESNMP) {
            advsnmp(1);
        }
        if (option & DISABLESNMP) {
            advsnmp(0);
        }
        if (option & ENABLESECURITY) {
            security(1, force);
        }
        if (option & DISABLESECURITY) {
            security(0, force);
        }
        if (rwlock()) {
            printf("Unfortunately system is in unreliable state; please reboot\n");
        }
    } else if (option) {
        printf("ERROR...Please retry\n");
    }

    return 0;
}

