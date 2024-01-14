/*
** RUNAGAIN -- Records how long since a program was first run in a time period, and decides if we should run program again (outside of that time period)
**
** USAGE:  runagain program_name period_secs [Command]
**    Three parameter Mode: Returns "True" or "False" if the passed program needs to be run again.  Also returns error code 0 for False, 1 for True.
**    Four parameter Mode: Runs the passed command (4th passed param) if it needs to be run again.  Returns error code 0 for wasn't run, 1 if it was.
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define LAST_RAN_FILE_FORMAT "/tmp/zlast_ran.%s"

int main(int argc, char **argv) 
{
    FILE * fstatef;
    char statefile[100];
    int period_sec;
    long unsigned int last_run_time;
    time_t now_time = time(0L);
    int ret = 1;

    if (argc != 3 && argc != 4) {
        printf("Usage: runagain name period_secs [Command]\n");
        exit(0);
    }

    sprintf(statefile, LAST_RAN_FILE_FORMAT, argv[1]);
    period_sec = atoi(argv[2]);

    // Read last_run file for "name", and if within time period, say dont re-run program yet
    fstatef = fopen(statefile, "r");
    if (fstatef) {
        if (1 == fscanf(fstatef, "%lu", &last_run_time)) {
            if (now_time - (time_t) last_run_time < period_sec) {
                ret = 0;
            }
        }
        fclose(fstatef);
    }

    // Save now as last_run if we are recommending program "name" be run
    if (ret) {
        if ((fstatef = fopen(statefile, "w"))) {
            fprintf(fstatef, "%lu\n", (long unsigned int) now_time);
            fclose(fstatef);
        } else {
            fprintf(stderr,"RUNAGAIN: ERROR: Unable to create state file %s\n", statefile);
        }
    }

    // If user passed us a command, run that command
    if (argc == 4) {
        if (ret) {
            system(argv[3]);
        }
    // Otherwise, report back to user if they should run their program again
    } else {
        printf(ret ? "True\n" : "False\n");
    }

    return ret;
}
