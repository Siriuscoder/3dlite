/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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

#include <lite3d/lite3d_mesh.h>
#include <lite3dpp/lite3dpp_vbo.h>
#include <lite3dpp/lite3dpp_buffer_wrapper.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT MeshPartition : public ConfigurableResource, public Noncopiable
    {
    public:

        MeshPartition(const String &name, const String &path, Main *main);
        virtual ~MeshPartition() = default;

        size_t chunksCount() const;
        LITE3D_DECLARE_PTR_METHODS(lite3d_mesh, mPartition);

        VBO vertexBuffer();
        VBO indexBuffer();

        lite3d_mesh_chunk *operator[](size_t index);
        size_t usedVideoMemBytes() const override;

        inline BufferBase::BufferUsage getUsageMode() const
        { return mUsageMode; }

        void extend(size_t extendVertexSize, size_t extendIndexSize);

        lite3d_mesh_chunk *append(const VertexArrayWrap &vertices, const IndexArrayWrap &indices, 
            const BufferLayout &layout);
        lite3d_mesh_chunk *append(const VertexArrayWrap &vertices, const BufferLayout &layout);
        MeshChunkArray loadMeshByAssimp(const String &filePath, const String &modelName, uint32_t flags);
        MeshChunkArray loadMesh(const String &filePath);
        void warmUpMemory();

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        lite3d_mesh mPartition = {0};
        BufferBase::BufferUsage mUsageMode = BufferBase::BufferUsage::ModeStaticDraw;
    };
}

