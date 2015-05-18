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
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <assimp/config.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_mesh_loader.h>
#include <3dlite/3dlite_m_codec.h>

static const struct aiNode *ai_find_node_by_name(const struct aiNode *root, const char *name)
{
    uint32_t i;
    const struct aiNode *result;

    if (strcmp(root->mName.data, name) == 0)
        return root;

    for (i = 0; i < root->mNumChildren; ++i)
    {
        if ((result = ai_find_node_by_name(root->mChildren[i], name)) != NULL)
            return result;
    }

    return NULL;
}

static int ai_node_load_to_vbo(lite3d_indexed_mesh *meshInst, const struct aiScene *scene,
    const struct aiNode *node, uint16_t access)
{
    uint8_t componentSize;
    lite3d_indexed_mesh_layout layout[10];
    size_t layoutCount;
    size_t verticesSize;
    size_t indexesSize;
    void *vertices;
    void *indexes;
    register uint32_t i;

    /* one mesh - one batch on single VBO */
    for (i = 0; i < node->mNumMeshes; ++i)
    {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        register float *pvertices;
        register uint32_t j;

        if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Skiping no triangles mesh..");
            continue;
        }

        layoutCount = 0;
        verticesSize = mesh->mNumVertices * sizeof (float) * 3;
        layout[layoutCount].binding = LITE3D_BUFFER_BINDING_ATTRIBUTE;
        layout[layoutCount].count = 3;
        layoutCount++;

        if (mesh->mNormals)
        {
            verticesSize += mesh->mNumVertices * sizeof (float) * 3;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_ATTRIBUTE;
            layout[layoutCount].count = 3;
            layoutCount++;
        }

        if (mesh->mColors[0])
        {
            verticesSize += mesh->mNumVertices * sizeof (float) * 4;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_ATTRIBUTE;
            layout[layoutCount].count = 4;
            layoutCount++;
        }

        if (mesh->mTextureCoords[0])
        {
            verticesSize += mesh->mNumVertices * sizeof (float) * 2;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_ATTRIBUTE;
            layout[layoutCount].count = 2;
            layoutCount++;
        }

        componentSize = mesh->mNumFaces <= 0xff ? 1 : (mesh->mNumFaces <= 0xffff ? 2 : 4);
        indexesSize = componentSize * mesh->mNumFaces * 3;

        vertices = lite3d_malloc(verticesSize);
        SDL_assert_release(vertices);
        indexes = lite3d_malloc(indexesSize);
        SDL_assert_release(indexes);

        pvertices = (float *) vertices;

        for (j = 0; j < mesh->mNumVertices; ++j)
        {
            *pvertices++ = mesh->mVertices[j].x;
            *pvertices++ = mesh->mVertices[j].y;
            *pvertices++ = mesh->mVertices[j].z;

            if (mesh->mNormals)
            {
                *pvertices++ = mesh->mNormals[j].x;
                *pvertices++ = mesh->mNormals[j].y;
                *pvertices++ = mesh->mNormals[j].z;
            }

            if (mesh->mColors[0])
            {
                *pvertices++ = mesh->mColors[0][j].r;
                *pvertices++ = mesh->mColors[0][j].g;
                *pvertices++ = mesh->mColors[0][j].b;
                *pvertices++ = mesh->mColors[0][j].a;
            }

            if (mesh->mTextureCoords[0])
            {
                *pvertices++ = mesh->mTextureCoords[0][j].x;
                *pvertices++ = mesh->mTextureCoords[0][j].y;
            }
        }

        if (componentSize == 1)
        {
            register uint8_t *pindexes8 = (uint8_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                /* only triangles used */
                if (mesh->mFaces[j].mNumIndices != 3)
                    continue;

                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[0];
                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[1];
                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[2];
            }
        }
        else if (componentSize == 2)
        {
            register uint16_t *pindexes16 = (uint16_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                /* only triangles used */
                if (mesh->mFaces[j].mNumIndices != 3)
                    continue;

                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[0];
                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[1];
                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[2];
            }
        }
        else if (componentSize == 4)
        {
            register uint32_t *pindexes32 = (uint32_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                /* only triangles used */
                if (mesh->mFaces[j].mNumIndices != 3)
                    continue;

                *pindexes32++ = mesh->mFaces[j].mIndices[0];
                *pindexes32++ = mesh->mFaces[j].mIndices[1];
                *pindexes32++ = mesh->mFaces[j].mIndices[2];
            }
        }

        if (!lite3d_indexed_mesh_extend_from_memory(meshInst, vertices, mesh->mNumVertices,
            layout, layoutCount, indexes, mesh->mNumFaces, 3, access))
            return LITE3D_FALSE;

        /* set material index to currently added meshChunk */
        LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&meshInst->chunks), node)->
            materialIndex = mesh->mMaterialIndex;

        lite3d_free(vertices);
        lite3d_free(indexes);
    }

    return LITE3D_TRUE;
}

