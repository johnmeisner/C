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
#include "rtxsrc/rtxCtype.h"
#include "rtsrc/ASN1TDynObjId.h"
#include "rtsrc/ASN1TObjId.h"

// Object identifier class methods

EXTRTMETHOD void ASN1TDynObjId::stdMemCopy
(OSSIZE _numids, const OSUINT32* _subids)
{
   if (_numids > 0) {
      pSubIds = new OSUINT32[_numids];
      if (pSubIds) {
         memAllocated = TRUE;
         mbStdMem = true;
         numids = (OSUINT16)_numids;
         for (OSUINT32 ui = 0; ui < numids; ui++) {
            pSubIds[ui] = _subids[ui];
         }
         return;
      }
   }
   numids = 0;
   pSubIds = NULL;
   memAllocated = FALSE;
   mbStdMem = false;
}

EXTRTMETHOD ASN1TDynObjId::ASN1TDynObjId
(OSOCTET _numids, const OSUINT32* _subids)
{
   stdMemCopy(_numids, _subids);
}

EXTRTMETHOD ASN1TDynObjId::ASN1TDynObjId (const ASN1OBJID& oid) {
   stdMemCopy(oid.numids, oid.subid);
}

EXTRTMETHOD ASN1TDynObjId::ASN1TDynObjId (const ASN1TDynObjId& oid) {
   stdMemCopy(oid.numids, oid.pSubIds);
}

EXTRTMETHOD ASN1TDynObjId::ASN1TDynObjId (const char *dotted_oid_string) {
   strToOid(dotted_oid_string);
}

EXTRTMETHOD ASN1TDynObjId::~ASN1TDynObjId () {
   if (memAllocated && mbStdMem) {
      delete [] pSubIds;
   }
}

void ASN1TDynObjId::operator= (const ASN1OBJID& rhs) {
   if (memAllocated && mbStdMem) {
      delete [] pSubIds;
   }
   stdMemCopy(rhs.numids, rhs.subid);
}

void ASN1TDynObjId::operator= (const ASN1TDynObjId& rhs) {
   if (memAllocated && mbStdMem) {
      delete [] pSubIds;
   }
   stdMemCopy(rhs.numids, rhs.pSubIds);
}


ASN1TDynObjId& ASN1TDynObjId::operator=(const char *dotted_oid_string) {
   if (memAllocated && mbStdMem) {
      delete [] pSubIds;
   }
   strToOid(dotted_oid_string);
   return *this;
}

int operator==(const ASN1TDynObjId &lhs, const ASN1TDynObjId& rhs) {
   if (lhs.numids != rhs.numids) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numids ; ui++) {
      if (lhs.pSubIds[ui] != rhs.pSubIds[ui]) return 0;
   }
   return 1;
}

int operator==(const ASN1TDynObjId &lhs, const char *dotted_oid_string) {
   ASN1TDynObjId tempOid(dotted_oid_string);
   return (lhs == tempOid) ? 1 : 0;
}

int operator!=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   return (!(lhs == rhs));   // just invert ==
}

int operator!=(const ASN1TDynObjId &lhs, const char *dotted_oid_string) {
   return (!(lhs==dotted_oid_string));   // just invert ==
}

int operator<(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   int result;
   if ((result = lhs.nCompare(rhs.numids, rhs)) < 0) return 1;
   if (result > 0) return 0;

   return (lhs.numids < rhs.numids);
}

int operator<(const ASN1TDynObjId &lhs, const char *dotted_oid_string) {
   ASN1TDynObjId tempOid(dotted_oid_string);  // create a temp oid object
   return (lhs < tempOid);  // compare using existing operator
}

int operator<=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   return ((lhs<rhs) || (lhs==rhs));
}

int operator<=(const ASN1TDynObjId &lhs, char *dotted_oid_string) {
   ASN1TDynObjId tempOid(dotted_oid_string);
   return (lhs <= tempOid);
}

int operator>(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   return (!(lhs<=rhs));
}

int operator>(const ASN1TDynObjId &lhs, const char *dotted_oid_string) {
   ASN1TDynObjId to(dotted_oid_string);
   return (!(lhs<= to));
}

int operator>=(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   return (!(lhs<rhs));
}

int operator>=(const ASN1TDynObjId &lhs, const char *dotted_oid_string) {
   ASN1TDynObjId tempOid(dotted_oid_string);
   return (!(lhs<tempOid));
}

ASN1TDynObjId operator+(const ASN1TDynObjId &lhs, const ASN1TDynObjId &rhs) {
   ASN1TDynObjId tmp(lhs);
   tmp += rhs;
   return tmp;
}

EXTRTMETHOD void ASN1TDynObjId::set_data
(const OSUINT32 *raw_oid, OSUINT32 oid_len)
{
   if (memAllocated && mbStdMem) {
      delete [] pSubIds;
   }
   stdMemCopy(oid_len, raw_oid);
}

