/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ns_pcap.c                                                        */
/*  Purpose: For implementation of the capturing of PCAP data                 */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ns_pcap.h"
#include "rsu_defs.h"

/* #defines */
//#define ENABLE_NS_PCAP_LOG

#ifdef ENABLE_NS_PCAP_LOG
#define NS_PCAP_LOG(fmt, args...) \
    { \
        FILE *fd = fopen("/mnt/rwflash/dbglogs/ns_pcap.log", "a"); \
        fprintf(fd, fmt, ##args); \
        fclose(fd); \
    }
#else
#define NS_PCAP_LOG(fmt, args...)
#endif

#define PCAP_MAX_DEV_NAME_LENGTH 16

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define cpu_to_le32(x) (x)
#define cpu_to_le16(x) (x)
#else
#define cpu_to_le32(x) \
    ((((x) & 0xff000000) >> 24) + \
     (((x) & 0xff0000)   >>  8) + \
     (((x) & 0xff00)     <<  8) + \
     (((x) & 0xff)       << 24));

#define cpu_to_le16(x) \
    ((((x) & 0xff00)     >> 8) + \
     (((x) & 0xff)       << 8));
#endif

/* Need MAX_RADIOS + 1 PCAP buffers, one for each of the DSRC radios and the
 * last one for the C-V2X radio */
#define PCAP_BUF_COUNT (MAX_RADIOS + 1)

/* enums */
enum {
    sniffrm           = 0x00000044,
    sniffrm_hosttime  = 0x00010044,
    sniffrm_mactime   = 0x00020044,
    sniffrm_channel   = 0x00030044,
    sniffrm_rssi      = 0x00040044,
    sniffrm_sq        = 0x00050044,
    sniffrm_signal    = 0x00060044,
    sniffrm_noise     = 0x00070044,
    sniffrm_rate      = 0x00080044,
    sniffrm_istx      = 0x00090044,
    sniffrm_frmlen    = 0x000A0044
};

/* typedefs */
typedef struct pcap {
    bool_t   enabled;           /* PCAP enabled for this radio */
    uint32_t mask;              /* PCAP mask */
    sem_t    lock;              /* lock for PCAP buffer */
    uint8_t  log_buf[PCAP_LOG_BUF_SIZE];
                                /* buffer for PCAP logged packets */
    uint32_t bytes_logged;      /* number of PCAP bytes logged */
    uint8_t  tx_log_buf[PCAP_TX_LOG_BUF_SIZE];
                                /* buffer for PCAP logged Tx packets */
    uint32_t tx_bytes_logged;   /* number of PCAP Tx bytes logged */
} pcapT;

typedef struct {
	uint32_t did;
	uint16_t status;
	uint16_t len;
	uint32_t data;
} p80211ItemUint32T;

typedef struct {
	uint32_t          msgcode;
	uint32_t          msglen;
	uint8_t           devname[PCAP_MAX_DEV_NAME_LENGTH];
	p80211ItemUint32T hosttime;
	p80211ItemUint32T mactime;
	p80211ItemUint32T channel;
	p80211ItemUint32T rssi;
	p80211ItemUint32T sq;
	p80211ItemUint32T signal;
	p80211ItemUint32T noise;
	p80211ItemUint32T rate;
	p80211ItemUint32T istx;
	p80211ItemUint32T frmlen;
} prismHdrT;

typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;      /* timestamp seconds */
    uint32_t ts_usec;     /* timestamp microseconds */
    uint32_t incl_len;    /* number of octets of packet saved in the file */
    uint32_t orig_len;    /* actual length of packet */
} pcapHdrT;

/* Local Variables */
static pcapT pcap[PCAP_BUF_COUNT];

#ifdef ENABLE_NS_PCAP_LOG
#define TOOK_READ  1
#define TOOK_WRITE 2
static int last_took = 0;
#endif

/* RSK error states. */
extern rskStatusType cv2xStatus;
/* metrics and reporting output. */
static uint32_t pcap_write_rolling_counter = 0x0;
static uint32_t pcap_read_rolling_counter = 0x0;

