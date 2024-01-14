
/*
** A simple program to get seconds since powerup for an uptime value
*/

#include <stdio.h>
#include <time.h>
#include <dn_types.h>
#include "conf_manager.h"

int32_t main (int32_t argc, char_t **argv)
{
    struct timespec ts;
    unsigned int uptime_days, uptime_hours, uptime_mins, uptime_sec;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uptime_days = ts.tv_sec / (24 * 60 * 60);
    ts.tv_sec -= uptime_days * (24 * 60 * 60);
    uptime_hours = ts.tv_sec / (60 * 60);
    ts.tv_sec -= uptime_hours * (60 * 60);
    uptime_mins = ts.tv_sec / 60;
    uptime_sec = ts.tv_sec  - uptime_mins * 60;

    if (uptime_days > 0) {
        printf("%u day%s; %02u:%02u:%02u\n", uptime_days, 
                uptime_days > 1 ? "s" : "",
                uptime_hours, uptime_mins, uptime_sec);
    } else {
        printf("%02u:%02u:%02u\n", uptime_hours, uptime_mins, uptime_sec);
    }
    
    return 0;
}

