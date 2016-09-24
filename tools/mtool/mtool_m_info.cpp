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
#include <iostream>

#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_mesh_codec.h>
#include <mtool/mtool_m_info.h>

MeshInfoCommand::MeshInfoCommand()
{}

void MeshInfoCommand::runImpl()
{
    printInfo(mMain.getResourceManager()->loadFileToMemory(mInputFilePath));
}

void MeshInfoCommand::parseCommandLineImpl(int argc, char *args[]) 
{
    Command::parseCommandLineImpl(argc, args);

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-i") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
            {
                mInputFilePath.assign("filesystem:");
                mInputFilePath.append(args[i + 1]);
            }
            else
                LITE3D_THROW("Missing input file");
        }
    }

    if(mInputFilePath.size() == 0)
        LITE3D_THROW("Missing input file");
}

void MeshInfoCommand::printInfo(const lite3d_file *meshFile)
{
    lite3d_mesh mesh;
    lite3d_mesh_chunk *meshChunk;
    lite3d_list_node *chunkNode;
    int chunksCount = 0;

    if (!lite3d_mesh_init(&mesh))
        return;

    if (!lite3d_mesh_m_decode(&mesh, meshFile->fileBuff,
        meshFile->fileSize, LITE3D_VBO_STATIC_READ))
    {
        lite3d_mesh_purge(&mesh);
        LITE3D_THROW("Bad format..");
    }
    
    printf("Mesh file version: %d.%d.%d\n\n", LITE3D_GET_VERSION_MAJ(mesh.version),
        LITE3D_GET_VERSION_MIN(mesh.version), LITE3D_GET_VERSION_PCH(mesh.version));
    printf("Vertex buffer:\n\n");
    printf("\tVertices count: %lu\n", mesh.verticesCount);
    printf("\tRaw size: %lu bytes\n\n", mesh.vertexBuffer.size);
    printf("Index buffer:\n\n");
    printf("\tElements count: %lu\n", mesh.elementsCount);
    printf("\tRaw size: %lu bytes\n\n", mesh.indexBuffer.size);

    printf("Chunks count: %lu\n\n", mesh.chunkCount);
    for (chunkNode = mesh.chunks.l.next; chunkNode != &mesh.chunks.l;
        chunkNode = lite3d_list_next(chunkNode), ++chunksCount)
    {
        uint32_t i;
        size_t offset = 0;

        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, chunkNode, node);

        printf("CHUNK %d\n", chunksCount);
        printf("\tMaterial index %d\n", meshChunk->materialIndex);
        printf("\tVertices count: %d\n", meshChunk->vao.verticesCount);
        printf("\tVertices size: %lu bytes\n", meshChunk->vao.verticesSize);
        printf("\tIndices count: %d\n", meshChunk->vao.indexesCount);
        printf("\tIndices size: %lu bytes\n", meshChunk->vao.indexesSize);
        printf("\tElements count: %d\n", meshChunk->vao.indexesCount / 3);
        printf("\tVertices offset: 0x%lx\n", meshChunk->vao.verticesOffset);
        printf("\tIndices offset: 0x%lx\n", meshChunk->vao.indexesOffset);
        printf("\tIndex size: %d bytes\n\n", lite3d_size_by_index_type(meshChunk->vao.indexType));

        printf("\tFORMAT\n");
        printf("\tLoc\tType\t\tData\tOffset\n");
        for (i = 0; i < meshChunk->layoutEntriesCount; ++i)
        {
            printf("\t%d\t%s\tFLOAT%d\t0x%lx\n", i,
                (meshChunk->layout[i].binding == LITE3D_BUFFER_BINDING_VERTEX ? "VERTEX\t" :
                (meshChunk->layout[i].binding == LITE3D_BUFFER_BINDING_COLOR ? "COLOR\t" :
                (meshChunk->layout[i].binding == LITE3D_BUFFER_BINDING_NORMAL ? "NORMAL\t" :
                (meshChunk->layout[i].binding == LITE3D_BUFFER_BINDING_TEXCOORD ? "TEXCOORD" : "ATTRIBUTE")))),
                meshChunk->layout[i].count, offset);

            offset += sizeof (float) * meshChunk->layout[i].count;
        }

        printf("\n\tStride: %lu bytes \n\n", offset);
    }

    lite3d_mesh_purge(&mesh);
}