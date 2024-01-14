/*
 *  Filename: conf_agent.c
 *  Purpose: Configuration agent interface to web GUI
 *          conf_manager <---> conf_agent <---> Web GUI
 *
 *  Copyright (C) 2021 DENSO International America, Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <time.h>
#include "dn_types.h"
#include "conf_manager.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN conf_agent_main
#else
#define MAIN main
#endif

/* Only write debug to file. Do NOT send to STDOUT or STDERR. Will clobber WEB GUI. */
STATIC int32_t debug_output = 0;             /* Debug OFF by default */

/*
 * Debug print
 */
#include <stdarg.h>  // for va_list
#define DPRINT(fmt, ...)   mylog("%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DPRINT0(fmt, ...)  mylog("%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
void mylog(char_t * fmt, ...)
{
    static char_t buffer[4000];
    static va_list argptr;
    static int32_t fd;

    if (debug_output) {
        va_start(argptr, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, argptr);
        va_end(argptr);
#if defined(MY_UNIT_TEST)
        fd = open("./zc_agt_log.txt", O_CREAT|O_WRONLY|O_APPEND,777);
#else
        fd = open("/tmp/zc_agt_log.txt", O_CREAT|O_WRONLY|O_APPEND, 777);
#endif
        if (fd >= 0) {
            if(-1 == write(fd, buffer, strlen(buffer))) { 
                /* We care alot. Silence of the Warnings. */
            }
            close(fd);
        }
    }
}
/*On fail will return NULL, otherwise pointer to s1. */
char_t * i2v_strncat(char_t * s1, char_t * s2, uint32_t size)
{

  if((NULL == s1) || (NULL == s2) || (0 == size)) {
      return NULL;
  }
  /* Account for null strncat will add. */
  if((size-1) < (strlen(s1) + strlen(s2))) {
      return NULL;
  }
  return strncat(s1,s2,size);
}
/*
 * config_send_msg()
 * Send Config message to conf_manager
 */
