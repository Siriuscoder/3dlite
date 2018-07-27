/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
    lite3d_mesh_layout layout[4 + AI_MAX_NUMBER_OF_COLOR_SETS + AI_MAX_NUMBER_OF_TEXTURECOORDS];
    uint32_t layoutCount;
    size_t verticesSize;
    size_t indexesSize;
    void *vertices;
    void *indexes;
    register uint32_t i;

    /* one mesh - one batch on single VBO */
    for (i = 0; i < node->mNumMeshes; ++i)
    {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        kmVec3 vmax = {0, 0, 0}, vmin = {0, 0, 0};
        lite3d_mesh_chunk *thisChunk;
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
        
        if (mesh->mTangents)
        {
            verticesSize += mesh->mNumVertices * sizeof (float) * 3;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_TANGENT;
            layout[layoutCount].count = 3;
            layoutCount++;
        }

        componentSize = sizeof(uint32_t);
        indexesSize = componentSize * mesh->mNumFaces * 3;

        vertices = lite3d_malloc(verticesSize);
        SDL_assert_release(vertices);
        indexes = lite3d_malloc(indexesSize);
        SDL_assert_release(indexes);

        pvertices = (float *) vertices;
        
        vmax.x = vmin.x = mesh->mVertices[0].x;
        vmax.y = vmin.y = mesh->mVertices[0].y;
        vmax.z = vmin.z = mesh->mVertices[0].z;

        for (j = 0; j < mesh->mNumVertices; ++j)
        {
            register uint32_t g;

            *pvertices++ = mesh->mVertices[j].x;
            *pvertices++ = mesh->mVertices[j].y;
            *pvertices++ = mesh->mVertices[j].z;
            
            vmin.x = LITE3D_MIN(mesh->mVertices[j].x, vmin.x);
            vmin.y = LITE3D_MIN(mesh->mVertices[j].y, vmin.y);
            vmin.z = LITE3D_MIN(mesh->mVertices[j].z, vmin.z);
            
            vmax.x = LITE3D_MAX(mesh->mVertices[j].x, vmax.x);
            vmax.y = LITE3D_MAX(mesh->mVertices[j].y, vmax.y);
            vmax.z = LITE3D_MAX(mesh->mVertices[j].z, vmax.z);
            
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
            
            if (mesh->mTangents)
            {
                *pvertices++ = mesh->mTangents[j].x;
                *pvertices++ = mesh->mTangents[j].y;
                *pvertices++ = mesh->mTangents[j].z;
            }
        }

        if (componentSize == sizeof(uint8_t))
        {
            register uint8_t *pindexes8 = (uint8_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[0];
                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[1];
                *pindexes8++ = (uint8_t) mesh->mFaces[j].mIndices[2];
            }
        }
        else if (componentSize == sizeof(uint16_t))
        {
            register uint16_t *pindexes16 = (uint16_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[0];
                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[1];
                *pindexes16++ = (uint16_t) mesh->mFaces[j].mIndices[2];
            }
        }
        else if (componentSize == sizeof(uint32_t))
        {
            register uint32_t *pindexes32 = (uint32_t *) indexes;

            for (j = 0; j < mesh->mNumFaces; ++j)
            {
                *pindexes32++ = mesh->mFaces[j].mIndices[0];
                *pindexes32++ = mesh->mFaces[j].mIndices[1];
                *pindexes32++ = mesh->mFaces[j].mIndices[2];
            }
        }

        if (!lite3d_mesh_indexed_extend_from_memory(meshInst, vertices, mesh->mNumVertices,
            layout, layoutCount, indexes, mesh->mNumFaces, componentSize, access))
            return LITE3D_FALSE;

        /* set material index to currently added meshChunk */
        thisChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&meshInst->chunks), node);
        thisChunk->materialIndex = mesh->mMaterialIndex;
        lite3d_bounding_vol_setup(&thisChunk->boundingVol, &vmin, &vmax);

        lite3d_free(vertices);
        lite3d_free(indexes);
    }

    return LITE3D_TRUE;
}

static int ai_load_light(const struct aiScene *scene, const struct aiNode *node, lite3d_assimp_loader_ctx ctx)
{
    uint32_t li;
    struct aiLight *light = NULL;
    lite3d_light_params params;
    kmMat4 transform;

    for (li = 0; li < scene->mNumLights; ++li)
    {
        if (strcmp(scene->mLights[li]->mName.data, node->mName.data) == 0)
        {
            light = scene->mLights[li];
            break;
        }
    }

    if (!light)
        return LITE3D_FALSE;

    memset(&params, 0, sizeof(params));
    kmMat4Fill(&transform, &node->mTransformation.a1);

    /* type */
    if (light->mType == aiLightSource_DIRECTIONAL)
        params.block1.x = LITE3D_LIGHT_DIRECTIONAL;
    else if (light->mType == aiLightSource_POINT)
        params.block1.x = LITE3D_LIGHT_POINT;
    else if (light->mType == aiLightSource_SPOT)
        params.block1.x = LITE3D_LIGHT_SPOT;

    /* enabled */
    params.block1.y = LITE3D_TRUE;
    params.block1.w = light->mPosition.x;
    params.block2.x = light->mPosition.y;
    params.block2.y = light->mPosition.z;
    params.block2.z = light->mColorAmbient.r;
    params.block2.w = light->mColorAmbient.g;
    params.block3.x = light->mColorAmbient.b;
    params.block3.y = light->mColorDiffuse.r;
    params.block3.z = light->mColorDiffuse.g;
    params.block3.w = light->mColorDiffuse.b;
    params.block4.x = light->mColorSpecular.r;
    params.block4.y = light->mColorSpecular.g;
    params.block4.z = light->mColorSpecular.b;
    params.block4.w = light->mDirection.x;
    params.block5.x = light->mDirection.y;
    params.block5.y = light->mDirection.z;
    params.block5.z = light->mAngleInnerCone;
    params.block5.w = light->mAngleOuterCone;
    params.block6.x = light->mAttenuationConstant;
    params.block6.y = light->mAttenuationLinear;
    params.block6.z = light->mAttenuationQuadratic;

    if (ctx.onLight)
        ctx.onLight(node->mName.data, &params, &transform, ctx.userdata);

    return LITE3D_TRUE;
}

