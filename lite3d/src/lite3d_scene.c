/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_scene.h>

typedef struct _mqr_unit
{
    lite3d_list_node queued;
    lite3d_material *material;
    lite3d_list nodes;
    lite3d_camera *currentCamera;
} _mqr_unit;

typedef struct _mqr_node
{
    lite3d_list_node unit;
    lite3d_scene_node *node;
    lite3d_mesh_chunk *meshChunk;
    uint32_t instancesCount;
    lite3d_bounding_vol boundingVol;
    float distanceToCamera;
    _mqr_unit *matUnit;
} _mqr_node;

typedef struct _light_node
{
    lite3d_scene_node *lightNode;
    lite3d_light_params *params;
    int32_t storageIndex;
} _light_node;

static int mqr_node_distance_comparator(const void *a, const void *b)
{
    float adist, bdist;
    
    adist = (*(_mqr_node **)a)->distanceToCamera;
    bdist = (*(_mqr_node **)b)->distanceToCamera;

    if(adist > bdist)
        return -1;
    if(adist == bdist)
        return 0;

    return 1;
}

static void mqr_render_batch(lite3d_scene *scene,
    _mqr_node *mqrNode)
{
    /* notify render batch */
    if (scene->beginDrawBatch && !scene->beginDrawBatch(scene, mqrNode->node,
        mqrNode->meshChunk, mqrNode->matUnit->material))
            return;
    /* bind meshChunk */
    if (scene->bindedMeshChunk != mqrNode->meshChunk)
    {
        lite3d_mesh_chunk_bind(mqrNode->meshChunk);
        scene->bindedMeshChunk = mqrNode->meshChunk;
    }
    
    /* do render batch */
    lite3d_mesh_chunk_draw(mqrNode->meshChunk, mqrNode->instancesCount);
    
    scene->stats.batchesCalled++;
    scene->stats.trianglesRendered += mqrNode->meshChunk->vao.elementsCount * mqrNode->instancesCount;
    scene->stats.verticesRendered += mqrNode->meshChunk->vao.verticesCount * mqrNode->instancesCount;
}

static void mqr_render_node(lite3d_material_pass *pass, void *data)
{
    _mqr_node *mqrNode;
    lite3d_scene *scene;

    mqrNode = (_mqr_node *) data;
    SDL_assert(mqrNode);

    scene = (lite3d_scene *) mqrNode->node->scene;
    SDL_assert(scene);

    /* setup global parameters (model) */
    lite3d_shader_set_model_matrix(&mqrNode->node->worldView);
    lite3d_shader_set_normal_matrix(&mqrNode->node->normalModel);
    /* setup changed uniforms parameters */
    lite3d_material_pass_set_params(mqrNode->matUnit->material, pass, LITE3D_FALSE);
    /* call rendering current chunk */
    mqr_render_batch(scene, mqrNode);
}

static int mqr_node_approve(lite3d_scene *scene, 
    _mqr_node *mqrNode)
{
    SDL_assert(scene);
    SDL_assert(mqrNode->node);
    SDL_assert(mqrNode->matUnit);
    SDL_assert(mqrNode->matUnit->currentCamera);

    if (mqrNode->node->invalidated)
    {
        /* recalc bounding volume if node begin invalidated (position or rotation changed)*/
        lite3d_bounding_vol_translate(&mqrNode->boundingVol,
            &mqrNode->meshChunk->boundingVol,
            &mqrNode->node->worldView);
    }

    if (mqrNode->node->invalidated || mqrNode->matUnit->currentCamera->cameraNode.invalidated)
    {
        mqrNode->distanceToCamera = lite3d_camera_distance(mqrNode->matUnit->currentCamera, &mqrNode->boundingVol.sphereCenter);
    }
    
    if (!mqrNode->node->renderable)
        return LITE3D_FALSE;
    if (!mqrNode->node->enabled)
        return LITE3D_FALSE;

    scene->stats.batchesTotal++;
    /* frustum test */
    if (mqrNode->node->frustumTest && !lite3d_frustum_test(&mqrNode->matUnit->currentCamera->frustum, &mqrNode->boundingVol))
    {
        if (scene->nodeOutOfFrustum)
            scene->nodeOutOfFrustum(scene, mqrNode->node,
            mqrNode->meshChunk, mqrNode->matUnit->material, &mqrNode->boundingVol, mqrNode->matUnit->currentCamera);
        return LITE3D_FALSE;
    }
    
    if (mqrNode->node->frustumTest && scene->nodeInFrustum)
        scene->nodeInFrustum(scene, mqrNode->node,
        mqrNode->meshChunk, mqrNode->matUnit->material, &mqrNode->boundingVol, mqrNode->matUnit->currentCamera);

    return LITE3D_TRUE;
}

