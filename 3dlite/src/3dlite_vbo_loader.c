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
    size_t offsetVertices,
    size_t offsetIndexes)
{
    lite3d_vao *vao;
    uint32_t attribIndex = 0;
    size_t i = 0;

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
                    GL_FALSE, stride, (void *) offsetVertices);
            }
                break;
            case LITE3D_BUFFER_BINDING_VERTEX:
            {
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) offsetVertices);
            }
                break;
            case LITE3D_BUFFER_BINDING_COLOR:
            {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) offsetVertices);
            }
                break;
            case LITE3D_BUFFER_BINDING_NORMAL:
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, stride,
                    (void *) offsetVertices);
            }
                break;
            case LITE3D_BUFFER_BINDING_TEXCOORD:
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(layout[i].count, GL_FLOAT, stride,
                    (void *) offsetVertices);
            }
                break;
        }

        offsetVertices += layout[i].count * sizeof (GLfloat);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vboIndexesID);
    /* end VAO binding */
    glBindVertexArray(0);

    vao->offsetIndexes = offsetIndexes;
    vao->indexType = indexComponentSize == 1 ? GL_UNSIGNED_BYTE :
        (indexComponentSize == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
    vao->elementType = indexComponents == 1 ? GL_POINTS :
        (indexComponents == 2 ? GL_LINES : GL_TRIANGLES);
    vao->elementsCount = indexesCount;
    lite3d_list_add_last_link(&vao->inVbo, &vbo->vaos);
    vbo->vaosCount++;

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
        indexComponents, componentSize, indexesCount, 0, 0))
        return LITE3D_FALSE;

    vbo->verticesCount = verticesCount;
    vbo->indexesCount = indexesCount;
    vbo->verticesSize = verticesSize;
    vbo->indexesSize = indexesSize;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
        indexComponents, componentSize, indexesCount, vbo->verticesSize, vbo->indexesSize))
        return LITE3D_FALSE;

    vbo->verticesCount += verticesCount;
    vbo->indexesCount += verticesCount;
    vbo->verticesSize += verticesSize;
    vbo->indexesSize += verticesSize;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return LITE3D_TRUE;
}

int lite3d_vbo_load(lite3d_vbo *vbo, lite3d_resource_file *resource, 
    const char *name)
{
    const struct aiScene* scene = NULL;
    SDL_assert(vbo && resource);

    if(!resource->isLoaded)
        return LITE3D_FALSE;
    ///aiGetPredefinedLogStream

    scene = aiImportFileFromMemory(resource->fileBuff, resource->fileSize, 0, NULL);
	/*
	aiProcess_GenSmoothNormals			
	aiProcess_JoinIdenticalVertices		
	aiProcess_LimitBoneWeights			
	aiProcess_RemoveRedundantMaterials  
	aiProcess_SplitLargeMeshes			
	aiProcess_Triangulate				          
	aiProcess_SortByPType               
	aiProcess_FindDegenerates           
	aiProcess_FindInvalidData         
    */
}
