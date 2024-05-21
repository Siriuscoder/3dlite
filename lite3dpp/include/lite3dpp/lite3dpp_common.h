/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024  Sirius (Korolev Nikita)
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

#include <limits>
#include <cmath>

#include <lite3d/lite3d_common.h>

#ifdef _MSC_VER

// Disable whining about using 'this' as a member initializer on VC++.
#   pragma warning(disable: 4355)
#   pragma warning(disable: 4127)
#   pragma warning(disable: 4251)

#   ifdef WIN_3DLITEPP_DLL
// When making the DLL, export tagged symbols, so they appear
// in the import library.
#   define LITE3DPP_EXPORT __declspec(dllexport)
#   elif !defined(WIN_3DLITEPP_DLL)
        // We must be _using_ the DLL, so import symbols instead.
#   define LITE3DPP_EXPORT __declspec(dllimport)
#   endif

#elif defined(__GNUC__) || defined(__clang__)
#   define LITE3DPP_EXPORT
#else
#   error "Unknown target compiler"
#endif

//#define LITE3DPP_USE_STL_ALLOCATOR

#include <lite3d/lite3d_kazmath.h>

#define LITE3D_THROW(mess) { lite3dpp::Stringstream exmess; exmess << __FILE__ << ":" << \
    LITE3D_CURRENT_FUNCTION << ":" << __LINE__ << ": " << mess; throw std::runtime_error(exmess.str()); }

namespace lite3dpp
{
    /* Main engine class */
    class LITE3DPP_EXPORT Main;
    class LITE3DPP_EXPORT Scene;
    class LITE3DPP_EXPORT Camera;
    class LITE3DPP_EXPORT SceneNode;
    class LITE3DPP_EXPORT Material;
    class LITE3DPP_EXPORT Texture;
    class LITE3DPP_EXPORT RenderTarget;
    class LITE3DPP_EXPORT ConfigurationWriter;
    class LITE3DPP_EXPORT ConfigurationReader;

    /* epsilon */
    const constexpr auto epsilon = std::numeric_limits<float>::epsilon();

    inline bool nonzero(float num)
    {
        return std::fabs(num) >= epsilon; 
    }

    inline bool iszero(float num)
    {
        return !nonzero(num);
    }

    inline bool near(float a, float b)
    {
        return std::fabs(a - b) <= epsilon;
    }
}

class JSONValue;
class asIScriptEngine;
class asIScriptModule;
class asIScriptFunction;
class asIScriptContext;

#define LITE3D_DECLARE_PTR_METHODS(type, param) \
    inline type *getPtr() \
    { return &param; } \
    inline const type *getPtr() const \
    { return &param; } \
    inline operator type *() \
    { return getPtr(); } \
    inline operator type *() const \
    { return const_cast<type *>(getPtr()); }
