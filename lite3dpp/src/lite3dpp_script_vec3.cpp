/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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

#include <lite3d/lite3d_common.h>
LITE3D_EXTERN {
#include <lite3d/kazmath/mat4.h>
#include <lite3d/kazmath/mat3.h>
#include <lite3d/kazmath/vec4.h>
#include <lite3d/kazmath/vec3.h>
}

#include <lite3dpp/as/angelscript.h>

#include <lite3dpp/lite3dpp_script_binding.h>

namespace lite3dpp
{
    static void Vec3Construct(kmVec3 &self)
    {
        self = KM_VEC3_ZERO;
    }

    static void Vec3Destruct(kmVec3 &self)
    {

    }
    
    static void Vec3ConstructXYZ(kmVec3 &self, float x, float y, float z)
    {
        kmVec3Fill(&self, x, y, z);
    }
    
    static void Vec3FromVec4(kmVec3 &self, const kmVec4 &other)
    {
        self.x = other.x;
        self.y = other.y;
        self.z = other.z;
    }

    static void Vec3ConstructCopy(kmVec3 &self, const kmVec3 &other)
    {
        self = other;
    }

    static kmVec3 &Vec3Assign(kmVec3 &self, const kmVec3 &other)
    {
        kmVec3Assign(&self, &other);
        return self;
    }

    static kmVec3 &Vec3SubAssign(kmVec3 &self, const kmVec3 &other)
    {
        kmVec3Subtract(&self, &self, &other);
        return self;
    }
    
    static kmVec3 &Vec3AddAssign(kmVec3 &self, const kmVec3 &other)
    {
        kmVec3Add(&self, &self, &other);
        return self;
    }

    static kmVec3 &Vec3MulAssign(kmVec3 &self, const kmVec3 &other)
    {
        kmVec3Mul(&self, &self, &other);
        return self;
    }

    static kmVec3 &Vec3MulMat3Assign(kmVec3 &self, const kmMat3 &other)
    {
        kmVec3MultiplyMat3(&self, &self, &other);
        return self;
    }

    static kmVec3 &Vec3MulMat4Assign(kmVec3 &self, const kmMat4 &other)
    {
        kmVec3MultiplyMat4(&self, &self, &other);
        return self;
    }
    
    static kmVec3 &Vec3DivAssign(kmVec3 &self, const kmVec3 &other)
    {
        kmVec3Div(&self, &self, &other);

        return self;
    }

    static kmVec3 Vec3Sub(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3Subtract(&res, &self, &other);

        return res;
    }
    
    static kmVec3 Vec3Add(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3Add(&res, &self, &other);

        return res;
    }

    static kmVec3 Vec3Mul(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3Mul(&res, &self, &other);

        return res;
    }

    static kmVec3 Vec3MulMat3(kmVec3 &self, const kmMat3 &other)
    {
        kmVec3 res;
        kmVec3MultiplyMat3(&res, &self, &other);
        return res;
    }

    static kmVec3 Vec3MulMat4(kmVec3 &self, const kmMat4 &other)
    {
        kmVec3 res;
        kmVec3MultiplyMat4(&res, &self, &other);
        return res;
    }
    
    static kmVec3 Vec3Div(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3Div(&res, &self, &other);

        return res;
    }

    static bool Vec3Eq(const kmVec3 &self, const kmVec3 &other)
    {
        return kmVec3AreEqual(&self, &other) == 1;
    }

    static float Vec3Length(const kmVec3 &self)
    {
        return kmVec3Length(&self);
    }

    static float Vec3LengthSq(const kmVec3 &self)
    {
        return kmVec3LengthSq(&self);
    }

    static void Vec3Normalize(kmVec3 &self)
    {
        kmVec3Normalize(&self, &self);
    }
    
    static kmVec3 Vec3Cross(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3Cross(&res, &self, &other);

        return res;
    }

    static float Vec3Dot(const kmVec3 &self, const kmVec3 &other)
    {
        return kmVec3Dot(&self, &other);
    }

    static void Vec3Scale(kmVec3 &self, float scale)
    {
        kmVec3Scale(&self, &self, scale);
    }

