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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxCharStr.h"

const char* berTagToString (ASN1TAG tag, char* buffer, size_t bufsiz)
{
   OSUINT32 idmask = 0xFFFFFFFF >> (((sizeof(ASN1TAG) - 2) * 8) + 3);
   OSUINT32 idcode = ((OSUINT32)tag) & idmask;
   ASN1TAG  tclass = tag & TM_PRIV;
   const char* classStr;
   char numbuf[20];

   switch (tclass) {
   case TM_UNIV: classStr = "UNIVERSAL "; break;
   case TM_APPL: classStr = "APPLICATION "; break;
   case TM_CTXT: classStr = ""; break;
   case TM_PRIV: classStr = "PRIVATE "; break;
   default:      classStr = "??? "; break;
   }
   rtxUIntToCharStr (idcode, numbuf, sizeof(numbuf), '\0');

   return rtxStrJoin (buffer, bufsiz, "[", classStr, numbuf, "]", 0);
}

const char* berTagToDynStr (OSCTXT* pctxt, ASN1TAG tag)
{
   char lbuf[32];
   berTagToString (tag, lbuf, sizeof(lbuf));
   return rtxStrdup (pctxt, lbuf);
}

const char* berTagToTypeName (ASN1TAG tag)
{
   ASN1TAG tclass = tag & TM_PRIV;

   if (tclass == TM_UNIV) {
      OSUINT32 idcode = tag & TM_IDCODE;
      switch (idcode) {
      case 0: return "EOC";
      case 1: return "BOOLEAN";
      case 2: return "INTEGER";
      case 3: return "BIT STRING";
      case 4: return "OCTET STRING";
      case 5: return "NULL";
      case 6: return "OBJECT IDENTIFIER";
      case 7: return "ObjectDescriptor";
      case 8: return "EXTERNAL";
      case 9: return "REAL";
      case 10: return "ENUMERATED";
      case 11: return "EMBEDDED PDV";
      case 12: return "UTF8String";
      case 13: return "RELATIVE-OID";
      case 14: return "TIME";
      case 16: return "SEQUENCE";
      case 17: return "SET";
      case 18: return "NumericString";
      case 19: return "PrintableString";
      case 20: return "T61String";
      case 21: return "VideotexString";
      case 22: return "IA5String";
      case 23: return "UTCTime";
      case 24: return "GeneralizedTime";
      case 25: return "GraphicString";
      case 26: return "VisibleString";
      case 27: return "GeneralString";
      case 28: return "UniversalString";
      case 30: return "BMPString";
      case 31: return "DATE";
      case 32: return "TIME-OF-DAY";
      case 33: return "DATE-TIME";
      case 34: return "DURATION";
      case 35: return "OID-IRI";
      default:;
      }
   }

   return 0;
}
