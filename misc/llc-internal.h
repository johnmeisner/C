/**
 * @addtogroup cohda_llc LLC module
 * @{
 *
 * @addtogroup cohda_llc_intern LLC internals
 * @{
 *
 * @file
 * LLC internal header
 *
 * This header file is for declarations and definitions internal to
 * the llc module. For public APIs and documentation, see
 * include/cohda/llc/llc.h and include/linux/cohda/llc/llc.h
 */

//------------------------------------------------------------------------------
// Copyright (c) 2013 Cohda Wireless Pty Ltd
//------------------------------------------------------------------------------

#ifndef __DRIVERS__COHDA__LLC__LLC_INTERNAL_H__
#define __DRIVERS__COHDA__LLC__LLC_INTERNAL_H__
#ifdef __KERNEL__

//------------------------------------------------------------------------------
// Included headers
//------------------------------------------------------------------------------
#include "cohda/llc/llc.h"
#include "llc-netdev.h"
#include "llc-pseudo.h"
#include "llc-monitor.h"
#include "llc-thread.h"
#include "llc-list.h"
#include "llc-msg.h"
#include "llc-usb.h"
#include "llc-transfer.h"
#include "llc-spi.h"
#include "llc-firmware.h"
#include "llc-ipv6.h"
#include "llc-device.h"

//------------------------------------------------------------------------------
// Macros & Constants
//------------------------------------------------------------------------------

// Don't enable the Linux 'module' exports if we're unit testing
#ifdef UNITTEST
#undef module_init
#define module_init(ARG)
#undef module_exit
#define module_exit(ARG)
#undef MODULE_AUTHOR
#define MODULE_AUTHOR(ARG)
#undef MODULE_DESCRIPTION
#define MODULE_DESCRIPTION(ARG)
#undef MODULE_LICENSE
#define MODULE_LICENSE(ARG)
#undef MODULE_VERSION
#define MODULE_VERSION(ARG)
#endif

/// Generic error code
#define LLC_STATUS_ERROR (-EINVAL)
/// Generic success value
#define LLC_STATUS_SUCCESS (0)

/* DENSO Changes for IPv6 - Begin */
#define MAX_IPV6_PACKET_SIZE       1500
#define ETHER_ADDR_LEN                6
#define ETHHDR_LEN                   14
#define ETHER_TYPE_LEN                2
#define ETHER_CRC_LEN                 4
#define ETHER_HDR_LEN                 ((ETHER_ADDR_LEN * 2) + ETHER_TYPE_LEN)
#define ETHER_MIN_LEN                64
#define ETHER_MAX_LEN              1518
#define ETHER_MAX_LEN_JUMBO        9018
#define ETHHDR_DEST_MAC_POSN          0
#define ETHHDR_SRC_MAC_POSN           6
#define ETHHDR_ETHERTYPE_POSN        12

#define IPV6_HDR_SRC_ADDR_POSN        8
#define IPV6_HDR_DEST_ADDR_POSN      24

#define QOSHDR_LEN                   26
#define QOSHDR_DEST_MAC_POSN          4
#define QOSHDR_SRC_MAC_POSN          10

#define ETHERTYPE_LEN                 2
#define IPV6_ETHERTYPE_HI_BYTE     0x86
#define IPV6_ETHERTYPE_LO_BYTE     0xdd

#define DEFAULT_CONFIGURED_ANTENNA    4
#define IPV6_MAX_TX_POWER_DBM        23
#define IPV6_MAX_TX_PRIORITY          7

#define IPV6_ADDR_LEN                16
#define MAX_NDP_TABLE_ENTRIES       100
#define MAX_IPV6_ADDR_STR_LEN        45
#define MAX_ETHER_ADDR_STR_LEN       17
#define NDP_EXPIRE_TIME_S            60
/* DENSO Changes for IPv6 - End */

/// No SPI download
#define LLC_SPI_FIRMWARE_MODE_NONE (0)
/// Module parameter for SPI download of DFU bootloader
#define LLC_SPI_FIRMWARE_MODE_DFU (1)
/// Module parameter for SPI download of full firmware
#define LLC_SPI_FIRMWARE_MODE_FULL (2)
/// Module parameter for SPI download of bootloader image
#define LLC_SPI_FIRMWARE_MODE_BOOTLOADER (3)
/// Module parameter for SPI download of fast boot image
#define LLC_SPI_FIRMWARE_MODE_FASTBOOT (4)

/// Module parameter for transfer mode USB
#define LLC_TRANSFER_MODE_USB (0)
/// Module parameter for transfer mode SPI
#define LLC_TRANSFER_MODE_SPI (1)

