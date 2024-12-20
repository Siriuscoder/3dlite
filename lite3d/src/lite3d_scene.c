/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#include <SDL_timer.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_metrics.h>
#include <lite3d/lite3d_query.h>
#include <lite3d/lite3d_scene.h>

extern int gQueryObjectSupported;

typedef struct _query_unit
{
    lite3d_query query;
    lite3d_camera *camera;
} _query_unit;

typedef struct _mqr_unit
{
    lite3d_list_node queued;
    lite3d_material *material;
    lite3d_list nodes;
} _mqr_unit;

typedef struct _mqr_node
{
    lite3d_list_node unit;
    lite3d_scene_node *node;
    lite3d_mesh_chunk *meshChunk;
    lite3d_mesh_chunk *bbMeshChunk;
    uint32_t instancesCount;
    lite3d_bounding_vol boundingVol;
    float distanceToCamera;
    lite3d_array queries;
    _query_unit *currentQuery;
    _mqr_unit *matUnit;
} _mqr_node;

// Сортирует от дальнего к ближнему
static int mqr_node_distance_comparator_to_near(const void *a, const void *b)
{
    float adist = (*(_mqr_node **)a)->distanceToCamera;
    float bdist = (*(_mqr_node **)b)->distanceToCamera;
    if(adist > bdist)
        return -1;
    if(adist == bdist)
        return 0;

    return 1;
}

// Сортирует от ближнего к дальнему
static int mqr_node_distance_comparator_from_near(const void *a, const void *b)
{
    float adist = (*(_mqr_node **)a)->distanceToCamera;
    float bdist = (*(_mqr_node **)b)->distanceToCamera;
    if(adist < bdist)
        return -1;
    if(adist == bdist)
        return 0;

    return 1;
}

static void mqr_node_set_shader_params(lite3d_scene *scene, lite3d_material_pass *pass, _mqr_node *mqrNode)
{
    kmMat4 screen;

    /* setup global parameters (model) */
    lite3d_shader_set_model_matrix(&mqrNode->node->worldView);
    lite3d_shader_set_normal_matrix(&mqrNode->node->normalModel);

    /* calc screen matrix */
    kmMat4Multiply(&screen, &scene->currentCamera->screen, &mqrNode->node->worldView);
    lite3d_shader_set_screen_matrix(&screen);

    /* setup changed uniforms parameters */
    lite3d_material_pass_set_params(mqrNode->matUnit->material, pass, LITE3D_FALSE);
}

static void mqr_render_mesh_chunk(lite3d_scene *scene, lite3d_mesh_chunk *chunk, uint32_t count)
{
    SDL_assert(chunk);
    SDL_assert(scene);

    if (scene->bindedMeshChunk != chunk)
    {
        lite3d_mesh_chunk_bind(chunk);
        scene->bindedMeshChunk = chunk;
        /* validate current shader program */
        if (!lite3d_shader_program_validate_current())
        {
            return;
        }
    }

    if (count > 1)
    {
        LITE3D_METRIC_CALL(lite3d_mesh_chunk_draw_instanced, (chunk, count))
    }
    else
    {
        LITE3D_METRIC_CALL(lite3d_mesh_chunk_draw, (chunk))
    }

    scene->stats.trianglesRendered += chunk->vao.elementsCount * count;
    scene->stats.verticesRendered += chunk->vao.verticesCount * count;
    scene->stats.batchCalled++;
    scene->stats.batchInstancedCalled += count;
}

