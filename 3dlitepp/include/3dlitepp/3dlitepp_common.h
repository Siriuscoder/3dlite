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
#pragma once

#include <stdint.h>
#include <stddef.h>

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


#   ifdef WIN_3DLITEPP_DLL
// When making the DLL, export tagged symbols, so they appear
// in the import library.
#   define LITE3DPP_EXPORT __declspec(dllexport)
#   elif !defined(WIN_3DLITEPP_DLL)
        // We must be _using_ the DLL, so import symbols instead.
#   define LITE3DPP_EXPORT __declspec(dllimport)
#   endif

#   define INLINE __inline
#   define DEVIL_CALL __stdcall

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
#       error "GCC compiller requred.."
#   endif

#   define LITE3D_EXPORT
#   define LITE3D_CLASS_EXPORT

#   define INLINE inline
#   define DEVIL_CALL
#endif
