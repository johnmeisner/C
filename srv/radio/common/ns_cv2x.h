/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ns_cv2x.h                                                        */
/*  Purpose: C-V2X interface module header file                               */
/*           Low-level interface to the C-V2X radio via the Autotalks         */
/*           SECTON SDK.                                                      */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.                       */
/*----------------------------------------------------------------------------*/
#ifndef _NS_CV2X_H
#define _NS_CV2X_H
#include "dn_types.h"
#include "ris_struct.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
** constant:  MAX_SPS_FLOWS
** @brief  Maximum number of SPS flows supported
**----------------------------------------------------------------------------*/
#define MAX_SPS_FLOWS 2


/* Prototypes of ns_cv2x_at.c functions.  These now start with "rs_" to
 * diffenciate them from the many functions in the Autotalks SECTON SDK API
 * which all start with "cv2x_".
 */
int rs_cv2x_module_init(void);
int rs_cv2x_module_term(void);
int rs_cv2x_init(void);
int rs_cv2x_init_tx(uint32_t serviceId, uint32_t reservedBufLen, int32_t peakTxPower);
int rs_cv2x_tx(uint32_t serviceId, uint8_t *data_buf, int32_t data_len, uint32_t psid);
int rs_cv2x_rx(uint8_t *cv2x_rx_buffer, uint32_t maxLen, uint32_t rx_timeout_usec, uint32_t * ppp);
int rs_cv2x_term_tx(uint32_t serviceId);
int rs_cv2x_term(void);
int rs_cv2x_get_radio_tallies(uint8_t radioNum, DeviceTallyType *tallies);
int32_t ns_get_cv2x_status(rskStatusType * rskStatusCopy);
#endif // _NS_CV2X_H

