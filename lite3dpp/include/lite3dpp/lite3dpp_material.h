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
#include <lite3d/lite3d_buffers_manip.h>

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
    outype get##ptype##Parameter(const String &name, uint16_t passNo); \
    static void set##ptype##GlobalParameter(const String &name, const intype &value); \
    static outype get##ptype##GlobalParameter(const String &name);
    
    class LITE3DPP_EXPORT Material : public ConfigurableResource, public Noncopiable
    {
    public:

        enum PolygonMode : uint8_t
        {
            PolygonPoint = LITE3D_POLYMODE_POINT,
            PolygonLine = LITE3D_POLYMODE_LINE,
            PolygonFill = LITE3D_POLYMODE_FILL
        };

        using PassParameters = stl<String, lite3d_shader_parameter>::unordered_map;
        using GlobalPassParameters = stl<String>::set;
        using Passes = stl<uint16_t, std::tuple<PassParameters, GlobalPassParameters>>::map;
        
        Material(const String &name, const String &path, Main *main);

        LITE3D_DECLARE_PTR_METHODS(lite3d_material, mMaterial)

        void addPass(uint16_t passNo);
        void removePass(uint16_t pass); 
        void setPassProgram(uint16_t pass, ShaderProgram *program);
        void setPassBlendMode(uint16_t pass, bool blendEnable, uint8_t mode);
        void setPolygonMode(uint16_t pass, PolygonMode mode);
        void setDoubleSided(uint16_t pass, bool flag);
        ShaderProgram *getPassProgram(uint16_t pass) const;
        bool hasParameter(const String &name, uint16_t passNo, bool isGlobal = false) const;
        
        /* if pass == 0 parameter will be used for all passes */
        LITE3D_MATERIAL_DECLARE_PARAMETER(Float, float, float)
        LITE3D_MATERIAL_DECLARE_PARAMETER(Int, int32_t, int32_t)
        LITE3D_MATERIAL_DECLARE_PARAMETER(UInt, uint32_t, uint32_t)
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

        lite3d_shader_parameter *getParameter(const String &name, 
            uint8_t type, uint16_t passNo, bool isGlobal, bool createIfNotExist);
        void parseParameteres(const ConfigurationReader &passJson, uint16_t passNo);
        
        static lite3d_shader_parameter *getGlobalParameter(const String &name, 
            uint8_t type, bool createIfNotExist);
        
    protected:

        lite3d_material mMaterial;
        Passes mPasses;
        static PassParameters mGlobalParameters;
    };
}