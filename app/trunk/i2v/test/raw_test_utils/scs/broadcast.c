#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MAX_PKT_SIZE 200
#define MAX_STR_SIZE 100

#define DEFAULT_FILE "ctrl.bin"
#define DEFAULT_IP "10.7.96.25"
#define DEFAULT_PORT  6053

typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOLEAN;

/* courtesy wireshark */
typedef struct pcap_hdr_s {
    unsigned int magicNum;
    unsigned short versionMaj;
    unsigned short versionMin;
    signed int thiszone;
    unsigned int sigfigs;
    unsigned int snaplen;
    unsigned int network;
} __attribute__((packed)) pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    unsigned int tsSec;
    unsigned int tsUsec;
    unsigned int capLen;
    unsigned int origLen;
} __attribute__((packed)) pcaprec_hdr_t;

static BOOLEAN mainloop = TRUE;
static int iterctr;

/* default values */
static char filename[MAX_STR_SIZE + 1] = {0};
static int interval = 100000;   /* 10 hz */
static char targetIp[MAX_STR_SIZE + 1] = {0};
static short int port = DEFAULT_PORT;
static BOOLEAN dontIterate = FALSE;


static void sig_handler(int __attribute__((unused)) sig)
{
    mainloop = FALSE;
}

static int init_sock(void)
{
    struct sockaddr_in local;
    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
       printf("socket creation failed\n");
       return -1;
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    //local.sin_addr.s_addr = htonl(0x0A076FFF);   /* 10.7.111.255 */
    //local.sin_addr.s_addr = htonl(0x0A076001);   /* 10.7.96.1 */
    if (!inet_aton(targetIp, &local.sin_addr)) {
        printf("\nError - IP address invalid %s\n", targetIp);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&local, sizeof(local)) == -1) {
        printf("connect socket failed\n");
        close(sock);
        return -1;
    }

   return sock;
}

/* hdrSz = udp header size = 42 bytes from wireshark
   dataSz = encapsulated TOM message in udp packet (not including tail frame check for udp)
            this is 83 for spat message - FIXME: need to consider other types of data but will 
            need to add additional handling for that anyway to distinguish pcap contents
   pktSz = udp header + tom + pcaprec_hdr_s + frame check = 145
   offset = pcap_hdr_s
   iterate = self explanatory
*/
static void get_data(FILE *f, char *buf, int hdrSz, int dataSz, int pktSz, int offset, BOOLEAN iterate)
{
    int i;

    if (iterate) {
        if (!feof(f)) {
            fseek(f, (iterctr * pktSz), SEEK_SET);
        } else {
            rewind(f);
            iterctr = 0;
        }
        if (offset) {
            if (!feof(f)) {
                fseek(f, offset, SEEK_CUR);
            }
        }
    } else if (offset) {
        if (!feof(f)) {
            fseek(f, offset, SEEK_SET);
        }
    }

    fread(buf, 1, (dataSz + hdrSz), f);
    for (i=0; i<dataSz; i++) {
        buf[i] = buf[i + hdrSz];
    }
}

static BOOLEAN usage(char *name)
{
    char *idx = &name[strlen(name) - 1];
    int count = 0;
    char *printname, *basename = NULL;
    BOOLEAN useRegName = TRUE;

    while (idx != name) {
        if (*idx == '/') {
            if (!count) break;
            basename = (char *)malloc(count);
            if (basename != NULL) {
                memset(basename, 0, count);
                strcpy(basename, ++idx);
                useRegName = FALSE;
            }
            break;
        }
        count++;
        idx--;
    }
    printname = (useRegName) ? name : basename;

    /* actual usage */
    printf("\nUsage: %s [-f <input file>] [-t <target IP>] [-p <target port>] [-i <interval>] [ -s ]\n", printname);
    printf("\n\tWhere all parameters are OPTIONAL:\n");
    printf("\t-f\tInput file name of PCAP file containing TOM format signal controller data\n");
    printf("\t\tOther formats not accepted\n");
    printf("\t-t\tTarget IP of RSE (multicast not supported)\n");
    printf("\t-p\tTarget port of RSE\n");
    printf("\t-s\tSuppress iteration of file (resending when end of file reached) - no input param\n");
    printf("\t-i\tInterval between messages sent where values are:\n");
    printf("\t\t  0  Default interval (100 ms)\n");
    printf("\t\t  1  1 sec\n");
    printf("\t\t  2  5 sec\n");
    printf("\t\t  3  10 sec\n");
    printf("\t\t  4  20 sec\n");
    printf("\nIf no input, a " DEFAULT_FILE " is expected in the current directory\n");
    printf("and the RSE IP is assumed to be %s with port %d\n", DEFAULT_IP, DEFAULT_PORT);
    printf("\nUse CTRL+C to quit\n\n");

    if (!useRegName) free(basename);
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    char c;
    unsigned char userInterval = 0;

    while((c=getopt(argc, argv, "f:i:hp:t:")) != -1) {
        switch (c) {
            case 'f':
                if (strlen(optarg) > MAX_STR_SIZE) {
                    printf("\nERROR - Input filename is too long\n");
                    return FALSE;
                }
                strcpy(filename, optarg);
                break;
            case 'h':
                usage(argv[0]);
                return FALSE;
            case 'i':
                userInterval = atoi(optarg);
                switch (userInterval) {
                    case 0:    break;  /* already set */
                    case 1: interval = 1000000;  break; /* 1 sec */
                    case 2: interval = 5000000;  break; /* 5 sec */
                    case 3: interval = 10000000;  break; /* 10 sec */
                    case 4: interval = 20000000;  break; /* 20 sec */
                    default: printf("\nERROR - invalid interval\n"); return FALSE;
                }
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                dontIterate = TRUE;
                break;
            case 't':
                if (strlen(optarg) > MAX_STR_SIZE) {
                    printf("\nERROR - Input IP is too long\n");
                    return FALSE;
                }
                strcpy(targetIp, optarg);
                break;
            default:
                break;
        }
    }

    if (!strlen(targetIp)) strcpy(targetIp, DEFAULT_IP);
    if (!strlen(filename)) strcpy(filename, DEFAULT_FILE);

    return TRUE;
}

int main(int argc, char *argv[])
{
    int sock;
    struct sigaction sa;
    //char buf[] = "this is a test";
    char buf[MAX_PKT_SIZE];
    FILE *f;

    if (!parseArgs(argc, argv)) {
        return -1;
    }

    sock = init_sock();
    iterctr = 0;

    if (NULL == (f = fopen(filename, "rb"))) {
        printf("\nfailed to open file\n");
        close(sock);
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (sock != -1) {
        while (mainloop) {
            //if (send(sock, buf, 14, MSG_DONTWAIT) == -1) {
            memset(buf, 0, MAX_PKT_SIZE);
            /* see get_data for magic numbers below - initial offset requires both pcap header types */
            get_data(f, buf, 42, 83, 145, sizeof(pcap_hdr_t) + sizeof(pcaprec_hdr_t), !dontIterate);
            if (write(sock, buf, 83) == -1) {   /* from wireshark */
                printf("send failed\n");
            }
            usleep(interval);  /* consider making this a config option */
            printf("next iteration\n");
        }
    }

    close(sock);
    fclose(f);
    printf("exiting...\n");
    return 0;
}
