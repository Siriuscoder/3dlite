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

#include <functional>

#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    Mesh::Mesh(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MESH)
    {}

    lite3d_mesh_chunk *Mesh::getChunk(uint32_t index)
    {
        if (index >= chunksCount())
        {
            LITE3D_THROW(getName() << ": Chunk index out of range: " << index << " of " << chunksCount());
        }

        return mMeshChunks[index].chunk;
    }

    lite3d_mesh_chunk *Mesh::getChunkBoudingBox(uint32_t index)
    {
        if (index >= mBoundingBoxMeshChunks.size())
        {
            LITE3D_THROW(getName() << ": Chunk index out of range: " << index << " of " << mBoundingBoxMeshChunks.size());
        }

        return mBoundingBoxMeshChunks[index].chunk;
    }

    void Mesh::initPartition(const ConfigurationReader &config)
    {
        ConfigurationWriter partitionCfg;
        partitionCfg.set(L"Dynamic", config.getBool(L"Dynamic", false));

        auto partitionName = config.getString(L"Partition", getName() + "_partition");
        mMeshPartition = getMain().getResourceManager()->queryResourceFromJson<MeshPartition>(
            partitionName, partitionCfg.write());
    }

    void Mesh::loadFromConfigImpl(const ConfigurationReader &config)
    {
        initPartition(config);

        if (config.getString(L"Model") == "Plane")
        {
            genPlane(config.getVec2(L"PlainSize"));
        }
        else if (config.getString(L"Model") == "BigTriangle")
        {
            genBigTriangle();
        }
        else if (config.getString(L"Model") == "Skybox")
        {
            genSkybox(config.getVec3(L"Center"), config.getVec3(L"Size"));
        }
        else if (config.getString(L"Model") == "Array")
        {
            stl<kmVec3>::vector points;
            for (auto &point : config.getObjects(L"Data"))
            {
                points.emplace_back(point.getVec3(L"Point"));
            }

            genArray(points, config.getVec3(L"BBMin"), config.getVec3(L"BBMax"));
        }
        else if (config.getString(L"Codec") == "m")
        {
            loadModel(config);
        }
        else if (config.getString(L"Codec") == "assimp")
        {
            loadAssimpModel(config);
        }
        else
        {
            LITE3D_THROW(getName() << ": Unknown model type");
        }

        if (config.getBool(L"MaterialMappingAutoOrdered", false))
        {
            autoAssignMaterialIndexes();
        }

        for (auto &matMap : config.getObjects(L"MaterialMapping"))
        {
            applyMaterial(matMap.getInt(L"MaterialIndex"), 
                getMain().getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
        }
    }

    void Mesh::autoAssignMaterialIndexes()
    {
        uint32_t materialIdx = 0; 
        auto func = [&materialIdx](ChunkEntity &chunk)
        {
            SDL_assert(chunk.chunk);
            chunk.chunk->materialIndex = materialIdx++;
        };

        std::for_each(mMeshChunks.begin(), mMeshChunks.end(), func); materialIdx = 0;
        std::for_each(mBoundingBoxMeshChunks.begin(), mBoundingBoxMeshChunks.end(), func);
    }

    void Mesh::unloadImpl()
    {}

    void Mesh::applyMaterial(uint32_t materialIdx, Material *material)
    {
        for (auto &chunk : mMeshChunks)
        {
            if (chunk.chunk->materialIndex == materialIdx)
            {
                chunk.material = material;
                return;
            }
        }

        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: Failed to apply material to index %d, index is not found",
            getName().c_str(), materialIdx);
    }

    void Mesh::genPlane(const kmVec2 &size)
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

        static const stl<lite3d_vao_layout>::vector layout = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3},
            { LITE3D_BUFFER_BINDING_TEXCOORD, 2}
        };

        auto index = appendChunk(BufferWrap(vertices, 6), layout);
        lite3d_bounding_vol_setup(&mMeshChunks[index].chunk->boundingVol, &vmin, &vmax);
        initBouningBox();
    }
    
    void Mesh::genBigTriangle()
    {
        kmVec3 vmax = {2, 2, 0}, vmin = {0, 0, 0};
        const float vertices[] = {
            0.0f, 0.0f,
            2.0f, 0.0f,
            0.0f, 2.0f
        };

        static const stl<lite3d_vao_layout>::vector layout = {
            { LITE3D_BUFFER_BINDING_VERTEX, 2}
        };

        auto index = appendChunk(BufferWrap(vertices, 3), layout);
        lite3d_bounding_vol_setup(&mMeshChunks[index].chunk->boundingVol, &vmin, &vmax);
    }
    
    void Mesh::genSkybox(const kmVec3 &center, const kmVec3 &size)
    {
        SDL_assert(mMeshPartition);
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
        
        static const stl<lite3d_vao_layout>::vector layout = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3 }
        };
        
        kmVec3 vmax = {center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2)}, 
            vmin = {center.x-(size.x/2), center.y-(size.y/2), center.z-(size.z/2)};
        
        auto index = appendChunk(BufferWrap(skyboxVertices, 36), layout);
        lite3d_bounding_vol_setup(&mMeshChunks[index].chunk->boundingVol, &vmin, &vmax);
    }

    void Mesh::genArray(const stl<kmVec3>::vector &points, const kmVec3 &bbmin, const kmVec3 &bbmax)
    {
        static const stl<lite3d_vao_layout>::vector layout = {
            { LITE3D_BUFFER_BINDING_VERTEX, 3 }
        };

        auto index = appendChunk(points, layout);
        lite3d_bounding_vol_setup(&mMeshChunks[index].chunk->boundingVol, &bbmin, &bbmax);
    }

    void Mesh::initBouningBox()
    {
/*
        lite3d_mesh_init(&mSelfBBMesh);
        mSelfBBMesh.userdata = this;

        BufferData vertexData;
        const uint32_t boxVerticesCount = 36;

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
            bbMeshChunk->materialIndex = meshChunk->materialIdx;
            bbMeshChunk->boundingVol = meshChunk->boundingVol;
        }
        */
    }

    void Mesh::loadModel(const ConfigurationReader &config)
    {
        SDL_assert(mMeshPartition);
        mMeshPartition->loadMesh(config.getString(L"Model"));
        initBouningBox();
    }

    void Mesh::loadAssimpModel(const ConfigurationReader &config)
    {
        SDL_assert(mMeshPartition);
        uint32_t flags = 0;
        if (config.getBool(L"Optimize"))
            flags |= LITE3D_OPTIMIZE_MESH_FLAG;
        if (config.getBool(L"FlipUV"))
            flags |= LITE3D_FLIP_UV_FLAG;

        mMeshPartition->loadMeshByAssimp(config.getString(L"Model"), 
            config.getString(L"ModelName"), flags);
        initBouningBox();
    }

    uint32_t Mesh::appendChunk(const BufferWrap &vertices, const BufferWrap &indices, const BufferLayout &layout)
    {
        SDL_assert(mMeshPartition);
        auto chunk = mMeshPartition->appendMeshChunk(vertices, indices, layout);
        mMeshChunks.push_back(ChunkEntity {
            static_cast<uint32_t>(mMeshPartition->chunksCount() - 1),
            chunk,
            nullptr
        });

        return mMeshChunks.size() - 1;
    }

    uint32_t Mesh::appendChunk(const BufferWrap &vertices, const BufferLayout &layout)
    {
        SDL_assert(mMeshPartition);
        auto chunk = mMeshPartition->appendMeshChunk(vertices, layout);
        mMeshChunks.push_back(ChunkEntity {
            static_cast<uint32_t>(mMeshPartition->chunksCount() - 1),
            chunk,
            nullptr
        });

        return mMeshChunks.size() - 1;
    }
}
