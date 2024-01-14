#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "wmh_api.h"
#include "wsu_sharedmem.h"

static bool_t mainLoop = TRUE;

static void exitHandler(int signal)
{
    if (mainLoop) {
        printf("exitHandler is called.\n");
    }

    mainLoop = FALSE;
}

int main(int argc, char **argv)
{
    shm_wmh_t *wmh_shm_ptr;

    wmh_shm_ptr = (shm_wmh_t *)wsu_share_init(sizeof(shm_wmh_t), WMH_SHM_PATH);

    if (wmh_shm_ptr == NULL) {
        printf("ERROR: Could not Map WMH shared memory!\n");
        return -1;
    }

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    while (mainLoop) {
        printf("%s", "\033[2J");
        printf("dwmhMonitorThreadCount    = %u\n", wmh_shm_ptr->dwmhMonitorThreadCount);
        printf("dwmhCrunchLiveDataCount   = %u\n", wmh_shm_ptr->dwmhCrunchLiveDataCount);
        printf("dwmhBSMTxCount            = %u\n", wmh_shm_ptr->dwmhBSMTxCount);
        printf("wsuRisSendWSMReqCallCount = %u\n\n", wmh_shm_ptr->wsuRisSendWSMReqCallCount);

        printf("dwmhReadLiveDataCode      = %u\n", wmh_shm_ptr->dwmhReadLiveDataCode);
        printf("dwmhCrunchLiveDataCode    = %u\n", wmh_shm_ptr->dwmhCrunchLiveDataCode);
        printf("dwmhProcessLiveInputCode  = %u\n", wmh_shm_ptr->dwmhProcessLiveInputCode);
        printf("dwmhBSMTxCode             = %u\n", wmh_shm_ptr->dwmhBSMTxCode);
        sleep(1);
    }

    wsu_share_kill(wmh_shm_ptr, sizeof(shm_wmh_t));
}