/* Functions */
/*------------------------------------------------------------------------------
** Function: pcap_init
**
** @brief    Initialize PCAP at startup
**
** @param    none
**
** @return   0 for success, -1 for failure
**----------------------------------------------------------------------------*/
int pcap_init(void)
{
    uint8_t radioNum;
    uint8_t i;

#ifdef ENABLE_NS_PCAP_LOG
    system("rm /mnt/rwflash/dbglogs/ns_pcap.log\n");
#endif

    /* For each radio, initialize its lock and initialize its PCAP parameters */
    for (radioNum = 0; radioNum < PCAP_BUF_COUNT; radioNum++) {
        if (sem_init(&pcap[radioNum].lock, 0, 1) < 0) {
            /* Something went wrong, abort */
            printf("%s: Error initializing PCAP lock: %s\n",
                     __func__, strerror(errno));
            fflush(stdout);

            /* Destroy all locks created so up until now */
            for (i = 0; i < radioNum; i++) {
                sem_destroy(&pcap[i].lock);
            }

            NS_PCAP_LOG("%s() failed sem_init()\n", __func__);
            return -1;
        }

        pcap[radioNum].enabled         = FALSE;
        pcap[radioNum].mask            = 0;
        pcap[radioNum].bytes_logged    = 0;
        pcap[radioNum].tx_bytes_logged = 0;
    }
    pcap_write_rolling_counter = 0x0;
    pcap_read_rolling_counter = 0x0;
    NS_PCAP_LOG("%s() succeeded\n", __func__);
    return 0;
}

/*------------------------------------------------------------------------------
** Function: pcap_term
**
** @brief    Terminate PCAP at shutdown
**
** @param    none
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_term(void)
{
    int radioNum;

    /* For each radio, disable it and destroy its lock */
    for (radioNum = 0; radioNum < PCAP_BUF_COUNT; radioNum++) {
        pcap[radioNum].enabled = FALSE;
        sem_destroy(&pcap[radioNum].lock);
    }

    NS_PCAP_LOG("%s()\n", __func__);
}

/*------------------------------------------------------------------------------
** Function: getPcapIdx
**
** @brief    Convert radioType and radioNum into the PCAP buffer to use.
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
**
** @return   Which PCAP buffer to use
**----------------------------------------------------------------------------*/
static int getPcapIdx(rsRadioType radioType, uint8_t radioNum)
{
    /* Use the last PCAP buffer for C-V2X */
    if (radioType == RT_CV2X) {
        return PCAP_BUF_COUNT - 1;
    }

    /* Otherwise, use one of the first MAX_RADIOS buffers */
    return (int)radioNum;
}

/*------------------------------------------------------------------------------
** Function: pcap_enable
**
** @brief    Enable PCAP
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
** @param    mask      -- Value to set the PCAP mask to. If
**                        PCAP_ALL_DEVICES_SHARED is set in the mask, radioType
**                        and radioNum are ignored and all DSRC radios are
**                        enabled.
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_enable(rsRadioType radioType, uint8_t radioNum, uint32_t mask)
{
    int pcapIdx;

    /* If PCAP_ALL_DEVICES_SHARED set, enable all DSRC PCAPs */
    if (mask & PCAP_ALL_DEVICES_SHARED) {
        for (radioNum = 0; radioNum < MAX_RADIOS; radioNum++) {
            pcapIdx = getPcapIdx(RT_DSRC, radioNum);
            pcap[pcapIdx].mask    = mask;
            pcap[pcapIdx].enabled = TRUE;
        }

        NS_PCAP_LOG("%s() for all DSRC radios\n", __func__);
    }
    else {
        /* Range check radioType and radioNum */
        if (((radioType != RT_CV2X) && (radioType != RT_DSRC)) ||
            ((radioType == RT_DSRC) && (radioNum >= MAX_RADIOS))) {
            NS_PCAP_LOG("%s(%d, %u, 0x%x) failed range checking\n",
                        __func__, radioType, radioNum, mask);
            return;
        }

        /* Enable this radio and set its mask */
        pcapIdx = getPcapIdx(radioType, radioNum);
        pcap[pcapIdx].mask    = mask;
        pcap[pcapIdx].enabled = TRUE;
        NS_PCAP_LOG("%s(%d, %u, 0x%x) succeeded\n",
                    __func__, radioType, radioNum, mask);
    }
}

