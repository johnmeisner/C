/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ns_pcap.h                                                        */
/*  Purpose: For implementation of the capturing of PCAP data                 */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef __NS_PCAP_H__
#define __NS_PCAP_H__

#include "ris_struct.h"

/* Bit definitions for sc_pcap_mask */
#define PCAP_TX                 0x01
#define PCAP_RX                 0x02
#define PCAP_TX_RX_SEPARATE     0x04
#define PCAP_ALL_DEVICES_SHARED 0x08
#define PCAP_PRISM_HDR          0x10

/* Functions */
/*------------------------------------------------------------------------------
** Function: pcap_init
**
** @brief    Initialize pcap at startup
**
** @param    none
**
** @return   0 for success, -1 for failure
**----------------------------------------------------------------------------*/
int pcap_init(void);

/*------------------------------------------------------------------------------
** Function: pcap_term
**
** @brief    Terminate pcap at shutdown
**
** @param    none
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_term(void);

/*------------------------------------------------------------------------------
** Function: pcap_enable
**
** @brief    Enable PCAP
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
** @param    mask      -- Value to set the PCAP mask to. If
**                        PCAP_ALL_DEVICES_SHARED is set in the mask, radioType
**                        and radioNum are ignored and all radios are enabled.
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_enable(rsRadioType radioType, uint8_t radioNum, uint32_t mask);

/*------------------------------------------------------------------------------
** Function: pcap_disable
**
** @brief    Disable PCAP. If PCAP was enabled with PCAP_ALL_DEVICES_SHARED set
**           in the mask, radioNum and radioNum are ignored and all radios are
**           disabled.
**
** @param    radioType -- Radio type. RT_CV2X or RT_DSRC.
** @param    radioNum  -- Radio number. Must be < MAX_RADIOS.
**
** @return   void
**----------------------------------------------------------------------------*/
void pcap_disable(rsRadioType radioType, uint8_t radioNum);

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
bool_t pcap_enabled(rsRadioType radioType, uint8_t radioNum, bool_t tx);

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
                uint8_t rate, bool_t tx);

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
** @param    tx           -- If TRUE, and the PCAP_TX_RX_SEPARATE flag is set in
**                           pcap[radioNum].mask, use tx_log_buf; else use
**                           log_buf
** @param    buf          -- Place to store a pointer to where to read from.
** @param    len          -- Number of bytes to read is written here.
** @param    outBufferNum -- A number is written to here to pass to
**                           pcap_read2().
**
** @return   0 for success, -1 for failure
**----------------------------------------------------------------------------*/
int pcap_read1(rsRadioType radioType, uint8_t radioNum, bool_t tx,
               uint8_t **buf, uint32_t *len, uint8_t *outBufferNum);

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
int32_t pcap_read2(uint8_t bufferNum);
#endif // __NS_PCAP_H__

