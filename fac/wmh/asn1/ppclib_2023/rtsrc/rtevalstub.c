#ifndef _BUILD_LICOBJ
#define _BUILD_LICOBJ
#endif
#include "rtxsrc/rtxContext.hh"

#ifndef _NO_LICENSE_CHECK
int /* rtCheckLicense */ rtxCL (OSCTXT* pCtxt, int bits)
{
   return 0;
}

#if 0
int rtInitAndCheckLicense (OSCTXT* pCtxt, int bits)
{
   return rtxInitContext (pCtxt);
}
#endif
#endif
