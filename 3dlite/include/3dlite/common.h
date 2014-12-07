/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
*
*	Foobar is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	Foobar is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#ifndef LITE3D_COMMON_H
#define	LITE3D_COMMON_H

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

#   if !defined(_MSC_VER) && !defined(__GNUC__)
#       error "GCC or MSVC compiller requred.."
#   endif

// Stuff for Visual C++ only
#   if defined(_MSC_VER)
// Disable whining about using 'this' as a member initializer on VC++.
#       pragma warning(disable: 4355)
#       pragma warning(disable: 4127)
#   endif


#   ifdef _WINDLL
// When making the DLL, export tagged symbols, so they appear
// in the import library.
#   define LITE3D_EXPORT __declspec(dllexport)
#   define LITE3D_CLASS_EXPORT LITE3D_EXPORT
#   elif !defined(_WINDLL)
        // We must be _using_ the DLL, so import symbols instead.
#   define LITE3D_EXPORT
#   define LITE3D_CLASS_EXPORT __declspec(dllimport)
#   endif

#   define STRUCT_PACKED(x) __declspec(align(x))
#   define INLINE __inline

#elif PLATFORM_Linux
    // If not Windows, we assume some sort of Unixy build environment,
    // where autotools is used.  (This includes Cygwin!)  #include the
    // config.h file only if this file was included from a non-header
    // file, because headers must not be dependent on config.h.
#   if !defined(__GNUC__)
#       error "GCC compiller requred.."
#   endif

#   define LITE3D_EXPORT
#   define LITE3D_CLASS_EXPORT
#   define STRUCT_PACKED(x) __attribute__ ((aligned(x)));
#   define INLINE inline
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
#define LITE3D_CPPEXPORT  LITE3D_CLASS_EXPORT LITE3D_EXTERN

#define VERSION_MAJ         0
#define VERSION_MIN         0
#define VERSION_PCH         1

#define VERSION_STRING      STR(VERSION_MAJ) "." STR(VERSION_MIN) \
    "." STR(VERSION_PCH)
#define FULL_VERSION        VERSION_STRING " at " __DATE__ " " __TIME__ \
    " " COMPILER_VERSION
#define VERSION_NUM         (uint32_t)((((uint32_t)VERSION_MAJ) << 16) | \
    (((uint32_t)VERSION_MIN) << 8) | ((uint32_t)VERSION_PCH))


#define LITE3D_MAX_FILE_PATH    1024
#define LITE3D_MAX_FILE_NAME    128
#define LITE3D_MAX_NAME         LITE3D_MAX_FILE_NAME

#endif	/* COMMON_H */

