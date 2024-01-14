#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>

//#define SVC_PORT 16052
#define SVC_PORT 16092

#if 0
static int init_sock(void)
{
    struct sockaddr_in6 local;
    struct sockaddr_in6 *lptr;
    int var, sock;
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    char i;

    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&var, sizeof(var)) < 0) {
        printf("setsockopt error\n");
        close(sock);
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_NUMERICHOST;

    var = getaddrinfo("fe80::201:bcff:fe00:22d5", NULL, &hints, &res);
    if (var != 0) {
        printf("couldn't get addr info of local ifc\n");
        return -1;
    }

    lptr = (struct sockaddr_in6 *)res->ai_addr;
    printf("this is ifc scope %d\n", lptr->sin6_scope_id);
    for (i=0; i<14; i=i+2) printf("%X%.2X:", lptr->sin6_addr.s6_addr[i], lptr->sin6_addr.s6_addr[i+1]);
    printf("%X%.2X\n", lptr->sin6_addr.s6_addr[i], lptr->sin6_addr.s6_addr[i+1]);
    printf("this is ifc name %d\n", if_nametoindex("eth0"));
#if 0
    if (res->ai_next != NULL) {
        printf("another address found\n");
        for (rp = res->ai_next; rp != NULL; rp = rp->ai_next) {
            lptr = (struct sockaddr_in6 *)rp->ai_addr;
            printf("scope %d\n", lptr->sin6_scope_id);
            printf("addr ");
            for (i=0; i<14; i=i+2) printf("%X%.2X:", lptr->sin6_addr.s6_addr[i], lptr->sin6_addr.s6_addr[i+1]);
            printf("%X%.2X\n", lptr->sin6_addr.s6_addr[i], lptr->sin6_addr.s6_addr[i+1]);
        }
        freeaddrinfo(res);
        return -1;
    }
#endif
    //fe80::250:b6ff:fe0d:99c4
    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;
    local.sin6_port = htons(SVC_PORT);
    local.sin6_addr.s6_addr[0] = 0xFE;
    local.sin6_addr.s6_addr[1] = 0x80;
#if 1
    local.sin6_addr.s6_addr[2] = 0x40;
    local.sin6_addr.s6_addr[3] = 0x70;
    local.sin6_addr.s6_addr[4] = 0xE0;
    local.sin6_addr.s6_addr[5] = 0xFB;
    local.sin6_addr.s6_addr[6] = 0x99;
    local.sin6_addr.s6_addr[7] = 0x99;
    local.sin6_addr.s6_addr[8] = 0x2;
    local.sin6_addr.s6_addr[9] = 0x80;
    local.sin6_addr.s6_addr[10] = 0x98;
    local.sin6_addr.s6_addr[11] = 0xFF;
    local.sin6_addr.s6_addr[12] = 0xFE;
    local.sin6_addr.s6_addr[13] = 0x10;
    local.sin6_addr.s6_addr[14] = 0xB9;
    local.sin6_addr.s6_addr[15] = 0x7E;
    local.sin6_scope_id = (uint32_t)if_nametoindex("ath0");
#else
    local.sin6_addr.s6_addr[8] = 0x2;
    local.sin6_addr.s6_addr[9] = 0x50;
    local.sin6_addr.s6_addr[10] = 0xB6;
    local.sin6_addr.s6_addr[11] = 0xFF;
    local.sin6_addr.s6_addr[12] = 0xFE;
    local.sin6_addr.s6_addr[13] = 0xD;
    local.sin6_addr.s6_addr[14] = 0x99;
    local.sin6_addr.s6_addr[15] = 0xC4;
    local.sin6_scope_id = (uint32_t)if_nametoindex("eth0");
    //local.sin6_scope_id = lptr->sin6_scope_id;
#endif
    freeaddrinfo(res);
    

    if (connect(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("connect error %d %d (%s)\n", sock, sizeof(local), strerror(errno));
        close(sock);
        return -1;
    }

    printf("connected to sock\n");
    return sock;
}

static void initipv6(char *mac)
{
    char buf[20];
    char cmd[200];
    char *tok, *val;
    FILE *f;
    char i;

    if ((f=fopen("/proc/sys/dev/ccl/radio0_mac_addr", "r")) == NULL) {
        printf("failed to get mac addr file\n");
        return;
    }

    i = fread(buf, sizeof(char), 20, f);
    fclose(f);

    tok = strtok(buf, ":");
    val = mac;
    while (tok != NULL) {
        *val = (char)strtoul(tok, NULL, 16);
        val++;
        tok = strtok(NULL, ":");
    }
    mac[0] ^= 0x02;

    sprintf(cmd, "ip -6 addr add FE80:4070:E0FB:9999:%X%.2X:%XFF:FE%.2X:%X%.2X/64 dev ath0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    system(cmd);
    sprintf(cmd, "ip -6 route add ::/0 via FE80:4070:E0FB:9999:0:0:0:1 dev ath0");
    system(cmd);
    sprintf(cmd, "ip -6 addr add FE80::%X%.2X:%XFF:FE%.2X:%X%.2X/64 dev ath0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    system(cmd);
    sprintf(cmd, "ip -6 neigh add FE80:4070:E0FB:9999:0:0:0:1 dev ath0 lladdr FE:80:40:70:E0:FB nud permanent 2>/dev/null");
    i=system(cmd);
    if (i != 0) printf("neighbor didn't work\n");
    if ((f=fopen("/etc/resolv.conf", "w")) == NULL) {
        printf("open resolve failed\n");
    } else {
        fprintf(f, "nameserver FE80:4070:E0FB:9999:0:0:0:1");
    }
}

static void closeipv6(char *mac)
{
    char cmd[200];

    sprintf(cmd, "ip -6 addr delete FE80::%X%.2X:%XFF:FE%.2X:%X%.2X/64 dev ath0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    system(cmd);
    sprintf(cmd, "ip -6 addr delete FE80:4070:E0FB:9999:%X%.2X:%XFF:FE%.2X:%X%.2X/64 dev ath0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    system(cmd);
    sprintf(cmd, "ip -6 route delete ::/0");
    system(cmd);
    sprintf(cmd, "ip -6 neigh delete FE80:4070:E0FB:9999:0:0:0:1 dev ath0 lladdr FE:80:40:70:E0:FB");
    system(cmd);
}

#endif  //top level if 0

static int init_sock(void) 
{
    int var, sock;
    struct sockaddr_in6 local;
    char name[50];

    printf("JJG: got initial\n");
    setenv("SOCK", "/llc", 1);
    //if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket error (%s)\n", strerror(errno));
        return -1;
    }
    printf("JJG: got here\n");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&var, sizeof(var)) < 0) {
        printf("setsockopt error\n");
        close(sock);
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;
    local.sin6_port = htons(SVC_PORT);
    local.sin6_scope_id = (uint32_t)if_nametoindex("llc0");
    /* hardcoded for now to 2003::10 */
    local.sin6_addr.s6_addr[0] = 0x20;
    local.sin6_addr.s6_addr[1] = 0x00;
    local.sin6_addr.s6_addr[15] = 0x10;

    if (connect(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("connect error %d %d (%s)\n", sock, sizeof(local), strerror(errno));
        usleep(5000);
        memset(name, 0, 50);
        inet_ntop(AF_INET6, local.sin6_addr.s6_addr, name, 50);
        printf("cnxn target %s %d\n", name, local.sin6_port);
        close(sock);
        return -1;
    }

    printf("connected to sock\n");
    return sock;
}

int main(void)
{
    char buf[] = "this is a test\n\n";
    int sock = -1;
    char mac[6];

#if 0
    initipv6(mac);
#endif

    sock = init_sock();
    //usleep(60000000);
    if (sock >= 0) {
        if(write(sock, buf, sizeof(buf)) <= 0) {
            printf("write failed\n");
        }
    }
    if(sock > 0) close(sock);
#if 0
    closeipv6(mac);
#endif
    return 0;
}
