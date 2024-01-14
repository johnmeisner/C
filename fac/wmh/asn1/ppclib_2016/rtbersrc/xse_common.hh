/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/
/**
 * @file xse_common.hh
 */
#ifndef _XSE_COMMON_HH_
#define _XSE_COMMON_HH_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtbersrc/asn1berStream.h"

/***********************************************************************
 *
 *  ENCODE FUNCTIONS
 *
 **********************************************************************/

int berEncStrmIdentifier (OSCTXT* pctxt, OSUINT32 ident);
int berEncStrmIntValue (OSCTXT* pctxt, OSINT32 value);

#endif
