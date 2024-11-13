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
    inline void set##n(const t &v) { mMaterialEntity.n = v; update(); } \
    inline t get##n() const { return mMaterialEntity.n; } 

namespace lite3dpp
{
    enum class PBRMaterialFlags : uint32_t
    {
        EMPTY = 0
    };

    enum class TextureFlags : uint32_t
    {
        EMPTY = 0,
        LOADED = 1u << 0,
        ALBEDO = 1u << 1,
        EMISSION = 1u << 2,
        ALPHA_MASK = 1u << 3,
        NORMAL = 1u << 4,
        SPECULAR = 1u << 5,
        ROUGHNESS = 1u << 6,
        METALLIC = 1u << 7,
        SPECULAR_ROUGNESS_METALLIC = 1u << 8,
        ROUGNESS_METALLIC = 1u << 9
    };

    LITE3D_DECLARE_ENUM_OPERATORS(PBRMaterialFlags);
    LITE3D_DECLARE_ENUM_OPERATORS(TextureFlags);

    class LITE3DPP_EXPORT PBRMaterial : public MultiRenderMaterial
    {
    public:
        
        using TextureIds = std::array<std::wstring_view, 9>;
        static constexpr inline TextureIds gTextureIds = {
            L"AlbedoTexture",
            L"EmissionTexture",
            L"AlphaMaskTexture",
            L"NormalTexture",
            L"SpecularTexture",
            L"RoughnessTexture",
            L"MetallicTexture",
            L"SpecularRoughnessMetallicTexture",
            L"RoughnessMetallicTexture"
        };

#pragma pack(push, 4)

        struct TextureHandleRaw
        {
            uint64_t textureHandle = 0;
            TextureFlags flags = TextureFlags::EMPTY;
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
            float GIkd;
            float GIks;
            float Ior;
            PBRMaterialFlags Flags = PBRMaterialFlags::EMPTY;
            TextureHandleRaw textures[8] = {};
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
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, GIkd);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, GIks);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Ior);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(PBRMaterialFlags, Flags);

        void addTexture(Texture *texture, TextureFlags flags);
        void update();

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;

    protected:

        PBRMaterialRaw mMaterialEntity;
        uint32_t mMaterialIndex = 0;
    };
}