/*------------------------------------------------------------------------------
** Function: pcap_disable
**
** @brief    Disable PCAP. If PCAP was enabled with PCAP_ALL_DEVICES_SHARED set
**           in the mask, radioNum and radioNum are ignored and all DSRC radios
**           are disabled.
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_disable(rsRadioType radioType, uint8_t radioNum)
{
    int pcapIdx;

    /* If PCAP_ALL_DEVICES_SHARED set, disable all DSRC PCAPs */
    if (pcap[0].mask & PCAP_ALL_DEVICES_SHARED) {
        for (radioNum = 0; radioNum < MAX_RADIOS; radioNum++) {
            pcapIdx = getPcapIdx(RT_DSRC, radioNum);
            pcap[pcapIdx].enabled = FALSE;
        }

        NS_PCAP_LOG("%s() for all DSRC radios\n", __func__);
    }
    else {
        /* Range check radioType and radioNum */
        if (((radioType != RT_CV2X) && (radioType != RT_DSRC)) ||
            ((radioType == RT_DSRC) && (radioNum >= MAX_RADIOS))) {
            NS_PCAP_LOG("%s(%d, %u) failed range checking\n",
                        __func__, radioType, radioNum);
            return;
        }

        /* Disable this radio */
        pcapIdx = getPcapIdx(radioType, radioNum);
        pcap[pcapIdx].enabled = FALSE;
        NS_PCAP_LOG("%s(%d, %u) succeeded\n", __func__, radioType, radioNum);
    }
}

/*------------------------------------------------------------------------------
** Function: pcap_enabled
** @brief    Returns whether or not PCAP is enabled for a given radio
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
** @param    tx        -- TRUE if for Tx packet; FALSE if for Rx packet
**
** @return   TRUE if enabled. FALSE otherwise.
**----------------------------------------------------------------------------*/
bool_t pcap_enabled(rsRadioType radioType, uint8_t radioNum, bool_t tx)
{
    int pcapIdx;

    /* Range check radioType and radioNum */
    if (((radioType != RT_CV2X) && (radioType != RT_DSRC)) ||
        ((radioType == RT_DSRC) && (radioNum >= MAX_RADIOS))) {
        return FALSE;
    }

    pcapIdx = getPcapIdx(radioType, radioNum);

    /* FALSE if PCAP not enabled */
    if (!pcap[pcapIdx].enabled) {
        return FALSE;
    }

    /* FALSE if tx and PCAP_TX not set */
    if (tx && !(pcap[pcapIdx].mask & PCAP_TX)) {
        return FALSE;
    }

    /* FALSE if rx and PCAP_RX not set */
    if (!tx && !(pcap[pcapIdx].mask & PCAP_RX)) {
        return FALSE;
    }

    /* TRUE otherwise */
    NS_PCAP_LOG("%s(%d, %u, %d) returned TRUE\n",
                __func__, radioType, radioNum, tx);
    return TRUE;
}

