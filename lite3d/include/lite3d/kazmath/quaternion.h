/*
Copyright (c) 2008, Luke Benstead.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef QUATERNION_H_INCLUDED
#define QUATERNION_H_INCLUDED

#include "utility.h"

struct kmMat4;
struct kmMat3;
struct kmVec3;

typedef struct kmQuaternion {
	kmScalar x;
	kmScalar y;
	kmScalar z;
	kmScalar w;
} kmQuaternion;

LITE3D_CEXPORT int kmQuaternionAreEqual(const kmQuaternion* p1, const kmQuaternion* p2);
LITE3D_CEXPORT kmQuaternion* kmQuaternionFill(kmQuaternion* pOut, kmScalar x, kmScalar y, kmScalar z, kmScalar w);
LITE3D_CEXPORT kmScalar 	kmQuaternionDot(const kmQuaternion* q1, const kmQuaternion* q2); /**< Returns the dot product of the 2 quaternions*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionExp(kmQuaternion* pOut, const kmQuaternion* pIn); /**< Returns the exponential of the quaternion*/

/**< Makes the passed quaternion an identity quaternion*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionIdentity(kmQuaternion* pOut);

/**< Returns the inverse of the passed Quaternion*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionInverse(kmQuaternion* pOut, const kmQuaternion* pIn);

/**< Returns true if the quaternion is an identity quaternion*/

LITE3D_CEXPORT int kmQuaternionIsIdentity(const kmQuaternion* pIn);

/**< Returns the length of the quaternion*/

LITE3D_CEXPORT kmScalar kmQuaternionLength(const kmQuaternion* pIn);

/**< Returns the length of the quaternion squared (prevents a sqrt)*/

LITE3D_CEXPORT kmScalar kmQuaternionLengthSq(const kmQuaternion* pIn);

/**< Returns the natural logarithm*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionLn(kmQuaternion* pOut, const kmQuaternion* pIn);

/**< Multiplies 2 quaternions together*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionMultiply(kmQuaternion* pOut, const kmQuaternion* q1, const kmQuaternion* q2);

/**< Normalizes a quaternion*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionNormalize(kmQuaternion* pOut, const kmQuaternion* pIn);

/**< Rotates a quaternion around an axis*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionRotationAxisAngle(kmQuaternion* pOut, const struct kmVec3* pV, kmScalar angle);

/**< Creates a quaternion from a rotation matrix*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionRotationMatrix(kmQuaternion* pOut, const struct kmMat3* pIn);

/**< Create a quaternion from yaw, pitch and roll*/

LITE3D_CEXPORT kmQuaternion* kmQuaternionRotationPitchYawRoll(kmQuaternion* pOut, kmScalar pitch, kmScalar yaw, kmScalar roll);
/**< Interpolate between 2 quaternions*/
LITE3D_CEXPORT kmQuaternion* kmQuaternionSlerp(kmQuaternion* pOut, const kmQuaternion* q1, const kmQuaternion* q2, kmScalar t);

/**< Get the axis and angle of rotation from a quaternion*/
LITE3D_CEXPORT void kmQuaternionToAxisAngle(const kmQuaternion* pIn, struct kmVec3* pVector, kmScalar* pAngle);

/**< Scale a quaternion*/
LITE3D_CEXPORT kmQuaternion* kmQuaternionScale(kmQuaternion* pOut, const kmQuaternion* pIn, kmScalar s);
LITE3D_CEXPORT kmQuaternion* kmQuaternionAssign(kmQuaternion* pOut, const kmQuaternion* pIn);
LITE3D_CEXPORT kmQuaternion* kmQuaternionAdd(kmQuaternion* pOut, const kmQuaternion* pQ1, const kmQuaternion* pQ2);
LITE3D_CEXPORT kmQuaternion* kmQuaternionSubtract(kmQuaternion* pOut, const kmQuaternion* pQ1, const kmQuaternion* pQ2);

LITE3D_CEXPORT kmQuaternion* kmQuaternionRotationBetweenVec3(kmQuaternion* pOut, const struct kmVec3* vec1, const struct kmVec3* vec2, const struct kmVec3* fallback);
LITE3D_CEXPORT struct kmVec3* kmQuaternionMultiplyVec3(struct kmVec3* pOut, const kmQuaternion* q, const struct kmVec3* v);

LITE3D_CEXPORT kmVec3* kmQuaternionGetUpVec3(kmVec3* pOut, const kmQuaternion* pIn);
LITE3D_CEXPORT kmVec3* kmQuaternionGetRightVec3(kmVec3* pOut, const kmQuaternion* pIn);
LITE3D_CEXPORT kmVec3* kmQuaternionGetForwardVec3RH(kmVec3* pOut, const kmQuaternion* pIn);
LITE3D_CEXPORT kmVec3* kmQuaternionGetForwardVec3LH(kmVec3* pOut, const kmQuaternion* pIn);

LITE3D_CEXPORT kmScalar kmQuaternionGetPitch(const kmQuaternion* q);
LITE3D_CEXPORT kmScalar kmQuaternionGetYaw(const kmQuaternion* q);
LITE3D_CEXPORT kmScalar kmQuaternionGetRoll(const kmQuaternion* q);

LITE3D_CEXPORT kmQuaternion* kmQuaternionLookRotation(kmQuaternion* pOut, const kmVec3* direction, const kmVec3* up);

static const kmQuaternion KM_QUATERNION_IDENTITY = { 0, 0, 0, 1 };

#endif
