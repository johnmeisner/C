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
#include "rtsrc/ASN1TOctStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxPrint.h"

static int nCompare
(OSUINT32 length, const ASN1DynOctStr &os1, const ASN1DynOctStr &os2);

static void StrToOctet (const char *cstring, ASN1DynOctStr* dstOctet);

 ASN1TDynOctStr& ASN1TDynOctStr::operator=(const ASN1TDynOctStr& octet)
{
   //!AB 07/21/2004: Not sure it is correct behaviour, but implementation
   // of this operator was absent at all. Thus I had to implement at least
   // something to satisfy linker.
   numocts = octet.numocts;
   data = octet.data;
   return *this;
}

EXTRTMETHOD int ASN1TDynOctStr::nCompare (OSUINT32 n, const ASN1TDynOctStr &o) const
{
   OSUINT32 n_max;
   OSUINT32 w, slen;

   if (n == 0) return 0;

   if ((numocts == 0) && (o.numocts == 0))
      return 0;

   if ((numocts == 0) && (o.numocts > 0))
      return -1;

   if ((numocts > 0) && (o.numocts == 0))
      return 1;

   slen = (numocts < o.numocts) ? numocts : o.numocts;
   w = (n <= slen) ? n : slen;

   OSUINT32 z = 0;
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
(OSUINT32 length, const ASN1DynOctStr &os1, const ASN1DynOctStr &os2)
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

   OSUINT32 z = 0;
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

EXTRTMETHOD const char* ASN1TDynOctStr::toString (OSCTXT* pctxt) const
{
   for (OSUINT32 i=0; i < numocts; i++) {
      if (!OS_ISPRINT (data[i]))
         return toHexString (pctxt);
   }

   char* pBuffer = (char*) rtxMemAlloc (pctxt, numocts+1) ;

   if (numocts)
      memcpy (pBuffer, data, (OSUINT32)numocts);

   pBuffer[numocts] = '\0';

   return pBuffer;
}

EXTRTMETHOD const char* ASN1TDynOctStr::toHexString (OSCTXT* pctxt) const
{
   OSUINT32 nbytes = (OSUINT32) ((numocts/16)+1) * 72 + 1;

   char* pBuffer = (char*) rtxMemAlloc (pctxt, nbytes) ;
   if (0 == pBuffer) return 0;

   rtxHexDumpToString (data, numocts, pBuffer, 0, nbytes);

   return pBuffer;
}

 int operator==(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){

   if (lhs.numocts != rhs.numocts) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numocts ; ui++) {
      if (lhs.data[ui] != rhs.data[ui]) return 0;
   }
   return 1;
}

 int operator==(const ASN1TDynOctStr &lhs,const char *string){

   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen, to) == 0);
}

 int operator==(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){

   if (lhs.numocts != rhs.numocts) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numocts ; ui++) {
      if (lhs.data[ui] != rhs.data[ui]) return 0;
   }
   return 1;
}

 int operator==(const ASN1DynOctStr &lhs,const char *string){
   ASN1DynOctStr to;
   StrToOctet(string, &to);
   int maxlen = lhs.numocts > to.numocts
             ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs,to) == 0);
}

 int operator!=(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
             ? lhs.numocts :  rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) != 0);
}

 int operator!=(const ASN1TDynOctStr &lhs,const char *string){

   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen, to) != 0);
}

 int operator!=(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){

   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs, rhs) != 0);
}

 int operator!=(const ASN1DynOctStr &lhs,const char *string){

   ASN1DynOctStr to;
   StrToOctet(string, &to);

   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs, to) != 0);
}


 int operator<(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){

   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) < 0);
}

 int operator<(const ASN1TDynOctStr &lhs,const char *string){
   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen,to) < 0);
}

 int operator<(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){

   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs,rhs) < 0);
}

 int operator<(const ASN1DynOctStr &lhs,const char *string){
   ASN1DynOctStr to;
   StrToOctet(string, &to);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs,to) < 0);
}

 int operator<=(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) <= 0);
}

 int operator<=(const ASN1TDynOctStr &lhs,const char *string){
   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen, to) <= 0);
}

 int operator<=(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs, rhs) <= 0);
}

 int operator<=(const ASN1DynOctStr &lhs,const char *string){
   ASN1DynOctStr to;
   StrToOctet(string, &to);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs, to) <= 0);
}

 int operator>(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) > 0);
}

 int operator>(const ASN1TDynOctStr &lhs,const char *string){
   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen, to) > 0);
}

 int operator>(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs, rhs) > 0);
}

 int operator>(const ASN1DynOctStr &lhs,const char *string){
   ASN1DynOctStr to;
   StrToOctet(string, &to);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs, to) > 0);
}

 int operator>=(const ASN1TDynOctStr &lhs,const ASN1TDynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
             ? lhs.numocts : rhs.numocts;
   return (lhs.nCompare(maxlen, rhs) >=0);
}

 int operator>=(const ASN1TDynOctStr &lhs,const char *string){
   ASN1TDynOctStr to(string);
   int maxlen = lhs.numocts > to.numocts
             ? lhs.numocts : to.numocts;
   return (lhs.nCompare(maxlen, to) >= 0);
}

 int operator>=(const ASN1DynOctStr &lhs,const ASN1DynOctStr &rhs){
   int maxlen = lhs.numocts > rhs.numocts
      ? lhs.numocts : rhs.numocts;
   return (nCompare(maxlen,lhs, rhs) >=0);
}

 int operator>=(const ASN1DynOctStr &lhs,const char *string){
   ASN1DynOctStr to;
   StrToOctet(string, &to);
   int maxlen = lhs.numocts > to.numocts
      ? lhs.numocts : to.numocts;
   return (nCompare(maxlen,lhs, to) >= 0);
}

static void StrToOctet (const char *cstring, ASN1DynOctStr* dstOctet)
{
   if (strlen(cstring) > 0) {
      dstOctet->numocts = (OSUINT32)strlen(cstring) + 1;
      dstOctet->data = (const OSOCTET*) cstring;
   }
   else
   {
      dstOctet->numocts = 0;
      dstOctet->data = NULL;
   }
}
