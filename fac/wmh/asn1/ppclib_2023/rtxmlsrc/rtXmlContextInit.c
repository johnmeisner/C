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

#include "rtxmlsrc/osrtxml.h"
#ifdef ASN1RT
#include "rtsrc/rtContext.h"
#endif
#include "rtxsrc/rtxContext.hh"

/*
 * This function initializes a context using a run-time key.  This form
 * is required for evaluation and limited distribution software.  The
 * compiler will generate a macro for rtXmlInitContext in the rtkey.h
 * file that will invoke this function with the generated run-time key.
 */
EXTXMLMETHOD int rtXmlInitContextUsingKey
(OSCTXT* pctxt, const OSOCTET* key, size_t keylen)
{
   int stat = rtXmlInitContext (pctxt);

   pctxt->key = key;
   pctxt->keylen = keylen;

   LCHECKX (pctxt);

   return stat;
}

EXTXMLMETHOD int rtXmlInitContext (OSCTXT* pctxt)
{
   int stat;
#ifdef ASN1RT
   stat = rtInitContext (pctxt);
#else
   stat = rtxInitContext (pctxt);
#endif
   if (0 == stat) {
      stat = rtXmlInitCtxtAppInfo (pctxt);
   }

   return stat;
}
