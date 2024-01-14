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

/* Run-time print utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/rtPrint.h"
#include "rtsrc/rtPrintToStream.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxCharStr.h"

#ifndef BITSTR_BYTES_IN_LINE
#define BITSTR_BYTES_IN_LINE 16
#endif

static int rtBitStringDumpToList (OSCTXT *pctxt, OSSIZE numbits,
                                  const OSOCTET* data, OSRTSList* pList)
{
   OSSIZE i, numocts = numbits / 8;
   char* pbuf, *ps;

   if (0 == numbits) return 0;

   pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
   if (0 == pbuf) return RTERR_NOMEM;

   for (i = 0; i < numocts; i++) {
      if ((i != 0) && (i % BITSTR_BYTES_IN_LINE == 0)) {
         rtxSListAppend (pList, (void*)pbuf);
         pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
         if (0 == pbuf) return RTERR_NOMEM;
      }

      if (i % BITSTR_BYTES_IN_LINE != 0) {
         *ps++ = ' ';
      }
      rtxByteToHexChar (data[i], ps, 3);
      ps += 2;
   }

   *ps = '\0';

   if (i * 8 != numbits) {
      OSOCTET tm = data[i];
      OSSIZE nmBits = numbits % 8;
      OSSIZE j;
      char buff[9];

      for (j = 0; j < nmBits; j++, tm<<=1)
         buff[j] = ((tm >> 7) & 1) + '0';
      for (; j < 8; j++)
         buff[j] = 'x';

      buff[sizeof(buff)-1] = '\0';

      if ((i % BITSTR_BYTES_IN_LINE) == (BITSTR_BYTES_IN_LINE - 1)) {
         rtxSListAppend (pList, (void*)pbuf);
         pbuf = ps = (char*) rtxMemAlloc (pctxt, 81);
         if (0 == pbuf) return RTERR_NOMEM;
      }
      else if (i > 0) {
         *ps++ = ' ';
      }
      *ps = '\0';
      rtxStrcat (pbuf, 81, buff);
   }

   rtxSListAppend (pList, (void*)pbuf);

   return 0;
}

EXTRTMETHOD int rtPrintToStreamBitStr
(OSCTXT *pctxt, const char* name, OSSIZE numbits,
 const OSOCTET* data, const char* conn)
{
   OSRTSList bitStrList;
   int ret;

   rtxSListInitEx (pctxt, &bitStrList);

   ret = rtxPrintToStream (pctxt, "%s%snumbits = %u\n", name, conn, numbits);
   if (ret != 0) return ret;

   ret = rtBitStringDumpToList (pctxt, numbits, data, &bitStrList);
   if (ret != 0) return ret;

   if (numbits >= 64) {
      OSRTSListNode* pnode = bitStrList.head;
      ret = rtxPrintToStream (pctxt, "%s%sdata =\n", name, conn);
      if (ret != 0) return ret;

      while (0 != pnode) {
         ret = rtxPrintToStream (pctxt, "%s\n", (char*)pnode->data);
         if (ret != 0) break;
         else pnode = pnode->next;
      }
   }
   else if (numbits > 0) {
      ret = rtxPrintToStream (pctxt, "%s%sdata = %s\n",
                              name, conn, (char*)bitStrList.head->data);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s%sdata = NULL\n", name, conn);
   }

   rtxSListFreeAll (&bitStrList);

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrExt(OSCTXT *pctxt, const char* name,
   OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata, const char* conn)
{
   OSRTSList bitStrList;
   OSSIZE dataSizeBits = dataSize * 8;
   int ret;

   rtxSListInitEx (pctxt, &bitStrList);

   ret = rtxPrintToStream (pctxt, "%s%snumbits = %u\n", name, conn, numbits);
   if (ret != 0) return ret;

   ret = rtBitStringDumpToList (pctxt,
              (numbits > dataSizeBits) ? dataSizeBits : numbits, data,
              &bitStrList);
   if (ret != 0) return ret;

   if (numbits >= 64) {
      OSRTSListNode* pnode = bitStrList.head;
      ret = rtxPrintToStream (pctxt, "%s%sdata =\n", name, conn);
      if (ret != 0) return ret;

      while (0 != pnode) {
         ret = rtxPrintToStream (pctxt, "%s\n", (char*)pnode->data);
         if (ret != 0) break;
         else pnode = pnode->next;
      }
   }
   else if (numbits > 0) {
      ret = rtxPrintToStream (pctxt, "%s%sdata = %s\n",
                              name, conn, (char*)bitStrList.head->data);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s%sdata = NULL\n", name, conn);
   }

   if (NULL != extdata)
   {
      ret = rtxPrintToStream (pctxt, "%s%sextdata = 0x%x\n",
                              name, conn, *extdata);
      if (0 != ret)
      {
         return ret;
      }
   }

   rtxSListFreeAll (&bitStrList);

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrBraceText
(OSCTXT *pctxt, const char* name, OSSIZE numbits, const OSOCTET* data)
{
   int ret = 0;
   if (numbits > 0) {
      OSRTSListNode* pnode;
      OSRTSList bitStrList;

      rtxSListInitEx (pctxt, &bitStrList);

      ret = rtBitStringDumpToList (pctxt, numbits, data, &bitStrList);
      if (ret != 0) return ret;

      if (numbits >= 64) {
         ret = rtxPrintToStream (pctxt, "%s = { %u,\n", name, numbits);
         if (ret != 0) return ret;

         pnode = bitStrList.head;
         while (0 != pnode) {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   %s\n", (char*)pnode->data);
            if (ret != 0) break;
            else pnode = pnode->next;
         }

         rtxPrintToStreamIndent (pctxt);
         ret = rtxPrintToStream (pctxt, "}\n");
      }
      else {
         ret = rtxPrintToStream (pctxt, "%s = { %u, %s }\n",
                                 name, numbits, (char*)bitStrList.head->data);
      }

      rtxSListFreeAll (&bitStrList);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s = { 0 }\n", name);
   }

   return ret;
}

EXTRTMETHOD int rtPrintToStreamBitStrBraceTextExt(OSCTXT *pctxt,
   const char* name, OSSIZE numbits, const OSOCTET* data, OSSIZE dataSize,
   const OSOCTET* extdata)
{
   OSSIZE dataSizeBits = dataSize * 8;
   int ret;

   if (numbits > 0) {
      OSRTSListNode* pnode;
      OSRTSList bitStrList;

      rtxSListInitEx (pctxt, &bitStrList);

      ret = rtBitStringDumpToList (pctxt,
              (numbits > dataSizeBits) ? dataSizeBits : numbits,
              data, &bitStrList);
      if (ret != 0) return ret;

      if (numbits >= 64) {
         ret = rtxPrintToStream (pctxt, "%s = { %u,\n", name, numbits);
         if (ret != 0) return ret;

         pnode = bitStrList.head;
         while (0 != pnode) {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   %s\n", (char*)pnode->data);
            if (ret != 0) break;
            else pnode = pnode->next;
         }

         if (NULL != extdata)
         {
            rtxPrintToStreamIndent (pctxt);
            ret = rtxPrintToStream (pctxt, "   0x%x\n", *extdata);
            if (ret != 0) return ret;
         }

         rtxPrintToStreamIndent (pctxt);
         ret = rtxPrintToStream (pctxt, "}\n");
      }
      else {
         if (NULL != extdata)
         {
            ret = rtxPrintToStream (pctxt, "%s = { %u, %s, 0x%x }\n",
                                    name, numbits,
                                    (char *)bitStrList.head->data,
                                    *extdata);
         }
         else
         {
            ret = rtxPrintToStream (pctxt, "%s = { %u, %s }\n",
                                    name, numbits,
                                    (char *)bitStrList.head->data);
         }
      }

      rtxSListFreeAll (&bitStrList);
   }
   else {
      ret = rtxPrintToStream (pctxt, "%s = { 0 }\n", name);
   }

   return ret;
}

EXTRTMETHOD int rtPrintToStreamOctStr
(OSCTXT *pctxt, const char* name, OSSIZE numocts,
 const OSOCTET* data, const char* conn)
{
   int ret = 0;
   ret = rtxPrintToStream(pctxt, "%s%snumocts = %u\n", name, conn, numocts);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStream (pctxt, data, numocts);
   return ret;
}


EXTRTMETHOD int rtPrintToStream16BitCharStr (OSCTXT *pctxt,
   const char* name, const Asn116BitCharString* bstring, const char* conn)
{
   int ret = 0;

   ret = rtxPrintToStream(pctxt, "%s%snchars = %u\n", name, conn,
                         bstring->nchars);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStreamEx (pctxt, (OSOCTET*)bstring->data,
                        bstring->nchars * 2, 2);
   return ret;
}


EXTRTMETHOD int rtPrintToStream32BitCharStr (OSCTXT *pctxt,
   const char* name, const Asn132BitCharString* bstring, const char* conn)
{
   int ret = 0;

   ret = rtxPrintToStream(pctxt,"%s%snchars = %u\n", name, conn,
                         bstring->nchars);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStreamEx (pctxt, (OSOCTET*)bstring->data,
                        bstring->nchars * sizeof (OS32BITCHAR), 4);
   return ret;
}


EXTRTMETHOD int rtPrintToStreamOIDValue (OSCTXT *pctxt, const ASN1OBJID* pOID)
{
   OSUINT32 ui;

   rtxPrintToStream(pctxt, "{ ");
   for (ui = 0; ui < pOID->numids; ui++) {
      rtxPrintToStream(pctxt, "%d ", pOID->subid[ui]);
   }
   rtxPrintToStream(pctxt, "}\n");
   return 0;
}

EXTRTMETHOD int rtPrintToStreamOID
(OSCTXT *pctxt, const char* name, const ASN1OBJID* pOID)
{
   int ret = 0;
   ret = rtxPrintToStream(pctxt,"%s = ", name);
   if(ret != 0) return ret;
   ret = rtPrintToStreamOIDValue (pctxt, pOID);
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOID64Value (OSCTXT *pctxt, const ASN1OID64* pOID)
{
   OSINT64 ui;
   rtxPrintToStream (pctxt, "{ ");
   for (ui = 0; ui < pOID->numids; ui++) {
      rtxPrintToStream (pctxt, OSINT64FMT" " ,
                       pOID->subid[(unsigned int)ui]);
   }
   rtxPrintToStream (pctxt, "}\n");
   return 0;
}

/* OID with subid of 64 bit */
EXTRTMETHOD int rtPrintToStreamOID64
(OSCTXT *pctxt, const char* name, const ASN1OID64* pOID)
{
   int ret = 0;
   ret = rtxPrintToStream(pctxt, "%s = ", name);
   if(ret != 0) return ret;
   ret = rtPrintToStreamOID64Value (pctxt, pOID);
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOpenType
(OSCTXT *pctxt,const char* name, OSSIZE numocts,
 const OSOCTET* data, const char* conn)
{
   int ret = 0;
   ret =  rtxPrintToStream(pctxt, "%s%snumocts = %u\n", name, conn, numocts);
   if(ret != 0) return ret;
   ret = rtxPrintToStream(pctxt, "%s%sdata = \n", name, conn);
   if(ret != 0) return ret;
   rtxHexDumpToStream (pctxt, data, numocts);
   return ret;
}

EXTRTMETHOD int rtPrintToStreamOpenTypeExt
(OSCTXT *pctxt, const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;
            rtPrintToStreamOpenType (pctxt, name, pOpenType->numocts,
                                     pOpenType->data, ".");
         }
         pnode = pnode->next;
      }
   }
   return 0;
}

