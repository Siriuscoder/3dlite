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
#include <lite3dpp_pipeline/lite3dpp_bloom.h>

#include <algorithm>
#include <SDL_assert.h>
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    BloomEffect::BloomEffect(Main& main, const String &pipelineName, const String &cameraName, 
        const ConfigurationReader &pipelineConfig) : 
        mMain(main),
        mPipelineName(pipelineName),
        mCameraName(cameraName)
    {
        mMinWidth = mMain.window()->width() / 40;
        mBloomRadius = pipelineConfig.getObject(L"BLOOM").getDouble(L"BloomRadius", mBloomRadius);
        mShaderPackage = pipelineConfig.getString(L"ShaderPackage");
        init();
    }

    BloomEffect::~BloomEffect()
    {
        mMain.getResourceManager()->releaseResource(mBloomRernderer->getName());

        for (auto material : mMaterialChain)
        {
            mMain.getResourceManager()->releaseResource(material->getName());
        }

        for (auto texture : mTextureChain)
        {
            mMain.getResourceManager()->releaseResource(texture->getName());
        }

        mMain.getResourceManager()->releaseResource(mBloomRT->getName());
    }

    TextureRenderTarget &BloomEffect::getRenderTarget()
    {
        SDL_assert(mBloomRT);
        return *mBloomRT;
    }

    Texture &BloomEffect::getLastTexture()
    {
        SDL_assert(mTextureChain.size() > 0);
        return *mTextureChain.back();
    }

    void BloomEffect::init()
    {
        ConfigurationWriter bloomRenderTargetConfig;
        bloomRenderTargetConfig.set(L"Scale", 2.0f) // Первый слой блума в 2 раза меньше чем размер окна.
            .set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::BLOOM))
            .set(L"CleanColorBuf", true)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false);

        mBloomRT = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
            mPipelineName + "_" + mCameraName + "_BloomPass",
            bloomRenderTargetConfig.write());

        initTextureChain();
        initBoomScene();
    }

    bool BloomEffect::beginDrawBatch(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material)
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

    bool BloomEffect::beginSceneRender(Scene *scene, Camera *camera)
    {
        // Скинем индекс цепочки в 0 в началале рисования сцены
        mChainState = 0;
        return true;
    }

    void BloomEffect::initTextureChain()
    {
        auto width = mMain.window()->width() / 2;
        auto height = mMain.window()->height() / 2;
        String textureName = mPipelineName + "_" + mCameraName + "_bloom_slice_";

        // Аллоцируем цепочку текстур под bloom, каждая в 2 раза меньше чем предидущая. 
        // Фильтрация линейная, это нам поможет более качественно сделать эффект сглаживания света
        // Альфа канал не нужен, но текстура должна быть float формата так как блумить будем HDR изображение  
        stl<TextureImage*>::vector textureChainTmp;
        for (int i = 0; width > mMinWidth; width /= 2, height /= 2, ++i)
        {
            ConfigurationWriter textureJson;
            auto textureData = textureJson.set(L"TextureType", "2D")
                .set(L"Width", width)
                .set(L"Height", height)
                .set(L"Filtering", "Linear")
                .set(L"Wrapping", "ClampToEdge")
                .set(L"Compression", false)
                .set(L"TextureFormat", "RGB")
                .set(L"InternalFormat", LITE3D_TEXTURE_INTERNAL_RGB32F).write();

            textureChainTmp.emplace_back(
                mMain.getResourceManager()->queryResource<TextureImage>(textureName + std::to_string(i) + ".texture", 
                textureData.data(), textureData.size()));
        }

        mMiddleTexture = textureChainTmp.back();
        // Перекладываем в основную цепочку, Downsampling
        mTextureChain.insert(mTextureChain.begin(), textureChainTmp.begin(), textureChainTmp.end());
        // Upsampling
        mTextureChain.insert(mTextureChain.end(), textureChainTmp.rbegin()+1, textureChainTmp.rend());
    }

    void BloomEffect::initBoomScene()
    {
        String matName = mPipelineName + "_" + mCameraName + "_bloom_slice_";
        // Получим финишную HDR текстру сцены после прогона освещения, будем ее блумить
        Texture *combinedTexture = mMain.getResourceManager()->queryResource<TextureImage>(
            mPipelineName + "_" + mCameraName + "_combined.texture");

        BigTriSceneGenerator bloomSceneConfig(mPipelineName + "_" + mCameraName + "_BloomStage");
        mBloomRernderer = mMain.getResourceManager()->queryResource<Scene>(bloomSceneConfig.getName(), 
            bloomSceneConfig.generate().write());
        mBloomRernderer->addObserver(this);

        for (size_t i = 0; i < mTextureChain.size(); ++i)
        {
            /* Загрузим для каждого слоя шейдер, downsample а потом upsample */
            ConfigurationWriter bloomSampleMaterialConfig;
            if (i <= mTextureChain.size() / 2)
            {
                bloomSampleMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                        .set(L"Program", ConfigurationWriter()
                            .set(L"Name", "BloomDownSample.program")
                            .set(L"Path", mShaderPackage + ":shaders/json/bloom_downsample.json"))
                        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                            ConfigurationWriter()
                                .set(L"Name", "screenMatrix")
                        })
                });
            }
            else
            {
                bloomSampleMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                        .set(L"Program", ConfigurationWriter()
                            .set(L"Name", "BloomUpSample.program")
                            .set(L"Path", mShaderPackage + ":shaders/json/bloom_upsample.json"))
                        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                            ConfigurationWriter()
                                .set(L"Name", "screenMatrix"),
                            ConfigurationWriter()
                                .set(L"Name", "BloomRadius")
                                .set(L"Type", "float")
                                .set(L"Value", mBloomRadius)
                        })
                });
            }

            Material *material = mMain.getResourceManager()->queryResource<Material>(matName + std::to_string(i) + ".material",
                bloomSampleMaterialConfig.write());
            /* Установим исходную текстуру для каждого bloom прохода, каждый проход берет результат предидущего */
            material->setSamplerParameter(1, "Source", i == 0 ? *combinedTexture : *mTextureChain[i-1]);

            /* Загрузим для каждого слоя свой полноэкранный треугольник с только что созданным шейдером */
            BigTriObjectGenerator triObject(material->getName());
            SceneObject *bloomSliceObj = mBloomRernderer->addObject(matName + std::to_string(i), triObject.generate());
            /* Спозиционируем треугольники чтобы они шли один за одним и рендерились в строгом порядке */
            bloomSliceObj->setPosition(kmVec3 { 0.0f, 0.0f, i * (0.5f / mTextureChain.size()) });
            mMaterialChain.emplace_back(material);
        }
    }

    kmVec3 BloomEffect::getLumaAverage() const
    {
        SDL_assert(mMiddleTexture);
        mMiddleTexture->getPixels(0, mBloomPixels);

        auto it = mBloomPixels.cbegin();
        kmVec3 lumaAverage = KM_VEC3_ZERO;
        for (; it != mBloomPixels.cend(); it += sizeof(kmVec3))
        {
            const kmVec3 *texel = reinterpret_cast<const kmVec3 *>(&(*it));
            kmVec3Add(&lumaAverage, &lumaAverage, texel);
        }

        kmVec3Scale(&lumaAverage, &lumaAverage, 1.0f / (mBloomPixels.size() / (3 * sizeof(float))));
        return lumaAverage;
    }
}}
