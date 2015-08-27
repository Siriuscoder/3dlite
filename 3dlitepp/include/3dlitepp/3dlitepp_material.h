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
#pragma once

#include <tuple>

#include <3dlite/3dlite_material.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_json_helper.h>
#include <3dlitepp/3dlitepp_resource.h>
#include <3dlitepp/3dlitepp_texture.h>
#include <3dlitepp/3dlitepp_shader_program.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Material : public JsonResource, public NoncopiableResource
    {
    public:

        typedef stl<lite3dpp_string, std::tuple<lite3d_material_pass *, lite3d_shader_parameter> >::map MaterialParameters;
        typedef stl<uint16_t,  lite3d_material_pass *>::map Passes;
        
        Material(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);

        ~Material();

        inline lite3d_material *getPtr()
        { return &mMaterial; }

        uint16_t addPass();
        void removePass(uint16_t pass); 
        void setPassProgram(uint16_t pass, ShaderProgram *program);
        /* if pass == 0 parameter will be used for all passes */
        void setFloatParameter(uint16_t pass, const lite3dpp_string &name, float value);
        void setFloatv3Parameter(uint16_t pass, const lite3dpp_string &name, const kmVec3 &value);
        void setFloatv4Parameter(uint16_t pass, const lite3dpp_string &name, const kmVec4 &value);
        void setFloatm3Parameter(uint16_t pass, const lite3dpp_string &name, const kmMat3 &value);
        void setFloatm4Parameter(uint16_t pass, const lite3dpp_string &name, const kmMat4 &value);
        void setSamplerTextureParameter(uint16_t pass, const lite3dpp_string &name, Texture *texture);
        
        ShaderProgram *getPassProgram(uint16_t pass) const;
        float getFloatParameter(uint16_t pass, const lite3dpp_string &name) const;
        kmVec3 getFloatv3Parameter(uint16_t pass, const lite3dpp_string &name) const;
        kmVec4 getFloatv4Parameter(uint16_t pass, const lite3dpp_string &name) const;
        kmMat3 getFloatm3Parameter(uint16_t pass, const lite3dpp_string &name) const;
        kmMat4 getFloatm4Parameter(uint16_t pass, const lite3dpp_string &name) const;
        
        
    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper);
        virtual void unloadImpl();

    private:

        lite3d_material mMaterial;
        MaterialParameters mMaterialParameters;
        Passes mPasses;
    };
}