static int ai_node_load_recursive(const struct aiScene *scene, 
    const struct aiNode *node, lite3d_retrieve_mesh retrieveMesh, 
    lite3d_mesh_loaded meshLoaded, uint16_t access)
{
    uint32_t i;

    if (node->mNumMeshes > 0)
    {
        lite3d_indexed_mesh *mesh;

        if((mesh = retrieveMesh()) == NULL)
            return LITE3D_FALSE;

        if(!ai_node_load_to_vbo(mesh, scene, node, access))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s load failed..",
                node->mName.data);
            return LITE3D_FALSE;
        }

        meshLoaded(mesh, node->mName.data);
    }

    for (i = 0; i < node->mNumChildren; ++i)
    {
        if (!ai_node_load_recursive(scene, node->mChildren[i], retrieveMesh, 
            meshLoaded, access))
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

static const struct aiScene *ai_load_scene(lite3d_resource_file *resource, uint32_t flags,
    struct aiPropertyStore *importProrerties)
{
    const struct aiScene *scene = NULL;
    struct aiMemoryInfo sceneMemory;
    uint32_t aiflags;

    SDL_assert(resource);

    if (!resource->isLoaded)
        return NULL;

    /* remove this components from loaded scene */
    /* speedup loading */
    aiSetImportPropertyInteger(importProrerties, AI_CONFIG_PP_RVC_FLAGS,
        aiComponent_TANGENTS_AND_BITANGENTS |
        aiComponent_LIGHTS |
        aiComponent_CAMERAS |
        aiComponent_ANIMATIONS |
        aiComponent_BONEWEIGHTS);
    /* parse scene from memory buffered file */
    scene = aiImportFileFromMemoryWithProperties(resource->fileBuff,
        resource->fileSize, aiProcess_RemoveComponent, NULL, importProrerties);
    if (!scene)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s import failed.. %s",
            resource->name, aiGetErrorString());
        return NULL;
    }

    aiflags = aiProcess_GenSmoothNormals |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData;

    if (flags & LITE3D_OPTIMIZE_MESH_FLAG)
        aiflags |= aiProcess_OptimizeMeshes |
        aiProcess_JoinIdenticalVertices;
    if (flags & LITE3D_FLIP_UV_FLAG)
        aiflags |= aiProcess_FlipUVs;
    if (flags & LITE3D_MERGE_NODES_FLAG)
        aiflags |= aiProcess_OptimizeGraph;

    aiGetMemoryRequirements(scene, &sceneMemory);

    return aiApplyPostProcessing(scene, aiflags);
}

int lite3d_indexed_mesh_load_from_memory(lite3d_indexed_mesh *mesh,
    const void *vertices,
    size_t verticesCount,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes,
    size_t elementsCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i;
    uint8_t componentSize;
    uint16_t componentType;
    uint16_t indexPrimitive;

    SDL_assert(mesh && layout);

    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = stride * verticesCount;

    /* store vertex data to GPU memory */
    if (!lite3d_vbo_buffer(&mesh->vertexBuffer, vertices, verticesSize, access))
        return LITE3D_FALSE;

    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * elementsCount;
    /* store index data to GPU memory */
    if (!lite3d_vbo_buffer(&mesh->indexBuffer, indexes, indexesSize, access))
        return LITE3D_FALSE;

    componentType = lite3d_index_component_type_by_size(componentSize);
    indexPrimitive = lite3d_index_primitive_by_components(indexComponents);

    /* append new batch */
    if (!lite3d_indexed_mesh_append_chunk(mesh, layout, layoutCount, stride,
        componentType, indexPrimitive, elementsCount * indexComponents,
        indexesSize, 0, verticesCount, verticesSize, 0))
        return LITE3D_FALSE;

    mesh->verticesCount = verticesCount;
    mesh->elementsCount = elementsCount;

    return LITE3D_TRUE;
}

