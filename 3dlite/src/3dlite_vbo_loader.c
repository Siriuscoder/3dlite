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

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_vbo_loader.h>

static int vbo_append_batch(lite3d_vbo *vbo,
    lite3d_component_layout *layout,
    size_t layoutCount,
    size_t stride,
    uint8_t indexComponents,
    uint8_t indexComponentSize,
    size_t indexesCount,
    size_t indexesSize,    
    size_t indexesOffset,
    size_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset)
{
    lite3d_vao *vao;
    uint32_t attribIndex = 0;
    size_t i = 0;
    size_t vOffset = verticesOffset;

    vao = lite3d_malloc(sizeof (lite3d_vao));
    SDL_assert_release(vao);

    if (!lite3d_vao_init(vao))
    {
        lite3d_free(vao);
        return LITE3D_FALSE;
    }

    /* VAO set current */
    glBindVertexArray(vao->vaoID);
    /* use single VBO to store all data */
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboVerticesID);
    /* bind all arrays and attribs into the current VAO */
    for (; i < layoutCount; ++i)
    {
        switch (layout[i].binding)
        {
            case LITE3D_BUFFER_BINDING_ATTRIBUTE:
            {
                glEnableVertexAttribArray(attribIndex);
                glVertexAttribPointer(attribIndex++, layout[i].count, GL_FLOAT,
                    GL_FALSE, stride, (void *) vOffset);
            }
            break;
            case LITE3D_BUFFER_BINDING_VERTEX:
            {
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) vOffset);
            }
            break;
            case LITE3D_BUFFER_BINDING_COLOR:
            {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) vOffset);
            }
            break;
            case LITE3D_BUFFER_BINDING_NORMAL:
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, stride,
                    (void *) vOffset);
            }
            break;
            case LITE3D_BUFFER_BINDING_TEXCOORD:
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) vOffset);
            }
            break;
        }

        vOffset += layout[i].count * sizeof (GLfloat);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vboIndexesID);
    /* end VAO binding */
    glBindVertexArray(0);

    vao->indexesOffset = indexesOffset;
    vao->indexType = indexComponentSize == 1 ? GL_UNSIGNED_BYTE :
        (indexComponentSize == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
    vao->elementType = indexComponents == 1 ? GL_POINTS :
        (indexComponents == 2 ? GL_LINES : GL_TRIANGLES);
    vao->elementsCount = indexesCount;
    vao->indexesSize = indexesSize;
    vao->verticesCount = verticesCount;
    vao->verticesSize = verticesSize;
    vao->verticesOffset = verticesOffset;

    lite3d_list_add_last_link(&vao->inVbo, &vbo->vaos);
    vbo->vaosCount++;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "VBO: 0x%x: BATCH 0x%x: %s, cv/ov/sv %d/%d/%d, ci/oi %d/%d",
        vbo, vao, indexComponents == 1 ? "POINTS" : (indexComponents == 2 ? "LINES" : "TRIANGLES"),
        vao->verticesCount, vao->verticesOffset, stride, vao->elementsCount, vao->indexesOffset);

    return LITE3D_TRUE;
}

static const struct aiNode *ai_find_node_by_name(const struct aiNode *root, const char *name)
{
    uint32_t i;
    const struct aiNode *result;

    if(strcmp(root->mName.data, name) == 0)
        return root;

    for(i = 0; i < root->mNumChildren; ++i)
    {
        if((result = ai_find_node_by_name(root->mChildren[i], name)) != NULL)
            return result;
    }

    return NULL;
}

