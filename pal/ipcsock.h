/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: ipcsock.h                                                    */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Library of IPC messaging functions                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/



#ifndef _IPCSOCK_H_
#define _IPCSOCK_H_ 1

#include <pthread.h>
#include <sys/types.h>
#include "dn_types.h"


/**
 * @brief Port numbers of the various services
 * @note  TPS_RECV_GPSDATA and TPS_RECV_DATAREQ are the same
 *          by design but could be renamed into one.
 */

#define V2XSP_PORTS_START      40001

#define TPS_RECV_GPSDATA_PORT  V2XSP_PORTS_START
#define EAS_RECV_XYZDATA_PORT  V2XSP_PORTS_START + 1
#define WSU_SERVICES_PORT      V2XSP_PORTS_START + 2
#define VIS_RECV_XYZDATA_PORT  V2XSP_PORTS_START + 3
#define CAN_RECV_XYZDATA_PORT  V2XSP_PORTS_START + 4
#define TPS_RECV_DATAREQ_PORT  TPS_RECV_GPSDATA_PORT

#define SR_CONTROL_PORT        V2XSP_PORTS_START + 5

#define SR_RECV_TPS_PORT       V2XSP_PORTS_START + 6
#define SR_RECV_VIS_PORT       V2XSP_PORTS_START + 7
#define SR_RECV_RIS0_PORT      V2XSP_PORTS_START + 8
#define SR_RECV_RTKNAV_PORT    V2XSP_PORTS_START + 9
#define SR_RECV_RIS1_PORT      V2XSP_PORTS_START + 10
#define SR_RECV_RIS2_PORT      V2XSP_PORTS_START + 11
#define SR_RECV_EUWMH_PORT     V2XSP_PORTS_START + 12
#define SR_RECV_BSMPUDP_PORT   V2XSP_PORTS_START + 13       // TODO: Remove "UDP" as redundant as everything is UDP now
#define SR_RECV_GNEU_PORT      V2XSP_PORTS_START + 14
#define SR_RECV_MSGPS_PORT     V2XSP_PORTS_START + 15
#define SR_RECV_TPSRAW_PORT    V2XSP_PORTS_START + 16

#define TPS_RECV_SR_PORT       V2XSP_PORTS_START + 17
#define VIS_RECV_SR_PORT       V2XSP_PORTS_START + 18
#define RIS0_RECV_SR_PORT      V2XSP_PORTS_START + 19
#define RTKNAV_RECV_SR_PORT    V2XSP_PORTS_START + 20
#define RIS1_RECV_SR_PORT      V2XSP_PORTS_START + 21
#define RIS2_RECV_SR_PORT      V2XSP_PORTS_START + 22
#define EUWMH_RECV_SR_PORT     V2XSP_PORTS_START + 23
#define BSMPUDP_RECV_SR_PORT   V2XSP_PORTS_START + 24
#define GNEU_RECV_SR_PORT      V2XSP_PORTS_START + 25
#define MSGPS_RECV_SR_PORT     V2XSP_PORTS_START + 26
#define TPSRAW_RECV_SR_PORT    V2XSP_PORTS_START + 27

#define CMM_RECV_PORT		   V2XSP_PORTS_START + 28
#define PRM_RECV_PORT          V2XSP_PORTS_START + 29
#define TAF_RECV_PORT          V2XSP_PORTS_START + 30
#define SAM_RECV_PORT          V2XSP_PORTS_START + 31
#define DLH_RECV_PORT          V2XSP_PORTS_START + 32
#define DLH_LOG_RECV_PORT      V2XSP_PORTS_START + 33
#define OMM_RECV_PORT          V2XSP_PORTS_START + 34
#define TCF_RECV_PORT          V2XSP_PORTS_START + 35
#define WDT_RECV_PORT          V2XSP_PORTS_START + 36
#define DGM_RECV_PORT          V2XSP_PORTS_START + 37
#define RS_RECV_PORT           V2XSP_PORTS_START + 38 // For messages to Radio Services
/* The following allow up to MAX_APPS (currently 8) instances of RIS to listen
 * for incoming indications. If MAX_APPS ever changes, this section will also
 * also need to be changed. */
#define RIS_RECV_IND_PORT1     V2XSP_PORTS_START + 39
#define RIS_RECV_IND_PORT2     V2XSP_PORTS_START + 40
#define RIS_RECV_IND_PORT3     V2XSP_PORTS_START + 41
#define RIS_RECV_IND_PORT4     V2XSP_PORTS_START + 42
#define RIS_RECV_IND_PORT5     V2XSP_PORTS_START + 43
#define RIS_RECV_IND_PORT6     V2XSP_PORTS_START + 44
#define RIS_RECV_IND_PORT7     V2XSP_PORTS_START + 45
#define RIS_RECV_IND_PORT8     V2XSP_PORTS_START + 46


/* Typedefs */
 typedef struct {
 	bool_t  available;
 	int32_t	fd;
 	void	*data;
 	size_t	size;
 	int32_t sender_port;    // Added for QNX SOCK_DGRAM port  20150527
} RcvDataType;


/* Signatures of the IPC functions */
int32_t wsuCreateSockServer(uint16_t port);
int32_t wsuConnectSocket (uint16_t ignored_unused_port);
bool_t wsuSendData (int32_t sock_fd, uint16_t port, void *data, size_t size);
int32_t wsuReceiveData (uint32_t ignored_unused_timeout, RcvDataType *rcv_data);


/* Signatures of Misc functions that were stuck in here because nowhere else was better */
int wsuCreateThread(void *(*thread_main) (void *), void *arg, pthread_t *threadId);


#endif
