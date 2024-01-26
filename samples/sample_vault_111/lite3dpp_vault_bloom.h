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

#include <algorithm>

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class SampleBloomEffect : public SceneObserver
{
public:
    SampleBloomEffect(Main& main) : 
        mMain(main)
    {
        mMinWidth = mMain.window()->width() / 40;
    }

    void init()
    {
        initTextureChain();
        initBoomScene();
        mBloomRT = mMain.getResourceManager()->queryResource<TextureRenderTarget>("BloomComputeStep");
    }

private:

    bool beginDrawBatch(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material) override
    {
        Texture *current = mTextureChain[mChainState++];
        stl<lite3d_framebuffer_attachment>::vector attachments = {
            lite3d_framebuffer_attachment { 
                {
                    LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER,
                    0
                },
                current->getPtr()
            }
        };

        // Устанавливаем текушую текстуру из цепочки в фреймбуфер, будем рендерить в нее предидущую
        mBloomRT->replaceAttachments(attachments, LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER);
        // Размер текстуры поменялся, устанавливаем новый размер 
        mBloomRT->resize(current->getPtr()->imageWidth, current->getPtr()->imageHeight);
        // Активируем фрейм буфер повторно (приводит к установке viewport с новыми параметрами)
        mBloomRT->setActive();

        return true;
    }

    bool beginSceneRender(Scene *scene, Camera *camera) override
    {
        // Скинем индекс цепочки в 0 в началале рисования сцены
        mChainState = 0;
        return true;
    }

    void initTextureChain()
    {
        auto width = mMain.window()->width() / 2;
        auto height = mMain.window()->height() / 2;
        String textureName("bloom_slice_");

        // Аллоцируем цепочку текстур под bloom, каждая в 2 раза меньше чем предидущая. 
        // Фильтрация линейная, это нам поможет более качественно сделать эффект сглаживания света
        // Альфа канал не нужен, но текстура должна быть float формата так как блумить будем HDR изображение  
        stl<Texture*>::vector textureChainTmp;
        for (int i = 0; width > mMinWidth; width /= 2, height /= 2, ++i)
        {
            ConfigurationWriter textureJson;
            auto textureData = textureJson.set(L"TextureType", "2D")
                .set(L"Width", width)
                .set(L"Height", height)
                .set(L"Filtering", "Linear")
                .set(L"Wrapping", "ClampToEdge")
                .set(L"Compression", true)
                .set(L"TextureFormat", "RGB")
                .set(L"InternalFormat", LITE3D_TEXTURE_INTERNAL_RGB32F).write();

            textureChainTmp.emplace_back(
                mMain.getResourceManager()->queryResource<TextureImage>(textureName + std::to_string(i) + ".texture", 
                textureData.data(), textureData.size()));
        }

        // Перекладываем в основную цепочку, Downsampling
        mTextureChain.insert(mTextureChain.begin(), textureChainTmp.begin(), textureChainTmp.end());
        // Upsampling
        mTextureChain.insert(mTextureChain.end(), textureChainTmp.rbegin()+1, textureChainTmp.rend());
    }

    void initBoomScene()
    {
        mBloomRernderer = mMain.getResourceManager()->queryResource<Scene>("BloomEffectRenderer", "vault_111:scenes/bloom.json");
        mBloomRernderer->addObserver(this);

        String matName("bloom_slice_");
        // Получим финишную HDR текстру сцены после прогона освещения, будем ее блумить
        Texture *combinedLightTexture = mMain.getResourceManager()->queryResource<TextureImage>("combined.texture");

        for (size_t i = 0; i < mTextureChain.size(); ++i)
        {
            /* Загрузим для каждого слоя свой полноэкранный треугольник (используем уже готовый postprocess, материал заменим потом) */
            SceneObject *bloomSliceObj = mBloomRernderer->addObject(matName + std::to_string(i), "vault_111:objects/Postprocess.json", NULL);
            /* Загрузим для каждого слоя материал, downsample а потом upsample */
            Material *material = mMain.getResourceManager()->queryResource<Material>(matName + std::to_string(i) + ".material", 
                (i <= mTextureChain.size() / 2) ? "vault_111:materials/bloom_downsample.json" : "vault_111:materials/bloom_upsample.json");

            /* Установим исходную текстуру для каждого bloom прохода, каждый проход берет результат предидущего */
            material->setSamplerParameter(1, "Source", i == 0 ? *combinedLightTexture : *mTextureChain[i-1]);

            /* Выставим параметры полноэкранного треугольника и подменим материал */
            MeshSceneNode *bigTriangleMeshNode = static_cast<MeshSceneNode *>(bloomSliceObj->getRoot());
            bigTriangleMeshNode->frustumTest(false);
            bigTriangleMeshNode->replaceMaterial(0, material);
            mMaterialChain.emplace_back(material);

            /* Спозиционируем треугольники чтобы они шли один за одним и рендерились в строгом порядке */
            bigTriangleMeshNode->setPosition(kmVec3 { 0.0f, 0.0f, i * (0.5f / mTextureChain.size()) });
        }
    }

    Main& mMain;
    int32_t mMinWidth = 1;
    Scene* mBloomRernderer = nullptr;
    TextureRenderTarget* mBloomRT = nullptr;
    stl<Texture*>::vector mTextureChain;
    stl<Material*>::vector mMaterialChain;
    int mChainState = 0;
};

}}
