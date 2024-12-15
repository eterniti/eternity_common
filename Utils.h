// TODO: file too bulky, split it into subfiles and include all parts here

#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>

#ifdef QT_VERSION
#include <QString>
#endif

#include <stdint.h>

#include <string>
#include <sstream>
#include <vector>
#include <math.h>

#include "tinyxml/tinyxml.h"
#include "debug.h"

#ifndef NO_ZLIB
#include <zlib.h>
#else
#define Z_DEFAULT_COMPRESSION (-1)
#endif

#define UNUSED(x) (void)(x)

#ifdef _MSC_VER

#define FUNCNAME    __FUNCSIG__

// WARNING: not safe
#define snprintf    _snprintf
#include "vs/dirent.h"

#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#include <direct.h>

#else

#include <dirent.h>
#define FUNCNAME    __PRETTY_FUNCTION__

#define _rmdir rmdir

#endif

#ifdef  _MSC_VER
#define fseeko fseek
#define ftello ftell
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#define off64_t int64_t
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define CPU_X86_64 1
#elif defined(_M_X86) || defined(__i386__)
#define CPU_X86 1
#endif

#include "UtilsFS.h"
#include "UtilsStr.h"
#include "UtilsXML.h"
#include "UtilsCrypto.h"
#include "UtilsZlib.h"
#include "UtilsMisc.h"

#ifdef QT_VERSION
#include "UtilsQt.h"
#endif


#endif // __UTILS_H__
