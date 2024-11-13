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
        LOADED = 1 << 0
    };

    LITE3D_DECLARE_ENUM_OPERATORS(PBRMaterialFlags);
    LITE3D_DECLARE_ENUM_OPERATORS(TextureFlags);

    class LITE3DPP_EXPORT PBRMaterial : public MultiRenderMaterial
    {
    public:

#pragma pack(push, 4)

        struct RawTextureHandle
        {
            uint64_t textureHandle;
            TextureFlags flags;
        };

        struct RawPBRMaterial
        {
            kmVec4 Albedo = KM_VEC4_ONE;
            kmVec4 Emission = KM_VEC4_ONE;
            kmVec4 F0 = KM_VEC4_ONE;
            kmVec4 NormalScale = KM_VEC4_ONE;
            float Alpha = 1.0f;
            float Specular = 1.0f;
            float Roughness = 1.0f;
            float Metallic = 1.0f;
            float GIdiffuse = 1.0f;
            float GIspecular = 1.0f;
            float Ior = 1.0f;
            PBRMaterialFlags Flags = PBRMaterialFlags::EMPTY;
            RawTextureHandle textures[8] = {0};
        };

#pragma pack(pop)


        PBRMaterial(const String &name, const String &path, Main *main);

        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, Albedo);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, Emission);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, F0);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(kmVec4, NormalScale);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Alpha);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Specular);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Roughness);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Metallic);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, GIdiffuse);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, GIspecular);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(float, Ior);
        LITE3D_DECLARE_PBR_MATERIAL_FIELD(PBRMaterialFlags, Flags);

        void addTexture(Texture *texture, TextureFlags flags);
        void update();

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;

    protected:

        RawPBRMaterial mMaterialEntity;
        size_t mMaterialIndex = 0;
    };
}