/*------------------------------------------------------------------------------
** Function: pcap_write
** @brief    Write a packet to the PCAP buffer. Creates the PCAP and Prism
**           headers, then logs the headers and the packet data to the PCAP
**           buffer.
**
** @param    data       -- Pointer to data to be written
** @param    len        -- Length of the data to be written
** @param    radioType  -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum   -- Radio number. Must be < PCAP_BUF_COUNT.
** @param    channelNum -- Channel number
** @param    pwr        -- Tx power (Tx) or RSSI (Rx)
** @param    rate       -- Tx or Rx rate
** @param    tx         -- TRUE if Tx packet; FALSE if Rx packet
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_write(void *data, uint32_t len, rsRadioType radioType,
                uint8_t radioNum, uint8_t channelNum, uint8_t pwr,
                uint8_t rate, bool_t tx)
{
    struct timeval  current_time;
    int             pcapIdx;
    int             bufferNum;
    bool_t          use_tx_buf;
    bool_t          includePrismHdr;
    int             prismHdrSize;
    prismHdrT       prismHdr;
    pcapHdrT        pcapHdr;
    uint8_t        *buf;
    uint32_t        bytes_logged;
    uint32_t        log_buf_size;

    /* Do range checking */
    if ((len == 0) ||
        ((radioType != RT_CV2X) && (radioType != RT_DSRC)) ||
        ((radioType == RT_DSRC) && (radioNum >= MAX_RADIOS))) {
        NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) "
                    "failed range checking\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx);
        return;
    }

    pcapIdx = getPcapIdx(radioType, radioNum);

    /* Return if not enabled */
    if (!pcap[pcapIdx].enabled) {
        NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) not enabled\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx);
        return;
    }

    /* If all devices shared, use the first log buffer (normally used for
     * radio 0) */
    bufferNum = (pcap[pcapIdx].mask & PCAP_ALL_DEVICES_SHARED) ? 0 : pcapIdx;
    /* Get time for timestamp */
    gettimeofday(&current_time, NULL);
    /* Determine which buffer to use */
    use_tx_buf = (tx && (pcap[pcapIdx].mask & PCAP_TX_RX_SEPARATE));
    /* Determine whether or not to include Prism header */
    includePrismHdr = (pcap[pcapIdx].mask & PCAP_PRISM_HDR);

    /* If Prism header is to be included, fill it out */
    if (includePrismHdr) {
        prismHdrSize = sizeof(prismHdrT);
        memset(&prismHdr, 0, sizeof(prismHdr));

        prismHdr.msgcode = cpu_to_le32(sniffrm);
        prismHdr.msglen  = cpu_to_le32(prismHdrSize);
        
        if (radioType == RT_CV2X) {
            snprintf((char *)prismHdr.devname, PCAP_MAX_DEV_NAME_LENGTH,
                     "cv2x0");
        }
        else {
            snprintf((char *)prismHdr.devname, PCAP_MAX_DEV_NAME_LENGTH,
                     "dsrc%u", radioNum);
        }

        prismHdr.istx.did  = cpu_to_le32(sniffrm_istx);
        prismHdr.istx.len  = cpu_to_le16(4);
        prismHdr.istx.data = tx ? 1 : 0;

        prismHdr.frmlen.did  = cpu_to_le32(sniffrm_frmlen);
        prismHdr.frmlen.len  = cpu_to_le16(4);
        prismHdr.frmlen.data = cpu_to_le32(len);

        prismHdr.channel.did  = cpu_to_le32(sniffrm_channel);
        prismHdr.channel.len  = cpu_to_le16(4);
        prismHdr.channel.data = cpu_to_le32(channelNum);

        prismHdr.rssi.did  = cpu_to_le32(sniffrm_rssi);
        prismHdr.rssi.len  = cpu_to_le16(4);
        prismHdr.rssi.data = cpu_to_le32(pwr);

        prismHdr.signal.did  = cpu_to_le32(sniffrm_signal);
        prismHdr.signal.len  = cpu_to_le16(4);
        prismHdr.signal.data = tx ? 0 : cpu_to_le32(pwr);

        prismHdr.rate.did  = cpu_to_le32(sniffrm_rate);
        prismHdr.rate.len  = cpu_to_le16(4);
        prismHdr.rate.data = cpu_to_le32(rate);
    }
    else {
        prismHdrSize = 0;
    }

    /* Fill out the PCAP record header */
    pcapHdr.ts_sec   = current_time.tv_sec;
    pcapHdr.ts_usec  = current_time.tv_usec;
    pcapHdr.incl_len = len + prismHdrSize;
    pcapHdr.orig_len = len + prismHdrSize;
#if 0
    sem_wait(&pcap[bufferNum].lock);
    {
#ifdef ENABLE_NS_PCAP_LOG
        if (last_took != TOOK_WRITE) {
            last_took = TOOK_WRITE;
            NS_PCAP_LOG("\n");
            NS_PCAP_LOG("%s() took &pcap[%d].lock\n", __func__, bufferNum);
        }
#endif

        /* Get the proper pointers and values */
        if (use_tx_buf) {
            buf          = pcap[bufferNum].tx_log_buf;
            bytes_logged = pcap[bufferNum].tx_bytes_logged;
            log_buf_size = PCAP_TX_LOG_BUF_SIZE;
        }
        else {
            buf          = pcap[bufferNum].log_buf;
            bytes_logged = pcap[bufferNum].bytes_logged;
            log_buf_size = PCAP_LOG_BUF_SIZE;
        }

        /* Don't log if not enough room in buffer */
        if (sizeof(pcapHdrT) + prismHdrSize + len >
            log_buf_size - bytes_logged) {
            printf("PCAP: No room in PCAP buffer for %cx packet!\n",
                     use_tx_buf ? 'T' : 'R');
            fflush(stdout);
            NS_PCAP_LOG("%s() posting &pcap[%d].lock\n", __func__, bufferNum);
            sem_post(&pcap[bufferNum].lock);
            NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) no room\n",
                        __func__, data, len, radioType, radioNum, channelNum,
                        pwr, rate, tx);
            return;
        }

        /* Write the PCAP record header */
        memcpy(&buf[bytes_logged], &pcapHdr, sizeof(pcapHdrT));
        bytes_logged += sizeof(pcapHdrT);

        /* If Prism header is to be included, write it out */
        if (includePrismHdr) {
            memcpy(&buf[bytes_logged], &prismHdr, prismHdrSize);
            bytes_logged += prismHdrSize;
        }

        /* Write out the packet data */
        memcpy(&buf[bytes_logged], data, len);
        bytes_logged += len;

        /* Save the new number of bytes logged */
        if (use_tx_buf) {
            pcap[bufferNum].tx_bytes_logged = bytes_logged;
        }
        else {
            pcap[bufferNum].bytes_logged = bytes_logged;
        }
    }

