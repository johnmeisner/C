/*
   (c) NXP B.V. 2017. All rights reserved.

   Disclaimer
   1. The NXP Software/Source Code is provided to Licensee "AS IS" without any
      warranties of any kind. NXP makes no warranties to Licensee and shall not
      indemnify Licensee or hold it harmless for any reason related to the NXP
      Software/Source Code or otherwise be liable to the NXP customer. The NXP
      customer acknowledges and agrees that the NXP Software/Source Code is
      provided AS-IS and accepts all risks of utilizing the NXP Software under
      the conditions set forth according to this disclaimer.

   2. NXP EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING,
      BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
      FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT OF INTELLECTUAL PROPERTY
      RIGHTS. NXP SHALL HAVE NO LIABILITY TO THE NXP CUSTOMER, OR ITS
      SUBSIDIARIES, AFFILIATES, OR ANY OTHER THIRD PARTY FOR ANY DAMAGES,
      INCLUDING WITHOUT LIMITATION, DAMAGES RESULTING OR ALLEGDED TO HAVE
      RESULTED FROM ANY DEFECT, ERROR OR OMMISSION IN THE NXP SOFTWARE/SOURCE
      CODE, THIRD PARTY APPLICATION SOFTWARE AND/OR DOCUMENTATION, OR AS A
      RESULT OF ANY INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHT OF ANY
      THIRD PARTY. IN NO EVENT SHALL NXP BE LIABLE FOR ANY INCIDENTAL,
      INDIRECT, SPECIAL, EXEMPLARY, PUNITIVE, OR CONSEQUENTIAL DAMAGES
      (INCLUDING LOST PROFITS) SUFFERED BY NXP CUSTOMER OR ITS SUBSIDIARIES,
      AFFILIATES, OR ANY OTHER THIRD PARTY ARISING OUT OF OR RELATED TO THE NXP
      SOFTWARE/SOURCE CODE EVEN IF NXP HAS BEEN ADVISED OF THE POSSIBILITY OF
      SUCH DAMAGES.

   3. NXP reserves the right to make changes to the NXP Software/Sourcecode any
      time, also without informing customer.

   4. Licensee agrees to indemnify and hold harmless NXP and its affiliated
      companies from and against any claims, suits, losses, damages,
      liabilities, costs and expenses (including reasonable attorney's fees)
      resulting from Licensee's and/or Licensee customer's/licensee's use of the
      NXP Software/Source Code.

 */

/**
 *
 * @file posixDal.c
 *
 * @author kanal
 *
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <time.h>
#include "config.h"
#include "v2xCryptoPal.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

#if defined( ENABLE_DAL_CFG_MK5 )

#define DAL_CFG_SYSFS_GPIO_DIR          "/sys/class/gpio"
/* DAL_CFG_SPI_DEVICE: should be set to the SPI device that is used to communicate with the Secure Element.
  For example in Linux its typically "/dev/spidev1.0"
 */
#define DAL_CFG_SPI_DEVICE              "/dev/spidev32766.0"

/*DAL_CFG_SPI_SPEED_IN_HZ: the SPI clock frequency. This should not exceed the capabilities of the application
  processor and Secure Element as specified in the datasheets. Ensure that the setup and hold times of both sides
  are satisfied. Also take additional delays caused by the board design into account.*/
#define DAL_CFG_SPI_SPEED_IN_HZ         5000000

/* GPIO number is calculated using the formula: (gpio_bank - 1) * 32 + gpio_bit
 * For example DAL_CFG_SPI_DAV_GPIO = (6-1)*32 + 3 =>163
 */
/*DAL_CFG_SPI_DAV_GPIO: should be set to the GPIO pin number which is connected to the DAV (GPO1) pin of the Secure Element.
  Enabling the use of the DAV and SE_RDY lines is controlled via a build diversity flag, refer V2X Crypto Library user manual.*/
#define DAL_CFG_SPI_DAV_GPIO            (163U) //CSI0_DAT17, GPIO6_IO03

/*DAL_CFG_SE_RESET_GPIO: should be set to the GPIO pin number which is connected to the RSTN pin of the Secure Element.*/
#define DAL_CFG_SE_RESET_GPIO           (164U) //CSI0_DAT18, GPIO6_IO04

/*DAL_CFG_SE_SWITCH_GPIO: should be set to the GPIO pin number that controls enabling/disabling VDD of Secure Element.*/
#define DAL_CFG_SE_SWITCH_GPIO          (155U) //CSI0_DAT9, GPIO5_IO27

/*DAL_CFG_SERDY_GPIO: should be set to the GPIO pin number which is connected to the SE_RDY (GPIO0) pin of the Secure Element.*/
#define DAL_CFG_SERDY_GPIO              (162U) //CSI0_DAT16, GPIO6_IO02



#elif defined( ENABLE_DAL_CFG_OPT1 )

#define DAL_CFG_SYSFS_GPIO_DIR          "/sys/class/gpio"
/*DAL_CFG_SPI_DEVICE: should be set to the SPI device that is used to communicate with the Secure Element.
  For example in Linux its typically "/dev/spidev1.0"
 */
#define DAL_CFG_SPI_DEVICE              "/dev/spidev1.0"

/*DAL_CFG_SPI_SPEED_IN_HZ: the SPI clock frequency. This should not exceed the capabilities of the application
  processor and Secure Element as specified in the datasheets. Ensure that the setup and hold times of both sides
  are satisfied. Also take additional delays caused by the board design into account.*/
#define DAL_CFG_SPI_SPEED_IN_HZ         5000000

/*DAL_CFG_SPI_DAV_GPIO: should be set to the GPIO pin number which is connected to the DAV (GPO1) pin of the Secure Element.
  Enabling the use of the DAV and SE_RDY lines is controlled via a build diversity flag, refer V2X Crypto library user manual for details.*/
#define DAL_CFG_SPI_DAV_GPIO            (164U)   //CSI0_DAT18, GPIO6_IO04

/*DAL_CFG_SE_RESET_GPIO: should be set to the GPIO pin number which is connected to the RSTN pin of the Secure Element.*/
#define DAL_CFG_SE_RESET_GPIO           (165U)  //CSI0_DAT19, GPIO6_IO05

/*DAL_CFG_SE_SWITCH_GPIO: should be set to the GPIO pin number that controls enabling/disabling VDD of Secure Element. For OPT1 platform the feature of
  controlling power supply to Secure Element is not supported hence its assigned the value "0".*/
#define DAL_CFG_SE_SWITCH_GPIO          (0U)       // Not supported

/*DAL_CFG_SERDY_GPIO: should be set to the GPIO number which is connected to the SE_RDY (GPIO0) pin of the Secure Element.*/
#define DAL_CFG_SERDY_GPIO              (163U) //CSI0_DAT17, GPIO6_IO03


#elif defined( ENABLE_DAL_CFG_EVK2 )

#define DAL_CFG_SYSFS_GPIO_DIR          "/sys/class/gpio"
/*DAL_CFG_SPI_DEVICE: should be set to the SPI device that is used to communicate with the Secure Element.
  For example in Linux its typically "/dev/spidev1.0"
*/
#define DAL_CFG_SPI_DEVICE              "/dev/spidev0.0"

/*DAL_CFG_SPI_SPEED_IN_HZ: the SPI clock frequency. This should not exceed the capabilities of the application
  processor and Secure Element as specified in the datasheets. Ensure that the setup and hold times of both sides
  are satisfied. Also take additional delays caused by the board design into account.*/
#define DAL_CFG_SPI_SPEED_IN_HZ         5000000

/*DAL_CFG_SPI_DAV_GPIO: should be set to the GPIO pin number which is connected to the DAV (GPO1) pin of the Secure Element.
  Enabling the use of the DAV and SE_RDY lines is controlled via a build diversity flag, refer V2X Crypto library user manual for details.*/
