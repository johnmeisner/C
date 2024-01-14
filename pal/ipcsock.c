/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: ipcsock.c                                                    */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Library of IPC messaging functions                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <netinet/in.h> // for INADDR_ANY
#include <arpa/inet.h>  // for inet_addr()
#include "ipcsock.h"
#include "dn_types.h"




/**
 * @brief Creates a socket and and binds it a port.
 * @note Function name is misleading; we're not really creating a server.
 * @todo Rename to createAndBindSocket()
 * @return Returns file descriptor of socket or -1 on errors
*/
int32_t wsuCreateSockServer(uint16_t port)
{
    int32_t enable = 1;
    struct sockaddr_in sin;
    int32_t sock_fd;

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("wsuCreateSockServer: createSocked failed, errno=%d!\n", errno);
        return -1;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int32_t)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed");
        close(sock_fd);
        return -1;
    }

    memset((char_t *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) < 0)
    {
        printf("wsuCreateSockServer: Bind on port %d failed, errno=%d!\n", port, errno);
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}


/**
 * @note This function does nothing in the QNX port because DGRAMs
 */
int32_t zrm_wsuWaitOnSingleSockConn(int32_t sockFd)
{
    return sockFd;      // With DGrams we don't need to wait for remotes to connect; we can recvfrom immediately
}




/**
 * @brief Creates a socket for sending messages
 * @note No connect is necessary for sending DGram, so this just
 *       creates our sending socket.  It also does bind(0) so there's a return
 *       address.
 * @todo: Rename to wsuCreateSendingSocket() (no params)
 * @return Returns file descriptor of socket or -1 on errors
 */
int32_t wsuConnectSocket (uint16_t ignored_unused_port)
{
    struct sockaddr_in myaddr_in;
    int32_t sock_fd;

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("wsuConnectSocket: create socket failed, errno=%d!\n", errno);
        return -1;
    }

    memset((char_t *)&myaddr_in, 0, sizeof(myaddr_in));
    myaddr_in.sin_family = AF_INET;
    myaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr_in.sin_port = htons(0);

    if (bind(sock_fd, (struct sockaddr *)&myaddr_in, sizeof(myaddr_in)) < 0) {
        printf("bind myaddr_in failed");
        return -1;
    }
    //printf("DEBUG: wsuConnectSocket: Successfully bound myself to port %d\n", myaddr_in.sin_port);

    return sock_fd;
}

/**
 * @brief Sends data to a socket + port
 */
bool_t wsuSendData (int32_t sock_fd, uint16_t port, void *data, size_t size)
{
    struct sockaddr_in dest;

    memset((char_t *) &dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* localhost, 127.0.0.1 */
    dest.sin_port = htons(port);

    while (sendto(sock_fd, data, size, 0, (struct sockaddr *) &dest, sizeof(dest)) < 0)
    {
        if (errno != EINTR)
        {
            printf("wsuSendData: SendTo of %lu bytes of data to port %d failed, errno=%d, sockfd=%d, pid=%d!\n", size, port, errno, sock_fd, getpid());
            return FALSE;
        }
    }
    //printf("wsuSendData: DEBUG: Sent %d bytes of data to port %d\n", size, port);
    return TRUE;
}


/*
 * @brief Receives data on our RcvDataType
 * @note  The timeout parameter (ignored) is ignored
 * @return Returns size (in bytes) of message or -1 on errors
 */
int32_t wsuReceiveData (uint32_t ignored_unused_timeout, RcvDataType *rcv_data)
{
    struct sockaddr_in from_in;
    socklen_t from_len = sizeof(from_in);
    int32_t n;

    /*
     * Usage Note:
     *  * The socket is created with  socket(AF_INET, SOCK_DGRAM, 0)
     *  * As such the return value & errno is interpreted as:
     *      >0  == number of bytes in packet returned by recvfrom()
     *       0  == zero-byte packet received (normal condition for SOCK_DGRAM)
     *      <0  == (only -1) A socket error condition specified by errno.
     *  * wsuReceiveData() interprets the recvfrom() errno and and sets
     *    its own return value accordingly.
     *       --EINTR:  Return zero when recvfrom() returns -1 due to EINTR error.
     *         This allows the calling function to treat the EINTR condition the
     *         same as receiving a zero-length packet. (usually ignore/continue)
     *       --BADF:  Return -1, but do not print an error msg since normal
     *         during thread shutdown.
     *       --All other errors:  Print a error message and return -1
     *         The calling function usually can ignore and continue.
     *
     *  * The DENSO V2X code (the calling functions) currently does not detect
     *    when the socket goes away (e.g. when process at other end crashes
     */

    rcv_data->available = FALSE;

    while ((n = recvfrom(rcv_data->fd, rcv_data->data, rcv_data->size, 0, (struct sockaddr *)&from_in, &from_len)) < 0)
    {
        if (n < 0)
        {
            if (errno != EINTR)
            {
                /* Don't print the error messages for EBADF. This can happen
                 * normally when we close a socket during the normal thread
                 * shutdown procedure. */
                if (errno != EBADF)
                {
                    printf("wsuReceiveData: Recvfrom failed, errno=%d!\n", errno);
                }

#ifdef DEBUG_SOCKET_EINTR
                printf("%s:DEBUG: recvfrom() (<0) rtn=%d, errno=%d[%s] [!=EINTR] fd=%d\n",
                        __func__, n, errno, strerror(errno), rcv_data->fd);    /*DEBUG*/
#endif /*DEBUG_SOCKET_EINTR*/
                return -1;
            }
#ifdef DEBUG_SOCKET_EINTR
            printf("%s:DEBUG: recvfrom() (<0) rtn=%d, errno=%d[%s] fd=%d [rtn 0]\n",
                    __func__, n, errno, strerror(errno), rcv_data->fd);   /*DEBUG*/
#endif /*DEBUG_SOCKET_EINTR*/
            return 0;
        }
    }
    //printf("wsuReceiveData: DEBUG: Received %d bytes of data on socket %d\n", n, rcv_data->fd);

    rcv_data->available = TRUE;
    rcv_data->sender_port = ntohs(from_in.sin_port);

#ifdef DEBUG_SOCKET_EINTR
   if (n==0)
       printf("%s:DEBUG: recvfrom() (==0) rtn=%d, errno=%d[%s] fd=%d\n",__func__,n,errno,strerror(errno),rcv_data->fd);/*DEBUG*/
#endif /*DEBUG_SOCKET_EINTR*/

    return n;
}





/*
 * @brief Creates a detached thread running the passed thread function.
 * @param thread_main -- [input]Pointer to the thread's main function.
 * @param arg         -- [input]Argument to pass to the thread's main function.
 * @param threadID    -- [output]The value of the pthread_t that is created by
 *                       pthread_create() is written to the location pointed to
 *                       by threadID.
 * @return 0 if successful, -1 if error
 */
int wsuCreateThread(void *(*thread_main)(void *), void *arg, pthread_t *threadID)
{
    int status;

    status = pthread_create(threadID, NULL, thread_main, arg);

    if (status != 0) {
        printf("wsuCreateThread(): Thread creation failed: %s\n",
               strerror(status));
        return -1;
    }

    return 0;
}
