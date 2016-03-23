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

#ifndef VEC2_H_INCLUDED
#define VEC2_H_INCLUDED

#include "utility.h"

struct kmMat3;

typedef struct kmVec2 {
    kmScalar x;
    kmScalar y;
} kmVec2;

LITE3D_CEXPORT kmVec2* kmVec2Fill(kmVec2* pOut, kmScalar x, kmScalar y);
LITE3D_CEXPORT kmScalar kmVec2Length(const kmVec2* pIn); /**< Returns the length of the vector*/
LITE3D_CEXPORT kmScalar kmVec2LengthSq(const kmVec2* pIn); /**< Returns the square of the length of the vector*/
LITE3D_CEXPORT kmVec2* kmVec2Normalize(kmVec2* pOut, const kmVec2* pIn); /**< Returns the vector passed in set to unit length*/
LITE3D_CEXPORT kmVec2* kmVec2Lerp(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2, kmScalar t);
LITE3D_CEXPORT kmVec2* kmVec2Add(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2); /**< Adds 2 vectors and returns the result*/
LITE3D_CEXPORT kmScalar kmVec2Dot(const kmVec2* pV1, const kmVec2* pV2); /** Returns the Dot product which is the cosine of the angle between the two vectors multiplied by their lengths */
LITE3D_CEXPORT kmScalar kmVec2Cross(const kmVec2* pV1, const kmVec2* pV2);
LITE3D_CEXPORT kmVec2* kmVec2Subtract(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2); /**< Subtracts 2 vectors and returns the result*/
LITE3D_CEXPORT kmVec2* kmVec2Mul( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 ); /**< Component-wise multiplication */
LITE3D_CEXPORT kmVec2* kmVec2Div( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 ); /**< Component-wise division*/
LITE3D_CEXPORT kmVec2* kmVec2Transform(kmVec2* pOut, const kmVec2* pV1, const struct kmMat3* pM); /** Transform the Vector */
LITE3D_CEXPORT kmVec2* kmVec2TransformCoord(kmVec2* pOut, const kmVec2* pV, const struct kmMat3* pM); /**<Transforms a 2D vector by a given matrix, projecting the result back into w = 1.*/
LITE3D_CEXPORT kmVec2* kmVec2Scale(kmVec2* pOut, const kmVec2* pIn, const kmScalar s); /**< Scales a vector to length s*/
LITE3D_CEXPORT int	kmVec2AreEqual(const kmVec2* p1, const kmVec2* p2); /**< Returns 1 if both vectors are equal*/
LITE3D_CEXPORT kmVec2* kmVec2Assign(kmVec2* pOut, const kmVec2* pIn);
LITE3D_CEXPORT kmVec2* kmVec2RotateBy(kmVec2* pOut, const kmVec2* pIn, const kmScalar degrees, const kmVec2* center); /**<Rotates the point anticlockwise around a center by an amount of degrees.*/
LITE3D_CEXPORT kmScalar kmVec2DegreesBetween(const kmVec2* v1, const kmVec2* v2);
LITE3D_CEXPORT kmScalar kmVec2DistanceBetween(const kmVec2* v1, const kmVec2* v2);
LITE3D_CEXPORT kmVec2* kmVec2MidPointBetween(kmVec2* pOut, const kmVec2* v1, const kmVec2* v2);
LITE3D_CEXPORT kmVec2* kmVec2MidPointBetween(kmVec2* pOut, const kmVec2* v1, const kmVec2* v2);

static const kmVec2 KM_VEC2_POS_Y = { 0, 1 };
static const kmVec2 KM_VEC2_NEG_Y = { 0, -1 };
static const kmVec2 KM_VEC2_NEG_X = { -1, 0 };
static const kmVec2 KM_VEC2_POS_X = { 1, 0 };
static const kmVec2 KM_VEC2_ZERO = { 0, 0 };
static const kmVec2 KM_VEC2_ONE = { 1, 1 };

#endif /* VEC2_H_INCLUDED */
