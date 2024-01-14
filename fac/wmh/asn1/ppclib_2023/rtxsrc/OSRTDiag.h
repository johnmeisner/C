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

#ifndef _OSRTDIAG_H_
#define _OSRTDIAG_H_

#include <stdarg.h>
#include "rtxsrc/rtxDiag.h"

class EXTRTCLASS OSRTDiag {
 public:
   void printf (const char* fmtspec, ...);
} ;

// An object of this class can be used to turn diag tracing on for the duration
// of a method.  Tracing will be switched back to its original state when the
// object is destroyed or goes out-of-scope.

class EXTRTCLASS OSRTDiagGuard {
 protected:
   OSCTXT* mpCtxt;
   OSBOOL mPrevState;
 public:
   OSRTDiagGuard (OSCTXT* pctxt) : mpCtxt(pctxt) {
      mPrevState = rtxSetDiag (mpCtxt, TRUE);
   }
   ~OSRTDiagGuard() {
      rtxSetDiag (mpCtxt, mPrevState);
   }
} ;

#ifdef _DEBUG
#define OSRTDIAGPRT(objptr,msg) if (objptr) objptr->printf(msg)
#define OSRTDIAGPRT1(objptr,msg,a) if (objptr) objptr->printf(msg,a)
#define OSRTDIAGPRT2(objptr,msg,a,b) if (objptr) objptr->printf(msg,a,b)
#define OSRTDIAGPRT3(objptr,msg,a,b,c) if (objptr) objptr->printf(msg,a,b,c)
#define OSRTDIAGPRT4(objptr,msg,a,b,c,d) if (objptr) objptr->printf(msg,a,b,c,d)
#else
#define OSRTDIAGPRT(objptr,msg)
#define OSRTDIAGPRT1(objptr,msg,a)
#define OSRTDIAGPRT2(objptr,msg,a,b)
#define OSRTDIAGPRT3(objptr,msg,a,b,c)
#define OSRTDIAGPRT4(objptr,msg,a,b,c,d)
#endif

#endif
