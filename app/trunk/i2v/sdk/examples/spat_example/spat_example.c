/****************************************************************************
*      DENSO International America, Inc.                                   *
*      North America Research Laboratory, California Office                *
*      3252 Business Park Drive                                            *
*      Vista, CA 92081                                                     *
****************************************************************************/

/**
  This is a simple example showing how to access DENSO RSU shared memory
  for SPaT data
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#include "rsuSdkShm.h"

/* RSU threads typically operate at 10 Hz; value below is for microseconds */
#define MINIMUM_SLEEP   1000000

static RSUBOOL mainloop = RSUTRUE;

/**
  @FXN: sigHandler
  @DESCRIPTION: signal handler function

  @RETURN: None

  @PARAM: sig - unused parameter passed when signal received

*/
static void sigHandler(int __attribute__((unused)) sig)
{
    mainloop = RSUFALSE;
}

/**
  @FXN: rsuShmAttach
  @DESCRIPTION: RSU shared memory attach function

  @PARAM: size - size of the shared memory
  @PARAM: spath - string path to the shared memory

  @RETURN: None

  @NOTE: This is the REQUIRED method to attach to RSU memory

*/
void *rsuShmAttach(size_t size, char *spath)
{
    char *shmAddr;

    int fd = shm_open(spath, O_RDONLY, 0444);
    if (fd < 0) {
        printf("ERROR: Failed to attach to RSU shm\n");
        return NULL;
    }
    shmAddr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if ((long) shmAddr == -1 || (long) shmAddr == 0) {
        printf("ERROR: mmap failed of RSU shm (%p)\n", shmAddr);
        return NULL;
    }

    return shmAddr;
}

/**
  @FXN: rsuShmDetach
  @DESCRIPTION: RSU shared memory detach attach function

  @PARAM: shmAddr - memory mapped address to the shared memory
  @PARAM: size - size of the shared memory

  @RETURN: None

  @NOTE: This is the REQUIRED method to attach to RSU memory

*/
void rsuShmDetach(void *shmAddr, size_t size)
{
    if (munmap(shmAddr, size) < 0) {
        printf("ERROR: munmap failed, errno=%d\n", errno);
    }
}

/**
  @FXN: getPhaseColor
  @DESCRIPTION: Converts phase numeric value into color string

  @PARAM: phaseVal - input phase numeric value from SPaT data

  @RETURN: Color string

  @NOTE: The color mapping is standard

*/
char *getPhaseColor(unsigned int phaseVal)
{
    switch (phaseVal) {
        case 1: return "Green";
        case 2: return "Yellow";
        case 4: return "Red";
        default: return "Dark";
    }
}

/**
  @FXN: main
  @DESCRIPTION: Main function

  @PARAM: argc - number of input arguments
  @PARAM: argv - string arguments

  @RETURN: Negative = error, 0 = no error, Positive = Unsupported

  @NOTE: user input is not illustrated, but can be added as desired;
         use Ctrl+C to exit

*/
int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
    rsuShmMasterT *shmPtr;
    rsuSPATDataType localSpatCopy;
    struct sigaction sa;
    int i;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    shmPtr = rsuShmAttach(sizeof(rsuShmMasterT), I2V_SHM_PATH);
    if (NULL == shmPtr) {
        printf("Exiting prematurely...\n");
        return -1;
    }

    if (!shmPtr->spatType) {
        printf("RSU is using legacy SPaT definition; unsupported in example!\nExiting...\n");
        rsuShmDetach(shmPtr, sizeof(rsuShmMasterT));
        return 1;  /* non-negative return */
    }

    while(mainloop) {
        /* print spat data periodically */
        usleep(MINIMUM_SLEEP);
        /* do not take time using shared memory, make a local copy */
        memcpy(&localSpatCopy, &shmPtr->liveData.modernSpat, sizeof(localSpatCopy));
        printf("Intersection ID: %d\n\n", localSpatCopy.intID);
        for (i=0; i<localSpatCopy.numApproach; i++) {
            printf("Approach ID[%d]: Color ==> %s Countdown ==> %d seconds\n", localSpatCopy.spatApproach[i].approachID,
                    getPhaseColor(localSpatCopy.spatApproach[i].curSigPhase), localSpatCopy.spatApproach[i].timeNextPhase/10);
        }
        printf("\n\n");
    }

    printf("Exiting operation\n");
    return 0;
}

