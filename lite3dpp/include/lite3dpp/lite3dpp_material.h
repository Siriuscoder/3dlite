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

#include <tuple>

#include <lite3d/lite3d_material.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_json_helper.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_texture.h>
#include <lite3dpp/lite3dpp_shader_program.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Material : public JsonResource, public NoncopiableResource
    {
    public:

        typedef stl<String, std::tuple<lite3d_material_pass *, lite3d_shader_parameter> >::map MaterialParameters;
        typedef stl<uint16_t,  lite3d_material_pass *>::map Passes;
        
        Material(const String &name, 
            const String &path, Main *main);

        ~Material();

        inline lite3d_material *getPtr()
        { return &mMaterial; }

        void addPass(uint16_t passNo);
        void removePass(uint16_t pass); 
        void setPassProgram(uint16_t pass, ShaderProgram *program);
        /* if pass == 0 parameter will be used for all passes */
        void setFloatParameter(uint16_t pass, const String &name, float value);
        void setFloatv3Parameter(uint16_t pass, const String &name, const kmVec3 &value);
        void setFloatv4Parameter(uint16_t pass, const String &name, const kmVec4 &value);
        void setFloatm3Parameter(uint16_t pass, const String &name, const kmMat3 &value);
        void setFloatm4Parameter(uint16_t pass, const String &name, const kmMat4 &value);
        void setSamplerTextureParameter(uint16_t pass, const String &name, Texture *texture);
        
        ShaderProgram *getPassProgram(uint16_t pass) const;
        float getFloatParameter(const String &name) const;
        kmVec3 getFloatv3Parameter(const String &name) const;
        kmVec4 getFloatv4Parameter(const String &name) const;
        kmMat3 getFloatm3Parameter(const String &name) const;
        kmMat4 getFloatm4Parameter(const String &name) const;
        Texture *getSamplerTextureParameter(const String &name) const;
        
    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override;
        virtual void loadImpl(const ResourceParameters &params) override;
        virtual void unloadImpl() override;

    private:

        lite3d_shader_parameter *getParameter(const String &name, 
            uint8_t type, uint8_t persist, lite3d_material_pass *passPtr);
        void addParameter(lite3d_material_pass *passPtr, lite3d_shader_parameter *parameterPtr);
        void parseParameteres(const JsonHelper &passJson, uint16_t passNo);

    private:

        lite3d_material mMaterial;
        MaterialParameters mMaterialParameters;
        Passes mPasses;
    };
}