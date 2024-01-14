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
#include <time.h>
#include <fcntl.h>

#define SVC_PORT 16092

typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOLEAN;

static BOOLEAN mainloop = TRUE;

static void sig_handler(int __attribute__((unused)) sig)
{
    mainloop = FALSE;
}

static int init4_sock(void)
{
    struct sockaddr_in local;
    int sock;
    int var;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
       printf("socket creation failed\n");
       return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&var, sizeof(var)) < 0) {
        printf("setsockopt error\n");
        close(sock);
        return -1;
    }

    if ((var = fcntl(sock, F_GETFL, 0)) < 0) {
        printf("fnctl error\n");
        close(sock);
        return -1;
    }

    var |= O_NONBLOCK;

    if (fcntl(sock, F_SETFL, var) < 0) {
        printf("set fnctl error\n");
        close(sock);
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(SVC_PORT);

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("bind error\n");
        return -1;
    }
    
    if (listen(sock, 1) < 0) {
        printf("listen error\n");
        return -1;
    }
    
    printf("init sock4 success\n");
    return sock;
}

static int init6_sock(void)
{
    struct sockaddr_in6 local;
    int sock;
    int var;

    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
       printf("socket creation failed\n");
       return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&var, sizeof(var)) < 0) {
        printf("setsockopt error\n");
        close(sock);
        return -1;
    }

    if ((var = fcntl(sock, F_GETFL, 0)) < 0) {
        printf("fnctl error\n");
        close(sock);
        return -1;
    }

    var |= O_NONBLOCK;

    if (fcntl(sock, F_SETFL, var) < 0) {
        printf("set fnctl error\n");
        close(sock);
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(SVC_PORT);

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("bind error\n");
        return -1;
    }

    if (listen(sock, 1) < 0) {
        printf("listen error\n");
        return -1;
    }
    
    printf("init sock6 success\n");
    return sock;
}

void process(BOOLEAN version)
{
#define MAX_TCP_DATA 16384
    int lsock = -1, csock = -1;
    struct sockaddr *client;
    int *size;
    int s4 = sizeof(struct sockaddr_in);
    int s6 = sizeof(struct sockaddr_in6);
    struct sockaddr_in c4;
    struct sockaddr_in6 c6;
    fd_set f;
    struct timeval to;
    int i, count, ret;
    char buf[MAX_TCP_DATA] = {0};

    if (version) {
        lsock = init4_sock();
        size = &s4;
        client = (struct sockaddr *)&c4;
    } else {
        lsock = init6_sock();
        size = &s6;
        client = (struct sockaddr *)&c6;
    }

    while (mainloop) {
        while (mainloop) {
            if ((csock = accept(lsock, client, size)) >= 0) {
                break;
            }
            if ((errno==EAGAIN)||(errno==EWOULDBLOCK)) {
                usleep(1000);
                continue;
            }
            printf("accept error listening (%s)\n", strerror(errno));
            /* mainloop = FALSE;
            printf("%d %d\n", lsock, *size);
            break; */
            continue;
        }
        if (!mainloop) break;

        while (mainloop) {
            FD_ZERO(&f);
            FD_SET(csock, &f);
            to.tv_sec = 15;
            to.tv_usec = 0;

            if ((ret = select(csock + 1, &f, NULL, NULL, &to)) < 0) {
                printf("error in retrieving client data\n");
                close(csock); csock = -1;
                break;
            } else if (!ret) {
                printf("timeout in retrieving client data\n");
                close(csock); csock = -1;
                break;
            }
            if (FD_ISSET(csock, &f)) {
                if ((count=read(csock, buf, MAX_TCP_DATA)) <= 0) {
                    printf("read error or disconnect\n");
                    close(csock); csock = -1;
                    break;
                } else {
                    printf("received client data: (%s)\n", buf);
                    /*for (i=0; i<count; i++) {
                        printf("%d ", buf[i]);
                    }
                    printf("\n"); */
                }
            }
        }
        if (!mainloop) break;
    }

    if (csock > 0) close(csock);
    if (lsock > 0) close(lsock);
}

int main(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    process(FALSE);

    printf("exiting...\n");
    return 0;
}
