/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rpsi.h                                                           */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Header file for rpsi.c                                        */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#ifndef _RPSI_H
#define _RPSI_H

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include "rs.h"

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/
rsResultCodeType createReceiveCommandsThread(void);
rsResultCodeType terminateReceiveCommandsThread(void);

#endif // _RPSI_H

