/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any layer version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <lite3dpp_pipeline/lite3dpp_gi_probe_manager.h>

#include <algorithm>
#include <SDL_assert.h>

#include <lite3dpp_pipeline/lite3dpp_pipeline_base.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

GIProbeManager::GIProbeManager(Main& mMain, PipelineBase &pipeline) : 
    mMain(mMain),
    mPipeline(pipeline),
    mPipelineName(pipeline.getName()),
    mShaderPackage(pipeline.getConfig().getString(L"ShaderPackage"))
{
    mMain.addObserver(this);

    int32_t maxSupportedThreadsGroupX;
    lite3d_shader_program_get_workgroup_limits(&maxSupportedThreadsGroupX, nullptr, nullptr);
    mSHMaxThreads = std::min(256, maxSupportedThreadsGroupX);
}

GIProbeManager::~GIProbeManager()
{
    mMain.removeObserver(this);
}

void GIProbeManager::initialize()
{
    ConfigurationReader config = mPipeline.getConfig().getObject(L"GI");
    mProbeCount = config.getInt(L"MaxCount", 1);
    mzNear = config.getDouble(L"ProbeZNearClip", 0.0f);
    mzFar = config.getDouble(L"ProbeZFarClip", 1.0f);

    calculateProbeBatchCount();

    if (!mMain.getResourceManager().resourceExists(GGXLUTName))
    {
        integrateGGX();
    }
    else
    {
        mBrdfLUT = mMain.getResourceManager().queryResource<TextureImage>(GGXLUTName);
    }

    // Буфер под информацию о пробниках 
    mProbesBuffer = mMain.getResourceManager().queryResourceFromJson<SSBO>(mPipelineName + "_EnvProbesData",
        "{\"Dynamic\": true}", &mPipeline);
    mProbesBuffer->extendBufferBytes(sizeof(ProbeRawEntity) * mProbeCount);

    // Буфер индексов для обновления пробников
    mProbesIndexBuffer = mMain.getResourceManager().queryResourceFromJson<UBO>(mPipelineName + "_EnvProbesIndex",
        "{\"Dynamic\": true}", &mPipeline);
    mProbesIndexBuffer->extendBufferBytes(sizeof(decltype(mProbesIndex)::value_type) * LITE3D_ALIGN(mMaxProbeBatchCount + 1, 4));

    createProbePass(config);
    createPrefilterShader(config);
    createDiffuseRadianceShader();
}

void GIProbeManager::integrateGGX()
{
    const int32_t LUTSize = 512;
    ConfigurationWriter IntergratedGGXLUTConfig;
    IntergratedGGXLUTConfig.set(L"TextureType", "2D")
        .set(L"Filtering", "Linear")
        .set(L"Width", LUTSize)
        .set(L"Height", LUTSize)
        .set(L"Depth", 1)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RG")
        .set(L"InternalFormat", "RG16F");

    mBrdfLUT = mMain.getResourceManager().queryResourceFromJson<TextureImage>(GGXLUTName, 
        IntergratedGGXLUTConfig.write());
    mBrdfLUT->pin(true);

    ConfigurationWriter shaderParams;
    shaderParams.set(L"Program", ConfigurationWriter()
        .set(L"Name", "IntergrateGGX.program")
        .set(L"Path", mShaderPackage + ":shaders/json/integrate_ggx.json"))
        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Type", "imageStore")
            .set(L"Name", "BrdfLUT")
            .set(L"Direction", "output")
            .set(L"TextureName", mBrdfLUT->getName())
    });

    auto intergrateGGXShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>("IntergrateGGX.comp",
        shaderParams.write(), &mPipeline);

    // 16 нитей в группе
    const uint32_t groupsCount = (LUTSize + 16 - 1) / 16;
    intergrateGGXShader->dispatch(groupsCount, groupsCount, 1);

    // Освобождаем ресурсы, расчет GGX делается только однажды, более шейдер не требуется
    mMain.getResourceManager().releaseResource(intergrateGGXShader->getProgram()->getName());
    mMain.getResourceManager().releaseResource(intergrateGGXShader->getName());
}

