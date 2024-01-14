/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#ifndef _RTGETLIBINFO_H_
#define _RTGETLIBINFO_H_

#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/osSysTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function returns the version of the ASN1C run-time.
 *
 * @RETURN       An integer representing the run-time version
 *               (e.g., 684 for v6.8.4).
 */
EXTERNRT int rtxGetLibVersion (void);

/**
 * This function returns a string of information about the run-time
 * that the calling application is using.
 *
 * @return      A string of information about the run-time.
 */
EXTERNRT const char* rtxGetLibInfo (void);

/**
 * This function indicates whether the run-time is limited or unlimited.
 *
 * @return      TRUE if limited, FALSE if unlimited.
 */
EXTERNRT OSBOOL rtxIsLimited (void);

#ifdef __cplusplus
}
#endif

#endif
