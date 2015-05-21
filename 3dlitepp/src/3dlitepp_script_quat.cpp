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
#include <3dlite/kazmath/vec3.h>
#include <3dlite/kazmath/quaternion.h>
}

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    static void QuaternionConstruct(kmQuaternion &self)
    {
        memset(&self, 0, sizeof(kmQuaternion));
    }
    
    static void QuaternionConstructXYZW(kmQuaternion &self, float x, float y, float z, float w)
    {
        kmQuaternionFill(&self, x, y, z, w);
    }
    
    static void QuaternionConstructCopy(kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternionAssign(&self, &other);
    }

    static kmQuaternion &QuaternionAssign(kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternionAssign(&self, &other);
        return self;
    }

    static kmQuaternion &QuaternionSubAssign(kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternionSubtract(&self, &self, &other);
        return self;
    }
    
    static kmQuaternion &QuaternionAddAssign(kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternionAdd(&self, &self, &other);
        return self;
    }

    static kmQuaternion &QuaternionMulAssign(kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternionMultiply(&self, &self, &other);
        return self;
    }

    static kmQuaternion QuaternionSub(const kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternion res;
        kmQuaternionSubtract(&res, &self, &other);

        return res;
    }
    
    static kmQuaternion QuaternionAdd(const kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternion res;
        kmQuaternionAdd(&res, &self, &other);

        return res;
    }

    static kmQuaternion QuaternionMul(const kmQuaternion &self, const kmQuaternion &other)
    {
        kmQuaternion res;
        kmQuaternionMultiply(&res, &self, &other);

        return res;
    }

    static kmVec3 QuaternionMulVec3(kmQuaternion &self, const kmVec3 &other)
    {
        kmVec3 res;
        kmQuaternionMultiplyVec3(&res, &self, &other);
        return res;
    }

    static bool QuaternionEq(const kmQuaternion &self, const kmQuaternion &other)
    {
        return kmQuaternionAreEqual(&self, &other) == 1;
    }

    static void QuaternionInverse(kmQuaternion &self)
    {
        kmQuaternionInverse(&self, &self);
    }

    static void QuaternionIdentity(kmQuaternion &self)
    {
        kmQuaternionIdentity(&self);
    }

    static float QuaternionLength(const kmQuaternion &self)
    {
        return kmQuaternionLength(&self);
    }

    static float QuaternionLengthSq(const kmQuaternion &self)
    {
        return kmQuaternionLengthSq(&self);
    }

    static void QuaternionNormalize(kmQuaternion &self)
    {
        kmQuaternionNormalize(&self, &self);
    }

    static float QuaternionDot(const kmQuaternion &self, const kmQuaternion &other)
    {
        return kmQuaternionDot(&self, &other);
    }

    static void QuaternionScale(kmQuaternion &self, float scale)
    {
        kmQuaternionScale(&self, &self, scale);
    }

    static kmVec3 QuaternionGetUpVec3(const kmQuaternion &self)
    {
        kmVec3 res;
        kmQuaternionGetUpVec3(&res, &self);
        return res;
    }

    static kmVec3 QuaternionGetRightVec3(const kmQuaternion &self)
    {
        kmVec3 res;
        kmQuaternionGetRightVec3(&res, &self);
        return res;
    }

    static kmVec3 QuaternionGetForwardVec3RH(const kmQuaternion &self)
    {
        kmVec3 res;
        kmQuaternionGetForwardVec3RH(&res, &self);
        return res;
    }

    static kmVec3 QuaternionGetForwardVec3LH(const kmQuaternion &self)
    {
        kmVec3 res;
        kmQuaternionGetForwardVec3LH(&res, &self);
        return res;
    }

    static void QuaternionRotationYawPitchRoll(kmQuaternion &self, float pitch, float yaw, float roll)
    {
        kmQuaternionRotationPitchYawRoll(&self, pitch, yaw, roll);
    }

    static void QuaternionRotationAxisAngle(kmQuaternion &self, const kmVec3 &vec1, float radians)
    {
        kmQuaternionRotationAxisAngle(&self, &vec1, radians);
    }

    static void QuaternionRotationMatrix(kmQuaternion &self, const kmMat3 &mat)
    {
        kmQuaternionRotationMatrix(&self, &mat);
    }

    static float QuaternionGetPitch(const kmQuaternion &self)
    {
        return kmQuaternionGetPitch(&self);
    }

    static float QuaternionGetYaw(const kmQuaternion &self)
    {
        return kmQuaternionGetYaw(&self);
    }

    static float QuaternionGetRoll(const kmQuaternion &self)
    {
        return kmQuaternionGetRoll(&self);
    }

    void RegisterScriptQuaternion(asIScriptEngine *engine)
    {
        /* register constructors */
        SDL_assert(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()", 
            asFUNCTION(QuaternionConstruct), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", 
            asFUNCTION(QuaternionConstructXYZW), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Quaternion &in)", 
            asFUNCTION(QuaternionConstructCopy), asCALL_CDECL_OBJFIRST) >= 0);

        /* register properties */
        SDL_assert(engine->RegisterObjectProperty("Quaternion", "float x", asOFFSET(kmQuaternion, x)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Quaternion", "float y", asOFFSET(kmQuaternion, y)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Quaternion", "float z", asOFFSET(kmQuaternion, z)) >= 0);
        SDL_assert(engine->RegisterObjectProperty("Quaternion", "float w", asOFFSET(kmQuaternion, w)) >= 0);

        /* register operators */
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion opSub(const Quaternion &in) const", asFUNCTION(QuaternionSub), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion opAdd(const Quaternion &in) const", asFUNCTION(QuaternionAdd), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(const Quaternion &in) const", asFUNCTION(QuaternionMul), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Vec3 opMul(const Vec3 &in) const", asFUNCTION(QuaternionMulVec3), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion &opAssign(const Quaternion &in)", asFUNCTION(QuaternionAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion &opSubAssign(const Quaternion &in)", asFUNCTION(QuaternionSubAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion &opAddAssign(const Quaternion &in)", asFUNCTION(QuaternionAddAssign), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion &opMulAssign(const Quaternion &in)", asFUNCTION(QuaternionMulAssign), asCALL_CDECL_OBJFIRST) >= 0);

        SDL_assert(engine->RegisterObjectMethod("Quaternion", "bool opEquals(const Quaternion &in) const", asFUNCTION(QuaternionEq), asCALL_CDECL_OBJFIRST) >= 0);

        /* register methods */
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "float length() const", asFUNCTION(QuaternionLength), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "float lengthSq() const", asFUNCTION(QuaternionLengthSq), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void normalize()", asFUNCTION(QuaternionNormalize), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Quaternion dot(const Quaternion &in) const", asFUNCTION(QuaternionDot), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void scale(float in)", asFUNCTION(QuaternionScale), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Vec3 getUpVec3() const", asFUNCTION(QuaternionGetUpVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Vec3 getRightVec3() const", asFUNCTION(QuaternionGetRightVec3), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Vec3 getForwardVec3RH() const", asFUNCTION(QuaternionGetForwardVec3RH), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "Vec3 getForwardVec3LH() const", asFUNCTION(QuaternionGetForwardVec3LH), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void inverse()", asFUNCTION(QuaternionInverse), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void identity()", asFUNCTION(QuaternionIdentity), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void rotationYawPitchRoll(float, float, float)", asFUNCTION(QuaternionRotationYawPitchRoll), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void rotationAxisAngle(const Vec3&, float)", asFUNCTION(QuaternionRotationAxisAngle), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "void rotationMatrix(const Mat3&)", asFUNCTION(QuaternionRotationMatrix), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "float getPitch() const", asFUNCTION(QuaternionGetPitch), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "float getYaw() const", asFUNCTION(QuaternionGetYaw), asCALL_CDECL_OBJFIRST) >= 0);
        SDL_assert(engine->RegisterObjectMethod("Quaternion", "float getRoll() const", asFUNCTION(QuaternionGetRoll), asCALL_CDECL_OBJFIRST) >= 0);
    }
}