static void mqr_render_batch(lite3d_material_pass *pass, _mqr_node *mqrNode, uint32_t flags)
{
    lite3d_scene *scene = (lite3d_scene *) mqrNode->node->scene;
    SDL_assert(mqrNode);
    SDL_assert(scene);

    /* notify render batch */
    if (scene->beginDrawBatch && !scene->beginDrawBatch(scene, mqrNode->node,
        mqrNode->meshChunk, mqrNode->matUnit->material))
            return;

    mqr_node_set_shader_params(scene, pass, mqrNode);
    
    /* call rendering current chunk */
    if (flags & LITE3D_RENDER_OCCLUSION_QUERY && mqrNode->bbMeshChunk)
    {
        SDL_assert(mqrNode->currentQuery);
        lite3d_query_begin(&mqrNode->currentQuery->query); 
        if (mqrNode->currentQuery->query.anyPassed)
        {
            // Render full complex batch under occlusion query if node visible
            mqr_render_mesh_chunk(scene, mqrNode->meshChunk, mqrNode->instancesCount);
        }
        else
        {
            // Render simplified batch under occlusion query if node occluded
            lite3d_depth_output(LITE3D_FALSE);
            lite3d_backface_culling(LITE3D_CULLFACE_FRONT);
            mqr_render_mesh_chunk(scene, mqrNode->bbMeshChunk, mqrNode->instancesCount);
            lite3d_backface_culling(pass->doubleSided ? LITE3D_CULLFACE_NEVER : LITE3D_CULLFACE_BACK);
            lite3d_depth_output(flags & LITE3D_RENDER_DEPTH_OUTPUT ? LITE3D_TRUE : LITE3D_FALSE);
        }

        lite3d_query_end(&mqrNode->currentQuery->query);
    }
    else
    {
        mqr_render_mesh_chunk(scene, mqrNode->meshChunk, mqrNode->instancesCount);
    }
}

