#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "dn_types.h"
//#include "wsu_sharedmem.h"
#include "cfgshare.h"
//#include "rsuhealth.h"
#include "i2v_util.h"

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV
#endif
#define MY_NAME        "densonor"

/* 20210215: add support for utility to save key cfgs through update */

/* first nor image did not have multiple banks */
/* disable this macro for microSD or eMMC boot for flashing */
#define MULT_BANKS

/* looking for a file densonor.tgz; if not found punt */
#define DENSONOR "/rwflash/densonor.bz2"
#define TMPDIR "/rwflash/.tmp-dn-hparc"
#define SEAL "sea.l"
#define NORPW "DNMasterPassDNFWT**()0"

#define MAXPARTS 6

#define MAGICPART "/dev/mtd8"
#ifdef MULT_BANKS
#define MAGICVAL  "0"
#define MAGICFAILURE 2

#ifdef DENSO_OFFICIAL
#define PUBLICKEY "/etc/publickey.pem"
#endif /* DENSO_OFFICIAL */

static int bankb = 0;

#endif /* MULT_BANKS */

/* set to 1 to force flash even if partition has matching image */
static char mustdo = 0;

static char *nornames[] = {
    "uboot",
    "uenv",
    "dtb",
    "kernel",
    "rootfs",
    "user",
};

/* platform variations */
typedef enum {
    MD, 
    MP,
} platform_e;

int flash(int idx)
{
    char cmd[200];
    struct stat st;

#ifdef MULT_BANKS
    int bankidx;

    /* the magic numbers below require understanding of the partition
       table depicted below
           partition | usage
           -----------------
               0        u-boot
               1        u-boot environment (unused in NOR)
               2        device tree (for kernel 1)
               3        kernel #1
               4        rootfs #1
               5        device tree (for kernel 2)
               6        kernel #2
               7        rootfs #2
               8        user space (magic partition)
               9        system space (do not touch partition)
    */
    if (bankb) {
        /* bankb means the second partition should be used */
        switch (idx) {
            case 2:
                bankidx = 5;
                break;
            case 3:
                bankidx = 6;
                break;
            case 4:
                bankidx = 7;
                break;
            default:
                bankidx = idx;
                break;
        }
    } else {
        bankidx = idx;
    }
#endif /* MULT_BANKS */
    memset(&st, 0, sizeof(st));
    sprintf(cmd, TMPDIR "/%s", nornames[idx]);
    if (stat(cmd, &st)) {
		printf("\nFAILURE in retrieving file status (%s)\n", nornames[idx]);
		return -1;
	}

    /* if allowed, compare first */
    if (!mustdo) {
#ifdef MULT_BANKS
        sprintf(cmd, "cmp " TMPDIR "/%s /dev/mtd%d -n %lu >>/dev/null", nornames[idx], bankidx, st.st_size);
#else
        sprintf(cmd, "cmp " TMPDIR "/%s /dev/mtd%d -n %lu >>/dev/null", nornames[idx], idx, st.st_size);
#endif
        if (!system(cmd)) {
            printf("\nMatch found for existing image and firmware package; skip flashing %s\n", nornames[idx]);
            return 0;
        }
    }

    printf("\nAttempting to program flash for %s\n", nornames[idx]);
#ifdef MULT_BANKS
    sprintf(cmd, "flash_erase /dev/mtd%d 0 0", bankidx);
#else
    sprintf(cmd, "flash_erase /dev/mtd%d 0 0", idx);
#endif /* MULT_BANKS */
    if (!system(cmd)) {
        printf("\nProgramming %s...\n", nornames[idx]);
#ifdef MULT_BANKS
        sprintf(cmd, "dd if=" TMPDIR "/%s of=/dev/mtd%d bs=4096 >>/dev/null", nornames[idx], bankidx);
#else
        sprintf(cmd, "dd if=" TMPDIR "/%s of=/dev/mtd%d bs=4096 >>/dev/null", nornames[idx], idx);
#endif /* MULT_BANKS */
        if (!system(cmd)) {
			/* verify */
			printf("Verifying %s...\n", nornames[idx]);
#ifdef MULT_BANKS
            sprintf(cmd, "cmp " TMPDIR "/%s /dev/mtd%d -n %lu >>/dev/null", nornames[idx], bankidx, st.st_size);
#else
            sprintf(cmd, "cmp " TMPDIR "/%s /dev/mtd%d -n %lu >>/dev/null", nornames[idx], idx, st.st_size);
#endif
            if (!system(cmd)) {
                printf("Done!\nSuccess flashing %s\n", nornames[idx]);
                return 0;
			}
			printf("\nVerification failure!");
        }
        printf("\nFAILURE in programming flash: %s\n", nornames[idx]);
    } else {
        printf("\nFAILURE in erasing flash for %s\n", nornames[idx]);
    }

    return -1;
}