int lite3d_indexed_mesh_extend_from_memory(lite3d_indexed_mesh *mesh,
    const void *vertices,
    size_t verticesCount,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes,
    size_t elementsCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i, offsetVertices = 0,
        offsetIndexes = 0;
    uint8_t componentSize;
    uint16_t componentType;
    uint16_t indexPrimitive;

    SDL_assert(mesh && layout);

    if (lite3d_list_is_empty(&mesh->chunks))
        return lite3d_indexed_mesh_load_from_memory(mesh, vertices, verticesCount,
        layout, layoutCount, indexes, elementsCount, indexComponents, access);

    /* calculate buffer parameters */
    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = stride * verticesCount;
    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * elementsCount;
    /* expand VBO */
    offsetVertices = mesh->vertexBuffer.size;
    offsetIndexes = mesh->indexBuffer.size;
    if (!lite3d_indexed_mesh_extend(mesh, verticesSize, indexesSize, access))
        return LITE3D_FALSE;

    /* copy vertices to the end of the vertex buffer */
    if (!lite3d_vbo_subbuffer(&mesh->vertexBuffer, vertices,
        offsetVertices, verticesSize))
        return LITE3D_FALSE;

    /* copy indexes to the end of the index buffer */
    if (!lite3d_vbo_subbuffer(&mesh->indexBuffer, indexes,
        offsetIndexes, indexesSize))
        return LITE3D_FALSE;

    componentType = lite3d_index_component_type_by_size(componentSize);
    indexPrimitive = lite3d_index_primitive_by_components(indexComponents);

    /* append new batch */
    if (!lite3d_indexed_mesh_append_chunk(mesh, layout, layoutCount, stride,
        componentType, indexPrimitive, elementsCount * indexComponents,
        indexesSize, offsetIndexes, verticesCount, verticesSize, offsetVertices))
        return LITE3D_FALSE;

    mesh->verticesCount += verticesCount;
    mesh->elementsCount += elementsCount;

    return LITE3D_TRUE;
}

int lite3d_indexed_mesh_load(lite3d_indexed_mesh *mesh, lite3d_resource_file *resource,
    const char *name, uint16_t access, uint32_t flags)
{
    const struct aiScene *scene = NULL;
    const struct aiNode *targetNode = NULL;
    struct aiPropertyStore *importProrerties;

    SDL_assert(mesh);

    importProrerties = aiCreatePropertyStore();
    SDL_assert_release(importProrerties);

    if((scene = ai_load_scene(resource, flags, importProrerties)) == NULL)
    {
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    /* try to find node by name */
    if (name)
    {
        targetNode = ai_find_node_by_name(scene->mRootNode, name);
        if (!targetNode)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s not found in the %s..",
                name, resource->name);
            aiReleaseImport(scene);
            aiReleasePropertyStore(importProrerties);
            return LITE3D_FALSE;
        }
    }
    else
    {
        if (scene->mRootNode->mNumChildren > 0)
            targetNode = scene->mRootNode->mChildren[0];
    }

    if (!targetNode)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s is empty..",
            resource->name);
        aiReleaseImport(scene);
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    if (!ai_node_load_to_vbo(mesh, scene, targetNode, access))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s (%s) load failed..",
            resource->name, targetNode->mName.data);
        aiReleaseImport(scene);
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s (%s) loaded, cv/ce/cb/ %d/%d/%d",
        resource->name, targetNode->mName.data, mesh->verticesCount, mesh->elementsCount, mesh->chunkCount);
    aiReleaseImport(scene);
    aiReleasePropertyStore(importProrerties);

    return LITE3D_TRUE;
}

int lite3d_indexed_mesh_load_recursive(lite3d_resource_file *resource, 
    lite3d_retrieve_mesh retrieveMesh, lite3d_mesh_loaded meshLoaded, uint16_t access, uint32_t flags)
{
    const struct aiScene *scene = NULL;
    struct aiPropertyStore *importProrerties;
    SDL_assert(retrieveMesh);
    SDL_assert(meshLoaded);

    importProrerties = aiCreatePropertyStore();
    SDL_assert_release(importProrerties);

    if((scene = ai_load_scene(resource, flags, importProrerties)) == NULL)
    {
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    if(!ai_node_load_recursive(scene, scene->mRootNode, retrieveMesh, meshLoaded, access))
        return LITE3D_FALSE;
    aiReleaseImport(scene);
    aiReleasePropertyStore(importProrerties);

    return LITE3D_TRUE;
}

void lite3d_indexed_mesh_order_mat_indexes(lite3d_indexed_mesh *mesh)
{
    lite3d_list_node *vaoLink;
    uint32_t materialIndex = 0;
    SDL_assert(mesh);

    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node)->
            materialIndex = materialIndex++;
    }
}

int lite3d_indexed_mesh_load_from_m_file(lite3d_indexed_mesh *mesh, lite3d_resource_file *resource,
    uint16_t access)
{
    if (!resource->isLoaded)
        return LITE3D_FALSE;

    return lite3d_indexed_mesh_m_decode(mesh, resource->fileBuff, resource->fileSize,
        access);
}

