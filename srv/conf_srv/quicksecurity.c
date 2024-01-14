/*
** Read security stats from radio shared memory
**
** Outputs in a format to be read by javascript
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>     // for va_list and friends
#include "dn_types.h"
#include "wsu_sharedmem.h"  // for wsu_share_init,kill
#include "../radio/ns/shm_rsk.h"  // for shm_rsk_t


FILE * fdebug = NULL;

void open_debug() { fdebug = fopen("/tmp/zqr_debug.txt","a"); }
void close_debug() { if (fdebug) fclose(fdebug); }
void debug_printf(char * fmt, ...) {
    if (fdebug) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(fdebug, fmt, ap);
        va_end(ap);
    }
}


int main(int argc, char **argv)
{
    shm_rsk_t * rskShmPtr;
    ALSMICntType rsk_alsmicnt_shm_data;
    P2PCntType rsk_p2pcnt_shm_data;

    rskShmPtr = (shm_rsk_t*) wsu_share_init(sizeof(shm_rsk_t), RSK_SHM_PATH);
    rsk_alsmicnt_shm_data = rskShmPtr->ALSMICnt;
    rsk_p2pcnt_shm_data   = rskShmPtr->P2PCnt;

    if (argc == 2 && !strcmp(argv[1],"--webgui-format"))
    {
      //for get_radio_detail_data page

        // Message signing, strips and verifies
        printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,",
            rsk_alsmicnt_shm_data.SignRequests,
            rsk_alsmicnt_shm_data.SignSuccesses,
            rsk_alsmicnt_shm_data.SignFailures,
            rsk_alsmicnt_shm_data.StripRequests,
            rsk_alsmicnt_shm_data.StripSuccesses,
            rsk_alsmicnt_shm_data.StripFailures,
            rsk_alsmicnt_shm_data.VerifyRequests,
            rsk_alsmicnt_shm_data.VerifySuccesses,
            rsk_alsmicnt_shm_data.VerifyFailures);

        printf("\n");

    } 
    else if (argc == 2 && !strcmp(argv[1],"--webgui-format2"))
    {
      //for get_security_data page

        // Message Signing
        printf("%u,%u,%u,%u,",
            rsk_alsmicnt_shm_data.SignRequests,
            rsk_alsmicnt_shm_data.SignSuccesses,
            rsk_alsmicnt_shm_data.SignFailures,
            rsk_alsmicnt_shm_data.LastSignErrorCode);

        // Message Verifications
        printf("%u,%u,%u,%u,%u,%u,%u,%u,",
            rsk_alsmicnt_shm_data.StripRequests,
            rsk_alsmicnt_shm_data.StripSuccesses,
            rsk_alsmicnt_shm_data.StripFailures,
            rsk_alsmicnt_shm_data.LastSignErrorCode,
            rsk_alsmicnt_shm_data.VerifyRequests,
            rsk_alsmicnt_shm_data.VerifySuccesses,
            rsk_alsmicnt_shm_data.VerifyFailures,
            rsk_alsmicnt_shm_data.LastVerifyErrorCode);

        printf("\n");
    }
    else {

        printf("Message Signing\n");
        printf("---------------\n");
        printf("Sign Requests: %u\n", rsk_alsmicnt_shm_data.SignRequests);
        printf("Sign Successes: %u\n", rsk_alsmicnt_shm_data.SignSuccesses);
        printf("Sign Failures: %u\n", rsk_alsmicnt_shm_data.SignFailures);
        printf("Last Sign Error Code: %u\n", rsk_alsmicnt_shm_data.LastSignErrorCode);

        printf("\n");

        printf("Message Verifications\n");
        printf("---------------------\n");
        printf("Successful Stripped Messages: %u\n", rsk_alsmicnt_shm_data.StripSuccesses);
        printf("Unsuccessful Stripped Messages: %u\n", rsk_alsmicnt_shm_data.StripFailures);
        printf("Verification Requests: %u\n", rsk_alsmicnt_shm_data.VerifyRequests);
        printf("Verification Successes: %u\n", rsk_alsmicnt_shm_data.VerifySuccesses);
        printf("Verification Failures: %u\n", rsk_alsmicnt_shm_data.VerifyFailures);
        printf("Last Verification Error Code: %u\n",  rsk_alsmicnt_shm_data.LastVerifyErrorCode);

        printf("\n");

        printf("Peer To Peer Messages\n");
        printf("---------------------\n");
        printf("P2P Transmitted: %u\n", rsk_p2pcnt_shm_data.P2PCallbackCalled);
        printf("P2P Tx Success: %u\n", rsk_p2pcnt_shm_data.P2PCallbackSendSuccess);
        printf("P2P Tx Fail: %u\n", rsk_p2pcnt_shm_data.P2PCallbackNoRadioConfigured + rsk_p2pcnt_shm_data.P2PCallbackLengthError + rsk_p2pcnt_shm_data.P2PCallbackSendError);
        printf("P2P Received: %u\n", rsk_p2pcnt_shm_data.P2PReceived);
        printf("P2P Process Success: %u\n", rsk_p2pcnt_shm_data.P2PProcessed);
        printf("P2P Process Fail: %u\n", rsk_p2pcnt_shm_data.P2PProcessError);

        wsu_share_kill(rskShmPtr, sizeof(shm_rsk_t));

    }

    return 0;
}
