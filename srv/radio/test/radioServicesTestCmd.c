/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: radioServicesTestCmd.c                                           */
/*  Purpose: Sends test commands and their associated parameters to           */
/*           radioServices                                                    */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-28  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "rs.h"
#include "ipcsock.h"

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
static int   radioServicesSock = -1;
static char *appName;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/**
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0.
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Blocks waiting  for areply via
**          reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    size_t      rcvSize;

    /* Open the socket to communicate with radioServices with if it isn't
     * already opened */
    if (radioServicesSock < 0) {
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("wsuConnectSocket() failed\n");
            return -1;
        }
    }

    /* Send the message to Radio Services */
    if (!wsuSendData(radioServicesSock, RS_RECV_PORT, msg, size)) {
        /* Send failed */
        printf("wsuSendData() failed\n");
        return -1;
    }

    /* Wait for a response */
    while (1) {
        /* Get the reply to the message */
        rcvData.available = FALSE;
        rcvData.fd        = radioServicesSock;
        rcvData.data      = reply;
        rcvData.size      = replySize;

        rcvSize = wsuReceiveData(/* Timeout ignored */0, &rcvData);

        if (rcvSize > 0) {
            /* We received data. In this case, there is no way
             * rcvData.available could have been set to FALSE. */
            break;
        }

        if (rcvSize < 0) {
            /* Receive failed */
            printf("wsuReceiveData() failed\n");
            return -1;
        }

        /* If rcvSize is 0, then continue in the loop and try again. A return
         * value of 0 is something that can occur normally with datagram
         * sockets; wsuConnectSocket() specifies SOCK_DGRAM on its call to
         * wsuSendData(). Also, a return value of 0 does NOT indicate the
         * socket was closed. wsuReceiveData() will also return 0 if errno is
         * set to EINTR upron return upon return from recvfrom(); in this case,
         * you should try again. */
    }

    return rcvSize;
}

/**
** Function:  cmdCertChangeTimeout
** @brief  Send a certificate change timeout command to radioServices
** @param  count -- [input]The number of certificate changes to suppress.
** @return 0 on success, -1 on error
**
** Details: Fills out an rsTestCmdType structure with a certificate change
**          timeout command and a count and sends it to radioServices.
**/
static int cmdCertChangeTimeout(int16_t count)
{
    rsTestCmdType    test;
    rsResultCodeType risRet;
    int32_t          size;

    /* Prepare and send the certificate change timeout command message and its
     * count parameter to radioServices */
    test.cmd     = WSU_NS_TEST_CMD;
    test.testCmd = RS_TEST_CMD_CERT_CHANGE_TIMEOUT;
    test.u.count = count;

    size = communicateWithRadioServices(&test, sizeof(test), &risRet,
                                        sizeof(risRet));

    if ((size < 0) || (risRet != RS_SUCCESS)) {
        printf("%s: Error sending unsolicited certificate change cmd msg to radioServices: %s\n",
               appName, strerror(errno));
        return -1;
    }

    return 0;
}

/**
** Function:  cmdUnsolicitedCertChange
** @brief  Send an unsolicited certificate change command to radioServices
** @return 0 on success, -1 on error
**
** Details: Fills out an rsTestCmdType structure with an unsolicited
**          certificate change command and sends it to radioServices.
**/
static int cmdUnsolicitedCertChange(void)
{
    rsTestCmdType    test;
    rsResultCodeType risRet;
    int32_t          size;

    /* Prepare and send the unsolicited certificate change command message to
     * radioServices */
    test.cmd     = WSU_NS_TEST_CMD;
    test.testCmd = RS_TEST_CMD_UNSOLICITED_CERT_CHANGE;

    size = communicateWithRadioServices(&test, sizeof(test), &risRet,
                                        sizeof(risRet));

    if ((size < 0) || (risRet != RS_SUCCESS)) {
        printf("%s: Error sending unsolicited certificate change cmd msg to radioServices: %s\n",
               appName, strerror(errno));
        return -1;
    }

    return 0;
}

