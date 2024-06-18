/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#ifndef LITE3D_CAMERA_H
#define	LITE3D_CAMERA_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_scene_node.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_frustum.h>

#define LITE3D_CAMERA_LINK_POSITION    0x01
#define LITE3D_CAMERA_LINK_ORIENTATION 0x02

typedef struct lite3d_camera
{
    /* Нода камеры */
    lite3d_scene_node cameraNode;
    /* Матрица перехода в view-space из world-space */
    kmMat4 view;
    /* Матрица перехода в screen-space из view-space */
    kmMat4 projection;
    /* Матрица перехода в screen-space из world-space */
    kmMat4 screen;
    /* Признак прямоугольной проекции */
    uint8_t isOrtho;

    struct projectionParamsStruct
    {
        float znear;
        float zfar;
        float left;
        float right;
        float bottom;
        float top;
        float fovy;
        float aspect;
    } projectionParams;
    
    lite3d_frustum frustum;

    const lite3d_scene_node *linkNode;
    const lite3d_scene_node *trackNode;
    uint8_t linkType;
    void *userdata;
} lite3d_camera;

LITE3D_CEXPORT void lite3d_camera_init(lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_camera_link_to(lite3d_camera *camera, const lite3d_scene_node *target, uint8_t linkType);
LITE3D_CEXPORT void lite3d_camera_tracking(lite3d_camera *camera, const lite3d_scene_node *target);
LITE3D_CEXPORT void lite3d_camera_update_view(lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_camera_compute_view(lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_camera_ortho(lite3d_camera *camera, float znear,
    float zfar, float left, float right, float bottom, float top);
LITE3D_CEXPORT void lite3d_camera_perspective(lite3d_camera *camera, float znear, float zfar, float fovy, float aspect);
/* lookat rotation, pointTo задан в локальной системе координат камеры */
LITE3D_CEXPORT void lite3d_camera_lookAt(lite3d_camera *camera, const kmVec3 *pointTo);
/* lookat rotation, pointTo задан в мировой системе координат */
LITE3D_CEXPORT void lite3d_camera_lookAt_world(lite3d_camera *camera, const kmVec3 *pointTo);
/* direction задан в локальной системе координат камеры */
LITE3D_CEXPORT void lite3d_camera_set_direction(lite3d_camera *camera, const kmVec3 *direction);
LITE3D_CEXPORT void lite3d_camera_direction(const lite3d_camera *camera, kmVec3 *vec);
/* Получение вектора направления в мировой системе координат */
LITE3D_CEXPORT void lite3d_camera_world_direction(const lite3d_camera *camera, kmVec3 *vec);
/* установка позиции в локальной системе координат */
LITE3D_CEXPORT void lite3d_camera_set_position(lite3d_camera *camera, const kmVec3 *position);
/* установка вращения в локальной системе координат */
LITE3D_CEXPORT void lite3d_camera_set_rotation(lite3d_camera *camera, const kmQuaternion *orietation);
/* Вращение в локальной системе координат */
LITE3D_CEXPORT void lite3d_camera_rotate(lite3d_camera *camera, const kmQuaternion *orietation);
// Доворот на углы в локальном пространстве камеры 
LITE3D_CEXPORT void lite3d_camera_yaw(lite3d_camera *camera, float angle);
LITE3D_CEXPORT void lite3d_camera_pitch(lite3d_camera *camera, float angle);
LITE3D_CEXPORT void lite3d_camera_roll(lite3d_camera *camera, float angle);
// Установка углов свободной камеры в локальном пространстве камеры
LITE3D_CEXPORT void lite3d_camera_set_yaw_pitch_roll(lite3d_camera *camera, float yaw, float pitch, float roll);
/* Перемещение в локальной системе координат */
LITE3D_CEXPORT void lite3d_camera_move(lite3d_camera *camera, const kmVec3 *value);
/* Перемещение в локальной системе координат относительно осей камеры  */
LITE3D_CEXPORT void lite3d_camera_move_relative(lite3d_camera *camera, const kmVec3 *value);
/* point задан в мировой системе координат */
LITE3D_CEXPORT float lite3d_camera_distance(const lite3d_camera *camera, const kmVec3 *point);
/* Получение позиции и вращения в мировой системе координат */
LITE3D_CEXPORT void lite3d_camera_world_position(const lite3d_camera *camera, kmVec3 *pos);
LITE3D_CEXPORT void lite3d_camera_world_rotation(const lite3d_camera *camera, kmQuaternion *q);

#endif	/* LITE3D_CAMERA_H */
