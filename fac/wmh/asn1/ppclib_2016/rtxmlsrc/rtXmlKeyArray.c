/*
 * Copyright (c) 2014-2018 by Objective Systems, Inc.
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

#include "rtxmlsrc/rtXmlKeyArray.h"
#include "rtxmlsrc/rtXmlErrCodes.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

int rtXmlKeyArrayInit
(OSCTXT* pctxt, OSXSDKeyArray* pArray, OSUINT32 nmFields, OSBOOL key,
 const char* nameStr)
{
   pArray->capacity = 8;
   pArray->count = 0;
   pArray->nmFields = nmFields;
   pArray->curField = 0;
   pArray->keyConstraint = key;
   pArray->duplicateField = -1;
   pArray->absentField = -1;
   pArray->name = nameStr;

   pArray->data = rtxMemAllocArray (pctxt, (pArray->capacity + 1) * nmFields,
                                    OSXSDKeyRecord);

   if (0 == pArray->data) {
      pArray->capacity = 0;
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return 0;
}

void rtXmlKeyArrayFree (OSCTXT* pctxt, OSXSDKeyArray* pArray)
{
   if (pArray->capacity > 0)
      rtxMemFreeArray (pctxt, pArray);

   pArray->capacity = 0;
}

/**
 * Compare two keys by comparing their respective fields.
 * @param pa: Pointer to first field of key value A.
 * @param pb: Pointer to first field of key value B.
 * @param nmFields: number of fields in each key.
 */
static int compareFields
(const OSXSDKeyRecord* pa, const OSXSDKeyRecord* pb, OSUINT32 nmFields)
{
   OSUINT32 i;
   int result;

   for (i = 0; i < nmFields; i++, pa++, pb++) {
      if (pa->tag == pb->tag) {
         if (pa->tag == OSXSDString) {
            result = OSCRTLSTRCMP ((const char*) pa->u.pValue,
                                 (const char*) pb->u.pValue);
            if ( result != 0 ) return result;
            /* else: compare next field */
         }
         else if (pa->tag == OSXSDDecimal) {
            if (pa->subTag == OSXSDDecimal) {
               OSREAL a = *(const OSREAL*) pa->u.pValue;
               OSREAL b;

               if (pb->subTag == OSXSDDecimal)
                  b = *(const OSREAL*) pb->u.pValue;
               else if (pb->subTag == OSXSDInt)
                  b = pb->u.int32;
               else if (pb->subTag == OSXSDUInt)
                  b = pb->u.uint32;
               else
                  return pa->subTag - pb->subTag; /* dead case */

               if (a > b)
                  return 1;
               else if (a < b)
                  return -1;
               else
                  continue; /* compare next field */
            }
            else if (pa->subTag == OSXSDInt) {
               if (pb->subTag == OSXSDDecimal) {
                  OSINT32 a = pa->u.int32;
                  OSREAL b = *(const OSREAL*) pb->u.pValue;

                  if (a > b)
                     return 1;
                  else if (a < b)
                     return -1;
                  else
                     continue; /* compare next field */
               }
               else if (pb->subTag == OSXSDInt) {
                  if (pa->u.int32 == pb->u.int32)
                     continue;

                  return (int) (pa->u.int32 - pb->u.int32);
               }
               else if (pb->subTag == OSXSDUInt) {
                  if (pa->u.int32 < 0 || pb->u.uint32 >= 0x80000000u)
                     return -1;

                  result = (int) (pa->u.int32 - (OSINT32) pb->u.uint32);

                  if (result == 0)
                     continue; /* compare next field */
                  else
                     return result;
               }
            }
            else if (pa->subTag == OSXSDUInt) {
               if (pb->subTag == OSXSDDecimal) {
                  OSUINT32 a = pa->u.uint32;
                  OSREAL b = *(const OSREAL*) pb->u.pValue;

                  if (a > b)
                     return 1;
                  else if (a < b)
                     return -1;
                  else
                     continue; /* compare next field */
               }
               else if (pb->subTag == OSXSDInt) {
                  if (pb->u.int32 < 0 || pa->u.uint32 >= 0x80000000u)
                     return 1;

                  result = (int) ((OSINT32) pa->u.uint32 - pb->u.int32);

                  if (result == 0)
                     continue; /* compare next field */
                  else
                     return result;
               }
               else if (pb->subTag == OSXSDUInt) {
                  OSUINT32 a = pa->u.uint32;
                  OSUINT32 b = pb->u.uint32;

                  if (a > b)
                     return 1;
                  else if (a < b)
                     return -1;
                  else
                     continue; /* compare next field */
               }
            }
         }
         else {
            /* other types */
         }
      }
      else
         return pa->tag - pb->tag;
   }

   return 0;
}


