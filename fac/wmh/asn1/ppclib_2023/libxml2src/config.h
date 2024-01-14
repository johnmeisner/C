/* A small note:
 *
 * This header isn't win32-specific, despite the #define here.  It's provided
 * this way because the configuration scripts created it as such.  We have made
 * small modifications to include appropriate headers and define appropriate
 * thread mechanisms. */

#ifndef __LIBXML_WIN32_CONFIG__
#define __LIBXML_WIN32_CONFIG__

#define HAVE_CTYPE_H
#define HAVE_STDARG_H
#define HAVE_MALLOC_H
#define HAVE_ERRNO_H /* make sure that applications using DLLs link against
                        libxml2.lib, NOT libxml2_a.lib */

#ifdef _WIN32_WCE
#undef HAVE_ERRNO_H
#include <windows.h>
#include "wincecompat.h"
#else
#define HAVE_SYS_STAT_H
#define HAVE__STAT
#define HAVE_STAT
#define HAVE_STDLIB_H
#define HAVE_TIME_H

#define HAVE_FCNTL_H

/* direct.h and io.h give gcc systems problems. */
#if defined(_WIN32) || defined (__WIN32__)
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#endif

#include <libxml/xmlversion.h>

/* *NIX uses dlopen.  While we don't use module support in XBinder, it's
 * possible that clients of the library will; so providing the right 
 * interface is important. */
#if !defined (_WIN32) && !defined (__WIN32__) && !defined(__thumb__) && !defined(TELIUM) && !defined(ARM_CORTEX_M0)
#define HAVE_DLOPEN
#define HAVE_DLFCN_H
#endif

#ifdef NEED_SOCKETS
#include <wsockcompat.h>
#endif

#define HAVE_ISINF
#define HAVE_ISNAN
#include <math.h>
#if defined(_MSC_VER) || defined(__BORLANDC__)
/* MS C-runtime has functions which can be used in order to determine if
   a given floating-point variable contains NaN, (+-)INF. These are 
   preferred, because floating-point technology is considered propriatary
   by MS and we can assume that their functions know more about their 
   oddities than we do. */
#include <float.h>
/* Bjorn Reese figured a quite nice construct for isinf() using the _fpclass
   function. */
#ifndef isinf
#define isinf(d) ((_fpclass(d) == _FPCLASS_PINF) ? 1 \
	: ((_fpclass(d) == _FPCLASS_NINF) ? -1 : 0))
#endif
/* _isnan(x) returns nonzero if (x == NaN) and zero otherwise. */
#ifndef isnan
#define isnan(d) (_isnan(d))
#endif
#else /* _MSC_VER */

#if defined(_WIN32) || defined (__WIN32__)
#ifndef isinf
static int isinf (double d) {
    int expon = 0;
    double val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 1;
        } else if (val == -0.5) {
            return -1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
#endif
#ifndef isnan
static int isnan (double d) {
    int expon = 0;
    double val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 0;
        } else if (val == -0.5) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}
#endif
#endif /* Windows */

#endif /* _MSC_VER */

#if defined(_MSC_VER)
#define mkdir(p,m) _mkdir(p)
/* snprintf is defined in MSVC 14.0+ (VS 2015); error if defined here. */
#if (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
/* vsnprintf is defined in MSVC 9.0+ (VS 2008); error if defined here. */
#if (_MSC_VER < 1500)
#define vsnprintf(b,c,f,a) _vsnprintf(b,c,f,a)
#endif
#elif defined(__MINGW32__)
#define mkdir(p,m) _mkdir(p)
#endif

/* Threading API to use should be specified here for compatibility reasons.
   This is however best specified on the compiler's command-line. */
/* Note:  when using this config as cross platform, we only use win32 threads
 * when compiling on a win32 system. */
#if defined(LIBXML_THREAD_ENABLED) && (defined (_WIN32) || defined (__WIN32__))
#if !defined(HAVE_PTHREAD_H) && !defined(HAVE_WIN32_THREADS)
#define HAVE_WIN32_THREADS
#endif
#endif

/* Some third-party libraries far from our control assume the following
   is defined, which it is not if we don't include windows.h. */
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(TRUE)
#define TRUE (!(FALSE))
#endif

#endif /* __LIBXML_WIN32_CONFIG__ */


