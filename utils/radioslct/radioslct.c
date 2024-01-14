#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/*
 NOTE: this utility is patterned after the new_gpio_exp.sh that was intended
 to be used to enable the RF switch.  The instructions provided for the use
 of that script were to just call the script with the values to set.  Hence,
 no check of the gpio is performed (i.e. init from the script); the system
 is assumed to be inited.
*/

/* these magic numbers correspond to the memory mapped values for the pins to be asserted */
#define CV2XPIN "176"
#define DSRCPIN "190"

/* commands */
#define ENABLEDSRC "echo 1 > /sys/class/gpio/gpio" DSRCPIN "/value; echo 0 > /sys/class/gpio/gpio" CV2XPIN "/value"
#define ENABLECV2X "echo 1 > /sys/class/gpio/gpio" CV2XPIN "/value; echo 0 > /sys/class/gpio/gpio" DSRCPIN "/value"
#define SETDEFAULT "echo 0 > /sys/class/gpio/gpio" CV2XPIN "/value; echo 0 > /sys/class/gpio/gpio" DSRCPIN "/value"

/* unmount and mount */
#define MOUNTCFGSRW \
    "umount /tmp/zk_inuse_mntpnt && mount -t vfat -o,rw /rwflash/zk_active_config_blob /tmp/zk_inuse_mntpnt"

#define MOUNTCFGSRO \
    "umount /tmp/zk_inuse_mntpnt && mount -t vfat -o,ro /rwflash/zk_active_config_blob /tmp/zk_inuse_mntpnt"


/* enough for 1 character value, \n and null term */
#define BUFLEN 3

/* value needs to be either 0 or 1; if correct, update i2v.conf */
static void set(char *valstring)
{
    char val, buf[100] = {0};    /* a waste of space probably */

    /* technically, should check for an error; if there is an error
       val gets set to 0; is this ok? maybe better than nothing */
    val = (char)strtoul(valstring, NULL, 10);
    if (val > 1) {
        /* invalid */
        printf("\nERROR: invalid input for setting RF switch: %s\n", valstring);
        return;
    }

    if (!system(MOUNTCFGSRW)) {
        /* now set val */
        sprintf(buf, "sed -i 's/\\(^PLATFORM_USE_RFSW.*= *\\).*/\\1%d/' /rwflash/configs/i2v.conf", val);
        if (system(buf)) {
            printf("\nERROR: failed to set RF switch configuration value...\n");
        }
        if (system(MOUNTCFGSRO)) {
            printf("\nERROR: internal error...please reboot...\n");
        }
    }
}

/* check in i2v.conf for the I2VRadioType and PLATFORM_USE_RFSW for decision
   to set radio mode (dsrc vs cv2x + use of RF switch)
   can also be used to set the PLATFORM_USE_RFSW value to either 0 or 1
*/
static void radioctl(void)
{
    FILE *pF;
    char val, buf[BUFLEN] = {0};

    /* first check if switch should be used (PLATFORM_USE_RFSW) */
    if (NULL !=
       (pF = popen("cat /rwflash/configs/i2v.conf | grep PLATFORM_USE_RFSW | awk '{print $3}'", "r"))) {
        fgets(buf, sizeof(char)*BUFLEN, pF);
        pclose(pF);

        /* eliminate any '\n'; assume one character for value */
        if (buf[1] == '\n') {
            buf[1] = 0;
        }

        val = (char)strtoul(buf, NULL, 10);
        if (val) {
            /* find out if using cv2x or not */
            if (NULL != 
               (pF = popen("cat /rwflash/configs/i2v.conf | grep I2VRadioType | awk '{print $3}' | sed 's/;//'", "r"))) {
            } else {
                printf("\nERROR: unable to read RF switch configuration...RF switch NOT SET!\n");
                printf("Consider stopping radio transmissions...\n");
                return;
            }
            fgets(buf, sizeof(char)*BUFLEN, pF);
            pclose(pF);
            /* eliminate any '\n'; assume one character for value */
            if (buf[1] == '\n') {
                buf[1] = 0;
            }
            /* repurpose val */
            val = (unsigned char)strtoul(buf, NULL, 10);
            if (!val) {
                /* DSRC mode */
                if (system(ENABLEDSRC)) {
                    printf("\nWARNING: problem setting DSRC mode to RF switch; state unknown...\n");
                    printf("Consider stopping radio transmissions...\n");
                }
            } else {
                /* C-V2X mode */
                if (system(ENABLECV2X)) {
                    printf("\nWARNING: problem setting CV2X mode to RF switch; state unknown...\n");
                    printf("Consider stopping radio transmissions...\n");
                }
            }
        } else {
            /* do not use switch; set values to default (just in case) */
            if (system(SETDEFAULT)) {
                printf("\nWARNING: problem setting defaults to RF switch; state unknown...\n");
                printf("Consider stopping radio transmissions...\n");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    signed char c;

    while ((c = getopt(argc, argv, "s:v")) != -1) {
        switch (c) {
            case 's':
                set(optarg);
                return 0;
            case 'v':
                printf("\nVersion: 1.0\n");
                return 0;
            default:
                break;
        }
    }

    /* reaching here means set up the radio */
    radioctl();

	return 0;
}

