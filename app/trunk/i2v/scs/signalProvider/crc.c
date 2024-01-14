/**************************************************************************
 *                                                                        *
 *     File Name:  crc.c                                                  *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *     Reproduced from:                                                   *
 *         lib_crc                                                        *
 *         Lammert Bies 1999-2008                                         *
 *         info@lammertbies.nl                                            *
 *         www.lammertbies.nl/comm/software/index.html                    *
 *                                                                        *
 **************************************************************************/
/* standalone file to eventually be relocated into i2v library assuming
   no license violation (see reproduced from above) */

#define CFALSE 0
#define CTRUE  1

#define P_16   0xA001
#define P_CCITT 0x1021

static int crcTab16Init = CFALSE;
static unsigned short crcTab16[256];
static int crcTabCCITT16Init = CFALSE;
static unsigned short crcTabCCITT16[256];

static void initCrc16Tab(void)
{
    int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) {
        crc = 0;
        c = (unsigned short) i;
        for (j=0; j<8; j++) {
            if ((crc ^ c) & 0x0001) {
                crc = (crc >> 1) ^ P_16;
            } else {
                crc = crc >> 1;
            }
            c = c >> 1;
        }
        crcTab16[i] = crc;
    }
    crcTab16Init = CTRUE;
}

static void initCrcCCITT16Tab(void)
{
    int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) {
        crc = 0;
        c = ((unsigned short) i) << 8;
        for (j=0; j<8; j++) {
            if ((crc ^ c) & 0x8000) {
                crc = (crc << 1) ^ P_CCITT;
            } else {
                crc = crc << 1;
            }
            c = c << 1;
        }
        crcTabCCITT16[i] = crc;
    }
    crcTabCCITT16Init = CTRUE;
}

static unsigned short updateCrc16(unsigned short crc, char c)
{
    unsigned short tmp, shortC;

    shortC = 0x00ff & (unsigned short) c;
    if (!crcTab16Init) {
        initCrc16Tab();
    }

    tmp = crc ^ shortC;
    crc = (crc >> 8) ^ crcTab16[tmp & 0xff];

    return crc;
}

static unsigned short updateCrcCCITT16(unsigned short crc, char c)
{
    unsigned short tmp, shortC;

    shortC = 0x00ff & (unsigned short) c;
    if (!crcTabCCITT16Init) {
        initCrcCCITT16Tab();
    }

    tmp = (crc >> 8) ^ shortC;
    crc = (crc << 8) ^ crcTabCCITT16[tmp & 0xff];

    return crc;
}

unsigned short calcCRC(char *buf, int length)
{
    int i;
    unsigned short crc = 0;

    for (i=0; i<length; i++) {
        crc = updateCrc16(crc, buf[i]);
    }

    return crc;
}

unsigned short calcCCITTCRC(char *buf, int length)
{
    int i;
    unsigned short crc = 0;

    for (i=0; i<length; i++) {
        crc = updateCrcCCITT16(crc, buf[i]);
    }

    return crc;
}