#define DAL_CFG_SPI_DAV_GPIO            (86U)     // i.MX6 pad LCD_DATA17, GPIO3_IO22

/*DAL_CFG_SE_RESET_GPIO: should be set to the GPIO pin number which is connected to the RSTN pin of the Secure Element.*/
#define DAL_CFG_SE_RESET_GPIO           (67U)     // i.MX6 pad LCD_VSYNC, GPIO3_IO03

/*DAL_CFG_SE_SWITCH_GPIO: should be set to the GPIO pin number that controls enabling/disabling VDD of Secure Element.*/
#define DAL_CFG_SE_SWITCH_GPIO          (102U)    // i.MX6 pad NAND_DATA04, GPIO4_IO06

/*DAL_CFG_SERDY_GPIO: should be set to the GPIO pin which number is connected to the SE_RDY (GPIO0) pin of the Secure Element.*/
#define DAL_CFG_SERDY_GPIO              (85U)     // i.MX6 pad LCD_DATA16, GPIO3_IO21


#elif defined( ENABLE_DAL_CFG_IMX8MEK )

#define DAL_CFG_SYSFS_GPIO_DIR          "/sys/class/gpio"
/*DAL_CFG_SPI_DEVICE: should be set to the SPI device that is used to communicate with the Secure Element.
  For example in Linux its typically "/dev/spidev1.0"
*/
#define DAL_CFG_SPI_DEVICE              "/dev/spidev0.0"

/*DAL_CFG_SPI_SPEED_IN_HZ: the SPI clock frequency. This should not exceed the capabilities of the application
  processor and Secure Element as specified in the datasheets. Ensure that the setup and hold times of both sides
  are satisfied. Also take additional delays caused by the board design into account.*/
#define DAL_CFG_SPI_SPEED_IN_HZ         5000000

/*DAL_CFG_SPI_DAV_GPIO: should be set to the GPIO pin number which is connected to the DAV (GPO1) pin of the Secure Element.
  Enabling the use of the DAV and SE_RDY lines is controlled via a build diversity flag, refer V2X Crypto library user manual for details.*/
#define DAL_CFG_SPI_DAV_GPIO            (494U)     // i.MX8 pad SPI3_SDO, GPIO0_IO14

/*DAL_CFG_SE_RESET_GPIO: should be set to the GPIO pin number which is connected to the RSTN pin of the Secure Element.*/
#define DAL_CFG_SE_RESET_GPIO           (455U)     // i.MX8 pad SPI0_CS1, GPIO1_IO07

/*DAL_CFG_SE_SWITCH_GPIO: should be set to the GPIO pin number that controls enabling/disabling VDD of Secure Element. For IMX8MEK platform the feature of
  controlling power supply to Secure Element is not supported hence its assigned the value "0".*/
#define DAL_CFG_SE_SWITCH_GPIO          (0U)       // Not supported

/*DAL_CFG_SERDY_GPIO: should be set to the GPIO pin number which is connected to the SE_RDY (GPIO0) pin of the Secure Element.*/
#define DAL_CFG_SERDY_GPIO              (496U)     // i.MX8 pad SPI3_CS0, GPIO0_IO16

#elif defined( ENABLE_DAL_CFG_H71E )

#define DAL_CFG_SYSFS_GPIO_DIR          "/sys/class/gpio"
#define DAL_CFG_SPI_DEVICE              "/dev/spidev0.0"
#define DAL_CFG_SPI_SPEED_IN_HZ         4800000

#define DAL_CFG_SPI_DAV_GPIO            (406U)		// i.MX8 pad QSPI0B_DQS_LSIO_GPIO3_IO22	
#define DAL_CFG_SPI_DAV_GPIO_EDGE       "rising"
#define DAL_CFG_SPI_DAV_GPIO_UBUF_MAX   (64)
#define DAL_CFG_GPIO_UBUF_MAX           (64)
#define DAL_CFG_GPIO_RDBUFF_MAX         (4)
#define DAL_CFG_GPIO_DIR_INPUT          (0)
#define DAL_CFG_GPIO_DIR_OUTPUT         (1)
/* IMX8_HSM_RST_3V3 */
// #define DAL_CFG_SE_RESET_GPIO           (158U)
#define DAL_CFG_SE_RESET_GPIO           (488U)		// i.MX8 pad ESAI0_TX4_RX1_LSIO_GPIO0_IO08
#define DAL_CFG_SE_SWITCH_GPIO          (0U)       // Not supported

/*DAL_CFG_SERDY_GPIO: should be set to the GPIO pin number which is connected to the SE_RDY (GPIO0) pin of the Secure Element.*/
#define DAL_CFG_SERDY_GPIO              (401U)     // i.MX8 pad QSPI0B_SCLK_LSIO_GPIO3_IO17
char dal_spi_target[50] = "";
int dal_dav_mode = 0; //0 interrupt, 1 polling, 2: force polling
int dal_dav_pin = 0;
int dal_reset_pin = 0;


#else

#error "Unknown Board DAL Cfg"

#endif

//Common Definitions for all the platforms


/*Response readiness(data availability) of Secure Element is indicated by DAV pin change from low to high.
 *The line stays high until the response is read. Due to this reason readiness/data availability is detected with
 *the rising edge of the DAV pin. Do not change this setting.
 */
#define DAL_CFG_SPI_DAV_GPIO_EDGE       "rising"

/*This is used to identify if the soft-reset command was accepted by Secure Element and the soft-reset is triggered
 *Do not change this setting
 */
#define DAL_CFG_SERDY_GPIO_EDGE         "rising"

/*The size of string used to refer to the GPIO pin. This must contain full path
 to the gpio files including direction, value and edge
 */
#define DAL_CFG_GPIO_UBUF_MAX           (64)
#define DAL_CFG_GPIO_RDBUFF_MAX         (4) //GPIO value '0' or '1' followed by null string. 3 bytes should also be sufficient

/*The below definitions are used as enumeration for configuring GPIO pins as input or output.
 *This means, the values assigned for DAL_CFG_GPIO_DIR_INPUT and DAL_CFG_GPIO_DIR_OUTPUT can be different than the ones mentioned here
 *but they must be unique. The value assigned should not exceed the range of uint32_t type to avoid MISRA code violations.
 */
//#define DAL_CFG_GPIO_DIR_INPUT          (0u) //Configures GPIO pin as input pin
//Configures GPIO pin as output pin. Typically used in controlling GPIO pin connected to RSTN pin or power supply to Secure Element
//#define DAL_CFG_GPIO_DIR_OUTPUT         (1u)


/*The v2xSe_reset() function can be used to either reset Secure Element via RSTN pin or control power supply.
 * The following definitions are used while controlling power supply(#CONFIG_POWER_CYCLE from config.h is set to 1)
 */
#define DAL_CFG_POWER_CYCLE_DELAY       (201) //Time in milli seconds for which VDD should be disabled
#define DAL_CFG_POWER_CYCLE_TON_DELAY   (5)   //Time in milli seconds to ensure VDD has reached min of 1.67V

// Runtime loading of posixdal dependent kernel module
#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
// Runtime Unloading of posixdal dependent kernel module
#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)

//Chip select line specific definitions
#define DAL_CS_ASSERTED               (1)
#define DAL_CS_DEASSERTED             (0)
#define DAL_CS_UNKNOWN                (-1)

//RSTN line specific definition
#define DAL_RESET_DELAY_US      1    /* 1us as per data sheet of SXF1800(Secure Element)*/

//The minimum time for which SPI Chipselect should be deasserted
#define DAL_CS_DELAY_US         1000 /* 1ms as per SPI T=1 protocol specification of SXF1800(Secure Element)*/

