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
#ifndef LITE3D_SCENE_NODE_H
#define	LITE3D_SCENE_NODE_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_kazmath.h>
#include <lite3d/lite3d_list.h>

typedef struct  lite3d_scene_node
{
    lite3d_list_node nodeLink;
    // Локальная трансформация обьекта, включает в себя транформацию позиции, маштаба и вращения заданные с помощью 
    // set_rotation и set_position и scale
    kmMat4 localMatrix;
    // Матрица преобразования из локальной системы координат в мировую систему координат (world space). 
    // Явзяется комбинацией матрицы worldMatrix базовой ноды и локальной матрицы трансформации localMatrix
    kmMat4 worldMatrix; 
    // Normal матрица, специальная матрица для корректного преобразования нормалей в случае масштабирования обьекта. 
    // Может быть использована для преобразования нормалей в мировую систему координат из локальной системы координат модели. 
    // Если масштабирование не применяется то для этого можно использовать worldMatrix.
    kmMat3 normalMatrix;
    // Локальное вращение обьекта
    kmQuaternion rotation;
    // Локальная позиция обьекта
    kmVec3 position;
    // Масштаб обьекта
    kmVec3 scale;
    uint8_t recalc;
    uint8_t invalidated;
    uint8_t rotationCentered;
    uint8_t isCamera;
    uint8_t renderable;
    uint8_t enabled;
    uint8_t visible;
    uint8_t frustumTest;
    struct lite3d_scene_node *baseNode;
    struct lite3d_list childNodes;
    void *scene;
    void *userdata;
} lite3d_scene_node;

LITE3D_CEXPORT void lite3d_scene_node_init(lite3d_scene_node *node);
LITE3D_CEXPORT void lite3d_scene_node_set_position(lite3d_scene_node *node, const kmVec3 *position);
LITE3D_CEXPORT void lite3d_scene_node_move(lite3d_scene_node *node, const kmVec3 *position);
LITE3D_CEXPORT void lite3d_scene_node_move_relative(lite3d_scene_node *node, const kmVec3 *vec);
LITE3D_CEXPORT void lite3d_scene_node_set_rotation(lite3d_scene_node *node, const kmQuaternion *quat);
LITE3D_CEXPORT void lite3d_scene_node_rotate(lite3d_scene_node *node, const kmQuaternion *quat);
LITE3D_CEXPORT void lite3d_scene_node_rotate_angle(lite3d_scene_node *node, const kmVec3 *axis, float angle);
LITE3D_CEXPORT void lite3d_scene_node_scale(lite3d_scene_node *node, const kmVec3 *scale);
LITE3D_CEXPORT void lite3d_scene_node_get_world_position(const lite3d_scene_node *node, kmVec3 *pos);
LITE3D_CEXPORT void lite3d_scene_node_get_world_rotation(const lite3d_scene_node *node, kmQuaternion *q);
LITE3D_CEXPORT void lite3d_scene_node_rotate_y(lite3d_scene_node *node, float angle);
LITE3D_CEXPORT void lite3d_scene_node_rotate_x(lite3d_scene_node *node, float angle);
LITE3D_CEXPORT void lite3d_scene_node_rotate_z(lite3d_scene_node *node, float angle);

LITE3D_CEXPORT uint8_t lite3d_scene_node_update(lite3d_scene_node *node);

#endif
