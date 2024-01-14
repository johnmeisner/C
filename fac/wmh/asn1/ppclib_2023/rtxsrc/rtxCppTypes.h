/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
 * @file rtxCppTypes.h
 * C++ common type and class definitions.
 */
#ifndef _RTXCPPTYPES_H_
#define _RTXCPPTYPES_H_

#include "rtxsrc/rtxCppAnyElement.h"
#include "rtxsrc/rtxCppDList.h"
#include "rtxsrc/rtxCppDynOctStr.h"
#include "rtxsrc/rtxCppXmlString.h"

#ifdef HAS_STL
#include "rtxsrc/rtxCppXmlSTLString.h"
#include "rtxsrc/rtxCppXmlSTLStringList.h"
#ifdef _MSC_VER
// Set warning level to 1 to suppress warnings in STL vector header file..
#pragma warning( push, 1 )
#endif /* _MSC_VER */

#include <vector>

#ifdef _MSC_VER
#pragma warning( pop )
#endif /* _MSC_VER */

#endif /* HAS_STL */

#endif /* _RTXCPPTYPES_H_ */
