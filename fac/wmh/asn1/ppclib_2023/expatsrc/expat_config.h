
#ifndef EXPAT_CONFIG_H
#define EXPAT_CONFIG_H

#include "autosense.h"
#if !defined(NIOS2) && !defined(__thumb__) && !defined(TELIUM) && !defined(ARM_CORTEX_M0) && !defined(XTENSA_TFUSION)
#include <memory.h>
#endif
#include <string.h>

#ifndef XML_NS
#define XML_NS
#endif

#ifdef XML_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

/* we will assume all Windows platforms are little endian */

#define BYTEORDER 1234

#ifdef XML_BUILDDLL
#define XMLIMPORT __declspec(dllexport)
#endif /* XML_BUILDDLL */

#ifdef XML_USEDLL
#define XMLIMPORT __declspec(dllimport)
#endif /* XML_BUILDDLL */
#endif

#if defined(XML_SOLARIS)
#define BYTEORDER 4321
#endif

#if defined(XML_AIX)
#define BYTEORDER 4321
#endif

#if defined(XML_HPUX)
#define BYTEORDER 4321
#endif

#if defined(XML_LINUX)
#define BYTEORDER 1234
#endif

#if defined(XML_MACOS) || defined(XML_MACOSX)
#include "macconfig.h"
#endif

#if defined(XML_AS400)
#define BYTEORDER 4321
#endif

#if defined(XML_TRU64)
#define BYTEORDER 1234
#endif

#if defined(__PPC__)  /*!AB: Power PC */
#ifdef BYTEORDER
#undef BYTEORDER
#endif
#define BYTEORDER 4321
#endif

/*#define XML_DTD 1*/

/* Windows has memmove() available. */

#define HAVE_MEMMOVE

#ifndef BYTEORDER
/* by default, use little-endian */
#define BYTEORDER 1234
#endif

#endif /* ndef EXPAT_CONFIG_H */