int32_t config_send_msg(char_t *conf_msg, char_t *resp_msg, int32_t len, int32_t timeout)
{
    mqd_t mq_config_manager;
    mqd_t mq_config_client = -1;
    int32_t iRet = 0;
    char_t client_queue_name [MAX_NAME_SIZE];
    mq_hdr_t *mq_hdr;
    struct timespec tm;

    /* Error check */
    if ( (conf_msg == NULL) || (resp_msg == NULL) ){
      return 1;
    }
    
    mq_hdr = (mq_hdr_t *)&conf_msg[0];
    mq_hdr->proc_id = getpid();    /* Proc id */
    mq_hdr->thrd_id = time(0L) & 0x1FFF;    /* For Linux, in lieu of QNX's Thread id */

    /* Open config manager MQ */
    if ((mq_config_manager = mq_open (QUEUE_NAME, O_WRONLY)) == -1) {
        DPRINT("conf_agent: Client: mq_open (server)\n");
        iRet = 1;
        goto conf_send_msg_exit;
    }

    /* Create client MQ, where conf manager send back response */
    sprintf(client_queue_name, "/%d%d", mq_hdr->proc_id, mq_hdr->thrd_id);
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MSG_BUFFER_SIZE;
    attr.mq_curmsgs = 0;
    if ((mq_config_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        DPRINT("conf_agent: Client: mq_open (client)\n");
        return 1;
    }

    /* send message to server */
    if (mq_send (mq_config_manager, (char *)conf_msg, len, 0) == -1) {
        DPRINT("conf_agent: Client: Not able to send message to server.\n");
        iRet = 1;
    }
    
    /*
     * Get response
     * Note: conf manager will response back to client MQ at pID&tID (process ID followed by thread ID
     */
    DPRINT("Waiting for response from : %s\n", QUEUE_NAME);
    clock_gettime(CLOCK_REALTIME, &tm);
    tm.tv_sec += timeout;         /* Set timeout */
    if (mq_timedreceive (mq_config_client, (char *)resp_msg, MSG_BUFFER_SIZE, NULL, &tm) == -1) {
        DPRINT("conf_agent: Client: mq_receive.\n");
        iRet=1;
    }

    DPRINT("Got a response from : %s\n", QUEUE_NAME);

conf_send_msg_exit:

    mq_close(mq_config_client);
    mq_unlink(client_queue_name);

    return iRet;
}

void print_usage(void)
{
    DPRINT("Usage: conf_agent OPERATION [PARAMS]\n");
    DPRINT("    OPERATION (w params):\n");
    DPRINT("        - READ CONFIG_GROUP CONFIG_ITEM\n");
    DPRINT("        - WRITE CONFIG_GROUP CONFIG_ITEM CONFIG_VALUE\n");
    DPRINT("        - UPDATE CONFIG_GROUP CONFIG_ITEM\n");
    DPRINT("        - UPDATE_ALL\n");
    DPRINT("        - LOGIN IP_ADDRESS\n");
    DPRINT("        - LOGOUT IP_ADDRESS\n");
    DPRINT("        - SESSION_CHK SESSION_ID\n");
    DPRINT("        - FACTORY_DEFAULTS_RESET CONFIRM\n");
    DPRINT("        - FACTORY_DEFAULTS_RESET_BUT_PRESERVE_NETWORKING CONFIRM\n");
    DPRINT("    Example:\n");
    DPRINT("        conf_agent READ I2V I2VSCSAppEnable\n");
}

int32_t MAIN(int32_t argc, char_t **argv)
{
    int32_t iRet = 1;
    mq_hdr_t conf_hdr;
    char_t msg_to_conf_mgr[MSG_BUFFER_SIZE];
    char_t *command = (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)];
    char_t msg_from_conf_mgr[MSG_BUFFER_SIZE];
    int32_t argv_offset = 0;
    int32_t cmd_timeout = 0;
    int32_t i = 0;

    /* Clear static */
    debug_output = 0;

    /* Clear memory */
    memset(&conf_hdr, 0, sizeof(conf_hdr));
    memset(msg_to_conf_mgr, 0, sizeof(msg_to_conf_mgr));
    memset(msg_from_conf_mgr, 0, sizeof(msg_from_conf_mgr));

    /* Check for too few or too many arguments. */
    if ((argc < CONF_AGENT_ARGC_MIN) || (CONF_AGENT_ARGC_MAX < argc)) {
        /* Unitl we parse "-d" there is no debug log and only printf is available. */
        printf("conf_agent: Error: Wrong number of arguements = %d. Must be between %d and %d in number. Please try again.\n",argc, CONF_AGENT_ARGC_MIN,CONF_AGENT_ARGC_MAX);
        return (1);
    }
    /* Error check argv to make sure none of them exceed MAX_TOKEN_SIZE. */
    for(i=1;i<argc;i++){
        if(MAX_TOKEN_SIZE < strlen(argv[i])) {
            printf("conf_agent: argv[%d]=[%s]\n",i,argv[i]); 
            printf("conf_agent: Error: Each arguement length can't exceed %d characters. Please try again.\n", MAX_TOKEN_SIZE);
            return (1);
        }
        if(0 == strlen(argv[i])) {
            printf("conf_agent: argv[%d]=[%s]\n",i,argv[i]);
            printf("conf_agent: Error: Each arguement length can't be 0 characters. Please try again.\n");
            return (1);
        }
    }

    /* Flip on debug. Now DPRINT will work. */
    if (!strcmp("-d", argv[1])) {
        debug_output = 1;
        ++argv_offset;
    }

    /* 
     *  Check TYPE OF OPERATION - 
     *      CONF_READ,
     *      CONF_WRITE,
     *      CONF_LOGIN,
     *      CONF_LOGOUT,
     *      CONF_SESSION_CHK,
     */
    conf_hdr.msg_attr.msg_op = CONF_NONE;     /* Default to NONE */
    if (0 == (strcmp("READ", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_READ;
    }
    if (0 == (strcmp("WRITE", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_WRITE;
    }
    if (0 == (strcmp("UPDATE", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_UPDATE;
    }
    if (0 == (strcmp("UPDATE_ALL", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_UPDATE_ALL;
    }
    if (0 == (strcmp("LOGIN", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_LOGIN;
    }
    if (0 == (strcmp("LOGOUT", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_LOGOUT;
    }
    if (0 == (strcmp("SESSION_CHK", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CONF_SESSION_CHK;
    }
    if (0 == (strcmp("FACTORY_DEFAULTS_RESET", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = FACTORY_RESET;
    }
    if (0 == (strcmp("FACTORY_DEFAULTS_RESET_BUT_PRESERVE_NETWORKING", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = FACTORY_RESET_BUT_PRESRV_NET;
    }
    if (0 == (strcmp("CUSTOMER_BACKUP", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CUSTOMER_BACKUP;
    }
    if (0 == (strcmp("CUSTOMER_RESET", argv[1+argv_offset])) ) {
        conf_hdr.msg_attr.msg_op = CUSTOMER_RESET;
    }
    
    /* 
     *  Only the WRITE operation can blow MSG_BUFFER_SIZE.
     *  The others dont have enough tokens to allow.
     *  But, we are safe against something smashing our strings.
     */
    switch (conf_hdr.msg_attr.msg_op){
        case CONF_READ:
            DPRINT0("CONF_READ\n");
            if ((argc < 4) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 5) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            /* Process config group param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            /* Process config item param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[3+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 2;    // set read timeout to 2 secs
            DPRINT("READ%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;
        case CONF_WRITE:
            DPRINT0("CONF_WRITE\n");
            
            if ((argc < 5) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 6) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            /* Process config group param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }

            /* Process config item param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[3+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            /* Process config value param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[4+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 15;    // set write timeout to 15 secs
            DPRINT("WRITE%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;
        case CONF_UPDATE:
            DPRINT0("CONF_UPDATE\n");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }

            /* Process config group param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }

            /* Process config item param */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[3+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 2;    // set read timeout to 2 secs
            DPRINT("UPDATE%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;

        case CONF_UPDATE_ALL:
            DPRINT0("CONF_UPDATE_ALL\n");
            cmd_timeout = 30;    // set read timeout to 30 secs
            DPRINT("UPDATE_ALL%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;

        case CONF_LOGIN:
            DPRINT0("CONF_LOGIN\n");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            /* Login in with IP as identifier 
             * script to pass $REMOTE_ADDR
             */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 2;    // set read timeout to 2 secs
            DPRINT("LOGIN%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;
        
        case CONF_LOGOUT:
            DPRINT0("CONF_LOGOUT\n");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            /* Login in with IP as identifier 
             * script to pass $REMOTE_ADDR
             */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 2;    // set read timeout to 2 secs
            DPRINT("LOGOUT%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;
        
        case CONF_SESSION_CHK:
            DPRINT0("CONF_SESSION_CHK\n");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!\n");
                print_usage();
                return (1);
            }
            /* return current session ID which the current user IP 
             * should match IP, else goto login page
             */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            DPRINT("SESSION_CHK%s\n", (char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            cmd_timeout = 2;    // set read timeout to 2 secs
            break;

        case FACTORY_RESET:
        case FACTORY_RESET_BUT_PRESRV_NET:
            DPRINT("FACTORY_RESET%s\n",
                    (conf_hdr.msg_attr.msg_op == FACTORY_RESET) ? "" : "_BUT_PRESERVE_NET");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            /* Add confirmation */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 30;    // set read timeout to 30 secs

            if (conf_hdr.msg_attr.msg_op == FACTORY_RESET) {
                DPRINT("FACTORY_RESET%s\n",(char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            } else {
                DPRINT("FACTORY_RESET_BUT_PRESERVE_NET%s\n",(char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            }
            break;

        case CUSTOMER_BACKUP:
            DPRINT("CUSTOMER_BACKUP");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            /* Add confirmation */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 30;    // set read timeout to 30 secs

            DPRINT("CUSTOMER_BACKUP%s\n",(char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;
            
        case CUSTOMER_RESET:
            DPRINT("CUSTOMER_RESET");
            if ((argc < 3) && (0 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            if ((argc < 4) && (1 == debug_output)){
                DPRINT("usage error!argc=%d,debug=%d\n",argc,debug_output);
                print_usage();
                return (1);
            }
            /* Add confirmation */
            if(NULL == i2v_strncat(command, "|", MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            if(NULL == i2v_strncat(command, argv[2+argv_offset], MSG_BUFFER_SIZE)) {
                DPRINT("Error: Total arguement list can't exceed %d characters. Please try again.\n", MSG_BUFFER_SIZE);
                return (1);
            }
            cmd_timeout = 30;    // set read timeout to 30 secs

            DPRINT("CUSTOMER_RESET%s\n",(char_t *)&msg_to_conf_mgr[sizeof(mq_hdr_t)]);
            break;


        default:
            /* Error - print usage here */
            DPRINT("USAGE error!  Unknown param %s\n", argv[1+argv_offset]);
            print_usage();
            return (1);
            break;
    }
    
    /* 
     *  Build message
     *  Msg format: 
     *      Read : HDR | "READ"  | CONF_GROUP | CONF_ITEM
     *      Write: HDR | "WRITE" | CONF_GROUP | CONF_ITEM | CONF_VAL
     */
    conf_hdr.msg_attr.msg_type = MSG_REQUEST;
    memcpy(&msg_to_conf_mgr, &conf_hdr, sizeof(mq_hdr_t));

    /* 
     * Send a msg to conf manager
     * Always get a response - 30 sec timeout for a write and 2 secs for read
     */
    iRet = config_send_msg((char_t *)msg_to_conf_mgr, (char_t *)msg_from_conf_mgr, MSG_BUFFER_SIZE, cmd_timeout);
    /* https does not like zero length resp msg. */
    if(0 == strnlen((char_t *)&msg_from_conf_mgr[sizeof(mq_hdr_t)], MSG_BUFFER_SIZE)) {
        msg_from_conf_mgr[sizeof(mq_hdr_t)    ] = ' ';
        msg_from_conf_mgr[sizeof(mq_hdr_t) + 1] = '\0';
    }
    /* 
     * process reponse
     */
    DPRINT("Message from conf_manager: %s\n", (char_t *)&msg_from_conf_mgr[sizeof(mq_hdr_t)] );

    /* Output result - to be picked up from scripts that generate webpage. DO NOT ALTER. */
    printf("%s\n", (char_t *)&msg_from_conf_mgr[sizeof(mq_hdr_t)] );
//TODO Always return 1? Guessing because bozo dont like it.
    return (iRet);
}

