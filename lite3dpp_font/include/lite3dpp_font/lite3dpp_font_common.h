/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2016  Sirius (Korolev Nikita)
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
#pragma once

#include <lite3d/lite3d_common.h>

#ifdef PLATFORM_Windows

#   if !defined(_MSC_VER) && !defined(__GNUC__)
#       error "GCC or MSVC compiller requred.."
#   endif

#   ifdef WIN_3DLITEPP_FONT_DLL
// When making the DLL, export tagged symbols, so they appear
// in the import library.
#   define LITE3DPP_FONT_EXPORT __declspec(dllexport)
#   elif !defined(WIN_3DLITEPP_FONT_DLL)
        // We must be _using_ the DLL, so import symbols instead.
#   define LITE3DPP_FONT_EXPORT __declspec(dllimport)
#   endif

#elif PLATFORM_Linux
    // If not Windows, we assume some sort of Unixy build environment,
    // where autotools is used.  (This includes Cygwin!)  #include the
    // config.h file only if this file was included from a non-header
    // file, because headers must not be dependent on config.h.
#   if !defined(__GNUC__)
#       error "GCC compiler requred.."
#   endif

#   define LITE3DPP_FONT_EXPORT

#endif

namespace nw
{
    class FontLib;
    class Font;
    class Text;
}