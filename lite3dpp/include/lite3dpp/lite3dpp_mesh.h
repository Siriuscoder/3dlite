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
    class LITE3DPP_EXPORT Mesh : public JsonResource, public NoncopiableResource
    {
    public:
        
        typedef stl<int, Material *>::map MaterialMapping;

        Mesh(const String &name, 
            const String &path, Main *main);
        ~Mesh();

        void mapMaterial(int unit, Material *material);
        inline const MaterialMapping &getMaterialMapping() const
        { return mMaterialMapping; }
        inline lite3d_mesh *getPtr()
        { return &mMesh; }

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper);
        virtual void unloadImpl();

    private:

        MaterialMapping mMaterialMapping;
        lite3d_mesh mMesh;
    };
}