//SPI mode specific definitions

#define DAL_CFG_SPI_MODE           0   /*SPI_CPHA | SPI_CPOL | SPI_LSB_FIRST*/
#define DAL_CFG_SPI_BITS_PER_WORD  8U   /* bits per word*/
#define DAL_CFG_SPI_DELAY_IN_US    0   /*delay in us*/



/******************************************************************************
 * INTERNAL VARIABLES
 *****************************************************************************/
static int32_t g_SpiDevHand=0;  /* spi file */

static uint64_t g_CsDeassertTime = 0;
static int8_t g_CsState = DAL_CS_UNKNOWN;

#define DAL_CFG_EVTMON_EPOLL_EVTS_MAX (20)

#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)

typedef struct
{
    v2xDal_ExtlEvtId_t    evtId;
    char *                evtName;
    int32_t               evtDevId;
    int32_t               evtValSysFd;
    int32_t               evtMonEpollFd;
    struct epoll_event    stEpollEvtCtrl;
    int32_t               evtPinDir;
    char *                evtEdge;
    bool                  modLocGpioExpDone;
} v2xEvtSign_Discp_t;

static v2xEvtSign_Discp_t gEvtSignCnt[0x02] = { \
    {
#if CONFIG_SERDY_EVT
        .evtId          = dal_extl_evtid_SERDY,
        .evtName        = "serdy",
        .evtDevId       = DAL_CFG_SERDY_GPIO,
        .evtValSysFd    = -1,
        .evtPinDir      = DAL_CFG_GPIO_DIR_INPUT,
        .evtEdge        = DAL_CFG_SERDY_GPIO_EDGE,
        .modLocGpioExpDone = false
#else
        .evtId          = -1,
        .evtName        = "serdy",
        .evtDevId       = -1,
        .evtValSysFd    = -1
#endif
    },
    {
#if CONFIG_SPI_DAV
        .evtId          = dal_extl_evtid_DAV,
        .evtName        = "dav",
        .evtDevId       = DAL_CFG_SPI_DAV_GPIO,
        .evtValSysFd    = -1,
        .evtPinDir      = DAL_CFG_GPIO_DIR_INPUT,
        .evtEdge        = DAL_CFG_SPI_DAV_GPIO_EDGE,
        .modLocGpioExpDone = false
#else
        .evtId          = -1,
        .evtName        = "dav",
        .evtDevId       = -1,
        .evtValSysFd    = -1
#endif
    }
};

#endif

static v2xDal_error_t v2xDal_ensureDelay(uint64_t startTime, uint32_t minDelayTime);
static bool v2xDal_gpioFileTest(int32_t gpio);
static v2xDal_error_t v2xDal_gpioExport(int32_t gpio, bool * pModLocGpioExpDone);
static v2xDal_error_t v2xDal_gpioUnexport(int32_t gpio, bool modLocGpioExpDone);
static v2xDal_error_t v2xDal_gpioSetDir(int32_t gpio, uint32_t outFlag);
static v2xDal_error_t v2xDal_gpioSetValue(int32_t gpio, uint32_t value);

#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)
static v2xDal_error_t v2xDal_gpioSetEdge( int32_t gpio, const char *edge);
static v2xDal_error_t v2xDal_gpioFdOpen( int32_t gpio, int32_t  * fdescp);
static v2xDal_error_t v2xDal_gpioFdClose( int32_t  fdescp );
static v2xDal_error_t v2xDal_gpioGetValue( int32_t gpio, int32_t fdescp, int32_t * pRdVal );
static v2xDal_error_t v2xdal_openExtlEvt( v2xDal_ExtlEvtId_t evtId );
static v2xDal_error_t v2xdal_closeExtlEvt( v2xDal_ExtlEvtId_t evtId );
#endif