static void usage(char *name)
{
    printf("\n%s programs the non-active bank of firmware in preparation to switch bank for new image.", name);
    printf("  The default location for the firmware image is in /rwflash.  This can be overridden.");
    printf("\n\nUsage: %s [ -F <dirent> | -f | -H | -r | -V ]\n", name);
    printf("\nAll parameters are OPTIONAL, where:\n");
    printf("\t-F <dirent> =\tFile name including path to location of firmware.\n");
    printf("\t-f =\tForce flashing everything even if the partition is\n\t\talready programmed with matching firmware.\n");
    printf("\t-H =\tThis help message.\n");
    printf("\t-r =\tReboot if the flash process successfully completes.\n");
    printf("\t-V =\tVersion information.\n\n");
}

int verify_platform(void)
{

    FILE *pf; 
    char buf[100];
    char hw[100];
    int  platform=-1; 
        
    /* Check Hardware Platform */ 
    if (!system("/usr/src/scripts/hw_get_ver.sh>/dev/null 2>/dev/null") && (NULL != (pf = popen("/usr/src/scripts/hw_get_ver.sh", "r")))) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), pf);
        pclose(pf);

    } else if (!system("/usr/src/wnc/scripts/get_hw_ver.sh>/dev/null 2>/dev/null") && (NULL != (pf = popen("/usr/src/wnc/scripts/get_hw_ver.sh", "r")))) {

        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), pf);
        pclose(pf);
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: unable to acess hardware platform information.\n");
        if(NULL != pf) {
            pclose(pf);
        }
        return -1;
    }

    if (1 != sscanf(buf, "HW Version: %s\n", hw)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: unable to parse hardware platform.\n");
        return -1;
    }
    
    if (!strncmp(hw, "EVT", 3)) {
        platform=MD; // EVT == MD
    } else if (!strncmp(hw, "DVT", 3)) { //TODO add other MP hardware platorms as needed
        platform=MP; //DVT == MP
    } else { 
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: unable to determine if hardware platform is MD or MP.\n");
        return -1;
    }
    
    /* Check that firmware is supported by platform type */
    if (MD==platform && 0==system("grep MD " TMPDIR "/" SEAL " >>/dev/null")) {
        /* we are MD and MD is supported */
        return 0;
    } else if (MP==platform && 0==system("grep MP " TMPDIR "/" SEAL " >>/dev/null")) {
        /* we are MP and MP is supported*/ 
        return 0;
    }
    return -1;
}

