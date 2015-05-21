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
#include <string.h>

#include <SDL_assert.h>

#include <3dlite/3dlite_common.h>
LITE3D_EXTERN {
#include <3dlite/kazmath/mat4.h>
#include <3dlite/kazmath/mat3.h>
#include <3dlite/kazmath/vec3.h>
}

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    static void Mat3Construct(kmMat3 &self)
    {
        memset(&self, 0, sizeof(kmMat3));
    }
    
    static void Mat3ConstructCopy(kmMat3 &self, const kmMat3 &other)
    {
        kmMat3Assign(&self, &other);
    }

    static void Mat3FromMat4(kmMat3 &self, const kmMat4 &other)
    {
        kmMat3AssignMat4(&self, &other);
    }

    static kmMat3 &Mat3Assign(kmMat3 &self, const kmMat3 &other)
    {
        kmMat3Assign(&self, &other);
        return self;
    }

    static kmMat3 &Mat3MulAssign(kmMat3 &self, const kmMat3 &other)
    {
        kmMat3Multiply(&self, &self, &other);
        return self;
    }

    static kmMat3 Mat3Mul(const kmMat3 &self, const kmMat3 &other)
    {
        kmMat3 res;
        kmMat3Multiply(&res, &self, &other);
        return res;
    }

    static bool Mat3Eq(const kmMat3 &self, const kmMat3 &other)
    {
        return kmMat3AreEqual(&self, &other) == 1;
    }

    static void Mat3Inverse(kmMat3 &self)
    {
        kmMat3Inverse(&self, &self);
    }

    static void Mat3Identity(kmMat3 &self)
    {
        kmMat3Identity(&self);
    }

    static void Mat3Transpose(kmMat3 &self)
    {
        kmMat3Transpose(&self, &self);
    }

    static float Mat3Determinant(const kmMat3 &self)
    {
        return kmMat3Determinant(&self);
    }

    static void Mat3RotationX(kmMat3 &self, float radians)
    {
        kmMat3RotationX(&self, radians);
    }

    static void Mat3RotationY(kmMat3 &self, float radians)
    {
        kmMat3RotationY(&self, radians);
    }

    static void Mat3RotationZ(kmMat3 &self, float radians)
    {
        kmMat3RotationZ(&self, radians);
    }

    static void Mat3RotationQuaternion(kmMat3 &self, const kmQuaternion &quat)
    {
        kmMat3RotationQuaternion(&self, &quat);
    }

    static void Mat3Scaling(kmMat3 &self, float x, float y)
    {
        kmMat3Scaling(&self, x, y);
    }

    static void Mat3Translation(kmMat3 &self, float x, float y)
    {
        kmMat3Translation(&self, x, y);
    }

    static kmVec3 Mat3GetUpVec3(const kmMat3 &self)
    {
        kmVec3 res;
        kmMat3GetUpVec3(&res, &self);
        return res;
    }

    static kmVec3 Mat3GetRightVec3(const kmMat3 &self)
    {
        kmVec3 res;
        kmMat3GetRightVec3(&res, &self);
        return res;
    }

    static kmVec3 Mat3GetForwardVec3(const kmMat3 &self)
    {
        kmVec3 res;
        kmMat3GetForwardVec3(&res, &self);
        return res;
    }

    static void Mat3LookAt(kmMat3 &self, const kmVec3 &vec1, const kmVec3 &vec2, const kmVec3 &vec3)
    {
        kmMat3LookAt(&self, &vec1, &vec2, &vec3);
    }

    static void Mat3RotationAxisAngle(kmMat3 &self, const kmVec3 &vec1, float radians)
    {
        kmMat3RotationAxisAngle(&self, &vec1, radians);
    }

    void RegisterScriptMat3(asIScriptEngine *engine)
    {
        /* register constructors */
        SDL_assert(engine->RegisterObjectBehaviour("Mat3", asBEHAVE_CONSTRUCT, "void f()", 
            asFUNCTION(Mat3Construct), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Mat3", asBEHAVE_CONSTRUCT, "void f(const Mat3 &in)", 
            asFUNCTION(Mat3ConstructCopy), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Mat3", asBEHAVE_CONSTRUCT, "void f(const Mat4 &in)", 
            asFUNCTION(Mat3FromMat4), asCALL_CDECL_OBJFIRST) >= 0);

        /* register properties */
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float x1", asOFFSET(kmMat3, mat[0])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float y1", asOFFSET(kmMat3, mat[1])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float z1", asOFFSET(kmMat3, mat[2])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float x2", asOFFSET(kmMat3, mat[3])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float y2", asOFFSET(kmMat3, mat[4])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float z2", asOFFSET(kmMat3, mat[5])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float x3", asOFFSET(kmMat3, mat[6])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float y3", asOFFSET(kmMat3, mat[7])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat3", "float z3", asOFFSET(kmMat3, mat[8])) >= 0);


        /* register operators */
        SDL_assert(engine->RegisterObjectMethod("Mat3", "Mat3 opMul(const Mat3 &in) const", asFUNCTION(Mat3Mul), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Mat3", "Mat3 &opAssign(const Mat3 &in)", asFUNCTION(Mat3Assign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "Mat3 &opMulAssign(const Mat3 &in)", asFUNCTION(Mat3MulAssign), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Mat3", "bool opEquals(const Mat3 &in) const", asFUNCTION(Mat3Eq), asCALL_CDECL_OBJFIRST) >= 0);

        /* register methods */
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void inverse()", asFUNCTION(Mat3Inverse), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void identity()", asFUNCTION(Mat3Identity), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void transpose()", asFUNCTION(Mat3Transpose), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "float determinant() const", asFUNCTION(Mat3Determinant), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void rotationX(float)", asFUNCTION(Mat3RotationX), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void rotationY(float)", asFUNCTION(Mat3RotationY), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void rotationZ(float)", asFUNCTION(Mat3RotationZ), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void rotationQuaternion(const Quaternion&)", asFUNCTION(Mat3RotationQuaternion), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void scaling(float, float)", asFUNCTION(Mat3Scaling), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void translation(float, float)", asFUNCTION(Mat3Translation), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "Vec3 getUpVec3() const", asFUNCTION(Mat3GetUpVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "Vec3 getRightVec3() const", asFUNCTION(Mat3GetRightVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "Vec3 getForwardVec3() const", asFUNCTION(Mat3GetForwardVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void lookAt(const Vec3&, const Vec3&, const Vec3&)", asFUNCTION(Mat3LookAt), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat3", "void rotationAxisAngle(const Vec3&, float)", asFUNCTION(Mat3RotationAxisAngle), asCALL_CDECL_OBJFIRST) >= 0);
    }
}