/******************************************************************************
 * POSIX DAL INTERNAL UTIL FUNCTIONS
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_ensureDelay
 *
 * DESCRIPTION  :   Function which ensures conditional sleep of certain time "minDelayTime".
 * If the difference of startTime and current time is already more than "minDelayTime"
 * then no additional sleep will be performed. Since its conditional sleep, the sleep operation
 * is performed only when required, rather than sleep at all times once called.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xDal_ensureDelay(uint64_t startTime, uint32_t minDelayTime)
{
    v2xDal_error_t ret = dal_error_NONE;
    uint64_t actualDelay;
    uint64_t currentTime;

    if (startTime == 0ULL)
    {
        // If the start time is unknown, we delay for the full time
        v2xOsal_usleep(minDelayTime);
    }
    else
    {
        // Get current time
        currentTime = v2xOsal_timeus();
        if (currentTime == 0ULL)
        {
            ret = dal_error_OS;
        }
        else
        {
            // Calculated the time already passed
            actualDelay = currentTime - startTime;
            // Wait additionally if needed
            if (actualDelay < (uint64_t)minDelayTime)
            {
                v2xOsal_usleep(minDelayTime - (uint32_t)actualDelay);
            }
        }
    }

    return ret;
}

/******************************************************************************
 * GPIO UTILL INTERNAL FUNCTION COLLECTIONS
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioFileTest
 *
 * DESCRIPTION  :   This function is to check if GPIO is already exported.
 *                  This function is used in v2xDal_gpioExport() and v2xDal_gpioUnexport()functions.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static bool v2xDal_gpioFileTest(int32_t gpio)
{
    bool funRetVal = false;
    int32_t  plfRetVal;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    plfRetVal = snprintf((char *)buf, sizeof(buf), DAL_CFG_SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    if(plfRetVal < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s \n",
                   gpio,
                   errno,
                   strerror(errno));
    }
    else
    {
        plfRetVal = access((const char *)buf, F_OK);

        if (plfRetVal == 0)
        {
            // FILE Exists ...
            // GPIO already exported ...
            funRetVal = true;
        }
        else
        {
            // FILE not Exists ...
            // GPIO not exported ...
            funRetVal = false;
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioExport
 *
 * DESCRIPTION  :   export the given gpio and make it available for sysfs access
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static __attribute__ ((unused)) v2xDal_error_t v2xDal_gpioExport( int32_t gpio, bool * pModLocGpioExpDone )
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  fd;
    size_t  len;
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    if( pModLocGpioExpDone != NULL)
    {
        *pModLocGpioExpDone = false;
    }

    // Test if GPIO file is already exported ...
    if( v2xDal_gpioFileTest( gpio ) == true )
    {
        // GPIO file already exist, nothing to be done ...
        v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"GPIO %d already seems to be exported !!!\n", gpio);
    }
    else
    {
        fd = open(DAL_CFG_SYSFS_GPIO_DIR "/export", O_WRONLY);
        if (fd == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) open(/export) failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));

            funRetVal = dal_error_OS;
        }

        if( funRetVal == dal_error_NONE )
        {
            rc = snprintf((char *)buf, sizeof(buf), "%d", gpio);

            if(rc < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s \n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }
            else
            {
                len = (size_t)rc;
                rc = write(fd, buf, len);
                if(rc == -1)
                {
                    if(&errno != NULL)
                    {
                        if( (errno == EBUSY) )
                        {
                            // Extra check for Fail proof ...
                            v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"GPIO %d already seems to be exported\n", gpio);
                        }
                        else
                        {
                            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                                       gpio,
                                       errno,
                                       strerror(errno));
                            funRetVal = dal_error_OS;
                        }
                    }
                    else
                    {
                        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed as &errno is pointing to NULL");
                        funRetVal = dal_error_OS;
                    }
                }
            }

            rc = close(fd);
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) close() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }

            if( ( funRetVal == dal_error_NONE ) && (pModLocGpioExpDone != NULL) )
            {
                *pModLocGpioExpDone = true;
            }
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioUnexport
 *
 * DESCRIPTION  :   Unexport the given gpio
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static __attribute__ ((unused)) v2xDal_error_t v2xDal_gpioUnexport( int32_t gpio, bool modLocGpioExpDone )
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  fd;
    size_t  len;
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    if( v2xDal_gpioFileTest( gpio ) == false )
    {
        // Test for GPIO file, if already unexported. skip the unexport process.
        v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"GPIO %d already seems to be UnExported !!!\n", gpio);
    }
    else if ( modLocGpioExpDone == true )
    {
        // GPIO is exported by v2xcrypto lib
        // Unexport if GPIO file exists ...

        fd = open(DAL_CFG_SYSFS_GPIO_DIR "/unexport", O_WRONLY);
        if (fd == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) open(/unexport) failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }

        if( funRetVal == dal_error_NONE )
        {
            rc = snprintf((char *)buf, sizeof(buf), "%d", gpio);
            if(rc < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s \n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }
            else
            {
                len = (size_t)rc;
                rc = write(fd, buf, len);
                if(rc == -1)
                {
                    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                               gpio,
                               errno,
                               strerror(errno));
                    funRetVal = dal_error_OS;
                }
            }

            rc = close(fd);
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) close() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }
        }
    }
    else
    {
        // GPIO is not exported by v2xcrypto lib
        // nothing to be done.
        v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"GPIO %d not exported by Dal!!!\n", gpio);
        //  (void)funRetVal; //MISRA Fix rule 15.7, cannot contain an empty else statement
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioSetDir
 *
 * DESCRIPTION  :   gpio set pin direction
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static __attribute__ ((unused))  v2xDal_error_t v2xDal_gpioSetDir(int32_t gpio, uint32_t outFlag)
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  fd;
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    rc = snprintf((char *)buf, sizeof(buf), DAL_CFG_SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    if(rc < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s \n",
                   gpio,
                   errno,
                   strerror(errno));
        funRetVal = dal_error_OS;
    }

    if( funRetVal == dal_error_NONE )
    {
        fd = open((const char *)buf, O_WRONLY);
        if (fd == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) open(%s) failed errno(%d)%s !!!\n",
                       gpio,
                       buf,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    if( funRetVal == dal_error_NONE )
    {

        if (outFlag != DAL_CFG_GPIO_DIR_INPUT)
        {
            const char * pwrArg = "out";

            rc = write(fd, (void const *)pwrArg, 4); // strln is 4
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }
        }
        else
        {
            const char * pwrArg = "in";

            rc = write(fd, (void const *)pwrArg, 3); // strln is 3
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }
        }

        rc = close(fd);
        if(rc == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) close() failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioSetValue
 *
 * DESCRIPTION  :   gpio set pin value
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static __attribute__ ((unused)) v2xDal_error_t v2xDal_gpioSetValue(int32_t gpio, uint32_t value)
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t fd;
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    rc = snprintf((char *)buf, sizeof(buf), DAL_CFG_SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    if(rc < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s \n",
                   gpio,
                   errno,
                   strerror(errno));
        funRetVal = dal_error_OS;
    }

    if( funRetVal == dal_error_NONE )
    {
        fd = open((const char *)buf, O_WRONLY);
        if (fd == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) open(%s) failed errno(%d)%s !!!\n",
                       gpio,
                       buf,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    if( funRetVal == dal_error_NONE )
    {
        if (value > 0U)
        {
            const char * pwrArg = "1";

            rc = write(fd, (void const *)pwrArg, 2); // strln is 2
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }

        }
        else
        {

            const char * pwrArg = "0";

            rc = write(fd, (void const *)pwrArg, 2); // strln is 2
            if(rc == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                           gpio,
                           errno,
                           strerror(errno));
                funRetVal = dal_error_OS;
            }

        }

        rc = close(fd);
        if(rc == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) close() failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }

    }

    return( funRetVal );
}

/******************************************************************************
 * SPI DATA VALID SIGNAL UTILL INTERNAL FUNCTION COLLECTIONS
 *****************************************************************************/
