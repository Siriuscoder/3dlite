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

lite3d_scene_node *lite3d_scene_node_init(lite3d_scene_node *node)
{
    SDL_assert(node);
    lite3d_list_link_init(&node->link);
    kmMat4Identity(&node->modelView);
    kmQuaternionIdentity(&node->rotation);
    kmVec3Fill(&node->position, 0, 0, 0);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_set_position(lite3d_scene_node *node, kmVec3 *position)
{
    SDL_assert(node && position);
    node->position = *position;
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_set_rotation(lite3d_scene_node *node, kmQuaternion *quat)
{
    SDL_assert(node && quat);
    node->rotation = *quat;
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_add_position(lite3d_scene_node *node, kmVec3 *position)
{
    SDL_assert(node && position);
    kmVec3Add(&node->position, &node->position, position);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_add_rotation(lite3d_scene_node *node, kmQuaternion *quat)
{
    SDL_assert(node && quat);
    kmQuaternionMultiply(&node->rotation, &node->rotation, quat);
    node->recalc = LITE3D_TRUE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_set_matrix(lite3d_scene_node *node, kmMat4 *mat)
{
    SDL_assert(node && mat);
    node->modelView = *mat;
    /* matrix state validated */
    node->recalc = LITE3D_FALSE;

    return node;
}

lite3d_scene_node *lite3d_scene_node_matrix_recalc(lite3d_scene_node *node)
{
    kmMat4 quatMat;
    kmMat4Translation(&node->modelView, node->position.x, node->position.y, node->position.z);
    kmMat4RotationQuaternion(&quatMat, &node->rotation);
    kmMat4Multiply(&node->modelView, &node->modelView, &quatMat);

    return node;
}

void lite3d_scene_node_paint_frame_begin(lite3d_scene_node *node)
{
    SDL_assert(node);
    if(node->recalc)
    {
        lite3d_scene_node_matrix_recalc(node);
        node->recalc = LITE3D_FALSE;
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(node->modelView.mat);
}

void lite3d_scene_node_paint_frame_end(lite3d_scene_node *node)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}