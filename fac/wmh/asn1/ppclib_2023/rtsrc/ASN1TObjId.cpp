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
#include "rtsrc/ASN1TObjId.h"

static int nCompare
(OSUINT32 length, const ASN1OBJID &oid1, const ASN1OBJID &oid2);

static int StrToOid (const char *str, ASN1OBJID * dstOid);

// Object identifier class methods

EXTRTMETHOD ASN1TObjId::ASN1TObjId (OSOCTET _numids, const OSUINT32* _subids) {
   numids = _numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = _subids[ui];
   }
}

EXTRTMETHOD ASN1TObjId::ASN1TObjId (const ASN1OBJID& oid) {
   numids = oid.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = oid.subid[ui];
   }
}

EXTRTMETHOD ASN1TObjId::ASN1TObjId (const ASN1TObjId& oid) {
   numids = oid.numids;
   for (OSUINT32 ui = 0; ui < numids; ui++) {
      subid[ui] = oid.subid[ui];
   }
}

EXTRTMETHOD ASN1TObjId::ASN1TObjId (const char *dotted_oid_string) {
   char const *str = dotted_oid_string;
   size_t index = 0, nz;

   if (str && *str) {
      nz = strlen(str);
   }
   else {
      numids = 0; nz = 0;
      subid[numids] = 0;
      return;
   }

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

   numids = (OSUINT32)index;
}

EXTRTMETHOD ASN1TObjId::~ASN1TObjId () {
}


      void ASN1TObjId::operator= (const ASN1OBJID& rhs) {
   numids = rhs.numids;
   for (OSUINT32 ui = 0; ui < numids && ui < ASN_K_MAXSUBIDS; ui++) {
      subid[ui] = rhs.subid[ui];
   }
}

void ASN1TObjId::operator= (const ASN1TObjId& rhs) {
   numids = rhs.numids;
   for (OSUINT32 ui = 0; ui < numids && ui < ASN_K_MAXSUBIDS; ui++) {
      subid[ui] = rhs.subid[ui];
   }
}


ASN1TObjId& ASN1TObjId::operator=(const char *dotted_oid_string) {
   StrToOid(dotted_oid_string, this);
   return *this;
}

 int operator==(const ASN1OBJID &lhs, const ASN1OBJID& rhs) {
   if (lhs.numids != rhs.numids) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numids ; ui++) {
      if (lhs.subid[ui] != rhs.subid[ui]) return 0;
   }
   return 1;
}

 int operator==(const ASN1TObjId &lhs, const ASN1TObjId& rhs) {
   if (lhs.numids != rhs.numids) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numids ; ui++) {
      if (lhs.subid[ui] != rhs.subid[ui]) return 0;
   }
   return 1;
}

 int operator==(const ASN1TObjId &lhs, const char *dotted_oid_string) {
   ASN1TObjId tempOid(dotted_oid_string);
   if(lhs==tempOid)
      return 1;
   return 0;
}

 int operator==(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   ASN1OBJID tempOid;
   if(StrToOid(dotted_oid_string, &tempOid) <= 0)
      return 0;

   if (lhs.numids != tempOid.numids) return 0;
   for (OSUINT32 ui = 0; ui < lhs.numids ; ui++) {
      if (lhs.subid[ui] != tempOid.subid[ui]) return 0;
   }
   return 1;
}


 int operator!=(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   return (!(lhs==rhs));   // just invert ==
}

 int operator!=(const ASN1OBJID &lhs, const ASN1OBJID &rhs) {
   return (!(lhs==rhs));   // just invert ==
}


 int operator!=(const ASN1TObjId &lhs, const char *dotted_oid_string) {
   return (!(lhs==dotted_oid_string));   // just invert ==
}


 int operator!=(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   return (!(lhs==dotted_oid_string));   // just invert ==
}


 int operator<(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   int result;
   if((result = lhs.nCompare(rhs.numids, rhs))<0)  return 1;
   if (result > 0)    return 0;

   return (lhs.numids < rhs.numids);
}


 int operator<(const ASN1OBJID &lhs, const ASN1OBJID &rhs) {
   int result;
   if((result = nCompare(rhs.numids, lhs, rhs))<0)  return 1;
   if (result > 0)    return 0;

   return (lhs.numids < rhs.numids);
}

 int operator<(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   ASN1OBJID tempOid;
   if(StrToOid(dotted_oid_string, &tempOid) <= 0)
      return 0;
   return (lhs < tempOid);  // compare using existing operator
}

 int operator<(const ASN1TObjId &lhs, const char *dotted_oid_string) {
   ASN1TObjId tempOid(dotted_oid_string);  // create a temp oid object
   return (lhs < tempOid);  // compare using existing operator
}


 int operator<=(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   return ((lhs<rhs) || (lhs==rhs));
}

 int operator<=(const ASN1OBJID &lhs, const ASN1OBJID &rhs) {
   return ((lhs<rhs) || (lhs==rhs));
}


 int operator<=(const ASN1TObjId &lhs,char *dotted_oid_string) {
   ASN1TObjId tempOid(dotted_oid_string);
   return (lhs <= tempOid);
}


 int operator<=(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   ASN1OBJID tempOid;
   if(StrToOid(dotted_oid_string, &tempOid) <= 0)
      return 0;
   return (lhs <= tempOid);
}



 int operator>(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   return (!(lhs<=rhs));
}

 int operator>(const ASN1OBJID &lhs, const ASN1OBJID &rhs) {
   return (!(lhs<=rhs));
}

 int operator>(const ASN1TObjId &lhs, const char *dotted_oid_string) {
   ASN1TObjId to(dotted_oid_string);
   return (!(lhs<= to));
}

 int operator>(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   ASN1OBJID tempOid;
   if(StrToOid(dotted_oid_string, &tempOid) <= 0)
      return 0;

   return (!(lhs<= tempOid));
}


 int operator>=(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   return (!(lhs<rhs));
}


 int operator>=(const ASN1TObjId &lhs, const char *dotted_oid_string) {
   ASN1TObjId tempOid(dotted_oid_string);
   return (!(lhs<tempOid));
}

 int operator>=(const ASN1OBJID &lhs, const ASN1OBJID &rhs) {
   return (!(lhs<rhs));
}

 int operator>=(const ASN1OBJID &lhs, const char *dotted_oid_string) {
   ASN1OBJID tempOid;
   if(StrToOid(dotted_oid_string, &tempOid) <= 0)
      return 0;

   return (!(lhs<tempOid));
}

 ASN1TObjId operator+(const ASN1TObjId &lhs, const ASN1TObjId &rhs) {
   ASN1TObjId tmp(lhs);
   tmp += rhs;
   return tmp;
}



