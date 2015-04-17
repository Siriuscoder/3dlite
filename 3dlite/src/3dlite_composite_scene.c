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
#include <3dlite/3dlite_composite_scene.h>

typedef struct lite3d_mqr_node
{
    lite3d_list_node unit;
    lite3d_composite_scene_node *node;
    lite3d_vao *vao;
} lite3d_mqr_node;

typedef struct lite3d_mqr_unit
{
    lite3d_list_node queued;
    lite3d_material *material;
    lite3d_list nodes;
} lite3d_mqr_unit;

static void mqr_unit_render(lite3d_material_pass *pass, void *data)
{
    lite3d_mqr_unit *mqrUnit = (lite3d_mqr_unit *) data;
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;
    lite3d_vao *prevVao = NULL;
    lite3d_scene *base = NULL;
    lite3d_composite_scene *scene = NULL;

    for (mqrListNode = mqrUnit->nodes.l.next;
        mqrListNode != &mqrUnit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (!mqrNode->node->node.enabled)
            continue;
        if (!mqrNode->node->node.renderable)
            continue;

        /* bind vao */
        if (prevVao != mqrNode->vao)
        {
            lite3d_vao_bind(mqrNode->vao);
            prevVao = mqrNode->vao;
        }

        base = (lite3d_scene *) mqrNode->node->node.scene;
        SDL_assert(base);

        scene = LITE3D_MEMBERCAST(lite3d_composite_scene, base, scene);
        /* setup global parameters (viewmodel) */
        lite3d_shader_set_modelview_matrix(&mqrNode->node->node.cameraView);
        /* setup changed uniforms parameters */
        lite3d_material_pass_set_params(mqrUnit->material, pass, LITE3D_TRUE);
        /* do render batch */
        if (scene->drawBatch)
            scene->drawBatch(scene, mqrNode->node,
            mqrNode->vao, mqrUnit->material);
        lite3d_vao_draw(mqrNode->vao);
        base->stats.batches++;
        base->stats.trianglesRendered += mqrNode->vao->elementsCount;
        base->stats.verticesRendered += mqrNode->vao->verticesCount;
    }

    if (base)
        base->stats.materialPassed++;

    lite3d_vao_unbind(prevVao);
}

static void mqr_render(struct lite3d_scene *base, lite3d_camera *camera)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_composite_scene *scene = NULL;

    scene = LITE3D_MEMBERCAST(lite3d_composite_scene, base, scene);
    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);

        lite3d_material_pass_render(mqrUnit->material, camera->materialPass,
            mqr_unit_render, mqrUnit);
        base->stats.materialBlocks++;
        base->stats.textureUnitsBinded += mqrUnit->material->textureUnitsBinded;
    }
}

static lite3d_mqr_node *check_mqr_material_index_exist(lite3d_mqr_unit *unit,
    lite3d_composite_scene_node *node, uint32_t index)
{
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (mqrNode->node == node && mqrNode->vao->materialIndex == index)
            return mqrNode;
    }

    return NULL;
}

static lite3d_mqr_node *check_mqr_node_exist(lite3d_mqr_unit *unit,
    lite3d_composite_scene_node *node)
{
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (mqrNode->node == node)
            return mqrNode;
    }

    return NULL;
}

static void mqr_unit_add_node(lite3d_mqr_unit *unit, lite3d_mqr_node *node)
{
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    /* insert node info list group by vao */
    /* it guarantee what node will be sorted by vao */
    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (mqrNode->vao == node->vao)
        {
            lite3d_list_insert_after_link(&node->unit, &mqrNode->unit);
            return;
        }
    }

    /* vao contained in node not found.. */
    lite3d_list_add_last_link(&node->unit, &unit->nodes);
}

void lite3d_composite_scene_node_init(lite3d_composite_scene_node *node, lite3d_vbo *vbo)
{
    SDL_assert(node);

    lite3d_scene_node_init(&node->node);
    node->vbo = vbo;

    node->node.enabled = LITE3D_TRUE;
    node->node.renderable = LITE3D_TRUE;
    node->node.rotationCentered = LITE3D_TRUE;
}

void lite3d_composite_scene_init(lite3d_composite_scene *scene)
{
    lite3d_scene_init(&scene->scene);
    lite3d_list_init(&scene->renderUnitQueue);
    scene->scene.doRender = mqr_render;
}

void lite3d_composite_scene_purge(lite3d_composite_scene *scene)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;
    lite3d_list_node *mqrListNode = NULL;

    SDL_assert(scene);
    while ((mqrUnitNode = lite3d_list_remove_first_link(&scene->renderUnitQueue)) != NULL)
    {
        mqrUnit = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        while ((mqrListNode = lite3d_list_remove_first_link(&mqrUnit->nodes)) != NULL)
        {
            mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }

        lite3d_free_pooled(LITE3D_POOL_NO1, mqrUnit);
    }
}

int lite3d_composite_scene_node_attach_material(
    lite3d_composite_scene_node *node, lite3d_material *material, uint32_t index)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_mqr_unit *mqrUnitTmp = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;
    lite3d_scene *base = NULL;
    lite3d_composite_scene *scene = NULL;

    SDL_assert(node);

    base = (lite3d_scene *) node->node.scene;
    scene = LITE3D_MEMBERCAST(lite3d_composite_scene, base, scene);

    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnitTmp = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        if (mqrUnitTmp->material == material)
            mqrUnit = mqrUnitTmp;

        /* check render unit node exist */
        if ((mqrNode = check_mqr_material_index_exist(mqrUnitTmp, node, index)) != NULL)
            /* unlink it */
            lite3d_list_unlink_link(&mqrNode->unit);
    }

    if (mqrUnit == NULL)
    {
        mqrUnit = (lite3d_mqr_unit *) lite3d_calloc_pooled(LITE3D_POOL_NO1,
            sizeof (lite3d_mqr_unit));
        lite3d_list_init(&mqrUnit->nodes);
        lite3d_list_link_init(&mqrUnit->queued);
        mqrUnit->material = material;

        lite3d_list_add_last_link(&mqrUnit->queued, &scene->renderUnitQueue);
    }

    SDL_assert_release(mqrUnit);

    if (mqrNode == NULL)
    {
        mqrNode = (lite3d_mqr_node *) lite3d_calloc_pooled(LITE3D_POOL_NO1,
            sizeof (lite3d_mqr_node));
        lite3d_list_link_init(&mqrNode->unit);
        mqrNode->node = node;
        mqrNode->vao = lite3d_vao_get_by_index(node->vbo, index);
        if (!mqrNode->vao)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: Material index %d not found in mesh..", __FUNCTION__);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
            return LITE3D_FALSE;
        }
    }

    SDL_assert_release(mqrNode);
    /* relink render node */
    mqr_unit_add_node(mqrUnit, mqrNode);
    return LITE3D_TRUE;
}

int lite3d_composite_scene_add_node(
    lite3d_composite_scene *scene,
    lite3d_composite_scene_node *node,
    lite3d_scene_node *baseNode)
{
    return lite3d_scene_node_add(&scene->scene, &node->node, baseNode);
}

int lite3d_composite_scene_remove(lite3d_composite_scene *scene,
    lite3d_composite_scene_node *node)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;

    if (!lite3d_scene_node_remove(&scene->scene, &node->node))
        return LITE3D_FALSE;

    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        /* check render unit node exist */
        while ((mqrNode = check_mqr_node_exist(mqrUnit, node)) != NULL)
        {
            /* unlink it */
            lite3d_list_unlink_link(&mqrNode->unit);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }
    }

    return LITE3D_TRUE;
}

