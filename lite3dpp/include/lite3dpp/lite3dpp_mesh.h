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

#include <lite3d/lite3d_mesh.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_material.h>
#include <lite3dpp/lite3dpp_vbo.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Mesh : public ConfigurableResource, public Noncopiable
    {
    public:
        
        typedef stl<lite3d_vao_layout>::vector BufferLayout; 
        typedef stl<int, Material *>::map MaterialMapping;

        Mesh(const String &name, 
            const String &path, Main *main);
        ~Mesh();

        void mapMaterial(int unit, Material *material);
        inline const MaterialMapping &getMaterialMapping() const
        { return mMaterialMapping; }
        inline lite3d_mesh *getPtr()
        { return &mMesh; }

        inline VBO vertexBuffer()
        { return VBO(mMesh.vertexBuffer); }
        inline VBO indexBuffer()
        { return VBO(mMesh.indexBuffer); }
        
        template<class V, class Indx>
        void addTriangleMeshChunk(const typename stl<V>::vector &vertices,
            const typename stl<Indx>::vector &indices, const BufferLayout &layout, 
            int indexSize, VBO::VBOMode mode = VBO::ModeDynamicDraw)
        {
            if(!lite3d_mesh_indexed_extend_from_memory(&mMesh, &vertices[0], vertices.size(),
                &layout[0], layout.size(), &indices[0], indices.size(), indexSize, mode))
                LITE3D_THROW(getName() << " append mesh chunk failed..");
        }

        template<class V>
        void addTriangleMeshChunk(const typename stl<V>::vector &vertices,
            const BufferLayout &layout, VBO::VBOMode mode = VBO::ModeDynamicDraw)
        {
            if(!lite3d_mesh_extend_from_memory(&mMesh, &vertices[0], vertices.size(),
                &layout[0], layout.size(), mode))
                LITE3D_THROW(getName() << " append mesh chunk failed..");
        }
        
        size_t usedVideoMemBytes() const override;

        void genPlain(const kmVec2 &size, bool dynamic);
        void genBigTriangle(bool dynamic);
        void genSkybox(const kmVec3 &center, const kmVec3 &size, bool dynamic);
        void genArray(const stl<kmVec3>::vector &points, const kmVec3 &bbmin, const kmVec3 &bbmax, bool dynamic);

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;

    private:

        MaterialMapping mMaterialMapping;
        lite3d_mesh mMesh;
        BufferData mVertexData;
        BufferData mIndexData;
    };
}

