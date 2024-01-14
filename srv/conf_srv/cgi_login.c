/*
 *  Filename: cgi_login.c   
 *  Purpose: Utility to handle Webgui logins
 *
 *  Copyright (C) 2020 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author     Comments
 * --------------------------------------------------
 *  2020-11-11  KFRANKEL   Initial release.
 *
 * NOTES:
 *   Replaces functionality of checkpw.sh script used on QNX 5910
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>     // for va_list and friends
#ifndef BUILD_DEVINSP
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#define __USE_GNU
#include <crypt.h>    /* __USE_GNU required for crypt_r */
#endif /* !BUILD_DEVINSP */
#include <errno.h>
#include "libcgipost.h"
#include "libtimefgets.h"
#include "dn_types.h"
           
#define DEBUG_OUTPUT_FILE     "/tmp/xcgi-login-debug.txt"
#define CUSTOMER_OUTPUT_FILE  "/rwflash/client.txt"
#define CLIENT_LOGIN_FILE     "/rwflash/client5.txt"

// UNCOMMENT to enable TIMEFGETS which is a fgets with a timeout -- handles
//   issue with bozohttpd not closing stdin to cgi scripts.  Comment out in
//   the future when/if bozohttpd fixes this issue.
#define WANT_TIMEFGETS

#define CGI_STDIN_TIMEOUT_MSEC 50    /* 50 msecs max waiting on stdin to complete */
#define MAXTRIES 6   /* maximum login tries allowed*/
#define BLOCK_TIME 10   // Time to block attempts (ie., no redirects for logins)
                        // Need to keep track of user also ... ip address

#ifdef BUILD_DEVINSP
#define USERNAME1 "rsuuser"
#define PASSWORD1 "HDFive218"
#endif /* BUILD_DEVINSP */

void client_printf(char * fmt, ...)
{
    va_list l_Arg; 

    // Lead with timestamp
    FILE * fout = fopen(CUSTOMER_OUTPUT_FILE, "a");
    time_t xx = time(0);
    struct tm * x = localtime(&xx);
    fprintf(fout, "%d%02d%02d-%02d%02d%02d: ", 1900+x->tm_year,x->tm_mon,x->tm_mday,x->tm_hour,x->tm_min,x->tm_sec);

    va_start(l_Arg,fmt);         // Last param BEFORE the "..."
    vfprintf(fout, fmt, l_Arg);  // Use the variable list all at once
    va_end(l_Arg);               // Close the va list
 
    fclose(fout);

}

#ifdef DEBUG_OUTPUT_FILE
void debug_printf(char * fmt, ...)
{
    //Declare a va_list macro and initialize it with va_start
    va_list l_Arg;

    // Lead with timestamp
    FILE * fout = fopen(DEBUG_OUTPUT_FILE, "a");
    time_t xx = time(0);
    struct tm * x = localtime(&xx);
    fprintf(fout, "%d%02d%02d-%02d%02d%02d: ", 1900+x->tm_year,x->tm_mon,x->tm_mday,x->tm_hour,x->tm_min,x->tm_sec);

    va_start(l_Arg,fmt);         // Last param BEFORE the "..."
    vfprintf(fout, fmt, l_Arg);  // Use the variable list all at once
    va_end(l_Arg);               // Close the va list

    fclose(fout);
}
#endif

#ifndef BUILD_DEVINSP
// checkpassword returns a boolean value (0 or 1) where 1 (True) indicates
// the password is correct for the input username
static int checkpassword(char *user, char *passwd)
{
    struct passwd    *pentry;
    struct spwd      *sentry;
    struct crypt_data data;

    memset(&data, 0, sizeof(data));
    
    if (!strcmp(user, "root")) {
        /* invalid user - do not allow root log in */
        return 0;
    }

    if (NULL == (pentry = getpwnam(user))) {
        /* invalid user */
        return 0;
    }

    if (strcmp(pentry->pw_passwd, "x")) {
        /* entry has a password in /etc/passwd file*/
        return !(strcmp(pentry->pw_passwd, crypt_r(passwd, pentry->pw_passwd, &data)));
    }
    
    /* need password from /etc/shadow */
    if (NULL == (sentry = getspnam(user))) {
        /* invalid user */
        return 0;
    }

    return !(strcmp(sentry->sp_pwdp, crypt_r(passwd, sentry->sp_pwdp, &data)));
}
#endif /* !BUILD_DEVINSP */