#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioSetEdge
 *
 * DESCRIPTION  :   gpio cfg event edge detect rising or falling
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static __attribute__ ((unused)) v2xDal_error_t v2xDal_gpioSetEdge(int32_t gpio, const char *edge)
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  fd;
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    rc = snprintf((char *)buf, sizeof(buf), DAL_CFG_SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    if(rc == -1)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s !!!\n",
                   gpio,
                   errno,
                   strerror(errno));
        funRetVal = dal_error_OS;
    }


    if( funRetVal == dal_error_NONE )
    {
        fd = open((const char *)buf, O_WRONLY);

        if (fd == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err open(%s) failed errno(%d)%s !!!\n",
                       buf,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    if( funRetVal == dal_error_NONE )
    {
        rc = write(fd, edge, strlen(edge) + 1U);
        if(rc == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) write() failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }

        rc = close(fd);
        if(rc == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) close() failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioFdOpen
 *
 * DESCRIPTION  :   gpio open sysfs file handle
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xDal_gpioFdOpen(int32_t gpio, int32_t  * fdescp)
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t oflag = (uint32_t)O_RDONLY | ((uint32_t)O_NONBLOCK);//Explicit typecasting required to fix MISRA 10.1 rule violation
    int32_t  rc;
    int8_t buf[DAL_CFG_GPIO_UBUF_MAX] = {0x00};

    rc = snprintf((char *)buf, sizeof(buf), DAL_CFG_SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    if(rc < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) snprintf() failed errno(%d)%s !!!\n",
                   gpio,
                   errno,
                   strerror(errno));
        funRetVal = dal_error_OS;
    }

    if( funRetVal == dal_error_NONE )
    {
        *fdescp = open((const char *)buf, oflag);
        if (*fdescp == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err open(%s) failed errno(%d)%s !!!\n",
                       buf,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioFdClose
 *
 * DESCRIPTION  :   gpio close sysfs file handle
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xDal_gpioFdClose( int32_t  fdescp )
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  plfRetVal;

    plfRetVal = close(fdescp);

    if (plfRetVal == -1)
    {
        funRetVal = dal_error_OS;
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_gpioGetValue
 *
 * DESCRIPTION  :   gpio get pin status
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xDal_gpioGetValue( int32_t gpio, int32_t fdescp, int32_t * pRdVal )
{
    v2xDal_error_t funRetVal = dal_error_NONE;
    int32_t  rc;
    char buf[DAL_CFG_GPIO_RDBUFF_MAX] = {0};

    /* First check if data available line is already high (data is already available)
       if not we wait for an rising edge
       buf[0] == 1 indicates data is available */
    /* To read the GPIO value, we seek back to the beginning, since the read will move it to the end of file */
    rc = lseek(fdescp, 0, SEEK_SET);
    if (rc == -1)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) lseek() failed errno(%d)%s !!!\n",
                   gpio,
                   errno,
                   strerror(errno));
        funRetVal = dal_error_OS;
    }

    if (funRetVal == dal_error_NONE)
    {
        /* Expected amount of data is 1 byte for value and possibly a line end */
        rc = read(fdescp, buf, DAL_CFG_GPIO_RDBUFF_MAX);

        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"gpio(%d) read rc(%d) buff-%s", gpio, rc, buf);

        if (rc == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) read() failed errno(%d)%s !!!\n",
                       gpio,
                       errno,
                       strerror(errno));
            funRetVal = dal_error_OS;
        }
        else if (rc == 0)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) read() returns insufficient data\n", gpio );
            funRetVal = dal_error_OS;
        }
        else if (rc >= DAL_CFG_GPIO_RDBUFF_MAX)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err gpio(%d) read() returns too much data\n", gpio );
            funRetVal = dal_error_OS;
        }
        else
        {
            funRetVal = dal_error_NONE;
        }
    }

    if( funRetVal == dal_error_NONE && pRdVal != NULL )
    {
        if (buf[0x00] != '0')
        {
            *pRdVal = 1;
        }
        else
        {
            *pRdVal = 0;
        }
    }

    return( funRetVal );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xdal_openExtlEvt
 *
 * DESCRIPTION  :   High level function, prepare for gpio event capture
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xdal_openExtlEvt( v2xDal_ExtlEvtId_t evtId )
{
    v2xDal_error_t status = dal_error_NONE;

    v2xEvtSign_Discp_t * pstEvSigDiscp = (v2xEvtSign_Discp_t *)&gEvtSignCnt[ evtId ];

    if( pstEvSigDiscp->evtId != evtId )
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err Inv EvtId(%d) configuration.\n",
                   evtId );
        status = dal_error_OS;
    }
    else
    {
        status = dal_error_NONE;

        // export and configure the pin for our usage
        status = v2xDal_gpioExport( dal_dav_pin, &pstEvSigDiscp->modLocGpioExpDone );

        // Cfg the pin direction to input
        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioSetDir( dal_dav_pin, pstEvSigDiscp->evtPinDir );
        }

        // Cfg the Edge to Rising edge
        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioSetEdge( dal_dav_pin, pstEvSigDiscp->evtEdge );
        }

        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioFdOpen( dal_dav_pin, &pstEvSigDiscp->evtValSysFd );
        }

        if( status == dal_error_NONE )
        {

            pstEvSigDiscp->evtMonEpollFd = epoll_create1(EPOLL_CLOEXEC);

            if( pstEvSigDiscp->evtMonEpollFd == -1 )
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err epoll_create() failed errno(%d)%s \n",
                           errno,
                           strerror(errno));

                status = dal_error_OS;
            }
            // assign it to default value -1
            pstEvSigDiscp->stEpollEvtCtrl.data.fd = -1;
        }

        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Exit RC(%d)\n", status);
    }

    return( status );
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xdal_closeExtlEvt
 *
 * DESCRIPTION  :   High level function, close the handles related to gpio event capture
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
static v2xDal_error_t v2xdal_closeExtlEvt( v2xDal_ExtlEvtId_t evtId )
{
    v2xDal_error_t status = dal_error_NONE;
    v2xDal_error_t statRc2 = dal_error_NONE;
    int32_t scRval ;

    v2xEvtSign_Discp_t * pstEvSigDiscp = (v2xEvtSign_Discp_t *)&gEvtSignCnt[ evtId ];

    if( pstEvSigDiscp->evtId != evtId )
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err Inv EvtId(%d) configuration.\n",
                   evtId );
        status = dal_error_OS;
    }
    else
    {
        status = dal_error_NONE;

        if( pstEvSigDiscp->evtMonEpollFd != -1 )
        {
            // delete epoll of fd
            if( pstEvSigDiscp->stEpollEvtCtrl.data.fd != -1 )
            {
                scRval = epoll_ctl(pstEvSigDiscp->evtMonEpollFd, EPOLL_CTL_DEL, pstEvSigDiscp->evtValSysFd, &pstEvSigDiscp->stEpollEvtCtrl);
                if( scRval == -1 )
                {
                    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err epoll_ctl() failed errno(%d)%s \n",
                               errno,
                               strerror(errno));
                    status = dal_error_OS;
                }
            }

            pstEvSigDiscp->stEpollEvtCtrl.data.fd = -1;

            scRval = close( pstEvSigDiscp->evtMonEpollFd );

            if( scRval == -1 )
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err close() failed errno(%d)%s \n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }

            pstEvSigDiscp->evtMonEpollFd = -1;
        }

        // Close the file discp ...
        if( pstEvSigDiscp->evtValSysFd != -1 )
        {
            (void)v2xDal_gpioFdClose( pstEvSigDiscp->evtValSysFd );

            pstEvSigDiscp->evtValSysFd = -1;
        }

        if( pstEvSigDiscp->modLocGpioExpDone == true )
        {
            statRc2 = v2xDal_gpioUnexport( dal_dav_pin, pstEvSigDiscp->modLocGpioExpDone );

            if( statRc2 != dal_error_NONE )
            {
                status = statRc2;
            }
        }
    }

    return( status );
}

#endif

v2xDal_error_t v2xDal_set_target(char *newtarget)
{
	strcpy(dal_spi_target, newtarget);
	return dal_error_NONE;
}

v2xDal_error_t v2xDal_set_dav_pin(int pin)
{
	dal_dav_pin = pin;
	return dal_error_NONE;
}

v2xDal_error_t v2xDal_set_dav_mode(int mode)
{
	dal_dav_mode = mode;
	return dal_error_NONE;
}

v2xDal_error_t v2xDal_set_rst_pin(int pin)
{
	dal_reset_pin = pin;
	return dal_error_NONE;
}

/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xSpiDal_open
 *
 * DESCRIPTION  :   Open spi device file handle.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_open(void)
{
    v2xDal_error_t retStatus=dal_error_NONE;

    int32_t ret;
    int32_t mode = DAL_CFG_SPI_MODE; /*SPI_CPHA | SPI_CPOL | SPI_LSB_FIRST*/
    int32_t bits = 8;
    int32_t speed = DAL_CFG_SPI_SPEED_IN_HZ;  // in Hz
    const char * device;
	  /* give default value - start*/

	if (dal_spi_target[0] == '\0')
		device = DAL_CFG_SPI_DEVICE;
	else
		device = dal_spi_target;

	if (!dal_dav_pin)
		dal_dav_pin = DAL_CFG_SPI_DAV_GPIO;

	if (!dal_reset_pin)
		dal_reset_pin = DAL_CFG_SE_RESET_GPIO;
/* If this data is needed then add to v2xSecExample output. */
#if defined(DENSO_EXTRA_DEBUG)
	printf("spi target : %s\n", device);
	printf("spi speed : %d\n", speed);
	printf("reset pin : %d\n", dal_reset_pin);
	printf("dav pin : %d\n", dal_dav_pin);
	printf("dav mode : %d\n", dal_dav_mode);
#else
	v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "spi target : %s\n", device);
	v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "spi speed : %d\n", speed);
	v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "reset pin : %d\n", dal_reset_pin);
	v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "dav pin : %d\n", dal_dav_pin);
	v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "dav mode : %d\n", dal_dav_mode);
#endif

    // Clear state variables
    g_CsDeassertTime = 0;
    g_CsState = DAL_CS_UNKNOWN;
    // Open device
    g_SpiDevHand = open(device, O_RDWR);
    if (g_SpiDevHand < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err open(%s) failed errno(%d)%s !!!\n",
                   device,
                   errno,
                   strerror(errno));
        retStatus=dal_error_OS;
    }
    else
    {
        v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "SPI opened:    %s\n", device);
    }

    if(retStatus==dal_error_NONE)
    {
        // spi mode
        ret = ioctl(g_SpiDevHand, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to set spi mode");
            retStatus=dal_error_OS;
        }
    }
    if(retStatus==dal_error_NONE)
    {
        ret = ioctl(g_SpiDevHand, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "Failed to get spi mode");
            retStatus=dal_error_OS;
        }
    }

    if(retStatus==dal_error_NONE)
    {
        // bits per word
        ret = ioctl(g_SpiDevHand, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to set bits per word");
            retStatus=dal_error_OS;
        }
    }
    if(retStatus==dal_error_NONE)
    {
        ret = ioctl(g_SpiDevHand, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to get bits per word");
            retStatus=dal_error_OS;
        }
    }
    if(retStatus==dal_error_NONE)
    {
        // max speed in hz
        ret = ioctl(g_SpiDevHand, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to set max speed hz");
            retStatus=dal_error_OS;
        }
    }
    if(retStatus==dal_error_NONE)
    {
        ret = ioctl(g_SpiDevHand, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to get max speed hz");
            retStatus=dal_error_OS;
        }
    }
    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "SPI mode:      %d\n", mode);
    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "Bits per word: %d\n", bits);
    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "Max speed:     %d KHz\n", speed/1000);

    //Open & Cfg the GPIO Handle for SPI Data Valid Signal & SE Ready
    //Integrate the block if CONFIG_SPI_DAV
