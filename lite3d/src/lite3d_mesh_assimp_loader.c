/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#ifdef INCLUDE_ASSIMP

#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <assimp/config.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_logger.h>
#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

static struct aiLogStream aiLogStream;

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

static int ai_node_load_to_vbo(lite3d_mesh *meshInst, const struct aiScene *scene,
    const struct aiNode *node, uint16_t access)
{
    uint8_t componentSize;
    lite3d_mesh_layout layout[10];
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
        layout[layoutCount].binding = LITE3D_BUFFER_BINDING_VERTEX;
        layout[layoutCount].count = 3;
        layoutCount++;

        if (mesh->mNormals)
        {
            verticesSize += mesh->mNumVertices * sizeof (float) * 3;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_NORMAL;
            layout[layoutCount].count = 3;
            layoutCount++;
        }

        for (j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j)
        {
            if (mesh->mColors[j])
            {
                verticesSize += mesh->mNumVertices * sizeof (float) * 4;
                layout[layoutCount].binding = LITE3D_BUFFER_BINDING_COLOR;
                layout[layoutCount].count = 4;
                layoutCount++;
            }
        }

        for (j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
        {
            if (mesh->mTextureCoords[j])
            {
                verticesSize += mesh->mNumVertices * sizeof (float) * 2;
                layout[layoutCount].binding = LITE3D_BUFFER_BINDING_TEXCOORD;
                layout[layoutCount].count = 2;
                layoutCount++;
            }
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
            register uint32_t g;

            *pvertices++ = mesh->mVertices[j].x;
            *pvertices++ = mesh->mVertices[j].y;
            *pvertices++ = mesh->mVertices[j].z;

            if (mesh->mNormals)
            {
                *pvertices++ = mesh->mNormals[j].x;
                *pvertices++ = mesh->mNormals[j].y;
                *pvertices++ = mesh->mNormals[j].z;
            }

            for (g = 0; g < AI_MAX_NUMBER_OF_COLOR_SETS; ++g)
            {
                if (mesh->mColors[g])
                {
                    *pvertices++ = mesh->mColors[g][j].r;
                    *pvertices++ = mesh->mColors[g][j].g;
                    *pvertices++ = mesh->mColors[g][j].b;
                    *pvertices++ = mesh->mColors[g][j].a;
                }
            }

            for (g = 0; g < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++g)
            {
                if (mesh->mTextureCoords[g])
                {
                    *pvertices++ = mesh->mTextureCoords[g][j].x;
                    *pvertices++ = mesh->mTextureCoords[g][j].y;
                }
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

        if (!lite3d_mesh_indexed_extend_from_memory(meshInst, vertices, mesh->mNumVertices,
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
        lite3d_mesh *mesh;

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

static const struct aiScene *ai_load_scene(const lite3d_file *resource, uint32_t flags,
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

int lite3d_assimp_mesh_load(lite3d_mesh *mesh, const lite3d_file *resource,
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

int lite3d_assimp_mesh_load_recursive(const lite3d_file *resource, 
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

static void aiLogFunc(const char* message , char* user)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assimp: %s",
        message);
}

void lite3d_assimp_logging_level(int8_t level)
{
    aiEnableVerboseLogging(level == LITE3D_LOGLEVEL_VERBOSE ? AI_TRUE : AI_FALSE);
}

void lite3d_assimp_logging_init(void)
{
    aiLogStream.callback = aiLogFunc;
    aiLogStream.user = NULL;

	aiAttachLogStream(&aiLogStream);
}

void lite3d_assimp_logging_release(void)
{
    aiDetachAllLogStreams();
}

#endif

