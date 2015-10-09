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
#include <lite3dpp/lite3dpp_json_helper.h>
#include <lite3dpp/lite3dpp_material.h>

namespace lite3dpp
{
    class Mesh;
    class LITE3DPP_EXPORT BufferMapper : public Manageable, public NoncopiableResource
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

    class LITE3DPP_EXPORT Mesh : public JsonResource, public NoncopiableResource
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

        void getVertexData(BufferData &buffer);
        void getIndexData(BufferData &buffer);

        BufferMapper mapVertexBuffer(uint16_t lockType);
        BufferMapper mapIndexBuffer(uint16_t lockType);

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override;
        virtual void unloadImpl() override;
        virtual void reloadFromJsonImpl(const JsonHelper &helper) override;

    private:

        MaterialMapping mMaterialMapping;
        lite3d_mesh mMesh;
        BufferData mVertexData;
        BufferData mIndexData;
    };
}

