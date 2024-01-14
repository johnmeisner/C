/*
 * Copyright (c) 2014-2018 Objective Systems, Inc.
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
 * @file rtxSysInfo.h
 */
#ifndef _RTXSYSINFO_H_
#define _RTXSYSINFO_H_

#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxExternDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function return the process ID of the currently running process.
 *
 * @return Process ID of currently running process.
 */
EXTERNRT int rtxGetPID ();

#ifndef _WIN32_WCE
/**
 * This function make a duplicate copy of an environment variable.
 * The variable should be used using the standard C RTL free function
 * (note: the OSCRTLFREE macro may be used to abstract the free function).
 *
 * @param name Name of environment variable to duplicate.
 * @return The duplicated environment variable value.
 */
EXTERNRT char* rtxEnvVarDup (const char* name);

/**
 * This function tests if an environment variable is set.
 *
 * @param name Name of environment variable to test.
 * @return True if environmental variable is set; false otherwise.
 */
EXTERNRT OSBOOL rtxEnvVarIsSet (const char* name);

/**
 * This function sets an environment variable to the given value.
 *
 * @param name Name of environment variable to test.
 * @param value Value to which variable should be set.
 * @param overwrite If non-zero, overwrite existing variable with value.
 * @return Status of operation, 0 = success.
 */
EXTERNRT int rtxEnvVarSet (const char* name, const char* value, int overwrite);
#endif

#ifdef __cplusplus
}
#endif

#endif