static int mqr_render_series_params(lite3d_vbo *buffer, const void *param, size_t psize)
{
    /* setup global parameters (model normal) */
    if (psize > buffer->size)
    {
        if(!lite3d_vbo_extend(buffer, psize - buffer->size))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unable to extend buffer for store batch series",
                LITE3D_CURRENT_FUNCTION);
            return LITE3D_FALSE;
        }
    }
    
    if (!lite3d_vbo_subbuffer(buffer, param, 0, psize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unable to write batch series to buffer",
            LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

static void mqr_render_batch_series(lite3d_material_pass *pass, _mqr_node *mqrNode, uint32_t continuedId, uint8_t batchCrop)
{
    lite3d_scene *scene = (lite3d_scene *) mqrNode->node->scene;
    SDL_assert(mqrNode);
    SDL_assert(scene);
    SDL_assert(mqrNode->meshChunk->mesh->auxBuffer);

    /* notify render batch */
    if (scene->beginDrawBatch && !scene->beginDrawBatch(scene, mqrNode->node,
        mqrNode->meshChunk, mqrNode->matUnit->material))
            return;

    LITE3D_ARR_ADD_ELEM(&scene->seriesMatrixes, kmMat4, mqrNode->node->worldView);
    /* call rendering current chunk */
    if (batchCrop)
    {
        if (!mqr_render_series_params(mqrNode->meshChunk->mesh->auxBuffer, 
            scene->seriesMatrixes.data, scene->seriesMatrixes.size * scene->seriesMatrixes.elemSize))
        {
            lite3d_array_clean(&scene->seriesMatrixes);
            return;
        }

        mqr_node_set_shader_params(scene, pass, mqrNode);
        mqr_render_mesh_chunk(scene, mqrNode->meshChunk, continuedId+1);

        lite3d_array_clean(&scene->seriesMatrixes);
    }
}

static void mqr_render_node(lite3d_material_pass *pass, _mqr_node *mqrNode, uint32_t continuedId, uint8_t batchCrop, uint32_t flags)
{
    if (flags & LITE3D_RENDER_INSTANCING)
        LITE3D_METRIC_CALL(mqr_render_batch_series, (pass, mqrNode, continuedId, batchCrop))
    else
        LITE3D_METRIC_CALL(mqr_render_batch, (pass, mqrNode, flags))
}

static int mqr_node_occluded(lite3d_scene *scene, _mqr_node *mqrNode, uint32_t flags)
{
    int queryFound = LITE3D_FALSE;
    mqrNode->currentQuery = NULL;

    if (!(flags & (LITE3D_RENDER_OCCLUSION_QUERY | LITE3D_RENDER_OCCLUSION_CULLING)))
    {
        return LITE3D_FALSE;
    }

    // Find query by camera
    LITE3D_ARR_FOREACH(&mqrNode->queries, _query_unit, mqrNode->currentQuery)
    {
        if (mqrNode->currentQuery->camera == scene->currentCamera)
        {
            queryFound = LITE3D_TRUE;
            break;
        }
    }

    if (!queryFound)
    {
        _query_unit newQuery;
        lite3d_query_init(&newQuery.query);
        newQuery.camera = scene->currentCamera;
        LITE3D_ARR_ADD_ELEM(&mqrNode->queries, _query_unit, newQuery);
        mqrNode->currentQuery = LITE3D_ARR_GET_LAST(&mqrNode->queries, _query_unit);
    }

    SDL_assert(mqrNode->currentQuery);

    // Check query from last LITE3D_RENDER_OCCLUSION_QUERY step, normaly - last frame
    if (flags & LITE3D_RENDER_OCCLUSION_QUERY)
    {
        if (mqrNode->currentQuery->query.inProgress)
        {
            lite3d_query_result(&mqrNode->currentQuery->query);
        }
    }

    return mqrNode->currentQuery->query.anyPassed == LITE3D_FALSE ? LITE3D_TRUE : LITE3D_FALSE;
}

static int mqr_node_approve(lite3d_scene *scene, _mqr_node *mqrNode, uint32_t flags)
{
    int nodeVisible = LITE3D_TRUE;
    int nodeApproved = LITE3D_TRUE;

    SDL_assert(scene);
    SDL_assert(mqrNode->node);
    SDL_assert(mqrNode->matUnit);
    SDL_assert(scene->currentCamera);
    
    if (!mqrNode->node->renderable)
        return LITE3D_FALSE;

    if (!mqrNode->node->enabled)
        return LITE3D_FALSE;

    scene->stats.batchTotal++;

    // Check occlusion culling if needed
    if (mqr_node_occluded(scene, mqrNode, flags))
    {
        if (flags & LITE3D_RENDER_OCCLUSION_CULLING)
        {
            nodeApproved = LITE3D_FALSE;
        }

        nodeVisible = LITE3D_FALSE;
        scene->stats.batchOccluded++;
    }

    // Check frustum culling if needed
    if (flags & LITE3D_RENDER_FRUSTUM_CULLING)
    {
        // Custom frustum check
        if (scene->customVisibilityCheck && flags & LITE3D_RENDER_CUSTOM_VISIBILITY_CHECK)
        {
            if (!scene->customVisibilityCheck(scene, mqrNode->node,
                mqrNode->meshChunk, mqrNode->matUnit->material, &mqrNode->boundingVol, scene->currentCamera))
            {
                nodeApproved = nodeVisible = LITE3D_FALSE;
            }
        }
        /* frustum test */
        else if (mqrNode->node->frustumTest && !lite3d_frustum_test(&scene->currentCamera->frustum, &mqrNode->boundingVol))
        {
            nodeApproved = nodeVisible = LITE3D_FALSE;
            if (scene->nodeOutOfFrustum)
            {
                scene->nodeOutOfFrustum(scene, mqrNode->node, mqrNode->meshChunk, mqrNode->matUnit->material, 
                    &mqrNode->boundingVol, scene->currentCamera);
            }
        }
        else if (mqrNode->node->frustumTest && scene->nodeInFrustum)
        {
            scene->nodeInFrustum(scene, mqrNode->node, mqrNode->meshChunk, mqrNode->matUnit->material, 
                &mqrNode->boundingVol, scene->currentCamera);
        }
    }

    if (nodeVisible)
    {
        mqrNode->node->visible = nodeVisible;
    }
    
    return nodeApproved;
}

static void mqr_unit_queue_render(lite3d_scene *scene, lite3d_array *queue, uint16_t pass, uint32_t flags)
{
    _mqr_node **mqrNode = NULL;
    _mqr_unit *curUnit = NULL;
    uint32_t continuedId = 0;
    lite3d_material_pass *matPass = NULL;

    LITE3D_ARR_FOREACH(queue, _mqr_node *, mqrNode)
    {
        uint8_t batchCrop = LITE3D_FALSE;
        if (curUnit != (*mqrNode)->matUnit)
        {
            matPass = lite3d_material_apply((*mqrNode)->matUnit->material, pass);
            scene->stats.textureUnitsBinded += matPass->bindContext.textureBindingsCount;
            scene->stats.blockUnitsBinded += matPass->bindContext.blockBindingsCount;
            scene->stats.materialPassed++;
            curUnit = (*mqrNode)->matUnit;
        }

        // check it last node 
        if (LITE3D_ARR_IS_LAST(queue, _mqr_node *, mqrNode))
            batchCrop = LITE3D_TRUE;
        else
        {
            if ((*mqrNode)->meshChunk != (*(mqrNode+1))->meshChunk)
                batchCrop = LITE3D_TRUE;
            if ((*mqrNode)->matUnit != (*(mqrNode+1))->matUnit)
                batchCrop = LITE3D_TRUE;
        }

        LITE3D_METRIC_CALL(mqr_render_node, (matPass, *mqrNode, continuedId, batchCrop, flags))
        continuedId = batchCrop ? 0 : continuedId+1;
    }
}

static void mqr_unit_make_queue(lite3d_scene *scene, _mqr_unit *mqrUnit, uint16_t pass, uint32_t flags)
{
    _mqr_node *mqrNode;
    lite3d_list_node *mqrListNode;
    SDL_assert(mqrUnit);

    for (mqrListNode = mqrUnit->nodes.l.next;
        mqrListNode != &mqrUnit->nodes.l; mqrListNode = lite3d_list_next(mqrListNode))
    {
        mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
        
        if (mqrNode->node->invalidated)
        {
            /* recalc bounding volume if node begin invalidated (position or rotation changed)*/
            lite3d_bounding_vol_translate(&mqrNode->boundingVol,
                &mqrNode->meshChunk->boundingVol,
                &mqrNode->node->worldView);
        }

        if (mqrNode->node->invalidated || scene->currentCamera->cameraNode.invalidated)
        {
            mqrNode->distanceToCamera = lite3d_camera_distance(scene->currentCamera, &mqrNode->boundingVol.sphereCenter);
        }
        
        /* ignore this entry if material pass not exist or empty */
        if (!lite3d_material_get_pass(mqrUnit->material, pass) || lite3d_material_pass_is_empty(mqrUnit->material, pass))
            continue;

        if (lite3d_material_pass_is_blend(mqrUnit->material, pass))
        {
            if ((flags & LITE3D_RENDER_TRANSPARENT) && mqr_node_approve(scene, mqrNode, flags))
                /* add to transparent stage */
                LITE3D_ARR_ADD_ELEM(&scene->stageTransparent, _mqr_node *, mqrNode);
        }
        else
        {
            if ((flags & LITE3D_RENDER_OPAQUE) && mqr_node_approve(scene, mqrNode, flags))
                /* add to opague stage */
                LITE3D_ARR_ADD_ELEM(&scene->stageOpague, _mqr_node *, mqrNode);
        }
    }
}

static void mqr_render_make_queue(struct lite3d_scene *scene, uint16_t pass, uint32_t flags)
{
    _mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;

    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);

        if (!lite3d_list_is_empty(&mqrUnit->nodes))
        {
            scene->stats.materialBlocks++;
            LITE3D_METRIC_CALL(mqr_unit_make_queue, (scene, mqrUnit, pass, flags))
        }
    }
}

