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

static void lite3d_scene_recursive_render(lite3d_scene *scene, lite3d_scene_node *node, lite3d_camera *camera)
{
    lite3d_list_node *nodeLink;
    lite3d_scene_node *child;
    uint8_t recalcNode;

    recalcNode = lite3d_scene_node_update(node);
    /* render all childrens firts */
    for (nodeLink = node->childNodes.l.next; 
        nodeLink != &node->childNodes.l; nodeLink = lite3d_list_next(nodeLink))
    {
        child = MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink);
        child->recalc = recalcNode;
        if(child->enabled)
        {
            lite3d_scene_recursive_render(scene, child, camera);
        }
    }

    /* apply camera and set modelview */
    lite3d_camera_to_node(camera, node);
    /* and now draw node */
    if(node->renderable)
    {
        node->doRenderNode(node);
        scene->stats.objectsRendered++;
    }
}

void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera)
{
    SDL_assert(scene && camera);
    /* clean statistic */
    memset(&scene->stats, 0, sizeof(scene->stats));

    if(scene->preRender)
        scene->preRender(scene, camera);
    /* setup camera projection */
    lite3d_camera_update_view(camera);
    /* render scene tree */
    lite3d_scene_recursive_render(scene, &scene->rootNode, camera);

    if(scene->postRender)
        scene->postRender(scene, camera);
}
