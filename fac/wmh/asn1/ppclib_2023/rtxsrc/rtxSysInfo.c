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

#ifdef _MSC_VER
#include <process.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>

#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxSysInfo.h"

EXTRTMETHOD int rtxGetPID ()
{
#ifdef _MSC_VER
   return (int)_getpid();
#else
   return getpid();
#endif
}

#ifndef _WIN32_WCE
EXTRTMETHOD char* rtxEnvVarDup (const char* name)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400) /* At least Visual Studio 2005 */
   char* buf = 0;
   size_t sz = 0;
   if (_dupenv_s(&buf, &sz, name) == 0) {
      return buf;
   }
#else
   const char* value = getenv(name);
   if (0 != value) {
      size_t bufsize = OSCRTLSTRLEN(value) + 1;
      char* buf = OSCRTLMALLOC (bufsize);
      return (0 == buf) ? 0 : rtxStrcpy (buf, bufsize, value);
   }
#endif
   return 0;
}

EXTRTMETHOD OSBOOL rtxEnvVarIsSet (const char* name)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400) /* At least Visual Studio 2005 */
   char* buf = 0;
   size_t sz = 0;
   if (_dupenv_s(&buf, &sz, name) == 0) {
      if (0 != buf) {
         free (buf);
         return TRUE;
      }
   }
   return FALSE;
#else
   return (OSBOOL)(0 != getenv(name));
#endif
}

#ifdef _MSC_VER
#if _MSC_VER < 1400 /* Visual Studio version < VS2005 */
#define USE_PUTENV
#endif
#else
#ifdef __hpux /* HP-UX aCC/gcc */
#define USE_PUTENV
#else
#ifdef __MINGW32__
#define USE_PUTENV
#endif /* MinGW */
#endif /* HP-UX aCC */
#endif /* _MSC_VER */

#ifdef USE_PUTENV
static int doPutenv (const char* name, const char* value)
{
   OSSIZE strlens = OSCRTLSTRLEN(name) + OSCRTLSTRLEN(value) + 2;
   char *pbuf = (char *)malloc(strlens);
   int stat = 0;

   if (0 != pbuf) {
      os_snprintf (pbuf, strlens, "%s=%s", name, value);
#ifdef _MSC_VER
      stat = _putenv (pbuf);
#else
      stat = putenv (pbuf);
#endif
      free (pbuf);
   }
   else stat = RTERR_NOMEM;

   return stat;
}
#endif

EXTRTMETHOD int rtxEnvVarSet
(const char* name, const char* value, int overwrite)
{
#ifdef _MSC_VER
#if _MSC_VER >= 1400 /* At least Visual Studio 2005 */
   return _putenv_s(name, value);
#else
   return doPutenv (name, value);
#endif
#else /* Not Microsoft */
#ifdef USE_PUTENV
   return doPutenv (name, value);
#else
   return setenv (name, value, overwrite);
#endif /* USE_PUTENV */
#endif /* _MSC_VER */
}
#endif /* _WIN32_WCE */