static void mqr_render_stage_opaque(struct lite3d_scene *scene, uint16_t pass, uint32_t flags)
{
    if (flags & LITE3D_RENDER_OPAQUE)
    {
        if (flags & LITE3D_RENDER_SORT_OPAQUE_TO_NEAR)
        {
            lite3d_array_qsort(&scene->stageOpague, mqr_node_distance_comparator_to_near);
        }
        else if (flags & LITE3D_RENDER_SORT_OPAQUE_FROM_NEAR)
        {
            lite3d_array_qsort(&scene->stageOpague, mqr_node_distance_comparator_from_near);
        }

        if (scene->beginOpaqueStageRender)
            LITE3D_METRIC_CALL(scene->beginOpaqueStageRender, (scene, scene->currentCamera))

        LITE3D_METRIC_CALL(mqr_unit_queue_render, (scene, &scene->stageOpague, pass, flags))
        // cleanup last rendered queue
        lite3d_array_clean(&scene->stageOpague);
    }
}

static void mqr_render_stage_transparent(struct lite3d_scene *scene, uint16_t pass, uint32_t flags)
{
    if (flags & LITE3D_RENDER_TRANSPARENT)
    {
        if (flags & LITE3D_RENDER_SORT_TRANSPARENT_TO_NEAR)
        {
            lite3d_array_qsort(&scene->stageTransparent, mqr_node_distance_comparator_to_near);
        }
        else if (flags & LITE3D_RENDER_SORT_OPAQUE_FROM_NEAR)
        {
            lite3d_array_qsort(&scene->stageTransparent, mqr_node_distance_comparator_from_near);
        }

        if (scene->beginBlendingStageRender)
            scene->beginBlendingStageRender(scene, scene->currentCamera);

        mqr_unit_queue_render(scene, &scene->stageTransparent, pass, flags);
        lite3d_array_clean(&scene->stageTransparent);
    }
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
    lite3d_scene_node *node)
{
    lite3d_list_node *nodeLink;
    lite3d_scene_node *child;
    uint8_t recalcNode;

    node->visible = LITE3D_FALSE; // Определим видимость далее
    if ((recalcNode = lite3d_scene_node_update(node)) == LITE3D_TRUE)
    {
        if (!node->isCamera)
        {
            LITE3D_ARR_ADD_ELEM(&scene->invalidatedUnits, lite3d_scene_node *, node);
        }
    }
    
    /* render all childrens first */
    for (nodeLink = node->childNodes.l.next;
        nodeLink != &node->childNodes.l; nodeLink = lite3d_list_next(nodeLink))
    {
        child = LITE3D_MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink);
        child->recalc = recalcNode ? LITE3D_TRUE : child->recalc;
        scene_recursive_nodes_update(scene, child);
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

static uint32_t scene_correct_render_flags(uint32_t flags)
{
    if (!gQueryObjectSupported)
    {
        flags &= ~(LITE3D_RENDER_OCCLUSION_CULLING | LITE3D_RENDER_OCCLUSION_QUERY);
    }

    if (flags & LITE3D_RENDER_OCCLUSION_QUERY)
    {
        flags &= ~LITE3D_RENDER_INSTANCING;
    }

    return flags;
}

void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera, 
    uint16_t pass, uint32_t flags)
{
    SDL_assert(scene && camera);
    /* clean statistic */
    memset(&scene->stats, 0, sizeof (scene->stats));

    flags = scene_correct_render_flags(flags);

    if (scene->beforeUpdateNodes)
        LITE3D_METRIC_CALL(scene->beforeUpdateNodes, (scene, camera))
    /* update scene tree */
    LITE3D_METRIC_CALL(scene_recursive_nodes_update, (scene, &scene->rootNode))
    /* update camera projection & transformation */
    LITE3D_METRIC_CALL(lite3d_camera_update_view, (camera))

    if (scene->beginSceneRender && !scene->beginSceneRender(scene, camera))
        return;

    scene->currentCamera = camera;
    LITE3D_METRIC_CALL(mqr_render_make_queue, (scene, pass, flags));
    /* render common objects */
    LITE3D_METRIC_CALL(mqr_render_stage_opaque, (scene, pass, flags))
    /* render transparent objects */
    LITE3D_METRIC_CALL(mqr_render_stage_transparent, (scene, pass, flags))
    

    if (scene->bindedMeshChunk)
    {
        lite3d_mesh_chunk_unbind(scene->bindedMeshChunk);
        scene->bindedMeshChunk = NULL;
    }

    if (scene->endSceneRender)
        LITE3D_METRIC_CALL(scene->endSceneRender, (scene, camera))

    LITE3D_METRIC_CALL(scene_updated_nodes_validate, (scene))
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

    lite3d_array_init(&scene->stageOpague, sizeof(_mqr_node *), 2);
    lite3d_array_init(&scene->stageTransparent, sizeof(_mqr_node *), 2);
    lite3d_array_init(&scene->invalidatedUnits, sizeof(lite3d_scene_node *), 2);
    lite3d_array_init(&scene->seriesMatrixes, sizeof(kmMat4), 10);
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
            _query_unit *query;
            mqrNode = LITE3D_MEMBERCAST(_mqr_node, mqrListNode, unit);
            LITE3D_ARR_FOREACH(&mqrNode->queries, _query_unit, query)
            {
                lite3d_query_purge(&query->query);
            }

            lite3d_array_purge(&mqrNode->queries);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }

        lite3d_free_pooled(LITE3D_POOL_NO1, mqrUnit);
    }

    lite3d_array_purge(&scene->seriesMatrixes);
    lite3d_array_purge(&scene->stageOpague);
    lite3d_array_purge(&scene->stageTransparent);
    lite3d_array_purge(&scene->invalidatedUnits);
}

