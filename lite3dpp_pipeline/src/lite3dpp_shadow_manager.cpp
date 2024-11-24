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
#include <lite3dpp_pipeline/lite3dpp_shadow_manager.h>

#include <algorithm>
#include <SDL_assert.h>
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    ShadowManager::ShadowCaster::ShadowCaster(Main& main, const String& name, LightSceneNode* node, 
        const lite3d_camera::projectionParamsStruct &params) : 
        mLightNode(node),
        mShadowCamera(main.addCamera(name))
    {
        SDL_assert(node);
        // Ставим перспективу сразу при инициализации, считаем что конус источника света не меняется 
        if (node->getLight()->getType() == LITE3D_LIGHT_DIRECTIONAL)
        {
            mShadowCamera->setupOrtho(params.znear, params.zfar, params.left, params.right, params.bottom, params.top);
        }
        else
        {
            float clipFar = params.zfar > 0.0 ? params.zfar : mLightNode->getLight()->getInfluenceDistance();
            mShadowCamera->setupPerspective(params.znear, clipFar, 
                kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), params.aspect);
        }
    }

    kmMat4 ShadowManager::ShadowCaster::getMatrix()
    {
        SDL_assert(mShadowCamera);
        // Обновим параметры теневой камеры
        mShadowCamera->setDirection(mLightNode->getLight()->getWorldDirection());
        mShadowCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        mShadowCamera->recalcFrustum();
        // Пересчитаем теневую матрицу
        return mShadowCamera->refreshProjViewMatrix();
    }

    void ShadowManager::DynamicShadowReceiver::move(const kmVec3 &value)
    {
        SDL_assert(mNode);
        mNode->move(value);
        invalidate();
    }

    void ShadowManager::DynamicShadowReceiver::rotateAngle(const kmVec3 &axis, float angle)
    {
        SDL_assert(mNode);
        mNode->rotateAngle(axis, angle);
        invalidate();
    }

    void ShadowManager::DynamicShadowReceiver::setPosition(const kmVec3 &pos)
    {
        SDL_assert(mNode);
        mNode->setPosition(pos);
        invalidate();
    }

    const kmVec3& ShadowManager::DynamicShadowReceiver::getPosition() const
    {
        SDL_assert(mNode);
        return mNode->getPosition();
    }

    void ShadowManager::DynamicShadowReceiver::clearVisibility()
    {
        mVisibility.clear();
    }

    void ShadowManager::DynamicShadowReceiver::addVisibility(ShadowCaster* sc)
    {
        if (std::find(mVisibility.begin(), mVisibility.end(), sc) == mVisibility.end())
        {
            mVisibility.emplace_back(sc);
        }
    }

    void ShadowManager::DynamicShadowReceiver::invalidate()
    {
        for (auto shadowCaster: mVisibility)
        {
            shadowCaster->invalidate();
        }
    }

    ShadowManager::ShadowManager(Main& main, const String& pipelineName, const ConfigurationReader& conf) : 
        mMain(main)
    {
        auto shadowConf = conf.getObject(L"ShadowMaps");
        mShadowsCastersMaxCount = shadowConf.getInt(L"MaxCount", 1);
        mWidth = shadowConf.getInt(L"Width", 1024);
        mHeight = shadowConf.getInt(L"Height", 1024);
        mProjection.znear = shadowConf.getDouble(L"NearClipPlane", 1.0);
        mProjection.zfar = shadowConf.getDouble(L"FarClipPlane");
        mProjection.left = shadowConf.getObject(L"DirectionLightShadowParams").getDouble(L"LeftClipPlane");
        mProjection.right = shadowConf.getObject(L"DirectionLightShadowParams").getDouble(L"RightClipPlane");
        mProjection.bottom = shadowConf.getObject(L"DirectionLightShadowParams").getDouble(L"BottomClipPlane");
        mProjection.top = shadowConf.getObject(L"DirectionLightShadowParams").getDouble(L"TopClipPlane");
        mProjection.aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    }

    ShadowManager::~ShadowManager()
    {
        if (mCleanStage)
        {
            mMain.getResourceManager()->releaseResource(mCleanStage->getName());
        }

        if (mCleanStageMaterial)
        {
            mMain.getResourceManager()->releaseResource(mCleanStageMaterial->getName());
        }

        if (mShadowPass)
        {
            mMain.getResourceManager()->releaseResource(mShadowPass->getName());
        }

        if (mShadowMap)
        {
            mMain.getResourceManager()->releaseResource(mShadowMap->getName());
        }

        if (mShadowMatrixBuffer)
        {
            mMain.getResourceManager()->releaseResource(mShadowMatrixBuffer->getName());
        }

        if (mShadowIndexBuffer)
        {
            mMain.getResourceManager()->releaseResource(mShadowIndexBuffer->getName());
        }
    }

    ShadowManager::ShadowCaster* ShadowManager::newShadowCaster(LightSceneNode* node)
    {
        if (mShadowsCastersMaxCount == mShadowCasters.size())
        {
            LITE3D_THROW("The maximum shadow casters limit is reached: " << mShadowsCastersMaxCount);
        }

        auto index = static_cast<uint32_t>(mShadowCasters.size());
        mShadowCasters.emplace_back(std::make_unique<ShadowCaster>(mMain, node->getName() + std::to_string(index), 
            node, mProjection));
        // Запишем в источник света индекс его теневой матрицы в UBO
        node->getLight()->setUserIndex(index);
        return mShadowCasters.back().get();
    }

    ShadowManager::DynamicShadowReceiver* ShadowManager::registerShadowReceiver(SceneNode *node)
    {
        auto it = mDynamicNodes.emplace(node, DynamicShadowReceiver(node));
        return &it.first->second;
    }

    bool ShadowManager::beginUpdate(RenderTarget *rt)
    { 
        SDL_assert(mShadowMatrixBuffer);
        SDL_assert(mShadowIndexBuffer);

        mHostShadowIndexes.resize(1, 0); // Reserve 0 index for size
        // Обновим матрицы по всем источникам отбрасывающим тень которые влияют на текущий кадр
        for (uint32_t index = 0; index < mShadowCasters.size(); ++index)
        {
            auto &shadowCaster = mShadowCasters[index];
            auto mat = shadowCaster->getMatrix();
            if (shadowCaster->invalidated() && shadowCaster->getNode()->isVisible())
            {
                mShadowMatrixBuffer->setElement<kmMat4>(index, &mat);
                mHostShadowIndexes.emplace_back(index);
            }
        }

        mHostShadowIndexes[0] = static_cast<IndexVector::value_type>(mHostShadowIndexes.size()-1);
        // Если тени перересовывать не надо то просто переходим к следующией RT
        if (mHostShadowIndexes.size() == 1)
        {
            return false;
        }

        mShadowIndexBuffer->setData(&mHostShadowIndexes[0], 0, mHostShadowIndexes.size() * sizeof(IndexVector::value_type));
        return true;
    }

    bool ShadowManager::beginSceneRender(Scene *scene, Camera *camera)
    {
        if (scene == mCleanStage)
        {
            // Так как мы используем texture_array для хранения теневых карт мы в режиме layered render мы не можем подчистить
            // отдельную карту теней, а перерисовываем мы не все. Для очистки только нужных теневых карт используем предварительный 
            // проход с BigTriangle (сцена shadow_clean) устанавливающий во все фрагменты теневого буфера значение 1.0, но дело в том что его надо 
            // выполянть без проверки глубины, а при выключении ZTEST запись в буфер глубины невозможна, поэтому включаем 
            // ZTEST и устанавливаем TestFuncAlways для гарантированной перезаписи буфера грубины. Но перед рендером основной сцены надо будет 
            // переключить обратно 
            RenderTarget::depthTestFunc(RenderTarget::TestFuncAlways);
        }

        return true; 
    }

    void ShadowManager::endSceneRender(Scene *scene, Camera *camera)
    {
        // После очистки теневых карт готовимся к перерисовке теней.
        if (scene == mCleanStage)
        {
            RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
            // Подчистим списки источников света для которых эта нода видима перед проверкой фрустума.
            for (auto& node: mDynamicNodes)
            {
                node.second.clearVisibility();
            }
        }
    }

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool ShadowManager::customVisibilityCheck(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera)
    {
        auto it = mDynamicNodes.find(node);
        DynamicShadowReceiver* dnode = it != mDynamicNodes.end() ? &it->second : nullptr;

        bool isVisible = false;
        for (auto& shadowCaster: mShadowCasters)
        {
            if (shadowCaster->getCamera()->inFrustum(*boundingVol))
            {
                if (dnode)
                {
                    // Текущая нода видима для этого истоника света, запомним это
                    dnode->addVisibility(shadowCaster.get());
                }

                if (shadowCaster->invalidated())
                {
                    isVisible = true;
                }
            }
        }

        return isVisible;
    }

    void ShadowManager::postUpdate(RenderTarget *rt)
    {
        // Валидейтим только перересованные тени, остальные будут перерисованы потом когда попадут в область видимости
        for (size_t i = 1; i < mHostShadowIndexes.size(); ++i)
        {
            mShadowCasters[mHostShadowIndexes[i]]->validate();
        }
    }

    void ShadowManager::createAuxiliaryBuffers(const String& pipelineName)
    {
        calculateLimits();

        mShadowMatrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(pipelineName + "_ShadowMatrixBuffer",
            "{\"Dynamic\": true}");
        mShadowIndexBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(pipelineName + "_ShadowIndexBuffer",
            "{\"Dynamic\": true}");

        mShadowMatrixBuffer->extendBufferBytes(sizeof(kmMat4) * mShadowsCastersMaxCount);
        mShadowIndexBuffer->extendBufferBytes(sizeof(IndexVector::value_type) * (mShadowsCastersMaxCount + 1));
        IndexVector::value_type initialZero = 0;
        mShadowIndexBuffer->setElement<IndexVector::value_type>(0, &initialZero);
    }

    void ShadowManager::calculateLimits()
    {
        int maxGeometryOutputVertices, maxGeometryTotalOutputComponents, UBOMaxSize;
        lite3d_shader_program_get_limitations(&maxGeometryOutputVertices, nullptr, &maxGeometryTotalOutputComponents);
        lite3d_vbo_get_limitations(&UBOMaxSize, nullptr, nullptr);

        // Число компонент на одну вершину в геометрическом шейдере рендера теневого атласа
        // Константа связана с кодом шейдера!!!
        const uint32_t componentsByVertex = 7; // UV + Position + drawId
        uint32_t a = maxGeometryTotalOutputComponents / (componentsByVertex * 3);
        uint32_t b = maxGeometryOutputVertices / 3;
        uint32_t c = UBOMaxSize / sizeof(kmMat4);

        uint32_t shadowCastersLimit = std::min(std::min(a, b), c);

        if (mShadowsCastersMaxCount > shadowCastersLimit)
        {
            LITE3D_THROW("Too much shadow casters count(" << mShadowsCastersMaxCount << ") are requested, "
                "max hardware posible limit is " << shadowCastersLimit);
        }

        ShaderProgram::addDefinition("LITE3D_SPOT_SHADOW_GS_MAX_VERTICES", std::to_string(mShadowsCastersMaxCount * 3));
        ShaderProgram::addDefinition("LITE3D_SPOT_SHADOW_MAX_COUNT", std::to_string(mShadowsCastersMaxCount));
    }

    void ShadowManager::createShadowRenderTarget(const String& pipelineName)
    {
        auto shadowMapName = pipelineName + "_ShadowMap.texture";
        ConfigurationWriter shadowTextureConfig;
        shadowTextureConfig.set(L"TextureType", "2D_SHADOW_ARRAY")
            .set(L"Filtering", "Linear")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "DEPTH")
            .set(L"Height", mHeight)
            .set(L"Width", mWidth)
            .set(L"Depth", mShadowsCastersMaxCount);

        mShadowMap = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(shadowMapName, shadowTextureConfig.write());

        ConfigurationWriter shadowRenderTargetConfig;
        shadowRenderTargetConfig.set(L"Width", mWidth)
            .set(L"Height", mHeight)
            .set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::ShadowMap))
            .set(L"CleanColorBuf", false)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false)
            .set(L"LayeredFramebuffer", true)
            .set(L"DepthAttachments", ConfigurationWriter()
                .set(L"TextureName", shadowMapName));

        mShadowPass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(pipelineName + "_ShadowPass",
            shadowRenderTargetConfig.write());
        mShadowPass->addObserver(this);
    }

    void ShadowManager::initialize(const String& pipelineName, const String& shaderPackage)
    {
        createAuxiliaryBuffers(pipelineName);
        createShadowRenderTarget(pipelineName);

        // Создание специальной сцены для предварительной частичной очистки теневых карт которые надо перерисовать в текущем кадре.
        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget(mShadowPass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::ShadowCleanStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::ShadowPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", false)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true));
            
        mCleanStage = mMain.getResourceManager()->queryResourceFromJson<Scene>(pipelineName + "_ShadowCleanStage",
            stageGenerator.generate().write());

        ConfigurationWriter cleanStageMaterialConfig;
        cleanStageMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::ShadowPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "ShadowMapClean.program")
                    .set(L"Path", shaderPackage + ":shaders/json/shadow_map_clean.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "screenMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "ShadowIndex")
                        .set(L"UBOName", mShadowIndexBuffer->getName())
                        .set(L"Type", "UBO")
                })
        });
        
        // Создаем служебный шейдер отвечающий за очистку теневых карт
        mCleanStageMaterial = mMain.getResourceManager()->queryResourceFromJson<Material>(
            pipelineName + "_ShadowCleanStage.material", cleanStageMaterialConfig.write());

        // Добавляем шейдер очистки на сцену 
        mCleanStage->addObject("ShadowCleanBigTri", BigTriObjectGenerator(mCleanStageMaterial->getName()).generate());
        mCleanStage->addObserver(this);
    }
}}
