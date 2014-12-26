/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#ifndef LITE3D_CAMERA_H
#define	LITE3D_CAMERA_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_scene_node.h>

/* compatible with GL */
#define LITE3D_POLYMODE_POINT        0x1B00
#define LITE3D_POLYMODE_LINE         0x1B01
#define LITE3D_POLYMODE_FILL         0x1B02

typedef struct lite3d_camera
{
    lite3d_scene_node cameraNode;
    kmMat4 projection;

    uint8_t cullBackFaces;
    uint16_t polygonMode;
    uint8_t isOrtho;

    union
    {
        struct
        {
            float near;
            float far;
            float left;
            float right;
            float bottom;
            float top;
        } ortho;

        struct
        {
            float znear;
            float zfar;
            float fovy;
            float aspect;
        } perspective;
    } projectionParams;
} lite3d_camera;

LITE3D_CEXPORT void lite3d_camera_init(lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_camera_apply(lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_camera_ortho(lite3d_camera *camera, float near,
    float far, float left, float right, float bottom, float top);
LITE3D_CEXPORT void lite3d_camera_perspective(lite3d_camera *camera, float znear,
    float zfar, float fovy, float aspect);
LITE3D_CEXPORT void lite3d_camera_lookAt(lite3d_camera *camera, kmVec3 *pointTo);

#endif	/* LITE3D_CAMERA_H */

