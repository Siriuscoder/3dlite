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
#ifndef LITE3D_MESHES_RENDER_H
#define	LITE3D_MESHES_RENDER_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_mesh.h>
#include <3dlite/3dlite_scene.h>
#include <3dlite/3dlite_material.h>

typedef struct lite3d_composite_scene_node
{
    lite3d_scene_node node;
    lite3d_indexed_mesh *mesh; 
} lite3d_composite_scene_node;

typedef struct lite3d_composite_scene
{
    lite3d_scene scene;
    lite3d_list renderUnitQueue;
    void (*drawBatch)(struct lite3d_composite_scene *scene, 
        struct lite3d_composite_scene_node *node, 
        lite3d_mesh_chunk *meshChunk, lite3d_material *material);
} lite3d_composite_scene;

LITE3D_CEXPORT void lite3d_composite_scene_init(lite3d_composite_scene *scene);
LITE3D_CEXPORT void lite3d_composite_scene_purge(lite3d_composite_scene *scene);
LITE3D_CEXPORT void lite3d_composite_scene_node_init(lite3d_composite_scene_node *node,
    lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT int lite3d_composite_scene_add_node(
    lite3d_composite_scene *scene, 
    lite3d_composite_scene_node *node,
    lite3d_scene_node *baseNode);
LITE3D_CEXPORT int lite3d_composite_scene_node_attach_material(
    lite3d_composite_scene_node *node, lite3d_material *material, uint32_t index);

LITE3D_CEXPORT int lite3d_composite_scene_remove(lite3d_composite_scene *scene, 
    lite3d_composite_scene_node *node);

#endif	/* LITE3D_MESHES_RENDER_H */

