/**
 * @file rtXmlLibrary.hh
 */

#ifndef _XERXMLLIBRARY_HH_
#define _XERXMLLIBRARY_HH_

#ifdef WIN32
// Windows header file
#include <windows.h>

#define LOADLIB(name)             LoadLibrary (name)
#define GETLIBPROC(lib,func,type) ((type)GetProcAddress (lib, func))
#define CLOSELIB(lib)             FreeLibrary (lib)

typedef HMODULE LIBHANDLE;

#else
// UNIX header file
#include <dlfcn.h>

#define LOADLIB(name)             dlopen (name, RTLD_NOW)
#define GETLIBPROC(lib,func,type) ((type)dlsym (lib, func))
#define CLOSELIB(lib)             dlclose (lib)

typedef void* LIBHANDLE;
#endif
#include "osrtxml.h"

extern LIBHANDLE rtXmlLib;

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*GetXMLFactoryProc)();
extern GetXMLFactoryProc getFactory;

#ifdef __cplusplus
}
#endif

#ifdef WIN32
#define DEFAULT_SAX_LIBRARY "expat"
#else
#define DEFAULT_SAX_LIBRARY "libexpat"
#endif

#endif /* _XERXMLLIBRARY_HH_ */
