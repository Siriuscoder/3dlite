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
#ifndef LITE3D_SCENE_H
#define	LITE3D_SCENE_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_camera.h>
#include <3dlite/3dlite_list.h>
#include <3dlite/3dlite_mesh.h>
#include <3dlite/3dlite_material.h>

typedef struct lite3d_scene_stats
{
    int32_t trianglesRendered;
    int32_t verticesRendered;
    int32_t objectsRendered;
    int32_t batches;
    int32_t materialBlocks;
    int32_t textureUnitsBinded;
    int32_t materialPassed;
} lite3d_scene_stats;

typedef struct lite3d_scene
{
    lite3d_scene_node rootNode;
    lite3d_scene_stats stats;
    lite3d_list renderUnitQueue;
    void (*drawBatch)(struct lite3d_scene *scene, 
        struct lite3d_scene_node *node, 
        lite3d_mesh_chunk *meshChunk, lite3d_material *material);
    void (*preRender)(struct lite3d_scene *scene, lite3d_camera *camera);
    void (*postRender)(struct lite3d_scene *scene, lite3d_camera *camera);
} lite3d_scene;

LITE3D_CEXPORT void lite3d_scene_render(lite3d_scene *scene, lite3d_camera *camera);
LITE3D_CEXPORT void lite3d_scene_init(lite3d_scene *scene);
LITE3D_CEXPORT void lite3d_scene_purge(lite3d_scene *scene);

LITE3D_CEXPORT int lite3d_scene_add_node(lite3d_scene *scene, 
    lite3d_scene_node *node, 
    lite3d_scene_node *baseNode);
LITE3D_CEXPORT int lite3d_scene_remove_node(lite3d_scene *scene, lite3d_scene_node *node);

LITE3D_CEXPORT int lite3d_scene_node_touch_material(
    lite3d_scene_node *node, lite3d_mesh_chunk *meshChunk, lite3d_material *material);

#endif	/* LITE3D_SCENE_H */

