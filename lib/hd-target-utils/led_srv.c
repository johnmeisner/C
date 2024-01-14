/*
*  Filename: led_srv.c
*  Purpose: LED service
*
*  Copyright (C) 2019 DENSO International America, Inc.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  01-12-17    LNGUYEN    Initial release.
*  03-13-19    JJG        Updates for new LED hardware and bootup (formatting too)
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
#include <mqueue.h>
// #include <process.h>
#include <gpio/gpio.h>
#include <hw/spi-master.h>
#include "dn_types.h"
#include "led_common.h"

/* Defines */
#define NCV7718_SPI_PORT        "/dev/spi2"
#define NCV7718_SPI_SS          1
#define GPIO_OUT_EN             "GPIO_OUT_EN"

/* JJG - new hardware; register values; need to be shifted 1 (values below shifted already) */

#define RED_LED_STATE           0x39c
#define AMBER_LED_STATE         0x396
#define GREEN_LED_STATE         0x39a
#define OFF_LED_STATE           0x01e

#define LED_BLNK_MIN_RATE       10              /* Minimum 10ms */
#define LED_BLNK_MAX_RATE       100000          /* MAX 10 sec */

#define ONE_SECOND              1000000         /* for usleep */
#define TENTH_SECOND            100000          /* for usleep */



/* Globals */
static wint32 fd;                         /* spi handle */
static mqd_t qd_server;
static struct mq_attr attr;
static wint8 mq_in_buffer [MSG_BUFFER_SIZE];
static gpio_connection_t       *gpio_connection;
static gpio_port_t             *gpio_out_en;
static wint8 led_lock = 0;

static WBOOL mainloop = WTRUE;


static wint32 debug_output = 0;    /* Debug OFF by default */


/* Globals */
static pthread_t led_mq_thread = NULL;
static pthread_t led_red_thread = NULL;
static pthread_t led_green_thread = NULL;


/* Forward declare */
static void *led_red_handler(void *ptr);
static void *led_green_handler(void *ptr);



/*
 * ledSigHandler()
 *
 * Signal handler when registered signals caught
 *
 */
