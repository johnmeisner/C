//
// A simple utility library to tally received ubx messages and to give ubx names to numeric ubx id's.
//

#include <stdio.h>
#include <string.h>     // memcpy, memset
#include "types.h"
#include "ubx_track.h"
#include <sys/time.h>   // gettimeofday


// Simple data for names for UBX messages
struct ubx_msg_info {
    char msgname[25];
    unsigned char classid;
    unsigned char msgid;
};
#define UBX_MSG_COUNT 125
struct ubx_msg_info ubx_msgs[UBX_MSG_COUNT] = {
    {"UBX-ACK-ACK", 0x05, 0x01},
    {"UBX-ACK-NAK", 0x05, 0x00},
    {"UBX-AID-ALM", 0x0B, 0x30},
    {"UBX-AID-AOP", 0x0B, 0x33},
    {"UBX-AID-EPH", 0x0B, 0x31},
    {"UBX-AID-HUI", 0x0B, 0x02},
    {"UBX-AID-INI", 0x0B, 0x01},
    {"UBX-CFG-ANT", 0x06, 0x13},
    {"UBX-CFG-CFG", 0x06, 0x09},
    {"UBX-CFG-DAT", 0x06, 0x06},
    {"UBX-CFG-DOSC", 0x06, 0x61},
    {"UBX-CFG-DYNSEED", 0x06, 0x85},
    {"UBX-CFG-ESRC", 0x06, 0x60},
    {"UBX-CFG-FIXSEED", 0x06, 0x84},
    {"UBX-CFG-GEOFENCE", 0x06, 0x69},
    {"UBX-CFG-GNSS", 0x06, 0x3E},
    {"UBX-CFG-HNR", 0x06, 0x5C},
    {"UBX-CFG-INF", 0x06, 0x02},
    {"UBX-CFG-ITFM", 0x06, 0x39},
    {"UBX-CFG-LOGFILTER", 0x06, 0x47},
    {"UBX-CFG-MSG", 0x06, 0x01},
    {"UBX-CFG-NAV5", 0x06, 0x24},
    {"UBX-CFG-NAVX5", 0x06, 0x23},
    {"UBX-CFG-NMEA", 0x06, 0x17},
    {"UBX-CFG-ODO", 0x06, 0x1E},
    {"UBX-CFG-PM2", 0x06, 0x3B},
    {"UBX-CFG-PMS", 0x06, 0x86},
    {"UBX-CFG-PRT", 0x06, 0x00},
    {"UBX-CFG-PWR", 0x06, 0x57},
    {"UBX-CFG-RATE", 0x06, 0x08},
    {"UBX-CFG-RINV", 0x06, 0x34},
    {"UBX-CFG-RST", 0x06, 0x04},
    {"UBX-CFG-RXM", 0x06, 0x11},
    {"UBX-CFG-SBAS", 0x06, 0x16},
    {"UBX-CFG-SMGR", 0x06, 0x62},
    {"UBX-CFG-TMODE2", 0x06, 0x3D},
    {"UBX-CFG-TP5", 0x06, 0x31},
    {"UBX-CFG-TXSLOT", 0x06, 0x53},
    {"UBX-CFG-USB", 0x06, 0x1B},
    {"UBX-ESF-INS", 0x10, 0x15},
    {"UBX-ESF-MEAS", 0x10, 0x02},
    {"UBX-ESF-RAW", 0x10, 0x03},
    {"UBX-ESF-STATUS", 0x10, 0x10},
    {"UBX-HNR-PVT", 0x28, 0x00},
    {"UBX-INF-DEBUG", 0x04, 0x04},
    {"UBX-INF-ERROR", 0x04, 0x00},
    {"UBX-INF-NOTICE", 0x04, 0x02},
    {"UBX-INF-TEST", 0x04, 0x03},
    {"UBX-INF-WARNING", 0x04, 0x01},
    {"UBX-LOG-CREATE", 0x21, 0x07},
    {"UBX-LOG-ERASE", 0x21, 0x03},
    {"UBX-LOG-FINDTIME", 0x21, 0x0E},
    {"UBX-LOG-INFO", 0x21, 0x08},
    {"UBX-LOG-RETRIEVE", 0x21, 0x09},
    {"UBX-LOG-RETRIEVEPOS", 0x21, 0x0b},
    {"UBX-LOG-RETRIEVEPOSEXTRA", 0x21, 0x0f},
    {"UBX-LOG-RETRIEVESTRING", 0x21, 0x0d},
    {"UBX-LOG-STRING", 0x21, 0x04},
    {"UBX-MGA-ACK", 0x13, 0x60},
    {"UBX-MGA-ANO", 0x13, 0x20},
    {"UBX-MGA-BDS", 0x13, 0x03},
    {"UBX-MGA-DBD", 0x13, 0x80},
    {"UBX-MGA-FLASH", 0x13, 0x21},
    {"UBX-MGA-GAL", 0x13, 0x02},
    {"UBX-MGA-GLO", 0x13, 0x06},
    {"UBX-MGA-GPS", 0x13, 0x00},
    {"UBX-MGA-INI", 0x13, 0x40},
    {"UBX-MGA-QZSS", 0x13, 0x05},
    {"UBX-MON-GNSS", 0x0A, 0x28},
    {"UBX-MON-HW", 0x0A, 0x09},
    {"UBX-MON-HW2", 0x0A, 0x0B},
    {"UBX-MON-IO", 0x0A, 0x02},
    {"UBX-MON-MSGPP", 0x0A, 0x06},
    {"UBX-MON-PATCH", 0x0A, 0x27},
    {"UBX-MON-RXBUF", 0x0A, 0x07},
    {"UBX-MON-RXR", 0x0A, 0x21},
    {"UBX-MON-SMGR", 0x0A, 0x2E},
    {"UBX-MON-TXBUF", 0x0A, 0x08},
    {"UBX-MON-VER", 0x0A, 0x04},
    {"UBX-NAV-AOPSTATUS", 0x01, 0x60},
    {"UBX-NAV-ATT", 0x01, 0x05},
    {"UBX-NAV-CLOCK", 0x01, 0x22},
    {"UBX-NAV-DGPS", 0x01, 0x31},
    {"UBX-NAV-DOP", 0x01, 0x04},
    {"UBX-NAV-EOE", 0x01, 0x61},
    {"UBX-NAV-GEOFENCE", 0x01, 0x39},
    {"UBX-NAV-ODO", 0x01, 0x09},
    {"UBX-NAV-ORB", 0x01, 0x34},
    {"UBX-NAV-POSECEF", 0x01, 0x01},
    {"UBX-NAV-POSLLH", 0x01, 0x02},
    {"UBX-NAV-PVT", 0x01, 0x07},
    {"UBX-NAV-RESETODO", 0x01, 0x10},
    {"UBX-NAV-SAT", 0x01, 0x35},
    {"UBX-NAV-SBAS", 0x01, 0x32},
    {"UBX-NAV-SOL", 0x01, 0x06},
    {"UBX-NAV-STATUS", 0x01, 0x03},
    {"UBX-NAV-SVINFO", 0x01, 0x30},
    {"UBX-NAV-TIMEBDS", 0x01, 0x24},
    {"UBX-NAV-TIMEGAL", 0x01, 0x25},
    {"UBX-NAV-TIMEGLO", 0x01, 0x23},
    {"UBX-NAV-TIMEGPS", 0x01, 0x20},
    {"UBX-NAV-TIMELS", 0x01, 0x26},
    {"UBX-NAV-TIMEUTC", 0x01, 0x21},
    {"UBX-NAV-VELECEF", 0x01, 0x11},
    {"UBX-NAV-VELNED", 0x01, 0x12},
    {"UBX-RXM-IMES", 0x02, 0x61},
    {"UBX-RXM-MEASX", 0x02, 0x14},
    {"UBX-RXM-PMREQ", 0x02, 0x41},
    {"UBX-RXM-RAWX", 0x02, 0x15},
    {"UBX-RXM-RLM", 0x02, 0x59},
    {"UBX-RXM-SFRBX", 0x02, 0x13},
    {"UBX-RXM-SVSI", 0x02, 0x20},
    {"UBX-SEC-SIGN", 0x27, 0x01},
    {"UBX-SEC-UNIQID", 0x27, 0x03},
    {"UBX-TIM-DOSC", 0x0D, 0x11},
    {"UBX-TIM-FCHG", 0x0D, 0x16},
    {"UBX-TIM-HOC", 0x0D, 0x17},
    {"UBX-TIM-SMEAS", 0x0D, 0x13},
    {"UBX-TIM-SVIN", 0x0D, 0x04},
    {"UBX-TIM-TM2", 0x0D, 0x03},
    {"UBX-TIM-TOS", 0x0D, 0x12},
    {"UBX-TIM-TP", 0x0D, 0x01},
    {"UBX-TIM-VCOCAL", 0x0D, 0x15},
    {"UBX-TIM-VRFY", 0x0D, 0x06},
    {"UBX-UPD-SOS", 0x09, 0x14}
};