void GIProbeManager::calculateProbeBatchCount()
{
    int maxGeometryOutputVertices, maxGeometryTotalOutputComponents, SSBOMaxSize;
    lite3d_shader_program_get_limitations(&maxGeometryOutputVertices, nullptr, &maxGeometryTotalOutputComponents);
    lite3d_vbo_get_limitations(nullptr, nullptr, &SSBOMaxSize);

    // Число компонент на одну вершину в геометрическом шейдере для рендера пробников
    // Константа связана с кодом шейдера!!!
    const uint32_t probeComponentsByVertex = 14; // pos + iuv + iwv + iwn + drawID + LayerID
    uint32_t a = maxGeometryTotalOutputComponents / (probeComponentsByVertex * 3 * 6);
    uint32_t b = maxGeometryOutputVertices / (3 * 6);
    mMaxProbeBatchCount = std::min(a, b);

    uint32_t maxPossibleProbeCount = SSBOMaxSize / sizeof(ProbeRawEntity);
    uint32_t maxProbeCount = std::min(maxPossibleProbeCount, MaxProbeCount);

    if (mProbeCount > maxProbeCount)
    {
        LITE3D_THROW("Too many probe count(" << mProbeCount << ") are requested, "
            "max supported limit is " << maxProbeCount);
    }

    ShaderProgram::addGlobalDefinition("LITE3D_ENV_PROBE_GS_MAX_VERTICES", std::to_string(mMaxProbeBatchCount * 3 * 6));
    ShaderProgram::addGlobalDefinition("LITE3D_ENV_PROBE_MAX", std::to_string(maxProbeCount));
}

void GIProbeManager::createProbePass(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"ProbeResolution", DefaultProbeResolution);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Depth", mProbeCount)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB16F");

    mEnvironmentProbeColor = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbe.texture", mapConfig.write(), &mPipeline);

    // Создание массива кубических текстур глубины для рендера сцены
    mapConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mEnvironmentProbeDepth = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbeDepth.texture", mapConfig.write(), &mPipeline);

    ConfigurationWriter passConfig;
    passConfig.set(L"Priority", static_cast<uint32_t>(RenderPassPriority::GIProbe))
        .set(L"CleanColorBuf", false)
        .set(L"CleanDepthBuf", false)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mEnvironmentProbeColor->getName())
            }))
        .set(L"DepthAttachments", ConfigurationWriter()
            .set(L"TextureName", mEnvironmentProbeDepth->getName()));

    mEnvironmentProbeColorPass = mMain.getResourceManager().queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_EnvironmentMultiProbePass", passConfig.write(), &mPipeline);
    mEnvironmentProbeColorPass->addObserver(this);
}

void GIProbeManager::createPrefilterShader(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"ProbeResolution", DefaultProbeResolution);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Depth", mProbeCount)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGBA")
        .set(L"InternalFormat", "RGBA16F");

    mPrefilteredEnvironment = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_PrefilteredEnvironmentMultiProbe.texture", mapConfig.write(), &mPipeline);

    ConfigurationWriter shaderParams;
    shaderParams.set(L"Program", ConfigurationWriter()
        .set(L"Name", "PrefilterEnvironment.program")
        .set(L"Path", mShaderPackage + ":shaders/json/prefilter_environment.json"))
        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Type", "imageStore")
            .set(L"Name", "PrefilteredEnvironment")
            .set(L"Direction", "output")
            .set(L"TextureName", mPrefilteredEnvironment->getName()),
        ConfigurationWriter().set(L"Type", "sampler")
            .set(L"Name", "EnvironmentProbe")
            .set(L"TextureName", mEnvironmentProbeColor->getName()),
        ConfigurationWriter().set(L"Type", "float")
            .set(L"Name", "Roughness")
            .set(L"Value", 1.0f),
        ConfigurationWriter().set(L"Type", "UBO")
            .set(L"Name", "EnvProbesIndex")
            .set(L"UBOName", getProbeIndexBufferName())
    });

    mPrefilterEnvironmentShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>(
        mPipelineName + "_PrefilterEnvironment.comp", shaderParams.write(), &mPipeline);
}