/// Module parameter for default SPI clock frequency - MHz
#define LLC_DEFAULT_SPI_FREQUENCY (15)
/// Module parameter for default SPI bits per word
#define LLC_DEFAULT_SPI_BITSPERWORD (32)
/// Module parameter for default SPI-MARS Control
/// 0 Streaming/Polling
/// 1 HW Interrupt
#define LLC_SPI_MODE_STREAMING (0)
#define LLC_SPI_MODE_INTERRUPT (1)
#define LLC_DEFAULT_SPI_MODE_CONTROL (LLC_SPI_MODE_STREAMING)

/// Max MTU for cw-llc
#define LLC_MAX_MTU (2400)

//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
// Kernel module related internal functions
int LLC_Setup (struct LLCDriver *pDrv);
int LLC_Release (struct LLCDriver *pDrv);
tMKxStatus LLC_Start (struct MKx *pMKx);
tMKxStatus LLC_Stop (struct MKx *pMKx);

// LLCRemote (MKx) API
tMKxStatus LLC_Config (struct MKx *pMKx,
                       tMKxRadio Radio,
                       tMKxRadioConfig *pConfig);
tMKxStatus LLC_TxReq (struct MKx *pMKx,
                      tMKxTxPacket *pTxPkt,
                      void *pFrmPriv);
tMKxStatus LLC_TxCnf (struct MKx *pMKx,
                      tMKxTxPacket *pTxPkt,
                      const tMKxTxEvent *pTxEvent,
                      void *pPriv);
tMKxStatus LLC_TxFlush (struct MKx *pMKx,
                        tMKxRadio Radio,
                        tMKxChannel Chan,
                        tMKxTxQueue TxQ);
tMKxStatus LLC_RxAlloc (struct MKx *pMKx,
                        int BufLen,
                        uint8_t **ppBuf,
                        void **ppPriv);
tMKxStatus LLC_RxInd (struct MKx *pMKx,
                      tMKxRxPacket *pRxPkt,
                      void *pPriv);
tMKxStatus LLC_NotifInd (struct MKx *pMKx,
                         tMKxNotif Notif);
tMKxStatus LLC_TempCfg (struct MKx *pMKx,
                        tMKxTempConfig *pCfg);
tMKxStatus LLC_Temp (struct MKx *pMKx,
                     tMKxTemp *pTemp);
tMKxStatus LLC_PowerDetCfg (struct MKx *pMKx,
                            tMKxPowerDetConfig *pCfg);
tMKxStatus LLC_GetTSFReq (struct MKx *pMKx);
tMKxStatus LLC_GetTSFInd (struct MKx *pMKx, tMKxTSF TSF);
tMKxStatus LLC_SetTSF (struct MKx *pMKx, tMKxSetTSF *pSetTSF);
tMKxStatus LLC_DebugReq (struct MKx *pMKx,
                         struct MKxIFMsg *pMsg);
tMKxStatus LLC_DebugInd (struct MKx *pMKx,
                         struct MKxIFMsg *pMsg);
tMKxStatus LLC_C2XSecReq (struct MKx *pMKx,
                          tMKxC2XSec *pMsg);
tMKxStatus LLC_C2XSecInd (struct MKx *pMKx,
                          tMKxC2XSec *pMsg);
int32_t LLC_GetLoopback (void);
uint32_t LLC_GetSPIDownload(void);
uint32_t LLC_GetTransferMode(void);
uint32_t LLC_GetSPIClockFrequencyMHz(void);
uint32_t LLC_GetSPIBitsPerWord(void);
uint32_t LLC_GetSPIMode(void);
uint32_t LLC_GetReset(void);
char *LLC_GetSDRImageName(void);
uint32_t LLC_GetIPv6Enabled(void);
uint32_t LLC_GetIPv6MCS(void);
/* Start DENSO Changes */
uint32_t LLC_GetIPv6TxPwr(void);
/* End DENSO Changes */
bool LLC_IsSilent (void);

static inline void LLC_SemInit (struct semaphore *pSem, int Val)
{
  sema_init(pSem, Val);
}

static inline int LLC_SemDown (struct semaphore *pSem)
{
  if (1)
    return down_timeout(pSem, msecs_to_jiffies(1000));
  else
    return down_killable(pSem);
}

static inline void LLC_SemUp (struct semaphore *pSem)
{
  up(pSem);
}

#else
#error This file should not be included from user space.
#endif // #ifdef __KERNEL__
#endif // #ifndef __DRIVERS__COHDA__LLC__LLC_INTERNAL_H__
/**
 * @}
 * @}
 */
