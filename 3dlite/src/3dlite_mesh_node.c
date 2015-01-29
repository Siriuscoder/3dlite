/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_mesh_node.h>
#include <3dlite/3dlite_scene.h>

static void render_mesh_node(struct lite3d_scene_node *node)
{
    lite3d_mesh_node *meshNode;
    lite3d_scene *scene;
    meshNode = MEMBERCAST(lite3d_mesh_node, node, sceneNode);
    scene = (lite3d_scene *)meshNode->sceneNode.scene;

    lite3d_vbo_draw(meshNode->vbo);
    scene->stats.batches += meshNode->vbo->vaosCount;
    scene->stats.materialBlocks += meshNode->vbo->vaosCount;
    scene->stats.trianglesRendered += meshNode->vbo->verticesCount;
}

void lite3d_mesh_node_init(lite3d_mesh_node *node, lite3d_vbo *vbo)
{
    SDL_assert(node);

    lite3d_scene_node_init(&node->sceneNode);
    node->vbo = vbo;

    node->sceneNode.doRenderNode = render_mesh_node;
    node->sceneNode.enabled = LITE3D_TRUE;
    node->sceneNode.renderable = LITE3D_TRUE;
    node->sceneNode.rotationCentered = LITE3D_TRUE;
}