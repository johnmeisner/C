#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cfgshare.h"

/* updated the size for i2v params + comments; saving all of it */
#define MAXLINE 200

/* unmount and mount */
#define MOUNTCFGSRW \
    "umount /tmp/zk_inuse_mntpnt && mount -t vfat -o,rw /rwflash/zk_active_config_blob /tmp/zk_inuse_mntpnt"

#define MOUNTCFGSRO \
    "umount /tmp/zk_inuse_mntpnt && mount -t vfat -o,ro /rwflash/zk_active_config_blob /tmp/zk_inuse_mntpnt"

#define NUMITEMS(x) (sizeof(x)/sizeof((x)[0]))

typedef struct {
    char *file;
    char *param;
} i2vCfgs;

/* add any new params that must be saved here 
   WARNING: this opens the potential for rollback incompatibility 
   if the list changes or gets reordered; not a big problem but
   if going forward, remember that the prior release will have the
   order listed here BEFORE you make any changes; these values 
   will get saved into a common file so if items get removed you
   may want to think about creating a second file for the new
   params and you'll have to handle extra defunct items in the
   upgrade list of settings */
static i2vCfgs saveparams[] = {
    {"/rwflash/configs/amh.conf", "ImmediateIPFilter"},
    {"/rwflash/configs/amh.conf", "ImmediateIP"},
    {"/rwflash/configs/amh.conf", "ImmediatePort"},
    {"/rwflash/configs/amh.conf", "AMHForwardEnable"},
    {"/rwflash/configs/i2v.conf", "I2VUnifiedChannelNumber"},
    {"/rwflash/configs/i2v.conf", "I2VTransmitPower"},
    {"/rwflash/configs/i2v.conf", "I2VRadioType"},
    {"/rwflash/configs/scs.conf", "SRMFwdIP"},
    {"/rwflash/configs/scs.conf", "SRMFwdPort"},
    {"/rwflash/configs/scs.conf", "SRMFwdInterface"},
    {"/rwflash/configs/scs.conf", "SRMFwdRequireTTI"},
    {"/rwflash/configs/scs.conf", "LocalSignalControllerIP"},
    {"/rwflash/configs/scs.conf", "LocalSignalControllerPort"},
    {"/rwflash/configs/scs.conf", "LocalSignalControllerEnable"},
    {"/rwflash/configs/scs.conf", "LocalSignalControllerHW"},
    {"/rwflash/configs/scs.conf", "LocalSignalSNMPPort"},
    {"/rwflash/configs/scs.conf", "BypassSignalControllerNTCIP"},
    {"/rwflash/configs/scs.conf", "BypassYellowDuration"},
    {"/rwflash/configs/scs.conf", "BypassPhaseMask"},
    {"/rwflash/configs/spat16.conf", "BSMRxForward"},
    {"/rwflash/configs/spat16.conf", "BSMForwardIP"},
    {"/rwflash/configs/spat16.conf", "BSMForwardPort"},
    {"/rwflash/configs/spat16.conf", "SPATForwarding"},
    {"/rwflash/configs/spat16.conf", "SPATInteroperabilityMode"},
    {"/rwflash/configs/spat16.conf", "IntersectionID"},
    {"/rwflash/configs/srm_rx.conf", "SRMTXVehBasicRole"},
    {"/rwflash/configs/srm_rx.conf", "SRMPermissive"},
};

/* check for existence of SAVEPATH and if found apply params
   supported params:
      IP_FOR_STATIC_ADDRESS (startup.sh)
      NETMASK_FOR_STATIC_ADDRESS (startup.sh)
*/
static void apply(void)
{
    struct stat st;
    FILE *f;
    unsigned char i, j; 
    char remounted = 0, varformask = 0, buf[MAXLINE] = {0};
    char command[100] = {0};  /* maybe a waste, but maybe big enough for the future */
    struct in_addr dummy;   /* for validation only */

    memset(&st, 0, sizeof(st));
    if (!stat(SAVEPATH, &st)) {
        if (NULL != (f = fopen(SAVEPATH, "r"))) {
            /* for now just IP is supported; this can be more elaborae
               in the future (more params supported)*/
            if (fgets(buf, MAXLINE, f)) {
                /* have IP; just do a sanity check */
                /* eliminate any '\n' */
                i = strlen(buf);
                if (buf[i-1] == '\n') {
                    buf[i-1] = 0;
                }
                if (inet_aton(buf, &dummy)) {
                    /* IP is valid */
                    /* set up mount for rw -> BEWARE OF FUTURE NAME CHANGES */
                    if (!system(MOUNTCFGSRW)) {
                        remounted = 1;  /* so mounting only done once and for remounting later */
                        sprintf(command, "sed -i 's/\\(^IP_FOR_ST.*=\\).*/\\1%s/' /rwflash/configs/startup.sh", buf);
                        if (system(command)) {
                            varformask = 1;
                            printf("\nWARNING: IP address has just reset to default...\n");
                        }
                    }
                }
            }
            /* netmask; apply only if IP succeeded and also check that mount point was remounted for writing */
            if (!varformask && remounted && fgets(buf, MAXLINE, f)) {
                /* have netmask; value must be between 1 - 31 */
                /* eliminate any '\n' */
                i = strlen(buf);
                if (buf[i-1] == '\n') {
                    buf[i-1] = 0;
                }
                varformask = (unsigned char)strtoul(buf, NULL, 10);  /* repurposing varformask now and forcing val < 256 */
                if (varformask && (varformask < 31)) {
                    /* can apply the mask */
                    sprintf(command, "sed -i 's/\\(^NETMASK_FOR_ST.*=\\).*/\\1%d/' /rwflash/configs/startup.sh", varformask);
                    if (system(command)) {
                        printf("\nWARNING: NETMASK has just reset to default...\n");
                        /* let's consider the implications of the mask not getting properly set: the IP may still be
                           inaccessible; should we reset the IP? one would hope it's less common that the mask being
                           set improperly causes more trouble than switching to the default IP which may be a subnet
                           completely inaccessible on a private network */
                    }
                } /* and to the comment above, what if junk was written or if someone mucked with the file, same issue happens */
            }

            /* i2v params - no validation; just mash away; items ASSUMED to be in matching order */
            if (remounted) {
                printf("\n");
                for (i=0; i<NUMITEMS(saveparams); i++) {
                    if (!fgets(buf, MAXLINE, f)) {
                        break;
                    }
                    j = strlen(buf);
                    if (buf[j-1] == '\n') {
                        buf[j-1] = 0;
                    }
                    sprintf(command, "sed -i 's/\\(^ *%s.*=\\).*/\\1%s/' %s", saveparams[i].param, buf, saveparams[i].file);
                    if (system(command)) {
                        printf("WARNING: %s has just been reset to default from %s\n", saveparams[i].param, saveparams[i].file);
                    } else {
                        printf("NOTE: %s has just been restored to desired value in %s\n", saveparams[i].param, saveparams[i].file);
                    }
                }
            }

            if (remounted) {
                /* now mount RO */
                if (system(MOUNTCFGSRO)) {
                    printf("\nERROR: Please reboot system when convenient...\n");
                }
            }
            fclose(f);
        }
        /* last step; delete the file */
        remove(SAVEPATH);
    }
}

