/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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
#include <lite3dpp_pipeline/lite3dpp_pipeline_base.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    ShadowManager::VisibilityHintNode::VisibilityHintNode(SceneNodeBase *node, bool recursive) : 
        mNode(node),
        mRecursive(recursive)
    {
        SDL_assert(mNode);
        mNode->addObserver(this);

        if (mRecursive)
        {
            node->iterateAllChilds([this](SceneNodeBase *childNode)
            {
                SDL_assert(childNode);
                childNode->addObserver(this);
            });
        }
    }

    ShadowManager::VisibilityHintNode::~VisibilityHintNode()
    {
        mNode->removeObserver(this);
        if (mRecursive)
        {
            mNode->iterateAllChilds([this](SceneNodeBase *childNode)
            {
                SDL_assert(childNode);
                childNode->removeObserver(this);
            });
        }
    }

    void ShadowManager::VisibilityHintNode::resetVision()
    {
        mAffectingShadowCasters.clear();
    }

    void ShadowManager::VisibilityHintNode::setVisibleFrom(ShadowCaster* sc)
    {
        SDL_assert(sc);
        mAffectingShadowCasters.emplace(sc);
    }

    void ShadowManager::VisibilityHintNode::setInvisibleFrom(ShadowCaster* sc)
    {
        SDL_assert(sc);
        mAffectingShadowCasters.erase(sc);
    }

    void ShadowManager::VisibilityHintNode::invalidate()
    {
        for (auto shadowCaster: mAffectingShadowCasters)
        {
            shadowCaster->invalidate();
        }
    }

    void ShadowManager::VisibilityHintNode::updatePosition(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowManager::VisibilityHintNode::updateRotation(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowManager::VisibilityHintNode::updateScale(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowManager::VisibilityHintNode::updateSkeletonPose(SceneNodeBase *node)
    {
        invalidate();
    }

    ShadowManager::ShadowManager(Main& main, PipelineBase &pipeline) : 
        mMain(main),
        mPipeline(pipeline)
    {}

    ShadowManager::~ShadowManager()
    {
        clear();
    }

    stl<ShadowCaster*>::vector ShadowManager::registerEmitter(LightSceneNode* emitter)
    {
        SDL_assert(emitter);

        if (!static_cast<bool>(emitter->getLight()->getFlags() & 
            (LightSourceFlags::ShadowDynamic | LightSourceFlags::ShadowStatic)))
        {
            LITE3D_THROW("Emitter '" << emitter->getName() << "' does not contain shadow parameters");
        }

        stl<ShadowCaster*>::vector shadowCasters;

        switch (emitter->getLight()->getType())
        {
            case LightSourceFlags::TypeDirectional:
                {
                    if (mCascadeShadowIsReserved)
                    {
                        LITE3D_THROW("Cascade shadows implementation currently supports only one directional light");
                    }

                    for (uint32_t i = 0; i < mOmniShadowCacheMaxCount; ++i)
                    {
                        auto it = mShadowCasters.emplace(emitter, std::make_unique<ShadowCasterCascade>(mMain, emitter, i));
                        shadowCasters.push_back(it->second.get());
                    }
                    mCascadeShadowIsReserved = true;
                }
                break;
            case LightSourceFlags::TypeDiskArea:
            case LightSourceFlags::TypeRectArea:
            case LightSourceFlags::TypeSpot:
                {
                    auto it = mShadowCasters.emplace(emitter, std::make_unique<ShadowCasterSpot>(mMain, emitter));
                    shadowCasters.push_back(it->second.get());
                }
                break;
            case LightSourceFlags::TypePoint:
                {
                    for (uint32_t i = 0; i < 6; ++i)
                    {
                        auto it = mShadowCasters.emplace(emitter, std::make_unique<ShadowCasterOmniDirectional>(mMain, emitter, i));
                        shadowCasters.push_back(it->second.get());
                    }
                }
                break;
            default:
                {
                    LITE3D_THROW("Unsupported emitter, name '" << emitter->getName() << "', type " << 
                        static_cast<int>(emitter->getLight()->getType()));
                }
                break;
        }

        return shadowCasters;
    }

    void ShadowManager::unregisterEmitter(LightSceneNode* emitter)
    {
        SDL_assert(emitter);

        auto range = mShadowCasters.equal_range(emitter);
        if (range.first == mShadowCasters.end())
        {
            LITE3D_THROW("Unable to delete shadow caster, emitter '" << emitter->getName() << "' is not found");
        }

        while (range.first != range.second)
        {
            SDL_assert(range.first->second->getNode() == emitter);

            if (range.first->second->cached())
            {
                // Remove from cache
                auto index = range.first->second->getCacheIndex();
                emitter->getLight()->setShadowIndex(-1);
                mShadowCastersCachePlaceHolders[index] = nullptr;
            }
 
            // Remove caster from tracked objects 
            for (auto& node: mVisibilityHintNodes)
            {
                node.second->setInvisibleFrom(range.first->second.get());
            }

            range.first = mShadowCasters.erase(range.first);
        }
    }

    void ShadowManager::clear()
    {
        for (auto &shadowCaster : mShadowCastersCachePlaceHolders)
        {
            if (shadowCaster)
            {
                shadowCaster->setCacheIndex(-1);
            }

            shadowCaster = nullptr;
        }

        mVisibilityHintNodes.clear();
        mShadowCasters.clear();
    }

    ShadowManager::VisibilityHintNode* ShadowManager::registerHintNode(SceneNodeBase *node, bool recursive)
    {
        auto it = mVisibilityHintNodes.find(node);
        if (it != mVisibilityHintNodes.end())
        {
            return it->second.get();
        }

        auto ins = mVisibilityHintNodes.try_emplace(node, std::make_unique<VisibilityHintNode>(node, recursive));
        // Force update one of the cached shadows, to cause update hint nodes visibility in customFrustumCheck 
        for (auto shadowCaster : mShadowCastersCachePlaceHolders)
        {
            if (shadowCaster)
            {
                shadowCaster->invalidate();
                break;
            }
        }

        return ins.first->second.get();
    }

    void ShadowManager::unregisterHintNode(SceneNodeBase *node)
    {
        mVisibilityHintNodes.erase(node);
    }

    bool ShadowManager::beginUpdate(RenderTarget *rt)
    { 
        SDL_assert(mShadowMatrixBuffer);
        SDL_assert(mShadowIndexBuffer);

        mHostShadowIndexes.resize(1, 0); // Reserve 0 index for size
        
        for (auto it = mShadowCasters.begin(); it != mShadowCasters.end();) 
        {
            // Больше источников чем в mMaxShadowsRebuildCount за один кадр перестроить нельзя, остальные 
            // доделаем потом, в следующих кадрах
            if (mHostShadowIndexes.size() >= mMaxShadowsRebuildCount)
                break;

            // Смотрим только на видимые в кадре источники света, остальные пока не интересуют
            if (it->first->getLight()->enabled() && it->first->isVisible())
            {
                // Если источник уже кеширован, теневая карта уже отрисована, проверим, может нужно ее перерисовать?
                if (it->second->cached())
                {
                    if (it->second->invalidated())
                    {
                        auto matrix = it->second->recalcMatrix();
                        uint32_t index = static_cast<uint32_t>(it->second->getCacheIndex());
                        mShadowMatrixBuffer->setElement<kmMat4>(index, &matrix);
                        mHostShadowIndexes.push_back(index);
                    }

                    ++it;
                    continue;
                }

                // Если сточник не кеширован, значит надо попробовать его закешировать и построить теневую карту
                uint32_t step = 1;
                uint32_t firstIndexToSearch = 0;
                uint32_t searchCount = 0;
                switch (it->second->getEmitterType())
                {
                    case ShadowCaster::EmitterType::CascadeShadow:
                        firstIndexToSearch = 0;
                        searchCount = mCascadeShadowCacheMaxCount;
                        step = mCascadeShadowCacheMaxCount;
                        break;
                    case ShadowCaster::EmitterType::OmniDirectionalShadow:
                        firstIndexToSearch = mCascadeShadowCacheMaxCount;
                        searchCount = mOmniShadowCacheMaxCount * 6;
                        step = 6;
                        break;
                    case ShadowCaster::EmitterType::SpotShadow:
                        firstIndexToSearch = mCascadeShadowCacheMaxCount + (mOmniShadowCacheMaxCount * 6);
                        searchCount = mSpotShadowCacheMaxCount;
                        step = 1;
                        break;
                };

                if ((mHostShadowIndexes.size() + step - 1) >= mMaxShadowsRebuildCount)
                {
                    std::advance(it, step);
                    continue;
                }
                
                bool placeFound = false;
                for (auto i = firstIndexToSearch; i < (firstIndexToSearch + searchCount); i += step)
                {
                    // нашли свободное место в кеше
                    if (!mShadowCastersCachePlaceHolders[i])
                    {
                        for (auto j = i; j < (i + step); ++it, ++j)
                        {
                            mShadowCastersCachePlaceHolders[j] = it->second.get();
                            it->second->setCacheIndex(j);

                            auto matrix = it->second->recalcMatrix();
                            mShadowMatrixBuffer->setElement<kmMat4>(j, &matrix);
                            mHostShadowIndexes.push_back(j);
                        }

                        placeFound = true;
                        break;
                    }
                }

                if (placeFound)
                {
                    continue;
                }
                    
                // Если свободного места не нашлось, про буем вытянуть из кеша старый источник, который не виден
                for (auto i = firstIndexToSearch; i < (firstIndexToSearch + searchCount); i += step)
                {
                    if (mShadowCastersCachePlaceHolders[i] && (!mShadowCastersCachePlaceHolders[i]->getNode()->isVisible() || 
                        !mShadowCastersCachePlaceHolders[i]->getNode()->getLight()->enabled()))
                    {
                        for (auto j = i; j < (i + step); ++it, ++j)
                        {
                            // Выкинуть из кеша старый
                            mShadowCastersCachePlaceHolders[j]->setCacheIndex(-1);
                            // Воткнуть на его место новый
                            mShadowCastersCachePlaceHolders[j] = it->second.get();
                            it->second->setCacheIndex(j);

                            auto matrix = it->second->recalcMatrix();
                            mShadowMatrixBuffer->setElement<kmMat4>(j, &matrix);
                            mHostShadowIndexes.push_back(j);
                        }

                        placeFound = true;
                        break;
                    }
                }

                if (placeFound)
                {
                    continue;
                }

                std::advance(it, step);
            }
            else
            {
                ++it;
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

    bool ShadowManager::beginSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params)
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

    void ShadowManager::endSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params)
    {
        // После очистки теневых карт готовимся к перерисовке теней.
        if (scene == mCleanStage)
        {
            RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
        }
    }

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool ShadowManager::customFrustumCheck(Scene *scene, SceneNodeBase *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera, const lite3d_scene_render_params *params)
    {
        if (static_cast<RenderPassStagePriority>(params->priority) != RenderPassStagePriority::ShadowBuildStage)
        {
            return true;
        }

        auto it = mVisibilityHintNodes.find(node);
        VisibilityHintNode* dnode = it != mVisibilityHintNodes.end() ? it->second.get() : nullptr;

        bool isVisible = false;
        for (auto shadowCaster: mShadowCastersCachePlaceHolders)
        {
            if (!shadowCaster)
                continue;

            if (shadowCaster->intersectFrustum(*boundingVol))
            {
                if (dnode && shadowCaster->dynamicShadow())
                {
                    // Текущая нода видима для этого истоника света, запомним это
                    dnode->setVisibleFrom(shadowCaster);
                }

                // Рисуем только те обьекты которые попадают в область видимости источников света которые сейчас обновляются
                if (std::find(mHostShadowIndexes.begin()+1, mHostShadowIndexes.end(), shadowCaster->getCacheIndex()) !=
                    mHostShadowIndexes.end())
                {
                    isVisible = true;
                }
            }
            else
            {
                if (dnode && shadowCaster->dynamicShadow())
                {
                    // Текущая нода НЕ видима для этого истоника света
                    dnode->setInvisibleFrom(shadowCaster);
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
            auto shadowCasterPlaceHolder = mShadowCastersCachePlaceHolders[mHostShadowIndexes[i]];
            if (shadowCasterPlaceHolder)
            {
                shadowCasterPlaceHolder->validate();
            }
        }

        mHostShadowIndexes.clear();
    }

    void ShadowManager::createAuxiliaryBuffers()
    {
        mShadowMatrixBuffer = mMain.getResourceManager().queryResourceFromJson<UBO>(mPipeline.getName() + "_ShadowMatrixBuffer",
            "{\"Dynamic\": true}", &mPipeline);
        mShadowIndexBuffer = mMain.getResourceManager().queryResourceFromJson<UBO>(mPipeline.getName() + "_ShadowIndexBuffer",
            "{\"Dynamic\": true}", &mPipeline);

        mShadowMatrixBuffer->extendBufferBytes(sizeof(kmMat4) * getShadowsCacheMaxCount());
        mShadowIndexBuffer->extendBufferBytes(sizeof(IndexVector::value_type) * (getShadowsCacheMaxCount() + 1));
        IndexVector::value_type initialZero = 0;
        mShadowIndexBuffer->setElement<IndexVector::value_type>(0, &initialZero);

        mHostShadowIndexes.reserve(getShadowsCacheMaxCount() + 1);
        mShadowCastersCachePlaceHolders.resize(getShadowsCacheMaxCount(), nullptr);
    }

    void ShadowManager::setupLimits()
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

        mMaxShadowsRebuildCount = std::min(std::min(a, b), c);

        if (getShadowsCacheMaxCount() > c)
        {
            LITE3D_THROW("Shadow matrix buffer limit exceeded (" << getShadowsCacheMaxCount() << " of " << c << ")");
        }

        ShaderProgram::addGlobalDefinition("LITE3D_SPOT_SHADOW_GS_MAX_VERTICES", std::to_string(mMaxShadowsRebuildCount * 3));
        ShaderProgram::addGlobalDefinition("LITE3D_SHADOW_CACHE_MAX_COUNT", std::to_string(getShadowsCacheMaxCount()));
    }

    void ShadowManager::createShadowRenderTarget()
    {
        auto shadowMapName = mPipeline.getName() + "_ShadowMap.texture";
        ConfigurationWriter shadowTextureConfig;
        shadowTextureConfig.set(L"TextureType", "2D_SHADOW_ARRAY")
            .set(L"Filtering", "Linear")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "DEPTH")
            .set(L"Height", mExtent)
            .set(L"Width", mExtent)
            .set(L"Depth", getShadowsCacheMaxCount());

        mShadowMap = mMain.getResourceManager().queryResourceFromJson<TextureImage>(shadowMapName, 
            shadowTextureConfig.write(), &mPipeline);

        ConfigurationWriter shadowRenderTargetConfig;
        shadowRenderTargetConfig.set(L"Width", mExtent)
            .set(L"Height", mExtent)
            .set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::ShadowMap))
            .set(L"CleanColorBuf", false)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false)
            .set(L"LayeredFramebuffer", true)
            .set(L"DepthAttachments", ConfigurationWriter()
                .set(L"TextureName", shadowMapName));

        mShadowPass = mMain.getResourceManager().queryResourceFromJson<TextureRenderTarget>(mPipeline.getName() + "_ShadowPass",
            shadowRenderTargetConfig.write(), &mPipeline);
        mShadowPass->addObserver(this);
    }

    void ShadowManager::initialize()
    {
        auto shadowParams = mPipeline.getConfig().getObject(L"ShadowMaps");
        mSpotShadowCacheMaxCount = shadowParams.getInt(L"SpotShadowCacheMaxCount", 0);
        mOmniShadowCacheMaxCount = shadowParams.getInt(L"OmniShadowCacheMaxCount", 0);
        mCascadeShadowCacheMaxCount = shadowParams.getInt(L"CascadeShadowCacheMaxCount", 0);
        mExtent = shadowParams.getInt(L"Extent", 512);

        setupLimits();
        createAuxiliaryBuffers();
        createShadowRenderTarget();

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
            
        mCleanStage = mMain.getResourceManager().queryResourceFromJson<Scene>(mPipeline.getName() + "_ShadowCleanStage",
            stageGenerator.generate().write(), &mPipeline);

        ConfigurationWriter cleanStageMaterialConfig;
        cleanStageMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::ShadowPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "ShadowMapClean.program")
                    .set(L"Path", mPipeline.getConfig().getString(L"ShaderPackage") + ":shaders/json/shadow_map_clean.json"))
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
        auto cleanStageMaterial = mMain.getResourceManager().queryResourceFromJson<Material>(
            mPipeline.getName() + "_ShadowCleanStage.material", cleanStageMaterialConfig.write(), &mPipeline);

        // Добавляем шейдер очистки на сцену 
        mCleanStage->addObject("ShadowCleanBigTri", BigTriObjectGenerator(cleanStageMaterial->getName()).generate());
        mCleanStage->addObserver(this);
    }
}}