int main(int argc, char *argv[])
{
    int i;
    char force = 0, skipPlatformCheck=0, reboot = 0, cmd[400], file[200] = {0};
    signed char c;

    sprintf(file, DENSONOR);
    while ((c = getopt(argc, argv, "F:fHh:p:rV")) != -1) {
        switch (c) {
            case 'F':    /* file + path */
                memset(file, 0, sizeof(file));
                strcpy(file, optarg);
                break;
            case 'f':    /* force flashing everything even if partition already programmed */
                printf("\nNOTE: user option specified: forcing flash of all partitions\n");
                mustdo = 1;
                break;
            case 'H':
                usage(argv[0]);
                return 0;
            case 'h':    /* bypass hash check */
                /* force option specified; message may need to change for future force options */
                if (!strncmp(optarg, NORPW, strlen(NORPW))) {
                    printf("\nNOTE: user option specified: hash verification ignored!\n");
                    force = 1;
                }
                break;
            case 'p':    /* bypass platform check */
                /* force option specified; message may need to change for future force options */
                if (!strncmp(optarg, NORPW, strlen(NORPW))) {
                    printf("\nNOTE: user option specified: platform verification ignored!\n");
                    skipPlatformCheck = 1;
                }
                break;
            case 'r':
                reboot = 1;
                break;
            case 'V':    /* version */
                printf("\nDENSO RSU-5940 NOR Flash Utility version 1.00\n\n");
                return 0;
            default:
                printf("\nInvalid option specified: -%c\n", c);
                break;
        }
    }

    i2vUtilEnableDebug(MY_NAME);
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)) { /* LOGMGR should be up by now, should be no need to retry. */
        printf("%s| Enable syslog failed.\n",MY_NAME);
    }

    sprintf(cmd, "[ -e %s ]", file);
    if (system(cmd)) {
        /* file doesn't exist */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sorry...expected input not found: [%s]\n", file);
        return -1;
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSUHEALTH_NOR_FLASHING_ON.\n");

    printf("\nWelcome to the DENSO RSU-5940 NOR Flash Utility...wait for environment setup; this will take a few minutes...\n");
    fflush(stdout);
    /* unpackage */
    sprintf(cmd, " mkdir -p " TMPDIR " && cd " TMPDIR " && rm -rf ./* && tar xjf %s", file);
    if (system(cmd)) {
        /* failed */
        return -1;
    }

    if (!force) {
        printf("\nPreparing to start flashing process...\n");
        fflush(stdout);
        /* verify everything before proceeding (unless user provides force) */
#ifdef DENSO_OFFICIAL
        sprintf(cmd, "[ -e " PUBLICKEY " ]");
        if (system(cmd)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: missing public key; unable to validate sigantures; not proceeding with upgrade\n");
            return -1;
        }
#endif /*DENSO_OFFICIAL*/ 
        /* special file - sea.l for evaluating package content and platform */
        if (system("[ -e " TMPDIR "/" SEAL " ]")) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: bad firmware package; validation missing\n");
            return -1;
        }
#ifdef DENSO_OFFICIAL
        if (system("[ -e " TMPDIR "/" SEAL ".sign ]")) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: bad firmware package; validation signature missing\n");
            return -1;
        }
        if (system("openssl dgst -sha1 -verify " PUBLICKEY " -signature " TMPDIR "/" SEAL ".sign " TMPDIR "/" SEAL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Unable to validate signature for platform; not proceeding with upgrade\n");
            return -1;
        }
#endif /* DENSO_OFFICIAL */
        if (system("grep 5940 " TMPDIR "/" SEAL " >>/dev/null")) {
            /* we are 5940; not found in the SEAL; invalid firmware */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Invalid firmware package for this platform; not proceeding with upgrade\n");
            return -1;
        }
        if (system("grep nor " TMPDIR "/" SEAL " >>/dev/null")) {
            /* we are expecting NOR image; not found in the SEAL; invalid firmware */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Invalid firmware package for this medium; not proceeding with upgrade\n");
            return -1;
        }

        for (i=0; i<MAXPARTS; i++) {
            sprintf(cmd, "[ -e " TMPDIR "/%s ]", nornames[i]);
            if (!system(cmd)) {
                sprintf(cmd, "[ -e " TMPDIR "/%s.md5 ]", nornames[i]);
                if (system(cmd)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: missing hash file; not proceeding with upgrade\n");
                    return -1;
                }
                sprintf(cmd, "grep \"`cat " TMPDIR "/%s.md5`\" " TMPDIR "/" SEAL " >>/dev/null", nornames[i]);
                if (system(cmd)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: mismatch on hash for %s for platform validation; not proceeding with upgrade\n", nornames[i]);
                    return -1;
                }
                sprintf(cmd, "[ \"`md5sum " TMPDIR "/%s | awk '{print $1}'`\" == \"`cat " TMPDIR "/%s.md5`\" ]", nornames[i], nornames[i]);
                if (system(cmd)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: mismatch on hash for %s; possible file corruption; not proceeding with upgrade\n", nornames[i]);
                    return -1;
                }
#ifdef DENSO_OFFICIAL    
                sprintf(cmd, "[ -e " TMPDIR "/%s.sign ]", nornames[i]);
                if (system(cmd)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: firmware file not signed; not proceeding with upgrade\n");
                    return -1;
                }
                sprintf(cmd, "openssl dgst -sha1 -verify %s -signature " TMPDIR "/%s.sign " TMPDIR 
                "/%s", PUBLICKEY, nornames[i], nornames[i]);
                if (system(cmd)) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Unable to validate signature; not proceeding with upgrade\n");
                    return -1;
                }
#endif /*DENSO_OFFICIAL*/
            }
        }
    }
    
    if (!skipPlatformCheck) {
        if (-1 == verify_platform()) {
            /* Verify the firmware matches the platform, either Monitor Deployment or Mass Production. */
            printf("ERROR: Unable to verify firmware package compatibility with hardware platform; not proceeding with upgrade. \n");
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: Unable to verify firmware package compatibility with hardware platform; not proceeding with upgrade. \n");
            return -1;
        }
    }
    