static void mqr_unit_render(lite3d_material_pass *pass, void *data)
{
    _mqr_unit *mqrUnit;
    _mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;
    lite3d_scene *scene = NULL;

    mqrUnit = (_mqr_unit *) data;
    SDL_assert(mqrUnit);

    for (mqrListNode = mqrUnit->nodes.l.next;
        mqrListNode != &mqrUnit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);

        scene = (lite3d_scene *) mqrNode->node->scene;
        if (!mqr_node_approve(scene, mqrNode))
            continue;

        if (pass->blending)
        {
            /* add to second stage */
            LITE3D_ARR_ADD_ELEM(&scene->sortedNodesByDistance, _mqr_node *, mqrNode);
        }
        else
        {
            mqr_render_node(pass, mqrNode);
        }
    }

    if (scene)
        scene->stats.materialPassed++;
}

static void mqr_render_stage_first(struct lite3d_scene *scene, lite3d_camera *camera, 
    uint16_t pass, uint32_t flags)
{
    _mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;

    LITE3D_ARR_ADD_ELEM(&scene->invalidatedUnits, lite3d_scene_node *, &camera->cameraNode);

    if (scene->beginFirstStageRender && (flags & LITE3D_RENDER_STAGE_FIRST))
        scene->beginFirstStageRender(scene, camera);

    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
        mqrUnit->currentCamera = camera;

        if (!lite3d_list_is_empty(&mqrUnit->nodes))
        {
            scene->stats.materialBlocks++;
            /* move transparent objects to blend stage */
            if (lite3d_material_pass_is_blend(mqrUnit->material, pass))
            {
                mqr_unit_render(lite3d_material_get_pass(mqrUnit->material, pass), mqrUnit);
                continue;
            }

            if (flags & LITE3D_RENDER_STAGE_FIRST)
            {
                lite3d_material_pass_render(mqrUnit->material, pass,
                    mqr_unit_render, mqrUnit);
                scene->stats.textureUnitsBinded += mqrUnit->material->textureUnitsBinded;
            }
        }
    }
}

static void mqr_render_stage_second(struct lite3d_scene *scene, lite3d_camera *camera, uint16_t pass)
{
    _mqr_node **mqrNode = NULL;

    if (scene->sortedNodesByDistance.size == 0)
        return;

    lite3d_array_qsort(&scene->sortedNodesByDistance, mqr_node_distance_comparator);

    if (scene->beginSecondStageRender)
        scene->beginSecondStageRender(scene, camera);

    LITE3D_ARR_FOREACH(&scene->sortedNodesByDistance, _mqr_node *, mqrNode)
    {
        lite3d_material_pass_render((*mqrNode)->matUnit->material, pass,
            mqr_render_node, *mqrNode);
        scene->stats.textureUnitsBinded += (*mqrNode)->matUnit->material->textureUnitsBinded;
    }
}

static void mqr_render_cleanup(struct lite3d_scene *scene)
{
    lite3d_array_clean(&scene->sortedNodesByDistance);    
}

static _mqr_node *mqr_check_mesh_chunk_exist(_mqr_unit *unit,
    lite3d_scene_node *node, lite3d_mesh_chunk *meshChunk)
{
    _mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
        if (mqrNode->node == node && mqrNode->meshChunk == meshChunk)
            return mqrNode;
    }

    return NULL;
}

static _mqr_node *mqr_check_node_exist(_mqr_unit *unit,
    lite3d_scene_node *node)
{
    _mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
        if (mqrNode->node == node)
            return mqrNode;
    }

    return NULL;
}

