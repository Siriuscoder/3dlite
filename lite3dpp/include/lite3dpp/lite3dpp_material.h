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

#include <tuple>

#include <lite3d/lite3d_material.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_texture.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>
#include <lite3dpp/lite3dpp_ssbo.h>
#include <lite3dpp/lite3dpp_ubo.h>
#include <lite3dpp/lite3dpp_shader_program.h>

namespace lite3dpp
{
#define LITE3D_MATERIAL_DECLARE_PARAMETER(ptype, intype, outype) \
    void set##ptype##Parameter(uint16_t pass, const String &name, const intype &value, bool isGlobal = false);\
    outype get##ptype##Parameter(const String &name) const; \
    static void set##ptype##GlobalParameter(const String &name, const intype &value); \
    static outype get##ptype##GlobalParameter(const String &name);\
    static outype get##ptype##ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params);
    
    class LITE3DPP_EXPORT Material : public ConfigurableResource, public Noncopiable
    {
    public:

        typedef stl<String, std::tuple<lite3d_material_pass *, lite3d_shader_parameter> >::unordered_map MaterialParameters;
        typedef stl<String, lite3d_material_pass *>::unordered_map MaterialGlobalParametersNames;
        typedef stl<uint16_t,  lite3d_material_pass *>::map Passes;
        
        Material(const String &name, 
            const String &path, Main *main);

        ~Material();

        LITE3D_DECLARE_PTR_METHODS(lite3d_material, mMaterial)

        void addPass(uint16_t passNo);
        void removePass(uint16_t pass); 
        void setPassProgram(uint16_t pass, ShaderProgram *program);
        void setPassBlendMode(uint16_t pass, bool blendEnable, uint8_t mode);
        ShaderProgram *getPassProgram(uint16_t pass) const;
        
        /* if pass == 0 parameter will be used for all passes */
        LITE3D_MATERIAL_DECLARE_PARAMETER(Float, float, float)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Int, int32_t, int32_t)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Floatv3, kmVec3, kmVec3)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Floatv4, kmVec4, kmVec4)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Floatm3, kmMat3, kmMat3)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Floatm4, kmMat4, kmMat4)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Sampler, Texture, Texture *)
        LITE3D_MATERIAL_DECLARE_PARAMETER(SSBO, SSBO, SSBO *)
        LITE3D_MATERIAL_DECLARE_PARAMETER(UBO, UBO, UBO *)

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
        
    private:

        lite3d_material mMaterial;
        MaterialParameters mMaterialParameters;
        Passes mPasses;
        MaterialGlobalParametersNames mGlobalParamNames;
        static MaterialParameters mGlobalParameters;
    };
}