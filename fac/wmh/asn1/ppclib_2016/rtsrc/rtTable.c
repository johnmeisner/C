/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/asn1type.h"
#include "rtsrc/rtTable.h"

EXTRTMETHOD void rtMemFreeNullPtr(OSCTXT* pctxt, void* pobject)
{
}

EXTRTMETHOD long rtBSearch (void *key, void *base, size_t nmemb, size_t size,
                int (*compar) (void *, void *))
{
   size_t l, u, idx;
   void *p;
   int comparison;

   l = 0;
   u = nmemb;
   while (l < u)
   {
      idx = (l + u) / 2;
      p = (void *) (((char *) base) + (idx * size) );
      comparison = (*compar) (key, p);
      if (comparison < 0) {
         u = idx;
      }
      else if (comparison > 0) {
         l = idx + 1;
      }
      else {
         return (long)idx;
      }
   }

   return -1L;
}

EXTRTMETHOD int rtLSearch (void *key, void *base, int nmemb, size_t size,
               int (*compar) (void *, void *))
{
   int l;
   void *p = base;
   nmemb--;

   for(l = 0; l < nmemb; l++)    {
      if(p != 0 && (*compar) (key, p) == 0) {
         return l;
      }
      p = (void *) ((char *)p + size);
   }

   /* Last Element */
   if(p != 0 && (*compar) (key, p) == 0) {
      return l;
   }

   return -1;
}

/* BOOLEAN (OSBOOL) */
EXTRTMETHOD int rtCmpTCBoolean (const OSBOOL* key, const OSBOOL* pCmpValue)
{
   if(*key == *pCmpValue) {
      return 0;
   }
   if(*key == TRUE) {
      return 1;
   }
   return -1;
}

/* INTEGER (OSINT8) */
EXTRTMETHOD int rtCmpTCINT8 (const OSINT8* key, const OSINT8* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCUINT8 (const OSUINT8* key, const OSUINT8* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCSINT (const OSINT16* key, const OSINT16* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCUSINT (const OSUINT16* key, const OSUINT16* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCINT (const OSINT32* key, const OSINT32* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCUINT (const OSUINT32* key, const OSUINT32* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCINT64 (const OSINT64* key, const OSINT64* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}

EXTRTMETHOD int rtCmpTCUINT64 (const OSUINT64* key, const OSUINT64* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* key > pCmpValue */
   return 1;
}


/* OBJECT IDENTIFIER / RELATIVE-OID */
EXTRTMETHOD int rtCmpTCOID (const ASN1OBJID* key, const ASN1OBJID* pcompOID)
{
   /* CmpTC No. Of Subids */
   if(key->numids == pcompOID->numids)
      {
         /* CmpTC Subids */
         unsigned int i;
         for(i=0; i< key->numids; i++)
            {
               if(key->subid[i] != pcompOID->subid[i])
                  {
                     return rtCmpTCUINT(&key->subid[i], &pcompOID->subid[i]);
                  }
            }
         /* All subIDS are equal */
         return 0;
      }
   return rtCmpTCUINT(&key->numids, &pcompOID->numids);
}

/* BIT STRING */
EXTRTMETHOD int rtCmpTCBitStr (OSUINT32 numbits, const OSOCTET* data,
   OSUINT32 cmpnumbits, const OSOCTET* cmpdata)
{
   if(numbits == cmpnumbits)
   {
      /* CmpTC bits */
      register int i, wholeBytesNum = numbits/8, extraBits = numbits % 8;
      for (i = 0; i < wholeBytesNum; i++) {
         if(data[i] != cmpdata[i])
         {
            return rtCmpTCOctet(&data[i], &cmpdata[i]);
         }
      }

      /* compare the bits in the last octet */
      if (extraBits > 0) {
         OSOCTET mask;
         mask = (OSOCTET)(0xFF << (8 - extraBits));
         if ((data[i] & (mask)) != (cmpdata[i] & (mask))) return -1;
      }
      /* All bits are equal */

      return 0;
   }
   return rtCmpTCUINT(&numbits, &cmpnumbits);
}

EXTRTMETHOD int rtCmpTCOctet (const OSOCTET* key, const OSOCTET* pCmpValue)
{
   if(*key == *pCmpValue)
   {
      return 0;
   }
   else if(*key < *pCmpValue)
   {
      return -1;
   }
   /* value > pCmpValue */
   return 1;
}

/* OCTET STRING */
EXTRTMETHOD int rtCmpTCOctStr (OSUINT32 numocts, const OSOCTET* data,
   OSUINT32 cmpnumocts, const OSOCTET* cmpdata)
{
   if(numocts == cmpnumocts)
   {
      unsigned int i;
      for(i=0; i < numocts; i++)
         {
            if(data[i] != cmpdata[i])
               {
                  /* const char* = OSOCTET* = unsigned char*  */
                  return rtCmpTCOctet(&data[i], &cmpdata[i]);
               }
         }
      return 0;
   }
   return rtCmpTCUINT(&numocts, &cmpnumocts);
}

/* CHARACTER STRING */
EXTRTMETHOD int rtCmpTCCharStr (const char* cstring, const char* compCstring)
{
   return strcmp((cstring), (compCstring));
}

/* BMP STRING */
EXTRTMETHOD int rtCmpTC16BitCharStr
(Asn116BitCharString* key, Asn116BitCharString* pCmpValue)
{
   if(key->nchars == pCmpValue->nchars)
      {
         unsigned int i;
         for(i=0; i< key->nchars; i++)
            {
               if(key->data[i] != pCmpValue->data[i])
                  {
                     /* OSUNICHAR = OSUINT32 = unsigned short */
                     return rtCmpTCUSINT(&key->data[i], &pCmpValue->data[i]);
                  }
            }
         return 0;
      }
   else return (key->nchars < pCmpValue->nchars) ? -1 : 1;
}

/* Universal string */
EXTRTMETHOD int rtCmpTC32BitCharStr
(Asn132BitCharString* key, Asn132BitCharString* pCmpValue)
{
   if(key->nchars == pCmpValue->nchars)
      {
         unsigned int i;
         for(i=0; i< key->nchars; i++)
            {
               if(key->data[i] != pCmpValue->data[i])
                  {
                     /* OS32BITCHAR = OSUINT32 */
                     return rtCmpTCUINT(&key->data[i], &pCmpValue->data[i]);
                  }
            }
         return 0;
      }
   else return (key->nchars < pCmpValue->nchars) ? -1 : 1;
}

EXTRTMETHOD int rtCmpTCReal (const OSREAL* key, const OSREAL* pCmpValue)
{
   if(*key == *pCmpValue)
      {
         return 0;
      }
   else if(*key < *pCmpValue)
      {
         return -1;
      }
   /* value > pCmpValue */
   return 1;
}