#ifdef ENABLE_NS_PCAP_LOG
    if (use_tx_buf) {
        NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) succeeded; "
                    "pcap[bufferNum].tx_bytes_logged = %u\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx, pcap[bufferNum].tx_bytes_logged);
    }
    else {
        NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) succeeded;"
                    " pcap[bufferNum].bytes_logged = %u\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx, pcap[bufferNum].bytes_logged);
    }

    NS_PCAP_LOG("%s() posting &pcap[%d].lock\n", __func__, bufferNum);
#endif
    sem_post(&pcap[bufferNum].lock);
#else /* If locked, drop data, and keep going. Don't hang around. */
    if(0 == sem_trywait(&pcap[bufferNum].lock)) {
        #ifdef ENABLE_NS_PCAP_LOG
        if (last_took != TOOK_WRITE) {
            last_took = TOOK_WRITE;
            NS_PCAP_LOG("\n");
            NS_PCAP_LOG("%s() took &pcap[%d].lock\n", __func__, bufferNum);
        }
        #endif

        /* Get the proper pointers and values */
        if (use_tx_buf) {
            buf          = pcap[bufferNum].tx_log_buf;
            bytes_logged = pcap[bufferNum].tx_bytes_logged;
            log_buf_size = PCAP_TX_LOG_BUF_SIZE;
        }
        else {
            buf          = pcap[bufferNum].log_buf;
            bytes_logged = pcap[bufferNum].bytes_logged;
            log_buf_size = PCAP_LOG_BUF_SIZE;
        }

        /* Don't log if not enough room in buffer */
        if (sizeof(pcapHdrT) + prismHdrSize + len >
            log_buf_size - bytes_logged) {
            printf("PCAP: No room in PCAP buffer for %cx packet!\n",
                     use_tx_buf ? 'T' : 'R');
            fflush(stdout);
            NS_PCAP_LOG("%s() posting &pcap[%d].lock\n", __func__, bufferNum);
            sem_post(&pcap[bufferNum].lock);
            NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) no room\n",
                        __func__, data, len, radioType, radioNum, channelNum,
                        pwr, rate, tx);
        } else {

            /* Write the PCAP record header */
            memcpy(&buf[bytes_logged], &pcapHdr, sizeof(pcapHdrT));
            bytes_logged += sizeof(pcapHdrT);

            /* If Prism header is to be included, write it out */
            if (includePrismHdr) {
                memcpy(&buf[bytes_logged], &prismHdr, prismHdrSize);
                bytes_logged += prismHdrSize;
            }

            /* Write out the packet data */
            memcpy(&buf[bytes_logged], data, len);
            bytes_logged += len;

            /* Save the new number of bytes logged */
            if (use_tx_buf) {
                pcap[bufferNum].tx_bytes_logged = bytes_logged;
            } else {
                pcap[bufferNum].bytes_logged = bytes_logged;
            }
            #ifdef ENABLE_NS_PCAP_LOG
            if (use_tx_buf) {
                NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) succeeded; "
                    "pcap[bufferNum].tx_bytes_logged = %u\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx, pcap[bufferNum].tx_bytes_logged);
            } else {
                NS_PCAP_LOG("%s(%p, %u, %d, %u, %d, %u, %u, %d) succeeded;"
                    " pcap[bufferNum].bytes_logged = %u\n",
                    __func__, data, len, radioType, radioNum, channelNum, pwr,
                    rate, tx, pcap[bufferNum].bytes_logged);
            }

            NS_PCAP_LOG("%s() posting &pcap[%d].lock\n", __func__, bufferNum);
            #endif
        }
        sem_post(&pcap[bufferNum].lock);
    } else {
        cv2xStatus.error_states |= CV2X_PCAP_WRITE_SEM_WAIT_FAIL;
        if(0 == (pcap_write_rolling_counter % 100)){
            printf("pcap_write sem fail lock(%u)\n",pcap_write_rolling_counter);
        }
        pcap_write_rolling_counter++;
    }
#endif
}