#if (CONFIG_SPI_DAV)
    if(retStatus==dal_error_NONE)
    {
        if( gEvtSignCnt[ dal_extl_evtid_DAV ].evtId == dal_extl_evtid_DAV )
        {
            retStatus = v2xdal_openExtlEvt( dal_extl_evtid_DAV );
        }
    }
#endif

#if (CONFIG_SERDY_EVT)
    if(retStatus==dal_error_NONE)
    {
        if( gEvtSignCnt[ dal_extl_evtid_SERDY ].evtId == dal_extl_evtid_SERDY )
        {
            retStatus = v2xdal_openExtlEvt( dal_extl_evtid_SERDY );
        }
    }
#endif
{
/* If this data is needed then add to v2xSecExample output. */
#if defined(DENSO_EXTRA_DEBUG)
    printf("[HSM1800] data available pin %u\n", dal_dav_pin);
    printf("[HSM1800] spi interface %s\n", device);
#else
    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "[HSM1800] data available pin %u\n", dal_dav_pin);
    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "[HSM1800] spi interface %s\n", device);
#endif
}
    return retStatus;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_close
 *
 * DESCRIPTION  :   Close spi device file handle.
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_close(void)
{
    v2xDal_error_t retStatus=dal_error_NONE;
    v2xDal_error_t intStatus=dal_error_NONE;

    int32_t ret = close(g_SpiDevHand);
    if (ret < 0)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"ERROR: Closing SPI failed\n");
        retStatus = dal_error_OS;
        (void)intStatus;
    }
    else
    {
        v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL, "SPI closed\n");
    }

    //Close the GPIO Handle for SPI Data Valid Signal
    //Integrate the block if CONFIG_SPI_DAV
#if (CONFIG_SPI_DAV)

    if( gEvtSignCnt[ dal_extl_evtid_DAV ].evtId == dal_extl_evtid_DAV )
    {
        intStatus = v2xdal_closeExtlEvt( dal_extl_evtid_DAV );

        if( intStatus != dal_error_NONE )
        {
            retStatus = intStatus;
        }
    }

#endif

#if (CONFIG_SERDY_EVT)

    if( gEvtSignCnt[ dal_extl_evtid_SERDY ].evtId == dal_extl_evtid_SERDY )
    {
        intStatus = v2xdal_closeExtlEvt( dal_extl_evtid_SERDY );

        if( intStatus != dal_error_NONE )
        {
            retStatus = intStatus;
        }
    }

#endif

    return retStatus;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_spiWrite
 *
 * DESCRIPTION  :   write the data in to secure element
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_spiWrite(void *pTxData,void *pRxData, uint32_t txDataLen)
{
    v2xDal_error_t status= dal_error_NONE;

    int32_t ret;
    int8_t *pRxBuf=pRxData;
    int8_t *txbuf=pTxData;

    // We can write and read at the same time
    int32_t speed = DAL_CFG_SPI_SPEED_IN_HZ; // in Hz
    int8_t bits = DAL_CFG_SPI_BITS_PER_WORD;       // bits per word
    struct spi_ioc_transfer tr =
    {
        .tx_buf = (unsigned long)txbuf,
        .rx_buf = (unsigned long)pRxBuf,
        .len = txDataLen,
        .delay_usecs = DAL_CFG_SPI_DELAY_IN_US,
        .speed_hz = speed,
        .bits_per_word = bits
    };

    if( pRxData != NULL )
    {
        (void)memset( pRxData, 0x00, txDataLen );
    }

#if(CONFIG_SPI_DAV)

    if(( gEvtSignCnt[ dal_extl_evtid_DAV ].evtId == dal_extl_evtid_DAV ) \
            && ( gEvtSignCnt[ dal_extl_evtid_DAV ].evtDevId == -1 )      \
            && ( gEvtSignCnt[ dal_extl_evtid_DAV ].evtValSysFd == -1 ))
    {
        v2xDbg_msg(DBG_LEVEL_3,DBG_COMMUNICATION_LEVEL, "Invalid Dav Evt poll status\n");
    }

#endif

    v2xDbg_printByteString(DBG_LEVEL_3,DBG_COMMUNICATION_LEVEL,"SPIWR Buff", (const uint8_t *)txbuf, txDataLen);

    // Ensure chip select is deasserted for more than DAL_CS_DELAY_US
    status = v2xDal_ensureDelay(g_CsDeassertTime, DAL_CS_DELAY_US);
    if (status != dal_error_NONE)
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to wait");
        status = dal_error_OS;
    }

    // Start transfer
    if (status == dal_error_NONE)
    {
        ret = ioctl(g_SpiDevHand, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to send spi message");
            g_CsState = DAL_CS_UNKNOWN;
            status = dal_error_OS;
        }
        else
        {
            // Register the time (pessimistic) that the chip select line is deasserted
            g_CsState = DAL_CS_DEASSERTED;
            g_CsDeassertTime = v2xOsal_timeus();
            if (g_CsDeassertTime == 0u)
            {
                status = dal_error_OS;
            }
        }
    }

    return status;
}