/**
** Function:  cmdIgnoreWSAs
** @brief  Send an ignore WSA command to radioServices
** @param  seconds -- [input]The number of seconds to ignore WSAs
** @return 0 on success, -1 on error
**
** Details: Fills out an rsTestCmdType structure with an ignore WSAs command
**          and a number of seconds and sends it to radioServices.
**/
static int cmdIgnoreWSAs(int16_t seconds)
{
    rsTestCmdType    test;
    rsResultCodeType risRet;
    int32_t          size;

    /* Prepare and send the ignore WSA command message and its number of
     * seconds parameter to radioServices */
    test.cmd       = WSU_NS_TEST_CMD;
    test.testCmd   = RS_TEST_CMD_IGNORE_WSAS;
    test.u.seconds = seconds;

    size = communicateWithRadioServices(&test, sizeof(test), &risRet,
                                        sizeof(risRet));

    if ((size < 0) || (risRet != RS_SUCCESS)) {
        printf("%s: Error sending ignore WSAs cmd msg to radioServices: %s\n",
               appName, strerror(errno));
        return -1;
    }

    return 0;
}

/**
** Function:  cmdIgnoreWSAs
** @brief  Print out the help menu
** @return void
**/
static void helpMenu(void)
{
    printf("\n");
    printf("Enter:\n");
    printf("  t - Simulate certificate change timeout\n");
    printf("  u - Simulate unsolicited certificate change\n");
    printf("  w - Cause system to ignore WSA's for a specified time\n");

    printf("  h - Help (display this menu again)\n");
    printf("  x - Exit\n");
}

/**
** Function:  user_input
** @brief  Gets input from the user
** @return void
**
** Details: Keeps prompting the user for commands. Depending on the command, it
**          may prompt the user for a secondary parameter such as a count or
**          number of seconds. Then it sends the command to radioServices by
**          calling the proper function. It returns when the user enters 'x'
**          for the command.
**/
static void user_input(void)
{
    bool_t  running = TRUE;
    char    cmdbuff[100];
    int8_t  ch;
    int16_t value16;

    printf("radioServices Test Command Application. Enter 'h' for help.\n");

    while (running) {
        /* Read New line character and dump it*/
        printf("-> ");

        if (fgets(cmdbuff, sizeof(cmdbuff), stdin) != NULL) {
            ch = cmdbuff[0];
        }
        else {
            ch = 0;
        }

        switch (ch) {
        case 't':
            printf("\n");
            printf("One timeout will cause radioServices to automatically\n");
            printf("retry the certificate change, and the retry will\n");
            printf("probably succeed. Two timeouts will result in a real\n");
            printf("timeout error\n");
            printf("\n");
            printf("Enter the number of timeouts: ");

            if (fgets(cmdbuff, sizeof(cmdbuff), stdin) != NULL) {
                value16 = atoi(cmdbuff);
            }
            else {
                value16 = 1;
            }

            if (value16 == 0) {
                value16 = 1;
            }

            cmdCertChangeTimeout(value16);
            break;

        case 'u':
            cmdUnsolicitedCertChange();
            break;

        case 'w':
            printf("\n");
            printf("Causes radioServices to ignore WSA's for the\n");
            printf("specified number of seconds. This will cause a\n");
            printf("SERVICE_NOT_AVAILABLE indication after the WSA timeout\n");
            printf("occurs, then a SERVICE_AVAILABLE indication after the\n");
            printf("rest of the time expires. If a user service is\n");
            printf("connected to a provider service, this will cause a\n");
            printf("disconnect followed by a reconnect (with a different\n");
            printf("MAC address).\n");
            printf("\n");
            printf("Enter the number of seconds: ");

            if (fgets(cmdbuff, sizeof(cmdbuff), stdin) != NULL) {
                value16 = atoi(cmdbuff);
            }
            else {
                value16 = 5;
            }

            if (value16 == 0) {
                value16 = 5;
            }

            cmdIgnoreWSAs(value16);
            break;

        case 'h':
            helpMenu();
            break;

        case 'x':
            running = FALSE;
            break;

        default:
            printf("   Enter 'h' for the help menu.\n");
            break;
        }
    }
}

int main(int argc, char **argv)
{
    appName = argv[0];

    /* Get commands from the keyboard */
    user_input();

    /* Close the connection to radioServices resource manager */
    if (radioServicesSock != -1) {
        close(radioServicesSock);
    }

    return 0;
}

