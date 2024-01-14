/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: cv2xcfg.c                                                        */
/* Purpose: Wrapper for cv2x-config operations                                */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20210??? JJG - Initial revision (no copyright header originally        */
/*     20210528 JJG - fix for properly setting v2x.xml                        */
/*     20210808 JJG - adding support for setting cv2x power                   */
/*     20211129 JJG - don't want to create a new utility for channel info;    */
/*                    reusing this module to provide that output              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
 NOTE: this utility is to perform cv2x configuration using the cv2x-config
 command

 20210311: this utility will need to be amended to check for the readiness of the
 radio stack; the cv2x-config command fails to apply changes whne the system is
 suck in the following mode:
 [ns_cv2x.cpp cv2x_init() 982]: Waiting for TX and RX to become active; wait_time

 For now, now check is made; once the check is made, then the power can be enabled
 (port from startup.sh into this utility)

*/
#if !defined(PLATFORM_HD_RSU_5940) /* Not tested on 5940 so leave out for now. */
#define CV2XCFGFILE  "/rwflash/customer/v2x.xml"
#define I2VCFGFILE   "/rwflash/configs/i2v.conf"

/* if desired in the future to disable result file, just change below to /dev/null */
#define RESULTFILE   "/tmp/cv2xcfgresult"

/* or is it just a qualcomm offset?; don't ask - i had to figure these values out; no documentation whatsoever */
#define CV2XOFFSET   5118
#define CV2XFACTOR   5
#endif /* 5912 */

/* channel frequencies (correct, not qualcomm-ized); only evens checked because of qualcomm's approach */
#define CH172        5860
#define CH174        5870
#define CH176        5880
#define CH178        5890
#define CH180        5900
#define CH182        5910
#define CH183        5915
#define CH184        5920


int main(int argc, char *argv[])
{
    FILE *pf=NULL;
    char buf[60];   /* power setting should never be this long or command */
    float power = 0.0f;
#if !defined(PLATFORM_HD_RSU_5940)
    struct stat st;
    unsigned char preserve = 0;
    unsigned char usecv2x = 0;
    unsigned char minfreq, maxfreq, diff;

    if ((argc > 1) && (!strcmp(argv[1], "preserve"))) {
        preserve = 1;
    }
#endif /* 5912 */

    /* 20211129: new support for providing channel info; provide range of frequency and
       try to determine channel */
    if ((argc > 1) && (!strcmp(argv[1], "channel"))) {
#if defined(PLATFORM_HD_RSU_5940)
        if (NULL != (pf = popen("cat /tmp/cv2x_freq.txt", "r"))) {
            fgets(buf, sizeof(buf), pf);
            pclose(pf);
            switch ((int32_t)strtoul(buf, NULL, 10)) {
                    case CH172:
                        printf("172\n");
                        break;
                    case CH174:
                        printf("174\n");
                        break;
                    case CH176:
                        printf("176\n");
                        break;
                    case CH178:
                        printf("178\n");
                        break;
                    case CH180:
                        printf("180\n");
                        break;
                    case CH182:
                        printf("182\n");
                        break;
                    case CH183:
                        printf("183\n");
                        break;
                    case CH184:
                        printf("184\n");
                        break;
                    default:
                        printf("UNKNOWN\n");
                        break;
            }
        }
#else
        if (NULL != (pf = popen("dncmdctl cv2x_get_capabilities | grep Freq | grep -o '[0-9]\\+' | grep '..'", "r"))) {
            fgets(buf, sizeof(buf), pf);
            minfreq = (unsigned short)strtoul(buf, NULL, 10);    /* buf will have \n but strtoul will ignore */
            /* adjustment to convert to expected frequency */
            minfreq /= CV2XFACTOR;
            minfreq -= CV2XOFFSET;
            fgets(buf, sizeof(buf), pf);    /* next frequency of output, the max */
            maxfreq = (unsigned short)strtoul(buf, NULL, 10);    /* buf will have \n but strtoul will ignore */
            /* adjustment to convert to expected frequency */
            maxfreq /= CV2XFACTOR;
            maxfreq -= CV2XOFFSET;
            pclose(pf);

            /* now check frquency; the min and max are the channel frequency range and the center frequency 
               is the frequency commonly associated with the channel; because as usual qualcomm ignores standards 
               and the 20 MHz channel is actually wrong (for 183); need to check if range is 10 or 20 MHz to know
               if channel is even or odd */
            diff = maxfreq - minfreq;
            if ((diff != 10) && (diff != 20)) {
                printf("UNKNOWN\nMin: %d Max: %d\n", minfreq, maxfreq);
            } else if (diff == 10) {
                /* even channel */
                switch (minfreq + diff/2) {
                    case CH172:
                        printf("172\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH174:
                        printf("174\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH176:
                        printf("176\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH178:
                        printf("178\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH180:
                        printf("180\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH182:
                        printf("182\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    case CH184:
                        printf("184\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                    default:
                        printf("UNKNOWN\nMin: %d Max: %d\n", minfreq, maxfreq);
                        break;
                }
            } else {
                /* odd channel - this is not standards based since qualcomm does it wrong */
                switch (minfreq + diff/2) {
                    /* magic 5 being added is to reflect odd channel frequencies */
                    case CH172:
                        printf("173\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    case CH174:
                        printf("175\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    case CH176:
                        printf("177\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    case CH178:
                        printf("179\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    case CH180:
                        printf("181\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    case CH182:
                        printf("183\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                    default:
                        printf("UNKNOWN\nMin: %d Max: %d\n", minfreq + 5, maxfreq + 5);
                        break;
                }
            }
        }
#endif /* 5912 vs 5940 */
        return 0; /* terminus operation; exit now */
    } /* If channel */

    if ((argc > 1) && (!strcmp(argv[1], "power"))) {
        pf = NULL;
#if defined(PLATFORM_HD_RSU_5940)
        if (NULL != (pf = popen("cat /tmp/cv2x_tx_pwr.txt", "r"))) {
            fgets(buf, sizeof(buf), pf);
            pclose(pf);
            power = (float)strtold(buf, NULL);
            if((power < 0.0f) || (23.0f < power)) {
                power = -98.8;
            }
            printf("%2.1f\n",power);
        } else {
            printf("-99.9\n");
        }
#endif /* 5940 */
        return 0; /* terminus operation; exit now */
    } /* If power */

#if defined(PLATFORM_HD_RSU_5940)
    /* Not tested, do nothing for now. */
#else
    /* need to check to see if radio is set for cv2x; if not, ignore erroring out */
    memset(&st, 0, sizeof(st));
    if (!stat(I2VCFGFILE, &st)) {
        if (NULL != 
           (pf = popen("cat " I2VCFGFILE " | grep RadioT | awk '{print $3}' | sed 's/;//'", "r"))) {
            fgets(buf, sizeof(buf), pf);
            pclose(pf);
            usecv2x = (uint8_t)strtoul(buf, NULL, 10);
        }
        /* if there was a failure reading, then follow on failure will be treated as real failures */
    }

    system("echo >> " RESULTFILE);    /* do not truncate any existing file */
    memset(&st, 0, sizeof(st));
    if (!stat(CV2XCFGFILE, &st)) {
        /* stop cv2x first! */
        if (!system("systemctl stop cv2x")) {
            /* send file for application */
            if (system("cv2x-config --update-config-file " CV2XCFGFILE)) {
                /* only print error if using cv2x */
                if (usecv2x) printf("\nError applying C-V2X settings\n");
                system("echo error applying c-v2x settings >> " RESULTFILE);
            } else {
                printf("\nSuccess applying C-V2X settings\n");
                if (!preserve) {
                    remove(CV2XCFGFILE);
                }
            }
            /* both cases restart cv2x?; for now yes */
            if (system("systemctl start cv2x")) {
                /* only print error if using cv2x */
                if (usecv2x) printf("ERROR: cv2xcfg unable to restart cv2x; reboot system!\n");
                system("echo cv2xcfg unable to restart cv2x >> " RESULTFILE);
                return (usecv2x) ? -1 : 0;
            }
        } else {
            /* only print error if using cv2x */
            if (usecv2x) printf("ERROR: cv2xcfg unable to stop cv2x; cannot apply v2x.xml file!\n");
            system("echo cv2xcfg unable to stop cv2x >> " RESULTFILE);
            return (usecv2x) ? -1 : 0;
        }
        system("echo success applying v2x.xml setting >> " RESULTFILE);
    }

    system("echo done cv2xcfg with success >> " RESULTFILE);
#endif /* 5912 vs 5940 */
	return 0;
}

