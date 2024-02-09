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
#include <SDL_log.h>

#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_mesh.h>

namespace lite3dpp
{
    Mesh::Mesh(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MESH)
    {}

    Mesh::~Mesh()
    {
        lite3d_mesh_purge(&mMesh);
        if (getBBPtr())
        {
            lite3d_mesh_purge(&mBBMesh);
        }
    }

    void Mesh::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_mesh_init(&mMesh);
        mMesh.userdata = this;

        if (helper.isEmpty())
            return;

        if (helper.getString(L"Model") == "Plain")
        {
            genPlain(helper.getVec2(L"PlainSize"), helper.getBool(L"Dynamic", false));
            loadBBMesh();
        }
        else if (helper.getString(L"Model") == "BigTriangle")
        {
            genBigTriangle(helper.getBool(L"Dynamic", false));
        }
        else if (helper.getString(L"Model") == "Skybox")
        {
            genSkybox(helper.getVec3(L"Center"), helper.getVec3(L"Size"), 
                helper.getBool(L"Dynamic", false));
        }
        else if (helper.getString(L"Model") == "Array")
        {
            stl<kmVec3>::vector points;
            for (auto &point : helper.getObjects(L"Data"))
            {
                points.emplace_back(point.getVec3(L"Point"));
            }

            genArray(points, helper.getVec3(L"BBMin"), helper.getVec3(L"BBMax"), helper.getBool(L"Dynamic", false));
        }
        else if (helper.getString(L"Codec", "m") == "m")
        {
            if (!lite3d_mesh_load_from_m_file(&mMesh, 
                mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                LITE3D_THROW(getName() << ": could not load mesh chunk, bad format");
            
            loadBBMesh();
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
                LITE3D_THROW(getName() << ": could not load mesh chunk, bad format");

            loadBBMesh();
        }
#endif

        if(helper.getBool(L"MaterialMappingAutoOrdered", false))
        {
            lite3d_mesh_order_mat_indexes(&mMesh);
            if (getBBPtr())
            {
                if (mMesh.chunks.size != mBBMesh.chunks.size)
                {
                    LITE3D_THROW(getName() << ": Mesh and BBMesh has different chunks count, can not reorder material indexes");
                }

                lite3d_mesh_order_mat_indexes(&mBBMesh);
            }
        }

        for(auto &matMap : helper.getObjects(L"MaterialMapping"))
        {
            mapMaterial(matMap.getInt(L"MaterialIndex"), 
                mMain->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
        }
    }

    size_t Mesh::usedVideoMemBytes() const
    {
        return mMesh.indexBuffer.size + mMesh.vertexBuffer.size + mBBMesh.vertexBuffer.size;
    }

    void Mesh::unloadImpl()
    {
        /* store buffers data */
        vertexBuffer().getData(mVertexData, 0, mMesh.vertexBuffer.size);
        indexBuffer().getData(mIndexData, 0, mMesh.indexBuffer.size);

        /* unload vbo from vmem */
        if (mMesh.vertexBuffer.size > 0)
            lite3d_vbo_buffer(&mMesh.vertexBuffer, NULL, 0, mMesh.vertexBuffer.access);
        if (mMesh.indexBuffer.size > 0)
            lite3d_vbo_buffer(&mMesh.indexBuffer, NULL, 0, mMesh.indexBuffer.access);

        if (getBBPtr() && mBBMesh.vertexBuffer.size > 0)
        {
            VBO(mBBMesh.vertexBuffer).getData(mBBVertexData, 0, mBBMesh.vertexBuffer.size);
            lite3d_vbo_buffer(&mBBMesh.vertexBuffer, NULL, 0, mBBMesh.vertexBuffer.access);
        }
    }

    void Mesh::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        /* restore data */
        if (mVertexData.size() > 0)
            if (!lite3d_vbo_buffer(&mMesh.vertexBuffer, &mVertexData[0], mVertexData.size(), mMesh.vertexBuffer.access))
                LITE3D_THROW(getName() << ": failed to reload vertex buffer");

        if (mIndexData.size() > 0)
            if (!lite3d_vbo_buffer(&mMesh.indexBuffer, &mIndexData[0], mIndexData.size(), mMesh.indexBuffer.access))
                LITE3D_THROW(getName() << ": failed to reload index buffer");

        if (mBBVertexData.size() > 0)
            if (!lite3d_vbo_buffer(&mBBMesh.vertexBuffer, &mBBVertexData[0], mBBVertexData.size(), mBBMesh.indexBuffer.access))
                LITE3D_THROW(getName() << ": failed to reload BB vertex buffer");
        
        mVertexData.clear();
        mIndexData.clear();
        mBBVertexData.clear();
    }

    void Mesh::mapMaterial(int unit, Material *material)
    {
        mMaterialMapping[unit] = material;
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

        const lite3d_vao_layout layout[] = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3},
            { LITE3D_BUFFER_BINDING_TEXCOORD, 2}
        };

        if (!lite3d_mesh_load_from_memory(&mMesh, vertices, 6, layout, 2, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Plain generation failed");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh.chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &vmin, &vmax);
    }
    
    void Mesh::genBigTriangle(bool dynamic)
    {
        kmVec3 vmax = {2, 2, 0}, vmin = {0, 0, 0};
        const float vertices[] = {
            0.0f, 0.0f,
            2.0f, 0.0f,
            0.0f, 2.0f
        };

        const lite3d_vao_layout layout[] = {
            { LITE3D_BUFFER_BINDING_VERTEX, 2}
        };

        if (!lite3d_mesh_load_from_memory(&mMesh, vertices, 3, layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("BigTriangle generation failed");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh.chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &vmin, &vmax);
    }
    
    void Mesh::genSkybox(const kmVec3 &center, const kmVec3 &size, bool dynamic)
    {
        const float skyboxVertices[] = {
            // positions          
            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),

            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),

            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),

            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),

            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z+(size.z/2),
            center.x-(size.x/2),  center.y+(size.y/2), center.z-(size.z/2),