int lite3d_scene_add_node(lite3d_scene *scene, lite3d_scene_node *node,
    lite3d_scene_node *baseNode)
{
    SDL_assert(scene && node);

    // linked already
    if (node->scene)
        return LITE3D_FALSE;

    /* mean root node */
    if (!baseNode)
        baseNode = &scene->rootNode;

    node->recalc = LITE3D_TRUE;
    node->baseNode = baseNode;
    node->scene = scene;
    lite3d_list_add_last_link(&node->nodeLink, &baseNode->childNodes);
    return LITE3D_TRUE;
}

int lite3d_scene_rebase_node(lite3d_scene *scene, lite3d_scene_node *node,
    lite3d_scene_node *baseNode)
{
    SDL_assert(scene && node);

    // not linked or linked to another scene
    if (node->scene != scene)
        return LITE3D_FALSE;

    /* mean root node */
    if (!baseNode)
        baseNode = &scene->rootNode;

    node->recalc = LITE3D_TRUE;
    node->baseNode = baseNode;
    lite3d_list_unlink_link(&node->nodeLink);
    lite3d_list_add_last_link(&node->nodeLink, &baseNode->childNodes);
    return LITE3D_TRUE;
}

int lite3d_scene_remove_node(lite3d_scene *scene, lite3d_scene_node *node)
{
    _mqr_unit *mqrUnit = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    lite3d_list_node *nodeLink = NULL;
    _mqr_node *mqrNode = NULL;

    if (node->scene != scene)
        return LITE3D_FALSE;

    /* move all child to parent node */
    for (nodeLink = node->childNodes.l.next;
        nodeLink != &node->childNodes.l; nodeLink = node->childNodes.l.next)
    {
        lite3d_scene_node *sceneNode = LITE3D_MEMBERCAST(lite3d_scene_node, nodeLink, nodeLink);
        lite3d_scene_rebase_node(scene, sceneNode, node->baseNode);
    }

    lite3d_list_unlink_link(&node->nodeLink);
    node->baseNode = NULL;
    node->scene = NULL;
    node->renderable = LITE3D_FALSE;
    
    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnit = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
        /* check render unit node exist */
        while ((mqrNode = mqr_check_node_exist(mqrUnit, node)) != NULL)
        {
            _query_unit *query;
            LITE3D_ARR_FOREACH(&mqrNode->queries, _query_unit, query)
            {
                lite3d_query_purge(&query->query);
            }

            lite3d_array_purge(&mqrNode->queries);
            lite3d_list_unlink_link(&mqrNode->unit);
            lite3d_free_pooled(LITE3D_POOL_NO1, mqrNode);
        }
    }

    return LITE3D_TRUE;
}

