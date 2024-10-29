/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#include <lite3dpp/lite3dpp_mesh_partition.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_mesh_loader.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    MeshPartition::MeshPartition(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MESH)
    {}

    VBO MeshPartition::vertexBuffer()
    {
        return VBO(getPtr()->vertexBuffer); 
    }

    VBO MeshPartition::indexBuffer()
    {
        return VBO(getPtr()->indexBuffer);
    }

    lite3d_mesh_chunk *MeshPartition::operator[](size_t index)
    {
        if (index >= chunksCount())
        {
            LITE3D_THROW(getName() << ": Chunk index out of range: " << index << " of " << chunksCount());
        }

        return static_cast<lite3d_mesh_chunk *>(lite3d_array_get(&mPartition.chunks, index));
    }

    void MeshPartition::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        mMode = helper.getBool(L"Dynamic", false) ? VBO::VBOMode::ModeDynamicDraw : VBO::VBOMode::ModeStaticDraw;
        if (!lite3d_mesh_init(&mPartition))
        {
            LITE3D_THROW(getName() << ": Failed to init partition");
        }

        mPartition.userdata = this;
    }

    size_t MeshPartition::usedVideoMemBytes() const
    {
        return mPartition.vertexBuffer.size + mPartition.indexBuffer.size;
    }

    void MeshPartition::unloadImpl()
    {
        lite3d_mesh_purge(&mPartition);
    }

    lite3d_mesh_chunk *MeshPartition::append(const VertexArrayWrap &vertices, const IndexArrayWrap &indices,
        const BufferLayout &layout)
    {
        // Поддерживаются только 4 байтные индексы, не вижу смысла байт и 2 байта.
        if (!lite3d_mesh_indexed_extend_from_memory(&mPartition, vertices.get<void>(), vertices.size(), 
            layout.data(), layout.size(), indices.get<void>(), indices.size(), mMode))
        {
            LITE3D_THROW(getName() << ": Failed to append mesh chunk");
        }

        return LITE3D_ARR_GET_LAST(&mPartition.chunks, lite3d_mesh_chunk);
    }

    lite3d_mesh_chunk *MeshPartition::append(const VertexArrayWrap &vertices, const BufferLayout &layout)
    {
        if (!lite3d_mesh_extend_from_memory(&mPartition, vertices.get<void>(), vertices.size(), 
            layout.data(), layout.size(), mMode))
        {
            LITE3D_THROW(getName() << ": Failed to append mesh chunk");
        }

        return LITE3D_ARR_GET_LAST(&mPartition.chunks, lite3d_mesh_chunk);
    }

    MeshChunkArray MeshPartition::loadMeshByAssimp(const String &filePath, const String &modelName, 
        uint32_t flags)
    {
#ifdef INCLUDE_ASSIMP
/*
        uint32_t flags = 0;
        if (helper.getBool(L"Optimize"))
            flags |= LITE3D_OPTIMIZE_MESH_FLAG;
        if (helper.getBool(L"FlipUV"))
            flags |= LITE3D_FLIP_UV_FLAG;
        
        if (!lite3d_assimp_mesh_load(mMesh, 
            getMain().getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
            helper.getString(L"ModelName").c_str(), 
            helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW,
            flags))
            LITE3D_THROW(getName() << ": could not load mesh chunk");
*/
#else
        LITE3D_THROW(getName() << ": assimp codec is not supported");
#endif
        return MeshChunkArray();
    }

    MeshChunkArray MeshPartition::loadMesh(const String &filePath)
    {
/*
        if (!lite3d_mesh_load_from_m_file(mMesh, 
            getMain().getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
            helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW(getName() << ": could not load mesh chunk");
*/
        return MeshChunkArray();
    }
}
