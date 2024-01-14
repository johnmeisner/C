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

//////////////////////////////////////////////////////////////////////
//
// ASN1CGeneralizedTime
//
// ISO 8601 time and date for ITU-T X.680.
//
// Author Artem Bolgar.
// version 3.00   03 Sep, 2004 (refactored to use ASN1TGeneralizedTime)
//
// History:
// version 2.17   11 Nov, 2003 (sscanf parameters changed to int from short;
//                              secFraction now may contain more than one
//                              digit)
// version 2.15    4 Sep, 2003 (rtxsrc/wceAddon.h include added)
// version 2.14   31 Jul, 2003 (leap year bug fixed in addDays)
// version 2.13   11 Jul, 2003 (2038 year problem fixed + some other fixes)
// version 2.12   03 Aug, 2002 (copy ctor, some fixes)
// version 1.11   3 Aug, 2002 (copy ctor)
// version 1.08   5 Jun, 2002 (some warnings on Solaris were eliminated)
// version 1.07   5 Jan, 2002
//

#include <stdio.h>
#include <stdlib.h>
#include "rtsrc/ASN1CGeneralizedTime.h"
#include "rtxsrc/rtxCtype.h"
#include "rtsrc/asn1intl.h"

#if defined(_WIN32_WCE)
#include "rtxsrc/wceAddon.h"
#endif

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime (
   OSRTMessageBufferIF& msgBuf, char*& buf, int bufSize, OSBOOL useDerRules) :
   ASN1CTime(msgBuf, buf, bufSize, useDerRules),
   timeObj (buf, useDerRules)
{
}

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime (OSRTMessageBufferIF& msgBuf,
   ASN1GeneralizedTime& buf, OSBOOL useDerRules) :
   ASN1CTime(msgBuf, buf, useDerRules),
   timeObj (buf, useDerRules)
{
}

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime(OSRTContext& ctxt, char*& buf,
   int bufSize, OSBOOL useDerRules):
   ASN1CTime(ctxt, buf, bufSize, useDerRules),
   timeObj (buf, useDerRules)
{
}

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime (
   OSRTContext& ctxt, ASN1GeneralizedTime& buf, OSBOOL useDerRules):
   ASN1CTime(ctxt, buf, useDerRules),
   timeObj (buf, useDerRules)
{
}

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime (
   char*& buf, int bufSize, OSBOOL useDerRules):
   ASN1CTime(buf, bufSize, useDerRules),
   timeObj (buf, useDerRules)
{
}

EXTRTMETHOD ASN1CGeneralizedTime::ASN1CGeneralizedTime(ASN1GeneralizedTime& buf,
                                           OSBOOL useDerRules) :
   ASN1CTime(buf, useDerRules),
   timeObj (buf, useDerRules)
{
}

/*
* Gets century part of the year (first 2 digits of the year).
* Returns: first 2 digits of the year - if success;
*          error code - if parsed value is invalid.
*/
EXTRTMETHOD int ASN1CGeneralizedTime::getCentury() {
   if(!parsed)
      timeObj.parseString(timeStr);
   return timeObj.getCentury();
}

/*
* Sets century part of the year (first 2 digits).
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CGeneralizedTime::setCentury(short century) {
   if(!parsed)
      timeObj.parseString(timeStr);
   int rv = timeObj.setCentury (century);
   compileString();
   return rv;
}

/*
* Converts time_t to time string.
* Parameter 'diffTime' == TRUE means the difference between local time
*   and UTC will be calculated; in other case only local time
*   will be stored.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CGeneralizedTime::setTime(time_t time, OSBOOL diffTime) {
   int rv = timeObj.setTime (time, diffTime);
   parsed = TRUE;
   compileString();
   return rv;
}

/*
* Compiles new time string accoring X.680 (clause 41) and ISO 8601.
* Returns 0, if succeed, or error code, if error.
*/
EXTRTMETHOD int ASN1CGeneralizedTime::compileString() {
   checkCapacity();
   return timeObj.compileString (timeStr, strSize);
}

