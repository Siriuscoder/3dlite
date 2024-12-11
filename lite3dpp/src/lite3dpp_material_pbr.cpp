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
#include <lite3dpp/lite3dpp_material_pbr.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    PBRMaterial::PBRMaterial(const String &name, 
        const String &path, Main *main) : 
        MultiRenderMaterial(name, path, main)
    {}

    void PBRMaterial::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        MultiRenderMaterial::loadFromConfigImpl(helper);

        setAlbedo(helper.getObject(L"Albedo").getVec3(L"Value", KM_VEC3_ONE), false);
        setEmission(helper.getObject(L"Emission").getVec3(L"Value", KM_VEC3_ZERO), false);
        setF0(helper.getObject(L"F0").getVec3(L"Value", kmVec3 { 0.04, 0.04, 0.04 }), false);
        setNormalScale(helper.getObject(L"NormalScale").getVec3(L"Value", KM_VEC3_ONE), false);
        setAlpha(helper.getObject(L"Alpha").getDouble(L"Value", 1.0), false);
        setSpecular(helper.getObject(L"Specular").getDouble(L"Value", 1.0), false);
        setRoughness(helper.getObject(L"Roughness").getDouble(L"Value", 1.0), false);
        setMetallic(helper.getObject(L"Metallic").getDouble(L"Value", 1.0), false);
        setEnvDiffuse(helper.getObject(L"EnvDiffuse").getDouble(L"Value", 1.0), false);
        setEnvSpecular(helper.getObject(L"EnvSpecular").getDouble(L"Value", 1.0), false);
        setIor(helper.getObject(L"Ior").getDouble(L"Value", 1.0), false);
        setEmissionStrength(helper.getObject(L"EmissionStrength").getDouble(L"Value", 0.0), false);
        setEnvironmentUVScale(helper.getObject(L"EnvironmentUVScale").getDouble(L"Value", 1.0), false);
        setEnvironmentSingleProbeIndex(helper.getObject(L"EnvironmentSingleProbeIndex").getInt(L"Value", 0), false);

        size_t index = 0;
        for (size_t i = 0; i < gTextureIds.size(); ++i)
        {
            WString wsName(gTextureIds[i]);
            if (helper.has(wsName))
            {
                auto textureCfg = helper.getObject(wsName);
                setTexture(getMain().getResourceManager()->queryResource<TextureImage>(
                    textureCfg.getString(L"TextureName"),
                    textureCfg.getString(L"TexturePath")), 
                    static_cast<TextureFlags>(1u << (i+1)), index++, false);
            }
        }

        if (helper.has(L"EnvironmentTexture"))
        {
            auto textureCfg = helper.getObject(L"EnvironmentTexture");
            setEnvironmentTexture(getMain().getResourceManager()->queryResource<TextureImage>(
                textureCfg.getString(L"TextureName"),
                textureCfg.getString(L"TexturePath")), false);
        }

        if (helper.has(L"EnvironmentProbeTexture"))
        {
            auto textureCfg = helper.getObject(L"EnvironmentProbeTexture");
            setEnvironmentProbeTexture(getMain().getResourceManager()->queryResource<TextureImage>(
                textureCfg.getString(L"TextureName"),
                textureCfg.getString(L"TexturePath")), false);
        }

        // Добавляем этот материал к остальным в буфер
        mMaterialDataBuffer->extendBufferBytes(sizeof(PBRMaterialRaw));
        mMaterialIndex = static_cast<uint32_t>(mMaterialDataBuffer->bufferSizeBytes() / sizeof(PBRMaterialRaw)) - 1;
        mMaterialEntity.Flags = PBRMaterialFlags::NORMAL_MAPPING_TANGENT_BITANGENT;
        update();
    }

    void PBRMaterial::setTexture(Texture *texture, TextureFlags flags, size_t index, bool updateData)
    {
        SDL_assert(texture);

        constexpr size_t maxTexturesCount = sizeof(PBRMaterialRaw::Textures) / sizeof(TextureHandleRaw);
        if (index >= maxTexturesCount)
        {
            LITE3D_THROW(getName() << ": Index out of range: " << index << ", max " << maxTexturesCount-1);
        }

        mMaterialEntity.Textures[index].flags = flags | TextureFlags::LOADED;
        mMaterialEntity.Textures[index].textureHandle = texture->handle();

        if (updateData)
        {
            update();
        }
    }

    void PBRMaterial::setEnvironmentTexture(Texture *texture, bool updateData)
    {
        SDL_assert(texture);

        mMaterialEntity.Environment.flags = TextureFlags::ENVIRONMENT | TextureFlags::LOADED;
        mMaterialEntity.Environment.textureHandle = texture->handle();
        mMaterialEntity.Flags = mMaterialEntity.Flags | PBRMaterialFlags::ENVIRONMENT_TEXTURE;
        
        if (updateData)
        {
            update();
        }
    }

    void PBRMaterial::setEnvironmentProbeTexture(Texture *texture, bool updateData)
    {
        SDL_assert(texture);

        mMaterialEntity.EnvironmentProbe.flags = TextureFlags::ENVIRONMENT_PROBE | TextureFlags::LOADED;
        mMaterialEntity.EnvironmentProbe.textureHandle = texture->handle();
        mMaterialEntity.Flags = mMaterialEntity.Flags | PBRMaterialFlags::ENVIRONMENT_MULTI_PROBE;
        if (updateData)
        {
            update();
        }
    }

    void PBRMaterial::update()
    {
        SDL_assert(mMaterialDataBuffer);
        mMaterialDataBuffer->setElement<PBRMaterialRaw>(mMaterialIndex, &mMaterialEntity);
        mMaterial.materialDataBufferIndex = mMaterialIndex;
    }
}
