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
#ifndef LITE3D_SCENE_H
#define	LITE3D_SCENE_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_camera.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_material.h>
#include <lite3d/lite3d_array.h>
#include <lite3d/lite3d_lighting.h>

#define LITE3D_RENDER_OPAQUE            ((uint32_t)0x1)
#define LITE3D_RENDER_BLEND             ((uint32_t)0x1 << 1)
#define LITE3D_RENDER_CLEAN_COLOR_BUF   ((uint32_t)0x1 << 2)
#define LITE3D_RENDER_CLEAN_DEPTH_BUF   ((uint32_t)0x1 << 3)
#define LITE3D_RENDER_CLEAN_STENCIL_BUF ((uint32_t)0x1 << 4)
#define LITE3D_RENDER_DEPTH_TEST        ((uint32_t)0x1 << 5)
#define LITE3D_RENDER_COLOR_OUTPUT      ((uint32_t)0x1 << 6)
#define LITE3D_RENDER_DEPTH_OUTPUT      ((uint32_t)0x1 << 7)
#define LITE3D_RENDER_STENCIL_OUTPUT    ((uint32_t)0x1 << 8)
#define LITE3D_RENDER_INSTANCING        ((uint32_t)0x1 << 9)
#define LITE3D_RENDER_OCCLUSION_QUERY   ((uint32_t)0x1 << 10)
#define LITE3D_RENDER_OCCLUSION_CULLING ((uint32_t)0x1 << 11)

#define LITE3D_RENDER_DEFAULT           (LITE3D_RENDER_OPAQUE | LITE3D_RENDER_BLEND | \
    LITE3D_RENDER_DEPTH_TEST | LITE3D_RENDER_COLOR_OUTPUT | LITE3D_RENDER_DEPTH_OUTPUT)

typedef struct lite3d_scene_stats
{
    int32_t trianglesRendered;
    int32_t verticesRendered;
    int32_t nodesTotal;
    int32_t batchesCalled;
    int32_t batchesTotal;
    int32_t materialBlocks;
    int32_t textureUnitsBinded;
    int32_t blockUnitsBinded;
    int32_t materialPassed;
} lite3d_scene_stats;

typedef struct lite3d_scene
{
    lite3d_scene_node rootNode;
    lite3d_scene_stats stats;
    lite3d_list materialRenderUnits;
    lite3d_array stageOpague;
    lite3d_array stageBlend;
    lite3d_array invalidatedUnits;
    lite3d_array seriesMatrixes;
    lite3d_mesh_chunk *bindedMeshChunk;
    lite3d_camera *currentCamera;
    void *userdata;
    int (*beginDrawBatch)(struct lite3d_scene *scene, 
        struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, struct lite3d_material *material);
    void (*nodeInFrustum)(struct lite3d_scene *scene, 
        struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
        struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol, 
        struct lite3d_camera *camera);
    void (*nodeOutOfFrustum)(struct lite3d_scene *scene, 
        struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
        struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
        struct lite3d_camera *camera);
    void (*beforeUpdateNodes)(struct lite3d_scene *scene, struct lite3d_camera *camera);
    int (*beginSceneRender)(struct lite3d_scene *scene, struct lite3d_camera *camera);
    void (*endSceneRender)(struct lite3d_scene *scene, struct lite3d_camera *camera);
    void (*beginFirstStageRender)(struct lite3d_scene *scene, struct lite3d_camera *camera);
    void (*beginSecondStageRender)(struct lite3d_scene *scene, struct lite3d_camera *camera);
} lite3d_scene;

LITE3D_CEXPORT void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera, 
    uint16_t pass, uint32_t flags);
LITE3D_CEXPORT void lite3d_scene_init(lite3d_scene *scene);
LITE3D_CEXPORT void lite3d_scene_purge(lite3d_scene *scene);

LITE3D_CEXPORT int lite3d_scene_add_node(lite3d_scene *scene, 
    lite3d_scene_node *node, 
    lite3d_scene_node *baseNode);

LITE3D_CEXPORT int lite3d_scene_remove_node(lite3d_scene *scene, lite3d_scene_node *node);

LITE3D_CEXPORT int lite3d_scene_node_touch_material(
    lite3d_scene_node *node, lite3d_mesh_chunk *meshChunk, 
    lite3d_material *material, uint32_t instancesCount);

#endif	/* LITE3D_SCENE_H */

