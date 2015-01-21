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
#ifndef LITE3D_SCENE_NODE_H
#define	LITE3D_SCENE_NODE_H

#include <3dlite/kazmath/mat3.h>
#include <3dlite/kazmath/mat4.h>
#include <3dlite/kazmath/vec3.h>
#include <3dlite/kazmath/quaternion.h>

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_list.h>

typedef struct  lite3d_scene_node
{
    lite3d_list_node nodeLink;
    kmMat4 localView;
    kmMat4 worldView;
    kmQuaternion rotation;
    kmVec3 position;
    kmVec3 scale;
    uint8_t recalc;
    uint8_t rotationCentered;
    uint8_t renderable;
    uint8_t enabled;
    struct lite3d_scene_node *baseNode;
    struct lite3d_list childNodes;
    void (*doRenderNode)(struct lite3d_scene_node *node);
    void *scene;
} lite3d_scene_node;

LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_init(lite3d_scene_node *node);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_set_position(lite3d_scene_node *node, const kmVec3 *position);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_move(lite3d_scene_node *node, const kmVec3 *position);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_set_rotation(lite3d_scene_node *node, const kmQuaternion *quat);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_rotate(lite3d_scene_node *node, const kmQuaternion *quat);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_rotate_angle(lite3d_scene_node *node, const kmVec3 *axis, float angle);
LITE3D_CEXPORT lite3d_scene_node *lite3d_scene_node_scale(lite3d_scene_node *node, const kmVec3 *scale);

LITE3D_CEXPORT uint8_t lite3d_scene_node_update(lite3d_scene_node *node);

#endif