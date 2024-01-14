/*
 * Copyright (c) 2016-2016 by Objective Systems, Inc.
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
/**
 * @file rtxGenValueType.h - Implementation of a generic value type for 
 * encoding and decoding values without schema.
 */
#ifndef _RTXGENVALUETYPE_H_
#define _RTXGENVALUETYPE_H_

#include "rtxsrc/rtxContext.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Generic data value type */
typedef enum {
   OSRTDataType_UnsignedInt, 
   OSRTDataType_SignedInt, 
   OSRTDataType_ByteArray, 
   OSRTDataType_CharStr, 
   OSRTDataType_Array, 
   OSRTDataType_Map,
   OSRTDataType_Bool,
   OSRTDataType_Float
} OSRTDataType;

typedef struct _OSRTGenPtrArray {
   OSSIZE nitems;
   struct _OSRTGenValue** values;
} OSRTGenPtrArray;

typedef struct _OSRTGenKeyValuePair {
   struct _OSRTGenValue* key;
   struct _OSRTGenValue* value;
} OSRTGenKeyValuePair;

typedef struct _OSRTGenValueMap {
   OSSIZE nitems;
   OSRTGenKeyValuePair* values;
} OSRTGenValueMap;

typedef struct _OSRTGenValue {
   OSRTDataType t;
   union {
      /* Unsigned and signed integer values */
#ifndef _NO_INT64_SUPPORT
      OSUINT64 uintval;
      OSINT64  intval;
#else
      OSUINT32 uintval;
      OSINT32  intval;
#endif
      /* Byte string value */
      OSDynOctStr64 bytestr;

      /* UTF-8 string value */
      const char* utf8str;

      /* Array */
      OSRTGenPtrArray array;

      /* Map */
      OSRTGenValueMap map;

      /* Boolean */
      OSBOOL boolval;

      /* Floating point */
      OSDOUBLE fltval;
   } u;
} OSRTGenValue;

/**
 * This function compares two generic values for equality.  Information 
 * on values in the structure that are not equal are returned on the 
 * error list within the context.
 *
 * @param pctxt    Pointer to a context structure.
 * @param pvalue1  Pointer to first value to compare.
 * @param pvalue2  Pointer to second value to compare.
 * @return Status of the comparison operation.  A negative value indicates
 *   the values are not equal.  Printing the error results using rtxErrPrint 
 *   or equivalent will show the specific items that don't match.
 */
EXTERNRT int rtxGenValueCompare 
(OSCTXT* pctxt, const OSRTGenValue* pvalue1, const OSRTGenValue* pvalue2);

/**
 * This function returns a textual identifier for the given enumerated
 * value type.
 *
 * @param id            Enumerated data type identifier (OSRTDataType)
 * @return              Textual identifier for id or '<unknown>'
 */
EXTERNRT const char* rtxGenValueGetIdent (OSRTDataType id);

#ifdef __cplusplus
}
#endif

#endif