static OSBOOL checkKey (OSXSDKeyArray* pArray)
{
   unsigned beg = 1;
   unsigned end = pArray->count;
   OSUINT32 nmFields = pArray->nmFields;
   OSXSDKeyRecord* pa = pArray->data;

   while (beg <= end) {
      unsigned mid = (beg + end) / 2;
      OSXSDKeyRecord* pb = pArray->data + mid * nmFields;
      int res = compareFields (pa, pb, nmFields);

      if (res > 0) beg = mid + 1;
      else if (res < 0) end = mid - 1;
      else
         return TRUE;
   }

   return FALSE;
}

static int expand (OSCTXT* pctxt, OSXSDKeyArray* pArray) {
   OSXSDKeyRecord* p = pArray->data;

   if (pArray->capacity < 256)
      pArray->capacity *= 2;
   else
      pArray->capacity += 256;

   pArray->data =
      rtxMemReallocArray (pctxt, pArray->data,
         (pArray->capacity + 1) * pArray->nmFields, OSXSDKeyRecord);

   if (0 == pArray->data) {
      rtxMemFreeArray (pctxt, p);
      pArray->capacity = 0;
      return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   return 0;
}

static void addErrParms (OSCTXT* pctxt, OSXSDKeyArray* pArray)
{
   OSUINT32 i;

   rtxErrAddStrParm (pctxt, pArray->name);

   for (i = 0; i < 4; i++) {
      if (i == 3 && pArray->nmFields > 4)
         rtxErrAddStrParm (pctxt, "...");
      else if (i >= pArray->nmFields)
         rtxErrAddStrParm (pctxt, "");
      else {
         OSXSDKeyRecord* p = pArray->data + i;

         if (p->tag == OSXSDString)
            rtxErrAddStrParm (pctxt, (const char*) p->u.pValue);
         else if (p->tag == OSXSDDecimal) {
            if (p->subTag == OSXSDDecimal)
               rtxErrAddDoubleParm (pctxt, *(OSREAL*) p->u.pValue);
            else if (p->subTag == OSXSDInt)
               rtxErrAddIntParm (pctxt, p->u.int32);
            else if (p->subTag == OSXSDUInt)
               rtxErrAddUIntParm (pctxt, p->u.uint32);
            else
               rtxErrAddStrParm (pctxt, "???");
         }
         else
            rtxErrAddStrParm (pctxt, "???");
      }
   }
}

/* This is invoked when all fields for a key have been set.  It relocates the
data for the key to the correct, sorted location.
*/
static int addKey (OSCTXT* pctxt, OSXSDKeyArray* pArray)
{
   unsigned beg = 1;
   unsigned end = pArray->count;
   OSUINT32 nmFields = pArray->nmFields;
   OSXSDKeyRecord* pa = pArray->data;

   while (beg <= end) {
      unsigned mid = (beg + end) / 2;
      OSXSDKeyRecord* pb = pArray->data + mid * nmFields;
      int res = compareFields (pa, pb, nmFields);

      if (res > 0) beg = mid + 1;
      else if (res < 0) end = mid - 1;
      else {
         addErrParms (pctxt, pArray);
         return LOG_RTERR (pctxt, XML_E_DUPLKEY);
      }
   }

   /*  The new key should be located prior to key at beg (which does not exist */
   /*  if the new key should be the last key), which means we'll stick it in  */
   /*  that slot after shifting everything right (if there is anything to shift). */

   if (pArray->count == pArray->capacity) {
      int stat = expand (pctxt, pArray);
      if (stat < 0)
         return LOG_RTERR (pctxt, stat);
   }

   pa = pArray->data + beg * nmFields;

   /* By assumption there is enough room to shift the keys. */
   if (beg <= pArray->count)
      OSCRTLMEMMOVE (pa + nmFields, pa,
         sizeof (OSXSDKeyRecord) * nmFields * (pArray->count + 1 - beg));

   OSCRTLSAFEMEMCPY (pa, sizeof(OSXSDKeyRecord) * nmFields,
         pArray->data, sizeof (OSXSDKeyRecord) * nmFields);

   pArray->count++;

   return 0;
}

void rtXmlKeyArraySetString
(OSXSDKeyArray* pArray, const OSUTF8CHAR* pValue, OSUINT32 fldNum)
{
   if (pArray->curField < fldNum) {
      /* absent field. unique - skip, key - error */
      pArray->absentField = (OSINT8) pArray->curField;
   }
   else if (pArray->curField > fldNum) {
      /* duplicate field */
      pArray->duplicateField = (OSINT8) fldNum;
   }
   else {
      OSXSDKeyRecord* p = pArray->data + pArray->curField;

      p->tag = (OSINT8) OSXSDString;
      p->subTag = 0;
      p->u.pValue = (const OSOCTET*) pValue;
      pArray->curField++;
   }
}

void rtXmlKeyArraySetInt
(OSXSDKeyArray* pArray, OSINT32 value, OSUINT32 fldNum)
{
   if (pArray->curField < fldNum) {
      /* absent field. unique - skip, key - error */
      pArray->absentField = (OSINT8) pArray->curField;
   }
   else if (pArray->curField > fldNum) {
      /* duplicate field */
      pArray->duplicateField = (OSINT8) fldNum;
   }
   else {
      OSXSDKeyRecord* p = pArray->data + pArray->curField;

      p->tag = (OSINT8) OSXSDDecimal;
      p->subTag = (OSINT8) OSXSDInt;
      p->u.int32 = value;
      pArray->curField++;
   }
}

void rtXmlKeyArraySetUInt
(OSXSDKeyArray* pArray, OSUINT32 value, OSUINT32 fldNum)
{
   if (pArray->curField < fldNum) {
      /* absent field. unique - skip, key - error */
      pArray->absentField = (OSINT8) pArray->curField;
   }
   else if (pArray->curField > fldNum) {
      /* duplicate field */
      pArray->duplicateField = (OSINT8) fldNum;
   }
   else {
      OSXSDKeyRecord* p = pArray->data + pArray->curField;

      p->tag = (OSINT8) OSXSDDecimal;
      p->subTag = (OSINT8) OSXSDUInt;
      p->u.int32 = value;
      pArray->curField++;
   }
}

void rtXmlKeyArraySetDecimal
(OSXSDKeyArray* pArray, const OSREAL* pValue, OSUINT32 fldNum)
{
   if (pArray->curField < fldNum) {
      /* absent field. unique - skip, key - error */
      pArray->absentField = (OSINT8) pArray->curField;
   }
   else if (pArray->curField > fldNum) {
      /* duplicate field */
      pArray->duplicateField = (OSINT8) fldNum;
   }
   else {
      OSXSDKeyRecord* p = pArray->data + pArray->curField;

      p->tag = (OSINT8) OSXSDDecimal;
      p->subTag = (OSINT8) OSXSDDecimal;
      p->u.pValue = (const OSOCTET*) pValue;
      pArray->curField++;
   }
}

int rtXmlKeyArrayAdd
(OSCTXT* pctxt, OSXSDKeyArray* pArray)
{
   int stat = 0;

   if (pArray->duplicateField != -1) {
      rtxErrAddIntParm (pctxt, pArray->duplicateField);
      rtxErrAddStrParm (pctxt, pArray->name);
      stat = LOG_RTERR (pctxt, XML_E_DUPLFLD);
   }
   else if ((pArray->absentField != -1 || pArray->curField == 0) &&
            pArray->keyConstraint)
   {
      rtxErrAddIntParm (pctxt,
                        (pArray->absentField < 0) ? 0 : pArray->absentField);
      rtxErrAddStrParm (pctxt, pArray->name);
      stat = LOG_RTERR (pctxt, XML_E_FLDABSENT);
   }
   else if (pArray->absentField == -1)
      stat = addKey (pctxt, pArray);

   pArray->curField = 0;
   pArray->duplicateField = -1;
   pArray->absentField = -1;

   return stat;
}

int rtXmlKeyArrayContains
(OSCTXT* pctxt, OSXSDKeyArray* pArray)
{
   int stat = 0;

   if (pArray->duplicateField != -1) {
      rtxErrAddIntParm (pctxt, pArray->duplicateField);
      rtxErrAddStrParm (pctxt, pArray->name);
      stat = LOG_RTERR (pctxt, XML_E_DUPLFLD);
   }
   else if (!pArray->absentField != -1) {
      if (!checkKey (pArray)) {
         addErrParms (pctxt, pArray);
         stat = LOG_RTERR (pctxt, XML_E_KEYNOTFOU);
      }
   }

   pArray->curField = 0;
   pArray->duplicateField = -1;
   pArray->absentField = -1;

   return stat;
}

