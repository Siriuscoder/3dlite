/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#ifndef LITE3D_FRUSTUM_H
#define	LITE3D_FRUSTUM_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_kazmath.h>

typedef struct lite3d_frustum  
{
    kmPlane clipPlains[6];
} lite3d_frustum;

typedef struct lite3d_bouding_vol
{
    kmVec3 box[8];
    kmVec3 sphereCenter;
    float radius;
} lite3d_bouding_vol;

LITE3D_CEXPORT void lite3d_frustum_compute(struct lite3d_frustum *frustum, 
    const struct kmMat4 *clip);
    
LITE3D_CEXPORT int lite3d_frustum_test(struct lite3d_frustum *frustum, 
    const struct lite3d_bouding_vol *vol);

LITE3D_CEXPORT void lite3d_bouding_vol_setup(struct lite3d_bouding_vol *vol,
    const kmVec3 *vmin, const kmVec3 *vmax);

#endif	/* LITE3D_FRUSTUM_H */