/*-----------------------------------------------------------------------------
 * FUNCTION     :   v2xDal_spiRead
 *
 * DESCRIPTION  :   read the data from secure element
 *
 * NOTES:       :   NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_spiRead(void *pRxData, int32_t rxDataSize, int32_t *pRxDataLen,v2xDal_spiCsCtrl_t csCtrl)
{

    v2xDal_error_t status= dal_error_NONE;
    int32_t ret;
    int8_t *rxBuf=pRxData;

    // We can write and read at the same time
    int32_t speed = DAL_CFG_SPI_SPEED_IN_HZ; // in Hz
    int8_t bits = DAL_CFG_SPI_BITS_PER_WORD;       // bits per word
    struct spi_ioc_transfer tr =
    {
        .tx_buf = (unsigned long)NULL,
        .rx_buf = (unsigned long)rxBuf,
        .len = rxDataSize,
        .delay_usecs = DAL_CFG_SPI_DELAY_IN_US,
        .speed_hz = speed,
        .bits_per_word = bits
    };

    if(csCtrl==dal_spics_active_CONTINUE)
    {
        tr.cs_change=1;
    }
    else
    {
        tr.cs_change=0;
    }

    // Ensure chip select is deasserted for more than DAL_CS_DELAY_US
    if (g_CsState != DAL_CS_ASSERTED)
    {
        status = v2xDal_ensureDelay(g_CsDeassertTime, DAL_CS_DELAY_US);
        if (status != dal_error_NONE)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to wait");
            status = dal_error_OS;
        }
    }

    // Start transfer
    if (status == dal_error_NONE)
    {
        ret = ioctl(g_SpiDevHand, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL, "ERROR: Failed to send spi message");
            *pRxDataLen=0;
            g_CsState = DAL_CS_UNKNOWN;
            status = dal_error_OS;
        }
        else
        {

            // Register the time (pessimistic) that the chip select line is deasserted
            if(csCtrl==dal_spics_active_CONTINUE)
            {
                g_CsState = DAL_CS_ASSERTED;
            }
            else
            {
                g_CsState = DAL_CS_DEASSERTED;
                g_CsDeassertTime = v2xOsal_timeus();
                if (g_CsDeassertTime == 0u)
                {
                    status = dal_error_OS;
                }
            }

            *pRxDataLen=rxDataSize;
            v2xDbg_printByteString(DBG_LEVEL_3,DBG_COMMUNICATION_LEVEL,"SPIRD Buff", (const uint8_t *)rxBuf, rxDataSize);
        }
    }

    return status;
}

/*-----------------------------------------------------------------------------
 * FUNCTION   : v2xDal_seReset
 *
 * DESCRIPTION  :   Util function to reset Secure Element.
 *
 * NOTES:
 *
 * platform             CONFIG_POWER_CYCLE == 0    CONFIG_POWER_CYCLE == 1
 * ------------------------------------------------------------------------
 * ENABLE_DAL_CFG_MK5   reset                      power cycle
 * ENABLE_DAL_CFG_OPT1  reset                      error
 * others               reset                      error
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_seReset(void)
{
    v2xDal_error_t status= dal_error_NONE;
    bool modLocGpioExpDone = false;

#if (CONFIG_POWER_CYCLE) && !defined(ENABLE_DAL_CFG_MK5)
#error "Error Wrong Config :: No power cycle method implemented for this platform"
#endif

    if( CONFIG_POWER_CYCLE == 0 )
    {
#if 1        
		int p_buff = dal_reset_pin;
		printf("[reset gpio] %d\n", p_buff);           
        status = v2xDal_gpioExport( dal_reset_pin, &modLocGpioExpDone );

        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioSetDir( dal_reset_pin, DAL_CFG_GPIO_DIR_OUTPUT );
        }

        if( status == dal_error_NONE )
        {
            //status = v2xDal_gpioSetValue( dal_reset_pin, 0x00U );
            status = v2xDal_gpioSetValue( dal_reset_pin, 0x01U );
        }

        if( status == dal_error_NONE )
        {
            v2xOsal_usleep(DAL_RESET_DELAY_US);
        }

        if( status == dal_error_NONE )
        {
            //status = v2xDal_gpioSetValue( dal_reset_pin, 0x01U );
            status = v2xDal_gpioSetValue( dal_reset_pin, 0x00U );
        }

        if( modLocGpioExpDone == true )
        {
            status = v2xDal_gpioUnexport( dal_reset_pin, modLocGpioExpDone );
        }
#else
        status = system("/usr/src/wnc/scripts/h7_1_gpio_exp.sh init"); /* dir output*/
    /* script init return false even if it action correctly. skip ret val */
    /*
        if (status)
            return dal_error_OS;
    */
        status = system("/usr/src/wnc/scripts/h7_1_gpio_exp.sh IMX8_HSM_RST_3V3 W 1"); /* PCB design is invert. 1: reset pin low */
        if (status)
            return dal_error_OS;
        status = system("/usr/src/wnc/scripts/h7_1_gpio_exp.sh IMX8_HSM_RST_3V3 W 0"); /* PCB design is invert. 0: reset pin high */
        if (status)
            return dal_error_OS;
        status = dal_error_NONE;
#endif        
    }
    else
    {

#if (CONFIG_POWER_CYCLE) && defined(ENABLE_DAL_CFG_MK5)

        int32_t  ret;
        int32_t  fd = -1;
        size_t image_size;
        struct stat st;
        void *image = NULL;

        ret = delete_module("spi_imx", O_NONBLOCK);

        if (ret < 0)
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err delete_module(spi_imx) failed errno(%d)%s\n",
                       errno,
                       strerror(errno));
            status=dal_error_OS;
        }

        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioExport( DAL_CFG_SE_SWITCH_GPIO, &modLocGpioExpDone );
        }

        if( status == dal_error_NONE )
        {
            status = v2xDal_gpioSetDir( DAL_CFG_SE_SWITCH_GPIO, DAL_CFG_GPIO_DIR_OUTPUT );
        }

        if( status == dal_error_NONE )
        {
            // Turn power switch off (switch is active low)
            status = v2xDal_gpioSetValue( DAL_CFG_SE_SWITCH_GPIO, 0x01U );
        }

        if( status == dal_error_NONE )
        {
            // Wait until VDD is for sure below 0.1V
            v2xOsal_usleep(DAL_CFG_POWER_CYCLE_DELAY * 1000);
        }

        if( status == dal_error_NONE )
        {
            // Turn power switch on (switch is active low)
            status = v2xDal_gpioSetValue( DAL_CFG_SE_SWITCH_GPIO, 0x00U );
        }

        if( status == dal_error_NONE )
        {
            // Wait until bootloader is completed to prevent reset in the bootloader
            // in case this function is called successively
            v2xOsal_usleep(DAL_CFG_POWER_CYCLE_TON_DELAY * 1000);
        }

        if( modLocGpioExpDone == true )
        {
            status = v2xDal_gpioUnexport( DAL_CFG_SE_SWITCH_GPIO, modLocGpioExpDone );
        }

        if( status == dal_error_NONE )
        {

            fd = open("spi-imx.ko", O_RDONLY);

            if (fd == -1)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err open(spi-imx.ko) failed errno(%d)%s\n",
                           errno,
                           strerror(errno));

                status = dal_error_OS;
            }
        }

        if( status == dal_error_NONE )
        {
            ret = fstat(fd, &st);

            if (ret < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err fstat failed errno(%d)%s\n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }
        }

        if( status == dal_error_NONE )
        {
            image_size = st.st_size;
            image = malloc(image_size);

            if( image == NULL )
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err malloc failed errno(%d)%s\n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }
        }

        if( status == dal_error_NONE )
        {
            ret = read(fd, image, image_size);

            if (ret < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err read failed errno(%d)%s\n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }
        }

        if( status == dal_error_NONE )
        {
            ret = init_module(image, image_size, "");

            if (ret < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err init_module failed errno(%d)%s\n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }
        }

        if( image != NULL )
        {
            free(image);
        }

        if( fd != -1 )
        {
            ret = close( fd );
            if(ret < 0)
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err close() failed errno(%d)%s !!!\n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
            }
        }
        /*#else
                (void)modLocGpioExpDone;
                (void)status;*/
#endif

    }

    return( status );
}

v2xDal_error_t v2xDal_getEvtPinState( v2xDal_ExtlEvtId_t evtId, int32_t * pRetSat )
{
    v2xDal_error_t status = dal_error_NONE;

    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"Enter \n");

#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)

    v2xEvtSign_Discp_t * pstEvSigDiscp = (v2xEvtSign_Discp_t *)&gEvtSignCnt[ evtId ];

    if(( pstEvSigDiscp->evtId != evtId ) \
            && ( dal_dav_pin == 0 ) \
            && ( pstEvSigDiscp->evtValSysFd == -1 ))
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err Inv EvtId(%d) / Inv FileDiscp / Dal is closed.\n",
                   evtId );
        status = dal_error_OS;
    }
    else
    {

        status = v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, pRetSat );

        if( status != dal_error_NONE )
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Warn v2xDal_gpioGetValue status=%d \n",
                       status );
        }
    }

#else

    (void)evtId;
    (void)pRetSat;

    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err %s:%d CONFIG_SERDY_EVT not enabled\n",
               __FUNCTION__,
               __LINE__);

    status = dal_error_OS;

#endif

    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"Exit \n");

    return status;
}

