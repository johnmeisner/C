/*
*  Filename: led_status.c
*  Purpose: LED status
*
*  Copyright (C) 2020 DENSO International America, Inc.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  05-05-20    JJG        This should have already existed
*  10-09-20    KF         Added option for formatting output into webgui format
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <process.h>
#include <gpio/gpio.h>
#include <hw/spi-master.h>
#include "led_common.h"

/* Defines */
#define NCV7718_SPI_PORT        "/dev/spi2"
#define NCV7718_SPI_SS          1
#define GPIO_OUT_EN             "GPIO_OUT_EN"

/* JJG - new hardware; register values; need to be shifted 1 (values below shifted already) */
/* 20200504: Migrate this and led_srv values into a common header */

/* a bit may be missing */
#define RED_LED_STATE           0x39c
#define RED_LED_ALT_STATE       0x38c
#define AMBER_LED_STATE         0x396
#define AMBER_LED_ALT_STATE     0x386
#define GREEN_LED_STATE         0x39a
#define GREEN_LED_ALT_STATE     0x38a
#define OFF_LED_STATE           0x01e


/* Globals */
static wint32 fd;                         /* spi handle */
static gpio_connection_t       *gpio_connection;
static gpio_port_t             *gpio_out_en;
static wint8 led_lock = 0;

/*
 * xchange_val()
 *
 * Read from spi register; spi is an exchange between master and slave
 */
int xchange_val(int fd, size_t slave, wint16 wval, wint16 *rval) {
    unsigned char rbuf[2] = {0};
    wint16 rint;
    unsigned char  wbuf[2]={(wval& 0xff), (wval >> 8) & 0xff};
    if(!spi_xchange(fd, slave, &wbuf[0], &rbuf[0], 2)) {
        fprintf(stderr,"%s: spi_xchange failed.\n", __FUNCTION__);
        return -1;
    } else {
        rint = rbuf[0] + (rbuf[1] << 8);
        *rval = rint;
    }
    return 0;
}

/*
 * led_status()
 *
 * Report status of LEDs
 */
static void led_status(wint32 fd, wint32 webgui_fmt)
{
    wint16 rval, saveval;

    /*
     * Simple lock to make sure no one else is touching the LED before it's done
     */

    while (led_lock) {
        usleep(1000); /* sleep for one 1ms */
    }

    led_lock = 1;  /* lock */

    /* read NCV7718 register */
    if(xchange_val(fd, NCV7718_SPI_SS, 0x4000, &rval) != 0) {
        fprintf(stderr, "failed to read NCV7718 register status.\n");
        led_lock = 0;  /* unlock */
        return;
    }
    saveval = rval;
    /* MUST set value read back to device to keep LED active */
    usleep(200);    /* copied from ncv7718-cmd implementation; why 200 microsec delay is unknown (not listed in datasheet) */
    xchange_val(fd, NCV7718_SPI_SS, rval, &rval);

    if (webgui_fmt) {
        if (((saveval & GREEN_LED_STATE) == GREEN_LED_STATE) || ((saveval & GREEN_LED_ALT_STATE) == GREEN_LED_ALT_STATE)) {
            printf("Active,\n");
        } else {
            printf("Inactive,\n");
        }
        if (((saveval & AMBER_LED_STATE) == AMBER_LED_STATE) || ((saveval & AMBER_LED_ALT_STATE) == AMBER_LED_ALT_STATE)) {
            printf("Active,\n");
        } else {
            printf("Inactive,\n");
        }
        if (((saveval & RED_LED_STATE) == RED_LED_STATE) || ((saveval & RED_LED_ALT_STATE) == RED_LED_ALT_STATE)) {
            printf("Active,\n");
        } else {
            printf("Inactive,\n");
        }
    } else {
        if (((saveval & RED_LED_STATE) == RED_LED_STATE) || ((saveval & RED_LED_ALT_STATE) == RED_LED_ALT_STATE)) {
            printf("RED LED: Active\n");
        } else {
            printf("RED LED: Inactive\n");
        }
        if (((saveval & GREEN_LED_STATE) == GREEN_LED_STATE) || ((saveval & GREEN_LED_ALT_STATE) == GREEN_LED_ALT_STATE)) {
            printf("GREEN LED: Active\n");
        } else {
            printf("GREEN LED: Inactive\n");
        }
        if (((saveval & AMBER_LED_STATE) == AMBER_LED_STATE) || ((saveval & AMBER_LED_ALT_STATE) == AMBER_LED_ALT_STATE)) {
            printf("AMBER LED: Active\n");
        } else {
            printf("AMBER LED: Inactive\n");
        }
    }

    led_lock = 0;  /* unlock */
}


/*
* main() - start of LED service
*
*/
wint32 main(wint32 argc, wint8 *argv[])
{
    /*
     * Open SPI interface for GPIO comunication
     */
    if((fd = spi_open(NCV7718_SPI_PORT)) < 0) {
        perror("failed to open device");
        return -1;
    }

    if (argc > 1 && (!strcmp(argv[1],"-h") || !strcmp(argv[1],"--help"))) {
        printf("Usage: led_status [--webgui-format]\n");
        exit(0);
    }


/* this is initial setup required; but this is handled by led_srv */
#if 0
    spi_cfg_t cfg;
    cfg.mode = 16 | SPI_MODE_BODER_MSB | SPI_MODE_CKPOL_HIGH;
    cfg.clock_rate = 5000000;

    if(spi_setcfg(fd, NCV7718_SPI_SS, &cfg) != 0) {
        spi_close(fd);
        fprintf(stderr, "spi_setcfg failed.\n");
        goto fail_and_close;
    }

    /* Enable NCV7718 driver for GPIO - ready for SPI communication */
    gpio_connection = gpio_connect(NULL,0);
    if(gpio_connection == NULL) {
        fprintf(stderr, "failed to create gpio_connection.\n");
        goto fail_and_close_gpiocnxn;
    }
    gpio_out_en = gpio_open_named_port(gpio_connection, GPIO_OUT_EN, 0);
    if (gpio_out_en == NULL) {
        fprintf(stderr, "failed to open GPIO port GPIO_OUT_EN.\n");
        goto fail_and_close_gpio;
    }
    gpio_out(gpio_out_en, 1, 1, 0);
    usleep(100); //Wait 100us for SPI to activate
#endif

    led_status(fd, (argc > 1 && !strcmp(argv[1],"--webgui-format")));

/* this is cleanup if performing initial setup */
#if 0 
fail_and_close_gpio:
    gpio_close(gpio_out_en);
fail_and_close_gpiocnxn:
    gpio_disconnect(gpio_connection);
fail_and_close:
    spi_close(fd);
fail_no_close:
#endif

    return 0;
}