int lite3d_scene_node_touch_material(struct lite3d_scene_node *node, 
    struct lite3d_mesh_chunk *meshChunk, struct lite3d_mesh_chunk *bbMeshChunk, 
    struct lite3d_material *material, uint32_t instancesCount)
{
    _mqr_unit *mqrUnit = NULL;
    _mqr_unit *mqrUnitTmp = NULL;
    lite3d_list_node *mqrUnitNode = NULL;
    _mqr_node *mqrNode = NULL;
    lite3d_scene *scene = NULL;

    SDL_assert(node);
    SDL_assert(material);

    if (instancesCount > 1 && !lite3d_check_instanced_arrays())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Could not add node to scene with instancesCount %u, instancing not supported", 
            LITE3D_CURRENT_FUNCTION, instancesCount);
        return LITE3D_FALSE;
    }

    scene = (lite3d_scene *) node->scene;

    for (mqrUnitNode = scene->materialRenderUnits.l.next;
        mqrUnitNode != &scene->materialRenderUnits.l; mqrUnitNode = lite3d_list_next(mqrUnitNode))
    {
        mqrUnitTmp = LITE3D_MEMBERCAST(_mqr_unit, mqrUnitNode, queued);
        if (mqrUnitTmp->material == material)
            mqrUnit = mqrUnitTmp;

        /* check render unit node exist */
        if (!mqrNode)
        {
            if ((mqrNode = mqr_check_mesh_chunk_exist(mqrUnitTmp, node, meshChunk)) != NULL)
            {
                /* unlink it and remap to new material later */
                lite3d_list_unlink_link(&mqrNode->unit);
            }
        }

        /* all found, nothing to do */
        if (mqrNode && mqrUnit)
        {
            break;
        }
    }

    if (mqrUnit == NULL)
    {
        mqrUnit = (_mqr_unit *) lite3d_calloc_pooled(LITE3D_POOL_NO1, sizeof (_mqr_unit));
        if (!mqrUnit) return LITE3D_FALSE;

        lite3d_list_init(&mqrUnit->nodes);
        lite3d_list_link_init(&mqrUnit->queued);
        mqrUnit->material = material;

        lite3d_list_add_last_link(&mqrUnit->queued, &scene->materialRenderUnits);
    }

    SDL_assert(mqrUnit);

    if (mqrNode == NULL)
    {
        if (!meshChunk)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: mesh chunk is null", LITE3D_CURRENT_FUNCTION);
            return LITE3D_FALSE;
        }

        mqrNode = (_mqr_node *) lite3d_calloc_pooled(LITE3D_POOL_NO1, sizeof (_mqr_node));
        if (!mqrNode) return LITE3D_FALSE;

        lite3d_list_link_init(&mqrNode->unit);
        lite3d_array_init(&mqrNode->queries, sizeof(_query_unit), 1);
        mqrNode->node = node;
    }

    SDL_assert(mqrNode);
    /* relink render node */
    mqrNode->meshChunk = meshChunk;
    mqrNode->bbMeshChunk = bbMeshChunk;
    mqrNode->boundingVol = meshChunk->boundingVol;
    mqrNode->instancesCount = instancesCount;
    mqrNode->matUnit = mqrUnit;
    mqrNode->node->recalc = LITE3D_TRUE;
    mqrNode->node->renderable = LITE3D_TRUE;
    mqr_unit_add_node(mqrUnit, mqrNode);

    return LITE3D_TRUE;
}
