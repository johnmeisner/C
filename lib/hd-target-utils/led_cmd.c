/*
 *  Filename: led_cmd.c
 *  Purpose: Test utility sending led command
 *
 *  Copyright (C) 2018 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  01-12-17    LNGUYEN    Initial release.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <process.h>
#include <string.h>
#include "led_common.h"

/* Global */
mq_led_hdr_t mq_hdr;
mqd_t qd_server = 0;
mqd_t qd_client = 0;

/*
 * Send LED cmd
 */
wint32 led_send_cmd(wuint8 *led_cmd, wint32 len)
{
    wint32 iRet = 0;
    mqd_t qd_server;
    
    /* Error check */
    if (led_cmd == NULL) {
      return iRet;
    }
    
    /* Open HCI Server mqueue */
    if ((qd_server = mq_open (QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        iRet = 1;
        goto led_send_cmd_exit;
    }

    /* send message to server */
    if (mq_send (qd_server, (char *)led_cmd, len, 0) == -1) {
        perror ("Client: Not able to send message to server");
        iRet = 1;
    }

led_send_cmd_exit:

    return iRet;
}

wint32 main (wint32 argc, wint8 **argv)
{
    wint32 iRet;
    mq_led_hdr_t led_hdr;

    /* Error check */
    if( argc < 3 ) {
      printf("Usage: %s <green|red> <on|off> <duration ms>\n", argv[0]);
      exit(0);
    }
    
    /* Check for LED type: RED or GREEN */
    if (0 == (strcmp("GREEN", strupr(argv[1]) )) ) {
            led_hdr.led_attr.led_type = LED_GREEN;
    } else if (0 == (strcmp("RED", strupr(argv[1]))) ) {
            led_hdr.led_attr.led_type = LED_RED;
    } else if (0 == (strcmp("AMBER", strupr(argv[1]))) ) {
            led_hdr.led_attr.led_type = LED_AMBER;
    } else {
        printf("Usage: %s <green|red> <on|off> <duration ms>\n", argv[0]);
        exit (0);
    }
        
    /* Check for LED mode: ON or OFF */
    if (0 == (strcmp("ON", strupr(argv[2]))) ) {
            led_hdr.led_attr.led_en = LED_ON;
    } else if (0 == (strcmp("OFF", strupr(argv[2]))) ) {
            led_hdr.led_attr.led_en = LED_OFF;
    } else {
        printf("Usage: %s <green|red> <on|off> <duration ms>\n", argv[0]);
        exit (0);
    }
        
    /* Check for blink rate (optional) */
    if (argc > 3) {
        led_hdr.led_attr.blink_rate = atoi(argv[3]);
    }
    else {
        led_hdr.led_attr.blink_rate = 0;
    }

    led_hdr.proc_id = getpid();
    led_hdr.thrd_id = gettid();

    /* Send the led command to led_srv via MQ */
    iRet = led_send_cmd( (wuint8 *)&led_hdr, sizeof(mq_led_hdr_t));

    return (iRet);
}
