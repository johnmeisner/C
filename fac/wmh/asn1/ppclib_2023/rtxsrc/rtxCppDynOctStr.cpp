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

#include "rtxsrc/rtxBase64.h"
#include "rtxsrc/rtxCppDynOctStr.h"

OSDynOctStrClass::OSDynOctStrClass ()
{
   numocts = 0; data = NULL;
}


OSDynOctStrClass::OSDynOctStrClass (OSSIZE numocts_, const OSOCTET* data_)
{
   this->numocts = 0;
   this->data = NULL;
   copyValue(numocts_, data_);
}

OSDynOctStrClass::OSDynOctStrClass (OSDynOctStr& os)
{
   numocts = 0;
   data = NULL;
   copyValue(os.numocts, os.data);
}


OSDynOctStrClass::OSDynOctStrClass (OSDynOctStr64& os)
{
   numocts = 0;
   data = NULL;
   copyValue(os.numocts, os.data);
}


OSDynOctStrClass::OSDynOctStrClass (const OSDynOctStrClass& os) :
   OSRTBaseType (os)
{
   numocts = 0;
   data = NULL;
   copyValue (os.numocts, os.data);
}

OSDynOctStrClass::~OSDynOctStrClass ()
{
   delete [] data;
}

void OSDynOctStrClass::copyValue (OSSIZE numocts_, const OSOCTET* data_)
{
   if (0 != data) delete [] data;
   numocts = numocts_;
   OSOCTET* newdata = new OSOCTET [numocts];
   if (0 != newdata)
      memcpy (newdata, data_, numocts);
   data = newdata;
}

void OSDynOctStrClass::setValue (OSSIZE numocts_, const OSOCTET* data_)
{
   copyValue(numocts_, data_);
}

int OSDynOctStrClass::setValue (const char* hexstr, size_t nchars)
{
   if (0 == nchars) nchars = strlen (hexstr);
   if (0 != data) delete [] data;

   int stat = rtxHexCharsToBinCount (hexstr, nchars);
   if (stat < 0) {
      numocts = 0; data = 0;
      return stat;
   }
   else numocts = stat;

   data = new OSOCTET [numocts];
   if (0 == data) return RTERR_NOMEM;

   stat = rtxHexCharsToBin (hexstr, nchars, data, numocts);
   if (stat < 0) {
      delete [] data;
      numocts = 0; data = 0;
      return stat;
   }

   return 0;
}

int OSDynOctStrClass::setValueFromBase64 (const char* base64str, size_t nchars)
{
   if (0 == nchars) nchars = strlen (base64str);
   if (0 != data) delete [] data;

   long lstat = rtxBase64GetBinDataLen (base64str, nchars);
   if (lstat < 0) {
      numocts = 0; data = 0;
      return (int) lstat;
   }
   else numocts = (OSSIZE) lstat;

   data = new OSOCTET [numocts];
   if (0 == data) return RTERR_NOMEM;

   int stat = rtxBase64DecodeDataToFSB (0, base64str, nchars, data, numocts);
   if (stat < 0) {
      delete [] data;
      numocts = 0; data = 0;
      return stat;
   }

   return 0;
}

OSDynOctStrClass& OSDynOctStrClass::operator= (const OSDynOctStrClass& orig)
{
   // test for assignment to self
   if (this != &orig)
      copyValue (orig.numocts, orig.data);

   return *this;
}

