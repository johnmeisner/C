/**
 * @addtogroup cohda_llc LLC module
 * @{
 *
 * @addtogroup cohda_llc_intern LLC internals
 * @{
 *
 * @section cohda_llc_ko LLC kernel module
 *
 * @file
 * LLC: kernel module
 *
 */

//------------------------------------------------------------------------------
// Copyright (c) 2013 Cohda Wireless Pty Ltd
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Included headers
//------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>

#include "cohda/llc/llc.h"
#include "cohda/llc/llc-api.h"
#include "llc-internal.h"

#define D_SUBMODULE LLC_Module
#include "debug-levels.h"

//------------------------------------------------------------------------------
// Macros & Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------

/// Debug framework control of debug levels
struct d_level D_LEVEL[] =
{
  D_SUBMODULE_DEFINE(LLC_Module),
  D_SUBMODULE_DEFINE(LLC_API),
  D_SUBMODULE_DEFINE(LLC_Device),
  D_SUBMODULE_DEFINE(LLC_NetDev),
  D_SUBMODULE_DEFINE(LLC_Netlink),
  D_SUBMODULE_DEFINE(LLC_Monitor),
  D_SUBMODULE_DEFINE(LLC_PseudoPHY),
  D_SUBMODULE_DEFINE(LLC_List),
  D_SUBMODULE_DEFINE(LLC_Msg),
  D_SUBMODULE_DEFINE(LLC_Thread),
  D_SUBMODULE_DEFINE(LLC_USB),
  D_SUBMODULE_DEFINE(LLC_Transfer),
  D_SUBMODULE_DEFINE(LLC_Firmware),
  D_SUBMODULE_DEFINE(LLC_SPI),
  D_SUBMODULE_DEFINE(LLC_IPV6),
  D_SUBMODULE_DEFINE(LLC_Debug),
};
size_t D_LEVEL_SIZE = ARRAY_SIZE(D_LEVEL);

// Used by LLC_ModuleInit() to pass to LLC_Setup()
static struct LLCDriver __LLCDrv;

// (Re)set by LLC_Setup() and used by LLC_Init()
static struct LLCDriver *pLLCDrv = &(__LLCDrv);

/// Module parameter: Time when to stop tx / rx for geofencing etc as
/// seconds from Unix epoch
static ulong SilentTime = 0;
module_param (SilentTime, ulong, 0644);

/// Module parameter: Loopback enabled?
static uint32_t loopback = 0;
module_param(loopback, uint, S_IRUGO);

/// Module parameter: SPI download full firmware or DFU bootloader
static uint32_t SPIDownload = LLC_SPI_FIRMWARE_MODE_NONE;
module_param (SPIDownload, uint, 0644);

/// Module parameter: USB or SPI transfer mode
static uint32_t TransferMode = LLC_TRANSFER_MODE_USB;
module_param (TransferMode, uint, 0644);

/// Module parameter: SPI clock frequence in MHz
static uint32_t SPIClockFrequencyMHz = LLC_DEFAULT_SPI_FREQUENCY;
module_param (SPIClockFrequencyMHz, uint, 0644);

/// Module parameter: SPI bits per word
static uint32_t SPIBitsPerWord = LLC_DEFAULT_SPI_BITSPERWORD;
module_param (SPIBitsPerWord, uint, 0644);

/// Module parameter: SPI-MARS control
static uint32_t SPIMode = LLC_DEFAULT_SPI_MODE_CONTROL;
module_param (SPIMode, uint, 0644);

/// Module parameter: SDR firmware image
static char *SDRImage;
module_param (SDRImage, charp, 0);

/// Module parameter: Reset on module load enabled?
static uint32_t Reset = 1;
module_param(Reset, uint, S_IRUGO);

static uint32_t IPv6Enabled = 0;
module_param(IPv6Enabled, uint, 0644);

static uint32_t IPv6MCS = 10;
module_param(IPv6MCS, uint, 0644);

/* Start DENSO Changes */
static uint32_t IPv6TxPwr = 40;
module_param(IPv6TxPwr, uint, 0644);
/* End DENSO Changes */


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