static int ai_node_load_to_vbo(lite3d_vbo *vbo, const struct aiScene *scene, 
    const struct aiNode *node, uint16_t access)
{
    uint8_t componentSize;
    lite3d_component_layout layout[10];
    size_t layoutCount = 0;
    size_t verticesSize;
    size_t indexesSize;
    void *vertices;
    void *indexes;
    register float *pvertices;
    register uint8_t *pindexes;
    register uint32_t i;

    /* one mesh - one batch on single VBO */
    for(i = 0; i < node->mNumMeshes; ++i)
    {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        uint32_t j;

        if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Skiping no triangles mesh..");
            continue;
        }

        verticesSize = mesh->mNumVertices * sizeof(float) * 3;
        layout[layoutCount].binding = LITE3D_BUFFER_BINDING_VERTEX;
        layout[layoutCount].count = 3;
        layoutCount++;

        if(mesh->mNormals)
        {
            verticesSize += mesh->mNumVertices * sizeof(float) * 3;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_NORMAL;
            layout[layoutCount].count = 3;
            layoutCount++;
        }

        if(mesh->mColors[0])
        {
            verticesSize += mesh->mNumVertices * sizeof(float) * 4;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_COLOR;
            layout[layoutCount].count = 4;
            layoutCount++;
        }

        if(mesh->mTextureCoords[0])
        {
            verticesSize += mesh->mNumVertices * sizeof(float) * 2;
            layout[layoutCount].binding = LITE3D_BUFFER_BINDING_TEXCOORD;
            layout[layoutCount].count = 2;
            layoutCount++;
        }

        componentSize = mesh->mNumFaces <= 0xff ? 1 : (mesh->mNumFaces <= 0xffff ? 2 : 4);
        indexesSize = componentSize * mesh->mNumFaces * 3;

        vertices = lite3d_malloc(verticesSize);
        SDL_assert_release(vertices);
        indexes = lite3d_malloc(indexesSize);
        SDL_assert_release(indexes);

        pvertices = (float *)vertices;
        pindexes = (uint8_t *)indexes;

        for(j = 0; j < mesh->mNumVertices; ++j)
        {
            *pvertices++ = mesh->mVertices[j].x;
            *pvertices++ = mesh->mVertices[j].y;
            *pvertices++ = mesh->mVertices[j].z;

            if(mesh->mNormals)
            {
                *pvertices++ = mesh->mNormals[j].x;
                *pvertices++ = mesh->mNormals[j].y;
                *pvertices++ = mesh->mNormals[j].z;
            }

            if(mesh->mColors[0])
            {
                *pvertices++ = mesh->mColors[0][j].r;
                *pvertices++ = mesh->mColors[0][j].g;
                *pvertices++ = mesh->mColors[0][j].b;
                *pvertices++ = mesh->mColors[0][j].a;
            }

            if(mesh->mTextureCoords[0])
            {
                *pvertices++ = mesh->mTextureCoords[0][j].x;
                *pvertices++ = mesh->mTextureCoords[0][j].y;
            }
        }

        for(j = 0; j < mesh->mNumFaces; ++j)
        {
            /* only triangles used */
            if(mesh->mFaces[j].mNumIndices != 3)
                continue;

            if(componentSize == 1)
            {
                *pindexes++ = (uint8_t)mesh->mFaces[j].mIndices[0];
                *pindexes++ = (uint8_t)mesh->mFaces[j].mIndices[1];
                *pindexes++ = (uint8_t)mesh->mFaces[j].mIndices[2];
            }
            else if(componentSize == 2)
            {
                *((uint16_t *)pindexes) = (uint16_t)mesh->mFaces[j].mIndices[0];
                pindexes += sizeof(uint16_t);
                *((uint16_t *)pindexes) = (uint16_t)mesh->mFaces[j].mIndices[1];
                pindexes += sizeof(uint16_t);
                *((uint16_t *)pindexes) = (uint16_t)mesh->mFaces[j].mIndices[2];
                pindexes += sizeof(uint16_t);
            }
            else if(componentSize == 4)
            {
                *((uint32_t *)pindexes) = mesh->mFaces[j].mIndices[0];
                pindexes += sizeof(uint32_t);
                *((uint32_t *)pindexes) = mesh->mFaces[j].mIndices[1];
                pindexes += sizeof(uint32_t);
                *((uint32_t *)pindexes) = mesh->mFaces[j].mIndices[2];
                pindexes += sizeof(uint32_t);
            }
        }

        if(!lite3d_vbo_extend_from_memory(vbo, vertices, mesh->mNumVertices, 
            layout, layoutCount, indexes, mesh->mNumFaces, 3, access))
            return LITE3D_FALSE;

        /* set material index to currently added vao */
        MEMBERCAST(lite3d_vao, lite3d_list_last_link(&vbo->vaos), inVbo)->
            materialIndex = mesh->mMaterialIndex;

        lite3d_free(vertices);
        lite3d_free(indexes);
    }

    return LITE3D_TRUE;
}

