/*
 * Copyright (c) 2014-2023 Objective Systems, Inc.
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxHashMap.h"
#include "rtxsrc/rtxMemory.h"

/*
Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
*/
static const OSUINT32 primes[] = {
   53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
   196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
   50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
};
static const double max_load_factor = 0.65;

EXTRTMETHOD void HASHMAPINITFUNC
(OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap, size_t capacity,
 OSUINT32 (*hashFunc)(HASHMAPKEYTYPE),
 OSBOOL (*keyEqualsFunc)(HASHMAPKEYTYPE,HASHMAPKEYTYPE))
{
   OSUINT32 i, size = primes[0];
   if (0 != pHashMap) {
      /* Enforce size as prime */
      for (i = 0; i < OSRTARRAYSIZE(primes); i++) {
         if (primes[i] > capacity) { size = primes[i]; break; }
      }
      pHashMap->table = rtxMemAllocArrayZ (pctxt, size, HASHMAPENTRYTYPE*);
      pHashMap->tableLength  = size;
      pHashMap->primeIndex   = i;
      pHashMap->entryCount   = 0;
      pHashMap->hashFunc     = hashFunc;
      pHashMap->equalsFunc   = keyEqualsFunc;
      pHashMap->loadLimit    = (OSUINT32) ceil (size * max_load_factor);
   }
}

EXTRTMETHOD HASHMAPTYPENAME* HASHMAPNEWFUNC
(OSCTXT* pctxt, size_t capacity, OSUINT32 (*hashFunc)(HASHMAPKEYTYPE),
 OSBOOL (*keyEqualsFunc)(HASHMAPKEYTYPE,HASHMAPKEYTYPE))
{
   HASHMAPTYPENAME* pHashMap = rtxMemAllocType (pctxt, HASHMAPTYPENAME);
   if (0 != pHashMap) {
      HASHMAPINITFUNC (pctxt, pHashMap, capacity, hashFunc, keyEqualsFunc);
   }
   return pHashMap;
}

EXTRTMETHOD HASHMAPTYPENAME* HASHMAPCOPYFUNC (OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap)
{
   HASHMAPTYPENAME* pNewHashMap = rtxMemAllocType (pctxt, HASHMAPTYPENAME);
   if (0 != pNewHashMap) {
      HASHMAPENTRYTYPE* pEntry;
      OSUINT32 i;

      pNewHashMap->tableLength  = pHashMap->tableLength;
      pNewHashMap->primeIndex   = pHashMap->primeIndex;
      pNewHashMap->entryCount   = 0;
      pNewHashMap->hashFunc     = pHashMap->hashFunc;
      pNewHashMap->equalsFunc   = pHashMap->equalsFunc;
      pNewHashMap->loadLimit    = pHashMap->loadLimit;

      pNewHashMap->table = rtxMemAllocArrayZ
         (pctxt, pHashMap->tableLength, HASHMAPENTRYTYPE*);

      /* Insert all entries from old table into new table */
      for (i = 0; i < pHashMap->tableLength; i++) {
         pEntry = pHashMap->table[i];
         while (0 != pEntry) {
            HASHMAPINSERTFUNC (pctxt, pNewHashMap, pEntry->key, pEntry->value);
            pEntry = pEntry->next;
         }
      }
   }
   return pNewHashMap;
}

EXTRTMETHOD void HASHMAPFREEFUNC (OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap)
{
   if (0 != pHashMap) {
      HASHMAPENTRYTYPE* pEntry, *pEntry2;
      OSUINT32 i;

      /* Free all entries in the map */
      for (i = 0; i < pHashMap->tableLength; i++) {
         pEntry = pHashMap->table[i];
         while (0 != pEntry) {
            pEntry2 = pEntry->next;
            rtxMemFreePtr (pctxt, pEntry);
            pEntry = pEntry2;
         }
      }
      rtxMemFreePtr (pctxt, pHashMap->table);
   }
}

static OSUINT32 indexFor (OSUINT32 tablelength, OSUINT32 hashvalue)
{
   return (hashvalue % tablelength);
}

static int hashMapExpand (OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap)
{
   HASHMAPENTRYTYPE* pEntry;
   HASHMAPENTRYTYPE** ppNewTable;
   OSUINT32 newsize, i, idx;

   /* Check if reached max capacity */
   if (pHashMap->primeIndex == (OSRTARRAYSIZE(primes) - 1))
      return RTERR_NOMEM;

   newsize = primes[++(pHashMap->primeIndex)];

   ppNewTable = rtxMemAllocArrayZ (pctxt, newsize, HASHMAPENTRYTYPE*);
   if (0 == ppNewTable) return RTERR_NOMEM;

   /* This algorithm is not 'stable'. ie. it reverses the list
    * when it transfers entries between the tables */
   for (i = 0; i < pHashMap->tableLength; i++) {
      while (0 != (pEntry = pHashMap->table[i])) {
         pHashMap->table[i] = pEntry->next;
         idx = indexFor (newsize, pEntry->hashCode);
         pEntry->next = ppNewTable[idx];
         ppNewTable[idx] = pEntry;
      }
   }
   rtxMemFreePtr (pctxt, pHashMap->table);
   pHashMap->table = ppNewTable;
   pHashMap->tableLength = newsize;
   pHashMap->loadLimit = (OSUINT32) ceil (newsize * max_load_factor);

   return 0;
}

static OSUINT32 computeHashCode (HASHMAPTYPENAME* pHashMap, HASHMAPKEYTYPE key)
{
   /* Aim to protect against poor hash functions by adding logic here
    * - logic taken from java 1.4 hashtable source */
   OSUINT32 i = pHashMap->hashFunc (key);
   i += ~(i << 9);
   i ^=  ((i >> 14) | (i << 18)); /* >>> */
   i +=  (i << 4);
   i ^=  ((i >> 10) | (i << 22)); /* >>> */
   return i;
}