int main()
{
    // Create cgi-post-parse structures
    char * wanted_fields[3] = { "username", "password" };
    char username[20] = {0}, password[50] = {0};
    char * value_dest[2] = { username, password };
    char cmd[200];
    size_t max_value_sizes[2] = { sizeof(username), sizeof(password) };
    bool_t login_valid = FALSE;
    int32_t rc;
    int32_t fails;
    int n;
    int flag = 0;

    struct client {
        int ip;
        char username[20];
        char password[20];
        int fails;
        int retries;
    };

    struct client num = { 0, "", "", 0, 0 };

    FILE * fptr = NULL;
    FILE * foutptr = NULL;

    /* Do NOT fail login if ACCESS CONTROL fails. */

    /* If first read fails then just means first time */
    if ((fptr = fopen(CLIENT_LOGIN_FILE, "rb")) == NULL) {    
        debug_printf("Error! opening file for read: errno(%s)\n",strerror(errno));
    } else {
        rewind(fptr);
        //read from file
        fread(&num, sizeof(num), 1, fptr);   // get num instance of client struct
        fails = num.fails;     // fails is the local variable read from log initially
        //debug_printf("fails: %d\n", fails);
        fclose(fptr);
    }

    /* Regardless of if read, open file to write now. */
    if ((fptr = fopen(CLIENT_LOGIN_FILE, "wb")) == NULL) {    
        debug_printf("Error! opening file for write: errno(%s)\n",strerror(errno));
    }

    #ifdef DEBUG_OUTPUT_FILE
    debug_printf("cgi-login started.\n");
    #endif

    // Process the POST from stdin
#ifdef WANT_TIMEFGETS
    char post_string[1000];
    int bytes_read;
    #ifdef DEBUG_OUTPUT_FILE
    debug_printf("cgi-login going to call timefgets...\n");
    #endif
    bytes_read = timefgets(post_string, sizeof(post_string), stdin, CGI_STDIN_TIMEOUT_MSEC);
#ifdef DEBUG_OUTPUT_FILE
    debug_printf("Read %d bytes with timefgets: \"%s\"\n", bytes_read, post_string);
#else
    (void) bytes_read;
#endif

    // Process user login post prior to logging username / password
    if (process_post_string(post_string, 2, wanted_fields, value_dest, max_value_sizes) == 2)
#else
    if (receive_and_process_post(stdin, 2, wanted_fields, value_dest, max_value_sizes) == 2)
#endif
    {
        #ifdef DEBUG_OUTPUT_FILE
        #endif
        // Test if we got a CORRECT match
#ifdef BUILD_DEVINSP
        if (strcmp(username, USERNAME1) == 0 && strcmp(password, PASSWORD1) == 0)
#else
        if (checkpassword(username, password))
#endif /* BUILD_DEVINSP */
        {
            login_valid = TRUE;
        }
    }

    #ifdef DEBUG_OUTPUT_FILE
    // debug_printf("cgi-login processed munch_post, login_valid = %s\n", login_valid == TRUE ? "TRUE" : "FALSE");
    debug_printf("cgi-login processed, login_valid = %s\n", login_valid == TRUE ? "TRUE" : "FALSE");
    client_printf("cgi-login processed, login_valid = %s\n", login_valid == TRUE ? "TRUE" : "FALSE");
    #endif

    // If credentials valid, log in user and redirect to main page
    if (login_valid)
    {
        // print results of client file ... one client login per line???
        debug_printf("ip: %d\tusername: %s\tpw: %s\tretires: %d\tfails: %d \n", num.ip, num.username, num.password, num.retries, num.fails);
    
        // Get remote IP from environment var and log in client
        sprintf(cmd, "/usr/bin/conf_agent LOGIN %s", getenv("REMOTE_ADDR"));
        rc = system(cmd);

        #ifdef DEBUG_OUTPUT_FILE
            debug_printf("The 'conf_agent LOGIN %s' system() call returned %d (want 0)\n", getenv("REMOTE_ADDR"), rc);
        #else
            (void) rc;      // Avoids a warning about unused rc (which it is, without #DEBUG_OUTPUT_FILE)
        #endif

        // Show user main page
        printf("Content-type: text/html\r\n\r\n");

        #ifdef DEBUG_OUTPUT_FILE
        //  debug_printf("Login succeeded, redirecting user to rsu_main\n");
        #endif
        printf("<meta http-equiv=\"refresh\" content=\"0; URL='/cgi-bin/rsu_main' \" />\r\n");

        #ifdef DEBUG_OUTPUT_FILE
            debug_printf("Login succeeded, system rc=%d\n", rc);
        #endif

    // Response to INCORRECT login
    } else {

        // add ip address / username for current failed login post
        memcpy(num.username, username, strlen(username));

        // increment fails 
        ++num.fails;

        // update file
        // make sure file pointer is at beginning
        if(NULL != fptr) {
            rewind(fptr);
            fwrite(&num, sizeof(num), 1, fptr);
            fflush(fptr);
        }
        debug_printf("ip: %d\tusername: %s\tpw: %s\tretires: %d\tfails: %d \n", num.ip, num.username, num.password, num.retries, num.fails);

        // only allow retry if fails less than maxtriess
        if (num.fails > MAXTRIES) {

            debug_printf("Authentication Failure\n");   //jm
            debug_printf("Do not redirect to retry\n"); 

            // do not allow retry
            printf("Content-type: text/html\r\n\r\n"); 
            printf("<h2>Max retries exceeded</h2>\r\n");
            printf("<h2>See Administrator</h2>\r\n");

        } else {

            debug_printf("fails: %d\n", fails);

            printf("Content-type: text/html\r\n\r\n");
            printf("<meta http-equiv=\"refresh\" content=\"0; URL='/index_retry.html' \" />\r\n");
            printf("<h2>Header of some sort</h2>\r\n");
            printf("<li>Line Item<br/>\r\n");
            printf("<li>Line Item 2<br/>\r\n");

        }

        #ifdef DEBUG_OUTPUT_FILE
        debug_printf("Login failed, redirected user to retry\n");
        #endif
    }

    if(NULL != fptr) fclose(fptr);

    return 0;
}
