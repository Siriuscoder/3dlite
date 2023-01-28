/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
*
*	Lite3D is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	Lite3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
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

#   if !defined(_MSC_VER) && !defined(__GNUC__) && !defined(RC_INVOKED)
#       error "GCC or MSVC compiller requred.."
#   endif

#   pragma warning(disable:4244)

#   ifdef WIN_3DLITE_DLL
// When making the DLL, export tagged symbols, so they appear
// in the import library.
#   define LITE3D_EXPORT __declspec(dllexport)
#   define LITE3D_CLASS_EXPORT LITE3D_EXPORT
#   else
// We must be _using_ the DLL, so import symbols instead.
#   define LITE3D_EXPORT
#   define LITE3D_CLASS_EXPORT __declspec(dllimport)
#   endif

#   define LITE3D_STRUCT_PACKED(x) __declspec(align(x))
#   define LITE3D_INLINE __inline
#   define LITE3D_DEVIL_CALL __stdcall

#   ifdef near
#   undef near
#   endif

#   ifdef far
#   undef far
#   endif

#elif PLATFORM_Linux
    // If not Windows, we assume some sort of Unixy build environment,
    // where autotools is used.  (This includes Cygwin!)  #include the
    // config.h file only if this file was included from a non-header
    // file, because headers must not be dependent on config.h.
#   if !defined(__GNUC__)
#       error "GCC compiler requred.."
#   endif

#   define LITE3D_EXPORT
#   define LITE3D_CLASS_EXPORT
#   define LITE3D_STRUCT_PACKED(x) __attribute__ ((aligned(x)));
#   define LITE3D_INLINE inline
#   define LITE3D_DEVIL_CALL
#else
#   error "Unknown target platform"
#endif

#ifdef __GNUC__
#   define LITE3D_COMPILER_VERSION "GCC " STR(__GNUC__) "." \
    STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#   define LITE3D_CURRENT_FUNCTION __func__
#elif defined(_MSC_VER)
#   define LITE3D_COMPILER_VERSION "MSC " STR(_MSC_FULL_VER)
#   define LITE3D_CURRENT_FUNCTION __FUNCTION__
#else
#   define LITE3D_COMPILER_VERSION "UNKNOWN"
#   define LITE3D_CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#define LITE3D_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define LITE3D_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define LITE3D_CEXPORT LITE3D_EXTERN LITE3D_EXPORT
#define LITE3D_CPPEXPORT LITE3D_EXTERN LITE3D_CLASS_EXPORT

#define LITE3D_VERSION_MAJ         1
#define LITE3D_VERSION_MIN         0
#define LITE3D_VERSION_PCH         7

#define LITE3D_VERSION_STRING      STR(LITE3D_VERSION_MAJ) "." STR(LITE3D_VERSION_MIN) \
    "." STR(LITE3D_VERSION_PCH)
#define LITE3D_FULL_VERSION        LITE3D_VERSION_STRING " at " __DATE__ " " __TIME__ \
    " " LITE3D_COMPILER_VERSION
#define LITE3D_VERSION_NUM         (uint32_t)((((uint32_t)LITE3D_VERSION_MAJ) << 16) | \
    (((uint32_t)LITE3D_VERSION_MIN) << 8) | ((uint32_t)LITE3D_VERSION_PCH))

#define LITE3D_GET_VERSION_MAJ(version) ((uint8_t)((version >> 16) & 0xff))
#define LITE3D_GET_VERSION_MIN(version) ((uint8_t)((version >> 8) & 0xff))
#define LITE3D_GET_VERSION_PCH(version) ((uint8_t)((version) & 0xff))

#define LITE3D_MAX_FILE_PATH    1024
#define LITE3D_MAX_FILE_NAME    128
#define LITE3D_MAX_METRIC_NAME  128
#define LITE3D_MAX_NAME         LITE3D_MAX_FILE_NAME

#define LITE3D_TRUE         1
#define LITE3D_FALSE        0

#endif	/* COMMON_H */

