/******************************************************************************
 * Copyright (C) NXP Semiconductors, 2016
 * This software is property of NXP Semiconductors. Unauthorized
 * duplication and disclosure to third parties is prohibited.
 *****************************************************************************/

/**
 *
 * @file v2xSe.c
 *
 * @author kanal
 *
 * @version 1.0
 *
 * @brief <brief file description>
 *
 * <More detailed description shall be added here.>
 *
 * @history  < list of the changes >
 *
 ******************************************************************************/
/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "v2xSe.h"
#include "version.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

#define V2XSE_MAX_KEY_ID 9999

#define V2XSE_MAX_DATA_INDEX_GSA 9999

typedef enum
{
    e_cmdEXIT=0,
    /* Device management */
    e_cmdActivate=1,                    e_cmdDeactivate,
    e_cmdConnect,                       e_cmdReset,
    e_cmdVersion,                       e_cmdGetSeInfo,
    e_cmdGetCryptoLibVersion,           e_cmdgetPlatformInfo,
    e_cmdgetPlatformConfig,             e_cmdChipInfo,
    e_cmdGetAttackLog,                  e_cmdRemainingNVM,
    e_cmdInvokeGC,
    /* Key management */
    e_cmdGenMaKey,                      e_cmdGetMaKey,
    e_cmdGenBaseKey,                    e_cmdGetBaseKey,
    e_cmdDeleteBaseKey,
    e_cmdGenRunTimeEccKeyPair,          e_cmdDeriveRunTimeEccKeyPair,
    e_cmdDeleteRunTimeEccKeyPair,       e_cmdGetRunTimeEccKeyPair,
    e_cmdActivateRunTimeEccKeyPair,
    /* Signature */
    e_cmdGenMaSig,                      e_cmdGenBaseSig,
    e_cmdGenRtSig,                      e_cmdGenRtSigLowLatency,
    /* ECIES */
    e_cmdEncryptUsingEcies,             e_cmdDecryptUsingMA,
    e_cmdDecryptUsingBaseEcies,         e_cmdDecryptUsingRunTimeEcies,
    /* GS */
    e_cmdStoreData,                     e_cmdDeleteData,
    e_cmdGetData,
    /* Utility */
    e_cmdRandomNumber,                    e_cmdGetSePhase,
    /* Key injection */
    e_cmdActivateWithSecurityLevel,

    e_cmdEndKeyInjection,

    /* Get Temperature */
    e_cmdGetChipTemperature


} exCmd_t;
/******************************************************************************
 * INTERNAL FUNCTION DECLARATIONS
 *****************************************************************************/
void resultPrint(const char *funName,int status,uint16_t hsmStatusCode);
static void resultPrintWithOutHsm(const char *funName,int status);
void printXYdata(uint8_t *xData, uint8_t xDataLen, uint8_t *yData, uint8_t yDataLen);
void resultPubkeyPrint(const char *funName,int status,uint16_t hsmStatusCode,TypePublicKey_t *pPublicKeyPlain,TypeCurveId_t curveId);
void resultSignaturePrint(const char *funName,int status,uint16_t hsmStatusCode,TypeSignature_t *signature,TypeCurveId_t curveId);
void resultDecryptPrint(const char *funName,int status,uint16_t hsmStatusCode,uint8_t msgLen,uint8_t *msg);
void displayMenu(void);
void readCurveId(TypeCurveId_t *curveId);
int readKeyId(void);
static void displayChipInfo(TypeChipInformation_t *pChipInfo,int status,uint16_t *pHsmStatusCode);
void displayAttackLog(int status,uint16_t hsmStatusCode,TypeAttackLog_t *pAttackLog);
void jcopSpecificCommand(exCmd_t cmd);
/******************************************************************************
 *  EXTERNAL FUNCTION DECLARATIONS
 *****************************************************************************/
int v2xDal_set_target(char *newtarget);
int v2xDal_set_dav_pin(int pin);
int v2xDal_set_dav_mode(int mode);
int v2xDal_set_rst_pin(int pin);

/******************************************************************************
 * GLOBAL Variable
 *****************************************************************************/
int dvt_test = 0;
int dav_pin = 0;
int dav_mode = 0; // 0: interrupt, 1: polling pin, 2: force polling
int spi_target_argc = 0;
int rst_pin = 0;
int denso_test = 0;

