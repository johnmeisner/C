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

#include <stdlib.h>
#include <string.h>
#include "rtsrc/ASN1TOctStr64.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxPrint.h"

static int nCompare
(OSSIZE length, const OSDynOctStr64 &os1, const OSDynOctStr64 &os2);

static void StrToOctet (const char *cstring, OSDynOctStr64* dstOctet);

ASN1TDynOctStr64& ASN1TDynOctStr64::operator=(const ASN1TDynOctStr64& octet)
{
   numocts = octet.numocts;
   data = octet.data;
   return *this;
}

EXTRTMETHOD int ASN1TDynOctStr64::nCompare
(OSSIZE n, const ASN1TDynOctStr64 &o) const
{
   OSSIZE n_max;
   OSSIZE w, slen;

   if (n == 0) return 0;

   if ((numocts == 0) && (o.numocts == 0))
      return 0;

   if ((numocts == 0) && (o.numocts > 0))
      return -1;

   if ((numocts > 0) && (o.numocts == 0))
      return 1;

   slen = (numocts < o.numocts) ? numocts : o.numocts;
   w = (n <= slen) ? n : slen;

   OSSIZE z = 0;
   while (z < w) {
      if (data[z] < o.data[z])
      return -1;
      if (data[z] > o.data[z])
      return 1;
      z++;
   }

   n_max = (numocts > o.numocts) ? numocts : o.numocts;
   if (n < n_max) n_max = n;

   if (w < n_max) { // ==> we have compared too few bytes
      if (numocts < o.numocts)
         return -1;
      else
         return 1;
   }

   return 0;
}

static int nCompare
(OSSIZE length, const OSDynOctStr64 &os1, const OSDynOctStr64 &os2)
{
   OSBOOL reduced_len = FALSE;

   while ((os1.numocts < length) && (os2.numocts < length))
      length--;

   if (length == 0) return 0;

   if (length > os1.numocts) {
      length = os1.numocts;
      reduced_len = TRUE;
   }
   if (length > os2.numocts) {
      length = os2.numocts;
      reduced_len = TRUE;
   }

   OSSIZE z = 0;
   while (z < length) {
      if (os1.data[z] < os2.data[z])
         return -1;
      if (os1.data[z] > os2.data[z])
         return 1;
      ++z;
   }

   if (reduced_len) {
      if (os1.numocts < os2.numocts) return -1;
      if (os1.numocts > os2.numocts) return 1;
   }

   return 0;
}

EXTRTMETHOD const char* ASN1TDynOctStr64::toString (OSCTXT* pctxt) const
{
   for (OSSIZE i=0; i < numocts; i++) {
      if (!OS_ISPRINT (data[i]))
         return toHexString (pctxt);
   }

   char* pBuffer = (char*) rtxMemAlloc (pctxt, numocts+1) ;

   if (numocts)
      memcpy (pBuffer, data, numocts);

   pBuffer[numocts] = '\0';

   return pBuffer;
}

EXTRTMETHOD const char* ASN1TDynOctStr64::toHexString (OSCTXT* pctxt) const
{
   OSSIZE nbytes = ((numocts/16)+1) * 72 + 1;

   char* pBuffer = (char*) rtxMemAlloc (pctxt, nbytes) ;
   if (0 == pBuffer) return 0;

   rtxHexDumpToString (data, numocts, pBuffer, 0, nbytes);

   return pBuffer;
}

bool operator==(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   if (lhs.numocts != rhs.numocts) return false;
   for (OSSIZE ui = 0; ui < lhs.numocts ; ui++) {
      if (lhs.data[ui] != rhs.data[ui]) return false;
   }
   return true;
}

bool operator==(const ASN1TDynOctStr64 &lhs,const char *string)
{
   ASN1TDynOctStr64 to(string);
   return (lhs == to);
}

bool operator==(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   if (lhs.numocts != rhs.numocts) return false;
   for (OSSIZE ui = 0; ui < lhs.numocts ; ui++) {
      if (lhs.data[ui] != rhs.data[ui]) return false;
   }
   return true;
}

bool operator==(const OSDynOctStr64 &lhs,const char *string)
{
   OSDynOctStr64 to;
   StrToOctet(string, &to);
   return (lhs == to);
}

bool operator!=(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   return !(lhs == rhs);
}

bool operator!=(const ASN1TDynOctStr64 &lhs,const char *string)
{
   return !(lhs == string);
}

bool operator!=(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   return !(lhs == rhs);
}

bool operator!=(const OSDynOctStr64 &lhs,const char *string)
{
   return !(lhs == string);
}

bool operator<(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   OSSIZE maxlen = lhs.numocts > rhs.numocts ? lhs.numocts : rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) < 0);
}

bool operator<(const ASN1TDynOctStr64 &lhs,const char *string)
{
   ASN1TDynOctStr64 to(string);
   return (lhs < to);
}

bool operator<(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   OSSIZE maxlen = lhs.numocts > rhs.numocts ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs,rhs) < 0);
}

bool operator<(const OSDynOctStr64 &lhs,const char *string)
{
   OSDynOctStr64 to;
   StrToOctet(string, &to);
   return (lhs < to);
}

bool operator<=(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   return (lhs < rhs || lhs == rhs);
}

bool operator<=(const ASN1TDynOctStr64 &lhs,const char *string)
{
   ASN1TDynOctStr64 to(string);
   return (lhs <= to);
}

bool operator<=(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   OSSIZE maxlen = lhs.numocts > rhs.numocts ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs, rhs) <= 0);
}

bool operator<=(const OSDynOctStr64 &lhs,const char *string){
   OSDynOctStr64 to;
   StrToOctet(string, &to);
   return (lhs <= to);
}

bool operator>(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   return !(lhs <= rhs);
}

bool operator>(const ASN1TDynOctStr64 &lhs,const char *string)
{
   ASN1TDynOctStr64 to(string);
   return (lhs > to);
}

bool operator>(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   OSSIZE maxlen = lhs.numocts > rhs.numocts ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen, lhs, rhs) > 0);
}

bool operator>(const OSDynOctStr64 &lhs,const char *string)
{
   OSDynOctStr64 to;
   StrToOctet(string, &to);
   return (lhs > to);
}

bool operator>=(const ASN1TDynOctStr64 &lhs,const ASN1TDynOctStr64 &rhs)
{
   return !(lhs < rhs);
}

bool operator>=(const ASN1TDynOctStr64 &lhs,const char *string)
{
   ASN1TDynOctStr64 to(string);
   return (lhs >= to);
}

bool operator>=(const OSDynOctStr64 &lhs,const OSDynOctStr64 &rhs)
{
   OSSIZE maxlen = lhs.numocts > rhs.numocts ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen, lhs, rhs) >=0);
}

bool operator>=(const OSDynOctStr64 &lhs,const char *string)
{
   OSDynOctStr64 to;
   StrToOctet(string, &to);
   return (lhs >= to);
}

static void StrToOctet (const char *cstring, OSDynOctStr64* dstOctet)
{
   if (strlen(cstring) > 0) {
      dstOctet->numocts = strlen(cstring) + 1;
      dstOctet->data = (OSOCTET*) cstring;
   }
   else dstOctet->numocts = 0;
}
