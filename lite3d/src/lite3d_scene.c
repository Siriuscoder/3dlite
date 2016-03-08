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
#include <stdlib.h>
#include <string.h>
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_scene.h>

typedef struct lite3d_mqr_node
{
    lite3d_list_node unit;
    lite3d_scene_node *node;
    lite3d_mesh_chunk *meshChunk;
    uint32_t instancesCount;
    lite3d_bouding_vol boudingVol;
    float distanceToCamera;
} lite3d_mqr_node;

typedef struct lite3d_mqr_unit
{
    lite3d_list_node queued;
    lite3d_material *material;
    lite3d_list nodes;
    lite3d_camera *currentCamera;
} lite3d_mqr_unit;

static void mqr_unit_render(lite3d_material_pass *pass, void *data)
{
    lite3d_mqr_unit *mqrUnit = (lite3d_mqr_unit *) data;
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;
    lite3d_mesh_chunk *prevVao = NULL;
    lite3d_scene *scene = NULL;

    for (mqrListNode = mqrUnit->nodes.l.next;
        mqrListNode != &mqrUnit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);

        SDL_assert(mqrNode->node);
        SDL_assert(mqrUnit->currentCamera);

        scene = (lite3d_scene *) mqrNode->node->scene;
        SDL_assert(scene);

        /* recalc bouding volume if node begin invalidated (position or rotation changed)*/
        if (mqrNode->node->invalidated)
        {
            lite3d_bouding_vol_translate(&mqrNode->boudingVol,
                &mqrNode->meshChunk->boudingVol,
                &mqrNode->node->worldView);
        }

        if (mqrNode->node->invalidated || mqrUnit->currentCamera->cameraNode.invalidated)
        {
            lite3d_frustum_distance_bouding_vol(&mqrUnit->currentCamera->frustum,
                &mqrNode->boudingVol);
        }

        if (!mqrNode->node->renderable)
            continue;
        if (!mqrNode->node->enabled)
            continue;

        scene->stats.batchesTotal++;
        /* frustum test */
        if (mqrNode->node->frustumTest && !lite3d_frustum_test(&mqrUnit->currentCamera->frustum, &mqrNode->boudingVol))
        {
            if (scene->nodeOutOfFrustum)
                scene->nodeOutOfFrustum(scene, mqrNode->node,
                mqrNode->meshChunk, mqrUnit->material, &mqrNode->boudingVol);
            continue;
        }

        if (mqrNode->node->frustumTest && scene->nodeInFrustum)
            scene->nodeInFrustum(scene, mqrNode->node,
            mqrNode->meshChunk, mqrUnit->material, &mqrNode->boudingVol);

        /* bind meshChunk */
        if (prevVao != mqrNode->meshChunk)
        {
            lite3d_mesh_chunk_bind(mqrNode->meshChunk);
            prevVao = mqrNode->meshChunk;
        }

        /* setup global parameters (viewmodel) */
        lite3d_shader_set_modelview_matrix(&mqrNode->node->modelView);
        lite3d_shader_set_model_matrix(&mqrNode->node->worldView);
        /* setup changed uniforms parameters */
        lite3d_material_pass_set_params(mqrUnit->material, pass, LITE3D_FALSE);
        /* notify render batch */
        if (scene->beginDrawBatch)
            scene->beginDrawBatch(scene, mqrNode->node,
            mqrNode->meshChunk, mqrUnit->material);
        /* do render batch */
        lite3d_mesh_chunk_draw(mqrNode->meshChunk, mqrNode->instancesCount);

        scene->stats.batchesCalled++;
        scene->stats.trianglesRendered += mqrNode->meshChunk->vao.elementsCount;
        scene->stats.verticesRendered += mqrNode->meshChunk->vao.verticesCount;
    }

    if (scene)
        scene->stats.materialPassed++;

    if (prevVao)
        lite3d_mesh_chunk_unbind(prevVao);
}

static void mqr_render(struct lite3d_scene *scene, lite3d_camera *camera, uint16_t pass)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;

    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        mqrUnit->currentCamera = camera;

        lite3d_material_pass_render(mqrUnit->material, pass,
            mqr_unit_render, mqrUnit);
        scene->stats.materialBlocks++;
        scene->stats.textureUnitsBinded += mqrUnit->material->textureUnitsBinded;
    }
}

static lite3d_mqr_node *mqr_check_mesh_chunk_exist(lite3d_mqr_unit *unit,
    lite3d_scene_node *node, lite3d_mesh_chunk *meshChunk)
{
    lite3d_mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (mqrNode->node == node && mqrNode->meshChunk == meshChunk)
            return mqrNode;
    }

    return NULL;
}

static lite3d_mqr_node *mqr_check_node_exist(lite3d_mqr_unit *unit,
    lite3d_scene_node *node)
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

    /* insert node info list group by meshChunk */
    /* it guarantee what node will be sorted by meshChunk */
    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(lite3d_mqr_node, mqrListNode, unit);
        if (mqrNode->meshChunk == node->meshChunk)
        {
            lite3d_list_insert_after_link(&node->unit, &mqrNode->unit);
            return;
        }
    }

    /* meshChunk contained in node not found.. */
    lite3d_list_add_last_link(&node->unit, &unit->nodes);
}

