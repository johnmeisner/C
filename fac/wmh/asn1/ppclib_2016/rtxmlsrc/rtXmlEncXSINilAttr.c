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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

static const OSUTF8CHAR XSI_NIL[] = "xsi:nil";
#define XSI_NIL_LEN (sizeof(XSI_NIL)-1)

EXTXMLMETHOD int rtXmlEncXSINilAttr (OSCTXT* pctxt)
{
   int stat;

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
      rtxCtxtTestFlag (pctxt, OSXMLC14N))
      stat = rtXmlEncStartAttrC14N (pctxt);

   stat = rtXmlEncUTF8Attr2
      (pctxt, XSI_NIL, XSI_NIL_LEN, OSUTF8("true"), 4);

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER ) &&
      rtxCtxtTestFlag (pctxt, OSXMLC14N))
      stat = rtXmlEncEndAttrC14N (pctxt);

   rtXmlSetEncXSINilAttr (pctxt, FALSE);

   return (0 != stat) ? LOG_RTERR (pctxt, stat) : 0;
}
