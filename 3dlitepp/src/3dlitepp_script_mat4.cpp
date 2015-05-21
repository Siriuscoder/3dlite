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
#include <3dlite/kazmath/vec3.h>
}

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    static void Mat4Construct(kmMat4 &self)
    {
        memset(&self, 0, sizeof(kmMat4));
    }
    
    static void Mat4ConstructCopy(kmMat4 &self, const kmMat4 &other)
    {
        self = other;
    }

    static void Mat4FromMat3(kmMat4 &self, const kmMat3 &other)
    {
        kmMat4AssignMat3(&self, &other);
    }

    static kmMat4 &Mat4Assign(kmMat4 &self, const kmMat4 &other)
    {
        kmMat4Assign(&self, &other);
        return self;
    }

    static kmMat4 &Mat4MulAssign(kmMat4 &self, const kmMat4 &other)
    {
        kmMat4Multiply(&self, &self, &other);
        return self;
    }

    static kmMat4 Mat4Mul(const kmMat4 &self, const kmMat4 &other)
    {
        kmMat4 res;
        kmMat4Multiply(&res, &self, &other);
        return res;
    }

    static bool Mat4Eq(const kmMat4 &self, const kmMat4 &other)
    {
        return kmMat4AreEqual(&self, &other) == 1;
    }

    static void Mat4Inverse(kmMat4 &self)
    {
        kmMat4Inverse(&self, &self);
    }

    static void Mat4Identity(kmMat4 &self)
    {
        kmMat4Identity(&self);
    }

    static void Mat4Transpose(kmMat4 &self)
    {
        kmMat4Transpose(&self, &self);
    }

    static void Mat4RotationX(kmMat4 &self, float radians)
    {
        kmMat4RotationX(&self, radians);
    }

    static void Mat4RotationY(kmMat4 &self, float radians)
    {
        kmMat4RotationY(&self, radians);
    }

    static void Mat4RotationZ(kmMat4 &self, float radians)
    {
        kmMat4RotationZ(&self, radians);
    }

    static void Mat4RotationYawPitchRoll(kmMat4 &self, float yaw, float pitch, float roll)
    {
        kmMat4RotationYawPitchRoll(&self, yaw, pitch, roll);
    }

    static void Mat4RotationQuaternion(kmMat4 &self, const kmQuaternion &quat)
    {
        kmMat4RotationQuaternion(&self, &quat);
    }

    static void Mat4Scaling(kmMat4 &self, float x, float y, float z)
    {
        kmMat4Scaling(&self, x, y, z);
    }

    static void Mat4Translation(kmMat4 &self, float x, float y, float z)
    {
        kmMat4Translation(&self, x, y, z);
    }

    static kmVec3 Mat4GetUpVec3(const kmMat4 &self)
    {
        kmVec3 res;
        kmMat4GetUpVec3(&res, &self);
        return res;
    }

    static kmVec3 Mat4GetRightVec3(const kmMat4 &self)
    {
        kmVec3 res;
        kmMat4GetRightVec3(&res, &self);
        return res;
    }

    static kmVec3 Mat4GetForwardVec3RH(const kmMat4 &self)
    {
        kmVec3 res;
        kmMat4GetForwardVec3RH(&res, &self);
        return res;
    }

    static kmVec3 Mat4GetForwardVec3LH(const kmMat4 &self)
    {
        kmVec3 res;
        kmMat4GetForwardVec3LH(&res, &self);
        return res;
    }

    static void Mat4PerspectiveProjection(kmMat4 &self, float x, float y, float z, float zz)
    {
        kmMat4PerspectiveProjection(&self, x, y, z, zz);
    }

    static void Mat4OrthographicProjection(kmMat4 &self, float x1, float x2, float x3, float x4, float x5, float x6)
    {
        kmMat4OrthographicProjection(&self, x1, x2, x3, x4, x5, x6);
    }

    static void Mat4LookAt(kmMat4 &self, const kmVec3 &vec1, const kmVec3 &vec2, const kmVec3 &vec3)
    {
        kmMat4LookAt(&self, &vec1, &vec2, &vec3);
    }

    static void Mat4RotationAxisAngle(kmMat4 &self, const kmVec3 &vec1, float radians)
    {
        kmMat4RotationAxisAngle(&self, &vec1, radians);
    }

    void RegisterScriptMat4(asIScriptEngine *engine)
    {
        /* register constructors */
        SDL_assert(engine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f()", 
            asFUNCTION(Mat4Construct), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f(const Mat4 &in)", 
            asFUNCTION(Mat4ConstructCopy), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Mat4", asBEHAVE_CONSTRUCT, "void f(const Mat3 &in)", 
            asFUNCTION(Mat4FromMat3), asCALL_CDECL_OBJFIRST) >= 0);

        /* register properties */
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float x1", asOFFSET(kmMat4, mat[0])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float y1", asOFFSET(kmMat4, mat[1])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float z1", asOFFSET(kmMat4, mat[2])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float w1", asOFFSET(kmMat4, mat[3])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float x2", asOFFSET(kmMat4, mat[4])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float y2", asOFFSET(kmMat4, mat[5])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float z2", asOFFSET(kmMat4, mat[6])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float w2", asOFFSET(kmMat4, mat[7])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float x3", asOFFSET(kmMat4, mat[8])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float y3", asOFFSET(kmMat4, mat[9])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float z3", asOFFSET(kmMat4, mat[10])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float w3", asOFFSET(kmMat4, mat[11])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float x4", asOFFSET(kmMat4, mat[12])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float y4", asOFFSET(kmMat4, mat[13])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float z4", asOFFSET(kmMat4, mat[14])) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Mat4", "float w4", asOFFSET(kmMat4, mat[15])) >= 0);

        /* register operators */
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Mat4 opMul(const Mat4 &in) const", asFUNCTION(Mat4Mul), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Mat4", "Mat4 &opAssign(const Mat4 &in)", asFUNCTION(Mat4Assign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Mat4 &opMulAssign(const Mat4 &in)", asFUNCTION(Mat4MulAssign), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Mat4", "bool opEquals(const Mat4 &in) const", asFUNCTION(Mat4Eq), asCALL_CDECL_OBJFIRST) >= 0);

        /* register methods */
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void inverse()", asFUNCTION(Mat4Inverse), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void identity()", asFUNCTION(Mat4Identity), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void transpose()", asFUNCTION(Mat4Transpose), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationX(float)", asFUNCTION(Mat4RotationX), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationY(float)", asFUNCTION(Mat4RotationY), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationZ(float)", asFUNCTION(Mat4RotationZ), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationYawPitchRoll(float, float, float)", asFUNCTION(Mat4RotationYawPitchRoll), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationQuaternion(const Quaternion&)", asFUNCTION(Mat4RotationQuaternion), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void scaling(float, float, float)", asFUNCTION(Mat4Scaling), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void translation(float, float, float)", asFUNCTION(Mat4Translation), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Vec3 getUpVec3() const", asFUNCTION(Mat4GetUpVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Vec3 getRightVec3() const", asFUNCTION(Mat4GetRightVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Vec3 getForwardVec3RH() const", asFUNCTION(Mat4GetForwardVec3RH), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "Vec3 getForwardVec3LH() const", asFUNCTION(Mat4GetForwardVec3LH), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void perspectiveProjection(float, float, float, float)", asFUNCTION(Mat4PerspectiveProjection), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void orthographicProjection(float, float, float, float, float)", asFUNCTION(Mat4OrthographicProjection), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void lookAt(const Vec3&, const Vec3&, const Vec3&)", asFUNCTION(Mat4LookAt), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Mat4", "void rotationAxisAngle(const Vec3&, float)", asFUNCTION(Mat4RotationAxisAngle), asCALL_CDECL_OBJFIRST) >= 0);
    }
}