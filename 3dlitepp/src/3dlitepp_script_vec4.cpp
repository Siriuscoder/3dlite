/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <SDL_assert.h>

#include <3dlite/3dlite_common.h>
LITE3D_EXTERN {
#include <3dlite/kazmath/mat4.h>
#include <3dlite/kazmath/vec4.h>
#include <3dlite/kazmath/vec3.h>
}

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    static void Vec4Construct(kmVec4 &self)
    {
        self = KM_VEC4_ZERO;
    }

    static void Vec4Destruct(kmVec4 &self)
    {

    }
    
    static void Vec4ConstructXYZ(kmVec4 &self, float x, float y, float z, float w)
    {
        kmVec4Fill(&self, x, y, z, w);
    }
    
    static void Vec4ConstructCopy(kmVec4 &self, const kmVec4 &other)
    {
        self = other;
    }

    static void Vec4FromVec3(kmVec4 &self, const kmVec3 &other)
    {
        self.x = other.x;
        self.y = other.y;
        self.z = other.z;
        self.x = 0;
    }

    static kmVec4 &Vec4Assign(kmVec4 &self, const kmVec4 &other)
    {
        kmVec4Assign(&self, &other);
        return self;
    }

    static kmVec4 &Vec4SubAssign(kmVec4 &self, const kmVec4 &other)
    {
        kmVec4Subtract(&self, &self, &other);
        return self;
    }
    
    static kmVec4 &Vec4AddAssign(kmVec4 &self, const kmVec4 &other)
    {
        kmVec4Add(&self, &self, &other);
        return self;
    }

    static kmVec4 &Vec4MulAssign(kmVec4 &self, const kmVec4 &other)
    {
        kmVec4Mul(&self, &self, &other);
        return self;
    }

    static kmVec4 &Vec4MulMat4Assign(kmVec4 &self, const kmMat4 &other)
    {
        kmVec4MultiplyMat4(&self, &self, &other);
        return self;
    }
    
    static kmVec4 &Vec4DivAssign(kmVec4 &self, const kmVec4 &other)
    {
        kmVec4Div(&self, &self, &other);

        return self;
    }

    static kmVec4 Vec4Sub(const kmVec4 &self, const kmVec4 &other)
    {
        kmVec4 res;
        kmVec4Subtract(&res, &self, &other);

        return res;
    }
    
    static kmVec4 Vec4Add(const kmVec4 &self, const kmVec4 &other)
    {
        kmVec4 res;
        kmVec4Add(&res, &self, &other);

        return res;
    }

    static kmVec4 Vec4Mul(const kmVec4 &self, const kmVec4 &other)
    {
        kmVec4 res;
        kmVec4Mul(&res, &self, &other);

        return res;
    }

    static kmVec4 Vec4MulMat4(kmVec4 &self, const kmMat4 &other)
    {
        kmVec4 res;
        kmVec4MultiplyMat4(&res, &self, &other);
        return res;
    }
    
    static kmVec4 Vec4Div(const kmVec4 &self, const kmVec4 &other)
    {
        kmVec4 res;
        kmVec4Div(&res, &self, &other);

        return res;
    }

    static bool Vec4Eq(const kmVec4 &self, const kmVec4 &other)
    {
        return kmVec4AreEqual(&self, &other) == 1;
    }

    static float Vec4Length(const kmVec4 &self)
    {
        return kmVec4Length(&self);
    }

    static float Vec4LengthSq(const kmVec4 &self)
    {
        return kmVec4LengthSq(&self);
    }

    static void Vec4Normalize(kmVec4 &self)
    {
        kmVec4Normalize(&self, &self);
    }

    static float Vec4Dot(const kmVec4 &self, const kmVec4 &other)
    {
        return kmVec4Dot(&self, &other);
    }

    static void Vec4Scale(kmVec4 &self, float scale)
    {
        kmVec4Scale(&self, &self, scale);
    }

    void RegisterScriptVec4(asIScriptEngine *engine)
    {
        /* register constructors */
        SDL_assert(engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f()", 
            asFUNCTION(Vec4Construct), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", 
            asFUNCTION(Vec4ConstructXYZ), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(const Vec4 &in)", 
            asFUNCTION(Vec4ConstructCopy), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(const Vec3 &in)", 
            asFUNCTION(Vec4FromVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec4", asBEHAVE_DESTRUCT, "void f()", 
            asFUNCTION(Vec4Destruct), asCALL_CDECL_OBJFIRST) >= 0);

        /* register properties */
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float x", asOFFSET(kmVec4, x)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float y", asOFFSET(kmVec4, y)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float z", asOFFSET(kmVec4, z)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float w", asOFFSET(kmVec4, w)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float s", asOFFSET(kmVec4, x)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec4", "float t", asOFFSET(kmVec4, y)) >= 0);

        /* register operators */
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 opSub(const Vec4 &in) const", asFUNCTION(Vec4Sub), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 opAdd(const Vec4 &in) const", asFUNCTION(Vec4Add), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 opMul(const Vec4 &in) const", asFUNCTION(Vec4Mul), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 opDiv(const Vec4 &in) const", asFUNCTION(Vec4Div), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 opMul(const Mat4 &in) const", asFUNCTION(Vec4MulMat4), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opAssign(const Vec4 &in)", asFUNCTION(Vec4Assign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opSubAssign(const Vec4 &in)", asFUNCTION(Vec4SubAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opAddAssign(const Vec4 &in)", asFUNCTION(Vec4AddAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opMulAssign(const Vec4 &in)", asFUNCTION(Vec4MulAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opDivAssign(const Vec4 &in)", asFUNCTION(Vec4DivAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 &opMulAssign(const Mat4 &in)", asFUNCTION(Vec4MulMat4Assign), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Vec4", "bool opEquals(const Vec4 &in) const", asFUNCTION(Vec4Eq), asCALL_CDECL_OBJFIRST) >= 0);

        /* register methods */
        SDL_assert(engine->RegisterObjectMethod("Vec4", "float length() const", asFUNCTION(Vec4Length), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "float lengthSq() const", asFUNCTION(Vec4LengthSq), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "void normalize()", asFUNCTION(Vec4Normalize), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "Vec4 dot(const Vec4 &in) const", asFUNCTION(Vec4Dot), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec4", "void scale(float)", asFUNCTION(Vec4Scale), asCALL_CDECL_OBJFIRST) >= 0);

        /* register constants */
        SDL_assert(engine->RegisterGlobalProperty("const Vec4 VEC4_ZERO", (void *)&KM_VEC4_ZERO) >= 0);
    }
}