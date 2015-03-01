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

static void scene_recursive_render(lite3d_scene *scene, lite3d_scene_node *node, lite3d_camera *camera)
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
        child->recalc = recalcNode ? LITE3D_TRUE : child->recalc;
        if (child->enabled)
        {
            scene_recursive_render(scene, child, camera);
        }
    }

    /* apply camera and set modelview */
    lite3d_camera_to_node(camera, node);
    /* and now draw node */
    if (node->renderable)
    {
        if (scene->preRenderNode)
            scene->preRenderNode(scene, node);
        /*  render node directly.. may be any object of: mesh, 
            paticle system, skin, cubebox, etc. */
        if (node->doRenderNode)
            node->doRenderNode(node);

        if (scene->postRenderNode)
            scene->postRenderNode(scene, node);
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
    /* setup camera projection */
    lite3d_camera_update_view(camera);
    /* render scene tree */
    scene_recursive_render(scene, &scene->rootNode, camera);

    if (scene->postRender)
        scene->postRender(scene, camera);
}

void lite3d_scene_init(lite3d_scene *scene)
{
    SDL_assert(scene);
    memset(scene, 0, sizeof (lite3d_scene));
    /* root scene node */
    lite3d_scene_node_init(&scene->rootNode);
    lite3d_list_init(&scene->renderQueue);
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
    if(node->scene != scene)
        return LITE3D_FALSE;
    
    lite3d_list_unlink_link(&node->nodeLink);
    node->baseNode = NULL;
    node->scene = NULL;
    
    return LITE3D_TRUE;
}