char * getUbxMsgName(unsigned char classid, unsigned char msgid)
{
    int i;

    for (i=0; i<UBX_MSG_COUNT; i++) {
        if (ubx_msgs[i].classid == classid && ubx_msgs[i].msgid == msgid) {
            return ubx_msgs[i].msgname;
        }
    }
    return "Unknown UBX Msg";
}







#define MAX_UBX_TRACK  100
#define MAX_UBX_MSG_SIZE 800
uint8_t  ubx_track_classid[MAX_UBX_TRACK];
uint8_t  ubx_track_msgid[MAX_UBX_TRACK];
uint32_t  ubx_track_count[MAX_UBX_TRACK];
uint32_t  ubx_track_last_size[MAX_UBX_TRACK];
uint32_t  ubx_track_max_size[MAX_UBX_TRACK];
uint8_t ubx_track_msgbuff[MAX_UBX_TRACK][MAX_UBX_MSG_SIZE];
uint8_t  ubx_track_population = 0;
// For rates,delta tracking
uint32_t  ubx_track_prev_count[MAX_UBX_TRACK];
struct timeval prev_status_timeval = {0};

void ubx_track_reset() { ubx_track_population = 0; }

void ubx_track_add(uint8_t classid, uint8_t msgid, uint32_t size, uint8_t * msgbuff)
{
    int msg_index = -1;
    int i;
    for (i=0; i<ubx_track_population && msg_index < 0; i++) {
        if (ubx_track_classid[i] == classid && ubx_track_msgid[i] == msgid) {
            msg_index = i;
        }
    }
    if (msg_index < 0 && ubx_track_population < MAX_UBX_TRACK-2) {
        ubx_track_classid[ubx_track_population] = classid;
        ubx_track_msgid[ubx_track_population] = msgid;
        ubx_track_count[ubx_track_population] = 0;
        msg_index = ubx_track_population;
        ++ubx_track_population;
    }
    if (msg_index < 0) {
        printf("ERROR! ubx_track: Exceeded max message types!\n");
        return;
    }

    ++ubx_track_count[msg_index];
    if (msgbuff != 0L) {
        if (size <= MAX_UBX_MSG_SIZE) {
            memcpy(ubx_track_msgbuff[msg_index], msgbuff, size);
        } else {
            if (ubx_track_last_size[msg_index] <= MAX_UBX_MSG_SIZE) {
                memset(ubx_track_msgbuff[msg_index], 0, MAX_UBX_MSG_SIZE);
            }
        }
    }
    ubx_track_last_size[msg_index] = size;
    if (size > ubx_track_max_size[msg_index]) {
        ubx_track_max_size[msg_index] = size;
    }
}