void GIProbeManager::createDiffuseRadianceShader()
{
    ConfigurationWriter shaderParams;
    shaderParams.set(L"Program", ConfigurationWriter()
        .set(L"Name", "DiffuseRadianceSHEnvironment.program")
        .set(L"Path", mShaderPackage + ":shaders/json/diffuse_radiance_sh.json"))
        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Type", "sampler")
            .set(L"Name", "EnvironmentProbes")
            .set(L"TextureName", mEnvironmentProbeColor->getName()),
        ConfigurationWriter().set(L"Type", "SSBO")
            .set(L"Name", "ProbesData")
            .set(L"SSBOName", getProbeBufferName()),
        ConfigurationWriter().set(L"Type", "UBO")
            .set(L"Name", "ProbesIndex")
            .set(L"UBOName", getProbeIndexBufferName())
    });

    ShaderProgram::addGlobalDefinition("LITE3D_SH_THREADS", std::to_string(mSHMaxThreads));
    mDiffuseRadianceSHShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>(
        mPipelineName + "_DiffuseRadianceSHEnvironment.comp", shaderParams.write(), &mPipeline);
}

void GIProbeManager::rebuild()
{
    for (auto &probe : mProbes)
    {
        probe.invalidate();
    }
}

bool GIProbeManager::beginUpdate(RenderTarget *rt)
{
    // Nothing to do
    if (std::find_if(mProbes.begin(), mProbes.end(), [](const EnvProbe &e) { return e.invalidated(); }) == mProbes.end())
    {
        return false;
    }

    SDL_assert((mProbes.size() * sizeof(ProbeRawEntity)) <= mProbesBuffer->bufferSizeBytes());
    mProbesIndex.resize(1);
    
    {
        auto lock = mProbesBuffer->map(BufferScopedMapper::BufferScopedMapperLockType::LockTypeWrite);
        auto probeBlock = lock.getPtr<ProbeRawEntity>(); 

        for (size_t i = 0; i < mProbes.size(); ++i)
        {
            if (mProbes[i].invalidated())
            {
                mProbes[i].rebuildMatrix();
                mProbes[i].writeProbe(&probeBlock[i]);
                mProbesIndex.emplace_back(i);
            }

            if (mProbesIndex.size() == (mMaxProbeBatchCount + 1))
            {
                break;
            }
        }
    }
    
    mProbesIndex[0] = static_cast<int32_t>(mProbesIndex.size() - 1);
    mProbesIndexBuffer->setData(&mProbesIndex[0], 0, sizeof(decltype(mProbesIndex)::value_type) * mProbesIndex.size());
    
    // Clear affected probes
    prepareAffectedProbes();

    return true;
}

bool GIProbeManager::customFrustumCheck(Scene *scene, SceneNodeBase *node, lite3d_mesh_chunk *meshChunk, Material *material,
    lite3d_bounding_vol *boundingVol, Camera *camera, const lite3d_scene_render_params *params)
{
    if (params->pass != static_cast<uint16_t>(TexturePassTypes::GIProbePass))
    {
        return true;
    }
    
    return node->isGIVisible();
}

void GIProbeManager::prepareAffectedProbes()
{
    const float cleanDepth = 1.0;
    const float cleanProbeColor[4] = { 0.0, 0.0, 0.0, 1.0 };
    // Clear affected probes
    for (size_t i = 1; i < mProbesIndex.size(); ++i)
    {
        auto probeIndex = mProbesIndex[i];
        for (int32_t layer = probeIndex * 6; layer < (probeIndex * 6) + 6; ++layer)
        {
            for (int8_t level = 0; level < mEnvironmentProbeColor->getTotalLevels(); ++level)
            {
                mEnvironmentProbeColor->clearPixels(cleanProbeColor, level, layer);
            }

            mEnvironmentProbeDepth->clearPixels(&cleanDepth, 0, layer);
        }
    }
}

