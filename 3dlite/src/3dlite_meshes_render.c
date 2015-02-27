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
#include <SDL_log.h>

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_meshes_render.h>

typedef struct lite3d_mqr_node
{
    lite3d_list_node unit;
    lite3d_mesh_node *node;
    lite3d_vao *vao;
} lite3d_mqr_node;
typedef struct lite3d_mqr_unit
{
    lite3d_list_node queued;
    lite3d_material *material;
    lite3d_list nodes;
} lite3d_mqr_unit;

static void do_mqr(struct lite3d_scene *scene)
{

}

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

static lite3d_mqr_unit *select_mqr_unit(lite3d_scene *scene, 
    lite3d_material *material)
{
    lite3d_mqr_unit *mqrUnit;
    lite3d_list_node *mqrUnitNode;

    for (mqrUnitNode = scene->renderQueue.l.next; 
        mqrUnitNode != &scene->renderQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        if(mqrUnit->material == material)
            return mqrUnit;
    }

    return NULL;
}

static lite3d_mqr_node *check_mqr_material_index_exist(lite3d_mqr_unit *unit, 
    lite3d_mesh_node *node, uint32_t index)
{
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next; 
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if(mqrNode->node == node && mqrNode->vao->materialIndex == index)
            return mqrNode;
    }

    return NULL;
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

void lite3d_scene_init_mqr(lite3d_scene *scene)
{
    lite3d_scene_init(scene);
    scene->doRender = do_mqr;
}

void lite3d_scene_purge_mqr(lite3d_scene *scene)
{

}

int lite3d_mesh_node_attach_material(lite3d_mesh_node *node,
    lite3d_material *material, uint32_t index)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_mqr_unit *mqrUnitTmp = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;
    lite3d_scene *scene = NULL;

    SDL_assert(node);

    scene = (lite3d_scene *)node->sceneNode.scene;

    for (mqrUnitNode = scene->renderQueue.l.next; 
        mqrUnitNode != &scene->renderQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnitTmp = MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        if(mqrUnitTmp->material == material)
            mqrUnit = mqrUnitTmp;

        /* check render unit node exist */
        if((mqrNode = check_mqr_material_index_exist(mqrUnitTmp, node, index)) != NULL)
            /* unlink it */
            lite3d_list_unlink_link(&mqrNode->unit);
    }

    if(mqrUnit == NULL)
    {
        mqrUnit = (lite3d_mqr_unit *)lite3d_calloc(sizeof(lite3d_mqr_unit));
        lite3d_list_init(&mqrUnit->nodes);
        lite3d_list_link_init(&mqrUnit->queued);
        lite3d_list_add_last_link(&mqrUnit->queued, &scene->renderQueue);
    }

    SDL_assert_release(mqrUnit);

    if(mqrNode == NULL)
    {
        mqrNode = (lite3d_mqr_node *)lite3d_calloc(sizeof(lite3d_mqr_node));
        lite3d_list_link_init(&mqrNode->unit);
        mqrNode->node = node;
        mqrNode->vao = lite3d_vao_get_by_index(node->vbo, index);
        if(!mqrNode->vao)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: Material index %d not found in mesh..", __FUNCTION__);
            lite3d_free(mqrNode);
            return LITE3D_FALSE;
        }
    }

    SDL_assert_release(mqrNode);
    /* relink render node */
    lite3d_list_add_last_link(&mqrNode->unit, &mqrUnit->nodes);
    return LITE3D_TRUE;
}

