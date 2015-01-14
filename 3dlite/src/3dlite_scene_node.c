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
#include <SDL_assert.h>
#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_scene_node.h>

lite3d_scene_node *lite3d_scene_node_init(lite3d_scene_node *node, lite3d_scene_node *baseNode)
{
    SDL_assert(node);
    lite3d_list_link_init(&node->sceneLink);
    kmMat4Identity(&node->localView);
    kmMat4Identity(&node->worldView);
    kmQuaternionIdentity(&node->rotation);
    kmVec3Fill(&node->position, 0, 0, 0);
    node->recalc = LITE3D_TRUE;
    node->baseNode = baseNode;
    node->rotationCentered = LITE3D_TRUE;
    node->renderable = LITE3D_TRUE;
    node->enabled = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_set_position(lite3d_scene_node *node, const kmVec3 *position)
{
    SDL_assert(node && position);
    node->position = *position;
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_set_rotation(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    node->rotation = *quat;
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_move(lite3d_scene_node *node, const kmVec3 *position)
{
    SDL_assert(node && position);
    kmVec3Add(&node->position, &node->position, position);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_rotate_quat(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    kmQuaternionMultiply(&node->rotation, &node->rotation, quat);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_rotate_angle(lite3d_scene_node *node, const kmVec3 *axis, float angle)
{
    kmQuaternion tmpQuat;
    SDL_assert(node && axis);
    kmQuaternionRotationAxisAngle(&tmpQuat, axis, angle);
    lite3d_scene_node_rotate_quat(node, &tmpQuat);

    return node;
}

uint8_t lite3d_scene_node_update(lite3d_scene_node *node)
{
    uint8_t updated = LITE3D_FALSE;
    SDL_assert(node);

    if (node->recalc)
    {
        kmMat4 transMat;
        kmQuaternionNormalize(&node->rotation,
            &node->rotation);

        kmMat4RotationQuaternion(&node->localView, &node->rotation);
        kmMat4Translation(&transMat,
            node->position.x,
            node->position.y,
            node->position.z);

        if (node->rotationCentered)
            kmMat4Multiply(&node->localView,
            &transMat, &node->localView);
        else
            kmMat4Multiply(&node->localView,
            &node->localView, &transMat);

        if (node->baseNode)
        {
            kmMat4Multiply(&node->worldView,
                &node->baseNode->worldView, &node->localView);
        }
        else
        {
            node->worldView = node->localView;
        }

        node->recalc = LITE3D_FALSE;
        updated = LITE3D_TRUE;
    }

    return updated;
}