void GIProbeManager::postUpdate(RenderTarget *rt)
{
    // nothing to do
    if (mProbesIndex.size() == 0 || mProbesIndex[0] == 0)
    {
        return;
    }
/*
    auto totalLevels = mPrefilteredEnvironment->getTotalLevels();
    auto levelSize = mPrefilteredEnvironment->getWidth();
    for (int8_t level = 0; level < totalLevels; ++level)
    {
        // Calculate a roughness value in the range [0, 1] based on the
        // current mip level and the total number of mip levels.
        const auto roughness = static_cast<float>(level) / static_cast<float>(totalLevels);
        mPrefilterEnvironmentShader->getShaderParameters().setFloatParameter("Roughness", roughness);
        // Bind the current mip level of the environment probe texture
        mPrefilterEnvironmentShader->getShaderParameters().getParameter("PrefilteredEnvironment", 
            LITE3D_SHADER_PARAMETER_IMAGE_STORE)->imageMipLevel = level;

        const uint32_t groupsCount = (levelSize + 16 - 1) / 16;
        mPrefilterEnvironmentShader->dispatch(groupsCount, groupsCount, mProbesIndex[0] * 6);

        // Shift to the next mip level
        levelSize >>= 1;
    }
*/
    // Recalc diffuse radiance SH coeffs foreach every affected probe in one dispatch call
    mDiffuseRadianceSHShader->dispatch(mProbesIndex.size() - 1, 1, 1);
}

size_t GIProbeManager::addProbe(const kmVec3 &position, EnvProbeFlags flags)
{
    if ((flags & EnvProbeFlags::Reusable) == EnvProbeFlags::Reusable)
    {
        LITE3D_THROW("Reusable is a technical flag and cannot be used to add a probe");
    }

    for (size_t i = 0; i < mProbes.size(); ++i)
    {
        if (mProbes[i].isReusable())
        {
            mProbes[i].setFlags(flags);
            mProbes[i].setPosition(position);
            return i;
        }
    }

    if (mProbes.size() == mProbeCount)
    {
        LITE3D_THROW("The maximum probes count limit is reached: " << mProbeCount);
    }

    mProbes.emplace_back(&mMain, mzNear, mzFar, flags);
    mProbes.back().setPosition(position);
    return mProbes.size() - 1;
}

void GIProbeManager::updateProbe(size_t index, const kmVec3 &position)
{
    if (index >= mProbes.size())
    {
        LITE3D_THROW("Index out of range: " << index << " of " << mProbes.size());
    }

    if (mProbes[index].isReusable())
    {
        LITE3D_THROW("Cannot update a removed probe: " << index);
    }

    mProbes[index].setPosition(position);
}

void GIProbeManager::removeProbe(size_t index)
{
    if (index >= mProbes.size())
    {
        LITE3D_THROW("Index out of range: " << index << " of " << mProbes.size());
    }

    const auto flags = mProbes[index].getFlags() | EnvProbeFlags::Reusable;
    const auto rawFlags = static_cast<uint32_t>(flags);
    mProbesBuffer->setData(&rawFlags, index * sizeof(ProbeRawEntity) + offsetof(ProbeRawEntity, flags), sizeof(rawFlags));
    mProbes[index].setFlags(flags);
}

GIProbeManager::EnvProbe::EnvProbe(Main *main, float zNear, float zFar, EnvProbeFlags flags) : 
    mProbeCamera(std::make_shared<Camera>("", main)),
    mFlags(flags)
{
    mProbeCamera->setupPerspective(zNear, zFar, 90.0f, 1.0f);
}

void GIProbeManager::EnvProbe::rebuildMatrix()
{
    mProbeCamera->computeCubeProjView(mViewProjMatrices);
    mInvalidated = false;
}

void GIProbeManager::EnvProbe::setPosition(const kmVec3 &pos)
{
    mProbeCamera->setPosition(pos);
    invalidate();
}

void GIProbeManager::EnvProbe::writeProbe(GIProbeManager::ProbeRawEntity *probe) const
{
    SDL_assert(mViewProjMatrices.size() == 6);
    SDL_assert(probe);

    memset(probe, 0, sizeof(GIProbeManager::ProbeRawEntity));

    probe->position.x = mProbeCamera->getPosition().x;
    probe->position.y = mProbeCamera->getPosition().y;
    probe->position.z = mProbeCamera->getPosition().z;
    probe->position.w = 0.0f;
    
    std::copy(mViewProjMatrices.begin(), mViewProjMatrices.end(), probe->viewProjMatrices);
    probe->flags[0] = static_cast<uint32_t>(mFlags);
}

}}