EXTRTMETHOD void ASN1TObjId::set_data(const OSUINT32 *raw_oid,
                          OSUINT32 oid_len)
{
   if (raw_oid && (oid_len > 0)) {
      numids = oid_len;
      for (OSUINT32 i=0; i < numids && i < ASN_K_MAXSUBIDS; i++)
         subid[i] = raw_oid[i];
   }
}


ASN1TObjId& ASN1TObjId::operator+=(const char *dotted_oid_string)
{
   ASN1TObjId tempOid(dotted_oid_string);
   if (tempOid.numids==0) return *this;
   numids += tempOid.numids;
   OSUINT32 uj=0;
   for (OSUINT32 ui = (numids - tempOid.numids);
	   ui < numids && ui < ASN_K_MAXSUBIDS; ui++, uj++)
   {
      subid[ui] = tempOid.subid[uj];
   }
   return *this;
}

ASN1TObjId& ASN1TObjId::operator+=(const OSUINT32 i) {
   ASN1TObjId tempOid((OSOCTET)1, &i);
   (*this) += tempOid;
   return *this;
}


ASN1TObjId& ASN1TObjId::operator+=(const ASN1TObjId &o) {
   if (o.numids == 0)
      return *this;
   numids += o.numids;
   OSUINT32 uj=0;
   for (OSUINT32 ui = (numids - o.numids); ui < numids && ui < ASN_K_MAXSUBIDS; ui++, uj++) {
      subid[ui] = o.subid[uj];
   }
   return *this;
}

EXTRTMETHOD const char *ASN1TObjId::toString (OSCTXT* pctxt) const
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

      pCurrLoc += os_snprintf (pCurrLoc, nbytes-1, "%u", subid[index]);
   }

   return pBuffer;
}

EXTRTMETHOD int ASN1TObjId::nCompare(const OSUINT32 n, const ASN1TObjId &o) const
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
      if (subid[z] < o.subid[z])
         return -1;
      if (subid[z] > o.subid[z])
         return 1;
      ++z;
   }

   if (reduced_len) {
      if (numids< o.numids) return -1;
      if (numids> o.numids) return 1;
   }
   return 0;
}


EXTRTMETHOD int ASN1TObjId::RnCompare(const OSUINT32 n, const ASN1TObjId &o) const {
   OSUINT32 length = n;

   while ((numids< length) && (o.numids< length))
      length--;

   if (o.numids< length)    return -1;

   if (numids < length) return -1;

   int start = (int) (numids -1);
   int end = (int) start - (int) length;
   for (int z = start; z > end; z--) {
      if (subid[z] < o.subid[z])   return -1;
      if (subid[z] > o.subid[z])   return 1;
   }
   return 0;
}

EXTRTMETHOD void ASN1TObjId::trim(const OSUINT32 n) {

   if ((n <= numids) && (n > 0)) {
      numids -= n;
   }
}

static int nCompare
(OSUINT32 length, const ASN1OBJID &oid1, const ASN1OBJID &oid2)
{
   OSBOOL reduced_len = FALSE;

   while ((oid1.numids< length) && (oid2.numids< length))
      length--;

   if (length == 0) return 0;

   if (length >oid1.numids) {
      length =oid1.numids;
      reduced_len = TRUE;
   }
   if (length > oid2.numids) {
      length = oid2.numids;
      reduced_len = TRUE;
   }

   unsigned int z = 0;
   while (z < length) {
      if (oid1.subid[z] < oid2.subid[z])
         return -1;
      if (oid1.subid[z] > oid2.subid[z])
         return 1;
      ++z;
   }

   if (reduced_len) {
      if (oid1.numids < oid2.numids) return -1;
      if (oid1.numids > oid2.numids) return 1;
   }
   return 0;
}

static int StrToOid (const char *str, ASN1OBJID * dstOid)
{
   size_t index = 0, nz;

   if (str && *str) {
      nz = strlen(str);
   }
   else {
      dstOid->numids= 0;
      dstOid->subid[dstOid->numids] = 0;
      return -1;
   }

   while ((*str) && (index < nz) && (index < ASN_K_MAXSUBIDS) ) {
      OSUINT32 number = 0;
      if (*str == '.') ++str;

      while (OS_ISDIGIT(*str))
         number = (number * 10) + *(str++) - '0';

      if ((*str) && (*str != '.')) {
         if ((*str) && (*str == '$')) {
            ++str;
            while ((*str) && (*str != '$')) {
               dstOid->subid[index] = (unsigned char)*str;
               ++str;
               ++index;
            }
            if (*str) ++str;
            continue;
         }
         return -1;
      }

      dstOid->subid[index++] = number;
   }

   dstOid->numids = (OSUINT32)index;

   return (int) index;
}