            center.x-(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z-(size.z/2),
            center.x-(size.x/2),  center.y-(size.y/2), center.z+(size.z/2),
            center.x+(size.x/2),  center.y-(size.y/2), center.z+(size.z/2)
        };
        
        const lite3d_vao_layout layout[] = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3}
        };
        
        kmVec3 vmax = {center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2)}, 
            vmin = {center.x-(size.x/2), center.y-(size.y/2), center.z-(size.z/2)};
        
        if (!lite3d_mesh_load_from_memory(&mMesh, skyboxVertices, 36, layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Failed to create mesh");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh.chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &vmin, &vmax);
    }

    void Mesh::genArray(const stl<kmVec3>::vector &points, const kmVec3 &bbmin, const kmVec3 &bbmax, bool dynamic)
    {
        const lite3d_vao_layout layout[] = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3 }
        };

        if (!lite3d_mesh_load_from_memory(&mMesh, &points[0], static_cast<uint32_t>(points.size()), 
            layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Failed to create mesh");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh.chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &bbmin, &bbmax);
    }

    void Mesh::loadBBMesh()
    {
        lite3d_mesh_init(&mBBMesh);
        mMesh.userdata = this;

        lite3d_mesh_chunk *meshChunk;
        BufferData vertexData;
        const uint32_t boxVerticesCount = 36;

        LITE3D_ARR_FOREACH(&mMesh.chunks, lite3d_mesh_chunk, meshChunk)
        {
            vertexData.resize(boxVerticesCount * meshChunk->vertexStride);
            std::fill(vertexData.begin(), vertexData.end(), 0);
            
            kmVec3 vmin = meshChunk->boundingVol.box[0];
            kmVec3 vmax = meshChunk->boundingVol.box[7];

            const float bbVertices[] = {
                vmin.x, vmin.y, vmax.z,
                vmax.x, vmin.y, vmax.z,
                vmax.x, vmax.y, vmax.z,

                vmax.x, vmax.y, vmax.z,
                vmin.x, vmax.y, vmax.z,
                vmin.x, vmin.y, vmax.z,

                vmin.x, vmin.y, vmin.z,
                vmin.x, vmax.y, vmin.z,
                vmax.x, vmax.y, vmin.z,

                vmax.x, vmax.y, vmin.z,
                vmax.x, vmin.y, vmin.z,
                vmin.x, vmin.y, vmin.z,

                vmin.x, vmax.y, vmin.z,
                vmin.x, vmax.y, vmax.z,
                vmax.x, vmax.y, vmax.z,

                vmax.x, vmax.y, vmax.z,
                vmax.x, vmax.y, vmin.z,
                vmin.x, vmax.y, vmin.z,

                vmin.x, vmin.y, vmin.z,
                vmax.x, vmin.y, vmin.z,
                vmax.x, vmin.y, vmax.z,

                vmax.x, vmin.y, vmax.z,
                vmin.x, vmin.y, vmax.z,
                vmin.x, vmin.y, vmin.z,

                vmax.x, vmin.y, vmin.z,
                vmax.x, vmax.y, vmin.z,
                vmax.x, vmax.y, vmax.z,

                vmax.x, vmax.y, vmax.z,
                vmax.x, vmin.y, vmax.z,
                vmax.x, vmin.y, vmin.z,

                vmin.x, vmin.y, vmin.z,
                vmin.x, vmin.y, vmax.z,
                vmin.x, vmax.y, vmax.z,

                vmin.x, vmax.y, vmax.z,
                vmin.x, vmax.y, vmin.z,
                vmin.x, vmin.y, vmin.z
            };

            bool skipChunk = true;
            size_t vOffset = 0;
            for (uint32_t i = 0; i < meshChunk->layoutEntriesCount; ++i)
            {
                if (meshChunk->layout[i].binding == LITE3D_BUFFER_BINDING_VERTEX)
                {
                    if (meshChunk->layout[i].count >= 3)
                    {
                        skipChunk = false;
                    }

                    break;
                }

                vOffset += meshChunk->layout[i].count * sizeof(float);
            }

            if (skipChunk)
            {
                continue;
            }

            uint8_t *pBuffer = &vertexData[vOffset];
            for (uint32_t i = 0; i < boxVerticesCount; ++i, pBuffer += meshChunk->vertexStride)
            {
                memcpy(pBuffer, &bbVertices[i * 3], sizeof(float) * 3);
            }

            if (!lite3d_mesh_extend_from_memory(&mBBMesh, &vertexData[0], boxVerticesCount, 
                meshChunk->layout, meshChunk->layoutEntriesCount, LITE3D_VBO_STATIC_DRAW))
            {
                LITE3D_THROW("Failed to extend BB mesh chunk");
            }

            lite3d_mesh_chunk *bbMeshChunk = LITE3D_ARR_GET_LAST(&mBBMesh.chunks, lite3d_mesh_chunk);
            bbMeshChunk->materialIndex = meshChunk->materialIndex;
            bbMeshChunk->boundingVol = meshChunk->boundingVol;
        }
    }
}
