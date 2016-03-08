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
#include <SDL_assert.h>

#include <lite3d/lite3d_frustum.h>

void lite3d_frustum_compute(struct lite3d_frustum *frustum,
    const struct kmMat4 *clip)
{
    SDL_assert(frustum);
    SDL_assert(clip);

    /* rigth plane */
    frustum->clipPlains[0].a = clip->mat[ 3] - clip->mat[ 0];
    frustum->clipPlains[0].b = clip->mat[ 7] - clip->mat[ 4];
    frustum->clipPlains[0].c = clip->mat[11] - clip->mat[ 8];
    frustum->clipPlains[0].d = clip->mat[15] - clip->mat[12];
    kmPlaneNormalize(&frustum->clipPlains[0], &frustum->clipPlains[0]);

    /* left plane */
    frustum->clipPlains[1].a = clip->mat[ 3] + clip->mat[ 0];
    frustum->clipPlains[1].b = clip->mat[ 7] + clip->mat[ 4];
    frustum->clipPlains[1].c = clip->mat[11] + clip->mat[ 8];
    frustum->clipPlains[1].d = clip->mat[15] + clip->mat[12];
    kmPlaneNormalize(&frustum->clipPlains[1], &frustum->clipPlains[1]);

    /* bottom plane */
    frustum->clipPlains[2].a = clip->mat[ 3] + clip->mat[ 1];
    frustum->clipPlains[2].b = clip->mat[ 7] + clip->mat[ 5];
    frustum->clipPlains[2].c = clip->mat[11] + clip->mat[ 9];
    frustum->clipPlains[2].d = clip->mat[15] + clip->mat[13];
    kmPlaneNormalize(&frustum->clipPlains[2], &frustum->clipPlains[2]);

    /* top plane  */
    frustum->clipPlains[3].a = clip->mat[ 3] - clip->mat[ 1];
    frustum->clipPlains[3].b = clip->mat[ 7] - clip->mat[ 5];
    frustum->clipPlains[3].c = clip->mat[11] - clip->mat[ 9];
    frustum->clipPlains[3].d = clip->mat[15] - clip->mat[13];
    kmPlaneNormalize(&frustum->clipPlains[3], &frustum->clipPlains[3]);

    /* rear plane */
    frustum->clipPlains[4].a = clip->mat[ 3] - clip->mat[ 2];
    frustum->clipPlains[4].b = clip->mat[ 7] - clip->mat[ 6];
    frustum->clipPlains[4].c = clip->mat[11] - clip->mat[10];
    frustum->clipPlains[4].d = clip->mat[15] - clip->mat[14];
    kmPlaneNormalize(&frustum->clipPlains[4], &frustum->clipPlains[4]);

    /* front plane */
    frustum->clipPlains[5].a = clip->mat[ 3] + clip->mat[ 2];
    frustum->clipPlains[5].b = clip->mat[ 7] + clip->mat[ 6];
    frustum->clipPlains[5].c = clip->mat[11] + clip->mat[10];
    frustum->clipPlains[5].d = clip->mat[15] + clip->mat[14];
    kmPlaneNormalize(&frustum->clipPlains[5], &frustum->clipPlains[5]);
}

static int testSphereInFrustum(struct lite3d_frustum *frustum,
    const struct lite3d_bouding_vol *vol)
{
    int i;
    for (i = 0; i < 6; ++i)
    {
        // frustum test not pass if sphere behind a plane */
        if (kmPlaneDistance(&frustum->clipPlains[i], &vol->sphereCenter) <= -vol->radius)
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

static int testBoxInFrustum(struct lite3d_frustum *frustum,
    const struct lite3d_bouding_vol *vol)
{
    int i, j;
    for (i = 0; i < 6; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            if (kmPlaneDistance(&frustum->clipPlains[i], &vol->box[j]) > 0)
                break;
        }

        if (j == 8)
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

int lite3d_frustum_test(struct lite3d_frustum *frustum,
    const struct lite3d_bouding_vol *vol)
{
    SDL_assert(frustum);
    SDL_assert(vol);

    /* bouding volume not setup, always true */
    if (vol->radius == 0.0f)
        return LITE3D_TRUE;

    if (!testSphereInFrustum(frustum, vol))
        return LITE3D_FALSE;

    return testBoxInFrustum(frustum, vol);
}

void lite3d_bouding_vol_setup(struct lite3d_bouding_vol *vol,
    const kmVec3 *vmin, const kmVec3 *vmax)
{
    float l, w, h;
    kmVec3 center;
    
    SDL_assert(vmin);
    SDL_assert(vmax);
    SDL_assert(vol);

    l = vmax->x - vmin->x;
    w = vmax->y - vmin->y;
    h = vmax->z - vmin->z;

    vol->box[0] = *vmin;

    vol->box[1].x = vmin->x;
    vol->box[1].y = vmin->y;
    vol->box[1].z = vmin->z + h;

    vol->box[2].x = vmin->x;
    vol->box[2].y = vmin->y + w;
    vol->box[2].z = vmin->z + h;

    vol->box[3].x = vmin->x;
    vol->box[3].y = vmin->y + w;
    vol->box[3].z = vmin->z;

    vol->box[4].x = vmin->x + l;
    vol->box[4].y = vmin->y;
    vol->box[4].z = vmin->z;

    vol->box[5].x = vmin->x + l;
    vol->box[5].y = vmin->y;
    vol->box[5].z = vmin->z + h;

    vol->box[6].x = vmin->x + l;
    vol->box[6].y = vmin->y + w;
    vol->box[6].z = vmin->z;

    vol->box[7] = *vmax;

    /* calc center vector */
    center.x = l / 2;
    center.y = w / 2;
    center.z = h / 2;

    kmVec3Add(&vol->sphereCenter, vmin, &center);
    vol->radius = kmVec3Length(&center);
}

void lite3d_bouding_vol_translate(struct lite3d_bouding_vol *volOut,
    const struct lite3d_bouding_vol *volIn, const struct kmMat4 *tr)
{
    int i;
    kmVec3 center;
    
    SDL_assert(volOut);
    SDL_assert(volIn);
    SDL_assert(tr);
    
    for (i = 0; i < 8; ++i)
    {
        kmVec3MultiplyMat4(&volOut->box[i], &volIn->box[i], tr);
    }

    kmVec3MultiplyMat4(&volOut->sphereCenter, &volIn->sphereCenter, tr);
    /* calculate radius */
    kmVec3Subtract(&center, &volIn->sphereCenter, &volIn->box[0]);
    kmVec3MultiplyMat4(&center, &center, tr);
    volOut->radius = kmVec3Length(&center);
}


float lite3d_frustum_distance(struct lite3d_frustum *frustum, 
    const kmVec3 *point)
{
    SDL_assert(frustum);
    SDL_assert(point);
    
    return kmPlaneDistance(&frustum->clipPlains[5], point);
}

float lite3d_frustum_distance_bouding_vol(struct lite3d_frustum *frustum, 
    const struct lite3d_bouding_vol *vol)
{
    SDL_assert(frustum);
    SDL_assert(vol);
       
    return lite3d_frustum_distance(frustum, &vol->sphereCenter);
}