int ASN1TDynObjId::appendSubIds(OSSIZE nsubids, const OSUINT32* pSubIdArray)
{
   if (nsubids == 0) return ASN_E_INVOBJID;
   OSSIZE tempNumids = numids + nsubids;
   if (tempNumids > ASN_K_MAXSUBIDS) return ASN_E_INVOBJID;
   OSUINT32* newSubIds = new OSUINT32[tempNumids];
   if (!newSubIds) { return RTERR_NOMEM; }
   OSUINT32 ui, uj;
   for (ui = 0; ui < numids; ui++) {
      newSubIds[ui] = pSubIds[ui];
   }
   for (uj = 0; uj < nsubids; ui++, uj++) {
      pSubIds[ui] = pSubIdArray[uj];
   }
   numids = (OSUINT16)tempNumids;
   pSubIds = newSubIds;
   memAllocated = TRUE;
   mbStdMem = true;
   return 0;
}

ASN1TDynObjId& ASN1TDynObjId::operator+=(const char *dotted_oid_string)
{
   ASN1TObjId tempOid(dotted_oid_string);
   appendSubIds(tempOid.numids, tempOid.subid);
   return *this;
}

ASN1TDynObjId& ASN1TDynObjId::operator+=(const OSUINT32 i)
{
   ASN1TObjId tempOid((OSOCTET)1, &i);
   appendSubIds(tempOid.numids, tempOid.subid);
   return *this;
}

ASN1TDynObjId& ASN1TDynObjId::operator+=(const ASN1TDynObjId &o)
{
   appendSubIds(o.numids, o.pSubIds);
   return *this;
}

EXTRTMETHOD const char *ASN1TDynObjId::toString (OSCTXT* pctxt) const
{
   OSUINT32 nbytes = (2 * numids - 1);
   OSBOOL first = TRUE;

   char* pBuffer = (char*) rtxMemAlloc (pctxt, nbytes) ;
   char* pCurrLoc = pBuffer;

   for (OSUINT32 index = 0; index < numids; ++index) {
      if (first)
         first = FALSE;
      else
         *pCurrLoc++ = '.';

      pCurrLoc += os_snprintf (pCurrLoc, nbytes-1, "%u", pSubIds[index]);
   }

   return pBuffer;
}

EXTRTMETHOD int ASN1TDynObjId::nCompare
(const OSUINT32 n, const ASN1TDynObjId &o) const
{
   OSUINT32 length = n;
   OSBOOL reduced_len = FALSE;

   while ((numids< length) && (o.numids< length))
      length--;

   if (length == 0) return 0;

   if (length >numids) {
      length =numids;
      reduced_len = TRUE;
   }
   if (length > o.numids) {
      length = o.numids;
      reduced_len = TRUE;
   }

   unsigned int z = 0;
   while (z < length) {
      if (pSubIds[z] < o.pSubIds[z])
         return -1;
      if (pSubIds[z] > o.pSubIds[z])
         return 1;
      ++z;
   }

   if (reduced_len) {
      if (numids< o.numids) return -1;
      if (numids> o.numids) return 1;
   }
   return 0;
}


EXTRTMETHOD int ASN1TDynObjId::RnCompare
(const OSUINT32 n, const ASN1TDynObjId &o) const
{
   OSUINT32 length = n;

   while ((numids< length) && (o.numids< length))
      length--;

   if (o.numids< length)    return -1;

   if (numids < length) return -1;

   int start = (int) (numids -1);
   int end = (int) start - (int) length;
   for (int z = start; z > end; z--) {
      if (pSubIds[z] < o.pSubIds[z])   return -1;
      if (pSubIds[z] > o.pSubIds[z])   return 1;
   }
   return 0;
}

EXTRTMETHOD void ASN1TDynObjId::trim(const OSUINT32 n) {

   if ((n <= numids) && (n > 0)) {
      numids -= n;
   }
}

int ASN1TDynObjId::strToOid (const char *str)
{
   size_t nz;

   if (str && *str) {
      nz = strlen(str);
   }
   else {
      numids = 0;
      pSubIds = NULL;
      memAllocated = FALSE;
      return -1;
   }

   OSUINT32 subid[ASN_K_MAXSUBIDS], index = 0;
   while ((*str) && (index < nz) && (index < ASN_K_MAXSUBIDS) ) {
      OSUINT32 number = 0;

      if (*str == '.') ++str;

      while (OS_ISDIGIT(*str))
         number = (number * 10) + *(str++) - '0';

      if ((*str) && (*str != '.')) {
         if ((*str) && (*str == '$')) {
            ++str;
            while ((*str) && (*str != '$')) {
               subid[index] = (unsigned char)*str;
               ++str;
               ++index;
            }
            if (*str) ++str;
            continue;
         }
      }
      subid[index++] = number;
   }

   stdMemCopy(index, subid);

   return (int) index;
}