uint8_t get_ubx_track_data(uint8_t classid, uint8_t msgid, ubx_track_msg_data_t * ptr)
{
    int i;
    for (i=0; i<ubx_track_population; i++) {
        if (ubx_track_classid[i] == classid && ubx_track_msgid[i] == msgid) {
            ptr->count = ubx_track_count[i];
            ptr->max_size = ubx_track_max_size[i];
            ptr->last_size = ubx_track_last_size[i];
            ptr->last_msg = &ubx_track_msgbuff[i][0];
            return 1;
        }
    }
    ptr->count = 0;
    return 0;
}

/* Named dump of UBX msg tracking buffer */
void ubx_track_dump(int w_msg_names, int w_msg_data)
{
    int i,j;
    char buff[30];
    double delta_secs = 0;
    struct timeval now_timeval;

    gettimeofday(&now_timeval, NULL);
    if (prev_status_timeval.tv_sec > 0) {
        delta_secs = (now_timeval.tv_sec - prev_status_timeval.tv_sec) + ((now_timeval.tv_usec - prev_status_timeval.tv_usec)/1000000.0);
    }

    if (w_msg_names) {
        printf("Message Name      ");
    }
    printf("ClassId  MsgId   Count");
    if (delta_secs > 0) {
        printf("  MsgRate");
    }
    printf("   MaxSize  LastSize");
    if (w_msg_data) {
        printf("  LastMsgData");
    }
    printf("\n");
    for (i=0; i<ubx_track_population; i++) {
        if (w_msg_names) {
            strcpy(buff, getUbxMsgName(ubx_track_classid[i],ubx_track_msgid[i]));
            while (strlen(buff) < 20) {
                strcat(buff, "    ");
            }
            buff[16] = 0;
            printf("  %s", buff);
        }
        printf("  0x%02x   0x%02x    %5u",
            ubx_track_classid[i],
            ubx_track_msgid[i],
            ubx_track_count[i]);
        if (delta_secs > 0) {
            printf("%7.1f/s", (double) (ubx_track_count[i] - ubx_track_prev_count[i]) / delta_secs);
        }
        printf("  %8u  %8u",
            ubx_track_max_size[i],
            ubx_track_last_size[i]);
        if (w_msg_data) {
            printf(" ");
            for (j=0; j<ubx_track_last_size[i] && j<MAX_UBX_MSG_SIZE; j++) {
                printf(" %02x", ubx_track_msgbuff[i][j]);
            }
        }
        printf("\n");
    }
    prev_status_timeval.tv_sec = now_timeval.tv_sec;
    prev_status_timeval.tv_usec = now_timeval.tv_usec;
    for (i=0; i<ubx_track_population; i++) {
        ubx_track_prev_count[i] = ubx_track_count[i];
    }
}