EXTRTMETHOD int rtPrintToStreamUnicodeCharStr
   (OSCTXT *pctxt, const char* name, const Asn116BitCharString* bstring)
{
   OSUINT32 i;
   if(rtxPrintToStream (pctxt, "%s = '", name) < 0)
      return -1;

   for (i = 0; i < bstring->nchars; i++) {
      if (OS_ISPRINT (bstring->data[i])){
         if(rtxPrintToStream (pctxt, "%c", bstring->data[i]) < 0)
            return -1;
      }
      else{
         if(rtxPrintToStream (pctxt, "0x%04x", bstring->data[i]) < 0)
            return -1;
       }
   }
   if(rtxPrintToStream (pctxt, "'\n") < 0)
      return -1;

   return 0;
}

EXTRTMETHOD int rtPrintToStreamUnivCharStr
   (OSCTXT *pctxt, const char* name, const Asn132BitCharString* bstring)
{
   OSUINT32 i;

   if(rtxPrintToStream (pctxt, "%s = '", name) < 0)
      return -1;

   for (i = 0; i < bstring->nchars; i++) {
      if (OS_ISPRINT (bstring->data[i])){
         if(rtxPrintToStream (pctxt, "%c", bstring->data[i]) < 0)
            return -1;
      }
      else{
         if(rtxPrintToStream (pctxt, "0x%08x", bstring->data[i]) < 0)
            return -1;
      }
   }
   if(rtxPrintToStream (pctxt, "'\n") < 0)
      return -1;

   return 0;
}

EXTRTMETHOD int rtPrintToStreamOpenTypeExtBraceText
   (OSCTXT *pctxt, const char* name, const OSRTDList* pElemList)
{
   ASN1OpenType* pOpenType;
   if (0 != pElemList) {
      OSRTDListNode* pnode = pElemList->head;
      while (0 != pnode) {
         if (0 != pnode->data) {
            pOpenType = (ASN1OpenType*) pnode->data;

            rtPrintToStreamIndent (pctxt);

            rtPrintToStreamHexStr
               (pctxt, name, pOpenType->numocts, pOpenType->data);
         }
         pnode = pnode->next;
      }
   }
   return 0;
}