EXTRTMETHOD int HASHMAPINSERTFUNC
(OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap, HASHMAPKEYTYPE key,
 HASHMAPVALUETYPE value)
{
   OSUINT32 idx;
   HASHMAPENTRYTYPE* pEntry;

   if (++(pHashMap->entryCount) > pHashMap->loadLimit) {
      hashMapExpand (pctxt, pHashMap);
   }
   pEntry = rtxMemAllocType (pctxt, HASHMAPENTRYTYPE);
   if (0 == pEntry) { --(pHashMap->entryCount); return RTERR_NOMEM; }

   pEntry->hashCode = computeHashCode (pHashMap, key);
   idx = indexFor (pHashMap->tableLength, pEntry->hashCode);
   pEntry->key = key;
   pEntry->value = value;
   pEntry->next = pHashMap->table[idx];
   pHashMap->table[idx] = pEntry;

   return 0;
}

EXTRTMETHOD int HASHMAPPUTFUNC
(OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap, HASHMAPKEYTYPE key,
 HASHMAPVALUETYPE value)
{
   OSUINT32 hashCode, idx;
   HASHMAPENTRYTYPE* pEntry;

   hashCode = computeHashCode (pHashMap, key);
   idx = indexFor (pHashMap->tableLength, hashCode);

   /* Search chain for key */

   pEntry = pHashMap->table[idx];
   while (0 != pEntry) {
      /* Check hash value to short circuit heavier comparison */
      if ((hashCode == pEntry->hashCode) &&
          (pHashMap->equalsFunc (key, pEntry->key))) {
         break;
      }
      pEntry = pEntry->next;
   }

   /* If found, update value; otherwise, insert */

   if (0 != pEntry) {
      pEntry->value = value;
   }
   else {
      if (++(pHashMap->entryCount) > pHashMap->loadLimit) {
         hashMapExpand (pctxt, pHashMap);
      }
      pEntry = rtxMemAllocType (pctxt, HASHMAPENTRYTYPE);
      if (0 == pEntry) { --(pHashMap->entryCount); return RTERR_NOMEM; }

      pEntry->hashCode = hashCode;
      idx = indexFor (pHashMap->tableLength, pEntry->hashCode);
      pEntry->key = key;
      pEntry->value = value;
      pEntry->next = pHashMap->table[idx];
      pHashMap->table[idx] = pEntry;
   }

   return 0;
}

EXTRTMETHOD OSBOOL HASHMAPSEARCHFUNC
(HASHMAPTYPENAME* pHashMap, HASHMAPKEYTYPE key, HASHMAPVALUETYPE* pvalue)
{
   OSUINT32 hashvalue, idx;
   HASHMAPENTRYTYPE* pEntry;

   hashvalue = computeHashCode (pHashMap, key);
   idx = indexFor (pHashMap->tableLength, hashvalue);
   pEntry = pHashMap->table[idx];
   while (0 != pEntry) {
      /* Check hash value to short circuit heavier comparison */
      if ((hashvalue == pEntry->hashCode) &&
          (pHashMap->equalsFunc (key, pEntry->key))) {
         if (0 != pvalue) *pvalue = pEntry->value;
         return TRUE;
      }
      pEntry = pEntry->next;
   }

   return FALSE;
}

EXTRTMETHOD OSBOOL HASHMAPREMOVEFUNC
(OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap, HASHMAPKEYTYPE key,
 HASHMAPVALUETYPE* pvalue)
{
   /* TODO: consider compacting the table when the load factor drops enough,
    *       or provide a 'compact' method. */

   HASHMAPENTRYTYPE* pEntry;
   HASHMAPENTRYTYPE** ppEntry;
   OSUINT32 hashvalue, idx;

   hashvalue = computeHashCode (pHashMap, key);
   idx = indexFor (pHashMap->tableLength, hashvalue);
   ppEntry = &(pHashMap->table[idx]);
   pEntry = *ppEntry;
   while (0 != pEntry) {
      /* Check hash value to short circuit heavier comparison */
      if ((hashvalue == pEntry->hashCode) &&
          (pHashMap->equalsFunc (key, pEntry->key)))
      {
         *ppEntry = pEntry->next;
         pHashMap->entryCount--;
         if (0 != pvalue) *pvalue = pEntry->value;
         rtxMemFreePtr (pctxt, pEntry);
         return TRUE;
      }
      ppEntry = &(pEntry->next);
      pEntry = pEntry->next;
   }

   return FALSE;
}

EXTRTMETHOD int HASHMAPSORTFUNC
(OSCTXT* pctxt, HASHMAPTYPENAME* pHashMap, OSRTDList* pSortedList,
 int (*compareFunc)(HASHMAPKEYTYPE key1, HASHMAPKEYTYPE key2))
{
   HASHMAPENTRYTYPE* pEntry, *pListEntry;
   OSRTDListNode* pListNode;
   OSUINT32 i, listidx;

   /* Iterate through all entries in the map */
   for (i = 0; i < pHashMap->tableLength; i++) {
      pEntry = pHashMap->table[i];
      while (0 != pEntry) {
         pListNode = pSortedList->head;
         listidx = 0;
         while (0 != pListNode) {
            pListEntry = (HASHMAPENTRYTYPE*) pListNode->data;
            if (compareFunc (pEntry->key, pListEntry->key) >= 0)
               break;
            pListNode = pListNode->next; listidx++;
         }
         /* Add entry to sorted list */
         rtxDListInsert (pctxt, pSortedList, listidx, pEntry);
         pEntry = pEntry->next;
      }
   }
   return 0;
}