static void scene_recursive_nodes_update(lite3d_scene *scene,
    lite3d_scene_node *node, lite3d_camera *camera)
{
    lite3d_list_node *nodeLink;
    lite3d_scene_node *child;
    uint8_t recalcNode;

    recalcNode = lite3d_scene_node_update(node);
    /* recalc modelview matrix */
    kmMat4Multiply(&node->modelView, &camera->cameraNode.localView, &node->worldView);
    /* render all childrens firts */
    for (nodeLink = node->childNodes.l.next;
        nodeLink != &node->childNodes.l; nodeLink = lite3d_list_next(nodeLink))
    {
        child = LITE3D_MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink);
        child->recalc = recalcNode ? LITE3D_TRUE : child->recalc;
        scene_recursive_nodes_update(scene, child, camera);
    }

    scene->stats.nodesTotal++;
}

static void scene_recursive_nodes_validate(lite3d_scene_node *node)
{
    lite3d_list_node *nodeLink;

    /* render all childrens firts */
    for (nodeLink = node->childNodes.l.next;
        nodeLink != &node->childNodes.l; nodeLink = lite3d_list_next(nodeLink))
        scene_recursive_nodes_validate(LITE3D_MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink));

    node->invalidated = LITE3D_FALSE;
}

void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera, uint16_t pass)
{
    SDL_assert(scene && camera);
    /* clean statistic */
    memset(&scene->stats, 0, sizeof (scene->stats));

    if (scene->beginSceneRender)
        scene->beginSceneRender(scene, camera);
    /* update camera projection & transformation */
    lite3d_camera_update_view(camera);
    /* update scene tree */
    scene_recursive_nodes_update(scene, &scene->rootNode, camera);
    /* render scene */
    mqr_render(scene, camera, pass);

    if (scene->endSceneRender)
        scene->endSceneRender(scene, camera);

    scene_recursive_nodes_validate(&scene->rootNode);
}

void lite3d_scene_init(lite3d_scene *scene)
{
    SDL_assert(scene);
    memset(scene, 0, sizeof (lite3d_scene));
    /* root scene node */
    lite3d_scene_node_init(&scene->rootNode);
    /* never render this node */
    scene->rootNode.renderable = LITE3D_FALSE;
    lite3d_list_init(&scene->renderUnitQueue);
}

void lite3d_scene_purge(lite3d_scene *scene)
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

int lite3d_scene_add_node(lite3d_scene *scene, lite3d_scene_node *node,
    lite3d_scene_node *baseNode)
{
    SDL_assert(scene && node);

    /* mean root node */
    if (!baseNode)
        baseNode = &scene->rootNode;

    node->baseNode = baseNode;
    node->scene = scene;
    lite3d_list_add_last_link(&node->nodeLink, &baseNode->childNodes);
    return LITE3D_TRUE;
}

int lite3d_scene_remove_node(lite3d_scene *scene, lite3d_scene_node *node)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;

    if (node->scene != scene)
        return LITE3D_FALSE;

    lite3d_list_unlink_link(&node->nodeLink);
    node->baseNode = NULL;
    node->scene = NULL;

    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        /* check render unit node exist */
        while ((mqrNode = mqr_check_node_exist(mqrUnit, node)) != NULL)
        {
            /* unlink it */
            lite3d_list_unlink_link(&mqrNode->unit);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }
    }

    return LITE3D_TRUE;
}

int lite3d_scene_node_touch_material(
    lite3d_scene_node *node, lite3d_mesh_chunk *meshChunk,
    lite3d_material *material, uint32_t instancesCount)
{
    lite3d_mqr_unit *mqrUnit = NULL;
    lite3d_mqr_unit *mqrUnitTmp = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_mqr_node *mqrNode = NULL;
    lite3d_scene *scene = NULL;

    SDL_assert(node);
    SDL_assert(material);

    scene = (lite3d_scene *) node->scene;

    for (mqrUnitNode = scene->renderUnitQueue.l.next;
        mqrUnitNode != &scene->renderUnitQueue.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnitTmp = LITE3D_MEMBERCAST(lite3d_mqr_unit, mqrUnitNode, queued);
        if (mqrUnitTmp->material == material)
            mqrUnit = mqrUnitTmp;

        /* check render unit node exist */
        if ((mqrNode = mqr_check_mesh_chunk_exist(mqrUnitTmp, node, meshChunk)) != NULL)
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

        if (!meshChunk)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: mesh chunk is null", LITE3D_CURRENT_FUNCTION);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
            return LITE3D_FALSE;
        }

        mqrNode->node = node;
        mqrNode->meshChunk = meshChunk;
        mqrNode->boudingVol = meshChunk->boudingVol;
        mqrNode->node->recalc = LITE3D_TRUE;
    }

    SDL_assert_release(mqrNode);
    /* relink render node */
    mqrNode->instancesCount = instancesCount;
    mqr_unit_add_node(mqrUnit, mqrNode);
    return LITE3D_TRUE;
}