static void ledSigHandler(int __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

/*
 * print_hex()
 *
 * Print a hex string, line break every 16 bytes
 *
 */
static void __attribute__((unused)) print_hex(wuint8 *hex_buff, wuint32 len)
{
    wuint32 i;

    /*Error check */
    if ( (hex_buff == NULL) || (len < 1)) return;

    for (i = 0; i < len; ++i) {
        if (i % 16 == 0) printf("%04X <", i);   /* new line */
        printf(" %02X", hex_buff[i]);
        if ((i + 1) % 16 == 0) printf(" >\n");  /* end of line print */
    }

    if (i % 16 != 0) printf(" >\n");
}

/*
 * read_val()
 *
 * Read from spi register
 */
static wint32 read_val(wint32 fd, size_t slave, wint32 *val)
{
    wint8 rbuf[4] = {0};
    wint32 rint;
    if(!spi_read(fd, slave, &rbuf[0], 4)) {
        fprintf(stderr, "%s: spi_read failed.\n", __FUNCTION__);
        return -1;
    } else {
        rint = rbuf[0] + (rbuf[1] << 8) + (rbuf[2] << 16) + (rbuf[3] << 24);
        *val = rint;
    }
    return 0;
}

/*
 * write_val()
 *
 * Write a value to register via spi interface
 *
 */
static wint32 write_val(wint32 fd, size_t slave, wint32 val) {
    wint8 wbuf[2]={(val & 0xff), (val >> 8) & 0xff};

    if(!spi_write(fd, slave, &wbuf[0], 2)) {
        fprintf(stderr,"%s: spi_write failed.\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

/*
 * led_control()
 *
 * control RED and GREEN LEDs
 */
static void led_control(wint32 fd, wint32 led_type, wint32 led_on)
{
    wint32 wval = 0;
    wint32 rval;

    /*
     * Simple lock to make sure no one else is touching the LED before it's done
     */

    while (led_lock) {
        usleep(1000); /* sleep for one 1ms */
    }

    led_lock = 1;  /* lock */

    /* read NCV7718 register */
    if(read_val(fd, NCV7718_SPI_SS, &rval) == 0) {
        if (debug_output) printf("NCV7718 register status 0x%08x\n", rval);
    } else {
        fprintf(stderr, "failed to read NCV7718 register status.\n");
        led_lock = 0;  /* unlock */
        return;
    }

    /* should be a switch, but really, there are only 3 leds */
    if (LED_RED == led_type) {
        if (led_on) {
            wval = (rval & 0xFFFFC000) | RED_LED_STATE;
        } else {
            wval = (rval & 0xFFFFC000) | OFF_LED_STATE;
        }
    } else if (LED_GREEN == led_type){
        if (led_on) {
            wval = (rval & 0xFFFFC000) | GREEN_LED_STATE;
        } else {
            wval = (rval & 0xFFFFC000) | OFF_LED_STATE;
        }
    } else {
        if (led_on) {
            wval = (rval & 0xFFFFC000) | AMBER_LED_STATE;
        } else {
            wval = (rval & 0xFFFFC000) | OFF_LED_STATE;
        }
    }

    /* write new val */
    if (debug_output) printf("write val: %x\n", wval);
    write_val(fd, NCV7718_SPI_SS, wval);

    led_lock = 0;  /* unlock */
}


/*
 * process_led_msg()
 *
 * Process incoming message from MQ
 */
static void process_led_msg(wint8 *led_msg, wuint32 len)
{
    mq_led_hdr_t *led_hdr = (mq_led_hdr_t *)led_msg;

    wint8 led_type;
    wint8 led_en;
    wint32 blink_rate;

    if (NULL == led_msg) return;        /* NULL error check */

    if (debug_output){
        debug_print_0 ("Got LED command: \n");
        debug_print   ("   led_hdr->proc_id: %d\n", led_hdr->proc_id);
        debug_print   ("   led_hdr->thrd_id: %d\n", led_hdr->thrd_id);
        debug_print   ("   led_hdr->led_attr.led_type: %d\n", led_hdr->led_attr.led_type);
        debug_print   ("   led_hdr->led_attr.led_en: %d\n", led_hdr->led_attr.led_en);
        debug_print   ("   led_hdr->led_attr.blink_rate: %d\n", led_hdr->led_attr.blink_rate);
    }

    led_type = led_hdr->led_attr.led_type;
    led_en = led_hdr->led_attr.led_en;
    blink_rate = led_hdr->led_attr.blink_rate;

    /* Terminate led threads */
    if (led_type == LED_RED) {
        if (led_red_thread) {
            pthread_cancel(led_red_thread);
            led_red_thread = NULL;
        }
    } else {
        if(led_green_thread){
            pthread_cancel(led_green_thread);
            led_green_thread = NULL;
        }
    }

    /* Set LED */
    led_control(fd, led_type, led_en);

    /* Start an LED thread if blink rate is specified */
    if (blink_rate > 0) {
        if (blink_rate < LED_BLNK_MIN_RATE) blink_rate = LED_BLNK_MIN_RATE;
        if (blink_rate > LED_BLNK_MAX_RATE) blink_rate = LED_BLNK_MAX_RATE;

        /* restart proper led thread */
        if (led_type == LED_RED) {
            pthread_create( &led_red_thread, NULL, (void *) &led_red_handler, &blink_rate);
            pthread_detach( led_red_thread);
        } else {
            pthread_create( &led_green_thread, NULL, (void *) &led_green_handler, &blink_rate);
            pthread_detach( led_green_thread);
        }
    }
}

/*
 * led_red_handler()
 *
 * handler of red blinking led thread
 *
 */
static void *led_red_handler(void *ptr)
{
    wint32 *blink_rate = (wint32 *)ptr;
    wint32 usleep_dur = *blink_rate * 1000;

    if (debug_output) debug_print_0("----- Starting led_red_thread ----- \n");
    if (debug_output) debug_print(" Process ID: %d and Thread ID: %d, blink_rate %d\n", getpid(), gettid(), usleep_dur);

    while(1) {
        led_control(fd, LED_RED, LED_ON);           /* RED ON */
        usleep(usleep_dur);
        led_control(fd, LED_RED, LED_OFF);          /* RED OFF */
        usleep(usleep_dur);
    }

    if (debug_output) debug_print_0("----- Exiting led_red_thread ----- \n");
    pthread_exit(0);
}

/*
 * led_green_handler()
 *
 * Handler of green blinking LED thread
 *
 */
static void *led_green_handler(void *ptr)
{
    wint32 *blink_rate = (wint32 *)ptr;
    wint32 usleep_dur = *blink_rate * 1000;

    if (debug_output) debug_print_0("----- Starting green_thread ----- \n");
    if (debug_output) debug_print(" Process ID: %d and Thread ID: %d, blink_rate %d\n", getpid(), gettid(), usleep_dur);

    while(1) {
        led_control(fd, LED_GREEN, LED_ON);          /* GREEN ON */
        usleep(usleep_dur);
        led_control(fd, LED_GREEN, LED_OFF);         /* GREEN OFF */
        usleep(usleep_dur);
    }

    if (debug_output) debug_print_0("----- Exiting green_thread ----- \n");
    pthread_exit(0);
}

/*
 * led_mq_handler()
 *
 * Handler of MQ thread
 *
 */
static void *led_mq_handler(void *ptr)
{
    mq_led_hdr_t *mq_hdr;
    wint32 msg_size = 0;

    if (debug_output) debug_print_0("----- Starting led_mq_thread ----- \n");
    if (debug_output) debug_print(" Process ID: %d and Thread ID: %d\n", getpid(), gettid());

    while (mainloop) {
        memset(mq_in_buffer, 0, sizeof(mq_in_buffer));
        if (debug_output) debug_print_0("Waiting for mq message ...\n");

        /* Pop a message from queue with highest priority */
        if ((msg_size = (mq_receive (qd_server, mq_in_buffer, MSG_BUFFER_SIZE, NULL)) ) == -1)  {
            perror ("LED MQ Server: mq_receive - exiting thread");
            break;
        }

        if (debug_output) debug_print("Got an mq message with len %d...\n", msg_size);

        mq_hdr = (mq_led_hdr_t *)&mq_in_buffer[0];

        if (debug_output) debug_print("Process: %d ; Thread: %d\n",
                mq_hdr->proc_id,
                mq_hdr->thrd_id);

        if (debug_output) debug_print_0("Received \n");
        if (debug_output) debug_print("led cmd: %s\n", (char *)(&mq_in_buffer[0]+sizeof(mq_led_hdr_t)) );

        process_led_msg(mq_in_buffer, msg_size);

    }

    pthread_exit(0);

}

/*
* main() - start of LED service
*
*/
wint32 main(wint32 argc, wint8 *argv[])
{
#define BOOTUPONLY 1
    wsint8 c_arg;
    wuint8 dummy = 0xFF;  /* used for bootup and exit type */
    struct sigaction sa;

    /* Process debug mode, serial port */
    while ((c_arg = getopt(argc, argv, "bd:")) != -1) {
        switch (c_arg) {
            case 'b':
            dummy = BOOTUPONLY;
            break;

            case 'd':           /* debug mode: 1-on; 0-off (default) */
            debug_output = atoi(optarg);
            debug_print("Debug mode %d\n", debug_output);
            break;

            default:
            break;
        }
    }

    /*
     * Open SPI interface for GPIO comunication
     */
    if((fd = spi_open(NCV7718_SPI_PORT)) < 0) {
        perror("failed to open device");
        goto fail_no_close;
    }

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

    if (dummy == BOOTUPONLY) {
        /* bootup sequence control for LEDs; then exit 
           allow 1 sec of LEDs to be enabled then turn off and move to next in 100 ms */
        led_control(fd, LED_RED, LED_ON);
        usleep(ONE_SECOND);
        led_control(fd, LED_RED, LED_OFF);
        usleep(TENTH_SECOND);
        led_control(fd, LED_AMBER, LED_ON);
        usleep(ONE_SECOND);
        led_control(fd, LED_AMBER, LED_OFF);
        usleep(TENTH_SECOND);
        led_control(fd, LED_GREEN, LED_ON);
        usleep(ONE_SECOND);
        led_control(fd, LED_GREEN, LED_OFF);

        /* no thread creation; cleanup */
        gpio_close(gpio_out_en);
        gpio_disconnect(gpio_connection);
        spi_close(fd);

        return 0;
    }

    /* reaching here means the program persists until terminated */

    /* catch SIGINT/SIGTERM */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = ledSigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /*
    * Set up LED message queue interface
    */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    if ((qd_server = mq_open (QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        goto fail_and_close_gpio;
    }

    /* reuse dummy for exit type; set to 0 to since no failures up to here */
    dummy = 0;

    /* Reset LEDs*/ /* JJG ??? do we want to do this while system running? */
    //led_control(fd, LED_RED, LED_OFF);          /* RED OFF */
    //led_control(fd, LED_GREEN, LED_OFF);        /* GREEN OFF */

    /* not changing the implementation; but why are we creating a thread? just use the current one */
    /*
    * Start MQ thread - handling incomming LED commands
    */
    pthread_create( &led_mq_thread, NULL, (void *) &led_mq_handler, NULL);
    pthread_detach( led_mq_thread);

    while (mainloop) {
        /* original implementation was probably better at handling blocking; pthread_join blocks, but
           because I've seen failures with pthread_join on QNX; using this approach */
        usleep(ONE_SECOND);
    }

    /* Wait for threads end */
    pthread_cancel(led_mq_thread);

fail_and_close_gpio:
    gpio_close(gpio_out_en);
fail_and_close_gpiocnxn:
    gpio_disconnect(gpio_connection);
fail_and_close:
    spi_close(fd);
fail_no_close:
    return (!dummy) ? 0 : -1;
}
