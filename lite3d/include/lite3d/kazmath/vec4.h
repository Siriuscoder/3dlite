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

#ifndef VEC4_H_INCLUDED
#define VEC4_H_INCLUDED

#include "utility.h"

struct kmMat4;

typedef struct kmVec4
{
	kmScalar x;
	kmScalar y;
	kmScalar z;
	kmScalar w;
} kmVec4;

LITE3D_CEXPORT kmVec4* kmVec4Fill(kmVec4* pOut, kmScalar x, kmScalar y, kmScalar z, kmScalar w);
LITE3D_CEXPORT kmVec4* kmVec4Add(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);
LITE3D_CEXPORT kmScalar kmVec4Dot(const kmVec4* pV1, const kmVec4* pV2);
LITE3D_CEXPORT kmScalar kmVec4Length(const kmVec4* pIn);
LITE3D_CEXPORT kmScalar kmVec4LengthSq(const kmVec4* pIn);
LITE3D_CEXPORT kmVec4* kmVec4Lerp(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2, kmScalar t);
LITE3D_CEXPORT kmVec4* kmVec4Normalize(kmVec4* pOut, const kmVec4* pIn);
LITE3D_CEXPORT kmVec4* kmVec4Scale(kmVec4* pOut, const kmVec4* pIn, const kmScalar s); /**< Scales a vector to length s*/
LITE3D_CEXPORT kmVec4* kmVec4Subtract(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);
LITE3D_CEXPORT kmVec4* kmVec4Mul( kmVec4* pOut,const kmVec4* pV1, const kmVec4* pV2 ); 
LITE3D_CEXPORT kmVec4* kmVec4Div( kmVec4* pOut,const kmVec4* pV1, const kmVec4* pV2 ); 

LITE3D_CEXPORT kmVec4* kmVec4MultiplyMat4(kmVec4* pOut, const kmVec4* pV, const struct kmMat4* pM);
LITE3D_CEXPORT kmVec4* kmVec4Transform(kmVec4* pOut, const kmVec4* pV, const struct kmMat4* pM);
LITE3D_CEXPORT kmVec4* kmVec4TransformArray(kmVec4* pOut, unsigned int outStride, 
    const kmVec4* pV, unsigned int vStride, const struct kmMat4* pM, unsigned int count);
LITE3D_CEXPORT int kmVec4AreEqual(const kmVec4* p1, const kmVec4* p2);
LITE3D_CEXPORT kmVec4* kmVec4Assign(kmVec4* pOut, const kmVec4* pIn);

static const kmVec4 KM_VEC4_ZERO = { 0, 0, 0, 0 };
static const kmVec4 KM_VEC4_ONE = { 1, 1, 1, 1 };

#endif /* VEC4_H_INCLUDED */