/*------------------------------------------------------------------------------
** Function: pcap_read1
**
** @brief    First half of the read from PCAP buffer function. Returns an
**           address and length that the caller can read from. Leaves the
**           semaphore taken so that the read can be safely done. Reads up to
**           PCAP_LOG_BUF_SIZE bytes for Rx and PCAP_TX_LOG_BUF_SIZE bytes for
**           Tx. Be sure destination buffer is large enough.
**
** @param    radioType    -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum     -- The radio number. Must be < PCAP_BUF_COUNT.
** @param    tx           -- If TRUE, and the PCAP_TX_RX_SEPARATE flag is set
**                           in pcap[radioNum].mask, use tx_log_buf; else use
**                           log_buf
** @param    buf          -- Place to store a pointer to where to read from.
** @param    len          -- Number of bytes to read is written here.
** @param    outBufferNum -- A number is written to here to pass to
**                           pcap_read2().
**
** @return   0 for success, -1 for failure
**----------------------------------------------------------------------------*/
int pcap_read1(rsRadioType radioType, uint8_t radioNum, bool_t tx,
               uint8_t **buf, uint32_t *len, uint8_t *outBufferNum)
{
    int    pcapIdx;
    int    bufferNum;
    bool_t use_tx_buf;

    /* Return error if invalid radio type or number. Note that we want to allow
     * reading from the buffer after capture has been disabled so that all
     * remaining data can be read. */
    if (((radioType != RT_CV2X) && (radioType != RT_DSRC)) ||
        ((radioType == RT_DSRC) && (radioNum >= MAX_RADIOS))) {
        NS_PCAP_LOG("%s(%d, %u, %d, %p, %p, %p) failed range checking\n",
                    __func__, radioType, radioNum, tx, buf, len, outBufferNum);
        return -1;
    }

    *len = 0;
    /* If all devices shared, use the first log buffer (normally used for radio
     * 0) */
    pcapIdx = getPcapIdx(radioType, radioNum);
    bufferNum = (pcap[pcapIdx].mask & PCAP_ALL_DEVICES_SHARED) ? 0 : pcapIdx;
    /* Determine which buffer to use */
    use_tx_buf = (tx && (pcap[pcapIdx].mask & PCAP_TX_RX_SEPARATE));
    /* Initialize the buffer and read pointers, and adjust byte count */
#if 0
    sem_wait(&pcap[bufferNum].lock);
#else /* If locked, keep going, don't hang. */
    if(0 == sem_trywait(&pcap[bufferNum].lock)) {

#ifdef ENABLE_NS_PCAP_LOG
    if (last_took != TOOK_READ) {
        last_took = TOOK_READ;
        NS_PCAP_LOG("\n");
    }

    NS_PCAP_LOG("%s() took &pcap[%d].lock\n", __func__, bufferNum);
#endif

    /* Get the proper pointers and values, and set the number of bytes
     * logged to zero (i.e. reset the buffers) */
    if (use_tx_buf) {
        *buf = pcap[bufferNum].tx_log_buf;
        *len = pcap[bufferNum].tx_bytes_logged;
        pcap[bufferNum].tx_bytes_logged = 0;
    }
    else {
        *buf = pcap[bufferNum].log_buf;
        *len = pcap[bufferNum].bytes_logged;
        pcap[bufferNum].bytes_logged = 0;
    }
    
    *outBufferNum = bufferNum;
    NS_PCAP_LOG("%s(%d, %u, %d, %p, %p, %p) succeeded; "
                "buf = %p; len = %u; outBufferNum = %u\n",
                __func__, radioType, radioNum, tx, buf, len, outBufferNum,
                *buf, *len, *outBufferNum);
    } else {
        cv2xStatus.error_states |= CV2X_PCAP_READ_SEM_WAIT_FAIL;
        if(0 == (pcap_read_rolling_counter % 100)){
            printf("pcap_read sem fail lock(%u)\n",pcap_read_rolling_counter);
        }
        pcap_read_rolling_counter++;
    }
#endif
    return 0;
}

/*------------------------------------------------------------------------------
** Function: pcap_read2
**
** @brief    Second half of the read from PCAP buffer function. Unlocks the
**           semaphore so that writing to the buffer can occur again.
**
** @param    bufferNum -- Indicates which semaphore to unlock.
**
** @return   void
**----------------------------------------------------------------------------*/
int32_t pcap_read2(uint8_t bufferNum)
{
    NS_PCAP_LOG("%s() posting &pcap[%d].lock\n", __func__, bufferNum);
    sem_post(&pcap[bufferNum].lock);

    return 0;
}

