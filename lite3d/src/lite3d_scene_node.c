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
#include <string.h>
#include <SDL_assert.h>
#include <lite3d/GL/glew.h>
#include <lite3d/lite3d_scene_node.h>

lite3d_scene_node *lite3d_scene_node_init(lite3d_scene_node *node)
{
    SDL_assert(node);
    memset(node, 0, sizeof (lite3d_scene_node));
    lite3d_list_link_init(&node->nodeLink);
    kmMat4Identity(&node->localView);
    kmMat4Identity(&node->worldView);
    kmMat4Identity(&node->modelView);
    kmQuaternionIdentity(&node->rotation);
    kmVec3Fill(&node->position, 0, 0, 0);
    kmVec3Fill(&node->scale, 1.0f, 1.0f, 1.0f);
    node->recalc = LITE3D_TRUE;
    node->invalidated = LITE3D_TRUE;
    node->rotationCentered = LITE3D_TRUE;
    node->isCamera = LITE3D_FALSE;
    node->renderable = LITE3D_TRUE;
    node->enabled = LITE3D_TRUE;
    node->visible = LITE3D_TRUE;
    lite3d_list_init(&node->childNodes);

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

lite3d_scene_node *lite3d_scene_node_rotate(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    kmQuaternionMultiply(&node->rotation, &node->rotation, quat);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_rotate_by(lite3d_scene_node *node, const kmQuaternion *quat)
{
    SDL_assert(node && quat);
    kmQuaternionMultiply(&node->rotation, quat, &node->rotation);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_rotate_angle(lite3d_scene_node *node, const kmVec3 *axis, float angle)
{
    kmQuaternion tmpQuat;
    SDL_assert(node && axis);
    kmQuaternionRotationAxisAngle(&tmpQuat, axis, angle);
    lite3d_scene_node_rotate(node, &tmpQuat);

    return node;
}

lite3d_scene_node *lite3d_scene_node_scale(lite3d_scene_node *node, const kmVec3 *scale)
{
    SDL_assert(node && scale);
    node->scale = *scale;
    node->recalc = LITE3D_TRUE;

    return node;
}

uint8_t lite3d_scene_node_update(lite3d_scene_node *node)
{
    uint8_t updated = LITE3D_FALSE;
    SDL_assert(node);

    if (node->recalc)
    {
        kmMat4 transMat;
        kmMat4 scaleMat;
        kmQuaternionNormalize(&node->rotation,
            &node->rotation);

        kmMat4RotationQuaternion(&node->localView, &node->rotation);
        kmMat4Translation(&transMat,
            node->isCamera ? -node->position.x : node->position.x,
            node->isCamera ? -node->position.y : node->position.y,
            node->isCamera ? -node->position.z : node->position.z);

        if (node->scale.x != 1.0f ||
            node->scale.y != 1.0f ||
            node->scale.z != 1.0f)
        {
            kmMat4Scaling(&scaleMat, node->scale.x,
                node->scale.y,
                node->scale.z);

            kmMat4Multiply(&transMat, &transMat, &scaleMat);
        }

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
        node->invalidated = LITE3D_TRUE;
        updated = LITE3D_TRUE;
    }

    return updated;
}