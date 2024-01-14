/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: type2str.h                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Prototypes of the functions defined in type2str.c             */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#ifndef _TYPE2STR_H
#define _TYPE2STR_H

#include <stdio.h>
#include "ris_struct.h"

char *cfgIdType2Str(cfgIdType cfgId);
char *rsResultCodeType2Str(rsResultCodeType result);

#endif // _TYPE2STR_H