    static kmVec3 Vec3HorizontalAngle(kmVec3 &self)
    {
        kmVec3 res;
        kmVec3GetHorizontalAngle(&res, &self);

        return res;
    }

    static kmVec3 Vec3RotationToDirection(const kmVec3 &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmVec3RotationToDirection(&res, &self, &other);

        return res;
    }

    void RegisterScriptVec3(asIScriptEngine *engine)
    {
        /* register constructors */
        SDL_assert(engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f()", 
            asFUNCTION(Vec3Construct), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", 
            asFUNCTION(Vec3ConstructXYZ), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const Vec3 &in)", 
            asFUNCTION(Vec3ConstructCopy), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const Vec4 &in)", 
            asFUNCTION(Vec3FromVec4), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Vec3", asBEHAVE_DESTRUCT, "void f()", 
            asFUNCTION(Vec3Destruct), asCALL_CDECL_OBJFIRST) >= 0);

        /* register properties */
        SDL_assert(engine->RegisterObjectProperty("Vec3", "float x", asOFFSET(kmVec3, x)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec3", "float y", asOFFSET(kmVec3, y)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec3", "float z", asOFFSET(kmVec3, z)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec3", "float s", asOFFSET(kmVec3, x)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Vec3", "float t", asOFFSET(kmVec3, y)) >= 0);

        /* register operators */
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opSub(const Vec3 &in) const", asFUNCTION(Vec3Sub), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opAdd(const Vec3 &in) const", asFUNCTION(Vec3Add), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opMul(const Vec3 &in) const", asFUNCTION(Vec3Mul), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opDiv(const Vec3 &in) const", asFUNCTION(Vec3Div), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opMul(const Mat3 &in) const", asFUNCTION(Vec3MulMat3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 opMul(const Mat4 &in) const", asFUNCTION(Vec3MulMat4), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opAssign(const Vec3 &in)", asFUNCTION(Vec3Assign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opSubAssign(const Vec3 &in)", asFUNCTION(Vec3SubAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opAddAssign(const Vec3 &in)", asFUNCTION(Vec3AddAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opMulAssign(const Vec3 &in)", asFUNCTION(Vec3MulAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opDivAssign(const Vec3 &in)", asFUNCTION(Vec3DivAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opMulAssign(const Mat3 &in)", asFUNCTION(Vec3MulMat3Assign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 &opMulAssign(const Mat4 &in)", asFUNCTION(Vec3MulMat4Assign), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Vec3", "bool opEquals(const Vec3 &in) const", asFUNCTION(Vec3Eq), asCALL_CDECL_OBJFIRST) >= 0);

        /* register methods */
        SDL_assert(engine->RegisterObjectMethod("Vec3", "float length() const", asFUNCTION(Vec3Length), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "float lengthSq() const", asFUNCTION(Vec3LengthSq), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "void normalize()", asFUNCTION(Vec3Normalize), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 cross(const Vec3 &in) const", asFUNCTION(Vec3Cross), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 dot(const Vec3 &in) const", asFUNCTION(Vec3Dot), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "void scale(float)", asFUNCTION(Vec3Scale), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 horizontalAngle() const", asFUNCTION(Vec3HorizontalAngle), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Vec3", "Vec3 rotationToDirection(const Vec3 &in) const", asFUNCTION(Vec3RotationToDirection), asCALL_CDECL_OBJFIRST) >= 0);

        /* register constants */
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_NEG_Z", (void *)&KM_VEC3_NEG_Z) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_POS_Z", (void *)&KM_VEC3_POS_Z) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_NEG_Y", (void *)&KM_VEC3_NEG_Y) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_POS_Y", (void *)&KM_VEC3_POS_Y) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_NEG_X", (void *)&KM_VEC3_NEG_X) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_POS_X", (void *)&KM_VEC3_POS_X) >= 0);
        SDL_assert(engine->RegisterGlobalProperty("const Vec3 VEC3_ZERO", (void *)&KM_VEC3_ZERO) >= 0);
    }
}