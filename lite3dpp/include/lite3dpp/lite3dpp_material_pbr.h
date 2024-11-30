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

#include <array>
#include <lite3dpp/lite3dpp_material_multi_render.h>

#define LITE3D_DECLARE_PBR_MATERIAL_FIELD(t, n) \
    inline void set##n(const t &v, bool updateData = true) { mMaterialEntity.n = v; if (updateData) update(); } \
    inline t get##n() const { return mMaterialEntity.n; } 

namespace lite3dpp
{
    enum class PBRMaterialFlags : uint32_t
    {
        EMPTY = 0,
        NORMAL_MAPPING_TANGENT = 1u << 0,
        NORMAL_MAPPING_TANGENT_BITANGENT = 1u << 1,
    };

    enum class TextureFlags : uint32_t
    {
        EMPTY = 0,
        LOADED = 1u << 0,
        ALBEDO = 1u << 1,
        EMISSION = 1u << 2,
        ALPHA_MASK = 1u << 3,
        NORMAL_RG = 1u << 4,
        NORMAL_RGB = 1u << 5,
        AO = 1u << 6,
        SPECULAR = 1u << 7,
        ROUGHNESS = 1u << 8,
        METALLIC = 1u << 9,
        SPECULAR_ROUGNESS_METALLIC = 1u << 10,
        ROUGNESS_METALLIC = 1u << 11,
        ENVIRONMENT = 1u << 12
    };

    LITE3D_DECLARE_ENUM_OPERATORS(PBRMaterialFlags);
    LITE3D_DECLARE_ENUM_OPERATORS(TextureFlags);

    class LITE3DPP_EXPORT PBRMaterial : public MultiRenderMaterial
    {
    public:
        
        using TextureIds = std::array<std::wstring_view, 11>;
        static constexpr inline TextureIds gTextureIds = {
            L"AlbedoTexture",
            L"EmissionTexture",
            L"AlphaMaskTexture",
            L"NormalRGTexture",
            L"NormalRGBTexture",
            L"AOTexture",
            L"SpecularTexture",
            L"RoughnessTexture",
            L"MetallicTexture",
            L"SpecularRoughnessMetallicTexture",
            L"RoughnessMetallicTexture"
        };

#pragma pack(push, 1)
// Для универасальности все структуры выравнены на 16 байт с помощью reseved паддингов
        struct TextureHandleRaw
        {
            uint64_t textureHandle = 0;
            TextureFlags flags = TextureFlags::EMPTY;
            uint32_t reserved = 0;
        };

        struct PBRMaterialRaw
        {
            kmVec4 Albedo;
            kmVec4 Emission;
            kmVec4 F0;
            kmVec4 NormalScale;
            float Alpha;
            float Specular;
            float Roughness;
            float Metallic;
            float EnvDiffuse;
            float EnvSpecular;
            float Ior;
            float EmissionStrength;
            float EnvironmentScale;
            uint32_t reserved[2] = {0};
            PBRMaterialFlags Flags = PBRMaterialFlags::EMPTY;
            TextureHandleRaw Textures[8];
            TextureHandleRaw Environment;
        };

#pragma pack(pop)

        PBRMaterial(const String &name, const String &path, Main *main);

        // Индекс материала в глобальном массиве материалов
        // У каждого материала будет уникальный индекс 
        inline uint32_t getMaterialIndex() const 
        { return mMaterialIndex; }

        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, Albedo);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, Emission);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, F0);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, NormalScale);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Alpha);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Specular);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Roughness);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Metallic);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, EnvDiffuse);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, EnvSpecular);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Ior);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, EmissionStrength);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, EnvironmentScale);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(PBRMaterialFlags, Flags);

        void setTexture(Texture *texture, TextureFlags flags, size_t index, bool updateData = true);
        void setEnvironmentTexture(Texture *texture, bool updateData);
        void update();

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;

    protected:

        PBRMaterialRaw mMaterialEntity;
        uint32_t mMaterialIndex = 0;
    };
}
