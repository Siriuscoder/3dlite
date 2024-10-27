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
#include <lite3dpp/lite3dpp_mesh.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    Mesh::Mesh(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MESH)
    {}

    lite3d_mesh *Mesh::getPtr()
    {
        SDL_assert(mMesh);
        return mMesh; 
    }
    
    lite3d_mesh *Mesh::getBBPtr()
    {
        if (mSelfBBMesh.version > 0)
        {
            return &mSelfBBMesh; 
        }

        return nullptr;
    }

    VBO Mesh::vertexBuffer()
    {
        return VBO(getPtr()->vertexBuffer); 
    }

    VBO Mesh::indexBuffer()
    {
        return VBO(getPtr()->indexBuffer);
    }

    lite3d_mesh_chunk *Mesh::operator[](uint32_t index)
    {
        SDL_assert(mMesh);
        if (index >= mChunksCount)
        {
            LITE3D_THROW(getName() << ": Chunk index out of range: " << index << " of " << mChunksCount);
        }

        return static_cast<lite3d_mesh_chunk *>(lite3d_array_get(&mMesh->chunks, mFirstChunkIndex + index));
    }

    void Mesh::initMesh(const ConfigurationReader &helper)
    {
        if (helper.getString(L"Codec") == "m")
        {
            if (helper.has(L"Partition"))
            {
                auto partitionMesh = getMain().getResourceManager()->queryResourceFromJson<Mesh>(
                    helper.getString(L"Partition"), LITE3D_EMPTY_JSON);

                mMesh = partitionMesh->getPtr();
                mChunksCount = mFirstChunkIndex = mMesh->chunks.size;
                return;
            }
        }

        lite3d_mesh_init(&mSelfMesh);
        mSelfMesh.userdata = this;
        mMesh = &mSelfMesh;
    }

    void Mesh::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (helper.isEmpty())
            return;

        initMesh(helper);

        if (helper.getString(L"Model") == "Plane")
        {
            genPlane(helper.getVec2(L"PlainSize"), helper.getBool(L"Dynamic", false));
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
        else if (helper.getString(L"Codec") == "m")
        {
            if (!lite3d_mesh_load_from_m_file(mMesh, 
                getMain().getResourceManager()->loadFileToMemory(helper.getString(L"Model")),
                helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                LITE3D_THROW(getName() << ": could not load mesh chunk, bad format");

            loadBBMesh();
        }
#ifdef INCLUDE_ASSIMP
        else if (helper.getString(L"Codec") == "assimp")
        {
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
                LITE3D_THROW(getName() << ": could not load mesh chunk, bad format");

            loadBBMesh();
        }
#endif

        if (helper.getBool(L"MaterialMappingAutoOrdered", false))
        {
            lite3d_mesh_order_mat_indexes(mMesh);
            if (getBBPtr())
            {
                lite3d_mesh_order_mat_indexes(&mSelfBBMesh);
            }
        }

        for (auto &matMap : helper.getObjects(L"MaterialMapping"))
        {
            applyMaterial(matMap.getInt(L"MaterialIndex"), 
                getMain().getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
        }

        recalcChunksCount();
    }

    size_t Mesh::usedVideoMemBytes() const
    {
        SDL_assert(mMesh);
        return mSelfMesh.indexBuffer.size + mSelfMesh.vertexBuffer.size + mSelfBBMesh.vertexBuffer.size;
    }

    void Mesh::unloadImpl()
    {
        if (mMesh == &mSelfMesh)
        {
            lite3d_mesh_purge(&mSelfMesh);
            mMesh = nullptr;
        }

        if (getBBPtr())
        {
            lite3d_mesh_purge(&mSelfBBMesh);
        }
    }

    void Mesh::applyMaterial(int unit, Material *material)
    {
        mMaterialMapping[unit] = material;
    }

    void Mesh::genPlane(const kmVec2 &size, bool dynamic)
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

        if (!lite3d_mesh_load_from_memory(mMesh, vertices, 6, layout, 2, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Plane generation failed");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh->chunks, lite3d_mesh_chunk);
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

        if (!lite3d_mesh_load_from_memory(mMesh, vertices, 3, layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("BigTriangle generation failed");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh->chunks, lite3d_mesh_chunk);
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
        
        if (!lite3d_mesh_load_from_memory(mMesh, skyboxVertices, 36, layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Failed to create mesh");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh->chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &vmin, &vmax);
    }

    void Mesh::genArray(const stl<kmVec3>::vector &points, const kmVec3 &bbmin, const kmVec3 &bbmax, bool dynamic)
    {
        const lite3d_vao_layout layout[] = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3 }
        };

        if (!lite3d_mesh_load_from_memory(mMesh, &points[0], static_cast<uint32_t>(points.size()), 
            layout, 1, dynamic ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            LITE3D_THROW("Failed to create mesh");

        lite3d_mesh_chunk *meshChunk = LITE3D_ARR_GET_LAST(&mMesh->chunks, lite3d_mesh_chunk);
        lite3d_bounding_vol_setup(&meshChunk->boundingVol, &bbmin, &bbmax);
    }

    void Mesh::loadBBMesh()
    {
        lite3d_mesh_init(&mSelfBBMesh);
        mSelfBBMesh.userdata = this;

        BufferData vertexData;
        const uint32_t boxVerticesCount = 36;

        recalcChunksCount();
        for (uint32_t i = 0; i < mChunksCount; ++i)
        {
            auto meshChunk = (*this)[i];
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

            if (!lite3d_mesh_extend_from_memory(&mSelfBBMesh, &vertexData[0], boxVerticesCount, 
                meshChunk->layout, meshChunk->layoutEntriesCount, LITE3D_VBO_STATIC_DRAW))
            {
                LITE3D_THROW("Failed to extend BB mesh chunk");
            }

            lite3d_mesh_chunk *bbMeshChunk = LITE3D_ARR_GET_LAST(&mSelfBBMesh.chunks, lite3d_mesh_chunk);
            bbMeshChunk->materialIndex = meshChunk->materialIndex;
            bbMeshChunk->boundingVol = meshChunk->boundingVol;
        }
    }

    void Mesh::recalcChunksCount()
    {
        mChunksCount = mMesh->chunks.size - mChunksCount;
    }
}
