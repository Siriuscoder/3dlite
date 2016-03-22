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
#pragma once

#include <lite3d/lite3d_mesh.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_material.h>

namespace lite3dpp
{
    class Mesh;
    class LITE3DPP_EXPORT BufferMapper : public Manageable, public Noncopiable
    {
    friend Mesh;
    private:

        BufferMapper(lite3d_vbo &source, uint16_t lockType);
        BufferMapper(const BufferMapper &other);

    public:

        BufferMapper(BufferMapper &&other);
        ~BufferMapper();

        template<class T>
        const T *getPtr() const
        { return static_cast<T *>(mPtr); }

        template<class T>
        T *getPtr()
        { return static_cast<T *>(mPtr); }

        inline size_t getSize()
        { return mSource.size; }

    private:

        lite3d_vbo &mSource;
        void *mPtr;
    };

    class LITE3DPP_EXPORT Mesh : public ConfigurableResource, public Noncopiable
    {
    public:
        
        typedef stl<unsigned char>::vector BufferData;
        typedef stl<lite3d_mesh_layout>::vector BufferLayout; 
        typedef stl<int, Material *>::map MaterialMapping;

        Mesh(const String &name, 
            const String &path, Main *main);
        ~Mesh();

        void mapMaterial(int unit, Material *material);
        inline const MaterialMapping &getMaterialMapping() const
        { return mMaterialMapping; }
        inline lite3d_mesh *getPtr()
        { return &mMesh; }
        
        template<class T>
        typename stl<T>::vector getVertexData()
        {
            typename stl<T>::vector buffer;
            if(mMesh.vertexBuffer.size > 0)
            {
                BufferMapper lock = mapVertexBuffer(LITE3D_VBO_MAP_READ_ONLY);
                buffer.resize(lock.getSize() / sizeof(T));

                memcpy(&buffer[0], lock.getPtr<void>(), buffer.size());
            }
            
            return buffer;
        }
        
        template<class T>
        typename stl<T>::vector getIndexData()
        {
            typename stl<T>::vector buffer;
            if(mMesh.indexBuffer.size > 0)
            {
                BufferMapper lock = mapIndexBuffer(LITE3D_VBO_MAP_READ_ONLY);
                buffer.resize(lock.getSize() / sizeof(T));

                memcpy(&buffer[0], lock.getPtr<void>(), buffer.size());
            }
            
            return buffer;
        }
        
        template<class V, class Indx>
        void addTriangleMeshChunk(const typename stl<V>::vector &vertices,
            const typename stl<Indx>::vector &indices, const BufferLayout &layout, 
            int indexSize, int mode = LITE3D_VBO_DYNAMIC_DRAW)
        {
            if(!lite3d_mesh_indexed_extend_from_memory(&mMesh, &vertices[0], vertices.size(),
                &layout[0], layout.size(), &indices[0], indices.size(), indexSize, mode))
                throw std::runtime_error(getName() + " append mesh chunk failed..");
        }

        template<class V>
        void addTriangleMeshChunk(const typename stl<V>::vector &vertices,
            const BufferLayout &layout, int mode = LITE3D_VBO_DYNAMIC_DRAW)
        {
            if(!lite3d_mesh_extend_from_memory(&mMesh, &vertices[0], vertices.size(),
                &layout[0], layout.size(), mode))
                throw std::runtime_error(getName() + " append mesh chunk failed..");
        }
        
        BufferMapper mapVertexBuffer(uint16_t lockType);
        BufferMapper mapIndexBuffer(uint16_t lockType);

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

