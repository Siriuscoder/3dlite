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
            LITE3D_THROW("vertex buffer map failed..");
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
        ConfigurableResource(name, path, main, AbstractResource::MESH)
    {}

    Mesh::~Mesh()
    {
        lite3d_mesh_purge(&mMesh);
    }

    void Mesh::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_mesh_init(&mMesh);
        if(helper.isEmpty())
            return;

        if(helper.getString(L"Model") == "Plain")
        {
            genPlain(helper.getVec2(L"PlainSize"), helper.getBool(L"Dynamic", false));
        }
        else if(helper.getString(L"Codec", "m") == "m")
        {
            if(!lite3d_mesh_load_from_m_file(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                LITE3D_THROW("Mesh bad format..");
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
                LITE3D_THROW("mesh bad format..");
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
        mVertexData = getVertexData<unsigned char>();
        mIndexData = getIndexData<unsigned char>();

        /* unload vbo from vmem */
        if (mMesh.vertexBuffer.size > 0)
            lite3d_vbo_buffer(&mMesh.vertexBuffer, NULL, 0, mMesh.vertexBuffer.access);
        if (mMesh.indexBuffer.size > 0)
            lite3d_vbo_buffer(&mMesh.indexBuffer, NULL, 0, mMesh.indexBuffer.access);
    }

    void Mesh::reloadFromConfigImpl(const ConfigurationReader &helper)
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

        LITE3D_THROW(getName() << " Could`t map vertex buffer.. it is empty..");
    }

    BufferMapper Mesh::mapIndexBuffer(uint16_t lockType)
    {
        if(mMesh.indexBuffer.size > 0)
            return BufferMapper(mMesh.indexBuffer, lockType);

        LITE3D_THROW(getName() << " Could`t map vertex buffer.. it is empty..");
    }

    void Mesh::genPlain(const kmVec2 &size, bool dynamic)
    {
        kmVec3 vmax = {size.x, 0, 0}, vmin = {0, -size.y, 0};
        const float vertices[] = {
            0.0f, -size.y, 0.0f, 0.0f, 1.0f,
            size.x, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, -size.y, 0.0f, 0.0f, 1.0f,
            size.x, -size.y, 0.0f, 1.0f, 1.0f,
            size.x, 0.0f, 0.0f, 1.0f, 0.0f
        };

        const lite3d_mesh_layout layout[] = {
            { LITE3D_BUFFER_BINDING_ATTRIBUTE, 3},
            { LITE3D_BUFFER_BINDING_ATTRIBUTE, 2}
        };

        if (!lite3d_mesh_load_from_memory(&mMesh, vertices, 6, layout, 2, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Plain generation failed");

        lite3d_mesh_chunk *meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&mMesh.chunks), node);
        lite3d_bouding_vol_setup(&meshChunk->boudingVol, &vmin, &vmax);
    }
}

