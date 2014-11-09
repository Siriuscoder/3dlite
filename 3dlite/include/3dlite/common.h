/* 
 * File:   common.h
 * Author: sirius
 *
 * Created on 9 Ноябрь 2014 г., 14:04
 */

#ifndef COMMON_H
#define	COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef	__cplusplus
#   define LITE3D_EXTERN extern "C" 
#else
#   define LITE3D_EXTERN
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef PLATFORM_Windows

// Stuff for Visual C++ only
#   if defined(_MSC_VER)
// Disable whining about using 'this' as a member initializer on VC++.
#		pragma warning(disable: 4355)
#   endif


#   ifdef _WINDLL
		// When making the DLL, export tagged symbols, so they appear
		// in the import library.
#	define LITE3D_EXPORT __declspec(dllexport)
#   elif !defined(_WINDLL)
		// We must be _using_ the DLL, so import symbols instead.
#	define LITE3D_EXPORT __declspec(dllimport)
#   endif

#elif PLATFORM_Linux
	// If not Windows, we assume some sort of Unixy build environment,
	// where autotools is used.  (This includes Cygwin!)  #include the
	// config.h file only if this file was included from a non-header
	// file, because headers must not be dependent on config.h.

#   define LITE3D_EXPORT
#endif

#ifdef __GNUC__
#   define COMPILER_VERSION "GCC " STR(__GNUC__) "." \
    STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__) 
#elif defined(_MSC_VER)
#   define COMPILER_VERSION "_MSC_"
#else
#   define COMPILER_VERSION "UNKNOWN"
#endif

#define LITE3D_CEXPORT  LITE3D_EXPORT LITE3D_EXTERN

#define VERSION_MAJ         0
#define VERSION_MIN         0
#define VERSION_PCH         1

#define VERSION_STRING      STR(VERSION_MAJ) "." STR(VERSION_MIN) \
    "." STR(VERSION_PCH)
#define FULL_VERSION        VERSION_STRING " at " __DATE__ " " __TIME__ \
    " " COMPILER_VERSION
#define VERSION_NUM         (uint32_t)((((uint32_t)VERSION_MAJ) << 16) | \
    (((uint32_t)VERSION_MIN) << 8) | ((uint32_t)VERSION_PCH))


#endif	/* COMMON_H */

