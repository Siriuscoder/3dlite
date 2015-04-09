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
#include <stdlib.h>
#include <string.h>
#include <SDL_assert.h>

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_scene.h>

static void scene_recursive_nodes_update(lite3d_scene *scene, 
    lite3d_scene_node *node, lite3d_camera *camera)
{
    lite3d_list_node *nodeLink;
    lite3d_scene_node *child;
    uint8_t recalcNode;

    recalcNode = lite3d_scene_node_update(node);
    /* recalc modelview matrix */
    kmMat4Multiply(&node->cameraView, &camera->cameraNode.localView, &node->worldView);
    /* render all childrens firts */
    for (nodeLink = node->childNodes.l.next;
        nodeLink != &node->childNodes.l; nodeLink = lite3d_list_next(nodeLink))
    {
        child = LITE3D_MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink);
        child->recalc = recalcNode ? LITE3D_TRUE : child->recalc;
        if (child->enabled)
        {
            scene_recursive_nodes_update(scene, child, camera);
        }
    }

    if (node->renderable)
    {
        scene->stats.objectsRendered++;
    }
}

void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera)
{
    SDL_assert(scene && camera);
    /* clean statistic */
    memset(&scene->stats, 0, sizeof (scene->stats));

    if (scene->preRender)
        scene->preRender(scene, camera);
    /* update camera projection & transformation */
    lite3d_camera_update_view(camera);
    /* update scene tree */
    scene_recursive_nodes_update(scene, &scene->rootNode, camera);
    /* render scene */
    if (scene->doRender)
        scene->doRender(scene, camera);

    if (scene->postRender)
        scene->postRender(scene, camera);
}

void lite3d_scene_init(lite3d_scene *scene)
{
    SDL_assert(scene);
    memset(scene, 0, sizeof (lite3d_scene));
    /* root scene node */
    lite3d_scene_node_init(&scene->rootNode);
    lite3d_list_init(&scene->renderUnitQueue);
    /* never render this node */
    scene->rootNode.renderable = LITE3D_FALSE;
}

int lite3d_scene_node_add(lite3d_scene *scene, lite3d_scene_node *node,
    lite3d_scene_node *baseNode)
{
    SDL_assert(scene && node);

    /* mean root node */
    if (!baseNode)
        baseNode = &scene->rootNode;
    else
    {
        if (baseNode->scene != scene)
            return LITE3D_FALSE;
    }

    node->baseNode = baseNode;
    node->scene = scene;
    lite3d_list_add_last_link(&node->nodeLink, &baseNode->childNodes);
    return LITE3D_TRUE;
}

int lite3d_scene_node_remove(lite3d_scene *scene, lite3d_scene_node *node)
{
    if (node->scene != scene)
        return LITE3D_FALSE;

    lite3d_list_unlink_link(&node->nodeLink);
    node->baseNode = NULL;
    node->scene = NULL;

    return LITE3D_TRUE;
}