/**
 * @brief Initialize the LLC and get a handle to it for further interaction
 * @param DevId Device number (0..LLC_DEV_CNT)
 * @param ppMKx Pointer to a MKx handle (populated by this function)
 * @return LLC_STATUS_SUCCESS (0) or a negative error code @sa eLLCStatus
 */
tMKxStatus MKx_Init (uint8_t DevId, struct MKx **ppMKx)
{
  int Res = LLC_STATUS_ERROR;
  struct LLCDevice *pDrv = NULL;

  d_fnstart(D_API, NULL, "(ppMKx %p)\n", ppMKx);

  if ((DevId >= LLC_DEV_CNT) || (ppMKx == NULL))
  {
    Res = -EINVAL;
    goto Error;
  }

  if (pLLCDrv == NULL)
  {
    Res = -ENOENT;
    goto Error;
  }
  pDrv = &(pLLCDrv->Dev[DevId]);
  if (pDrv == NULL)
  {
    Res = -ENODEV;
    goto Error;
  }

  *ppMKx = &(pDrv->MKx);
  Res = LLC_Start(*ppMKx);

Error:
  d_fnend(D_API, NULL, "(ppMKx %p) = %d\n", ppMKx, Res);
  return Res;
}
EXPORT_SYMBOL(MKx_Init);

/**
 * @brief De-initialize the LLC
 * @param pMKx handle obtained using MKx_Init()
 * @return LLC_STATUS_SUCCESS (0) or a negative error code @sa eLLCStatus
 */
tMKxStatus MKx_Exit (struct MKx *pMKx)
{
  int Res = LLC_STATUS_ERROR;

  d_fnstart(D_API, NULL, "(pMKx %p)\n", pMKx);

  Res = LLC_Stop(pMKx);

  d_fnend(D_API, NULL, "(pMKx %p) = %d\n", pMKx, Res);
  return Res;
}
EXPORT_SYMBOL(MKx_Exit);

/**
 * @brief Setup function for the LLC module
 * @param pDrv LLC structure to initialize
 * @return 0 for success, or a negative errno
 *
 * Initializes all the parts of the LLC module
 *
 */
int LLC_Setup (struct LLCDriver *pDrv)
{
  int Res = -ENODEV;
  int i;
  struct MKx *pMKx[LLC_DEV_CNT] = { NULL, NULL };

  d_fnstart(D_INTERN, NULL, "(pDrv %p)\n", pDrv);
  d_assert(pDrv != NULL);

  // pLLCDrv is used within MKx_Init()
  pLLCDrv = pDrv;
  pDrv->Magic = LLC_DRV_MAGIC;

  Res = LLC_DriverSetup(pDrv);
  if (Res != 0)
  {
    goto ErrorDevice;
  }

  for (i = 0; i < LLC_DEV_CNT; i++)
  {
    struct LLCDevice *pDev = &(pDrv->Dev[i]);

    pDev->Magic = LLC_DEV_MAGIC;
    pDev->pDriver = pDrv;

    pDev->MKx.Magic = MKX_API_MAGIC;
    Res = MKx_Init(i, &(pMKx[i]));
    if (Res != 0)
      goto ErrorLLCInit;
  }

  Res = 0;
  goto Success;

ErrorLLCInit:
  for (i = 0; i < LLC_DEV_CNT; i++)
  {
    if (pMKx[i] != NULL)
    {
      MKx_Exit(pMKx[i]);
    }
  }
  LLC_DriverRelease(pDrv);
ErrorDevice:
Success:
  d_fnend(D_INTERN, NULL, "(pDrv %p) = %d\n", pDrv, Res);
  return Res;
}
EXPORT_SYMBOL(LLC_Setup);

/**
 * @brief Release the LLC resources
 * @param pDrv LLC driver pointer
 * @return 0 for success or negative errno
 *
 * Undoes any setup done by LLC_Setup()
 *
 */