/*
    If adding settings in the future, add them on a separate line
    keep this file in sync

    Supported params saved:
    IP address (from /rwflash/configs/startup.sh)
    Net mask (from /rwflash/configs/startup.sh)
*/
static void preserve(char *arg)
{
    FILE *f, *pF;
    unsigned char i, j; 
    char proceed = 0, buf[MAXLINE] = {0}, cmd[400]; /* waste of space */

    if (!strcmp(arg, CFGAUTHVAL)) {
        /* preserve key settings; start by creating file */
        if (NULL != (f = fopen(SAVEPATH, "w+"))) {
            /* saving IP address FIRST */
            if (NULL != 
               (pF = popen("cat /rwflash/configs/startup.sh | grep ^IP_FOR_ST | sed 's/IP_FOR_ST.*=//'", "r"))) {
                fgets(buf, sizeof(char)*MAXLINE, pF);
                pclose(pF);
                /* add a '\n' if not already there; there should be space */
                i = strlen(buf);
                if ((buf[i-1] != '\n') && (i > MAXLINE - 2)) {   /* this is no longer valid as MAXLINE has changed */
                    /* problemo; IP address is too long (which means invalid); do not save */
                    printf("\nUh Oh: check /rwflash/configs/startup.sh IP; appears invalid!!\n");
                } else {
                    if (buf[i-1] != '\n') {
                        buf[i] = '\n';
                        i++;
                    }
                    if (i == fwrite(buf, sizeof(char), i, f)) {
                        proceed = 1;
                    }
                }
            }
            if (proceed) {
                proceed = 0;  /* reset; for the future */
                memset(buf, 0, sizeof(buf));   /* just in case */
                /* saving NETMASK SECOND */
                if (NULL != 
                   (pF = popen("cat /rwflash/configs/startup.sh | grep ^NETMASK_FOR_ST | sed 's/NETMASK_FOR_ST.*=//'", "r"))) {
                    fgets(buf, sizeof(char)*MAXLINE, pF);
                    pclose(pF);
                    /* add a '\n' if not already there; there should be space */
                    i = strlen(buf);
                    if ((i > 2) && !(i == 3 && (buf[2] == '\n'))) {
                        /* netmask can't be greater than 31 (2 chars); do not save */
                        printf("\nUh Oh: check /rwflash/configs/startup.sh NETMASK; appears invalid (%d)!!\n", i);
                    } else {
                        if (buf[i-1] != '\n') {
                            buf[i] = '\n';
                            i++;
                        }
                        /* for the future */
                        if (i == fwrite(buf, sizeof(char), i, f)) {
                            proceed = 1;
                        }
                    }
                }
            }

            /* i2v */
            i = NUMITEMS(saveparams);
            while (proceed && i) {
                memset(buf, 0, sizeof(buf));
                j = NUMITEMS(saveparams) - i;
                sprintf(cmd, "cat %s | grep \" *%s\" | sed 's/ *%s.*=//'", saveparams[j].file, saveparams[j].param, saveparams[j].param);
                if (NULL != (pF = popen(cmd, "r"))) {
                    fgets(buf, sizeof(char)*MAXLINE, pF);
                    pclose(pF);
                    /* save to repurpose j now */
                    j = strlen(buf);
                    if (buf[j-1] != '\n') {
                        buf[j] = '\n';
                        j++;
                    }
                    if (j != fwrite(buf, sizeof(char), j, f)) {
                        printf("\nWARNING: failure in preserving config param; stopping remaining params\n");
                        proceed = 0;
                    }
                } else {
                    printf("\nWARNING: failure in preserving config param %s (%s); will continue with remaining params\n", saveparams[j].param, saveparams[j].file);
                }
                i--;
            }
            fclose(f);
        }
    }
}


int main(int argc, char *argv[])
{
    signed char c;

    while ((c = getopt(argc, argv, "ap:v")) != -1) {
        switch (c) {
            case 'a':
                apply();
                break;
            case 'p':
                preserve(optarg);
                break;
            case 'v':
                printf("Version: 1.0\n");
                break;
            default:
                break;
        }
    }

    return 0;
}

