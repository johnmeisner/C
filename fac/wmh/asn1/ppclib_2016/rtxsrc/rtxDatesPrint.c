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
#include <math.h>
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxDateTime.h"
#include "rtxsrc/rtxPrint.h"

EXTRTMETHOD void rtxPrintGYear (const char* name, const OSNumDateTime* pvalue)
{
   OSUTF8CHAR tmpStr[100];

   rtxGYearToString (pvalue, tmpStr, sizeof (tmpStr));
   printf ("%s  = '%s'\n", name, (const char*)tmpStr);
}

EXTRTMETHOD void rtxPrintGYearMonth (const char* name, const OSNumDateTime* pvalue)
{
   OSUTF8CHAR tmpStr[100];

   rtxGYearMonthToString (pvalue, tmpStr, sizeof (tmpStr));
   printf ("%s  = '%s'\n", name, (const char*)tmpStr);
}

EXTRTMETHOD void rtxPrintGMonth (const char* name, const OSNumDateTime* pvalue)
{
   OSUTF8CHAR tmpStr[100];

   rtxGMonthToString (pvalue, tmpStr, sizeof (tmpStr));
   printf ("%s  = '%s'\n", name, (const char*)tmpStr);
}

EXTRTMETHOD void rtxPrintGMonthDay (const char* name, const OSNumDateTime* pvalue)
{
   OSUTF8CHAR tmpStr[100];

   rtxGMonthDayToString (pvalue, tmpStr, sizeof (tmpStr));
   printf ("%s  = '%s'\n", name, (const char*)tmpStr);
}

EXTRTMETHOD void rtxPrintGDay (const char* name, const OSNumDateTime* pvalue)
{
   OSUTF8CHAR tmpStr[100];

   rtxGDayToString (pvalue, tmpStr, sizeof (tmpStr));
   printf ("%s  = '%s'\n", name, (const char*)tmpStr);
}

