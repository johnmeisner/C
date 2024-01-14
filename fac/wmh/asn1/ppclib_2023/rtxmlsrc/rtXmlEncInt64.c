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

#ifndef RTXMLENCINTFUNC
#define RTXMLENCINTFUNC rtXmlEncInt64
#define RTXMLENCINTVALUEFUNC rtXmlEncInt64Value
#define RTXINTTOCHARSTR rtxInt64ToCharStr
#define OSINTTYPE OSINT64
#endif

/*
For some reason, at least with Visual C++ 6, we sometimes come to this spot
in the compile with RTXMLENCINTFUNC defined, but with some or all of the
pieces that should be defined with it being undefined.  Simply adding a few
#pragma message statements to the #ifndef block above prevents this from
happening, so it appears we have stumbled over a quirk of the Visual C++ 6
compiler.  We are adding the block below to ensure that everything that's
needed is defined.
*/

#ifdef RTXMLENCINTFUNC

#ifndef RTXMLENCINTVALUEFUNC
#define RTXMLENCINTVALUEFUNC rtXmlEncInt64Value
#endif

#ifndef RTXINTTOCHARSTR
#define RTXINTTOCHARSTR rtxInt64ToCharStr
#endif

#ifndef OSINTTYPE
#define OSINTTYPE OSINT64
#endif

#endif

#include "rtXmlEncInt.c"
