/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <tuple>

#include <lite3d/lite3d_material.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_texture.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>
#include <lite3dpp/lite3dpp_shader_program.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Material : public ConfigurableResource, public Noncopiable
    {
    public:

        typedef stl<String, std::tuple<lite3d_material_pass *, lite3d_shader_parameter> >::map MaterialParameters;
        typedef stl<String, lite3d_material_pass *>::map MaterialGlobalParametersNames;
        typedef stl<uint16_t,  lite3d_material_pass *>::map Passes;
        
        Material(const String &name, 
            const String &path, Main *main);

        ~Material();

        inline lite3d_material *getPtr()
        { return &mMaterial; }

        void addPass(uint16_t passNo);
        void removePass(uint16_t pass); 
        void setPassProgram(uint16_t pass, ShaderProgram *program);
        void setPassBlendMode(uint16_t pass, bool blendEnable, uint8_t mode);
        /* if pass == 0 parameter will be used for all passes */
        void setFloatParameter(uint16_t pass, const String &name, float value, bool isGlobal);
        void setFloatv3Parameter(uint16_t pass, const String &name, const kmVec3 &value, bool isGlobal = false);
        void setFloatv4Parameter(uint16_t pass, const String &name, const kmVec4 &value, bool isGlobal = false);
        void setFloatm3Parameter(uint16_t pass, const String &name, const kmMat3 &value, bool isGlobal = false);
        void setFloatm4Parameter(uint16_t pass, const String &name, const kmMat4 &value, bool isGlobal = false);
        void setSamplerTextureParameter(uint16_t pass, const String &name, Texture *texture, bool isGlobal = false);
        
        ShaderProgram *getPassProgram(uint16_t pass) const;
        float getFloatParameter(const String &name) const;
        kmVec3 getFloatv3Parameter(const String &name) const;
        kmVec4 getFloatv4Parameter(const String &name) const;
        kmMat3 getFloatm3Parameter(const String &name) const;
        kmMat4 getFloatm4Parameter(const String &name) const;
        Texture *getSamplerTextureParameter(const String &name) const;
        
        static void setFloatGlobalParameter(const String &name, float value);
        static void setFloatv3GlobalParameter(const String &name, const kmVec3 &value);
        static void setFloatv4GlobalParameter(const String &name, const kmVec4 &value);
        static void setFloatm3GlobalParameter(const String &name, const kmMat3 &value);
        static void setFloatm4GlobalParameter(const String &name, const kmMat4 &value);
        static void setSamplerTextureGlobalParameter(const String &name, Texture *texture);
        
        static float getFloatGlobalParameter(const String &name);
        static kmVec3 getFloatv3GlobalParameter(const String &name);
        static kmVec4 getFloatv4GlobalParameter(const String &name);
        static kmMat3 getFloatm3GlobalParameter(const String &name);
        static kmMat4 getFloatm4GlobalParameter(const String &name);
        static Texture *getSamplerTextureGlobalParameter(const String &name);
        
    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        lite3d_shader_parameter *getParameter(const String &name, 
            uint8_t type, lite3d_material_pass *passPtr, bool isGlobal);
        void addParameter(lite3d_material_pass *passPtr, lite3d_shader_parameter *parameterPtr);
        void parseParameteres(const ConfigurationReader &passJson, uint16_t passNo);
        
        static lite3d_shader_parameter *getGlobalParameter(const String &name, 
            uint8_t type);

        static float getFloatParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        static kmVec3 getFloatv3ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        static kmVec4 getFloatv4ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        static kmMat3 getFloatm3ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        static kmMat4 getFloatm4ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        static Texture *getSamplerTextureParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
        
    private:

        lite3d_material mMaterial;
        MaterialParameters mMaterialParameters;
        Passes mPasses;
        MaterialGlobalParametersNames mGlobalParamNames;
        static MaterialParameters mGlobalParameters;
    };
}