/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.hh"

EXTXMLMETHOD OSBOOL rtXmlCmpQName
(const OSUTF8CHAR* qname1, const OSUTF8CHAR* name2, const OSUTF8CHAR* nsPrefix2)
{
   const OSUTF8CHAR* lname = qname1;

   if (nsPrefix2 != 0) {
      const OSUTF8CHAR* prefixPtr = nsPrefix2;

      /* compare prefixes first */
      while ((*lname != 0) && (*lname != ':') && (*prefixPtr != 0)) {
         if (*lname != (OSUTF8CHAR)*prefixPtr)
            return FALSE;
         lname++;
         prefixPtr++;
      }
      if (*lname == ':') lname++;
   }
   else {
      const OSUTF8CHAR* _lname = lname;

      /* skip prefix in qname */
      while ((*lname != 0) && (*lname != ':'))
         lname++;
      if (*lname == ':') lname++;
      else if (*lname == 0) lname = _lname; /* no prefix - restore ptr */
   }
   /* compare names */
   if (*lname != 0) {
      const OSUTF8CHAR* name2Ptr = name2;

      while ((*lname != 0) && (*name2Ptr != ':')) {
         if (*lname != (OSUTF8CHAR)*name2Ptr)
            return FALSE;
         lname++;
         name2Ptr++;
      }
      return (OSBOOL)(*lname == *name2Ptr);
   }

   return FALSE;
}

