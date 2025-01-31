/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
#include <string.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_scene_node.h>

void lite3d_scene_node_init(lite3d_scene_node *node)
{
    SDL_assert(node);
    memset(node, 0, sizeof (lite3d_scene_node));
    lite3d_list_link_init(&node->nodeLink);
    kmMat4Identity(&node->localMatrix);
    kmMat4Identity(&node->worldMatrix);
    kmMat3Identity(&node->normalMatrix);
    kmQuaternionIdentity(&node->rotation);
    kmVec3Fill(&node->position, 0, 0, 0);
    kmVec3Fill(&node->scale, 1.0f, 1.0f, 1.0f);
    node->recalc = LITE3D_TRUE;
    node->invalidated = LITE3D_TRUE;
    node->rotationCentered = LITE3D_TRUE;
    node->isCamera = LITE3D_FALSE;
    node->renderable = LITE3D_FALSE;
    node->enabled = LITE3D_TRUE;
    node->visible = LITE3D_TRUE;
    node->frustumTest = LITE3D_TRUE;
    lite3d_list_init(&node->childNodes);
}

void lite3d_scene_node_set_position(lite3d_scene_node *node, const kmVec3 *position)
{
    SDL_assert(node && position);
    node->position = *position;
    node->recalc = LITE3D_TRUE;
}

void lite3d_scene_node_set_rotation(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    node->rotation = *quat;
    node->recalc = LITE3D_TRUE;
}

void lite3d_scene_node_move(lite3d_scene_node *node, const kmVec3 *position)
{
    SDL_assert(node && position);
    kmVec3Add(&node->position, &node->position, position);
    node->recalc = LITE3D_TRUE;
}

void lite3d_scene_node_move_relative(lite3d_scene_node *node, const kmVec3 *vec)
{
    kmVec3 relative;

    SDL_assert(node && vec);
    kmQuaternionMultiplyVec3(&relative, &node->rotation, vec);
    lite3d_scene_node_move(node, &relative);
}

void lite3d_scene_node_rotate(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    kmQuaternionMultiply(&node->rotation, &node->rotation, quat);
    node->recalc = LITE3D_TRUE;
}

void lite3d_scene_node_rotate_angle(lite3d_scene_node *node, const kmVec3 *axis, float angle)
{
    kmQuaternion tmpQuat;
    SDL_assert(node && axis);
    kmQuaternionRotationAxisAngle(&tmpQuat, axis, angle);
    lite3d_scene_node_rotate(node, &tmpQuat);
}

void lite3d_scene_node_scale(lite3d_scene_node *node, const kmVec3 *scale)
{
    SDL_assert(node && scale);
    node->scale = *scale;
    node->recalc = LITE3D_TRUE;
}

void lite3d_scene_node_get_world_position(const lite3d_scene_node *node, kmVec3 *pos)
{
    SDL_assert(node && pos);
    kmMat4ExtractPosition(pos, &node->worldMatrix);
}

void lite3d_scene_node_get_world_rotation(const lite3d_scene_node *node, kmQuaternion *q)
{
    kmMat3 worldRotation;

    SDL_assert(node && q);
    kmMat4ExtractRotation(&worldRotation, &node->worldMatrix);
    kmQuaternionRotationMatrix(q, &worldRotation);
}

void lite3d_scene_node_rotate_y(lite3d_scene_node *node, float angle)
{
    SDL_assert(node);
    lite3d_scene_node_rotate_angle(node, &KM_VEC3_POS_Y, angle);
}

void lite3d_scene_node_rotate_x(lite3d_scene_node *node, float angle)
{
    SDL_assert(node);
    lite3d_scene_node_rotate_angle(node, &KM_VEC3_POS_X, angle);
}

void lite3d_scene_node_rotate_z(lite3d_scene_node *node, float angle)
{
    SDL_assert(node);
    lite3d_scene_node_rotate_angle(node, &KM_VEC3_POS_Z, angle);
}

static void scene_node_update(lite3d_scene_node *node)
{
    kmMat4 transMat;
    kmMat4 scaleMat;
    kmQuaternionNormalize(&node->rotation, &node->rotation);
    kmMat4RotationQuaternion(&node->localMatrix, &node->rotation);
    kmMat4Translation(&transMat, node->position.x, node->position.y, node->position.z);
    
    if (!kmAlmostEqual(node->scale.x, 1.0f) || 
        !kmAlmostEqual(node->scale.y, 1.0f) || 
        !kmAlmostEqual(node->scale.z, 1.0f))
    {
        kmMat4Scaling(&scaleMat, node->scale.x, node->scale.y, node->scale.z);
        kmMat4Multiply(&transMat, &transMat, &scaleMat);
    }

    if (node->rotationCentered)
    {
        kmMat4Multiply(&node->localMatrix, &transMat, &node->localMatrix);
    }
    else
    {
        kmMat4Multiply(&node->localMatrix, &node->localMatrix, &transMat);
    }

    if (node->baseNode)
    {
        kmMat4Multiply(&node->worldMatrix, &node->baseNode->worldMatrix, &node->localMatrix);
    }
    else
    {
        node->worldMatrix = node->localMatrix;
    }
    
    kmMat3NormalMatrix(&node->normalMatrix, &node->worldMatrix);
}

uint8_t lite3d_scene_node_update(lite3d_scene_node *node)
{
    uint8_t updated = LITE3D_FALSE;
    SDL_assert(node);
    
    if (node->recalc)
    {
        scene_node_update(node);
        node->recalc = LITE3D_FALSE;
        node->invalidated = LITE3D_TRUE;
        updated = LITE3D_TRUE;
    }

    return updated;
}