/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/
int main(int argc, char *argv[])
{
    int cmd=1;
    int status=0;
    int userOptForKey=0;
    TypeRtKeyId_t rtKeyId=0;
    TypeBaseKeyId_t baseKeyId=0;
    TypeCurveId_t curveId=V2XSE_CURVE_NISTP256;
    TypeSW_t hsmStatusCode=0x0000;
    TypePublicKey_t publicKeyPlain= {{0},{0}};
    TypePublicKey_t pubKeyPlain= {{0},{0}};
    TypeHashLength_t hashLength;
    TypeHash_t hashValue= {{
            0x98,0x45,0x23,0x64,0x15,0xE6,0x17,0xA8,0xF9,0x98,0x11,0xC2,0x13,0x14,0x15,0x98,
            0xCC,0x11,0x19,0xFF,0x21,0x01,0x23,0xAB,0x25,0x91,0x23,0x28,0x19,0xCA,0xAA,0x32,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        }
    };
    TypeSignature_t signature;
    TypeLowlatencyIndicator_t fastIndicator=0;
    struct timespec t_start= {0,0},t_end= {0,0};
    TypeInt256_t fvSign= {{
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff
        }
    };
    TypeInt256_t rvij= {{
            0xA9, 0xFB, 0x57, 0xDB,
            0xA1, 0xEE, 0xA9, 0xBC,
            0x3E, 0x66, 0x0A, 0x90,
            0x9D, 0x83, 0x8D, 0x71,
            0x8C, 0x39, 0x7A, 0xA3,
            0xB5, 0x61, 0xA6, 0xF7,
            0x90, 0x1E, 0x0E, 0x82,
            0x97, 0x48, 0x56, 0xA6
        }
    };
    TypeInt256_t hvij=  {{
            0x00, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x00
        }
    };
    TypeVersion_t version= {{0}};
    TypeLen_t length=0;
    TypeRandomNumber_t randomNumber= {{0}};
    TypeEncryptEcies_t eciesData;
    TypeDecryptEcies_t decryEciesData;
    TypeVCTData_t vctData;
    uint8_t data[16] = {0x34,0xAB,0x18,0x78,0x92,0xAD,0xF1,0xEF,0x03,0x45,0xFA,0x39,0x22,0x89,0x10,0xCC};
    uint8_t msgData[16]= {0};
    TypeLen_t msgLen=0;
    appletSelection_t appletId;
    channelSecLevel_t securityLevel;
    TypeInformation_t seInfo;
    int returnPubKey=1;
    int i;
    uint8_t m=0;
    uint8_t defaultPubKey[64]=
    {
        0x95, 0xCD, 0xCD, 0x86, 0x7D, 0x7E, 0x51, 0xEE, 0xAB, 0xB2, 0x23, 0x0D, 0xEF, 0xCA, 0x15, 0xA9,
        0x89, 0xB8, 0x8A, 0x76, 0x39, 0x11, 0xF0, 0x4E, 0x68, 0xF7, 0xEE, 0x76, 0xB6, 0xF8, 0x98, 0xB6,
        0xFA, 0x9E, 0x78, 0xED, 0x2C, 0x93, 0x6C, 0xE0, 0x4E, 0xD2, 0xF5, 0xA3, 0xFE, 0xA0, 0x0E, 0x0D,
        0x87, 0x06, 0x0D, 0xE4, 0x34, 0x93, 0x8B, 0x13, 0xA0, 0x6A, 0xAB, 0xE3, 0x21, 0x08, 0xC8, 0x1F
    };
    int index;
    uint8_t gsData[256]=
    {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    };
    uint32_t gsLength=10;
    uint32_t tempLen=0;
    uint8_t tempGsData[260];
    TypeLen_t tempGsLength=0;
    uint8_t phaseInfo;
    TypeHashLength_t hashLenFromCurve =0;
    decryEciesData.pVctData=&vctData;
    decryEciesData.vctLen = V2XSE_MAX_VCT_DATA_SIZE;
    eciesData.curveId=V2XSE_CURVE_NISTP256;
    //eciesData.kdfParamP1;
    eciesData.kdfParamP1Len=0;
    eciesData.macLen=32;
    //eciesData.macParamP2
    eciesData.macParamP2Len=0;
    eciesData.msgLen = (TypeLen_t)(sizeof(data));
    eciesData.pEccPublicKey=&publicKeyPlain;

    eciesData.pMsgData=(TypePlainText_t *)data;

    //decryEciesData.kdfParamP1
    decryEciesData.kdfParamP1Len=0;
    decryEciesData.macLen=32;
    //decryEciesData.macParamP2
    decryEciesData.macParamP2Len=0;

    dvt_test = 0;
    dav_pin = 0;
    dav_mode = 0;
    spi_target_argc = 0;
    rst_pin = 0;
    denso_test = 0;

    //get dvt test, target, DAV test mode, DAV pin
{
    int loop;
    //
    for (loop = 1; loop < argc; loop++) {

        //DENSO test
        if (!strcmp(argv[loop], "hsm_temp")) {
            denso_test = 1;
            continue;
        }

        //dvt test
        if (!dvt_test && !strcmp(argv[loop], "dvt-test")) {
            dvt_test = 1;
            continue;
        }

        // get target
        if (!spi_target_argc && !strcmp(argv[loop], "-t")) {
            if (loop + 1 == argc) {
                printf("invalid -t argument\n");
                return -1; /* invalid argument*/
            } else {
                v2xDal_set_target(argv[loop + 1]);
                spi_target_argc = 1;
                loop++;
                continue;
            }
        }

        //get dav pin
        if (!dav_pin && !strcmp(argv[loop], "-d")) {
            if (loop + 1 == argc) {
                printf("invalid -d argument\n");
                return -1; /* invalid argument*/
            } else {
                dav_pin = atoi(argv[loop + 1]);
                v2xDal_set_dav_pin(dav_pin);
                loop++;
                continue;
            }
        }

        //get dav mode
        if (!dav_mode && !strcmp(argv[loop], "-m")) {
            if (loop + 1 == argc) {
                printf("invalid -m argument\n");
                return -1; /* invalid argument*/
            } else {
                dav_mode = atoi(argv[loop + 1]);
                v2xDal_set_dav_mode(dav_mode);
                loop++;
                continue;
            }
        }

        //get reset pin
        if (!rst_pin && !strcmp(argv[loop], "-r")) {
            if (loop + 1 == argc) {
                printf("invalid -r argument\n");
                return -1; /* invalid argument*/
            } else {
                rst_pin = atoi(argv[loop + 1]);
                v2xDal_set_rst_pin(rst_pin);
                loop++;
                continue;
            }
        }
        if (!strcmp(argv[loop], "-h")) {
            printf("-m dav mode\n");
            printf("-d dav pin\n");
            printf("-r reset pin\n");
            printf("dvt-test\n");
            printf("-t spi node name\n");
            printf("-h spi Help Context\n");
                    printf("Mainboard HSM example: v2xse-example-app -t /dev/spidev0.0 -d 477 -m 1 -r 206 dvt-test\n");
                    printf("DSRC DC HSM example:   v2xse-example-app -t /dev/spidev3.0 -d 416 -m 1 -r 401 dvt-test\n");
            return 0;
        }
    }
}

    if(denso_test) {
        int32_t ret = V2XSE_SUCCESS;
        double temperature = -999.6;
        if(V2XSE_SUCCESS != (ret = v2xSe_activate(e_US_AND_GS,&hsmStatusCode))){
            temperature = -999.7;
        } else {
            if(V2XSE_FAILURE == (ret = v2xSe_connect())) {
                temperature = -999.8;
            } else {
                if(V2XSE_SUCCESS != (ret = v2xSe_getChipTemperature(&hsmStatusCode,&temperature))){
                    temperature = -999.9;
                }
            }
        }
        if(V2XSE_SUCCESS == ret) {
            (void)printf("%3.2f,\n",temperature);
        } else {
            (void)printf("0x%x,\n",hsmStatusCode);
        }
        return 0;
    }
    (void)printf("Date:%s Time:%s week no:%s day:%s base ver:%s\n",V2X_HOSTAPP_BUILD_DATE,V2X_HOSTAPP_BUILD_TIME,V2X_HOSTAPP_BUILD_WK_NUMBER,
                 V2X_HOSTAPP_BUILD_WK_DAY,V2X_HOSTAPP_BASELINE_VER);
    (void)v2xSe_getCryptoLibVersion(&version);
    (void)printf("Crypto Lib Version: %x.%x.%x\n",version.data[0],version.data[1],version.data[2]);
    displayMenu();
    do
    {

        if (!dvt_test) {
            (void)printf("==================================================================================\n");
            (void)printf("INFO: Press 0 to exit\n");
            (void)printf("INFO: To Display menu again provide number more than 100 or unused command number\n");
            (void)printf("\nINFO: Provide command number to be executed: ");
        }
        if (dvt_test == 1) {
            cmd = e_cmdActivate;
            dvt_test = 2;
        } else if (dvt_test == 2) {
            cmd = e_cmdChipInfo;
            dvt_test = 3;
        } else if (dvt_test == 3) {
            cmd = e_cmdgetPlatformInfo;
            dvt_test = 4;
        } else if (dvt_test == 4) {
            cmd = 0;           
        } else {
            if(scanf("%d",&cmd)==0)
                break;
        }

        switch((exCmd_t)cmd)
        {
        case e_cmdActivate:
            if (!dvt_test) {
                (void)printf("\nSelect the applet(EU Applet=%d, US Applet=%d, EU and GS:%d, US and GS:%d ): ",e_EU, e_US,e_EU_AND_GS,
                    e_US_AND_GS);
            }

            if (dvt_test == 2) {
                appletId = e_EU;
            } else {
                (void)fflush(stdout);
                (void)scanf("%d", (int *)&appletId);
            }            
            if(appletId<=e_US_AND_GS)
            {

                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_activate(appletId,&hsmStatusCode);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_activate, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                resultPrint("Activate",status,hsmStatusCode);

            }
            else
            {
                (void)printf("\nWrong applet ID entered\n");
            }


            break;

        case e_cmdActivateWithSecurityLevel:
            status = V2XSE_SUCCESS;
            (void)printf("\nSelect the applet(EU Applet=%d, US Applet=%d, EU and GS=%d, US and GS=%d ): ",e_EU, e_US,e_EU_AND_GS,
                         e_US_AND_GS);
            (void)fflush(stdout);
            (void)scanf("%d", (int *)&appletId);
            if(appletId<=e_US_AND_GS)
            {
                (void)printf("\nSelect security level (1, 2, 3, 4, 5 ): ");
                (void)fflush(stdout);
                (void)scanf("%d", (int *)&securityLevel);
            }
            else
            {
                (void)printf("\nWrong applet ID entered\n");
                status = V2XSE_FAILURE;
            }
            if(status == V2XSE_SUCCESS)
            {
                if((securityLevel > e_channelSecLevel_5) || (securityLevel < e_channelSecLevel_1))
                {
                    (void)printf("\nWrong security level entered\n");

                    status = V2XSE_FAILURE;
                }
            }
            if(status == V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_activateWithSecurityLevel(appletId,securityLevel,&hsmStatusCode);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_activateWithSecurityLevel, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

                resultPrint("Activate With Security Level",status,hsmStatusCode);
            }
            break;

        case e_cmdDeactivate:
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_deactivate();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_deactivate, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrintWithOutHsm("Deactivate",status);
            break;
        case e_cmdConnect:
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_connect();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_connect, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrintWithOutHsm("Connect",status);
            break;
        case e_cmdVersion:
            (void)printf("\nSelect the applet(V2x Applet=0, GS Applet=1): ");
            (void)fflush(stdout);
            (void)scanf("%d", (int *)&appletId);
            status=V2XSE_SUCCESS;
            if(appletId==(appletSelection_t)0)
            {
                appletId=e_V2X;
            }
            else if(appletId==(appletSelection_t)1)
            {
                appletId=e_GS;
            }
            else
            {
                status=V2XSE_FAILURE;
                hsmStatusCode=V2XSE_WRONG_DATA;
            }
            if(status==V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_getAppletVersion(appletId,&hsmStatusCode,&version);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_getAppletVersion, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            }
            resultPrint("Get Applet Version",status,hsmStatusCode);
            if(status==V2XSE_SUCCESS)
            {
                (void)printf("\nGet Applet Version: %d.%d.%d \n",version.data[0],version.data[1],version.data[2]);
            }
            break;
        case e_cmdRandomNumber:
            (void)printf("\nEnter the length of Random Number: ");
            (void)fflush(stdout);
            (void)scanf("%d", &tempLen);
            if(tempLen>V2XSE_MAX_RND_NUM_SIZE)
            {
                tempLen=V2XSE_MAX_RND_NUM_SIZE+1u;
            }
            tempLen=tempLen & 0x00FFu;
            length=(TypeLen_t)tempLen;

            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getRandomNumber(length,&hsmStatusCode,&randomNumber);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getRandomNumber, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            resultPrint("Random Number",status,hsmStatusCode);
            if((status==V2XSE_SUCCESS)&&((uint8_t)length<V2XSE_MAX_RND_NUM_SIZE))
            {
                printXYdata((uint8_t *)randomNumber.data,length, NULL, 0);
            }
            break;

        case e_cmdGetSeInfo:
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getSeInfo(&hsmStatusCode,&seInfo);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getSeInfo, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrint("Get SE Info",status,hsmStatusCode);
            if(status==V2XSE_SUCCESS)
            {
                (void)printf("\nMaximum RT Key Allowed  :%d\t\tMaximum Base Key Allowed:%d\n\r",seInfo.maxRtKeysAllowed,seInfo.maxBaKeysAllowed);
                (void)printf("FipsModeIndicator       :%d\t\tProofOfPossession       :%d\n\r",seInfo.fipsModeIndicator,seInfo.proofOfPossession);
                (void)printf("RollBackProtection      :%d\t\tRtKeyDerivation         :%d \n\r",seInfo.rollBackProtection,seInfo.rtKeyDerivation);
                (void)printf("ECIES support           :%d\t\tNumber of Prepared Value:%d\n\r",seInfo.eciesSupport,seInfo.numPreparedVal);
                (void)printf("MaxDataSlot             :%d\n\r",seInfo.maxDataSlots);
            }
            break;

        case e_cmdGenRunTimeEccKeyPair:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            readCurveId(&curveId);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_generateRtEccKeyPair(rtKeyId,curveId,&hsmStatusCode,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_generateRtEccKeyPair, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Generate RunTime Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdGenBaseKey:
            baseKeyId=(TypeBaseKeyId_t )readKeyId();;
            readCurveId(&curveId);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_generateBaEccKeyPair(baseKeyId,curveId,&hsmStatusCode,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: GenerateBaseEccKeyPair, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Generate Base Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdActivateRunTimeEccKeyPair:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_activateRtKeyForSigning(rtKeyId,&hsmStatusCode);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_activateRtKeyForSigning, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrint("Activate RunTime Ecc Key Pair",status,hsmStatusCode);
            break;
        case e_cmdGetRunTimeEccKeyPair:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getRtEccPublicKey(rtKeyId,&hsmStatusCode,&curveId,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getRtEccPublicKey, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Get RunTime Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdDeleteRunTimeEccKeyPair:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_deleteRtEccPrivateKey(rtKeyId,&hsmStatusCode);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: e_cmdDeleteRunTimeEccKeyPair, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrint("Delete RunTime Ecc Key Pair",status,hsmStatusCode);
            break;
        case e_cmdGetBaseKey:
            baseKeyId=(TypeBaseKeyId_t )readKeyId();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getBaEccPublicKey(baseKeyId,&hsmStatusCode,&curveId,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getBaEccPublicKey, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Get Base Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdDeleteBaseKey:
            baseKeyId=(TypeBaseKeyId_t )readKeyId();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_deleteBaEccPrivateKey(baseKeyId,&hsmStatusCode);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_deleteBaEccPrivateKey, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrint("Delete Base Ecc Key Pair",status,hsmStatusCode);
            break;
        case e_cmdDeriveRunTimeEccKeyPair:
            (void)printf("RtKeyId, ");
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            (void)printf("BaseKeyId, ");
            baseKeyId=(TypeBaseKeyId_t )readKeyId();
            (void)printf("ReturnPubKey, 0: disable public key output, 1: enable public key output\n");
            (void)printf("Enter your choice:");
            (void)fflush(stdout);
            (void)scanf("%d",&returnPubKey);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_deriveRtEccKeyPair(baseKeyId,&fvSign,&rvij,&hvij,rtKeyId,(TypePubKeyOut_t)returnPubKey,&hsmStatusCode,&curveId,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: DeriveRunTimeEccKeyPair, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            if(returnPubKey!=0)
            {
                resultPubkeyPrint("Derive RunTime Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            }
            else
            {
                resultPrint("Derive RunTime Ecc Key Pair",status,hsmStatusCode);
            }

            break;
        case e_cmdGenMaKey:
            readCurveId(&curveId);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_generateMaEccKeyPair(curveId,&hsmStatusCode,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_generateMaEccKeyPair, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Generate Module Authentication Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdGetMaKey:
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getMaEccPublicKey(&hsmStatusCode,&curveId,&publicKeyPlain);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getMaEccPublicKey, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPubkeyPrint("Get Module Authentication Ecc Key Pair",status,hsmStatusCode,&publicKeyPlain,curveId);
            break;
        case e_cmdGenRtSigLowLatency:
            curveId=V2XSE_CURVE_NISTP256;/* The value can be V2XSE_CURVE_NISTP256, V2XSE_CURVE_BP256R1 or V2XSE_CURVE_BP256T1, these values represent 256 bit ecc curves and
                           as Runtime signature is always with 256 bit curves, one of these values can be passed*/

            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_createRtSignLowLatency( &hashValue,&hsmStatusCode,&signature,&fastIndicator);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_createRtSignLowLatency, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            if(status==V2XSE_SUCCESS)
            {
                (void)printf("\nINFO: FastIndicator: %d",fastIndicator);
            }
            resultSignaturePrint("CreateRtSignLowLatency",status,hsmStatusCode,&signature,curveId);
            break;
        case e_cmdGenRtSig:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            status=v2xSe_getRtEccPublicKey(rtKeyId,&hsmStatusCode,&curveId,&publicKeyPlain);
            if(status==V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_createRtSign(rtKeyId,&hashValue,&hsmStatusCode,&signature);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_createRtSignatureFromHash, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                resultSignaturePrint("Create Run Time Signature from hash",status,hsmStatusCode,&signature,curveId);
            }
            else
            {
                resultSignaturePrint("v2xSe_getRtEccPublicKey",status,hsmStatusCode,&signature,curveId);
            }
            break;
        case e_cmdGenBaseSig:
            baseKeyId=(TypeBaseKeyId_t )readKeyId();
            status=v2xSe_getBaEccPublicKey(baseKeyId,&hsmStatusCode,&curveId,&publicKeyPlain);
            if(status==V2XSE_SUCCESS)
            {
                status = v2xSe_getKeyLenFromCurveID(curveId)/2;
                if(status != V2XSE_FAILURE)
                {
                    hashLenFromCurve = (TypeHashLength_t)status; //Type casting as required by MISRA
                }

                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_createBaSign(baseKeyId,hashLenFromCurve,&hashValue,&hsmStatusCode,&signature);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_createBaseSignatureFromHash, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                resultSignaturePrint("Create Base Signature from hash",status,hsmStatusCode,&signature,curveId);
            }
            else
            {
                resultSignaturePrint("v2xSe_getBaEccPublicKey",status,hsmStatusCode,&signature,curveId);
            }
            break;
        case e_cmdGenMaSig:
            status=v2xSe_getMaEccPublicKey(&hsmStatusCode,&curveId,&publicKeyPlain);
            if(status==V2XSE_SUCCESS)
            {
                i=v2xSe_getKeyLenFromCurveID(curveId)/2;
                if(i!=V2XSE_FAILURE)
                {
                    hashLength=(TypeHashLength_t)i;

                    (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                    status=v2xSe_createMaSign(hashLength,&hashValue,&hsmStatusCode,&signature);
                    (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                    (void)fprintf( stdout,"\n\rINFO: v2xSe_createMaSign, Latency: %.5f s \n\r",
                                   ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                    resultSignaturePrint("Create MA Signature from hash",status,hsmStatusCode,&signature,curveId);
                }
                else
                {
                    status=V2XSE_FAILURE;
                    hsmStatusCode=V2XSE_UNDEFINED_ERROR;
                    resultSignaturePrint("v2xSe_getKeyLenFromCurveID",status,hsmStatusCode,&signature,curveId);
                }
            }
            else
            {
                resultSignaturePrint("v2xSe_getMaEccPublicKey",status,hsmStatusCode,&signature,curveId);
            }
            break;
        case e_cmdEncryptUsingEcies:
            (void)fprintf( stdout,"INFO: Select 0 for Default Public Key or 1 for Public key Generated during current execution:\n\r");
            (void)fflush(stdout);
            (void)scanf("%d", &userOptForKey);
            if(userOptForKey ==0)
            {
                eciesData.pEccPublicKey=&pubKeyPlain;
                (void)memcpy(eciesData.pEccPublicKey->x,&defaultPubKey[0],32);
                (void)memcpy(eciesData.pEccPublicKey->y,&defaultPubKey[32],32);
                eciesData.curveId=V2XSE_CURVE_NISTP256;
            }
            else
            {
                eciesData.pEccPublicKey=&publicKeyPlain;
                eciesData.curveId=curveId;
            }
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_encryptUsingEcies (&eciesData,&hsmStatusCode,&decryEciesData.vctLen,decryEciesData.pVctData );
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_encryptUsingEcies, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            resultPrint("Encrypt using ECIES",status,hsmStatusCode);
            break;
        case e_cmdDecryptUsingRunTimeEcies:
            rtKeyId=(TypeRtKeyId_t )readKeyId();
            (void)fprintf( stdout,"INFO: Select 0 for Encrypt and Decrypt or 1 for previously generated VCT data during current execution:\n\r");
            (void)fflush(stdout);
            (void)scanf("%d", &userOptForKey);
            status=V2XSE_SUCCESS;
            if(userOptForKey ==0)
            {
                readCurveId(&curveId);
                status=v2xSe_generateRtEccKeyPair(rtKeyId,curveId,&hsmStatusCode,&publicKeyPlain);
                if(status==V2XSE_SUCCESS)
                {
                    status=v2xSe_encryptUsingEcies (&eciesData,&hsmStatusCode,&decryEciesData.vctLen,decryEciesData.pVctData );
                }
            }
            if(status==V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_decryptUsingRtEcies(rtKeyId,&decryEciesData,&hsmStatusCode,&msgLen,(TypePlainText_t *)msgData);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_decryptUsingRtEcies, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            }
            resultDecryptPrint("Decrypt using RunTime Ecies",status,hsmStatusCode, msgLen,msgData);
            break;
        case e_cmdDecryptUsingBaseEcies:
            baseKeyId=(TypeBaseKeyId_t )readKeyId();
            (void)fprintf( stdout,"INFO: Select 0 for Encrypt and Decrypt or 1 for previously generated VCT data during current execution:\n\r");
            (void)fflush(stdout);
            (void)scanf("%d", &userOptForKey);
            status=V2XSE_SUCCESS;
            if(userOptForKey ==0)
            {
                readCurveId(&curveId);
                status=v2xSe_generateBaEccKeyPair(baseKeyId,curveId,&hsmStatusCode,&publicKeyPlain);
                if(status==V2XSE_SUCCESS)
                {
                    status=v2xSe_encryptUsingEcies (&eciesData,&hsmStatusCode,&decryEciesData.vctLen,decryEciesData.pVctData );
                }
            }
            if(status==V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_decryptUsingBaEcies(baseKeyId,&decryEciesData,&hsmStatusCode,&msgLen,(TypePlainText_t *)msgData);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_decryptUsingBaEcies, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            }
            resultDecryptPrint("Decrypt using Base Ecies",status,hsmStatusCode, msgLen,msgData);
            break;
        case e_cmdDecryptUsingMA:
            (void)fprintf( stdout,"INFO: Select 0 for Encrypt and Decrypt or 1 for previously generated VCT data during current execution:\n\r");
            (void)fflush(stdout);
            (void)scanf("%d", &userOptForKey);
            status=V2XSE_SUCCESS;
            if(userOptForKey ==0)
            {
                status=v2xSe_getMaEccPublicKey(&hsmStatusCode,&curveId,&publicKeyPlain);
                if(status==V2XSE_SUCCESS)
                {
                    status=v2xSe_encryptUsingEcies (&eciesData,&hsmStatusCode,&decryEciesData.vctLen,decryEciesData.pVctData );
                }
            }
            if(status==V2XSE_SUCCESS)
            {
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_decryptUsingMaEcies(&decryEciesData,&hsmStatusCode,&msgLen,(TypePlainText_t *)msgData);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: v2xSe_decryptUsingMaEcies, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));

            }
            resultDecryptPrint("Decrypt using MA Ecies",status,hsmStatusCode, msgLen,msgData);
            break;
        case e_cmdGetCryptoLibVersion:
        case e_cmdChipInfo:
        case e_cmdgetPlatformConfig:
        case e_cmdgetPlatformInfo:
        case e_cmdGetAttackLog:
        case e_cmdRemainingNVM:
        case e_cmdInvokeGC:
        case e_cmdGetChipTemperature:
            jcopSpecificCommand((exCmd_t)cmd);
            break;

        case e_cmdStoreData:
            (void)printf("\nEnter index: ");
            (void)fflush(stdout);
            (void)scanf("%d", &index);
            if(index >=0)
            {

                if(index>V2XSE_MAX_DATA_INDEX_GSA)
                {
                    index=V2XSE_MAX_DATA_INDEX_GSA+1;
                }
                (void)printf("\nEnter length: ");
                (void)fflush(stdout);
                (void)scanf("%d", &gsLength);

                if(gsLength>V2XSE_MAX_DATA_SIZE_GSA)
                {
                    gsLength=V2XSE_MAX_DATA_SIZE_GSA+1u;
                }
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_storeData((TypeGsDataIndex_t)index, (TypeLen_t)gsLength, gsData,&hsmStatusCode);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: Store data, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                resultPrint("Store data",status,hsmStatusCode);
            }
            else
            {
                (void)printf("\nInvalid index entered\n");
                status = V2XSE_FAILURE;
            }
            break;

        case e_cmdDeleteData:
            (void)printf("\nEnter index: ");
            (void)fflush(stdout);
            (void)scanf("%d", &index);
            if(index >=0)
            {


                if(index>V2XSE_MAX_DATA_INDEX_GSA)
                {
                    index=V2XSE_MAX_DATA_INDEX_GSA+1;
                }
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_deleteData((TypeGsDataIndex_t)index, &hsmStatusCode);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: Delete data, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                resultPrint("Delete data",status,hsmStatusCode);
            }
            else
            {
                (void)printf("\nInvalid index entered\n");
                status = V2XSE_FAILURE;
            }
            break;

        case e_cmdGetData:
            (void)printf("\nEnter index: ");
            (void)fflush(stdout);
            (void)scanf("%d", &index);
            if(index >=0)
            {
                if(index>V2XSE_MAX_DATA_INDEX_GSA)
                {
                    index=V2XSE_MAX_DATA_INDEX_GSA+1;
                }
                (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
                status=v2xSe_getData((TypeGsDataIndex_t)index, &tempGsLength, tempGsData,&hsmStatusCode);
                (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
                (void)fprintf( stdout,"\n\rINFO: Get data, Latency: %.5f s \n\r",
                               ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
                if(status==V2XSE_SUCCESS)
                {
                    (void)printf("\nLength:%d \n\rData: ",tempGsLength);
                    for(m=0; m<tempGsLength; m++)
                    {
                        (void)printf("%02X ",tempGsData[m]);
                    }
                }
                resultPrint("Get data",status,hsmStatusCode);
            }
            else
            {
                (void)printf("\nInvalid index entered\n");
                status = V2XSE_FAILURE;
            }

            break;

        case e_cmdReset:

            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_reset();
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"INFO: v2xSe_reset, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)((double)t_end.tv_nsec)))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrintWithOutHsm("Reset",status);
            break;

        case e_cmdEndKeyInjection:
            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_endKeyInjection (&hsmStatusCode);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"INFO: v2xSe_endKeyInjection, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            resultPrint("End Key Injection",status,hsmStatusCode);
            break;

        case  e_cmdGetSePhase:

            (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
            status=v2xSe_getSePhase(&phaseInfo,&hsmStatusCode);
            (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
            (void)fprintf( stdout,"\n\rINFO: v2xSe_getSePhase, Latency: %.5f s \n\r",
                           ((double)t_end.tv_sec+1.0e-9*((double)((double)t_end.tv_nsec)))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
            if(status == V2XSE_SUCCESS)
            {
                if(phaseInfo == V2XSE_NORMAL_OPERATING_PHASE)
                {
                    (void)fprintf( stdout,"INFO: Device in Normal Operating phase");
                }
                if(phaseInfo == V2XSE_KEY_INJECTION_PHASE)
                {
                    (void)fprintf( stdout,"INFO: Device in Key injection phase");
                }
            }
            resultPrint("GetSePhase",status,hsmStatusCode);

            break;

        case e_cmdEXIT:
            break;
        default:
            if(cmd>=100)
            {
                displayMenu();
            }
            else
            {
                (void)printf("\nWrong Command...\n");
            }
            break;

        }
    }
    while(cmd!=0);

    (void)printf("\nGood Bye...\n");
    (void)printf("\n==================================================================================\n");
    return 0;
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   resultDecryptPrint
 *
 * DESCRIPTION  :   Display decrypt API response.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void resultDecryptPrint(const char *funName,int status,uint16_t hsmStatusCode,uint8_t msgLen,uint8_t *msg)
{
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("INFO: %s, Result: Success\n",funName);
        printXYdata(msg, (uint8_t)msgLen, NULL, 0);
    }
    else
    {
        (void)printf("ERROR:%s Result: Failure, Status: %d HSM Code:%4x\n",funName,status,hsmStatusCode);
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   resultSignaturePrint
 *
 * DESCRIPTION  :   Display signature response.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void resultSignaturePrint(const char *funName,int status,uint16_t hsmStatusCode,TypeSignature_t *signature,TypeCurveId_t curveId)
{
    int32_t temp;
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("\nINFO: Signature, Ry:%x\n",signature->Ry);
        (void)printf("\nINFO: %s, Result: Success\n",funName);
        temp= v2xSe_getKeyLenFromCurveID(curveId);
        if(temp !=V2XSE_FAILURE)
        {
            temp = temp /2;
            printXYdata(signature->r,(uint8_t)temp, signature->s, temp);
        }
    }
    else
    {
        (void)printf("\nERROR:%s Result: Failure, Status: %d HSM Code:%4x\n",funName,status,hsmStatusCode);
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   resultPubkeyPrint
 *
 * DESCRIPTION  :   Display public key response.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void resultPubkeyPrint(const char *funName,int status,uint16_t hsmStatusCode,TypePublicKey_t *pPublicKeyPlain,TypeCurveId_t curveId)
{
    int32_t temp;
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("INFO: %s, Result: Success\n",funName);
        temp = v2xSe_getKeyLenFromCurveID(curveId);
        if(temp != V2XSE_FAILURE )
        {
            temp = temp /2;
            printXYdata(pPublicKeyPlain->x, (uint8_t)temp, pPublicKeyPlain->y, temp);
        }
    }
    else
    {
        (void)printf("ERROR:%s Result: Failure, Status: %d HSM Code:%4x\n",funName,status,hsmStatusCode);
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   resultPrintWithOutHsm
 *
 * DESCRIPTION  :   Display response status
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static void resultPrintWithOutHsm(const char *funName,int status)
{
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("INFO: %s, Result: Success\n",funName);
    }
    else
    {
        (void)printf("ERROR:%s Result: Failure, Status: %d\n",funName,status);
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   resultPrint
 *
 * DESCRIPTION  :   Display response status
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void resultPrint(const char *funName,int status,uint16_t hsmStatusCode)
{
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("INFO: %s, Result: Success\n",funName);
    }

    else
    {
        /*artf689500 : Add correspondent instructions to check if certain keyfile is copied in v2xse-example-app and v2xse-sanity utilities when 0x6F03 error occurs*/
        if( (strcmp(funName, "Activate") == 0 || strcmp(funName, "Activate With Security Level") == 0) && (hsmStatusCode ==V2XSE_SCP03_KEYFILE_ERROR))
        {
            (void)fprintf(stdout,"ERROR:Key file not copied from SXF1800 package into correct path or\n"
                          "      Key file contains invalid number of keys or keys are of invalid length\n");
            (void)printf("ERROR:%s Result: Failure, Status: %d HSM Code:%4x\n",funName,status,hsmStatusCode);

        }
        else
        {
            (void)printf("ERROR:%s Result: Failure, Status: %d HSM Code:%4x\n",funName,status,hsmStatusCode);
        }
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   printXYdata
 *
 * DESCRIPTION  :   Display public key
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void printXYdata(uint8_t *xData, uint8_t xDataLen, uint8_t *yData, uint8_t yDataLen)
{
    uint8_t i;
    uint8_t *x=xData;
    uint8_t *y=yData;
    if(xDataLen!=0u)
    {
        (void)printf("\n0x");
        for(i=0; i<xDataLen; i++)
        {
            (void)printf("%02X",*x);
            x++;
        }
    }
    if(yDataLen!=0u)
    {
        (void)printf("\n0x");
        for(i=0; i<yDataLen; i++)
        {
            (void)printf("%02X",*y);
            y++;
        }
    }
    (void)printf("\n");
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   displayMenu
 *
 * DESCRIPTION  :   Display Menu for user input
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void displayMenu(void)
{
    (void)printf("\n==================================================================================\n");

    (void)printf("%02d: activate                                %02d: deactivate\n",e_cmdActivate,e_cmdDeactivate);
    (void)printf("%02d: connect                                 %02d: reset\n",e_cmdConnect,e_cmdReset);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: getAppletVersion                        %02d: getSeInfo\n",e_cmdVersion,e_cmdGetSeInfo);
    (void)printf("%02d: getCryptoLibVersion                     %02d: getPlatformInfo\n",e_cmdGetCryptoLibVersion,e_cmdgetPlatformInfo);
    (void)printf("%02d: getPlatformConfig                       %02d: getChipInfo\n",e_cmdgetPlatformConfig,e_cmdChipInfo);
    (void)printf("%02d: getAttackLog                            %02d: getNvm\n",e_cmdGetAttackLog,e_cmdRemainingNVM);
    (void)printf("%02d: invokeGarbageCollector                   \n",e_cmdInvokeGC);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: generateMaEccKeyPair                    %02d: getMaEccPublicKey\n",e_cmdGenMaKey,e_cmdGetMaKey);
    (void)printf("%02d: generateBaEccKeyPair                    %02d: getBaEccPublicKey\n",e_cmdGenBaseKey,e_cmdGetBaseKey);
    (void)printf("%02d: deleteBaEccPrivateKey                   %02d: generateRtEccKeyPair\n",e_cmdDeleteBaseKey,e_cmdGenRunTimeEccKeyPair);
    (void)printf("%02d: deriveRtEccKeyPair                      %02d: deleteRtEccPrivateKey\n",e_cmdDeriveRunTimeEccKeyPair,e_cmdDeleteRunTimeEccKeyPair);
    (void)printf("%02d: getRtEccPublicKey                       %02d: activateRtKeyForSigning\n",e_cmdGetRunTimeEccKeyPair,
                 e_cmdActivateRunTimeEccKeyPair);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: createMaSign                            %02d: createBaSign\n",e_cmdGenMaSig,e_cmdGenBaseSig);
    (void)printf("%02d: createRtSign                            %02d: createRtSignLowLatency \n",e_cmdGenRtSig,e_cmdGenRtSigLowLatency);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: encryptUsingEcies                       %02d: decryptUsingMaEcies\n",e_cmdEncryptUsingEcies,e_cmdDecryptUsingMA);
    (void)printf("%02d: decryptUsingBaEcies                     %02d: decryptUsingRtEcies\n",e_cmdDecryptUsingBaseEcies,e_cmdDecryptUsingRunTimeEcies);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: storeData                               %02d: deleteData\n",e_cmdStoreData,e_cmdDeleteData);
    (void)printf("%02d: getData                                 \n",e_cmdGetData);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: getRandomNumber                         %02d: getSePhase \n", e_cmdRandomNumber, e_cmdGetSePhase );
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: activateWithSecurityLevel               %02d: endKeyInjection\n",e_cmdActivateWithSecurityLevel, e_cmdEndKeyInjection);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: getChipTemperature\n", e_cmdGetChipTemperature);
    (void)printf("----------------------------------------------------------------------------------\n");
    (void)printf("%02d: EXIT\n", e_cmdEXIT);
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   readCurveId
 *
 * DESCRIPTION  :   Reading curve Id from user input
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void readCurveId(TypeCurveId_t *curveId)
{
    int temp;
    (void)printf("\nEnter curve Id: ");
    (void)fflush(stdout);
    (void)scanf("%d", &temp);
    *curveId=(TypeCurveId_t)temp;
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   readKeyId
 *
 * DESCRIPTION  :   Reading Key Id from user input
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
int readKeyId(void)
{
    int keyId;
    (void)printf("Enter key Id: ");
    (void)fflush(stdout);
    (void)scanf("%d", &keyId);
    if(keyId>V2XSE_MAX_KEY_ID)
    {
        keyId=V2XSE_MAX_KEY_ID+1;
    }
    return keyId;
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   displayAttackLog
 *
 * DESCRIPTION  :   Display Attack related log
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void displayAttackLog(int status,uint16_t hsmStatusCode,TypeAttackLog_t *pAttackLog)
{
    uint32_t i;
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("\nINFO: GetAttackLog, Result: Success, Status: %d\n",status);
        (void)printf("\nINFO: Length of Attack Log in Bytes: %d\n",pAttackLog->len);
        (void)printf("\nINFO: Current Attack log status: %d\n",pAttackLog->currAttackCntrStatus);
        if(pAttackLog->len!=0u)
        {

            for(i=0u; i<pAttackLog->len; i++)
            {
                if(i%16u==0u)
                {
                    (void)printf("\n");
                }
                if(i==0u)
                {
                    (void)printf("0x");
                }
                (void)printf("%02x",pAttackLog->data[i]);
            }
            (void)printf("\n");
        }
    }
    else
    {
        (void)printf("\nERROR:GetAttackLog Result: Failure, Status: %d HSM Code:%4x\n",status,hsmStatusCode);
    }

}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   jcopSpecificCommand
 *
 * DESCRIPTION  :   Executing JCOP specific Command
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
void jcopSpecificCommand(exCmd_t cmd)
{
    int status=0;
    TypeSW_t hsmStatusCode=V2XSE_NO_ERROR;
    TypeChipInformation_t chipInfo= {{0}};
    TypePlatformConfiguration_t platformConfig= {{0}};
    TypePlatformIdentity_t platformIdentifier= {{0}};
    TypeAttackLog_t attackLog= {0,0,{0}};
    TypeVersion_t version= {{0}};
    struct timespec t_start= {0,0},t_end= {0,0};

    uint32_t memSize;
    uint8_t i=0;
    double temperature;
    switch((exCmd_t)cmd)
    {
    case e_cmdChipInfo:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getChipInfo(&hsmStatusCode,&chipInfo);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getChipInfo, Latency: %.5f s \n\n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        displayChipInfo(&chipInfo,status,&hsmStatusCode);
        break;
    case e_cmdgetPlatformConfig:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getPlatformConfig(&hsmStatusCode,&platformConfig);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getPlatformConfig, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        if(status==V2XSE_SUCCESS)
        {
            (void)printf("\nPlatform Configuration: %02Xh%c%c%c\n",platformConfig.data[0],(char)platformConfig.data[1],(char)platformConfig.data[2],
                         (char)platformConfig.data[3]);
        }
        resultPrint("Get Platform Configuration",status,hsmStatusCode);
        break;
    case e_cmdgetPlatformInfo:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getPlatformInfo(&hsmStatusCode,&platformIdentifier);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getPlatformInfo, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        if(status==V2XSE_SUCCESS)
        {
            (void)printf("\nPlatform Identifier: ");
            for(i=0; i < V2XSE_PLATFORM_IDENTITY; i++)
            {
                (void)printf("%c",(char)platformIdentifier.data[i]);
            }
            (void)printf("\n");
        }
        resultPrint("Get Platform Information",status,hsmStatusCode);
        break;
    case e_cmdGetAttackLog:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getAttackLog(&hsmStatusCode,&attackLog);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getAttackLog, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        displayAttackLog(status,hsmStatusCode,&attackLog);
        break;
    case e_cmdGetCryptoLibVersion:
        (void)v2xSe_getCryptoLibVersion(&version);
        (void)printf("Crypto Lib Version: %x.%x.%x\n",version.data[0],version.data[1],version.data[2]);
        break;
    case e_cmdRemainingNVM:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getRemainingNvm (&memSize,&hsmStatusCode);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getRemainingNvm, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        if(status==V2XSE_SUCCESS)
        {
            (void)printf("\nMemory Remaining:%d bytes\n",memSize);
        }
        resultPrint("Get Remaining Memory",status,hsmStatusCode);
        break;
    case e_cmdInvokeGC:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_invokeGarbageCollector(&hsmStatusCode);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_invokeGarbageCollector, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        resultPrint("Invoke GarbageCollector",status,hsmStatusCode);
        break;
    case e_cmdGetChipTemperature:
        (void)clock_gettime(CLOCK_MONOTONIC,&t_start);
        status=v2xSe_getChipTemperature(&hsmStatusCode,&temperature);
        (void)clock_gettime(CLOCK_MONOTONIC,&t_end);
        (void)fprintf( stdout,"\n\rINFO: v2xSe_getChipTemperature, Latency: %.5f s \n\r",
                       ((double)t_end.tv_sec+1.0e-9*((double)t_end.tv_nsec))-((double)t_start.tv_sec+1.0e-9*((double)t_start.tv_nsec)));
        if(status==V2XSE_SUCCESS)
        {
            (void)printf("\nINFO: Chip Temperature is: %3.2f degree celsius\n",temperature);
        }
        resultPrint("Get Chip Temperature",status,hsmStatusCode);
        break;
    default:
        (void)printf("\nINFO: Invalid input command\n");
        break;
    }
}
/*-----------------------------------------------------------------------------
 * FUNCTION     :   displayChipInfo
 *
 * DESCRIPTION  :   display chip information
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static void displayChipInfo(TypeChipInformation_t *pChipInfo,int status,uint16_t *pHsmStatusCode)
{
    uint8_t i;
    if(status==V2XSE_SUCCESS)
    {
        (void)printf("Chip Information: ");
        for(i=0; i < V2XSE_SERIAL_NUMBER; i++)
        {
            (void)printf("%02x",pChipInfo->data[i]);
        }
        (void)printf("\n");
        (void)printf("INFO: ChipInfo, Result: Success\n");
    }
    else
    {
        (void)printf("ERROR:ChipInfo Result: Failure, Status: %d HSM Code:%4x\n",status,*pHsmStatusCode);
    }
}
