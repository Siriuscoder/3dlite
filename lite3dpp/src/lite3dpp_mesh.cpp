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
#include <SDL_log.h>

#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_mesh.h>

namespace lite3dpp
{
    BufferMapper::BufferMapper(lite3d_vbo &source, uint16_t lockType) : 
        mSource(source)
    {
        if((mPtr = lite3d_vbo_map(&source, lockType)) == NULL)
            throw std::runtime_error("vertex buffer map failed..");
    }

    BufferMapper::BufferMapper(const BufferMapper &other) :
        mSource(other.mSource),
        mPtr(other.mPtr)
    {}

    BufferMapper::BufferMapper(BufferMapper &&other) : 
        mSource(other.mSource),
        mPtr(other.mPtr)
    {
        other.mPtr = NULL;
    }

    BufferMapper::~BufferMapper()
    {
        if(mPtr)
            lite3d_vbo_unmap(&mSource);
    }

    Mesh::Mesh(const String &name, 
        const String &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::MESH)
    {}

    Mesh::~Mesh()
    {
        lite3d_mesh_purge(&mMesh);
    }

    void Mesh::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_mesh_init(&mMesh);
        if(helper.isEmpty())
            return;

        if(helper.getString(L"Codec", "m") == "m")
        {
            if(!lite3d_mesh_load_from_m_file(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                throw std::runtime_error("Mesh bad format..");
        }
#ifdef INCLUDE_ASSIMP
        else
        {
            uint32_t flags = 0;
            if(helper.getBool(L"Optimize"))
                flags |= LITE3D_OPTIMIZE_MESH_FLAG;
            if(helper.getBool(L"FlipUV"))
                flags |= LITE3D_FLIP_UV_FLAG;

            if(!lite3d_assimp_mesh_load(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getString(L"ModelName").c_str(), 
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW,
                flags))
                throw std::runtime_error("mesh bad format..");
        }
#endif

        if(helper.getBool(L"MaterialMappingAutoOrdered", false))
            lite3d_mesh_order_mat_indexes(&mMesh);

        for(auto &matMap : helper.getObjects(L"MaterialMapping"))
        {
            mapMaterial(matMap.getInt(L"MaterialIndex"), 
                mMain->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
        }

        setBufferedSize(mMesh.indexBuffer.size + mMesh.vertexBuffer.size);
        mMesh.userdata = this;
    }

    void Mesh::unloadImpl()
    {
        /* store buffers data */
        getVertexData(mVertexData);
        getIndexData(mIndexData);

        /* unload vbo from vmem */
        lite3d_vbo_buffer(&mMesh.vertexBuffer, NULL, 0, mMesh.vertexBuffer.access);
        lite3d_vbo_buffer(&mMesh.indexBuffer, NULL, 0, mMesh.indexBuffer.access);
    }

    void Mesh::reloadFromJsonImpl(const JsonHelper &helper)
    {
        /* restore data */
        if(mVertexData.size() > 0)
            lite3d_vbo_buffer(&mMesh.vertexBuffer, &mVertexData[0], mVertexData.size(), mMesh.vertexBuffer.access);

        if(mIndexData.size() > 0)
            lite3d_vbo_buffer(&mMesh.indexBuffer, &mIndexData[0], mIndexData.size(), mMesh.indexBuffer.access);

        mVertexData.clear();
        mIndexData.clear();
    }

    void Mesh::mapMaterial(int unit, Material *material)
    {
        mMaterialMapping[unit] = material;
    }

    BufferMapper Mesh::mapVertexBuffer(uint16_t lockType)
    {
        if(mMesh.vertexBuffer.size > 0)
            return BufferMapper(mMesh.vertexBuffer, lockType);

        throw std::runtime_error(getName() + " Could`t map vertex buffer.. it is empty..");
    }

    BufferMapper Mesh::mapIndexBuffer(uint16_t lockType)
    {
        if(mMesh.indexBuffer.size > 0)
            return BufferMapper(mMesh.indexBuffer, lockType);

        throw std::runtime_error(getName() + " Could`t map vertex buffer.. it is empty..");
    }

    void Mesh::getVertexData(BufferData &buffer)
    {
        if(mMesh.vertexBuffer.size > 0)
        {
            BufferMapper lock = mapVertexBuffer(LITE3D_VBO_MAP_READ_ONLY);
            buffer.resize(lock.getSize());

            memcpy(&buffer[0], lock.getPtr<void>(), buffer.size());
        }
    }

    void Mesh::getIndexData(BufferData &buffer)
    {
        if(mMesh.indexBuffer.size > 0)
        {
            BufferMapper lock = mapIndexBuffer(LITE3D_VBO_MAP_READ_ONLY);
            buffer.resize(lock.getSize());

            memcpy(&buffer[0], lock.getPtr<void>(), buffer.size());
        }
    }
}

