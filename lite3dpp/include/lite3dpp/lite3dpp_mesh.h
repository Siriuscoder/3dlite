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
#pragma once

#include <optional>

#include <lite3d/lite3d_mesh.h>
#include <lite3dpp/lite3dpp_material.h>
#include <lite3dpp/lite3dpp_mesh_partition.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Mesh : public ConfigurableResource, public Noncopiable
    {
    public:

        struct ChunkEntity
        {
            uint32_t chunkIndexInsidePartition = 0;
            std::optional<uint32_t> boudingBoxChunkIndex;
            lite3d_mesh_chunk *chunk = nullptr;
            Material *material = nullptr;
        };

        Mesh(const String &name, const String &path, Main *main);
        virtual ~Mesh() = default;

        void applyMaterial(uint32_t materialIdx, Material *material);
        inline size_t chunksCount() const
        { return mMeshChunks.size(); }

        MeshPartition *getPartition() 
        { return mMeshPartition; }
        MeshPartition *getBoudingBoxPartition()
        { return mBoundingBoxMeshPartition; }
        ChunkEntity getChunk(size_t index) const;
        ChunkEntity getChunkBoudingBox(size_t index) const;
        void autoAssignMaterialIndexes();

        /* return the index of the appended chunk */
        ChunkEntity append(const VertexArrayWrap &vertices, const IndexArrayWrap &indices, const BufferLayout &layout,
            bool createBoundingBoxMesh = true);

        ChunkEntity append(const VertexArrayWrap &vertices, const BufferLayout &layout,
            bool createBoundingBoxMesh = true);
        
    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &config) override;
        virtual void unloadImpl() override;
        void initPartition(const ConfigurationReader &config);
        void loadModel(const ConfigurationReader &config);
        void loadAssimpModel(const ConfigurationReader &config);
        void genPlane(const kmVec2 &size);
        void genBigTriangle();
        void genSkybox(const kmVec3 &center, const kmVec3 &size);
        void genArray(const stl<kmVec3>::vector &points, const kmVec3 &bbmin, const kmVec3 &bbmax);
        void appendChunks(const MeshChunkArray &chunks, bool createBoundingBoxMesh);
        void createBoudingBox(const lite3d_mesh_chunk *chunk);

    private:

        using MeshChunks = stl<ChunkEntity>::vector;

        MeshPartition *mMeshPartition = nullptr;
        MeshPartition *mBoundingBoxMeshPartition = nullptr;
        MeshChunks mMeshChunks;
        MeshChunks mBoundingBoxMeshChunks;
    };
}

