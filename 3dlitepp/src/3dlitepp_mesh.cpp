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
#include <SDL_log.h>

#include <3dlite/3dlite_mesh_loader.h>
#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_mesh.h>

namespace lite3dpp
{
    Mesh::Mesh(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::MESH)
    {}

    Mesh::~Mesh()
    {}

    void Mesh::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_indexed_mesh_init(&mMesh);
        if(helper.getString(L"Codec", "m") == "m")
        {
            if(!lite3d_indexed_mesh_load_from_m_file(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                throw std::runtime_error("Mesh bad format..");
        }
        else
        {
            uint32_t flags = 0;
            if(helper.getBool(L"Optimize"))
                flags |= LITE3D_OPTIMIZE_MESH_FLAG;
            if(helper.getBool(L"FlipUV"))
                flags |= LITE3D_FLIP_UV_FLAG;

            if(!lite3d_indexed_mesh_load(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getString(L"ModelName").c_str(), 
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW,
                flags))
                throw std::runtime_error("mesh bad format..");
        }

        if(helper.getBool(L"MaterialMappingAutoOrdered", false))
            lite3d_indexed_mesh_order_mat_indexes(&mMesh);

        for(auto &matMap : helper.getObjects(L"MaterialMapping"))
        {
            mapMaterial(matMap.getInt(L"MaterialIndex"), 
                mMain->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
        }

        setBufferedSize(mMesh.indexBuffer.size + mMesh.vertexBuffer.size);
    }

    void Mesh::unloadImpl()
    {
        lite3d_indexed_mesh_purge(&mMesh);
    }

    void Mesh::mapMaterial(int unit, Material *material)
    {
        mMaterialMapping[unit] = material;
    }
}

