/*
*  Filename: health_monitor_srv.c
*  Purpose: mitigates CPU utilization spike
*
*  Copyright (C) 2020 DENSO International America, Inc.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  05/02/19    SBAUCOM       Initial release.
*  11/13/20    KFRANKEL      Hercules conversion
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
#include <unistd.h>
#include "dn_types.h"
// #include <gpio/gpio.h>
// #include <hw/spi-master.h>
#include "led_common.h"     // for debug_print()
// #include <signal.h>


/* Defines */
#define SIXTY_SECOND 60000000 /*for usleep*/

/* Globals */
static int32_t debug_output = 0;    /* Debug OFF by default */


int32_t main(int32_t argc, char_t * argv[])
{
	int32_t c_arg;

	/* Process debug mode, serial port */
	while ((c_arg = getopt(argc, argv, "d:")) != -1)
	{
		switch (c_arg)
		{
		case 'd':           /* debug mode */
			debug_output = atoi(optarg);
			debug_print("Debug mode %d\n", debug_output);
			break;

		default:
			break;
		}
	}

	while (1)
	{
		usleep(SIXTY_SECOND);
		system("slay ps 2>/dev/null"); /*mitigate CPU utilization spike*/
	}
	return (0);
}