int LLC_Release (struct LLCDriver *pDrv)
{
  int Res = -ENODEV;
  int i;

  d_fnstart(D_INTERN, NULL, "(pDrv %p)\n", pDrv);
  d_assert(pDrv != NULL);

  // Release everything in the opposite order that it was created in
  // LLC_Setup()

  Res = 0;

  for (i = 0; i < LLC_DEV_CNT; i++)
  {
    struct MKx *pMKx = &(pDrv->Dev[i].MKx);
    Res += MKx_Exit(pMKx);
  }

  Res += LLC_DriverRelease(pDrv);

  d_fnend(D_INTERN, NULL, "(pDrv %p) = %d\n", pDrv, Res);
  return Res;
}
EXPORT_SYMBOL(LLC_Release);


/**
 * @brief Access the module parameter 'Loopback'
 */
int32_t LLC_GetLoopback(void)
{
  return loopback;
}

/**
 * @brief Access the module parameter 'SPIDownload'
 */
uint32_t LLC_GetSPIDownload(void)
{
  return SPIDownload;
}

/**
 * @brief Access the module parameter 'TransferMode'
 */
uint32_t LLC_GetTransferMode(void)
{
  return TransferMode;
}

/**
 * @brief Access the module parameter 'SPIBitsPerWord'
 */
uint32_t LLC_GetSPIBitsPerWord(void)
{
  return SPIBitsPerWord;
}

/**
 * @brief Access the module parameter 'SPIClockFrequency'
 * @brief Access the module parameter 'SDRImage'
 */
uint32_t LLC_GetSPIClockFrequencyMHz(void)
{
  return SPIClockFrequencyMHz;
}

/**
 * @brief Access the module parameter 'SPIMode'
 */
uint32_t LLC_GetSPIMode(void)
{
  return SPIMode;
}

/**
 * @brief Access the module parameter 'SDRImage'
 */
char *LLC_GetSDRImageName(void)
{
  return SDRImage;
}

/**
 * @brief Access the module parameter 'Reset'
 */
uint32_t LLC_GetReset(void)
{
  return Reset;
}


/**
 * @brief Access the module parameter 'IPv6Enabled'
 */
uint32_t LLC_GetIPv6Enabled(void)
{
  return IPv6Enabled;
}

/**
 * @brief Access the module parameter 'IPv6MCS'
 */
uint32_t LLC_GetIPv6MCS(void)
{
  return IPv6MCS;
}

/* Start DENSO Changes */
/**
 * @brief Access the module parameter 'IPv6TxPwr'
 */
uint32_t LLC_GetIPv6TxPwr(void)
{
  return IPv6TxPwr;
}
/* End DENSO Changes */


/**
 * @brief
 */


/**
 * @brief Determine if the LLC should block transmissions
 * This is done by accesing the module parameter 'SilentTime'
 */
bool LLC_IsSilent(void)
{
  bool Silent = false;

  if (SilentTime > 0)
  {
    struct timeval Now;

    do_gettimeofday(&Now);

    Silent = SilentTime <= Now.tv_sec;
  }
  return Silent;
}

/**
 * @brief Initialize the 'LLC' kernel module
 *
 */
static int __init LLC_ModuleInit(void)
{
  int Res = -ENOSYS;
  struct LLCDriver *pDrv = pLLCDrv;

  d_init();
  d_fnstart(D_INTERN, NULL, "(pDrv %p)\n", pDrv);

  memset(pDrv, 0, sizeof(struct LLCDriver));
  Res = LLC_Setup(pDrv);

  d_fnend(D_INTERN, NULL, "(pDrv %p) = %d\n", pDrv, Res);
  if (Res != 0)
    d_exit();
  return Res;
}
module_init(LLC_ModuleInit);

/**
 * @brief De-Initialize the 'LLC' kernel module
 *
 */
static void __exit LLC_ModuleExit(void)
{
  struct LLCDriver *pDrv = pLLCDrv;
  int Res = -ENOSYS;

  d_fnstart(D_INTERN, NULL, "(void) [pDrv %p]\n", pDrv);

  Res = LLC_Release(pDrv);
  (void)Res;

  d_fnend(D_INTERN, NULL, "(void) [pDrv %p] = void\n", pDrv);
  d_exit();
  return;
}
module_exit(LLC_ModuleExit);

MODULE_AUTHOR("Cohda Wireless");
MODULE_DESCRIPTION("Cohda LLCremote kernel module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.0");

/**
 * @}
 * @}
 */

