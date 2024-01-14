/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlErrCodes.h"
#include "rtxsrc/rtxError.h"

#ifndef _COMPACT

/* Error status text */
static const char* g_status_text[] = {
    "General error", /* ?? */
    "Invalid symbol '%s' detected in XML stream",
    "Start/end XML tags mismatch: expected '%s', parsed '%s'",
    "Duplicate attribute found",
    "Bad character reference '%s;' found",
    "Invalid data mode",
    "Unexpected end-of-file (document) found",
    "Current tag is not matched to specified one",
    "Required element '%s' missed",
    "One or more required element missed",
    "minOccurs is not reached",
    "Unexpected start tag <%s> found",
    "Unexpected end tag </%s> found",
    "Unexpected identifier encountered:\n"
       "   expected = '%s':%s,\n   parsed = '%s':%s",
    "Unknown xsi:type '%s':%s",
    "Namespace URI for prefix '%s' not found",
    "Key not found in '%s', fields: %s %s %s %s",
    "Duplicate key found in '%s', fields: %s %s %s %s",
    "Field #%s absent in some key of '%s'",
    "Key has duplicate field #%s in '%s'",
    "Element is not empty, as is required."
} ;

EXTXMLMETHOD void rtErrXmlInit ()
{
   rtxErrAddErrorTableEntry (g_status_text, XML_E_BASE, (XML_E_BASE-99));
}
#else
EXTXMLMETHOD void rtErrXmlInit () {}
#endif