static int ai_node_load_recursive(const struct aiScene *scene, 
    const struct aiNode *node, lite3d_assimp_loader_ctx ctx,
    uint16_t access)
{
    uint32_t i;
    lite3d_mesh *mesh = NULL;
    kmMat4 transform;

    if (node->mNumMeshes > 0)
    {
        if(ctx.onAllocMesh && ((mesh = ctx.onAllocMesh(ctx.userdata)) == NULL))
            return LITE3D_FALSE;

        if(!ai_node_load_to_vbo(mesh, scene, node, access))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s load failed..",
                node->mName.data);
            return LITE3D_FALSE;
        }
    }

    kmMat4Fill(&transform, &node->mTransformation.a1);

    if (ctx.onMesh)
        ctx.onMesh(mesh, &transform, node->mName.data, ctx.userdata);

    ai_load_light(scene, node, ctx);

    if (ctx.onLevelPush)
        ctx.onLevelPush(ctx.userdata);

    for (i = 0; i < node->mNumChildren; ++i)
    {
        if (!ai_node_load_recursive(scene, node->mChildren[i], ctx, access))
            return LITE3D_FALSE;
    }

    if (ctx.onLevelPop)
        ctx.onLevelPop(ctx.userdata);

    return LITE3D_TRUE;
}

static int ai_load_materials(const struct aiScene *scene, lite3d_assimp_loader_ctx ctx)
{
    uint32_t mati;
    for (mati = 0; mati < scene->mNumMaterials; ++mati)
    {
        struct aiString matName;
        struct aiString diffuseTextureFile;
        struct aiString normalTextureFile;
        struct aiString reflectionTextureFile;
        kmVec4 ambient;
        kmVec4 diffuse;
        kmVec4 specular;
        kmVec4 emissive;
        kmVec4 reflective;
        kmVec4 transparent;

        aiGetMaterialString(scene->mMaterials[mati], AI_MATKEY_NAME, &matName);
        
        if (ctx.onMaterial)
            ctx.onMaterial(matName.data, mati, 
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_AMBIENT, (struct aiColor4D *)&ambient) == aiReturn_SUCCESS ? &ambient : NULL,
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_DIFFUSE, (struct aiColor4D *)&diffuse) == aiReturn_SUCCESS ? &diffuse : NULL,
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_SPECULAR, (struct aiColor4D *)&specular) == aiReturn_SUCCESS ? &specular : NULL,
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_EMISSIVE, (struct aiColor4D *)&emissive) == aiReturn_SUCCESS ? &emissive : NULL,
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_REFLECTIVE, (struct aiColor4D *)&reflective) == aiReturn_SUCCESS ? &reflective : NULL,
            aiGetMaterialColor(scene->mMaterials[mati], AI_MATKEY_COLOR_TRANSPARENT, (struct aiColor4D *)&transparent) == aiReturn_SUCCESS ? &transparent : NULL,
            aiGetMaterialTexture(scene->mMaterials[mati], aiTextureType_DIFFUSE, 0, &diffuseTextureFile, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS ? diffuseTextureFile.data : NULL,
            aiGetMaterialTexture(scene->mMaterials[mati], aiTextureType_NORMALS, 0, &normalTextureFile, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS ? normalTextureFile.data : NULL,
            aiGetMaterialTexture(scene->mMaterials[mati], aiTextureType_REFLECTION, 0, &reflectionTextureFile, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS ? reflectionTextureFile.data : NULL,
            ctx.userdata);
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
        aiComponent_CAMERAS |
        aiComponent_ANIMATIONS |
        aiComponent_BONEWEIGHTS);
    /* parse scene from memory buffered file */
    scene = aiImportFileFromMemoryWithProperties(resource->fileBuff,
        (unsigned int)resource->fileSize, aiProcess_RemoveComponent, NULL, importProrerties);
    if (!scene)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s import failed.. %s",
            resource->name, aiGetErrorString());
        return NULL;
    }

    aiflags = aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
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

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s (%s) loaded, cv/ce/cb/ %ld/%ld/%ld",
        resource->name, targetNode->mName.data, mesh->verticesCount, mesh->elementsCount, mesh->chunkCount);
    aiReleaseImport(scene);
    aiReleasePropertyStore(importProrerties);

    return LITE3D_TRUE;
}

int lite3d_assimp_mesh_load_recursive(const lite3d_file *resource, 
    lite3d_assimp_loader_ctx ctx,
    uint16_t access, uint32_t flags)
{
    const struct aiScene *scene = NULL;
    struct aiPropertyStore *importProrerties;

    importProrerties = aiCreatePropertyStore();
    SDL_assert_release(importProrerties);

    if ((scene = ai_load_scene(resource, flags, importProrerties)) == NULL)
    {
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    if (ctx.onLevelPush)
        ctx.onLevelPush(ctx.userdata);

    if (!ai_load_materials(scene, ctx))
        return LITE3D_FALSE;

    if (!ai_node_load_recursive(scene, scene->mRootNode, ctx, access))
        return LITE3D_FALSE;

    if (ctx.onLevelPop)
        ctx.onLevelPop(ctx.userdata);

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