int lite3d_vbo_load_from_memory(lite3d_vbo *vbo,
    void *vertices,
    size_t verticesCount,
    lite3d_component_layout *layout,
    size_t layoutCount,
    void *indexes,
    size_t indexesCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i;
    uint8_t componentSize;

    SDL_assert(vbo && layout);

    lite3d_misc_gl_error_stack_clean();
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboVerticesID);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (GLfloat);
    verticesSize = stride * verticesCount;
    /* store vertex data to GPU memory */
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, access);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vboIndexesID);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * indexesCount;
    /* store index data to GPU memory */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexesSize, indexes, access);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* append new batch */
    if (!vbo_append_batch(vbo, layout, layoutCount, stride,
        indexComponents, componentSize, indexesCount, 
        indexesSize, 0, verticesCount, verticesSize, 0))
        return LITE3D_FALSE;

    vbo->verticesCount = verticesCount;
    vbo->indexesCount = indexesCount;
    vbo->verticesSize = verticesSize;
    vbo->indexesSize = indexesSize;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "VBO: 0x%x: cv %d, ci %d",
        vbo, vbo->verticesCount, vbo->indexesCount);

    return LITE3D_TRUE;
}

int lite3d_vbo_extend_from_memory(lite3d_vbo *vbo,
    void *vertices,
    size_t verticesCount,
    lite3d_component_layout *layout,
    size_t layoutCount,
    void *indexes,
    size_t indexesCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i;
    uint8_t componentSize;

    SDL_assert(vbo && layout);

    if (lite3d_list_is_empty(&vbo->vaos))
        return lite3d_vbo_load_from_memory(vbo, vertices, verticesCount,
        layout, layoutCount, indexes, indexesCount, indexComponents, access);

    /* calculate buffer parameters */
    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (GLfloat);
    verticesSize = stride * verticesCount;
    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * indexesCount;
    /* expand VBO */
    if (!lite3d_vbo_extend(vbo, verticesSize, indexesSize, access))
        return LITE3D_FALSE;

    /* copy vertices to the end of the vertex buffer */
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboVerticesID);
    glBufferSubData(GL_ARRAY_BUFFER, vbo->verticesSize, verticesSize, vertices);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* copy indexes to the end of the index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vboIndexesID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, vbo->indexesSize, indexesSize, vertices);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* append new batch */
    if (!vbo_append_batch(vbo, layout, layoutCount, stride,
        indexComponents, componentSize, indexesCount,
        indexesSize, vbo->indexesSize, verticesCount, verticesSize, vbo->verticesSize))
        return LITE3D_FALSE;

    vbo->verticesCount += verticesCount;
    vbo->indexesCount += verticesCount;
    vbo->verticesSize += verticesSize;
    vbo->indexesSize += verticesSize;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "VBO: 0x%x: cv %d, ci %d, batches %d",
        vbo, vbo->verticesCount, vbo->indexesCount, vbo->vaosCount);

    return LITE3D_TRUE;
}

int lite3d_vbo_load(lite3d_vbo *vbo, lite3d_resource_file *resource, 
    const char *name, uint16_t access)
{
    const struct aiScene *scene = NULL;
    const struct aiNode *targetNode = NULL;
    struct aiPropertyStore *importProrerties;
    struct aiMemoryInfo sceneMemory;
    
    SDL_assert(vbo && resource);

    if(!resource->isLoaded)
        return LITE3D_FALSE;

    importProrerties = aiCreatePropertyStore();
    SDL_assert_release(importProrerties);

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
        resource->fileSize, 0, NULL, importProrerties);
    if(!scene)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s import failed.. %s",
            resource->name, aiGetErrorString());
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }
    
    aiGetMemoryRequirements(scene, &sceneMemory);
    scene = aiApplyPostProcessing(scene, 
        aiProcess_GenSmoothNormals |
        aiProcess_OptimizeMeshes |
        aiProcess_JoinIdenticalVertices | 
        aiProcess_RemoveRedundantMaterials |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_FindDegenerates |   
        aiProcess_FindInvalidData);
    
    if(!scene)
    {
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    /* try to find node by name */ 
    if(name)
    {
        targetNode = ai_find_node_by_name(scene->mRootNode, name);
        if(!targetNode)
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
        if(scene->mRootNode->mNumChildren > 0)
            targetNode = scene->mRootNode->mChildren[0];
    }

    if(!targetNode)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s is empty..",
            resource->name);
        aiReleaseImport(scene);
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    if(!ai_node_load_to_vbo(vbo, scene, targetNode, access))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s load failed..",
            resource->name);
        aiReleaseImport(scene);
        aiReleasePropertyStore(importProrerties);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MESH: %s loaded, cv/ci/cb/ %d/%d/%d",
        resource->name, vbo->verticesCount, vbo->indexesCount, vbo->vaosCount);
    aiReleaseImport(scene);
    aiReleasePropertyStore(importProrerties);

    return LITE3D_TRUE;
}