#ifdef WANT_DEBUG_MAIN



uint32_t get_ubx_count(uint8_t classid, uint8_t msgid)
{
    int i;
    for (i=0; i<ubx_track_population; i++) {
        if (ubx_track_classid[i] == classid && ubx_track_msgid[i] == msgid) {
            return ubx_track_count[i];
        }
    }
    return 0;
}

void ubx_track_test1()
{
    int i;
    ubx_track_reset();

    for (i=0; i<10; i++) {
        ubx_track_add(1,1,7,0);
        ubx_track_add(2,1,0,0);
        ubx_track_add(2,2,0,0);
    }
    for (i=0; i<4; i++) {
        ubx_track_add(1,1,5,(uint8_t *)"Hello");
        ubx_track_add(i+4, 5,0,0);
    }
    ubx_track_dump(0,0);
    printf("\n");
    ubx_track_dump(1,0);
    printf("\n");
    ubx_track_dump(1,1);
    printf("\n");

    printf("1,1 - %s\n", get_ubx_count(1,1) == 14 ? "passed" : "FAILED!");
    printf("2,1 - %s\n", get_ubx_count(2,1) == 10 ? "passed" : "FAILED!");
    printf("2,2 - %s\n", get_ubx_count(2,2) == 10 ? "passed" : "FAILED!");
    printf("2,2 - %s\n", get_ubx_count(2,2) == 10 ? "passed" : "FAILED!");
    printf("4,5 - %s\n", get_ubx_count(4,5) == 1 ? "passed" : "FAILED!");
    printf("5,5 - %s\n", get_ubx_count(5,5) == 1 ? "passed" : "FAILED!");
    printf("6,5 - %s\n", get_ubx_count(6,5) == 1 ? "passed" : "FAILED!");
    printf("7,5 - %s\n", get_ubx_count(7,5) == 1 ? "passed" : "FAILED!");

    ubx_track_msg_data_t x;
    printf("lastMsg(1,1) == hello - %s\n", get_ubx_track_data(1,1,&x) && strncmp((char *)x.last_msg,"Hello",5) == 0 ? "passed" : "FAILED!");
}

int main()
{
    ubx_track_test1();
    return 0;
}

#endif