static void mqr_unit_add_node(_mqr_unit *unit, _mqr_node *node)
{
    _mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;

    /* insert node info list group by meshChunk */
    /* it guarantee what node will be sorted by meshChunk */
    for (mqrListNode = unit->nodes.l.next;
        mqrListNode != &unit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
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

    if ((recalcNode = lite3d_scene_node_update(node)) == LITE3D_TRUE)
        LITE3D_ARR_ADD_ELEM(&scene->invalidatedUnits, lite3d_scene_node *, node);

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

static void scene_updated_nodes_validate(lite3d_scene *scene)
{
    lite3d_scene_node **node;
    LITE3D_ARR_FOREACH(&scene->invalidatedUnits, lite3d_scene_node *, node)
    {
        (*node)->invalidated = LITE3D_FALSE;
    }

    lite3d_array_clean(&scene->invalidatedUnits);
}

void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera, 
    uint16_t pass, uint32_t flags)
{
    SDL_assert(scene && camera);
    /* clean statistic */
    memset(&scene->stats, 0, sizeof (scene->stats));

    if (scene->beforeUpdateNodes)
        scene->beforeUpdateNodes(scene, camera);
    /* update camera projection & transformation */
    lite3d_camera_update_view(camera);
    /* update scene tree */
    scene_recursive_nodes_update(scene, &scene->rootNode, camera);

    if (scene->beginSceneRender && !scene->beginSceneRender(scene, camera))
        return;

    /* render common objects */
    mqr_render_stage_first(scene, camera, pass, flags);
    /* render transparent objects */
    if (flags & LITE3D_RENDER_STAGE_SECOND)
        mqr_render_stage_second(scene, camera, pass);
    

    if (scene->bindedMeshChunk)
    {
        lite3d_mesh_chunk_unbind(scene->bindedMeshChunk);
        scene->bindedMeshChunk = NULL;
    }

    if (scene->endSceneRender)
        scene->endSceneRender(scene, camera);

    scene_updated_nodes_validate(scene);
    mqr_render_cleanup(scene);
}

void lite3d_scene_init(lite3d_scene *scene)
{
    SDL_assert(scene);
    memset(scene, 0, sizeof (lite3d_scene));
    /* root scene node */
    lite3d_scene_node_init(&scene->rootNode);
    /* never render this node */
    scene->rootNode.renderable = LITE3D_FALSE;
    lite3d_list_init(&scene->materialRenderUnits);

    lite3d_array_init(&scene->sortedNodesByDistance, sizeof(_mqr_node *), 2);
    lite3d_array_init(&scene->invalidatedUnits, sizeof(lite3d_scene_node *), 2);
}

void lite3d_scene_purge(lite3d_scene *scene)
{
    _mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    _mqr_node *mqrNode = NULL;
    lite3d_list_node *mqrListNode = NULL;

    SDL_assert(scene);
    while ((mqrUnitNode = lite3d_list_remove_first_link(&scene->materialRenderUnits)) != NULL)
    {
        mqrUnit = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
        while ((mqrListNode = lite3d_list_remove_first_link(&mqrUnit->nodes)) != NULL)
        {
            mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }

        lite3d_free_pooled(LITE3D_POOL_NO1, mqrUnit);
    }

    lite3d_array_purge(&scene->sortedNodesByDistance);
    lite3d_array_purge(&scene->invalidatedUnits);
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
    _mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    _mqr_node *mqrNode = NULL;

    if (node->scene != scene)
        return LITE3D_FALSE;

    lite3d_list_unlink_link(&node->nodeLink);
    node->baseNode = NULL;
    node->scene = NULL;

    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
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
    _mqr_unit *mqrUnit = NULL;
    _mqr_unit *mqrUnitTmp = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    _mqr_node *mqrNode = NULL;
    lite3d_scene *scene = NULL;

    SDL_assert(node);
    SDL_assert(material);

    scene = (lite3d_scene *) node->scene;

    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnitTmp = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
        if (mqrUnitTmp->material == material)
            mqrUnit = mqrUnitTmp;

        /* check render unit node exist */
        if ((mqrNode = mqr_check_mesh_chunk_exist(mqrUnitTmp, node, meshChunk)) != NULL)
            /* unlink it */
            lite3d_list_unlink_link(&mqrNode->unit);
    }

    if (mqrUnit == NULL)
    {
        mqrUnit = (_mqr_unit *) lite3d_calloc_pooled(LITE3D_POOL_NO1,
            sizeof (_mqr_unit));
        lite3d_list_init(&mqrUnit->nodes);
        lite3d_list_link_init(&mqrUnit->queued);
        mqrUnit->material = material;

        lite3d_list_add_last_link(&mqrUnit->queued, &scene->materialRenderUnits);
    }

    SDL_assert_release(mqrUnit);

    if (mqrNode == NULL)
    {
        if (!meshChunk)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: mesh chunk is null", LITE3D_CURRENT_FUNCTION);
            return LITE3D_FALSE;
        }

        mqrNode = (_mqr_node *) lite3d_calloc_pooled(LITE3D_POOL_NO1,
            sizeof (_mqr_node));
        lite3d_list_link_init(&mqrNode->unit);
        mqrNode->node = node;
    }

    SDL_assert_release(mqrNode);
    /* relink render node */
    mqrNode->meshChunk = meshChunk;
    mqrNode->boundingVol = meshChunk->boundingVol;
    mqrNode->instancesCount = instancesCount;
    mqrNode->matUnit = mqrUnit;
    mqrNode->node->recalc = LITE3D_TRUE;
    mqr_unit_add_node(mqrUnit, mqrNode);

    return LITE3D_TRUE;
}