#ifdef MULT_BANKS
    /* determine bank to flash; partitions are either mtdblock5 or mtdblock8; bankb set when mounted partition is 5 */
    bankb = !system("[ \"`mount | grep jffs2 | awk '{print $1}' | sed 's/.*\\([0-9]\\)/\\1/'`\" == \"4\" ]");
#endif /* MULT_BANKS */

    /* activate amber led */
    system("/usr/local/bin/rsuhealth -las");

    printf("\nStarting flashing process...WARNING: DO NOT POWER OFF OR REBOOT UNTIL COMPLETE!!!\n");
    fflush(stdout);
    for (i=0; i<MAXPARTS; i++) {
        sprintf(cmd, "[ -e " TMPDIR "/%s ]", nornames[i]);
        if (!system(cmd)) {
            /* file exists, program */
            if (flash(i)) {
                /* failure */
                system("cd - > /dev/null 2>&1");
                system("rm -rf " TMPDIR);
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Exiting with failure...review errors and retry accordingly.\n");
                return -1;
            }
            fflush(stdout);
            fflush(stderr);
        }
    }

    /* check on 'magic' partition to see if it needs to be initialized
       there is a step that's performed during boot up called run_once.sh (it may get changed in the future)
       that sets a 'flag' => "runonce" at an offset in the magic partition; this flag is what will be used
       to verify if the partition needs to get cleaned up */
    if (!system("dd if=" MAGICPART " of=/tmp/check bs=4 skip=32768 count=2 2>/dev/null")) {
        if (system("[[ \"`cat /tmp/check`\" == \"runonce\" ]]")) {
            /* this means runonce NOT set */
            system("flash_erase " MAGICPART " 0 0 > /dev/null");
        }
        system("rm -f /tmp/check");
    }
#ifdef MULT_BANKS
    /* need to set 'magic' partition so u-boot will load correct bank after reboot */
    if (!system("flash_erase " MAGICPART " 0 1")) {
        /* only need to program if using 'B' bank */
        if (bankb) {
            if (system("echo " MAGICVAL " | dd of=" MAGICPART)) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: failure at setting flag for switching to new image! Reboot and retry...\n");
                if (reboot) {
                    printf("Aborting automatic reboot due to above error\n");
                }
                reboot = MAGICFAILURE;  /* applies to both auto reboot option or not; repurposing reboot */
            }
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: failure in preparing to switch to new image after reboot! System will boot to existing firmware. Reboot and retry...\n");
        if (reboot) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Aborting automatic reboot due to above error\n");
        }
        reboot = MAGICFAILURE;  /* applies to both auto reboot option or not; repurposing reboot */
    }
#else
    /* need to erase anything that may have been set */
    if (system("flash_erase " MAGICPART " 0 1")) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: failure in preparing boot environment for NOR. Reboot and retry OR try NOR at your own risk...\n");
        if (reboot) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Aborting automatic reboot due to above error\n");
        }
        reboot = MAGICFAILURE;  /* applies to both auto reboot option or not; repurposing reboot */
    }
#endif /* MULT_BANKS */

    system("cd - > /dev/null 2>&1");
    system("rm -rf " TMPDIR);
 
    /* deactivate amber led */
    system("/usr/local/bin/rsuhealth -llr");

    if (reboot == MAGICFAILURE) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSUHEALTH_NOR_FLASHING_FAILED.\n");
        printf("NOR flashing complete...see above error.\n");
        return 1;
    }
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSUHEALTH_NOR_FLASHING_DONE.\n");
    i2vUtilDisableSyslog();

    if (!reboot) {
#ifdef MULT_BANKS
        printf("\nNOR flashing complete...Reboot system to activate new firmware.\n\n");
#else
        printf("\nNOR flashing complete...Power off device; adjust harness for booting from NOR; and then power on device for firmware to take effect.\n\n");
#endif /* MULT_BANKS */
    } else {
        printf("\nNOR flashing complete...rebooting system now.\n\n");
        sleep(2);    /* allow 2 seconds to see the message */
        system("reboot");
    }
    i2vUtilDisableDebug();
    return 0;
}