/*-----------------------------------------------------------------------------
 * FUNCTION   : v2xDal_startPollForEvt
 *
 * DESCRIPTION  :   Creates Evt monitor instance "epoll"- to monitor event changes on gpio.
 *
 * NOTES:       : NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_startPollForEvt( v2xDal_ExtlEvtId_t evtId )
{
    v2xDal_error_t status = dal_error_NONE;

    v2xDal_error_t statusCleanup = dal_error_NONE;

    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"Enter \n");

#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)
    int32_t scRval;
    v2xEvtSign_Discp_t * pstEvSigDiscp = (v2xEvtSign_Discp_t *)&gEvtSignCnt[ evtId ];

    if(( pstEvSigDiscp->evtId != evtId ) \
            && ( dal_dav_pin == 0 ) \
            && ( pstEvSigDiscp->evtValSysFd == -1 ))
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err Inv EvtId(%d) / Inv FileDiscp / Dal is closed.\n",
                   evtId );
        status = dal_error_OS;
    }
    else
    {

        (void)memset( (void *)&pstEvSigDiscp->stEpollEvtCtrl, 0x00, sizeof(struct epoll_event) );

        pstEvSigDiscp->stEpollEvtCtrl.data.fd = -1;

        // gpio sysfs work around - To avoid initial false detection,
        // dummy read is required after edge cfg,
        status = v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, NULL );

        // Open the GPIO PIN file descriptor
        if( status == dal_error_NONE )
        {

            // Tst-cfg1 :: EPOLLIN | EPOLLERR | EPOLLPRI | EPOLLET ( working )..
            // Tst-cfg2 :: EPOLLPRI | EPOLLERR  (working )
            pstEvSigDiscp->stEpollEvtCtrl.events = (uint32_t)EPOLLPRI | ((uint32_t)EPOLLERR);//Typecasting to fix MISRA rule 10.1
            pstEvSigDiscp->stEpollEvtCtrl.data.fd = pstEvSigDiscp->evtValSysFd;

            scRval = epoll_ctl( pstEvSigDiscp->evtMonEpollFd, EPOLL_CTL_ADD,
                                pstEvSigDiscp->evtValSysFd,
                                &pstEvSigDiscp->stEpollEvtCtrl );

            if( scRval == -1 )
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err epoll_ctl() failed errno(%d)%s \n",
                           errno,
                           strerror(errno));
                status = dal_error_OS;
                pstEvSigDiscp->stEpollEvtCtrl.data.fd = -1;
            }
        }

        // Dummy read to clean sysfs gpio state.
        statusCleanup = v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, NULL );
        if(statusCleanup!=dal_error_NONE)
        {
            status = dal_error_OS;
        }
    }

#else

    (void)evtId;
    (void)statusCleanup;

    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err %s:%d CONFIG_SERDY_EVT | CONFIG_SPI_DAV not enabled\n",
               __FUNCTION__,
               __LINE__);

    status = dal_error_OS;

#endif

    v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"Exit status(%d)\n", status);

    return( status );
}

/*-----------------------------------------------------------------------------
 * FUNCTION   : v2xDal_getEvtStatus
 *
 * DESCRIPTION  :   Check for the status from Evt monitor instance "epoll",
 *                  created via v2xDal_startPollForEvt
 *
 * NOTES:       : NA
 *
 *-----------------------------------------------------------------------------
 */
v2xDal_error_t v2xDal_getEvtStatus( v2xDal_ExtlEvtId_t evtId, int32_t timeOut, int32_t * pRetSat )
{

    v2xDal_error_t status = dal_error_NONE;

#if (CONFIG_SPI_DAV) || (CONFIG_SERDY_EVT)

    int32_t scRval;
    int32_t itrCnt;
    int32_t numEvts;
    int32_t pinStatus;
    struct epoll_event stEvtStaList[DAL_CFG_EVTMON_EPOLL_EVTS_MAX];

    v2xEvtSign_Discp_t * pstEvSigDiscp = (v2xEvtSign_Discp_t *)&gEvtSignCnt[ evtId ];

    if(( pstEvSigDiscp->evtId != evtId ) \
            && ( dal_dav_pin == 0 ) \
            && ( pstEvSigDiscp->evtValSysFd == -1 ) \
            && ( pstEvSigDiscp->stEpollEvtCtrl.data.fd == -1))
    {
        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err Inv EvtId(%d) / Inv FileDiscp / Dal is closed.\n",
                   evtId );
        status = dal_error_OS;
    }
    else
    {
        (void)v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, &pinStatus );

        /* Observation of Data Available Pin status
        if(pinStatus == 1)
            printf("Data Available Pin High\n");
		else
			printf("Data Available Pin Low\n");
        */

        (void)memset((void*)stEvtStaList, 0x00, sizeof(struct epoll_event) * (uint16_t)DAL_CFG_EVTMON_EPOLL_EVTS_MAX);

        scRval = epoll_wait( pstEvSigDiscp->evtMonEpollFd, stEvtStaList, DAL_CFG_EVTMON_EPOLL_EVTS_MAX, timeOut);

        if ( scRval == -1 )
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err epoll() failed errno(%d)%s \n",
                       errno,
                       strerror(errno));
            status = dal_error_OS;
        }
        else if (scRval == 0)
        {
            v2xDbg_msg(DBG_LEVEL_2,DBG_COMMUNICATION_LEVEL,"Err epoll() timeout!\n");
            status = dal_error_TIMEOUT;
        }
        else
        {
            numEvts = 0x00;

            for( itrCnt = 0x00; itrCnt < scRval; itrCnt++ )
            {
                if(  pstEvSigDiscp->evtValSysFd == stEvtStaList[itrCnt].data.fd )
                {
                    // Perform dummy read on File discp for clearing event ...
                    status = v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, &pinStatus );
                    if (status == dal_error_NONE)
                    {
                        if (pRetSat != NULL)
                        {
                            // Keep overwriting such that we return the latest status
                            *pRetSat = pinStatus;
                        }
                    }
                    else
                    {
                        // In case of error we continue to cleanup as much as possible, but this function will return an error
                        v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Error reading GPIO %s for clearing event, itrCnt(%d)\n",
                                   pstEvSigDiscp->evtName, itrCnt );
                    }

                    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Info Pin %s redy status(0x%x)\n",
                               pstEvSigDiscp->evtName,
                               stEvtStaList[itrCnt].events);
                    if( ( stEvtStaList[itrCnt].events & pstEvSigDiscp->stEpollEvtCtrl.events) != 0x00u )
                    {
                        numEvts++;
                    }
                }
            }

            if( numEvts == 0x00 )
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err No Ready evt from Pin %s Wrong trigger\n",
                           pstEvSigDiscp->evtName);
                status = dal_error_OS;
            }
            else
            {
                v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Info %d evt from Pin %s \n",
                           numEvts,
                           pstEvSigDiscp->evtName);
            }
        }
        (void)v2xDal_gpioGetValue( dal_dav_pin, pstEvSigDiscp->evtValSysFd, &pinStatus );
        
        /* Observation of Data Available Pin status
        if(pinStatus == 1)
            printf("Data Available Pin High\n");
		else
			printf("Data Available Pin Low\n");
        */

        // Remove the GPIO FD from epoll queue
        scRval = epoll_ctl( pstEvSigDiscp->evtMonEpollFd, EPOLL_CTL_DEL,
                            pstEvSigDiscp->evtValSysFd,
                            &pstEvSigDiscp->stEpollEvtCtrl );

        if( scRval == -1 )
        {
            v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err %s:%d epoll_ctl() failed errno(%d)%s \n",
                       __FUNCTION__,
                       __LINE__,
                       errno,
                       strerror(errno));
            status = dal_error_OS;
        }

        pstEvSigDiscp->stEpollEvtCtrl.data.fd = -1;
    }

#else

    (void)evtId;
    (void)timeOut;

    v2xDbg_msg(DBG_LEVEL_1,DBG_COMMUNICATION_LEVEL,"Err %s:%d CONFIG_SERDY_EVT | CONFIG_SPI_DAV not enabled\n",
               __FUNCTION__,
               __LINE__);

    status = dal_error_OS;

#endif

    return status;
}
